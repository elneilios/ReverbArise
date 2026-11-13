#pragma once
#include "WorkspaceTheme.h"
#include "SongBarRuler.h"
#include "LearnTimelineModel.h"
#include "Controls/Switch.h"
#include "Controls/SliderControl.h"
#include "Controls/FieldNumericInput.h"
#include "SoundPresetBinding.h"
#include "Controls/ActionButton.h"
#include "Controls/ActionButtonStateField.h"
#include "Controls/LatchingActionButton.h"
#include "Controls/SegmentedSelector.h"
#include "Controls/ChoiceSelector.h"
#include "Controls/FocusViewport.h"
#include "Controls/WrappedLabel.h"
#include "Controls/WaveformGestures.h"
#include <DSP/SwellEngine.h>
#include <Capture/CaptureProgress.h>
#include <Capture/LoopCapturePreference.h>
#include <span>

namespace m3::ui
{
// Learn owns a separate dry-only canvas. It has no access to Perform navigation
// or wet editing. Provisional display telemetry is separate from accepted source.
class LearnWorkspace final:public juce::Component,public juce::SettableTooltipClient
{
public:
    LearnWorkspace(SwellEngine& source,std::function<workflow::Snapshot()> readFlow):engine(source),flowNow(std::move(readFlow)),irBinding(irEditor,source)
    {
        setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(false);
        setTooltip(inspectionHelp);setName("learnWorkspace");setTitle("Learn: source capture and generation preferences");
        for(auto* c:std::array<juce::Component*,13>{&captureTitle,&recaptureHelp,&recapture,&automaticLabel,&automaticHelp,&automatic,
            &autoStopHelp,&autoStop,&irEditor,&sensitivityLabel,&sensitivity,&generate,&manualHelp})addAndMakeVisible(c);
        captureTitle.setText("Source Capture",juce::dontSendNotification);applyTextRole(captureTitle,TextRole::sectionHeading);
        automaticLabel.setText("Automatic Swells",juce::dontSendNotification);applyTextRole(automaticLabel,TextRole::sectionHeading);
        sensitivityLabel.setText("Event sensitivity",juce::dontSendNotification);
        applyTextRole(sensitivityLabel,TextRole::controlLabel);
        for(auto* label:std::initializer_list<juce::Label*>{&recaptureHelp,&automaticHelp,&autoStopHelp})applyTextRole(*label,TextRole::helper,juce::Justification::topLeft);
        for(auto* label:std::initializer_list<juce::Label*>{&captureTitle,&automaticLabel,&sensitivityLabel,&recaptureHelp,&automaticHelp,&autoStopHelp,&manualHelp})label->setBorderSize({});
        automaticHelp.setText({},juce::dontSendNotification);
        autoStopHelp.setText({},juce::dontSendNotification);
        applyTextRole(manualHelp,TextRole::helper,juce::Justification::centred);
        manualHelp.setText(manualPlacementHelp,juce::dontSendNotification);
        captureTitle.setName("learnCaptureHeading");automaticLabel.setName("learnAutomaticHeading");
        irEditor.setName("learnIrEditor");irEditor.presetPicker().setName("learnDefaultIR");
        irEditor.importAction().setName("learnImportIR");irEditor.pickerLabel().setName("learnIRLabel");
        sensitivityLabel.setName("learnSensitivityLabel");recaptureHelp.setName("learnRecaptureHelp");automaticHelp.setName("learnAutomaticHelp");
        recapture.setName("recaptureExisting");automatic.setName("learnAutomaticSwells");sensitivity.setName("learnMarkerSensitivity");
        autoStop.setName("learnAutoGenerateOnStop");autoStopHelp.setName("learnAutoGenerateHelp");
        recapture.setTooltip("Replace captured audio encountered during the next Learn pass.");
        automatic.setTooltip("Detect events in captured audio for automatic swells.");
        autoStop.setTooltip("Generate swells when the eligible Learn pass stops.");
        for(auto* toggle:{&recapture,&automatic,&autoStop})toggle->setUnavailableReason("Wait for the current capture pass to finish.");
        generate.setName("learnGenerateSwells");manualHelp.setName("learnManualSwellsHelp");
        autoStop.beforeInvocation=[this]{autoStopIntent=GenerationIntent{displaySession,displayPass,0};};
        autoStop.onClick=[this]{const auto intent=autoStopIntent.value_or(GenerationIntent{displaySession,displayPass,0});autoStopIntent.reset();
            engine.setAutoGenerateOnStop(autoStop.getToggleState(),intent.session,intent.pass);if(onRefresh)onRefresh();};
        generate.beforeInvocation=[this]{generateIntent=GenerationIntent{displaySession,displayPass,generationView.displayed?generationView.displayed->serial:0};};
        generate.onClick=[this]{const auto intent=generateIntent.value_or(GenerationIntent{displaySession,displayPass,generationView.displayed?generationView.displayed->serial:0});generateIntent.reset();
            engine.generateLearn(intent.session,intent.pass,intent.serial);if(onRefresh)onRefresh();};
        recapture.beforeInvocation=[this]{recapturePass=flowNow().pass;};automatic.beforeInvocation=[this]{automaticPass=flowNow().pass;};
        recapture.onClick=[this]{if(recapturePass.value_or(flowNow().pass)==flowNow().pass && onRecapture)onRecapture(recapture.getToggleState());if(onRefresh)onRefresh();};
        automatic.onClick=[this]{engine.setLearningAutomatic(automatic.getToggleState(),automaticPass.value_or(flowNow().pass));if(onRefresh)onRefresh();};
        sensitivity.setRange(0,100,.1);
        sensitivity.setWheelSteps(1,.1);
        sensitivity.setTextValueSuffix(" %");sensitivity.setNumDecimalPlacesToDisplay(1);
        sensitivity.setTooltip("Low: fewer, stronger attacks. High: finer event detail. Updates the marker preview asynchronously.");
        sensitivity.beforeInvocation=[this]{sensitivityPass=flowNow().pass;};
        sensitivity.onValueChange=[this]{if(!refreshing){const auto s=engine.savedSession();engine.setPreferences(s.auditionDb,static_cast<float>(sensitivity.getValue()/100),s.separationSeconds,s.autoRegenerate,sensitivityPass.value_or(flowNow().pass));}};
        irBinding.onRefresh=[this]{if(onRefresh)onRefresh();};
        setupLoopControls();
        loopPoliciesViewport.setName("learnLoopPolicies");
        loopPoliciesViewport.setViewedComponent(&loopPolicies,false);
        loopPoliciesViewport.setScrollBarsShown(true,false);
        loopPoliciesViewport.setFocusRevealEnabled(true);
        addChildComponent(loopPoliciesViewport);
    }
    std::function<void(bool)> onRecapture;
    std::function<void()> onRefresh;
    std::function<void(LoopCapturePreference)> onLoopPreference;
    std::function<void(bool,uint64_t,uint64_t,LoopCapturePreference)> onCaptureArm;
    void displayCaptureControls(LoopCapturePreference preference, bool canArm, bool armed, bool busy,
                     bool finishing, bool recaptureEnabled, const juce::String& status)
    {
        loopSettings=preference;
        loopRecaptureOn=recaptureEnabled;
        loopMode.display(preference.enabled);
        loopMode.setTooltip("Record a loop using 4/4 timing. See the user guide for host setup.");
        loopMethod.setSelectedId(static_cast<int>(preference.method)+1,juce::dontSendNotification);
        loopUnit.setSelectedId(preference.unit==LoopCapturePreference::Unit::bars?1:2,juce::dontSendNotification);
        if (!loopCount.isBeingEdited())
            loopCount.setText(juce::String(static_cast<int>(preference.count)),juce::dontSendNotification);
        for (auto* control : std::initializer_list<juce::Component*>{&loopMode,&loopMethod,&loopUnit,&loopCount})
            control->setEnabled(!busy);
        auto& arm=loopArm.action();
        arm.setToggleState(armed && !finishing,juce::dontSendNotification);
        arm.setEnabled(!finishing && (armed || canArm));
        const bool endSignal=preference.method==LoopCapturePreference::Method::endSignal;
        const bool endOnRepeat=preference.method==LoopCapturePreference::Method::endOnRepeat;
        loopMethod.setTooltip(endOnRepeat
            ? "Record one whole pass of a repeat."
            : endSignal
            ? "Click Arm again or send a Stop MIDI note to finish at the next bar line."
            : "Record for the number of bars or beats you choose.");
        auto armHelp=status;
        if (!preference.enabled && arm.isEnabled())
            armHelp=armed ? "Cancel this take. DAW Stop finishes a recording."
                          : "Arm one Song take. Recording starts during valid DAW playback.";
        else if (arm.isEnabled())
        {
            if (!armed && endOnRepeat)
                armHelp="Wait for the next repeat to start, then record one pass.";
            else if (!armed)
                armHelp=endSignal ? "Arm the next recording." : "Record the chosen length starting on a bar line.";
            else if (flowNow().phase!=workflow::Phase::recording)
                armHelp="Cancel before recording starts.";
            else
                armHelp=endSignal ? "Finish recording at the next bar line." : "Cancel this recording.";
        }
        loopArm.setActionHelp(armHelp);
        loopArm.setState(status.isNotEmpty()?status:(canArm?"Disarmed":"Capture unavailable"));
    }
    void display(const CaptureSessionSnapshot& source,const EditorSnapshot& editor,const SongTime& time,
                 workflow::Snapshot flow,bool position,const CaptureProgress& pending={},bool songTimingBlocked=false)
    {
        const bool loopSource=source.sourceKind()==CaptureSourceKind::loop;
        const bool wasFreshDomain=progress.freshDomain;
        progress=flow.locked() && pending.pass==flow.pass?pending:CaptureProgress{};
        const bool coordinateChange=wasFreshDomain!=progress.freshDomain
            || source.sourceKind()!=capture.sourceKind() || (loopSource && source.reference!=capture.reference);
        refreshing=true;capture=source;clock=time;playhead=position && !phraseTimeline();
        hostSongTimingBlocked=songTimingBlocked;
        if (phraseTimeline()) clock.valid=false;
        automaticOn=editor.session.automaticProcessing && editor.session.autoMarkersEnabled;
        generationView=editor.learnGeneration;displaySession=editor.sessionEpoch;displayPass=flow.pass;
        if(generationView.displayed)
        {
            const auto& identity=generationView.displayed->identity;
            if(progress.freshDomain || (loopSource && (identity.loop!=source.manifest.loop || identity.sourceRevision!=source.manifest.revision
                || identity.sourceReference!=source.reference || source.unavailable))
                || (!loopSource && identity.loop))
                generationView.displayed.reset();
        }
        if(loopSource && !generationView.displayed && generationView.canGenerate)
        {
            generationView.canGenerate=false;
            generationView.reason="Wait for markers from the accepted Loop.";
        }
        autoStop.setToggleState(editor.session.autoGenerateOnStop,juce::dontSendNotification);
        autoStop.setEnabled(!usesLoopPolicies() && (!flow.locked() || editor.session.autoGenerateOnStop));
        autoStop.setTooltip(usesLoopPolicies()?"Generate on Stop applies to Song capture.":juce::String{});
        const bool rebuilding=!editor.session.swells.empty() || !editor.session.suppressed.empty();
        generate.setButtonText(rebuilding?"Regenerate Swells":"Generate Swells");
        generate.setEnabled(generationView.canGenerate);
        const auto helper=automaticOn && !generationView.canGenerate?generationView.reason:juce::String(manualPlacementHelp);
        manualHelp.setText(helper,juce::dontSendNotification);
        const auto actionHelp=juce::String("Build swells from the current markers. Keep frozen swells and replace all others.");
        const auto fullHelp=generationView.canGenerate?actionHelp:generationView.reason+" "+actionHelp;
        generate.setDescription(fullHelp);generate.setHelpText(fullHelp);generate.setTooltip(fullHelp);
        phase=flow.phase;
        const bool recaptureOn=loopSettings.enabled?loopRecaptureOn:flow.recapture;
        recapture.setToggleState(recaptureOn,juce::dontSendNotification);automatic.setToggleState(automaticOn,juce::dontSendNotification);
        recapture.setEnabled(!flow.locked());automatic.setEnabled(!flow.locked());sensitivity.setEnabled(!flow.locked());
        if(!sensitivity.isMouseButtonDown() && !sensitivity.hasKeyboardFocus(true))sensitivity.setValue(editor.session.sensitivity*100,juce::dontSendNotification);
        recaptureHelp.setText(loopSettings.enabled
            ? (recaptureOn?"Allow replacing the accepted Loop.":"Keep the accepted Loop.")
            : (recaptureOn?"Replace audio next pass.":"Keep captured audio."),juce::dontSendNotification);
        irBinding.displayDefault(editor,flow.mode==workflow::Mode::capture && (!responsive || selectedSection==2));
        if(!responsive)irEditor.setVisible(automaticOn);
        for(auto* c:std::array<juce::Component*,4>{&sensitivityLabel,&sensitivity,&autoStop,&autoStopHelp})c->setVisible(automaticOn);
        const auto scope=juce::String(source.manifest.projectId)+"/"+source.manifest.instanceId;
        const auto session=engine.assignmentEpoch();
        // A mode roundtrip/reset may complete between paints. A recording pass
        // increment alone must retain the pin throughout recording/settlement.
        const auto modeReset=flow.phase==workflow::Phase::idle && flow.result==workflow::Result::none && flow.pass!=inspectionWorkflow;
        inspectionWorkflow=flow.pass;
        if(scope!=viewScope || session!=inspectionSession || modeReset || coordinateChange){
            resetNavigation();clearInspection();viewScope=scope;inspectionSession=session;}
        view.update(timelineSource(),progress,clock);
        if(manualView)viewport.constrain(fullExtent(),minimumSpan());else viewport=fullExtent();
        const auto rate=progress.sampleRate>0?progress.sampleRate:loopSource || clock.rate<=0?capture.manifest.sampleRate:clock.rate;
        const auto duration=[&](long double frames){return rate>0?juce::String(static_cast<double>(frames/rate),1)+" s":juce::String(static_cast<double>(frames),0)+" samples";};
        totalsText="Mapped "+duration(learnMappedFrames(capture.manifest))+" | Recording "+duration(learnProvisionalFrames(progress))+" | Extent "+duration(learnExtentFrames(capture,progress,clock));
        if (source.sourceKind()==CaptureSourceKind::loop || loopSettings.enabled)
        {
            const bool accepted=source.reference.isNotEmpty() || !source.manifest.ranges.empty();
            totalsText=accepted?(source.sourceKind()==CaptureSourceKind::loop?"Accepted Loop":"Accepted Song"):"No accepted source";
            if (accepted && source.unavailable) totalsText+=" | Audio unavailable";
            else if (source.manifest.loop) totalsText+=" | "+duration(source.manifest.loop->endSample);
            else if (accepted) totalsText+=" | "+duration(learnMappedFrames(source.manifest));
        }
        statusText=phase==workflow::Phase::recording?"Recording":phase==workflow::Phase::draining || phase==workflow::Phase::validating?"Finishing capture":phase==workflow::Phase::failed?"Capture failed; previous audio kept":flow.result==workflow::Result::cancelled?"Capture cancelled":source.unavailable?"Some audio unavailable":hasSource()?"Captured":hostSongTimingBlocked?"":"Ready to capture";
        if(hasSource() && rate>0)statusText+=" | "+duration(flow.locked()?learnProvisionalFrames(progress):learnMappedFrames(capture.manifest));
        if(automaticOn && generationView.displayed)statusText+=" | "+juce::String(static_cast<int>(generationView.displayed->positions.size()))+" events"
            +(generationView.updating?" (updating)":generationView.displayed->identity.provisional?" (preview)":"");
        if(progress.limited)statusText+=" | Preview limited";
        if(capture.failure && capture.failure.event.pass==flow.pass && (flow.locked() || phase==workflow::Phase::failed))
            statusText=juce::String(captureFailureReasonText(capture.failure.event.reason))+" | Stop and retry capture.";
        else if(phase==workflow::Phase::failed && capture.manifest.ranges.empty())statusText="Capture not accepted | No earlier committed source.";
        updateAccessibleDescription();
        refreshing=false;resized();repaint();
    }
    void visibilityChanged()override{mouseUpGesture();if(isVisible()){resetNavigation();clearInspection();}}
    void openDefaultIrFile(juce::Component* opener=nullptr)
    {
        if(responsive){selectedSection=2;updateSections();resized();}
        irBinding.displayDefault(engine.snapshot(),soundShowing());
        irBinding.openFile(opener);
    }
    bool soundShowing()const noexcept{return isVisible() && (!responsive || selectedSection==2);}
    void suspendSound(){irBinding.setActive(false);}
    void fitDrySource(){resetNavigation();view.update(timelineSource(),progress,clock);viewport=fullExtent();repaint();}
    std::pair<double,double> dryViewport()const noexcept{return {viewport.first,viewport.last};}
    const juce::String& coverageTotalsText()const noexcept{return totalsText;}
    LearnCoverageInfo coverageAt(int64_t sample)const{return learnCoverageAt(timelineSource(),progress,sample,sampleBound(view.first),sampleBound(view.last));}
    void mouseMove(const juce::MouseEvent& e)override
    {
        const auto next=chart.contains(e.position.toInt())?nearestEvent(pointerSample(e.position)):std::optional<int64_t>{};
        if(next!=hoverEvent){hoverEvent=next;repaint(chart);}
        if(inspectionPinned)return;
        if(chart.contains(e.position.toInt())){inspection=pointerSample(e.position);updateAccessibleDescription();repaint(timelineStatus);}else clearInspection();
    }
    void mouseExit(const juce::MouseEvent&)override{hoverEvent.reset();repaint(chart);if(!inspectionPinned)clearInspection();}
    void mouseDown(const juce::MouseEvent& e)override
    {
        mouseUpGesture();
        if(!chart.contains(e.getPosition()))return;
        pinDragging=controls::waveformButton(e.mods,juce::ModifierKeys::leftButtonModifier);
        panning=controls::waveformButton(e.mods,juce::ModifierKeys::middleButtonModifier);
        if(!pinDragging && !panning)return;
        focus->pointerInput();
        if(isShowing())grabKeyboardFocus();
        if(pinDragging)pinAt(pointerSample(e.position));
        else panStart=viewport;
    }
    void mouseDrag(const juce::MouseEvent& e)override
    {
        if(chart.isEmpty())return;
        if(pinDragging && controls::waveformButton(e.mods,juce::ModifierKeys::leftButtonModifier))pinAt(pointerSample(e.position));
        if(!panning || !controls::waveformButton(e.mods,juce::ModifierKeys::middleButtonModifier))return;
        viewport=panStart;viewport.shift(-static_cast<double>(e.getDistanceFromDragStartX())/chart.getWidth());
        viewport.constrain(fullExtent(),minimumSpan());viewNavigated();
    }
    void mouseUp(const juce::MouseEvent&)override{mouseUpGesture();}
    void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel)override
    {
        if(chart.contains(e.getPosition()) && controls::zoomWaveform(viewport,wheel.deltaY,
            (e.position.x-chart.getX())/chart.getWidth(),fullExtent(),minimumSpan()))viewNavigated();
    }
    void focusGained(FocusChangeType cause)override{if(cause==focusChangedByTabKey)focus->keyboardInput();repaint();}
    void focusLost(FocusChangeType)override{mouseUpGesture();repaint();}
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override{return controls::waveformAccessibility(*this,*focus);}
    bool hasPinnedInspection()const noexcept{return inspectionPinned;}
    std::optional<int64_t> inspectedSample()const noexcept{return inspection;}
    juce::String inspectionText()const{return inspection?inspectionReadout().text:coverageKey;}
    juce::String visibleInspectionText()const{return inspection?inspectionReadout().musical:juce::String{};}
    juce::String diagnosticDetails()const{return totalsText+"\n"+(inspection?inspectionReadout().details:coverageKey);}
    bool keyPressed(const juce::KeyPress& key)override
    {
        if(!hasKeyboardFocus(false))return false;
        focus->keyboardInput();
        if(key==juce::KeyPress::escapeKey){clearInspection();return true;}
        auto sample=inspection.value_or(sampleBound(viewport.first));
        if(key==juce::KeyPress::homeKey)sample=sampleBound(view.first);
        else if(key==juce::KeyPress::endKey)sample=sampleBound(view.last)-1;
        else if(key==juce::KeyPress::rightKey)sample=coverageAt(sample).end;
        else if(key==juce::KeyPress::leftKey){const auto start=coverageAt(sample).start;sample=start>INT64_MIN?start-1:start;}
        else return false;
        sample=std::clamp(sample,sampleBound(view.first),sampleBound(view.last)-1);
        if(sample<viewport.first || sample>=viewport.last)
        {
            const auto half=viewport.span()*.5;
            viewport={sample-half,sample+half};
            viewport.constrain(fullExtent(),minimumSpan());
        }
        pinAt(sample);return true;
    }
    juce::Rectangle<int> timelineBounds()const noexcept{return chart;}
    juce::Rectangle<int> timelineHeadingBounds()const noexcept{return timelineHeading;}
    juce::Rectangle<int> timelineCaptionBounds()const noexcept{return timelineCaption;}
    juce::Rectangle<int> timelineRulerBounds()const noexcept{return timelineRuler;}
    juce::Rectangle<int> timelineStatusBounds()const noexcept{return timelineStatus;}
    bool showsEmptyPrompt()const noexcept{return !hasSource() && phase!=workflow::Phase::recording && phase!=workflow::Phase::draining && phase!=workflow::Phase::validating;}
    std::optional<int64_t> songPlayhead()const noexcept
    {return playhead && clock.valid ? std::optional{clock.sample} : std::nullopt;}
    const juce::String& timelineStatusText()const noexcept{return statusText;}
    std::array<juce::Rectangle<int>,3> emptyStateRegions()const noexcept
    {
        const auto area=chart.reduced(spacing::section);
        constexpr auto total=learnGeometry::emptyIconHeight+learnGeometry::emptyIconGap+learnGeometry::emptyHeadingHeight+learnGeometry::emptyTextGap+learnGeometry::emptyTextHeight;
        auto group=area.withSizeKeepingCentre(area.getWidth(),total);
        const auto icon=group.removeFromTop(learnGeometry::emptyIconHeight);group.removeFromTop(learnGeometry::emptyIconGap);
        const auto heading=group.removeFromTop(learnGeometry::emptyHeadingHeight);group.removeFromTop(learnGeometry::emptyTextGap);
        return {icon,heading,group};
    }
    void setResponsivePresentation(ResponsiveTypography type)
    {
        if(!responsive)
        {
            responsive=true;captureTitle.setText("Learn Settings",juce::dontSendNotification);
            irEditor.alignLeadingControlEdges();
            addAndMakeVisible(sections);sections.setName("learnSections");sections.setTitle("Learn settings section");
            sections.onRequest=[this](int id){selectedSection=id;updateSections();resized();if(onRefresh)onRefresh();};
            addAndMakeVisible(captureGroup);captureGroup.setText("Source Capture",juce::dontSendNotification);applyTextRole(captureGroup,TextRole::sectionHeading);
            captureGroup.setName("learnSourceCaptureHeading");
            captureGroup.setBorderSize({});
        }
        sections.setTextHeight(type.body);updateSections();resized();
    }
    void resized()override
    {
        if(responsive){layoutResponsive();return;}
        auto a=getLocalBounds();left=a.removeFromLeft(workspaceGeometry::learnSettingsWidth);a.removeFromLeft(workspaceGeometry::columnGap);right=a;
        auto controls=left.withTrimmedTop(spacing::related);
        layoutGenerationAction(controls);
        placePolicyControls(usesLoopPolicies());
        const auto toggleGroup=[&](controls::Switch& toggle,controls::WrappedLabel& description)
        {
            toggle.setBounds(controls.removeFromTop(controlGeometry::switchHeight));
            const auto height=description.preferredHeight(controls.getWidth());
            if(height>0)controls.removeFromTop(spacing::labelHelp);
            description.setBounds(controls.removeFromTop(height));
        };
        if(usesLoopPolicies())
        {
            auto heading=controls.removeFromTop(controlGeometry::headingHeight).reduced(policyFocusMargin,0);
            captureTitle.setBounds(heading);
            controls.removeFromTop(spacing::related);
            layoutLoopPolicies(controls);
        }
        else
        {
        captureTitle.setBounds(controls.removeFromTop(controlGeometry::headingHeight));controls.removeFromTop(spacing::related);
        layoutLoopControls(controls,true);
        toggleGroup(recapture,recaptureHelp);
        for (auto* control : std::initializer_list<juce::Component*>{&recapture,&recaptureHelp,&automaticLabel,&automatic,&manualHelp})
            control->setVisible(true);
        controls.removeFromTop(spacing::related);divider=controls.getY();controls.removeFromTop(spacing::related);
        automaticLabel.setBounds(controls.removeFromTop(controlGeometry::headingHeight));controls.removeFromTop(spacing::related);
        toggleGroup(automatic,automaticHelp);
        if(automaticOn){controls.removeFromTop(spacing::related);toggleGroup(autoStop,autoStopHelp);controls.removeFromTop(spacing::group);
            irEditor.setBounds(controls.removeFromTop(irEditor.preferredHeight(controls.getWidth())));controls.removeFromTop(spacing::group);
            sensitivityLabel.setBounds(controls.removeFromTop(controlGeometry::labelHeight));controls.removeFromTop(spacing::tight);sensitivity.setBounds(controls.removeFromTop(controlGeometry::height));}
        }
        auto canvas=right.reduced(0,spacing::related);timelineHeading=canvas.removeFromTop(controlGeometry::headingHeight);canvas.removeFromTop(spacing::tight);
        timelineCaption=canvas.removeFromTop(learnTypography::secondaryLineHeight);canvas.removeFromTop(learnGeometry::metadataGap);
        timelineRuler=canvas.removeFromTop(learnTypography::secondaryLineHeight);canvas.removeFromTop(learnGeometry::rulerViewportGap);
        // A width-proportional audio band avoids a tall, mostly empty plot while
        // preserving the horizontal song mapping and comfortable control tokens.
        chart=canvas.removeFromTop(std::max(0,std::min(canvas.getHeight()-learnTypography::secondaryLineHeight-spacing::related,canvas.getWidth()/2)));
        canvas.removeFromTop(spacing::related);timelineStatus=canvas.removeFromTop(learnTypography::secondaryLineHeight);
    }
    void paint(juce::Graphics& g)override
    {
        g.setColour(separator);
        if(responsive){g.drawVerticalLine(left.getRight(),static_cast<float>(responsiveGeometry::dividerClearance),static_cast<float>(getHeight()-responsiveGeometry::dividerClearance));if(selectedSection==1 && !usesLoopPolicies())g.drawHorizontalLine(divider,0.f,static_cast<float>(left.getRight()-responsiveGeometry::dividerClearance));}
        else{g.drawVerticalLine(left.getRight()+workspaceGeometry::columnGap/2,static_cast<float>(spacing::related),static_cast<float>(getHeight()-spacing::related));if(!usesLoopPolicies())g.drawHorizontalLine(divider,0.f,static_cast<float>(left.getRight()));}
        applyTextRole(g,*this,TextRole::sectionHeading);g.drawText("Audio",timelineHeading,juce::Justification::left);
        const bool showAcceptedSummary=(loopSettings.enabled || phraseTimeline())
            && phase!=workflow::Phase::recording && phase!=workflow::Phase::draining && phase!=workflow::Phase::validating;
        applyTextRole(g,*this,TextRole::helper);
        g.drawText(showAcceptedSummary?totalsText:statusText,timelineCaption,juce::Justification::left);
        g.setColour(outline);g.drawRoundedRectangle(controlGeometry::insideStroke(chart,rule),radius,rule);
        const auto [first,last]=viewport;
        const auto x=[&](double sample){return chart.getX()+static_cast<float>((sample-first)/(last-first)*chart.getWidth());};
        applyTextRole(g,*this,TextRole::ruler);
        long double origin;const auto musical=songOrigin(clock,origin);
        const auto ticks=musical?makeSongBarRuler(first,last,chart.getWidth(),clock.rate,clock.bpm,clock.numerator,clock.denominator,clock.sample,clock.ppq):std::vector<SongBarTick>{};
        const auto readable=!ticks.empty() && std::all_of(ticks.begin(),ticks.end(),[](const auto& tick){return std::abs(tick.bar)<1000000;});
        if(readable)for(const auto& tick:ticks)
        {const auto width=std::min(static_cast<float>(controlGeometry::rulerLabelWidth),static_cast<float>(chart.getWidth())),at=std::clamp(x(tick.samplePosition)-width*.5f,static_cast<float>(chart.getX()),static_cast<float>(chart.getRight())-width);
            g.drawText(juce::String(tick.bar)+".1",juce::Rectangle<float>(at,static_cast<float>(timelineRuler.getY()),width,static_cast<float>(timelineRuler.getHeight())),juce::Justification::centred);}
        else if(phraseTimeline())g.drawText("Phrase-local audio",timelineRuler,juce::Justification::left);
        else if(!hostSongTimingBlocked && (!loopSettings.enabled || hasSource()))g.drawText("Time unavailable",timelineRuler,juce::Justification::left);
        const auto mid=static_cast<float>(chart.getCentreY()),height=chart.getHeight()*.4f;
        const auto drawSource=[&](const auto& regions,bool provisional)
        {
            juce::Graphics::ScopedSaveState scope(g);g.reduceClipRegion(chart.reduced(1));
            for(const auto& region:regions)
            {
                const auto a=x(static_cast<double>(region.range.startSample)),b=x(static_cast<double>(region.range.startSample)+region.range.frameCount);
                if(!region.available){
                    g.setColour(danger.withAlpha(.15f));
                    g.fillRect(juce::Rectangle<float>(a,static_cast<float>(chart.getY()),std::max(1.f,b-a),static_cast<float>(chart.getHeight())));
                    if(!region.available && b-a>90){g.setColour(danger);g.drawText("Audio unavailable",juce::Rectangle<float>(a+4,static_cast<float>(chart.getY()+4),b-a-8,20),juce::Justification::left);}
                }
                if(provisional){g.setColour(timing.withAlpha(.08f));g.fillRect(juce::Rectangle<float>(a,static_cast<float>(chart.getY()),std::max(1.f,b-a),static_cast<float>(chart.getHeight())));}
                g.setColour(region.available?provisional?text.withAlpha(.8f):text:danger);
                if(region.range.kind==CaptureCoverageKind::silence)g.drawHorizontalLine(static_cast<int>(mid),a,b);
                for(const auto& peak:region.peaks){const auto at=x(static_cast<double>(peak.startSample)),end=x(static_cast<double>(peak.startSample)+peak.frameCount);
                    if(end<chart.getX() || at>chart.getRight())continue;
                    const auto top=mid-std::clamp(peak.maximum,-1.0f,1.0f)*height,bottom=mid-std::clamp(peak.minimum,-1.0f,1.0f)*height;
                    g.fillRect(juce::Rectangle<float>(at,top,std::max(rule,end-at),std::max(rule,bottom-top)));}
                if(provisional){g.setColour(timing.withAlpha(.6f));g.drawHorizontalLine(chart.getBottom()-2,a,b);}
            }
        };
        // The dark background is genuine uncaptured space; never infer silence
        // from a small peak or extend a neighbouring region across a seek.
        drawSource(timelineSource().overview,false);
        if(progress.limited && progress.recordedLoopEnd()>0)
            drawSource(std::span<const CaptureOverviewRange>{&progress.loopWaveform,1},true);
        else drawSource(progress.ranges,true);
        if(automaticOn && generationView.displayed)
        {
            juce::Graphics::ScopedSaveState scope(g);g.reduceClipRegion(chart.reduced(juce::roundToInt(rule)));
            const auto pinned=inspectionPinned && inspection?nearestEvent(*inspection):std::optional<int64_t>{};
            const auto& markers=generationView.displayed->positions;
            auto marker=std::lower_bound(markers.begin(),markers.end(),sampleBound(first),[](const auto& candidate,int64_t sample){return candidate.sample<sample;});
            int previousColumn=INT_MIN;
            const float dash[]{static_cast<float>(spacing::tight),static_cast<float>(spacing::tight)};
            for(;marker!=markers.end() && marker->sample<=last;++marker)
            {
                const auto at=x(static_cast<double>(marker->sample));const auto column=juce::roundToInt(at);
                if(column==previousColumn)continue;previousColumn=column;
                g.setColour((hoverEvent && *hoverEvent==marker->sample) || (pinned && *pinned==marker->sample)?timing:separator);
                const auto top=static_cast<float>(chart.getY()+spacing::tight),head=static_cast<float>(spacing::labelHelp),half=static_cast<float>(spacing::tight);
                g.drawDashedLine({at,top+head,at,static_cast<float>(chart.getBottom()-spacing::tight)},dash,2,rule);
                juce::Path triangle;triangle.startNewSubPath(at,top+head);triangle.lineTo(at-half,top);triangle.lineTo(at+half,top);triangle.closeSubPath();
                if(generationView.updating || generationView.displayed->identity.provisional)g.strokePath(triangle,juce::PathStrokeType(rule));else g.fillPath(triangle);
            }
        }
        if(musical && first<origin && origin<last){g.setColour(secondary);g.drawVerticalLine(static_cast<int>(x(static_cast<double>(origin))),static_cast<float>(chart.getY()),static_cast<float>(chart.getBottom()));
            g.drawText("Song start",juce::Rectangle<float>(x(static_cast<double>(origin))+4,static_cast<float>(chart.getY()+4),80,20),juce::Justification::left);}
        if(showsEmptyPrompt())
        {
            const auto regions=emptyStateRegions();const auto iconCentre=regions[0].getCentre();
            g.setColour(learnTypography::descriptionColour);
            constexpr std::array<float,9> bars{8,17,29,13,38,22,32,16,7};
            for(size_t i=0;i<bars.size();++i){const auto h=bars[i];g.fillRoundedRectangle(static_cast<float>(iconCentre.x)-35+static_cast<float>(i)*8,static_cast<float>(iconCentre.y)-h*.5f,3,h,1.5f);}
            applyTextRole(g,*this,TextRole::emptyHeading);
            const bool canStart=loopArm.action().isEnabled() && !loopArm.action().getToggleState();
            g.drawFittedText(loopSettings.enabled
                ? (canStart?"Capture a loop":"No loop recorded yet")
                : (canStart && !hostSongTimingBlocked?"Arm Song capture":"No song audio yet"),regions[1],juce::Justification::centred,1);
            applyTextRole(g,*this,TextRole::emptyBody);
            const auto* armHelp = loopSettings.method == LoopCapturePreference::Method::endOnRepeat
                ? "Click Arm to record one full repeat.\nRecording starts at the beginning of the next pass."
                : loopSettings.method == LoopCapturePreference::Method::endSignal
                ? "Click Arm to start recording in sync.\nClick Arm again or send a Stop MIDI note to finish at the next bar line."
                : "Choose a length and click Arm.\nRecording starts on a bar line and stops after the chosen length.";
            g.drawFittedText(loopSettings.enabled
                ? (canStart ? armHelp : "")
                : (hostSongTimingBlocked?"":"Click Arm, then play audio in your DAW."),
                regions[2],juce::Justification::centredTop,2);
        }
        if(inspectionPinned && inspection){const auto at=x(static_cast<double>(*inspection));
            juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(chart.reduced(1));
            const auto region=coverageAt(*inspection);const auto a=std::clamp(x(static_cast<double>(region.start)),static_cast<float>(chart.getX()),static_cast<float>(chart.getRight()));
            const auto b=std::clamp(x(static_cast<double>(region.end)),static_cast<float>(chart.getX()),static_cast<float>(chart.getRight()));
            g.setColour(text.withAlpha(.04f));g.fillRect(juce::Rectangle<float>(a,static_cast<float>(chart.getY()+4),std::max(0.f,b-a),static_cast<float>(chart.getHeight()-8)));
            g.setColour(text);const float dash[]{3,3};g.drawDashedLine({at,static_cast<float>(chart.getY()+5),at,static_cast<float>(chart.getBottom()-5)},dash,2,1);
            g.fillEllipse(at-3,static_cast<float>(chart.getY()+5),6,6);
        }
        if(const auto sample=songPlayhead();sample && *sample>=first && *sample<=last)
        {g.setColour(timing);g.drawVerticalLine(static_cast<int>(x(static_cast<double>(*sample))),static_cast<float>(chart.getY()),static_cast<float>(chart.getBottom()));}
        applyTextRole(g,*this,TextRole::helper);g.drawText(visibleInspectionText(),timelineStatus,juce::Justification::left);
        controls::paintWaveformFocus(g,*this,chart.expanded(controlGeometry::focusPadding),*focus);
    }
private:
    const CaptureSessionSnapshot& timelineSource()const
    {
        static const CaptureSessionSnapshot empty;
        return progress.freshDomain?empty:capture;
    }
    bool phraseTimeline()const noexcept
    {
        return (progress.freshDomain?progress.kind:capture.sourceKind())==CaptureSourceKind::loop;
    }
    LearnInspectionReadout inspectionReadout()const
    {
        const auto& manifest=timelineSource().manifest;
        const auto rate=progress.sampleRate>0?progress.sampleRate:manifest.sampleRate;
        auto time=clock;
        if(const auto& phrase=manifest.loop)
            time={true,0,0,rate,phrase->tempoBpm,phrase->meterNumerator,phrase->meterDenominator};
        return learnInspectionReadout(*inspection,coverageAt(*inspection),time,rate);
    }
    void setupLoopControls()
    {
        for (auto* control : std::initializer_list<juce::Component*>{&loopMode,&loopMethod,&loopUnit,&loopCount,&loopArm})
            addChildComponent(control);
        loopMode.setName("learnLoopMode");
        loopMethod.setName("learnLoopMethod");loopMethod.setTitle("Capture method");
        loopUnit.setName("learnLoopUnit");loopUnit.setTitle("Duration unit");
        loopCount.setName("learnLoopCount");loopCount.setTitle("Duration count");
        loopArm.setName("learnCaptureArm");
        auto& arm=loopArm.action();
        arm.setName("learnLoopArm");
        arm.setClickingTogglesState(true);
        loopArm.setStateFieldName("learnLoopStatus");
        loopMethod.addItem("Duration",1);loopMethod.addItem("End on Signal",2);loopMethod.addItem("End on repeat",3);
        loopUnit.addItem("Bars",1);loopUnit.addItem("Beats",2);
        loopCount.setEditable(true,true,false);
        loopCount.setKeyboardType(juce::TextInputTarget::numericKeyboard);
        loopCount.setColour(juce::Label::textColourId,text);
        loopCount.setColour(juce::Label::outlineColourId,outline);
        const auto submit=[this](LoopCapturePreference next)
        {
            if (onLoopPreference) onLoopPreference(next);
            if (onRefresh) onRefresh();
        };
        loopMode.onClick=[this,submit]{auto next=loopSettings;next.enabled=loopMode.getToggleState();submit(next);};
        loopMethod.onChange=[this,submit]{
            auto next=loopSettings;
            const auto selected=loopMethod.getSelectedId();
            if (selected<1 || selected>3) return;
            next.method=static_cast<LoopCapturePreference::Method>(selected-1);
            submit(next);
        };
        loopUnit.onChange=[this,submit]{
            auto next=loopSettings;
            next.unit=loopUnit.getSelectedId()==1?LoopCapturePreference::Unit::bars:LoopCapturePreference::Unit::beats;
            submit(next);
        };
        loopCount.onTextChange=[this,submit]{
            auto next=loopSettings;
            const auto entry=loopCount.getText().trim();
            if (entry.isNotEmpty() && entry.containsOnly("0123456789")) next.count=static_cast<unsigned>(entry.getIntValue());
            submit(next.valid()?next:loopSettings);
        };
        loopCount.onStep=[this,submit](int change){
            auto next=loopSettings;
            next.count=static_cast<unsigned>(std::clamp(static_cast<int>(next.count)+change,1,static_cast<int>(LoopCapturePreference::maximumCount)));
            submit(next);
        };
        arm.beforeInvocation=[this]{loopArmIntent=CaptureArmIntent{flowNow().pass,engine.assignmentEpoch(),loopSettings};};
        arm.onClick=[this]{
            const auto flow=flowNow();
            const auto intent=std::exchange(loopArmIntent,{});
            if(onCaptureArm && intent && (loopArm.action().getToggleState() || flow.locked()))
                onCaptureArm(loopArm.action().getToggleState(),intent->pass,intent->session,intent->preference);
            if(onRefresh)onRefresh();
        };
    }
    void layoutLoopControls(juce::Rectangle<int>& column, bool visible)
    {
        const bool show=visible && static_cast<bool>(onLoopPreference);
        const bool loop=show && loopSettings.enabled;
        const bool duration=loop && loopSettings.method==LoopCapturePreference::Method::duration;
        loopMode.setVisible(show);
        loopMethod.setVisible(loop);
        loopArm.setVisible(show);
        loopUnit.setVisible(duration);loopCount.setVisible(duration);
        if (!show) return;
        loopArm.setBounds(column.removeFromTop(controlGeometry::height).expanded(policyFocusMargin,0));
        column.removeFromTop(spacing::related);
        auto row=column.removeFromTop(controlGeometry::height);
        loopMode.setBounds(responsive || usesLoopPolicies()?row.expanded(policyFocusMargin,0):row);
        if (!loop) return;
        column.removeFromTop(spacing::related);
        loopMethod.setRowBounds(column.removeFromTop(controlGeometry::height).expanded(policyFocusMargin,0));
        if (duration)
        {
            column.removeFromTop(spacing::related);
            auto values=column.removeFromTop(controlGeometry::height);
            loopCount.setBounds(values.removeFromLeft(controlGeometry::valueWidth)
                                    .withSizeKeepingCentre(controlGeometry::valueWidth,controlGeometry::valueHeight));
            values.removeFromLeft(spacing::related+policyFocusMargin);
            loopUnit.setRowBounds(values.expanded(policyFocusMargin,0));
        }
        column.removeFromTop(spacing::tight);
    }
    LoopCapturePreference loopSettings;
    bool loopRecaptureOn=false;
    controls::LatchingActionButton loopMode{"Loop Mode"};
    controls::ActionButtonStateField loopArm{"Arm",controls::CommandIcon::record,controls::CommandVariant::destructive,
        "Capture status",controlGeometry::valueWidth,workspaceGeometry::learnSettingsWidth-controlGeometry::valueWidth};
    controls::ChoiceSelector loopMethod,loopUnit;
    controls::FieldNumericInput loopCount{"Duration count","Number of bars or beats to record."};
    struct CaptureArmIntent { uint64_t pass,session;LoopCapturePreference preference; };
    std::optional<CaptureArmIntent> loopArmIntent;
    static constexpr int policyFocusMargin=controls::SelectorPresentation::focusMargin;
    juce::Component loopPolicies;
    controls::FocusViewport loopPoliciesViewport;
    bool usesLoopPolicies()const noexcept
    {return loopSettings.enabled || capture.sourceKind()==CaptureSourceKind::loop;}
    juce::Rectangle<int> policyContentBounds(juce::Rectangle<int> bounds)const
    {return bounds.withTrimmedRight(loopPoliciesViewport.getScrollBarThickness()).reduced(policyFocusMargin,0);}
    void placePolicyControls(bool inViewport)
    {
        auto* parent=inViewport?&loopPolicies:static_cast<juce::Component*>(this);
        for (auto* control : std::initializer_list<juce::Component*>{&captureGroup,&loopMode,&loopMethod,&loopUnit,&loopCount,
             &loopArm,&recapture,&recaptureHelp,&automaticLabel,&automatic,
             &autoStop,&sensitivityLabel,&sensitivity})
            if (control->getParentComponent()!=parent) parent->addChildComponent(control);
        loopPoliciesViewport.setVisible(inViewport && (!responsive || selectedSection==1));
    }
    void layoutLoopPolicies(juce::Rectangle<int> bounds)
    {
        loopPoliciesViewport.setBounds(bounds);
        const auto width=std::max(0,bounds.getWidth()-loopPoliciesViewport.getScrollBarThickness());
        auto column=policyContentBounds(bounds.withZeroOrigin());
        const auto row=[&](int height){auto rect=column.withHeight(height);column.setY(column.getY()+height);return rect;};
        const auto toggle=[&](controls::Switch& value){
            value.setBounds(row(controlGeometry::switchHeight));
        };
        captureGroup.setVisible(responsive);
        if(responsive)captureGroup.setBounds(row(responsiveGeometry::headingHeight));
        // Use an unbounded vertical budget so the viewport can reveal the whole policy.
        auto captureRows=column.withHeight(8*controlGeometry::switchHeight);
        layoutLoopControls(captureRows,true);column.setY(captureRows.getY());
        for(auto* control : std::initializer_list<juce::Component*>{&recapture,&recaptureHelp})
            control->setVisible(true);
        toggle(recapture);recaptureHelp.setBounds(row(recaptureHelp.preferredHeight(column.getWidth())));
        row(spacing::group);
        automaticLabel.setBounds(row(controlGeometry::headingHeight));
        toggle(automatic);
        for(auto* control : std::initializer_list<juce::Component*>{&automaticLabel,&automatic})
            control->setVisible(true);
        for(auto* control : std::initializer_list<juce::Component*>{&autoStop,&sensitivity,&sensitivityLabel})
            control->setVisible(automaticOn);
        if(automaticOn)
        {
            sensitivityLabel.setBounds(row(controlGeometry::labelHeight));
            sensitivity.setBounds(row(controlGeometry::height));
            toggle(autoStop);
        }
        for(auto* control : std::initializer_list<juce::Component*>{&automaticHelp,&autoStopHelp})
            control->setVisible(false);
        if(!responsive)irEditor.setVisible(false);
        loopPolicies.setSize(width,std::max(bounds.getHeight(),column.getY()+spacing::related));
    }
    bool responsive=false;int selectedSection=1;
    controls::SegmentedSelector sections;
    controls::EllipsisLabel captureGroup;
    void updateSections()
    {sections.display({{1,"Capture & Generate","Capture and generation policies"},{2,"Sound","Default sound for new swells"}},selectedSection);}
    void layoutGenerationAction(juce::Rectangle<int>& column)
    {
        generate.setVisible(automaticOn);
        column.removeFromBottom(spacing::group);
        auto helper=policyContentBounds(column.removeFromBottom(controlGeometry::helperLineHeight));
        manualHelp.setBounds(helper);
        if(automaticOn)
        {
            column.removeFromBottom(spacing::labelHelp);
            generate.setBounds(policyContentBounds(column.removeFromBottom(generatePreviewHeight)).expanded(policyFocusMargin,0));
        }
        column.removeFromBottom(spacing::related);
    }
    void layoutResponsive()
    {
        using namespace responsiveGeometry;
        auto area=getLocalBounds();left=area.removeFromLeft(284);area.removeFromLeft(16);right=area;
        auto column=left.withTrimmedRight(groupGap);
        captureTitle.setBounds(policyContentBounds(column.removeFromTop(headingHeight)));column.removeFromTop(spacing::tight);
        sections.setBounds(policyContentBounds(column.removeFromTop(tabsHeight)).expanded(policyFocusMargin,0));
        column.removeFromTop(contentPadding);
        const auto captureVisible=selectedSection==1,soundVisible=selectedSection==2;
        layoutGenerationAction(column);
        const auto width=policyContentBounds(column).getWidth();
        const auto songSettingsHeight=2*headingHeight+4*controlGeometry::switchHeight
            +2*controlHeight+24+3*contentPadding+2*spacing::related
            +recaptureHelp.preferredHeight(width);
        const bool scrollPolicies=usesLoopPolicies() || column.getHeight()<songSettingsHeight;
        placePolicyControls(scrollPolicies);
        if(scrollPolicies)layoutLoopPolicies(column);
        else
        {
        column=policyContentBounds(column);
        captureGroup.setVisible(captureVisible);captureGroup.setBounds(column.removeFromTop(headingHeight));
        const auto toggleRow=[&](controls::Switch& value)
        {value.setBounds(column.removeFromTop(controlGeometry::switchHeight));};
        layoutLoopControls(column,captureVisible);
        if(!loopSettings.enabled){toggleRow(recapture);recaptureHelp.setBounds(column.removeFromTop(recaptureHelp.preferredHeight(column.getWidth())));}
        column.removeFromTop(contentPadding);divider=column.getY();column.removeFromTop(contentPadding);
        automaticLabel.setBounds(column.removeFromTop(headingHeight));toggleRow(automatic);toggleRow(autoStop);
        column.removeFromTop(contentPadding);sensitivityLabel.setBounds(column.removeFromTop(24));sensitivity.setBounds(column.removeFromTop(controlHeight));
        for(auto* c:std::initializer_list<juce::Component*>{&recapture,&recaptureHelp,&automaticLabel,&automatic})c->setVisible(captureVisible && !loopSettings.enabled);
        for(auto* c:std::initializer_list<juce::Component*>{&autoStop,&sensitivity,&sensitivityLabel})c->setVisible(captureVisible && automaticOn);
        automaticHelp.setVisible(false);autoStopHelp.setVisible(false);
        }
        auto sound=policyContentBounds(left.withTrimmedRight(groupGap)).withTrimmedTop(sections.getBottom()+contentPadding);
        const auto actionsTop=generate.isVisible()?generate.getY():manualHelp.getY();
        sound.setHeight(std::max(0,std::min(sound.getBottom(),actionsTop-contentPadding)-sound.getY()));
        irEditor.setBounds(sound.expanded(policyFocusMargin,0));irEditor.setVisible(soundVisible);
        auto canvas=right;timelineHeading=canvas.removeFromTop(headingHeight);canvas.removeFromTop(spacing::tight);timelineCaption=canvas.removeFromTop(20);canvas.removeFromTop(contentPadding);
        timelineRuler=canvas.removeFromTop(20);canvas.removeFromTop(spacing::tight);chart=canvas.removeFromTop(std::max(0,std::min(canvas.getHeight()-32,canvas.getWidth()/2)));
        canvas.removeFromTop(contentPadding);timelineStatus=canvas.removeFromTop(20);
    }
    std::optional<int64_t> nearestEvent(int64_t sample)const
    {
        if(!automaticOn || !generationView.displayed || chart.getWidth()<=0)return {};
        const auto& events=generationView.displayed->positions;if(events.empty())return {};
        auto it=std::lower_bound(events.begin(),events.end(),sample,[](const auto& event,int64_t at){return event.sample<at;});
        if(it==events.end())--it;
        else if(it!=events.begin() && std::abs(static_cast<long double>((it-1)->sample)-sample)<std::abs(static_cast<long double>(it->sample)-sample))--it;
        const auto pixels=std::abs((static_cast<long double>(it->sample)-sample)*chart.getWidth()/viewport.span());
        return pixels<=spacing::tight*2?std::optional<int64_t>{it->sample}:std::nullopt;
    }
    static int64_t sampleBound(double value)noexcept
    {return static_cast<int64_t>(std::clamp(static_cast<long double>(value),static_cast<long double>(INT64_MIN),static_cast<long double>(INT64_MAX)));}
    int64_t pointerSample(juce::Point<float> point)const noexcept
    {
        const auto x=std::clamp(point.x,static_cast<float>(chart.getX()),static_cast<float>(chart.getRight()));
        return std::clamp(viewport.sample(x,chart.getX(),chart.getWidth()),sampleBound(view.first),sampleBound(view.last)-1);
    }
    SwellViewport fullExtent()const noexcept{return {view.first,view.last};}
    double minimumSpan()const noexcept{return std::max(1,chart.getWidth());}
    void mouseUpGesture(){pinDragging=panning=false;}
    void resetNavigation(){view.reset();manualView=false;mouseUpGesture();}
    void pinAt(int64_t sample){inspection=sample;inspectionPinned=true;updateAccessibleDescription();repaint();}
    void viewNavigated()
    {
        manualView=viewport.span()<fullExtent().span();
        hoverEvent.reset();
        if(!inspectionPinned)inspection.reset();
        updateAccessibleDescription();repaint();
    }
    void updateAccessibleDescription(){
        const juce::String markerHelp="Event guides brighten on hover or pin. Solid yellow: DAW position. White pin: inspection.";
        setTooltip(inspectionHelp+"\n"+markerHelp+"\n"+visibleInspectionText());setDescription(inspectionHelp+" "+statusText+" "+markerHelp+" "+visibleInspectionText());}
    void clearInspection(){const auto changed=inspection.has_value() || inspectionPinned || hoverEvent.has_value();inspection.reset();inspectionPinned=false;hoverEvent.reset();
        if(changed){updateAccessibleDescription();repaint();}}
    inline static const juce::String inspectionHelp="Left-click or drag to pin inspection. Wheel zooms at the pointer. Middle-drag pans. Home/End and left/right inspect regions. Escape clears with waveform focus.";
    inline static const juce::String coverageKey="Audio | Silence | Gap: uncaptured | Unavailable | Recording";
    LearnDryView view;uint64_t inspectionSession=0,inspectionWorkflow=0;bool inspectionPinned=false;std::optional<int64_t> inspection,hoverEvent;juce::String totalsText,viewScope;
    struct GenerationIntent{uint64_t session,pass,serial;};
    SwellEngine& engine;std::function<workflow::Snapshot()> flowNow;
    controls::Switch recapture{"Recapture audio",controls::ToggleLabelSide::beforeIndicator};
    controls::Switch automatic{"Detect events",controls::ToggleLabelSide::beforeIndicator};
    // Candidate geometry for the #178 native review.
    static constexpr int generatePreviewHeight=48+2*policyFocusMargin;
    static constexpr auto manualPlacementHelp="Place swells manually in Perform";
    controls::Switch autoStop{"Generate on Stop",controls::ToggleLabelSide::beforeIndicator};
    controls::ActionButton generate{"Generate Swells",controls::CommandIcon::generateSwells,true};
    controls::SoundPresetEditor irEditor;SoundPresetBinding irBinding;
    controls::SliderControl sensitivity{{"Event sensitivity (%)", "Adjust how many events are detected in captured audio."}};
    controls::EllipsisLabel captureTitle,automaticLabel,sensitivityLabel;
    controls::WrappedLabel recaptureHelp,automaticHelp;
    controls::WrappedLabel autoStopHelp;
    controls::EllipsisLabel manualHelp;
    LearnGenerationView generationView;uint64_t displaySession=0,displayPass=0;
    std::optional<GenerationIntent> autoStopIntent,generateIntent;
    std::optional<uint64_t> recapturePass,automaticPass,sensitivityPass;
    bool hasSource()const noexcept
    {
        const auto& source=timelineSource();
        return !source.manifest.ranges.empty() || !source.overview.empty()
            || !progress.ranges.empty() || progress.recordedLoopEnd()>0;
    }
    juce::String statusText;CaptureSessionSnapshot capture;CaptureProgress progress;SongTime clock;workflow::Phase phase=workflow::Phase::idle;
    juce::Rectangle<int> left,right,chart,timelineHeading,timelineCaption,timelineRuler,timelineStatus;int divider=0;bool automaticOn=true,refreshing=false,playhead=false;
    juce::SharedResourcePointer<controls::FocusVisibility> focus;
    SwellViewport viewport{0,1},panStart;
    bool manualView=false,panning=false,pinDragging=false;
    bool hostSongTimingBlocked=false;
};
}
