#pragma once
#include "WorkspaceTheme.h"
#include "SongBarRuler.h"
#include "LearnTimelineModel.h"
#include "Controls/Switch.h"
#include "Controls/SliderControl.h"
#include "Controls/IconActionButton.h"
#include "Controls/IrSelector.h"
#include "Controls/ActionButton.h"
#include "Controls/SegmentedSelector.h"
#include <DSP/SwellEngine.h>
#include <Capture/CaptureProgress.h>

namespace m3::ui
{
// Learn owns a separate dry-only canvas. It has no access to Perform navigation
// or wet editing. Provisional display telemetry is separate from accepted source.
class LearnWorkspace final:public juce::Component,public juce::SettableTooltipClient
{
public:
    LearnWorkspace(SwellEngine& source,std::function<workflow::Snapshot()> readFlow):engine(source),flowNow(std::move(readFlow))
    {
        setWantsKeyboardFocus(true);setMouseClickGrabsKeyboardFocus(true);
        setTooltip(inspectionHelp);setName("learnWorkspace");setTitle("Learn: source capture and generation preferences");
        for(auto* c:std::array<juce::Component*,20>{&captureTitle,&recaptureLabel,&recaptureHelp,&recapture,&automaticLabel,&automaticToggleLabel,&automaticHelp,&automatic,
            &autoStopLabel,&autoStopHelp,&autoStop,&irLabel,&irBox,&fileButton,&irStatus,&sensitivityLabel,&sensitivity,&generate,&generateReason,&manualHelp})addAndMakeVisible(c);
        captureTitle.setText("Source Capture",juce::dontSendNotification);applyTextRole(captureTitle,TextRole::sectionHeading);
        recaptureLabel.setText("Recapture audio",juce::dontSendNotification);
        automaticLabel.setText("Automatic Swells",juce::dontSendNotification);applyTextRole(automaticLabel,TextRole::sectionHeading);
        automaticToggleLabel.setText("Detect events",juce::dontSendNotification);
        irLabel.setText("Default IR",juce::dontSendNotification);sensitivityLabel.setText("Event sensitivity",juce::dontSendNotification);
        autoStopLabel.setText("Generate on Stop",juce::dontSendNotification);
        for(auto* label:{&recaptureLabel,&automaticToggleLabel,&irLabel,&sensitivityLabel,&autoStopLabel})applyTextRole(*label,TextRole::controlLabel);
        for(auto* label:{&recaptureHelp,&automaticHelp,&irStatus,&autoStopHelp,&manualHelp,&generateReason})applyTextRole(*label,TextRole::helper,juce::Justification::topLeft);
        irStatus.setMinimumHorizontalScale(1.f);
        irStatus.setText("Used by Generate.",juce::dontSendNotification);
        irStatus.setTooltip(irStatus.getText());
        for(auto* label:{&captureTitle,&recaptureLabel,&automaticLabel,&automaticToggleLabel,&irLabel,&sensitivityLabel,&recaptureHelp,&automaticHelp,&irStatus,&autoStopLabel,&autoStopHelp,&manualHelp,&generateReason})label->setBorderSize({});
        automaticHelp.setText({},juce::dontSendNotification);
        autoStopHelp.setText({},juce::dontSendNotification);
        manualHelp.setText(wrappedHelper("Want to place swells yourself? Add manual swells in Perform."),juce::dontSendNotification);
        manualHelp.setTooltip(manualHelp.getText().replace("\n"," "));
        captureTitle.setName("learnCaptureHeading");automaticLabel.setName("learnAutomaticHeading");recaptureLabel.setName("learnRecaptureLabel");automaticToggleLabel.setName("learnAutomaticLabel");
        irLabel.setName("learnIRLabel");sensitivityLabel.setName("learnSensitivityLabel");recaptureHelp.setName("learnRecaptureHelp");automaticHelp.setName("learnAutomaticHelp");
        recapture.setName("recaptureExisting");automatic.setName("learnAutomaticSwells");irBox.setName("learnDefaultIR");fileButton.setName("learnImportIR");irStatus.setName("learnIRStatus");sensitivity.setName("learnMarkerSensitivity");
        autoStop.setName("learnAutoGenerateOnStop");autoStopLabel.setName("learnAutoGenerateLabel");autoStopHelp.setName("learnAutoGenerateHelp");
        generate.setName("learnGenerateSwells");manualHelp.setName("learnManualSwellsHelp");generateReason.setName("learnGenerateReason");
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
        sensitivity.setTextValueSuffix(" %");sensitivity.setNumDecimalPlacesToDisplay(1);
        sensitivity.setTooltip("Low: fewer, stronger attacks. High: finer event detail. Updates the marker preview asynchronously.");
        sensitivity.beforeInvocation=[this]{sensitivityPass=flowNow().pass;};
        sensitivity.onValueChange=[this]{if(!refreshing){const auto s=engine.savedSession();engine.setPreferences(s.auditionDb,static_cast<float>(sensitivity.getValue()/100),s.separationSeconds,s.autoRegenerate,sensitivityPass.value_or(flowNow().pass));}};
        irBox.beforeInvocation=[this]{irIntent=Intent{engine.assignmentEpoch(),flowNow().pass,choices};};
        irBox.onChange=[this]{if(refreshing)return;const auto intent=irIntent.value_or(Intent{engine.assignmentEpoch(),flowNow().pass,choices});irIntent.reset();
            const auto at=irBox.getSelectedId()-1;if(at<0 || static_cast<size_t>(at)>=intent.choices.size())return;const auto& c=intent.choices[static_cast<size_t>(at)];
            if(c.presetId.isNotEmpty())engine.assignDefaultBundledIR(c.presetId,intent.session,intent.pass);else engine.assignDefaultManagedIR(c.managed,c.name,intent.session,intent.pass);if(onRefresh)onRefresh();};
        fileButton.onClick=[this]{const auto session=engine.assignmentEpoch(),pass=flowNow().pass;
            chooser=std::make_unique<juce::FileChooser>("Import default impulse response",juce::File{},"*.wav;*.wv");
            chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
                [safe=juce::Component::SafePointer<LearnWorkspace>(this),session,pass](const auto& dialog){if(safe && dialog.getResult()!=juce::File{})safe->engine.assignDefaultIR(dialog.getResult(),session,pass);});};
    }
    std::function<void(bool)> onRecapture;
    std::function<void()> onRefresh;
    void display(const CaptureSessionSnapshot& source,const EditorSnapshot& editor,const SongTime& time,workflow::Snapshot flow,bool position,const CaptureProgress& pending={})
    {
        refreshing=true;capture=source;clock=time;playhead=position;automaticOn=editor.session.automaticProcessing && editor.session.autoMarkersEnabled;
        generationView=editor.learnGeneration;displaySession=editor.sessionEpoch;displayPass=flow.pass;
        autoStop.setToggleState(editor.session.autoGenerateOnStop,juce::dontSendNotification);
        autoStop.setEnabled(!flow.locked() || editor.session.autoGenerateOnStop);
        generate.setEnabled(generationView.canGenerate);
        generateReason.setText(wrappedHelper(generationView.canGenerate?"Ready to generate":generationView.reason),juce::dontSendNotification);
        generate.setDescription(generationView.reason);generate.setHelpText(generationView.reason);
        generate.setTooltip("Create or replace automatically detected swells using the markers shown. Manual and frozen swells are preserved.\n"+generationView.reason);
        generateReason.setTooltip(generationView.reason);
        phase=flow.phase;progress=flow.locked() && pending.pass==flow.pass?pending:CaptureProgress{};
        recapture.setToggleState(flow.recapture,juce::dontSendNotification);automatic.setToggleState(automaticOn,juce::dontSendNotification);
        recapture.setEnabled(!flow.locked());automatic.setEnabled(!flow.locked());irBox.setEnabled(!flow.locked());fileButton.setEnabled(!flow.locked());sensitivity.setEnabled(!flow.locked());
        if(!sensitivity.isMouseButtonDown() && !sensitivity.hasKeyboardFocus(true))sensitivity.setValue(editor.session.sensitivity*100,juce::dontSendNotification);
        recaptureHelp.setText(flow.recapture?"Replace audio next pass.":"Keep captured audio.",juce::dontSendNotification);
        const auto list=engine.irChoices();juce::String key;for(const auto& c:list)key<<c.id<<"|"<<c.name<<"\n";
        if(key!=catalogue){irBox.hidePopup();catalogue=key;choices=list;irBox.clear(juce::dontSendNotification);int index=0;for(const auto& c:choices)irBox.addItem(c.name,++index);}
        int selected=0;for(size_t i=0;i<choices.size();++i)if(choices[i].content==editor.session.defaultIR.content){selected=static_cast<int>(i+1);break;}
        irBox.setSelectedId(selected,juce::dontSendNotification);if(!selected)irBox.setTextWhenNothingSelected("Default IR not ready");
        for(auto* c:std::array<juce::Component*,11>{&irLabel,&irBox,&fileButton,&irStatus,&sensitivityLabel,&sensitivity,&autoStop,&autoStopLabel,&autoStopHelp,&generate,&generateReason})c->setVisible(automaticOn);
        const auto scope=juce::String(source.manifest.projectId)+"/"+source.manifest.instanceId;
        const auto session=engine.assignmentEpoch();
        // A mode roundtrip/reset may complete between paints. A recording pass
        // increment alone must retain the pin throughout recording/settlement.
        const auto modeReset=flow.phase==workflow::Phase::idle && flow.result==workflow::Result::none && flow.pass!=inspectionWorkflow;
        inspectionWorkflow=flow.pass;
        if(scope!=viewScope || session!=inspectionSession || modeReset){
            if(scope!=viewScope)view.reset();clearInspection();viewScope=scope;inspectionSession=session;}
        view.update(capture,progress,clock);first=view.first;last=view.last;
        const auto rate=clock.rate>0?clock.rate:capture.manifest.sampleRate;
        const auto duration=[&](long double frames){return rate>0?juce::String(static_cast<double>(frames/rate),1)+" s":juce::String(static_cast<double>(frames),0)+" samples";};
        totalsText="Mapped "+duration(learnMappedFrames(capture.manifest))+" | Recording "+duration(learnProvisionalFrames(progress))+" | Extent "+duration(learnExtentFrames(capture,progress,clock));
        statusText=phase==workflow::Phase::recording?"Recording":phase==workflow::Phase::draining || phase==workflow::Phase::validating?"Finishing capture":phase==workflow::Phase::failed?"Capture failed; previous audio kept":flow.result==workflow::Result::cancelled?"Capture cancelled":source.unavailable?"Some audio unavailable":hasSource()?"Captured":"Ready to capture";
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
    void visibilityChanged()override{if(isVisible()){view.reset();clearInspection();}}
    void fitDrySource(){view.reset();view.update(capture,progress,clock);first=view.first;last=view.last;repaint();}
    std::pair<double,double> dryViewport()const noexcept{return {first,last};}
    const juce::String& coverageTotalsText()const noexcept{return totalsText;}
    LearnCoverageInfo coverageAt(int64_t sample)const{return learnCoverageAt(capture,progress,sample,sampleBound(first),sampleBound(last));}
    void mouseMove(const juce::MouseEvent& e)override
    {
        const auto next=chart.contains(e.position.toInt())?nearestEvent(pointerSample(e.position)):std::optional<int64_t>{};
        if(next!=hoverEvent){hoverEvent=next;repaint(chart);}
        if(inspectionPinned)return;
        if(chart.contains(e.position.toInt())){inspection=pointerSample(e.position);updateAccessibleDescription();repaint(timelineStatus);}else clearInspection();
    }
    void mouseExit(const juce::MouseEvent&)override{hoverEvent.reset();repaint(chart);if(!inspectionPinned)clearInspection();}
    void mouseDown(const juce::MouseEvent& e)override
    {if(e.mods.isLeftButtonDown() && chart.contains(e.position.toInt())){inspection=pointerSample(e.position);inspectionPinned=true;grabKeyboardFocus();updateAccessibleDescription();repaint();}}
    void focusGained(juce::Component::FocusChangeType)override{repaint(chart);}
    void focusLost(juce::Component::FocusChangeType)override{repaint(chart);}
    bool hasPinnedInspection()const noexcept{return inspectionPinned;}
    std::optional<int64_t> inspectedSample()const noexcept{return inspection;}
    juce::String inspectionText()const{return inspection?learnInspectionReadout(*inspection,coverageAt(*inspection),clock,capture.manifest.sampleRate).text:coverageKey;}
    juce::String visibleInspectionText()const{return inspection?learnInspectionReadout(*inspection,coverageAt(*inspection),clock,capture.manifest.sampleRate).musical:juce::String{};}
    juce::String diagnosticDetails()const{return totalsText+"\n"+(inspection?learnInspectionReadout(*inspection,coverageAt(*inspection),clock,capture.manifest.sampleRate).details:coverageKey);}
    bool keyPressed(const juce::KeyPress& key)override
    {
        if(!hasKeyboardFocus(false))return false;
        if(key==juce::KeyPress::escapeKey){clearInspection();return true;}
        auto sample=inspection.value_or(sampleBound(first));
        if(key==juce::KeyPress::homeKey)sample=sampleBound(first);
        else if(key==juce::KeyPress::endKey)sample=sampleBound(last)-1;
        else if(key==juce::KeyPress::rightKey)sample=coverageAt(sample).end;
        else if(key==juce::KeyPress::leftKey){const auto start=coverageAt(sample).start;sample=start>INT64_MIN?start-1:start;}
        else return false;
        inspection=std::clamp(sample,sampleBound(first),sampleBound(last)-1);inspectionPinned=true;updateAccessibleDescription();repaint();return true;
    }
    juce::Rectangle<int> timelineBounds()const noexcept{return chart;}
    juce::Rectangle<int> timelineHeadingBounds()const noexcept{return timelineHeading;}
    juce::Rectangle<int> timelineCaptionBounds()const noexcept{return timelineCaption;}
    juce::Rectangle<int> timelineRulerBounds()const noexcept{return timelineRuler;}
    juce::Rectangle<int> timelineStatusBounds()const noexcept{return timelineStatus;}
    bool showsEmptyPrompt()const noexcept{return !hasSource() && phase!=workflow::Phase::recording && phase!=workflow::Phase::draining && phase!=workflow::Phase::validating;}
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
            addAndMakeVisible(sections);sections.setName("learnSections");sections.setTitle("Learn settings section");
            sections.onRequest=[this](int id){selectedSection=id;updateSections();resized();};
            addAndMakeVisible(captureGroup);captureGroup.setText("Source Capture",juce::dontSendNotification);applyTextRole(captureGroup,TextRole::sectionHeading);
            addAndMakeVisible(soundScope);soundScope.setText("Defaults for new swells. Existing swells stay unchanged.",juce::dontSendNotification);applyTextRole(soundScope,TextRole::helper);
            captureGroup.setName("learnSourceCaptureHeading");soundScope.setName("learnSoundScope");
            captureGroup.setBorderSize({});soundScope.setBorderSize({});
        }
        sections.setTextHeight(type.body);updateSections();resized();
    }
    void resized()override
    {
        if(responsive){layoutResponsive();return;}
        auto a=getLocalBounds();left=a.removeFromLeft(workspaceGeometry::learnSettingsWidth);a.removeFromLeft(workspaceGeometry::columnGap);right=a;
        auto controls=left.withTrimmedTop(spacing::related);
        const auto toggleGroup=[&](controls::Switch& toggle,juce::Label& label,juce::Label& description)
        {
            auto row=controls.removeFromTop(controlGeometry::switchHeight);toggle.setBounds(row.removeFromRight(controlGeometry::switchWidth));row.removeFromRight(spacing::related);
            label.setBounds(row.withSizeKeepingCentre(row.getWidth(),juce::roundToInt(bodySize)));
            const auto lines=description.getText().isEmpty()?0:description.getText().containsChar('\n')?2:1;
            description.setBounds(row.getX(),label.getBottom()+spacing::labelHelp,row.getWidth(),lines*learnTypography::secondaryLineHeight);
            controls.removeFromTop(std::max(0,description.getBottom()-controls.getY()));
        };
        captureTitle.setBounds(controls.removeFromTop(controlGeometry::headingHeight));controls.removeFromTop(spacing::related);
        toggleGroup(recapture,recaptureLabel,recaptureHelp);
        controls.removeFromTop(spacing::related);divider=controls.getY();controls.removeFromTop(spacing::related);
        automaticLabel.setBounds(controls.removeFromTop(controlGeometry::headingHeight));controls.removeFromTop(spacing::related);
        toggleGroup(automatic,automaticToggleLabel,automaticHelp);
        if(automaticOn){controls.removeFromTop(spacing::related);toggleGroup(autoStop,autoStopLabel,autoStopHelp);controls.removeFromTop(spacing::group);
            irLabel.setBounds(controls.removeFromTop(controlGeometry::labelHeight));controls.removeFromTop(spacing::tight);
            auto row=controls.removeFromTop(controlGeometry::height);fileButton.setBounds(row.removeFromRight(controlGeometry::height));row.removeFromRight(spacing::tight);irBox.setBounds(row);
            controls.removeFromTop(spacing::labelHelp);irStatus.setBounds(controls.removeFromTop(controlGeometry::feedbackHeight));controls.removeFromTop(spacing::group);
            sensitivityLabel.setBounds(controls.removeFromTop(controlGeometry::labelHeight));controls.removeFromTop(spacing::tight);sensitivity.setBounds(controls.removeFromTop(controlGeometry::height));
            controls.removeFromTop(spacing::related);generate.setBounds(controls.removeFromTop(controlGeometry::height));
            controls.removeFromTop(spacing::labelHelp);generateReason.setBounds(controls.removeFromTop(2*learnTypography::secondaryLineHeight));}
        controls.removeFromTop(automaticOn?spacing::labelHelp:spacing::group);
        manualHelp.setBounds(controls.removeFromTop(2*learnTypography::secondaryLineHeight));
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
        if(responsive){g.drawVerticalLine(left.getRight(),static_cast<float>(responsiveGeometry::dividerClearance),static_cast<float>(getHeight()-responsiveGeometry::dividerClearance));if(selectedSection==1)g.drawHorizontalLine(divider,0.f,static_cast<float>(left.getRight()-responsiveGeometry::dividerClearance));}
        else{g.drawVerticalLine(left.getRight()+workspaceGeometry::columnGap/2,static_cast<float>(spacing::related),static_cast<float>(getHeight()-spacing::related));g.drawHorizontalLine(divider,0.f,static_cast<float>(left.getRight()));}
        applyTextRole(g,*this,TextRole::sectionHeading);g.drawText("Audio",timelineHeading,juce::Justification::left);
        applyTextRole(g,*this,TextRole::helper);g.drawText(statusText,timelineCaption,juce::Justification::left);
        g.setColour(outline);g.drawRoundedRectangle(controlGeometry::insideStroke(chart,rule),radius,rule);
        const auto x=[&](double sample){return chart.getX()+static_cast<float>((sample-first)/(last-first)*chart.getWidth());};
        applyTextRole(g,*this,TextRole::ruler);
        long double origin;const auto musical=songOrigin(clock,origin);
        const auto ticks=musical?makeSongBarRuler(first,last,chart.getWidth(),clock.rate,clock.bpm,clock.numerator,clock.denominator,clock.sample,clock.ppq):std::vector<SongBarTick>{};
        const auto readable=!ticks.empty() && std::all_of(ticks.begin(),ticks.end(),[](const auto& tick){return std::abs(tick.bar)<1000000;});
        if(readable)for(const auto& tick:ticks)
        {const auto width=std::min(static_cast<float>(controlGeometry::rulerLabelWidth),static_cast<float>(chart.getWidth())),at=std::clamp(x(tick.samplePosition)-width*.5f,static_cast<float>(chart.getX()),static_cast<float>(chart.getRight())-width);
            g.drawText(juce::String(tick.bar)+".1",juce::Rectangle<float>(at,static_cast<float>(timelineRuler.getY()),width,static_cast<float>(timelineRuler.getHeight())),juce::Justification::centred);}
        else g.drawText("Time unavailable",timelineRuler,juce::Justification::left);
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
        drawSource(capture.overview,false);drawSource(progress.ranges,true);
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
            applyTextRole(g,*this,TextRole::emptyHeading);g.drawFittedText("Play audio in your DAW",regions[1],juce::Justification::centred,1);
            applyTextRole(g,*this,TextRole::emptyBody);g.drawFittedText(automaticOn?"Review events before generating swells.":"Learn audio, then shape it in Perform.",regions[2],juce::Justification::centredTop,2);
        }
        if(hasKeyboardFocus(false)){
            g.setColour(aqua.withAlpha(.7f));g.drawRoundedRectangle(controlGeometry::insideStroke(chart.reduced(3),rule),radius,rule);}
        if(inspectionPinned && inspection){const auto at=x(static_cast<double>(*inspection));
            const auto region=coverageAt(*inspection);const auto a=std::clamp(x(static_cast<double>(region.start)),static_cast<float>(chart.getX()),static_cast<float>(chart.getRight()));
            const auto b=std::clamp(x(static_cast<double>(region.end)),static_cast<float>(chart.getX()),static_cast<float>(chart.getRight()));
            g.setColour(text.withAlpha(.04f));g.fillRect(juce::Rectangle<float>(a,static_cast<float>(chart.getY()+4),std::max(0.f,b-a),static_cast<float>(chart.getHeight()-8)));
            g.setColour(text);const float dash[]{3,3};g.drawDashedLine({at,static_cast<float>(chart.getY()+5),at,static_cast<float>(chart.getBottom()-5)},dash,2,1);
            g.fillEllipse(at-3,static_cast<float>(chart.getY()+5),6,6);
        }
        if(playhead && clock.sample>=first && clock.sample<=last){g.setColour(timing);g.drawVerticalLine(static_cast<int>(x(static_cast<double>(clock.sample))),static_cast<float>(chart.getY()),static_cast<float>(chart.getBottom()));}
        applyTextRole(g,*this,TextRole::helper);g.drawText(visibleInspectionText(),timelineStatus,juce::Justification::left);
    }
private:
    bool responsive=false;int selectedSection=1;
    controls::SegmentedSelector sections;
    juce::Label captureGroup,soundScope;
    void updateSections()
    {sections.display({{1,"Capture & Generate","Capture and generation policies"},{2,"Sound","Default sound for new swells"}},selectedSection);}
    void layoutResponsive()
    {
        using namespace responsiveGeometry;
        auto area=getLocalBounds();left=area.removeFromLeft(284);area.removeFromLeft(16);right=area;
        auto column=left.withTrimmedRight(groupGap);captureTitle.setBounds(column.removeFromTop(headingHeight));column.removeFromTop(spacing::tight);
        sections.setBounds(column.removeFromTop(tabsHeight));column.removeFromTop(contentPadding);
        auto actions=column.removeFromBottom(132);actions.removeFromTop(contentPadding);
        generate.setBounds(actions.removeFromTop(controlHeight));actions.removeFromTop(spacing::tight);
        generateReason.setBounds(actions.removeFromTop(40));actions.removeFromTop(contentPadding);manualHelp.setBounds(actions.removeFromTop(40));
        const auto captureVisible=selectedSection==1,soundVisible=selectedSection==2;
        captureGroup.setVisible(captureVisible);captureGroup.setBounds(column.removeFromTop(headingHeight));
        const auto toggleRow=[&](controls::Switch& value,juce::Label& label)
        {auto row=column.removeFromTop(36);value.setBounds(row.removeFromRight(controlGeometry::switchWidth));row.removeFromRight(contentPadding);label.setBounds(row);};
        toggleRow(recapture,recaptureLabel);recaptureHelp.setBounds(column.removeFromTop(20));column.removeFromTop(contentPadding);divider=column.getY();column.removeFromTop(contentPadding);
        automaticLabel.setBounds(column.removeFromTop(headingHeight));toggleRow(automatic,automaticToggleLabel);toggleRow(autoStop,autoStopLabel);
        column.removeFromTop(contentPadding);sensitivityLabel.setBounds(column.removeFromTop(24));sensitivity.setBounds(column.removeFromTop(controlHeight));
        for(auto* c:std::initializer_list<juce::Component*>{&recapture,&recaptureLabel,&recaptureHelp,&automaticLabel,&automatic,&automaticToggleLabel})c->setVisible(captureVisible);
        for(auto* c:std::initializer_list<juce::Component*>{&autoStop,&autoStopLabel,&sensitivity,&sensitivityLabel})c->setVisible(captureVisible && automaticOn);
        auto sound=left.withTrimmedRight(groupGap).withTrimmedTop(sections.getBottom()+contentPadding);
        irLabel.setBounds(sound.removeFromTop(24));auto choice=sound.removeFromTop(controlHeight);fileButton.setBounds(choice.removeFromRight(controlHeight));choice.removeFromRight(spacing::tight);irBox.setBounds(choice);
        sound.removeFromTop(contentPadding);irStatus.setBounds(sound.removeFromTop(20));
        sound.removeFromTop(contentPadding);soundScope.setBounds(sound.removeFromTop(40));soundScope.setVisible(soundVisible);
        for(auto* c:std::initializer_list<juce::Component*>{&irLabel,&irBox,&fileButton,&irStatus})c->setVisible(soundVisible);
        automaticHelp.setVisible(false);autoStopHelp.setVisible(false);generate.setVisible(automaticOn);generateReason.setVisible(automaticOn);
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
        const auto pixels=std::abs((static_cast<long double>(it->sample)-sample)*chart.getWidth()/(last-first));
        return pixels<=spacing::tight*2?std::optional<int64_t>{it->sample}:std::nullopt;
    }
    static juce::String wrappedHelper(const juce::String& text)
    {
        const auto words=juce::StringArray::fromTokens(text,false);juce::String result,line;
        const auto font=textFont(TextRole::helper);
        for(const auto& word:words)
        {
            const auto next=line.isEmpty()?word:line+" "+word;
            if(line.isNotEmpty() && juce::GlyphArrangement::getStringWidth(font,next)>workspaceGeometry::learnSettingsWidth)
            {result+=line+"\n";line=word;}
            else line=next;
        }
        return result+line;
    }
    static int64_t sampleBound(double value)noexcept
    {return static_cast<int64_t>(std::clamp(static_cast<long double>(value),static_cast<long double>(INT64_MIN),static_cast<long double>(INT64_MAX)));}
    int64_t pointerSample(juce::Point<float> point)const noexcept{return sampleBound(first+(last-first)*(point.x-chart.getX())/chart.getWidth());}
    void updateAccessibleDescription(){
        const juce::String markerHelp="Event guides brighten on hover or pin. Solid yellow: DAW position. White pin: inspection.";
        setTooltip(inspectionHelp+"\n"+markerHelp+"\n"+visibleInspectionText());setDescription(inspectionHelp+" "+statusText+" "+markerHelp+" "+visibleInspectionText());}
    void clearInspection(){const auto changed=inspection.has_value() || inspectionPinned || hoverEvent.has_value();inspection.reset();inspectionPinned=false;hoverEvent.reset();
        if(changed){updateAccessibleDescription();repaint();}}
    inline static const juce::String inspectionHelp="Left-click to pin/relocate inspection. The pin survives focus changes. Home/End and left/right inspect regions; Escape clears with waveform focus.";
    inline static const juce::String coverageKey="Audio | Silence | Gap: uncaptured | Unavailable | Recording";
    LearnDryView view;uint64_t inspectionSession=0,inspectionWorkflow=0;bool inspectionPinned=false;std::optional<int64_t> inspection,hoverEvent;juce::String totalsText,viewScope;
    struct Intent{uint64_t session,pass;std::vector<IRChoice> choices;};
    struct GenerationIntent{uint64_t session,pass,serial;};
    SwellEngine& engine;std::function<workflow::Snapshot()> flowNow;
    controls::Switch recapture{"Recapture existing audio"},automatic{"Enable automatic swells"};
    controls::Switch autoStop{"Auto-generate on Stop"};controls::ActionButton generate{"Generate Swells",controls::CommandIcon::none,true};
    controls::IconActionButton fileButton{controls::ActionIcon::openFile};controls::IrSelector irBox;controls::SliderControl sensitivity;
    juce::Label captureTitle,recaptureLabel,recaptureHelp,automaticLabel,automaticToggleLabel,automaticHelp,irLabel,irStatus,sensitivityLabel;
    juce::Label autoStopLabel,autoStopHelp,manualHelp,generateReason;
    LearnGenerationView generationView;uint64_t displaySession=0,displayPass=0;
    std::optional<GenerationIntent> autoStopIntent,generateIntent;
    std::unique_ptr<juce::FileChooser> chooser;std::optional<uint64_t> recapturePass,automaticPass,sensitivityPass;std::optional<Intent> irIntent;
    bool hasSource()const noexcept{return !capture.manifest.ranges.empty() || !capture.overview.empty() || !progress.ranges.empty();}
    std::vector<IRChoice> choices;juce::String catalogue,statusText;CaptureSessionSnapshot capture;CaptureProgress progress;SongTime clock;workflow::Phase phase=workflow::Phase::idle;
    juce::Rectangle<int> left,right,chart,timelineHeading,timelineCaption,timelineRuler,timelineStatus;int divider=0;double first=0,last=1;bool automaticOn=true,refreshing=false,playhead=false;
};
}
