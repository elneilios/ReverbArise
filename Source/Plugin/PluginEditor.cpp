#include "PluginEditor.h"
#include <UI/LevelAdjustment.h>
#include <UI/PerformText.h>
#include <UI/MusicalPosition.h>
#include <UI/LiveDiagnosticsText.h>
#include <UI/CaptureFailureText.h>
#include <DSP/DefaultImpulse.h>
#include <UI/InitialPerformView.h>
#include <UI/Controls/OwnedAlertWindow.h>

namespace
{
juce::String phaseName(m3::Phase phase)
{
    switch(phase)
    {
        case m3::Phase::empty:return "No IR";case m3::Phase::checking:return "Checking";case m3::Phase::queued:return "Queued";
        case m3::Phase::rendering:return "Rendering";case m3::Phase::ready:return "Ready";case m3::Phase::stale:return "Stale";
        case m3::Phase::error:return "Error";case m3::Phase::restoring:return "Restoring";
    }return {};
}
juce::String peakText(float peak){return peak>0?juce::String(juce::Decibels::gainToDecibels(peak),1)+" dBFS":juce::String("silent");}
juce::String sampleRateText(double rate)
{
    return juce::String(rate/1000,3).trimCharactersAtEnd("0").trimCharactersAtEnd(".")+" kHz";
}

class DeleteSwellsDialog final : public m3::ui::controls::OwnedAlertWindow
{
public:
    DeleteSwellsDialog(juce::Component& owner,int count)
        : OwnedAlertWindow("Delete "+juce::String(count)+" selected swells?",
                      "This removes all "+juce::String(count)+" selected swells from the list.",
                      juce::MessageBoxIconType::NoIcon,&owner)
    {
        using namespace m3::ui;
        setName("deleteSwellConfirmation");
        setEscapeKeyCancels(true);
        cancel.setName("cancelSwellDeletion");
        remove.setName("confirmSwellDeletion");
        actions.addAndMakeVisible(cancel);
        actions.addAndMakeVisible(remove);
        constexpr int buttonWidth=164;
        const auto height=controlGeometry::height+2*controlGeometry::focusPadding;
        actions.setSize(2*buttonWidth+spacing::related,juce::roundToInt(height));
        cancel.setBounds(actions.getLocalBounds().removeFromLeft(buttonWidth));
        remove.setBounds(actions.getLocalBounds().removeFromRight(buttonWidth));
        cancel.onClick=[this]{finish(0);};
        remove.onClick=[this]{finish(1);};
        addCustomComponent(&actions);
    }

    ~DeleteSwellsDialog() override
    {
        removeCustomComponent(0);
    }

    void show(std::function<void(int)> complete)
    {
        showNative();
        enterModalState(true,juce::ModalCallbackFunction::create(std::move(complete)),false);
        cancel.grabKeyboardFocus();
    }

private:
    juce::Component actions;
    m3::ui::controls::ActionButton cancel{"Cancel"};
    m3::ui::controls::ActionButton remove{"Delete swells",m3::ui::controls::CommandIcon::none,m3::ui::controls::CommandVariant::destructive};
};

class RenameSwellDialog final : public m3::ui::controls::OwnedAlertWindow
{
public:
    using Submit = std::function<bool(const juce::String&)>;

    RenameSwellDialog(juce::Component& owner,const juce::String& currentName,Submit submitName)
        : OwnedAlertWindow("Rename swell",{},juce::MessageBoxIconType::NoIcon,&owner),
          submit(std::move(submitName))
    {
        using namespace m3::ui;
        using namespace controls;
        setEscapeKeyCancels(true);
        caption.setText("Name",juce::dontSendNotification);
        applyTextRole(caption,TextRole::controlLabel);
        caption.setBorderSize({});
        caption.setAccessible(false);
        field.setName("name");
        field.setText(currentName,false);
        field.setSelectAllWhenFocused(true);
        field.onReturnKey=[this]{accept();};
        field.onEscapeKey=[this]{finish(0);};
        field.onFocusLost=[this]{if(isCurrentlyModal())validate();};
        ok.onClick=[this]{accept();};
        cancel.onClick=[this]{finish(0);};
        for(auto* child:std::initializer_list<juce::Component*>{&caption,&field,&feedback,&ok,&cancel})
            form.addAndMakeVisible(child);

        const int width=320;
        const auto frame=juce::Rectangle<int>(0,controlGeometry::labelHeight,width,
            controlGeometry::height+2*SelectorPresentation::focusMargin);
        const auto body=commandBodyBounds(frame);
        caption.setBounds(body.withY(0).withHeight(controlGeometry::labelHeight));
        field.setBounds(frame);
        const auto actions=FieldPresentation::feedbackBounds(body,body.getWidth()).getBottom()+spacing::related;
        ok.setBounds(body.getX(),actions,(body.getWidth()-spacing::related)/2,controlGeometry::height);
        cancel.setBounds(ok.getRight()+spacing::related,actions,body.getRight()-ok.getRight()-spacing::related,ok.getHeight());
        form.setSize(width,ok.getBottom());
        addCustomComponent(&form);
        describe("This name appears in the Swells list.",false);
    }

    ~RenameSwellDialog() override
    {
        field.onFocusLost={};
        removeCustomComponent(0);
    }

    void show(std::function<void(int)> complete)
    {
        showNative();
        enterModalState(true,juce::ModalCallbackFunction::create(std::move(complete)),true);
        field.grabKeyboardFocus();
        field.selectAll();
    }

private:
    void describe(const juce::String& message,bool error)
    {
        field.setInvalid(error);
        field.setFieldDescription(message);
        feedback.arrange(message,error,m3::ui::controls::commandBodyBounds(field.getBounds()));
    }

    bool validate()
    {
        const bool valid=field.getText().isNotEmpty();
        describe(valid?"This name appears in the Swells list.":"Enter a name for this swell before choosing OK.",!valid);
        return valid;
    }

    void accept()
    {
        if(!isCurrentlyModal())return;
        if(!validate())
        {
            field.grabKeyboardFocus();
            return;
        }
        if(submit(field.getText()))finish(1);
        else describe("This swell is no longer available. Close the dialog and try again.",true);
    }

    juce::Component form;
    juce::Label caption;
    m3::ui::controls::FieldTextEditor field{"Name"};
    m3::ui::controls::FieldFeedbackLabel feedback;
    m3::ui::controls::ActionButton ok{"OK"},cancel{"Cancel"};
    Submit submit;
};
}
PreverbAudioProcessorEditor::PreverbAudioProcessorEditor(PreverbAudioProcessor& p,std::function<void(const juce::String&)> writer)
    :AudioProcessorEditor(p),irBinding(irEditor,p.swellEngine()),processor(p),learnWorkspace(p.swellEngine(),[&p]{return p.getWorkflowSnapshot();}),exportWorkspace(p.swellEngine()),timeline(p.swellEngine())
{
    buildInformationWriter=writer?std::move(writer):[](const juce::String& text){juce::SystemClipboard::copyTextToClipboard(text);};
    setLookAndFeel(&workspaceTheme);addMouseListener(this,true);
    addAndMakeVisible(hoverHelp);hoverHelp.setName("hoverHelp");m3::ui::applyTextRole(hoverHelp,m3::ui::TextRole::helper);
    hoverHelp.setColour(juce::Label::textColourId,m3::ui::contextHelpInk);hoverHelp.setInterceptsMouseClicks(false,false);
    addChildComponent(learnWorkspace);addChildComponent(exportWorkspace);
    exportWorkspace.onReturn=[this]{processor.requestWorkflowMode(workflow::Mode::live);refresh();};
    exportWorkspace.onLoopSettings=[this](auto settings,uint64_t session,uint64_t pass){processor.swellEngine().setLoopExportOptions({settings.repeats,settings.gridBeats},session,pass);refresh();};
    exportWorkspace.onLoopLevels=[this](auto levels,uint64_t session,uint64_t pass){processor.swellEngine().setExportLevels(levels.dry,levels.wet,session,pass);refresh();};
    learnWorkspace.onRecapture=[this](bool enabled){
        if(processor.getLoopCapturePreference().enabled)processor.setLoopRecapture(enabled);
        else processor.setRecaptureExisting(enabled);
    };learnWorkspace.onRefresh=[this]{refresh();};
    learnWorkspace.onLoopPreference=[this](LoopCapturePreference preference){processor.setLoopCapturePreference(preference);};
    learnWorkspace.onLoopArm=[this](bool arm,uint64_t pass){if(arm)processor.armLoopCapture();else processor.requestLoopStop(pass);};
    addAndMakeVisible(workflowHeader);addAndMakeVisible(recaptureExisting);
    noticeBar.setName("workspaceNotice");noticeBar.setHeaderAppearance(true);addChildComponent(noticeBar);
    recaptureExisting.setName("legacyRecaptureExisting");
    workflowHeader.onModeRequested=[this](workflow::Mode mode){if(!updating && !destroying){processor.requestWorkflowMode(mode);refresh();}};
    recaptureExisting.onClick=[this]{processor.setRecaptureExisting(recaptureExisting.getToggleState());refresh();};
    setResizable(true,true);setResizeLimits(1100,850,2000,1400);
    for(auto* label:std::initializer_list<juce::Label*>{&status,&detail,&listLabel,&auditionLabel,&separationLabel,&spanLabel,&barStartLabel,&barEndLabel,&recaptureStatus,&toneStartLabel,&toneEndLabel})
    {label->setColour(juce::Label::textColourId,m3::ui::text);label->setJustificationType(juce::Justification::centredLeft);addAndMakeVisible(label);}
    m3::ui::applyTextRole(status,m3::ui::TextRole::helper,juce::Justification::topLeft);
    m3::ui::applyTextRole(detail,m3::ui::TextRole::helper,juce::Justification::topLeft);
    listLabel.setText("Swells",juce::dontSendNotification);auditionLabel.setText("Audition level",juce::dontSendNotification);
    separationLabel.setText("Min. separation",juce::dontSendNotification);
    spanLabel.setText("Bars",juce::dontSendNotification);
    barStartLabel.setText("Bar start",juce::dontSendNotification);barEndLabel.setText("End excl.",juce::dontSendNotification);
    toneStartLabel.setText("Tone start bar",juce::dontSendNotification);toneEndLabel.setText("End exclusive",juce::dontSendNotification);
    addAndMakeVisible(removeButton);
    for(auto* button:{&addButton,&generateButton,&allButton,&auditionButton,&stopButton,&analyzeButton,&confirmButton,&recaptureButton,&cancelButton,&fullButton,&resetButton,&setBarsButton,&clearButton})addAndMakeVisible(button);
    addAndMakeVisible(assetInfoButton);assetInfoButton.setTooltip("Copy the selected swell's managed asset paths for backup or validation.");
    assetInfoButton.onClick=[this]
    {
        const auto directory=processor.swellEngine().assetDirectory();juce::String info="Preverb working assets: "+directory.getFullPathName();
        if(const auto w=m3::find(state.session,state.session.primary))
        {info<<"\nSwell: "<<w->name<<"\nIR: "<<(w->ir.present()?directory.getChildFile(w->ir.path).getFullPathName():juce::String("not assigned"))
            <<"\nRender: "<<(w->render.present()?directory.getChildFile(w->render.path).getFullPathName():juce::String("not generated"));}
        juce::SystemClipboard::copyTextToClipboard(info);
    };
    for(auto* button:std::array<juce::Button*,6>{&autoButton,&enabledButton,&muteButton,&soloButton,&mappingButton,&toneButton})addAndMakeVisible(button);
    for(auto* box:std::array<juce::ComboBox*,3>{&candidateBox,&inCurveBox,&outCurveBox})addAndMakeVisible(box);
    inCurveBox.setName("fadeInCurve");outCurveBox.setName("fadeOutCurve");
    addAndMakeVisible(performZoom);addAndMakeVisible(snapButton);addAndMakeVisible(gridBox);addAndMakeVisible(listCount);
    performZoom.setName("performZoom");snapButton.setName("performSnap");gridBox.setName("performGrid");
    m3::ui::applyTextRole(listCount,m3::ui::TextRole::helper,juce::Justification::centredRight);
    generateButton.setName("generate");auditionButton.setName("audition");stopButton.setName("stopAudition");detail.setName("previewStatus");
    list.getProperties().set("contextHelp","Select a swell to edit. Use its checkbox to enable or disable it. Right-click the row or use its three dots for more actions.");
    list.onSelection=[this](auto ids,auto primary)
    {if(updating || destroying)return;processor.swellEngine().selectTargets(ids,primary,state.sessionEpoch);refresh();};
    list.onReveal=[this](auto id,uint64_t epoch)
    {
        if(updating || destroying || id.isEmpty() || learningLayout || exportLayout || !list.isVisible())return;
        refresh();
        if(state.sessionEpoch==epoch && state.session.primary==id && !learningLayout && !exportLayout)
            timeline.fitSelection();
    };
    list.onToggleEnabled=[this](auto id)
    {if(updating || destroying)return;processor.swellEngine().edit(id,[](auto& w,auto&){w.enabled=!w.enabled;return true;},state.sessionEpoch);refresh();};
    list.onMenu=[this](auto request){if(!updating && !destroying)showSwellMenu(std::move(request));};
    addAndMakeVisible(list);addAndMakeVisible(timeline);
    addAndMakeVisible(songOverview);timeline.setInlineOverview(false);timeline.onEnvelopeEdited=[this]{refresh();};
    const std::array<juce::String,fieldCount> labels{"Name","Arrival (sample)","Source start (sample)","Source end exclusive","Audible start (sample)","Audible end exclusive","Fade in (ms)","Fade out (ms)"};
    const std::array<juce::String,fieldCount> names{"swellName","arrivalSample","sourceSampleStart","sourceSampleEnd","audibleStart","audibleEnd","fadeInMs","fadeOutMs"};
    for(int i=0;i<fieldCount;++i)
    {
        fieldLabels[i].setText(labels[i],juce::dontSendNotification);m3::ui::applyTextRole(fieldLabels[i],m3::ui::TextRole::controlLabel);addAndMakeVisible(fieldLabels[i]);
        fields[i].setName(names[i]);fields[i].setTitle(labels[i]);fields[i].setReadOnly(true);addAndMakeVisible(fields[i]);
    }
    barStartEditor.setName("sourceStart");barEndEditor.setName("sourceEnd");barStartEditor.setText("1",false);barEndEditor.setText("2",false);
    for(auto* field:{&barStartEditor,&barEndEditor})
    {
        field->setInputRestrictions(12,"0123456789");
        field->setNumericCharacters("0123456789");
        addAndMakeVisible(field);
        field->onReturnKey=[this]{applyBarRange();};
        field->onFocusLost=[this]{validateBarRangeDraft();};
    }
    const auto slider=[&](juce::Slider& control,double a,double b,double step,const juce::String& suffix)
    {control.setRange(a,b,step);control.setSliderStyle(juce::Slider::LinearHorizontal);control.setTextBoxStyle(juce::Slider::TextBoxRight,false,78,22);control.setTextValueSuffix(suffix);addAndMakeVisible(control);};
    slider(auditionSlider,-60,0,0.1," dB");auditionSlider.setName("auditionLevel");
    slider(separationSlider,1,2000,1," ms");slider(toneStartSlider,1,511,1,"");slider(toneEndSlider,2,512,1,"");
    auditionSlider.setWheelSteps(1,.1);
    separationSlider.setWheelSteps(10,1);
    toneStartSlider.setWheelSteps(1,1);
    toneEndSlider.setWheelSteps(1,1);
    for(size_t n=0;n<m3::gridChoices.size();++n)gridBox.addItem(juce::String(m3::gridChoices[n].label.data()),static_cast<int>(n)+1);
    gridBox.setSelectedId(3,juce::dontSendNotification);gridBox.setTitle("Snap grid division");
    for(auto* box:{&inCurveBox,&outCurveBox}){box->addItem("Linear",1);box->addItem("Smooth",2);box->addItem("Exponential",3);}
    snapButton.setToggleState(true,juce::dontSendNotification);
    addButton.beforeInvocation=[this]{
        addTargetInvocation.reset();
        const auto identity=timeline.targetIdentity();
        const auto sample=identity.sourceKind==CaptureSourceKind::loop
            ? timeline.loopInsertionSample() : std::optional{processor.getSongTime().sample};
        if(sample)addTargetInvocation=std::pair{*sample,identity};
    };
    addButton.onClick=[this]{
        const auto invocation=std::exchange(addTargetInvocation,std::nullopt);
        if(!invocation || updating || destroying)return;
        processor.swellEngine().addTarget(invocation->first,nullptr,invocation->second);
        refresh();
    };
    addButton.setTooltip("Move FL Studio's playhead to captured audio, or double-click the source waveform to add a target.");
    removeButton.setName("settingsDeleteSwells");
    removeButton.bindInvocation([this]() -> m3::ui::controls::CapturedActionButton::Invocation
    {
        if(updating || destroying)return {};
        auto current=processor.swellEngine().snapshot();
        const auto flow=processor.getWorkflowSnapshot();
        if(current.session.selected.empty() || flow.mode!=workflow::Mode::live)return {};
        return [this,ids=std::move(current.session.selected),epoch=current.sessionEpoch,pass=flow.pass]() mutable
        {
            requestDeleteSwells(std::move(ids),epoch,pass);
        };
    });
    generateButton.onClick=[this]{processor.swellEngine().generate();refresh();};allButton.onClick=[this]{showRegenerationResult(processor.swellEngine().generate(true));refresh();};
    auditionButton.onClick=[this]{
        if(!processor.auditionPreview())auditionRetryAfter=juce::Time::getMillisecondCounterHiRes()+2000;
        refresh();
    };stopButton.onClick=[this]{processor.swellEngine().stop();};
    auditionSlider.onValueChange=[this]{if(!updating)setPreferences();};
    separationSlider.beforeInvocation=[this]{separationPass=processor.getWorkflowSnapshot().pass;};
    separationSlider.onValueChange=[this]{if(!updating){const auto s=processor.swellEngine().savedSession();processor.swellEngine().setPreferences(s.auditionDb,s.sensitivity,separationSlider.getValue()/1000,s.autoRegenerate,separationPass.value_or(processor.getWorkflowSnapshot().pass));}};
    autoButton.setName("autoRefreshAfterRecapture");
    autoButton.beforeInvocation=[this]{refreshPass=processor.getWorkflowSnapshot().pass;};
    autoButton.onClick=[this]{const auto s=processor.swellEngine().savedSession();const auto pass=std::exchange(refreshPass,{}).value_or(processor.getWorkflowSnapshot().pass);
        processor.swellEngine().setPreferences(s.auditionDb,s.sensitivity,s.separationSeconds,autoButton.getToggleState(),pass);refresh();};
    analyzeButton.onClick=[this]
    {
        const auto capture=processor.getCaptureSnapshot();
        if(capture.manifest.ranges.empty()){processor.swellEngine().diagnostic("Capture source before suggesting onsets");return;}
        const auto& last=capture.manifest.ranges.back();processor.swellEngine().analyze(capture.manifest.ranges.front().startSample,last.startSample+static_cast<int64_t>(last.frameCount));
    };
    candidateBox.beforeInvocation=[this]
    {
        const juce::Component::SafePointer<PreverbAudioProcessorEditor> safe(this);
        candidateBox.canCommit=[safe,source=onsetSource,epoch=state.sessionEpoch,pass=state.workflowState.pass]
        {
            if(!safe || safe->destroying || safe->updating)return false;
            const auto current=safe->processor.swellEngine().snapshot();
            if(current.sessionEpoch!=epoch || current.workflowState.pass!=pass)return false;
            const auto capture=safe->processor.getCaptureSnapshot();
            if(source!=std::pair{capture.reference,capture.manifest.revision})return false;
            const auto viewport=safe->timeline.viewport();
            return safe->candidateBox.matches(current.candidates,viewport.first,viewport.last);
        };
    };
    confirmButton.onClick=[this]{if(const auto sample=candidateBox.selectedSample())processor.swellEngine().confirmCandidateAt(*sample);refresh();};
    timeline.onCandidateSelected=[this](int index)
    {
        if(index<0 || index>=static_cast<int>(state.candidates.size()))return;
        const auto viewport=timeline.viewport();
        candidateBox.display(state.candidates,viewport.first,viewport.last,state.candidates[static_cast<size_t>(index)].sample);
    };
    const auto beginNavigation=[this]{navigationInvocation.reset();const auto flow=processor.getWorkflowSnapshot();
        if(!updating && flow.mode==workflow::Mode::live)navigationInvocation=NavigationScope{processor.swellEngine().assignmentEpoch(),flow.pass,state.session.primary};};
    performZoom.beforeInvocation=snapButton.beforeInvocation=gridBox.beforeInvocation=beginNavigation;
    timeline.onViewportChanged=[this]{if(!updating && !learningLayout && !exportLayout){performFitted=true;navigationDirty=true;publishPerformNavigation();}refreshPerformNavigation();songOverview.repaint();};
    performZoom.onZoomStep=[this](double factor){if(navigationIntentCurrent()){timeline.zoom(factor);refreshPerformNavigation();songOverview.repaint();}};
    performZoom.onZoomFraction=[this](double fraction){if(navigationIntentCurrent()){timeline.setZoomFraction(fraction);refreshPerformNavigation();songOverview.repaint();}};
    performZoom.onFit=[this]{if(navigationIntentCurrent()){timeline.fitSelection();performFitted=true;refreshPerformNavigation();songOverview.repaint();}};
    snapButton.onClick=[this]{if(navigationIntentCurrent()){timeline.setSnap(!timeline.requestedSnap());for(auto& cell:valueControls)cell.rebaseGesture();navigationDirty=true;publishPerformNavigation();}refreshPerformNavigation();};
    gridBox.onChange=[this]{const auto index=gridBox.getSelectedId()-1;if(navigationIntentCurrent() && index>=0 && index<static_cast<int>(m3::gridChoices.size())){timeline.setGridDivision(m3::gridChoices[static_cast<size_t>(index)].division);for(auto& cell:valueControls)cell.cancel();navigationDirty=true;publishPerformNavigation();}refreshPerformNavigation();};
    recaptureButton.onClick=[this]{armSelected();};cancelButton.onClick=[this]{processor.cancelRecapture();refresh();};
    fullButton.onClick=[this]{shapeReset(true);};resetButton.onClick=[this]{shapeReset(false);};setBarsButton.onClick=[this]{applyBarRange();};
    // A popup inspects a snapshot. Legacy Settings commands must still refer to
    // that session/swell when the asynchronous choice eventually returns.
    const auto captureSelectorContext=[this](m3::ui::controls::ChoiceSelector& selector,bool perSwell)
    {
        const auto epoch=state.sessionEpoch,pass=processor.getWorkflowSnapshot().pass;
        const auto primary=state.session.primary;const auto selected=state.session.selected;
        const auto* swell=m3::find(state.session,primary);
        const auto generation=swell?swell->inputGeneration:0;
        const juce::Component::SafePointer<PreverbAudioProcessorEditor> safe(this);
        selector.canCommit=[safe,epoch,pass,primary,selected,generation,perSwell]
        {
            if(!safe || safe->destroying || safe->updating)return false;
            const auto current=safe->processor.swellEngine().snapshot();
            if(current.sessionEpoch!=epoch || safe->processor.getWorkflowSnapshot().pass!=pass)return false;
            if(!perSwell)return true;
            const auto* target=m3::find(current.session,primary);
            return target && current.session.primary==primary && current.session.selected==selected && target->inputGeneration==generation;
        };
    };
    inCurveBox.beforeInvocation=[this,captureSelectorContext]{captureSelectorContext(inCurveBox,true);};
    outCurveBox.beforeInvocation=[this,captureSelectorContext]{captureSelectorContext(outCurveBox,true);};
    liveRoutingBox.beforeInvocation=[this,captureSelectorContext]{captureSelectorContext(liveRoutingBox,false);};
    gainPolicyBox.beforeInvocation=[this,captureSelectorContext]{captureSelectorContext(gainPolicyBox,false);};
    inCurveBox.onChange=[this]{curveChanged(true);};outCurveBox.onChange=[this]{curveChanged(false);};
    const auto flagScope=[this]{return FlagScope{state.sessionEpoch,processor.getWorkflowSnapshot().pass,state.session.primary};};
    const auto bindFlag=[this,flagScope](m3::ui::controls::BooleanToggle& control,std::optional<FlagScope>& pending,auto apply)
    {
        control.beforeInvocation=[&pending,flagScope]{pending=flagScope();};
        control.onClick=[this,&control,&pending,flagScope,apply]
        {
            const auto target=std::exchange(pending,{}).value_or(flagScope());
            if(updating)return;
            const auto current=processor.swellEngine().snapshot();
            if(control.isEnabled() && target.primary.isNotEmpty() && current.sessionEpoch==target.epoch
                && current.session.primary==target.primary && processor.getWorkflowSnapshot().pass==target.pass)
                apply(target,control.getToggleState());
            refresh();
        };
    };
    const auto activity=[this](bool m3::Swell::*flag)
    {
        return [this,flag](const FlagScope& target,bool value)
        {processor.swellEngine().edit(target.primary,[flag,value](auto& w,auto&){w.*flag=value;return true;},target.epoch);};
    };
    enabledButton.setName("swellEnabled");soloButton.setName("swellSolo");muteButton.setName("swellMute");
    bindFlag(enabledButton,enabledScope,activity(&m3::Swell::enabled));
    bindFlag(muteButton,muteScope,activity(&m3::Swell::muted));
    bindFlag(soloButton,soloScope,activity(&m3::Swell::solo));
    bindFlag(freezeButton,freezeScope,[this](const FlagScope& target,bool value)
    {processor.swellEngine().setFrozen(target.primary,value,target.epoch);});
    mappingButton.onClick=[this]{processor.setMappingEnabled(mappingButton.getToggleState());};clearButton.onClick=[this]{processor.requestClearCapture();refresh();};
    toneButton.onClick=[this]{processor.setTestToneEnabled(toneButton.getToggleState());};
    toneStartSlider.onValueChange=[this]{if(!updating)processor.setToneStartBar(static_cast<int>(toneStartSlider.getValue()));};
    toneEndSlider.onValueChange=[this]{if(!updating)processor.setToneEndBar(static_cast<int>(toneEndSlider.getValue()));};
    for(auto* label:std::initializer_list<juce::Label*>{&headerStatus,&selectedStatus,&monitorStatus,&helper})
    {addAndMakeVisible(label);label->setColour(juce::Label::textColourId,m3::ui::secondary);}
    headerStatus.setName("processingStatus");selectedStatus.setName("selectedStatus");
    headerStatus.setBorderSize(juce::BorderSize<int>(0,24,0,8));
    helper.setText("Live playback: awaiting prepared Song audio",juce::dontSendNotification);
    liveButton.setName("livePlayback");liveRoutingBox.setName("liveRouting");liveLevel.setName("liveWetLevel");liveLevel.setTooltip("Adjust the live wet level. New audition monitoring uses this gain too; captured and live dry stay unchanged.");
    liveRoutingBox.addItem("Dry + Wet",1);liveRoutingBox.addItem("Wet Only",2);
    liveLevelLabel.setText("Live wet",juce::dontSendNotification);
    liveLevel.setRange(-60,0,.1);liveLevel.setTextValueSuffix(" dB");liveLevel.setSliderStyle(juce::Slider::LinearHorizontal);
    liveLevel.setWheelSteps(1,.1);
    liveLevel.setTextBoxStyle(juce::Slider::TextBoxRight,false,75,24);
    gainPolicyLabel.setText("Gain staging",juce::dontSendNotification);gainPolicyBox.setName("gainPolicy");
    gainPolicyBox.addItem("Legacy (original projects)",1);gainPolicyBox.addItem("Musical (source energy +6 dB)",2);
    gainPolicyBox.addItem("Saved musical prototype",3);gainPolicyBox.setItemEnabled(3,false);
    gainPolicyBox.setChoiceReason(3,"Displayed for saved-project compatibility; choose Legacy or Musical to change policy");
    gainPolicyBox.onChange=[this]{if(!updating && gainPolicyBox.getSelectedId()!=3){processor.swellEngine().setGainPolicy(gainPolicyBox.getSelectedId()==2?m3::GainPolicy::musical:m3::GainPolicy::legacy);refresh();}};
    addAndMakeVisible(liveButton);addAndMakeVisible(liveRoutingBox);addAndMakeVisible(liveLevel);addAndMakeVisible(liveLevelLabel);
    const auto updateLive=[this]{if(!updating)processor.swellEngine().setLivePlayback(liveButton.getToggleState(),
        liveRoutingBox.getSelectedId()==2?m3::LiveRouting::wetOnly:m3::LiveRouting::dryWet,static_cast<float>(liveLevel.getValue()));};
    liveButton.onClick=updateLive;liveRoutingBox.onChange=updateLive;liveLevel.onValueChange=updateLive;
    helper.setFont(juce::FontOptions(m3::ui::smallSize));
    for(auto* button:{&defaultIRButton,&suppressionButton})addAndMakeVisible(button);
    addAndMakeVisible(auditionWetOnly);auditionWetOnly.setName("auditionWetOnly");monitorStatus.setName("auditionStatus");
    addChildComponent(loopCue);addChildComponent(loopPlaybackStatus);
    loopCue.setName("loopCue");loopPlaybackStatus.setName("loopPlaybackStatus");
    m3::ui::applyTextRole(loopPlaybackStatus,m3::ui::TextRole::helper);
    loopCue.beforeInvocation=[this]{loopButtonInvocation=std::pair{processor.getWorkflowSnapshot().pass,loopButtonStops};};
    loopCue.onClick=[this]{
        const auto invocation=std::exchange(loopButtonInvocation,std::nullopt);
        const auto flow=processor.getWorkflowSnapshot();
        if(!invocation || updating || destroying || !loopPerformLayout || flow.mode!=workflow::Mode::live || flow.pass!=invocation->first)return;
        if(invocation->second)processor.requestLoopPlaybackStop();
        else processor.requestLoopCue();
        refresh();
    };
    for(auto* button:std::array<juce::Button*,2>{&automaticButton,&sampleUnitsButton})addAndMakeVisible(button);
    sampleUnitsButton.setName("sampleUnits");automaticButton.setName("automaticProcessing");
    workflowHeader.onSettings=[this]{if(!destroying)setSettingsOpen(!settingsWindow || !settingsWindow->isVisible());};
    automaticButton.beforeInvocation=[this]{automaticPass=processor.getWorkflowSnapshot().pass;};
    automaticButton.onClick=[this]{const auto pass=std::exchange(automaticPass,{}).value_or(processor.getWorkflowSnapshot().pass);
        processor.swellEngine().setAutomaticProcessing(automaticButton.getToggleState(),pass);refresh();};
    sampleUnitsButton.onClick=[this]{refresh();};
    suppressionButton.onClick=[this]{processor.swellEngine().resetSuppression();refresh();};
    defaultIRButton.onClick=[this]{learnWorkspace.openDefaultIrFile(&defaultIRButton);};
    auditionWetOnly.beforeInvocation=[this]{auditionInvocation=std::pair{state.sessionEpoch,processor.getWorkflowSnapshot().pass};};
    auditionWetOnly.onClick=[this]{if(!updating && auditionInvocation && auditionWetOnly.isEnabled())processor.swellEngine().setAuditionWetOnly(auditionWetOnly.getToggleState(),auditionInvocation->first,auditionInvocation->second);auditionInvocation.reset();refresh();};
    auditionWetOnly.setTooltip("Off: captured dry plus wet at Live Wet gain. On: wet only at that gain. Loops until Stop; safety protection remains active.");
    enabledButton.setTooltip("Include this swell in wet playback and audition.");soloButton.setTooltip("Solo this swell: exclude other non-solo swells from wet playback and audition.");
    stopButton.setTooltip("Stop audition preview; does not stop the DAW");auditionButton.setButtonText("Audition");auditionLabel.setText("Trim (dB)",juce::dontSendNotification);auditionSlider.setDoubleClickReturnValue(true,0);
    inspectorViewport.setViewedComponent(&inspectorContent,false);diagnosticsViewport.setViewedComponent(&diagnosticsContent,false);
    inspectorViewport.setName("inspectorViewport");diagnosticsViewport.setName("diagnosticsViewport");
    inspectorViewport.setScrollBarsShown(true,false);diagnosticsViewport.setScrollBarsShown(true,false);
    addAndMakeVisible(inspectorViewport);diagnosticsViewport.setVisible(true);
    for(auto* control:std::array<juce::Component*,8>{&selectedStatus,&enabledButton,&muteButton,&soloButton,&fullButton,&resetButton,&inCurveBox,&outCurveBox,})inspectorContent.addAndMakeVisible(control);
    slider(tailSlider,0,5,.01," s");tailSlider.setName("auditionTail");tailSlider.setTooltip("Captured Dry + Wet context after the later of the original arrival and wet endpoint. Dry audio is unchanged.");
    tailSlider.setWheelSteps(.1,.01);
    tailSlider.onValueChange=[this]{if(!updating)processor.swellEngine().setPostRoll(tailSlider.getValue());};
    addAndMakeVisible(tailLabel);tailLabel.setText("Listen after",juce::dontSendNotification);
    for(auto* label:std::initializer_list<juce::Label*>{&markerLabel,&markerState}){addAndMakeVisible(label);label->setColour(juce::Label::textColourId,m3::ui::secondary);}
    markerLabel.setText("Marker sensitivity",juce::dontSendNotification);markerKnob.setName("markerSensitivity");markerButton.setName("autoMarkers");isolateButton.setName("isolateSelected");
    slider(markerKnob,0,100,.1," %");markerKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    markerKnob.setTooltip("Lower: fewer, stronger attacks. Higher: finer detail. Changes reanalyse captured audio and preserve manual edits. This does not detect musical keys.");
    addAndMakeVisible(markerButton);addAndMakeVisible(isolateButton);
    markerKnob.beforeInvocation=[this]{markerPass=processor.getWorkflowSnapshot().pass;};
    markerKnob.onValueChange=[this]{if(!updating){const auto s=processor.swellEngine().savedSession();processor.swellEngine().setPreferences(s.auditionDb,static_cast<float>(markerKnob.getValue()/100),s.separationSeconds,s.autoRegenerate,markerPass.value_or(processor.getWorkflowSnapshot().pass));}};
    addAndMakeVisible(regenerationLabel);regenerationLabel.setText("Regeneration sensitivity",juce::dontSendNotification);
    slider(regenerationKnob,0,100,.1," %");regenerationKnob.setName("regenerationSensitivity");regenerationKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    regenerationKnob.setTooltip("Lower tolerates more performance variation against the retained capture; 100% compares exact samples. Separate from marker detection. Explicit recapture remains exact.");
    regenerationKnob.onValueChange=[this]{if(!updating)processor.swellEngine().setRegenerationSensitivity(static_cast<float>(regenerationKnob.getValue()/100));};
    inspectorContent.addAndMakeVisible(freezeButton);freezeButton.setName("freezeRender");
    freezeButton.setTooltip("Keep this rendered wet audio fixed against source changes. Unfreeze to change source/IR or extend render support. Envelope, gain and placement remain editable.");
    markerButton.beforeInvocation=[this]{markingPass=processor.getWorkflowSnapshot().pass;};
    markerButton.onClick=[this]{processor.swellEngine().setAutoMarkers(markerButton.getToggleState(),markingPass.value_or(processor.getWorkflowSnapshot().pass));refresh();};
    addAndMakeVisible(isolateDryButton);isolateDryButton.setName("isolateDry");
    isolateDryButton.setTooltip("Replay captured dry only within the selected source interval; overlapping mixed notes inside it cannot be separated. Live input is unchanged. Off includes the listening context.");
    isolateDryButton.onClick=[this]{processor.swellEngine().setIsolateDry(isolateDryButton.getToggleState());refresh();};
    isolateButton.onClick=[this]{processor.swellEngine().setIsolateSelected(isolateButton.getToggleState());refresh();};
    diagnosticsContent.addAndMakeVisible(placementLabel);diagnosticsContent.addAndMakeVisible(placementEditor);
    placementLabel.setText("Move wet (ms)",juce::dontSendNotification);placementEditor.setName("legacy_wetPlacement");
    diagnosticsContent.addAndMakeVisible(timingButton);timingButton.setName("advancedTiming");
    timingButton.onClick=[this]{advancedTiming=!advancedTiming;resized();};
    diagnosticsContent.addAndMakeVisible(copyDiagnosticsButton);copyDiagnosticsButton.setName("copyDiagnostics");
    copyDiagnosticsButton.setTooltip("Copy the complete diagnostic text, precise host/source timing and retained offline export error to paste into a support message.");
    copyDiagnosticsButton.onClick=[this]{
        diagnosticsRefreshedAt=-1;refresh();
        juce::SystemClipboard::copyTextToClipboard("ReverbArise diagnostics\n\n"+status.getText()+"\n\n"+captureDiagnostic.getText()
            +"\n\n"+detail.getText()+"\n\n"+playbackDiagnostic.getText()+"\n\n"+policyDiagnostic.getText()
            +"\n\nSnap\nEffective snap: "+juce::String(timeline.effectiveSnap()?"On":"Off")
            +" | Fixed grid available: "+juce::String(timeline.gridAvailable()?"Yes":"No")+"\n\n"+liveDiagnostic.getText());
    };
    // The swell name is renamed from the list row menu, not the Selected panel.
    for(int i=arrival;i<fieldCount;++i){inspectorContent.addAndMakeVisible(fieldLabels[i]);inspectorContent.addAndMakeVisible(fields[i]);}
    fields[name].setVisible(false);fieldLabels[name].setVisible(false);
    for(auto* control:std::array<juce::Component*,28>{&status,&detail,&generateButton,&allButton,&analyzeButton,&confirmButton,&candidateBox,&separationLabel,&separationSlider,&autoButton,&recaptureButton,&cancelButton,&recaptureStatus,&mappingButton,&toneButton,&toneStartLabel,&toneStartSlider,&toneEndLabel,&toneEndSlider,&clearButton,&assetInfoButton,&barStartLabel,&barEndLabel,&barStartEditor,&barEndEditor,&setBarsButton,&auditionLabel,&auditionSlider})diagnosticsContent.addAndMakeVisible(control);
    for(auto* control:std::array<juce::Component*,4>{&automaticButton,&sampleUnitsButton,&defaultIRButton,&suppressionButton})diagnosticsContent.addAndMakeVisible(control);
    for(auto* control:std::array<juce::Component*,10>{&removeButton,&liveButton,&liveRoutingBox,&liveLevel,&liveLevelLabel,&isolateButton,&isolateDryButton,&tailSlider,&tailLabel,&helper})diagnosticsContent.addAndMakeVisible(control);
    for(auto* label:std::initializer_list<juce::Label*>{&inspectorHeading,&listEmpty,&inspectorEmpty,&auditionHeading,&editorReadout})
    {addAndMakeVisible(label);label->setColour(juce::Label::textColourId,m3::ui::text);label->setInterceptsMouseClicks(false,false);}
    inspectorHeading.setText("Selected Swell",juce::dontSendNotification);m3::ui::applyTextRole(inspectorHeading,m3::ui::TextRole::sectionHeading);
    auditionHeading.setText("Audition",juce::dontSendNotification);m3::ui::applyTextRole(auditionHeading,m3::ui::TextRole::sectionHeading);
    addAndMakeVisible(hostInfo);hostInfo.setInterceptsMouseClicks(false,false);
    listEmpty.setText("No swells yet\n\nUse Learn to capture source.\nIn Perform, add a swell with +.",juce::dontSendNotification);listEmpty.setJustificationType(juce::Justification::centred);m3::ui::applyTextRole(listEmpty,m3::ui::TextRole::emptyBody,juce::Justification::centred);
    inspectorEmpty.setText("No swell selected\n\nSelect a swell to adjust\nits settings.",juce::dontSendNotification);inspectorEmpty.setJustificationType(juce::Justification::centred);m3::ui::applyTextRole(inspectorEmpty,m3::ui::TextRole::emptyBody,juce::Justification::centred);
    m3::ui::applyTextRole(listLabel,m3::ui::TextRole::sectionHeading);
    headerStatus.setBorderSize(juce::BorderSize<int>(0));headerStatus.setFont(juce::FontOptions(m3::ui::smallSize));
    headerStatus.setJustificationType(juce::Justification::centredRight);
    workflowHeader.settingsAction().setTooltip("Settings: playback, processing, selected swell, audition and recovery");
    markerButton.setButtonText("Auto-marker");
    const char* groupNames[]={"Timing","Envelope","Level"};
    for(size_t i=0;i<inspectorGroups.size();++i){auto& group=inspectorGroups[i];inspectorContent.addAndMakeVisible(group);group.setText(groupNames[i],juce::dontSendNotification);group.setName("inspector"+juce::String(groupNames[i])+"Group");m3::ui::applyTextRole(group,m3::ui::TextRole::controlLabel);}
    const std::array<juce::String,valueCount> valueTitles{"Target","Source Start","Source End","Pre-roll (s)","End Offset (s)","Fade In (ms)","Fade Out (ms)","Level (dB)","Curve In (unitless)","Curve Out (unitless)","Move Wet (ms)"};
    const std::array<juce::String,valueCount> valueNames{"arrivalSample","sourceSampleStart","sourceSampleEnd","preRoll","endOffset","fadeInMs","fadeOutMs","swellLevel","curveInTension","curveOutTension","wetPlacement"};
    // Keep the older exact-bound tools available in Settings until its dedicated
    // cleanup stage; only the new cells own the primary timing/shape names.
    for(int field:{arrival,sourceA,sourceB,fadeIn,fadeOut})fields[field].setName("legacy_"+fields[field].getName());
    placementEditor.setName("legacy_wetPlacement");
    for(int index=0;index<valueCount;++index)
    {
        auto& cell=valueControls[index];inspectorContent.addAndMakeVisible(cell);
        cell.configure(valueTitles[index],valueNames[index],index==targetValue?"":index<=sourceEndValue?"Absolute musical position: bar.beat.fraction. Typed entry is never snapped. Ctrl bypasses Snap; Shift gives fine adjustment.":
            index==preRollValue?"Seconds before the fixed dry target. Negative values start after it. Placement is inverted when editing.":
            index==endOffsetValue?"Signed seconds from the dry target. Negative creates a gap; positive overlaps unchanged dry audio.":
            index==placementValue?"Move wet audio and envelope together in milliseconds; dry stays fixed. Typed entry is never snapped.":
            index==inCurveValue || index==outCurveValue?"Independent curve tension, -1 to +1. Curve preset is in Settings.":"Type an exact value, or drag the knob. Shift-drag for fine adjustment.");
        cell.onBegin=[this,index]{beginValueEdit(index);};cell.onEnd=[this,index]{valueAnchors[index].reset();};
        cell.onDelta=[this,index](double delta){commitValueEdit(index,{},delta);};
        cell.onRebase=[this,index]{rebaseValueEdit(index);};
        cell.onStep=[this,index](double amount,bool fineWheel){stepValueEdit(index,amount,fineWheel);};
        cell.setResetAction([this,index]{return prepareValueReset(index);});
        cell.onModifierInput=[this](const juce::ModifierKeys& mods){modifierKeysChanged(mods);};
        cell.onCommit=[this,index](const auto& text){return commitValueEdit(index,text);};
        cell.onFeedbackChanged=[this]{valueFeedbackChanged=true;};
        if(index==levelValue)
        {
            cell.setHomeDetent({0,m3::ui::levelAdjustment::homeBandDb});
            cell.onAbsolute=[this](double db){commitValueEdit(levelValue,m3::ui::preciseValue(db));};
            cell.onDialPress=[this]
            {
                const auto& anchor=valueAnchors[levelValue];
                const auto& invocation=valueInvocations[levelValue];
                const auto* displayed=m3::find(state.session,state.session.primary);
                if(!anchor || !displayed || invocation.epoch!=state.sessionEpoch || anchor->id!=displayed->id
                    || anchor->inputGeneration!=displayed->inputGeneration || invocation.selected!=state.session.selected)
                {
                    valueControls[levelValue].cancel();
                    return;
                }
                if(anchor->shape.gainDb<m3::ui::levelAdjustment::minimumDb)
                    commitValueEdit(levelValue,{},0.);
            };
        }
    }
    irEditor.setName("selectedIrEditor");irEditor.selector.setName("selectedIR");irEditor.fileButton.setName("loadIR");irEditor.status.setName("irAssignmentStatus");
    inspectorContent.addAndMakeVisible(irEditor);inspectorContent.addAndMakeVisible(performSoundScope);
    performSoundScope.setName("performSoundScope");performSoundScope.setText("Selected swell only",juce::dontSendNotification);
    m3::ui::applyTextRole(performSoundScope,m3::ui::TextRole::helper);performSoundScope.setTooltip(performSoundScope.getText());
    irBinding.onRefresh=[this]{if(!destroying)refresh();};
    resetButton.setButtonText("Reset");resetButton.setName("resetWetShape");
    resetButton.setTooltip("Reset only the wet shape: start/end, fades, curves, level and placement return to defaults. Source points, IR and activity stay unchanged.");
    inspectorContent.addAndMakeVisible(resetMenu);resetMenu.setName("resetSwellMenu");resetMenu.setButtonText("Reset All");
    resetMenu.setTooltip("Reset this whole swell back to creation defaults: wet shape plus dry source points, IR, enabled/solo/freeze and activity.");
    resetMenu.onClick=[this]{const auto id=state.session.primary;processor.swellEngine().resetSwell(id,true);refresh();};
    for(auto* label:std::initializer_list<juce::Label*>{&captureDiagnostic,&playbackDiagnostic,&policyDiagnostic}){diagnosticsContent.addAndMakeVisible(label);m3::ui::applyTextRole(*label,m3::ui::TextRole::helper,juce::Justification::topLeft);}
    diagnosticsContent.addAndMakeVisible(liveDiagnostic);liveDiagnostic.setName("diagnosticLiveAdmission");
    m3::ui::applyTextRole(liveDiagnostic,m3::ui::TextRole::helper,juce::Justification::topLeft);
    status.setName("diagnosticTiming");captureDiagnostic.setName("diagnosticCapture");playbackDiagnostic.setName("diagnosticPlayback");policyDiagnostic.setName("diagnosticPolicy");
    toneButton.setName("scheduledTone");
    for(auto* label:std::initializer_list<juce::Label*>{&gainPolicyLabel,&liveLevelLabel,&tailLabel,&placementLabel})
        m3::ui::applyTextRole(*label,m3::ui::TextRole::controlLabel);
    initialiseSettings();
    for(auto* heading:{&listLabel,&inspectorHeading,&auditionHeading})m3::ui::applyTextRole(*heading,m3::ui::TextRole::sectionHeading);
    for(auto* label:std::initializer_list<juce::Label*>{&selectedStatus,&monitorStatus,&editorReadout})m3::ui::applyTextRole(*label,m3::ui::TextRole::helper);
    for(auto* label:{&listEmpty,&inspectorEmpty})m3::ui::applyTextRole(*label,m3::ui::TextRole::emptyBody,juce::Justification::centred);
    // setResizeLimits may already have brought the editor to the minimum.
    // Lay out once after every child is attached even if setSize is a no-op.
    setSize(1100,850);resized();refresh();startTimerHz(10);
}
PreverbAudioProcessorEditor::~PreverbAudioProcessorEditor(){publishPerformNavigation();destroying=true;swellMenu.reset();deleteSwellConfirmation.reset();if(renameDialog)renameDialog->exitModalState(0);recoveryWarning.reset();recoveryInspection.cancel();removeMouseListener(this);stopTimer();if(presentation)presentation->persistIfDue(juce::Time::getMillisecondCounterHiRes(),true);diagnosticsWindow.reset();settingsWindow.reset();list.setModel(nullptr);setLookAndFeel(nullptr);}
void PreverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    if(responsivePresentation){g.fillAll(m3::ui::background);return;}
    paintWorkspace(g);
}
void PreverbAudioProcessorEditor::paintWorkspace(juce::Graphics& g)
{
    if(responsivePresentation)
    {
        using namespace m3::ui;
        const ResponsiveWorkspaceLayout layout(workspaceBounds());g.fillAll(background);g.setColour(separator);
        g.drawHorizontalLine(layout.header.getBottom(),float(layout.header.getX()),float(layout.header.getRight()));
        if(learningLayout || exportLayout)return;
        for(const auto y:{layout.work.getBottom(),layout.audition.getBottom(),layout.overview.getBottom()})
            g.drawHorizontalLine(y,float(layout.header.getX()),float(layout.header.getRight()));
        for(const auto line:{layout.listDivider(),layout.inspectorDivider()})
            g.drawVerticalLine(juce::roundToInt(line.getStartX()),line.getStartY(),line.getEndY());
        if(loopPerformLayout)g.drawVerticalLine(monitorStatus.getRight(),static_cast<float>(loopCue.getY()),static_cast<float>(loopCue.getBottom()));
        return;
    }
    g.fillAll(m3::ui::background);const m3::ui::WorkspaceLayout layout(getLocalBounds());g.setColour(m3::ui::separator);
    if(learningLayout || exportLayout){g.drawHorizontalLine(layout.header.getBottom(),static_cast<float>(layout.header.getX()),static_cast<float>(layout.header.getRight()));return;}
    const m3::ui::PerformLayout perform(getLocalBounds());
    for(const auto y:{perform.header.getBottom(),perform.work.getBottom(),perform.audition.getBottom(),perform.overview.getBottom()})
        g.drawHorizontalLine(y,static_cast<float>(perform.header.getX()),static_cast<float>(perform.header.getRight()));
    for(const auto x:{perform.list.getRight()+m3::ui::performGeometry::columnGap/2,perform.inspector.getX()-m3::ui::performGeometry::columnGap/2})
        g.drawVerticalLine(x,static_cast<float>(perform.work.getY()+m3::ui::performGeometry::columnInset),static_cast<float>(perform.work.getBottom()-m3::ui::performGeometry::columnInset));
    if(loopPerformLayout)g.drawVerticalLine(monitorStatus.getRight(),static_cast<float>(loopCue.getY()),static_cast<float>(loopCue.getBottom()));
}
void PreverbAudioProcessorEditor::requestDeleteSwells(std::vector<juce::String> ids,uint64_t epoch,
                                                      uint64_t pass,juce::String focusRow,bool keyboard)
{
    if(ids.empty() || destroying || deleteSwellConfirmation
       || processor.swellEngine().assignmentEpoch()!=epoch || processor.getWorkflowSnapshot().pass!=pass
       || processor.getWorkflowSnapshot().mode!=workflow::Mode::live)return;

    const auto count=ids.size();
    const juce::Component::SafePointer<PreverbAudioProcessorEditor> safe(this);
    auto complete=[safe,ids=std::move(ids),epoch,pass,focusRow=std::move(focusRow),keyboard](int result)
    {
        if(!safe || safe->destroying)return;
        safe->deleteSwellConfirmation.reset();
        if(result==1)
        {
            safe->processor.swellEngine().removeTargets(ids,epoch,pass);
            safe->refresh();
        }
        if(safe && focusRow.isNotEmpty() && safe->list.isShowing())
        {
            auto focus=m3::ui::controls::SurfaceFocusReturn::capture(safe->list,safe.getComponent());focus.keyboard=keyboard;
            focus.post({},[safe,focusRow](bool keys){if(safe)safe->list.restoreMenuFocus(focusRow,keys);});
        }
    };
    if(count==1){complete(1);return;}

    auto dialog=std::make_unique<DeleteSwellsDialog>(*this,static_cast<int>(count));
    auto* window=dialog.get();
    deleteSwellConfirmation=std::move(dialog);
    window->show(std::move(complete));
}

void PreverbAudioProcessorEditor::showSwellMenu(m3::ui::controls::SwellList::MenuRequest request)
{
    using namespace m3::ui::controls;
    if(swellMenu || deleteSwellConfirmation || !list.isShowing() || !list.isEnabled())return;

    const auto snapshot=processor.swellEngine().snapshot();
    const auto flow=processor.getWorkflowSnapshot();
    const auto* swell=m3::find(snapshot.session,request.id);
    if(!swell || snapshot.sessionEpoch!=request.epoch || flow.mode!=workflow::Mode::live)return;

    const auto& selection=snapshot.session.selected;
    auto targets=std::find(selection.begin(),selection.end(),request.id)!=selection.end()
        ? selection : std::vector<juce::String>{request.id};
    const auto deleteLabel=targets.size()>1?"Delete "+juce::String(static_cast<int>(targets.size()))+" swells":juce::String("Delete");
    std::vector<PopupChoice> actions{{1,"Rename...",{},selection.size()<=1,false,PopupRowRole::action},
                                     {2,deleteLabel,{},true,false,PopupRowRole::destructiveAction}};
    const juce::Component::SafePointer<PreverbAudioProcessorEditor> safe(this);
    swellMenu=std::make_unique<ChoicePopup>(list,std::move(actions),
        [safe,request,name=swell->name,pass=flow.pass,targets=std::move(targets)](int result,PopupDismissal why)
        {
            if(!safe || safe->destroying)return;
            const auto keyboard=safe->swellMenu && safe->swellMenu->showFocusOnReturn();
            safe->swellMenu.reset();
            if(!safe || why==PopupDismissal::outside || why==PopupDismissal::ownerGone
               || !safe->list.isShowing() || !safe->list.isEnabled())return;

            if(why==PopupDismissal::tabForward)
            {
                safe->inspectorSections.grabKeyboardFocus();
                return;
            }
            if(why==PopupDismissal::tabBackward)
            {
                safe->list.restoreMenuFocus(request.id,true,false);
                return;
            }
            safe->list.restoreMenuFocus(request.id,keyboard);

            auto& engine=safe->processor.swellEngine();
            if(engine.assignmentEpoch()!=request.epoch || safe->processor.getWorkflowSnapshot().pass!=pass)return;
            if(result==2)
            {
                safe->requestDeleteSwells(targets,request.epoch,pass,request.id,keyboard);
                return;
            }

            const auto current=engine.savedSession();
            if(result!=1 || safe->renameDialog || current.selected.size()>1 || !m3::find(current,request.id))return;
            auto* dialog=new RenameSwellDialog(*safe,name,[safe,request,pass](const juce::String& value)
            {
                if(!safe || safe->destroying || safe->processor.getWorkflowSnapshot().pass!=pass
                   || safe->processor.swellEngine().savedSession().selected.size()>1)return false;
                const bool accepted=safe->processor.swellEngine().edit(request.id,[&](auto& swell,auto&){swell.name=value;return true;},request.epoch);
                if(safe)safe->refresh();
                return accepted;
            });
            safe->renameDialog=dialog;
            auto focus=m3::ui::controls::SurfaceFocusReturn::capture(safe->list,safe.getComponent());focus.keyboard=keyboard;
            dialog->show([safe,focus,id=request.id](int){focus.post({},[safe,id](bool keyboard){if(safe)safe->list.restoreMenuFocus(id,keyboard);});});
        });
    swellMenu->setName("swellActionMenu");
    swellMenu->setTitle("Actions for "+swell->name);
    swellMenu->show(request.anchor,request.keyboard);
}
void PreverbAudioProcessorEditor::setPreferences()
{const auto s=processor.swellEngine().savedSession();processor.swellEngine().setPreferences(static_cast<float>(auditionSlider.getValue()),s.sensitivity,s.separationSeconds,s.autoRegenerate);}
void PreverbAudioProcessorEditor::curveChanged(bool in)
{
    if(updating || destroying)return;const auto curve=static_cast<m3::Curve>((in?inCurveBox:outCurveBox).getSelectedId()-1);
    processor.swellEngine().edit(state.session.primary,[&](auto& w,auto&){if(in)w.shape.inCurve=curve;else w.shape.outCurve=curve;return true;});
}
void PreverbAudioProcessorEditor::applyBarRange()
{
    if(updating || destroying || !sourceRangeDraftTarget || sourceRangeDraftTarget->primary.isEmpty())return;
    const auto start=barStartEditor.getText();
    const auto end=barEndEditor.getText();
    if(start.isEmpty() && end.isEmpty())return;
    sourceRangeResult=processor.setSourceBars(start.getLargeIntValue(),end.getLargeIntValue(),sourceRangeDraftTarget);
    refresh();
}
void PreverbAudioProcessorEditor::validateBarRangeDraft()
{
    if(updating || destroying || !sourceRangeDraftTarget)return;
    const auto* swell=m3::find(state.session,sourceRangeDraftTarget->primary);
    if(!swell || swell->frozen)return;
    const auto start=barStartEditor.getText();
    const auto end=barEndEditor.getText();
    sourceRangeResult=start.isEmpty() && end.isEmpty() ? PreverbAudioProcessor::SourceBarSubmission{}
        : processor.checkSourceBars(start.getLargeIntValue(),end.getLargeIntValue(),*sourceRangeDraftTarget);
    refresh();
}
void PreverbAudioProcessorEditor::shapeReset(bool full)
{
    if(!full){processor.swellEngine().resetSwell(state.session.primary,false);refresh();return;}
    const auto raw=state.raw;
    processor.swellEngine().edit(state.session.primary,[&](auto& w,auto& error)
    {if(full){
        if(w.ir.present() && w.ir.content!=w.render.irContent){
            const auto length=std::round(w.ir.frames*state.session.timing.rate/w.ir.rate);int64_t start;
            if(!state.session.timing.valid || !std::isfinite(length) || length<1 || length>=static_cast<double>(INT64_MAX)
                || !m3::checkedSubtract(w.sourceStart,static_cast<int64_t>(length)-1,start)){error="Full response needs valid requested IR timing";return false;}
            return m3::fullShape(w,start,m3::renderInputEnd(w),error);}
        if(!raw)return false;return m3::fullShape(w,raw->start,raw->end,error);}m3::resetShape(w);return true;});refresh();
}
void PreverbAudioProcessorEditor::armSelected()
{
    int64_t a=INT64_MAX,b=INT64_MIN;
    for(const auto& id:state.session.selected)if(const auto w=m3::find(state.session,id)){a=std::min(a,w->sourceStart);b=std::max(b,w->sourceEnd);}
    if(b>a)processor.armRecapture(a,b);refresh();
}
void PreverbAudioProcessorEditor::resized()
{
    if(responsivePresentation){resizePresentation();return;}
    const m3::ui::WorkspaceLayout layout(getLocalBounds());
    const auto inset=m3::ui::workspaceGeometry::outerInset(getWidth());
    hoverHelp.setBounds(inset,getHeight()-m3::ui::controlGeometry::feedbackHeight,getWidth()-2*inset,m3::ui::controlGeometry::feedbackHeight);
    workflowHeader.setBounds(layout.header);
    noticeBar.setBounds(layout.notice);
    if(learningLayout || exportLayout){auto body=getLocalBounds().reduced(m3::ui::workspaceGeometry::outerInset(getWidth()));body.setTop(layout.header.getBottom()+m3::ui::spacing::group);
        body.setBottom(std::min(body.getBottom(),hoverHelp.getY()));learnWorkspace.setBounds(body);exportWorkspace.setBounds(body);return;}
    const m3::ui::PerformLayout perform(getLocalBounds());
    using namespace m3::ui;
    for(auto* component:std::initializer_list<juce::Component*>{&markerButton,&markerLabel,&markerKnob,&recaptureExisting,&headerStatus,&spanLabel}){component->setBounds({});component->setVisible(false);}
    regenerationLabel.setBounds({});regenerationKnob.setBounds({});
    markerState.setBounds({});
    auto listArea=perform.list.reduced(0,performGeometry::columnInset);const auto titleY=listArea.getY();auto listTop=listArea.removeFromTop(36);listTop.removeFromRight(42);
    listCount.setBounds(listTop.removeFromRight(44));listLabel.setBounds(listTop);addButton.setBounds(perform.list.getRight()-36,titleY+2,34,34);addButton.setButtonText("+");
    list.setBounds(listArea.withTrimmedTop(8));listEmpty.setBounds(listArea.reduced(6,30));
    auto inspectorArea=perform.inspector.reduced(0,performGeometry::columnInset);inspectorHeading.setBounds(inspectorArea.removeFromTop(36));inspectorArea.removeFromTop(8);
    inspectorViewport.setBounds(inspectorArea);inspectorEmpty.setBounds(inspectorArea.reduced(10));
    const auto inspectorWidth=perform.inspector.getWidth();
    auto navigation=perform.toolbar.reduced(performGeometry::toolbarInset);
    gridBox.setRowBounds(navigation.removeFromRight(performGeometry::gridWidth));navigation.removeFromRight(performGeometry::toolbarGap);
    snapButton.setBounds(navigation.removeFromRight(performGeometry::snapWidth));navigation.removeFromRight(performGeometry::toolbarGap);performZoom.setBounds(navigation);
    editorReadout.setBounds(perform.readout);editorReadout.setName("editorReadout");
    timeline.setBounds(perform.detail.withTrimmedTop(performGeometry::columnInset));
    auto audition=perform.audition.reduced(0,performGeometry::bandInset);auto auditionRow=audition.removeFromTop(36);
    auditionHeading.setBounds(auditionRow.removeFromLeft(112));auditionButton.setBounds(auditionRow.removeFromLeft(50));auditionRow.removeFromLeft(8);stopButton.setBounds(auditionRow.removeFromLeft(50));auditionRow.removeFromLeft(performGeometry::toolbarGap);
    auditionWetOnly.setBounds(auditionRow.removeFromLeft(controlGeometry::valueWidth+2*spacing::related));
    auditionRow.removeFromLeft(spacing::related);layoutLoopPlayback(auditionRow);
    songOverview.setBounds(perform.overview.reduced(0,performGeometry::bandInset));hostInfo.setBounds(perform.info.withBottom(std::min(perform.info.getBottom(),hoverHelp.getY())));
    const auto width=inspectorWidth-16;
    auto inspector=juce::Rectangle<int>(width,std::numeric_limits<int>::max()).reduced(2,4);
    selectedStatus.setBounds(inspector.removeFromTop(24));
    fields[name].setBounds({});fieldLabels[name].setBounds({});
    auto row=inspector.removeFromTop(36);enabledButton.setBounds(row.removeFromLeft((inspectorWidth-20)/3));soloButton.setBounds(row.removeFromLeft((inspectorWidth-20)/3));freezeButton.setBounds(row);
    performSoundScope.setBounds(inspector.removeFromTop(controlGeometry::feedbackHeight));
    irEditor.setBounds(inspector.removeFromTop(irEditor.preferredHeight(inspector.getWidth())));
    inspector.removeFromTop(12);
    for(auto& group:inspectorGroups)group.setVisible(true);
    {
        const auto group=[&](int index){inspectorGroups[index].setBounds(inspector.removeFromTop(controlGeometry::labelHeight));inspector.removeFromTop(performGeometry::toolbarGap);};
        const auto values=[&](int first,int second=-1)
        {inspector.removeFromTop(layoutValueRow(inspector.withHeight(64),first,second,performGeometry::toolbarGap));};
        group(0);values(targetValue,placementValue);values(sourceStartValue);values(sourceEndValue);
        inspector.removeFromTop(spacing::group);group(1);values(preRollValue,endOffsetValue);values(fadeInValue,fadeOutValue);values(inCurveValue,outCurveValue);
        inspector.removeFromTop(spacing::group);group(2);values(levelValue);
    }
    // Destructive resets sit beneath all the knob controls.
    inspector.removeFromTop(12);
    row=inspector.removeFromTop(34);auto resetLeft=row.removeFromLeft((row.getWidth()-8)/2);row.removeFromLeft(8);
    resetButton.setBounds(resetLeft);resetMenu.setBounds(row);
    inspectorContent.setSize(width,row.getBottom()+spacing::tight);
    // Precision observations are owned by Diagnostics; layout refreshes below
    // must not repeatedly reparent controls or shrink its scroll surface.
    for(int field=arrival;field<fieldCount;++field){diagnosticsContent.addAndMakeVisible(fieldLabels[field]);diagnosticsContent.addAndMakeVisible(fields[field]);}
    layoutSettings();layoutLiveDiagnostics();

}
void PreverbAudioProcessorEditor::enableResponsiveReview(std::shared_ptr<m3::ui::PresentationStorage> storage,std::optional<m3::ui::PresentationFitContext> display)
{
    enableResponsivePresentation(storage?std::move(storage):std::make_shared<m3::ui::TransientPresentationStorage>(),display);
}
void PreverbAudioProcessorEditor::enableResponsivePresentation(std::shared_ptr<m3::ui::PresentationStorage> storage,std::optional<m3::ui::PresentationFitContext> display)
{
    if(responsivePresentation)return;
    inspectorSections.setName("inspectorSections");addAndMakeVisible(inspectorSections);
    list.setInspectorTabTarget(inspectorSections);
    inspectorSections.onRequest=[this](int section){inspectorSection=section;refreshResponsiveSections();};
    // Fixed actions and flags belong to the editor, outside the scrolling fields.
    for(auto* child:std::initializer_list<juce::Component*>{&selectedStatus,&enabledButton,&soloButton,&freezeButton,&resetButton,&resetMenu})
        addAndMakeVisible(child);
    for(auto& control:valueControls)control.setCompactPresentation(true);
    timeline.setResponsivePresentation(true);
    inspectorViewport.setFocusRevealEnabled(true);diagnosticsViewport.setFocusRevealEnabled(true);
    const auto children=getChildren();
    for(auto* child:children)
        if(child!=recoveryWarning.get() && dynamic_cast<juce::ResizableCornerComponent*>(child)==nullptr && dynamic_cast<juce::ResizableBorderComponent*>(child)==nullptr)
            presentationSurface.contentRoot().addChildComponent(child);
    addAndMakeVisible(presentationSurface);presentationSurface.toBack();
    presentationSurface.setPainter([this](auto& drawing){paintWorkspace(drawing);});
    responsivePresentation=true;reviewDisplay=display;
    presentationStorage=storage?std::move(storage):std::make_shared<m3::ui::TransientPresentationStorage>();
    presentation=std::make_unique<m3::ui::EditorPresentationState>(*presentationStorage,presentationDisplay());
    initialisePresentation();applyPresentation();refresh();
}
void PreverbAudioProcessorEditor::refreshResponsiveSections()
{
    const bool selected=m3::find(state.session,state.session.primary)!=nullptr && !learningLayout && !exportLayout;
    inspectorSections.display({{1,"Timing","Target and source timing"},{2,"Envelope","Swell envelope"},{3,"Sound","Sound for selected swell"}},inspectorSection);
    for(auto* child:std::initializer_list<juce::Component*>{&inspectorSections,&selectedStatus,&enabledButton,&soloButton,&freezeButton,&resetButton,&resetMenu})
        child->setVisible(selected);
    for(auto& group:inspectorGroups)group.setVisible(false);
    for(int i=0;i<valueCount;++i)
    {
        const auto section=i==levelValue?3:(i==targetValue || i==placementValue || i==sourceStartValue || i==sourceEndValue)?1:2;
        valueControls[i].setVisible(selected && section==inspectorSection);
    }
    for(auto* child:std::initializer_list<juce::Component*>{&irEditor,&performSoundScope})child->setVisible(selected && inspectorSection==3);
    // Mute/full response/curve presets are owned by Settings, not this inspector.
    // Section refresh must not hide their existing access route.
    const bool editable=inspectorContent.isEnabled();
    for(auto* child:std::initializer_list<juce::Component*>{&enabledButton,&soloButton,&freezeButton,&resetButton,&resetMenu})child->setEnabled(child->isEnabled() && selected && editable);
    repaint();
}
void PreverbAudioProcessorEditor::layoutResponsive()
{
    using namespace m3::ui;using namespace responsiveGeometry;
    const auto bounds=workspaceBounds();
    const ResponsiveWorkspaceLayout layout(bounds);const auto type=ResponsiveTypography::at(bounds.getWidth(),bounds.getHeight());
    workspaceTheme.setResponsiveViewport(bounds.getWidth(),bounds.getHeight());
    // SliderControl owns a private shared theme. Resolve that same profile
    // without recreating controls or interrupting an active text invocation.
    const auto updateThemes=[&](auto&& self,juce::Component& parent)->void
    {
        if(auto* theme=dynamic_cast<WorkspaceTheme*>(&parent.getLookAndFeel()))theme->setResponsiveViewport(bounds.getWidth(),bounds.getHeight());
        if(auto* slider=dynamic_cast<m3::ui::controls::InvocationSlider*>(&parent))slider->setValueFont(componentTextFont(parent,TextRole::controlValue));
        for(auto* child:parent.getChildren())self(self,*child);
    };
    updateThemes(updateThemes,*this);
    for(auto& control:valueControls)control.lookAndFeelChanged();
    workflowHeader.setResponsivePresentation(type);workflowHeader.setBounds(layout.header);
    noticeBar.setResponsivePresentation(type);noticeBar.setBounds(layout.notice);hoverHelp.setBounds(layout.help);
    learnWorkspace.setResponsivePresentation(type);exportWorkspace.setResponsivePresentation();
    learnWorkspace.setBounds(layout.body);exportWorkspace.setBounds(layout.body);
    if(learningLayout || exportLayout)return;
    for(auto* child:std::initializer_list<juce::Component*>{&markerButton,&markerLabel,&markerKnob,&recaptureExisting,&headerStatus,&spanLabel,&regenerationLabel,&regenerationKnob,&markerState})
    {child->setBounds({});child->setVisible(false);}
    auto listArea=layout.list.withTrimmedRight(contentPadding);auto title=listArea.removeFromTop(headingHeight);
    auto add=title.removeFromRight(24);addButton.setBounds(add);addButton.setButtonText("+");
    listCount.setBounds(title.removeFromRight(36));listLabel.setBounds(title);listArea.removeFromTop(contentPadding);
    list.setBounds(listArea.withTrimmedBottom(contentPadding));listEmpty.setBounds(list.getBounds().reduced(contentPadding));
    auto heading=layout.inspectorHeading;selectedStatus.setBounds(heading.removeFromRight(56));inspectorHeading.setBounds(heading);
    auto flags=layout.flags;enabledButton.setBounds(flags.removeFromLeft(90));soloButton.setBounds(flags.removeFromLeft(65));freezeButton.setBounds(flags);
    inspectorSections.setTextHeight(type.body);inspectorSections.setBounds(layout.tabs);
    inspectorViewport.setBounds(layout.fields);inspectorEmpty.setBounds(layout.fields);
    auto resets=layout.resets;resetButton.setBounds(resets.removeFromLeft((resets.getWidth()-contentPadding)/2));resets.removeFromLeft(contentPadding);resetMenu.setBounds(resets);
    layoutResponsiveValues();
    timeline.setBounds(layout.detail);editorReadout.setBounds(layout.readout);
    performZoom.setBounds(layout.zoom.reduced(0,3));snapButton.setBounds(layout.snap.reduced(0,3));gridBox.setRowBounds(layout.grid.reduced(0,3));
    auto audition=layout.audition.reduced(0,contentPadding);auditionHeading.setBounds(audition.removeFromLeft(88));
    auditionButton.setBounds(audition.removeFromLeft(40));audition.removeFromLeft(contentPadding);stopButton.setBounds(audition.removeFromLeft(40));audition.removeFromLeft(contentPadding);
    auditionWetOnly.setBounds(audition.removeFromLeft(96));audition.removeFromLeft(contentPadding);layoutLoopPlayback(audition);
    songOverview.setBounds(layout.overview.reduced(0,contentPadding));hostInfo.setBounds(layout.position);
    for(int field=arrival;field<fieldCount;++field){diagnosticsContent.addAndMakeVisible(fieldLabels[field]);diagnosticsContent.addAndMakeVisible(fields[field]);}
    layoutSettings();layoutPresentationSettings();layoutLiveDiagnostics();refreshResponsiveSections();
}
int PreverbAudioProcessorEditor::layoutValueRow(juce::Rectangle<int> row,int first,int second,int gap)
{
    if(second<0)return valueControls[first].setValueBounds(row);
    const auto left=row.removeFromLeft((row.getWidth()-gap)/2);
    row.removeFromLeft(gap);
    return std::max(valueControls[first].setValueBounds(left),valueControls[second].setValueBounds(row));
}
void PreverbAudioProcessorEditor::layoutResponsiveValues()
{
    using namespace m3::ui;
    using namespace responsiveGeometry;
    const auto width=inspectorViewport.getWidth();
    const auto values=[&](int y,int first,int second=-1)
    {return y+layoutValueRow({0,y,width,valueRowHeight},first,second,contentPadding);};
    const auto timingBottom=values(values(values(0,targetValue,placementValue),sourceStartValue),sourceEndValue);
    const auto envelopeBottom=values(values(values(0,preRollValue,endOffsetValue),fadeInValue,fadeOutValue),inCurveValue,outCurveValue);
    performSoundScope.setBounds(0,0,width,controlGeometry::feedbackHeight);
    const auto soundTop=controlGeometry::feedbackHeight+contentPadding;
    irEditor.setBounds(0,soundTop,width,irEditor.preferredHeight(width));
    const auto soundBottom=values(irEditor.getBottom()+contentPadding,levelValue);
    inspectorContent.setSize(width,std::max({inspectorViewport.getHeight(),timingBottom,envelopeBottom,soundBottom}));
}
void PreverbAudioProcessorEditor::layoutLiveDiagnostics()
{
    const auto inset=m3::ui::spacing::section;
    const juce::Rectangle<int> panelBounds(0,0,960,850);auto diagnostic=panelBounds.reduced(20);
    const auto message=[&](m3::ui::controls::WrappedLabel& label,int minimum)
    {
        const auto height=std::max(minimum,label.preferredHeight(diagnostic.getWidth()));
        label.setBounds(diagnostic.getX(),diagnostic.getY(),diagnostic.getWidth(),height);
        diagnostic.setY(diagnostic.getY()+height);
        diagnostic.setHeight(std::max(diagnostic.getHeight(),height));
    };
    message(status,72);
    message(captureDiagnostic,72);
    message(detail,140);
    message(playbackDiagnostic,80);
    message(policyDiagnostic,96);
    auto row=diagnostic.removeFromTop(36);timingButton.setBounds(row.removeFromLeft(290).reduced(1));assetInfoButton.setBounds(row.removeFromLeft(180).reduced(1));
    copyDiagnosticsButton.setBounds(row.removeFromLeft(200).reduced(1));
    for(int field=arrival;field<fieldCount;++field){row=diagnostic.removeFromTop(32);fieldLabels[field].setBounds(row.removeFromLeft(200));fields[field].setBounds(row.removeFromLeft(300).reduced(1,3));}
    row=diagnostic.removeFromTop(34);placementLabel.setBounds(row.removeFromLeft(200));placementEditor.setBounds(row.removeFromLeft(300));
    helper.setBounds({});
    const auto height=liveDiagnostic.preferredHeight(status.getWidth())+inset;
    liveDiagnostic.setBounds(status.getX(),placementEditor.getBottom()+inset,status.getWidth(),height);
    diagnosticsContent.setSize(panelBounds.getWidth(),liveDiagnostic.getBottom()+inset);
}
void PreverbAudioProcessorEditor::layoutLoopPlayback(juce::Rectangle<int> row)
{
    using namespace m3::ui;
    if(!loopPerformLayout){monitorStatus.setBounds(row);return;}
    const auto buttonWidth=2*controlGeometry::valueWidth;
    auto playback=row.removeFromRight(std::min(row.getWidth(),buttonWidth+2*spacing::related+3*controlGeometry::valueWidth));
    monitorStatus.setBounds(row);
    playback.removeFromLeft(spacing::related);loopCue.setBounds(playback.removeFromLeft(buttonWidth));
    playback.removeFromLeft(spacing::related);loopPlaybackStatus.setBounds(playback);
}

void PreverbAudioProcessorEditor::refreshLoopPlayback(bool visible,bool liveEnabled,double capturedRate,
                                                     const HostPositionDiagnostics& diagnostics)
{
    using Phase=m3::LoopLiveReference::State;
    using Prep=m3::LoopPlaybackPublication::Preparation;
    using Playing=m3::LoopCueScheduler::State;
    loopCue.setVisible(visible);loopPlaybackStatus.setVisible(visible);
    if(!visible){timeline.setLoopPlayhead({});loopButtonInvocation.reset();return;}
    const auto playback=processor.loopPlaybackStatus();
    timeline.setLoopPlayhead(playback.playhead);
    const auto active=playback.playback!=Playing::standby && playback.playback!=Playing::stopping;
    const auto playlist=processor.usesFlStudioLoopTiming();
    const auto transportPlaying=!playlist || diagnostics.isPlaying;
    const auto rateMismatch=capturedRate>0 && diagnostics.sampleRate>0 && capturedRate!=diagnostics.sampleRate;
    const auto ready=liveEnabled && transportPlaying && !rateMismatch
        && playback.phase==Phase::ready && playback.preparation==Prep::ready;
    juce::String label="Cue Loop",text,help;
    if(playback.playback==Playing::queued){label="Cancel Loop";text="Waiting for bar";help="Cancel the queued Loop before it starts.";}
    else if(playback.playback==Playing::leadIn || playback.playback==Playing::repeating)
    {
        label="Stop Loop";
        text=playback.playback==Playing::leadIn?"Playing lead-in":"Playing loop";
        help="Finish this pass and skip the next lead-in.";
    }
    else if(playback.playback==Playing::finishing){label="Stop Now";text="Finishing loop";help="Stop immediately with a short fade. Dry audio continues.";}
    else if(playback.playback==Playing::stopping){label="Stop Loop";text="Fading out";help="Loop playback is fading out. Dry audio continues.";}
    else if(!liveEnabled){text="Live output off";help="Enable Live playback in Settings.";}
    else if(rateMismatch)
    {
        const auto expected=sampleRateText(capturedRate);
        text="Loop needs "+expected;
        help="Set the host sample rate to "+expected+" to play this Loop.";
    }
    else if(!transportPlaying){text="Start Playlist playback";help="Press Play in FL Studio to enable Cue Loop.";}
    else if(playlist && playback.phase!=Phase::ready){text="Playlist timing unavailable";help="Use steady 4/4 Playlist timing at the Loop's sample rate.";}
    else if(playback.phase==Phase::unconfigured){text="Start the clock";help="Send one Clock pulse per beat to enable Cue Loop.";}
    else if(playback.phase==Phase::waitingForClock){text="Waiting for clock";help="Keep the Clock running so Cue Loop can find the next bar.";}
    else if(playback.phase==Phase::unsupported){text="Unsupported phrase input";help="Use a verified phrase input configuration for this sample rate and callback size.";}
    else if(playback.phase==Phase::mismatch){text="Reference differs from Loop";help="Match the accepted Loop length, tempo, meter and sample rate.";}
    else if(playback.phase==Phase::lost){text="Sync lost";help="Restore the recurring phrase markers, then Cue again.";}
    else if(playback.phase==Phase::waiting){text="Verifying phrase sync";help="Keep the phrase reference running through two matching intervals.";}
    else if(playback.preparation==Prep::noWet){text=playlist?"Generate swells first":"No prepared wet";help="Prepare at least one enabled, audible swell.";}
    else if(playback.preparation==Prep::budget){text="Wet exceeds limit";help="Reduce wet overlap or the amount of prepared audio.";}
    else if(playback.preparation==Prep::waiting){text="Preparing wet";help="Wait for the current swell renders.";}
    else {text="Ready";help="Cue the next start that has room for the full lead-in.";}
    loopButtonStops=active;
    loopCue.setButtonText(label);
    loopCue.setVariant(playback.playback==Playing::standby?m3::ui::controls::CommandVariant::neutral:m3::ui::controls::CommandVariant::destructive);
    loopCue.setEnabled(active || (ready && playback.playback==Playing::standby));
    loopCue.setTooltip(help);loopCue.setHelpText(help);
    loopPlaybackStatus.setText(text,juce::dontSendNotification);loopPlaybackStatus.setTooltip(help);loopPlaybackStatus.setDescription(text.isEmpty()?help:text+". "+help);
}

void PreverbAudioProcessorEditor::refresh()
{
    if(destroying)return;updating=true;state=processor.swellEngine().snapshot();const auto capture=processor.getCaptureSnapshot();
    const bool loopSource=capture.sourceKind()==CaptureSourceKind::loop;
    const auto diagnostics=processor.getHostPositionDiagnostics();
    const auto hostTime=diagnostics.songTime();
    const auto time=loopSource?processor.getSourceEditTime(capture):hostTime;
    refreshStorageScope(capture,state.sessionEpoch);
    serviceAssetRecovery();
    if(regenerationFeedbackSession!=state.sessionEpoch)
    {regenerationFeedback.setText({},juce::dontSendNotification);regenerationFeedback.setDescription({});regenerationFeedbackSession=state.sessionEpoch;}
    const auto mode=state.workflowState.mode;
    if(presentationSession!=state.sessionEpoch)presentationSession=state.sessionEpoch;
    bool initialFrame=false;
    if(mode==workflow::Mode::live && !state.session.swells.empty())
    {
        const auto first=m3::ui::firstChronologicalSwell(state.session);
        if(state.initialPerformHandoff && processor.swellEngine().selectInitialPerform(state.sessionEpoch,state.initialPerformHandoff,first))
        {
            state=processor.swellEngine().snapshot();initialFrame=state.sessionEpoch==presentationSession
                && state.session.primary==first && processor.getWorkflowSnapshot().mode==workflow::Mode::live;
        }
        processor.swellEngine().establishPerformPosition(state.sessionEpoch);
    }
    const auto nextLearning=mode==workflow::Mode::capture,nextExport=mode==workflow::Mode::finalise;
    const auto loopSettings=processor.getLoopCapturePreference();
    const bool songView=!loopSource && !nextExport && (!nextLearning || !loopSettings.enabled);
    std::optional<m3::ui::controls::NoticeMessage> songTimingIssue;
    bool songPlayheadAvailable=false;
    if(songView)
    {
        using m3::ui::controls::NoticeMessage;
        using m3::ui::controls::NoticeTone;
        long double origin;
        if(!songOrigin(hostTime,origin))
        {
            if(diagnostics.blockSize>0)
                songTimingIssue=NoticeMessage{"Song Mode needs bar and beat position, tempo and time signature from the host.",
                    "For a clock-based setup, use Loop Mode in Learn.",NoticeTone::warning};
        }
        else
        {
            const auto clock=m3::gridClockContext(state.gridClock,!nextLearning && diagnostics.isPlaying);
            const auto& accepted=state.session.timing.valid?state.session.timing:hostTime;
            const auto rate=capture.manifest.ranges.empty()?hostTime.rate:capture.manifest.sampleRate;
            m3::GridEvidence evidence;
            songPlayheadAvailable=m3::HostGrid::make(accepted,hostTime,rate,m3::GridDivision::beat,&evidence,
                diagnostics.samplePositionAvailable,&clock).has_value();
            if(evidence.gate==m3::GridGate::clockRejected || evidence.gate==m3::GridGate::runOrigin)
                songTimingIssue=NoticeMessage{"Song timing changed during playback.",
                    "Stop playback, then restart from a position with a steady tempo and time signature.",NoticeTone::warning};
            else if(!songPlayheadAvailable && evidence.gate!=m3::GridGate::clockUnavailable)
                songTimingIssue=NoticeMessage{"Host timing differs from the captured song.",
                    "Check the host’s song position, tempo, time signature and sample rate against the original capture.",NoticeTone::warning};
        }
    }
    const auto loopLayoutChanged=loopPerformLayout!=(!nextLearning && !nextExport && loopSource);
    loopPerformLayout=!nextLearning && !nextExport && loopSource;
    if(exportLayout && !nextExport)exportWorkspace.resetView();
    const auto wasSpecial=learningLayout || exportLayout,nextSpecial=nextLearning || nextExport;
    const auto leavingLearn=wasSpecial && !nextSpecial;
    const auto isShared=[this](juce::Component* c){return c==recoveryWarning.get() || c==&workflowHeader || c==&noticeBar || c==&hoverHelp || c==&learnWorkspace || c==&exportWorkspace
        || dynamic_cast<juce::ResizableCornerComponent*>(c)!=nullptr || dynamic_cast<juce::ResizableBorderComponent*>(c)!=nullptr;};
    if(nextLearning!=learningLayout || nextExport!=exportLayout)
    {
        if(nextSpecial && !wasSpecial){if(hasPerformWorkspace)performViewport=timeline.viewport();performVisibility.clear();for(auto* c:workspaceRoot().getChildren())if(!isShared(c))performVisibility[c]=c->isVisible();}
        else if(!nextSpecial){for(const auto& [c,visible]:performVisibility)if(c->getParentComponent()==&workspaceRoot())c->setVisible(visible);performVisibility.clear();}
        learningLayout=nextLearning;exportLayout=nextExport;resized();
    }
    else if(loopLayoutChanged)resized();
    refreshLoopPlayback(loopPerformLayout,state.session.liveEnabled,capture.manifest.sampleRate,diagnostics);
    if(!nextSpecial){timeline.setData(capture,state,time,loopSource?time.valid:diagnostics.samplePositionAvailable,!loopSource && diagnostics.isPlaying);
        timeline.setSongPlayheadAvailable(songPlayheadAvailable);
        if(leavingLearn && performViewport)timeline.setViewport(*performViewport);hasPerformWorkspace=true;}
    const auto addPlayhead=timeline.transportPlayhead();
    const bool addAtViewStart=loopPerformLayout && (!addPlayhead || *addPlayhead<0);
    addButton.setTitle(addAtViewStart?"Add at view start":"Add at playhead");
    addButton.setTooltip(loopPerformLayout
        ? (addAtViewStart?"Add a swell at the start of the visible loop. Double-click the waveform to choose a different point."
                         :"Add a swell at the loop playhead. Double-click the waveform to choose a different point.")
        : "Move FL Studio's playhead to captured audio, or double-click the source waveform to add a target.");
    hostInfo.display(loopPerformLayout ? m3::ui::loopTransportPresentation(time,addPlayhead)
        : m3::ui::transportPresentation(hostTime,diagnostics.samplePositionAvailable,
            diagnostics.tempoAvailable,diagnostics.bpm,diagnostics.meterAvailable,diagnostics.numerator,diagnostics.denominator));
    liveButton.setToggleState(state.session.liveEnabled,juce::dontSendNotification);
    liveRoutingBox.setSelectedId(state.session.liveRouting==m3::LiveRouting::wetOnly?2:1,juce::dontSendNotification);
    gainPolicyBox.setSelectedId(state.session.gainPolicy==m3::GainPolicy::musical?(state.session.gainRevision==m3::approvedGainRevision?2:3):1,juce::dontSendNotification);
    liveLevel.setRange(-60,state.session.gainPolicy==m3::GainPolicy::musical?12:0,.1);
    if(!liveLevel.isMouseButtonDown() && !liveLevel.hasKeyboardFocus(true))liveLevel.setValue(state.session.liveWetDb,juce::dontSendNotification);
    const auto liveText=(state.liveStatus.startsWith("Export incomplete") || state.liveStatus.startsWith("Export warning"))?state.liveStatus:
        !state.session.liveEnabled?juce::String("Live off | dry passthrough"):
        state.liveIncompatible?juce::String("Live: awaiting compatible timing/preparation"):
        state.liveMissing?juce::String("Live: missing prepared coverage | ")+state.liveStatus:
        (diagnostics.isPlaying?juce::String("Live Song | "):juce::String("Stopped | "))+state.liveStatus+" | guard "+juce::String(state.liveAttenuationDb,1)+" dB";
    helper.setText(liveText,juce::dontSendNotification);helper.setTooltip(liveText);
    const auto musical=state.session.gainPolicy==m3::GainPolicy::musical;
    const auto protection=musical && state.liveReady && state.liveAttenuationDb<-.01f?juce::String(" | ceiling-constrained"):juce::String{};
    helper.setText(liveText+protection,juce::dontSendNotification);
    helper.setTooltip(liveText+protection+(musical?" | Musical source-relative calibration; wet ceiling -1 dBFS":" | Legacy wet ceiling -6 dBFS"));
    songOverview.repaint();
    const auto w=m3::find(state.session,state.session.primary);
    juce::String timingText="Target: --    |    Pre-roll: --    |    Length: --";
    if(w && state.raw && time.rate>0)
    {int64_t a,b;if(m3::requestedShapeBounds(*w,state.raw->start,a,b))
        timingText="Target: "+m3::musicalPosition(w->arrival,time)+"    |    Pre-roll: "+juce::String(static_cast<double>((static_cast<long double>(w->arrival)-a)/time.rate),2)
            +" s    |    Length: "+juce::String(static_cast<double>((static_cast<long double>(b)-a)/time.rate),2)+" s";}
    editorReadout.setText(timingText,juce::dontSendNotification);editorReadout.setTooltip(timingText);
    listEmpty.setVisible(state.session.swells.empty());inspectorEmpty.setVisible(w==nullptr);inspectorViewport.setVisible(w!=nullptr);
    for(auto* child:inspectorContent.getChildren())child->setVisible(w!=nullptr);
    for(int field:{sourceA,sourceB,audibleA}){fields[field].setVisible(w && advancedTiming);fieldLabels[field].setVisible(w && advancedTiming);}
    refreshValues(w,!nextSpecial?timeline.acceptedGridTiming():time);
    listLabel.setText("Swells",juce::dontSendNotification);listCount.setText("("+juce::String(static_cast<int>(state.session.swells.size()))+")",juce::dontSendNotification);
    if(!learningLayout && !exportLayout && state.sessionEpoch!=navigationEpoch)
    {
        navigationEpoch=state.sessionEpoch;performFitted=false;navigationDirty=false;performViewport.reset();navigationInvocation.reset();
        const auto preferences=m3::normalizedPerformNavigation(state.session.performUi);timeline.setSnap(preferences.snap);timeline.setGridDivision(preferences.grid);
        if(preferences.hasView){auto view=m3::SwellViewport{preferences.first,preferences.last};view.constrain(timeline.navigationBounds(),timeline.minimumViewSpan());timeline.setViewport(view);performFitted=true;}
    }
    if(!learningLayout && !exportLayout && initialFrame)
    {
        timeline.revealInitialSelection();performViewport=timeline.viewport();performFitted=true;navigationDirty=true;
    }
    if(!learningLayout && !exportLayout && !performFitted && (!capture.manifest.ranges.empty() || !state.session.swells.empty()))
    {timeline.fitSelection();performFitted=true;}
    if(!learningLayout && !exportLayout)
    {
        std::vector<m3::ui::controls::SwellListRow> rows;rows.reserve(state.session.swells.size());
        for(const auto& swell:state.session.swells)
            rows.push_back({swell.id,swell.name,swell.irName,swell.phase==m3::Phase::ready?phaseName(swell.phase):swell.status,
                swell.arrival,swell.enabled,swell.muted,swell.solo,swell.frozen,swell.phase==m3::Phase::error});
        list.display(std::move(rows),state.session.selected,state.session.primary,state.sessionEpoch);
    }
    enabledButton.setToggleState(w && w->enabled,juce::dontSendNotification);muteButton.setToggleState(w && w->muted,juce::dontSendNotification);
    soloButton.setToggleState(w && w->solo,juce::dontSendNotification);freezeButton.setToggleState(w && w->frozen,juce::dontSendNotification);
    if(w){inCurveBox.setSelectedId(static_cast<int>(w->shape.inCurve)+1,juce::dontSendNotification);outCurveBox.setSelectedId(static_cast<int>(w->shape.outCurve)+1,juce::dontSendNotification);}
    if(!markerKnob.isMouseButtonDown() && !markerKnob.hasKeyboardFocus(true))markerKnob.setValue(state.session.sensitivity*100,juce::dontSendNotification);
    if(!regenerationKnob.isMouseButtonDown() && !regenerationKnob.hasKeyboardFocus(true))regenerationKnob.setValue(state.session.regenerationSensitivity*100,juce::dontSendNotification);
    markerButton.setToggleState(state.session.autoMarkersEnabled,juce::dontSendNotification);
    markerState.setText(!state.session.autoMarkersEnabled?"Off":!state.session.automaticProcessing?"Master paused":state.analyzing?"Analysing":"On",juce::dontSendNotification);
    isolateDryButton.setToggleState(state.session.isolateDry,juce::dontSendNotification);isolateDryButton.setEnabled(state.session.auditionMode==m3::AuditionMode::dryWet);
    isolateDryButton.setUnavailableReason("Available during dry + wet audition. The dry isolation setting is retained.");
    isolateButton.setToggleState(state.session.isolateSelected,juce::dontSendNotification);
    if(!tailSlider.isMouseButtonDown() && !tailSlider.hasKeyboardFocus(true))tailSlider.setValue(state.session.postRollSeconds,juce::dontSendNotification);
    if(!auditionSlider.isMouseButtonDown() && !auditionSlider.hasKeyboardFocus(true))auditionSlider.setValue(state.session.auditionDb,juce::dontSendNotification);
    if(!separationSlider.isMouseButtonDown())separationSlider.setValue(state.session.separationSeconds*1000,juce::dontSendNotification);
    autoButton.setToggleState(state.session.autoRegenerate,juce::dontSendNotification);
    automaticButton.setToggleState(state.session.automaticProcessing,juce::dontSendNotification);
    const bool wetOnly=state.session.auditionMode==m3::AuditionMode::wetOnly;
    auditionWetOnly.display(wetOnly);auditionWetOnly.setButtonText(wetOnly?"Wet only":"Dry + wet");
    auditionWetOnly.setTitle(wetOnly?"Audition mode: Wet only":"Audition mode: Dry + wet");
    auditionWetOnly.setTooltip(wetOnly?"Selected: Wet only. Click to include captured dry at live levels.":"Selected: Dry + wet. Click to hear wet only. Wet uses Live Wet gain; safety protection remains active.");
    auditionSlider.setEnabled(!state.session.auditionFollowsLive);
    auditionSlider.setTooltip(state.session.auditionFollowsLive?"Stored legacy trim is inactive: audition follows Live Wet gain. Reset to 0 dB by double-click in legacy monitoring.":"Legacy monitoring trim, -60 to 0 dB. Double-click resets to 0 dB.");
    selectedStatus.setText(w?phaseName(w->phase)+" | "+w->name:"Select a swell",juce::dontSendNotification);
    selectedStatus.setTooltip(w?w->status:state.message);
    if(w && musical){const auto& g=w->calibration;const auto calibrationText=g.state==m3::CalibrationState::unavailable?
            juce::String("Calibration unavailable"):"Calibration "+juce::String(g.baselineDb,1)+" dB"+(g.state==m3::CalibrationState::capped?" (capped)":"");
        selectedStatus.setTooltip(w->status+"\n"+calibrationText+" | Requested Level "+juce::String(w->shape.gainDb,1)+" dB");
        if(w->phase==m3::Phase::ready && (g.state==m3::CalibrationState::unavailable || g.state==m3::CalibrationState::capped))
            selectedStatus.setText(phaseName(w->phase)+" | "+(g.state==m3::CalibrationState::unavailable?"Calibration unavailable":"Calibration capped"),juce::dontSendNotification);}
    selectedStatus.setTooltip(selectedStatus.getText()+"\n"+selectedStatus.getTooltip());
    irBinding.displaySelected(state);
    if(responsivePresentation && !learningLayout && !exportLayout && irEditor.getHeight()!=irEditor.preferredHeight())layoutResponsive();
    resetButton.setEnabled(w && time.valid && capture.manifest.sampleRate==time.rate);resetMenu.setEnabled(w && time.valid);
    const std::array<juce::String,5> timingLabels{"Arrival","Source start","Source end","Wet start","Wet end"};
    for(int field=arrival;field<=audibleB;++field)
    {
        const auto label=timingLabels[static_cast<size_t>(field-arrival)];
        const bool samples=sampleUnitsButton.getToggleState();
        fieldLabels[field].setText(label+(samples?" sample":""),juce::dontSendNotification);
        fields[field].setTitle(label+(samples?" (sample)":" (bar.beat.fraction)"));
    }
    juce::String aggregate;
    int ready=0,updatingCount=0,failed=0,waiting=0;
    for(const auto& item:state.session.swells){if(item.phase==m3::Phase::ready)++ready;else if(item.phase==m3::Phase::error)++failed;else if(item.phase==m3::Phase::stale || item.phase==m3::Phase::empty)++waiting;else ++updatingCount;}
    aggregate=diagnostics.isPlaying && processor.isMappingEnabled()?"Mapping":"Last captured";
    if(!time.valid)aggregate="Waiting for song position";
    else if(capture.unavailable || capture.status.startsWith("Stale"))aggregate="Mapping paused: "+capture.status;
    else if(state.defaultIRError.isNotEmpty())aggregate="Missing default IR | Settings > Processing";
    aggregate<<" | "<<ready<<" ready";if(updatingCount)aggregate<<" | "<<updatingCount<<" updating";if(waiting)aggregate<<" | "<<waiting<<" waiting";if(failed)aggregate<<" | "<<failed<<" need attention";
    if(!state.session.automaticProcessing)aggregate<<" | Manual processing";
    if((state.liveStatus.startsWith("Export incomplete") || state.liveStatus.startsWith("Export warning")))aggregate=state.liveStatus;
    if(state.liveStatus.startsWith("Export warning"))aggregate=state.liveStatus.upToFirstOccurrenceOf(" | ",false,false);
    if(processor.isTestToneEnabled())aggregate<<" | 440 Hz test tone armed (Settings > Recovery to turn off)";
    if(!state.session.liveEnabled)aggregate<<" | Live off";
    else if(state.session.liveRouting==m3::LiveRouting::wetOnly)aggregate<<" | Live Wet Only";
    if(state.session.liveWetDb!=0)aggregate<<" | Live wet "<<juce::String(state.session.liveWetDb,1)<<" dB";
    if(musical && state.liveReady && state.liveAttenuationDb<-.01f)aggregate<<" | Wet protection "<<juce::String(state.liveAttenuationDb,1)<<" dB";
    if(state.actionError.isNotEmpty() && !state.liveStatus.startsWith("Export"))aggregate=state.actionError;
    headerStatus.setText(aggregate,juce::dontSendNotification);headerStatus.setTooltip(aggregate+"\n"+((state.liveStatus.startsWith("Export incomplete") || state.liveStatus.startsWith("Export warning"))?state.liveStatus:state.message));
    const auto source=std::pair{capture.reference,capture.manifest.revision};
    if(onsetSource!=source){candidateBox.hidePopup();onsetSource=source;}
    const auto viewport=timeline.viewport();
    candidateBox.display(state.candidates,viewport.first,viewport.last,candidateBox.selectedSample());
    confirmButton.setEnabled(candidateBox.selectedSample().has_value());analyzeButton.setEnabled(!capture.manifest.ranges.empty() && !state.analyzing);
    const auto hasSelection=!state.session.selected.empty();bool selectedIR=false,selectedImport=false;
    for(const auto& swell:state.session.swells)
    {
        if(std::find(state.session.selected.begin(),state.session.selected.end(),swell.id)!=state.session.selected.end())
        {selectedIR=selectedIR || swell.ir.present();selectedImport=selectedImport || swell.status=="Importing IR";}
    }
    removeButton.setEnabled(hasSelection);generateButton.setEnabled(hasSelection && selectedIR && !selectedImport);allButton.setEnabled(true);
    fullButton.setEnabled(w && state.raw);
    freezeButton.setEnabled(w && (w->frozen || w->phase==m3::Phase::ready));
    freezeButton.setUnavailableReason(w?"Wait for a ready render before freezing.":"Select a swell to freeze its render.");
    for(auto* control:std::initializer_list<m3::ui::controls::BooleanToggle*>{&enabledButton,&soloButton,&muteButton})
        control->setUnavailableReason("Select a swell to change this setting.");
    for(auto* control:std::array<juce::Component*,6>{&enabledButton,&muteButton,&soloButton,&inCurveBox,&outCurveBox,&setBarsButton})control->setEnabled(w!=nullptr);
    if(w && w->frozen){for(int field:{arrival,sourceA,sourceB})fields[field].setEnabled(false);setBarsButton.setEnabled(false);}
    refreshSourceRangeFeedback(w);
    timingButton.setEnabled(w!=nullptr);
    mappingButton.setToggleState(processor.isMappingEnabled(),juce::dontSendNotification);toneButton.setToggleState(processor.isTestToneEnabled(),juce::dontSendNotification);
    toneStartSlider.setValue(processor.getToneStartBar(),juce::dontSendNotification);toneEndSlider.setValue(processor.getToneEndBar(),juce::dontSendNotification);
    const auto now=juce::Time::getMillisecondCounterHiRes();
    if(diagnosticsOpen && (diagnosticsRefreshedAt<0 || now-diagnosticsRefreshedAt>=200))
    {
    const auto timingEvidence=(mode==workflow::Mode::live?m3::ui::gridDiagnosticsText(timeline.gridEvidence())
        :juce::String("Fixed grid: not evaluated outside Perform\n"))+"\n"+m3::ui::liveDiagnosticsText(state);
    liveDiagnostic.setText(timingEvidence,juce::dontSendNotification);liveDiagnostic.setDescription(timingEvidence);
    juce::String diagnostic="Transport: ";diagnostic<<(!diagnostics.positionAvailable?"unavailable":diagnostics.isPlaying?"playing":"stopped")<<" | sample "<<(diagnostics.samplePositionAvailable?juce::String(diagnostics.samplePosition):"unavailable")<<" | PPQ "<<(diagnostics.ppqPositionAvailable?juce::String(diagnostics.ppqPosition,3):"unavailable")
        <<" | "<<(diagnostics.tempoAvailable?juce::String(diagnostics.bpm,2):"unavailable")<<" BPM | "<<(diagnostics.meterAvailable?juce::String(diagnostics.numerator)+"/"+juce::String(diagnostics.denominator):"meter unavailable")<<" | "<<juce::String(diagnostics.sampleRate,0)<<" Hz | block "<<diagnostics.blockSize;
    status.setText("Timing\n"+diagnostic,juce::dontSendNotification);
    juce::String captured="Capture / assets\n";captured<<capture.status<<" | revision "<<juce::String(static_cast<juce::int64>(capture.manifest.revision))<<" | dropped "<<juce::String(static_cast<juce::int64>(processor.getDroppedCaptureBlockCount()))<<" | discontinuities "<<juce::String(static_cast<juce::int64>(processor.getTimelineDiscontinuityCount()));
    if(capture.failure)captured<<"\n"<<m3::ui::captureFailureText(capture.failure,capture.failure.event.pass==processor.getWorkflowSnapshot().pass
        && (processor.getWorkflowSnapshot().locked() || processor.getWorkflowSnapshot().phase==workflow::Phase::failed));
    if(learningLayout)captured<<"\nLearn inspection\n"<<learnWorkspace.diagnosticDetails();
    if(const auto omitted=processor.getOmittedCaptureFailureCount())
        captured<<"\nDiagnostic capacity: "<<juce::String(static_cast<juce::int64>(omitted))<<" failure events omitted";
    captureDiagnostic.setText(captured,juce::dontSendNotification);captureDiagnostic.setTooltip(captured);
    layoutLiveDiagnostics();
    playbackDiagnostic.setText("Playback / export\n"+liveText+"\nOffline blocks: "+juce::String(static_cast<juce::int64>(state.offlineBlocks))+" | "+state.offlineStatus,juce::dontSendNotification);
    policyDiagnostic.setText("Processing / gain\n"+juce::String(musical?"Musical":"Legacy")+" | "+state.session.gainRevision
        +"\nLive requested "+juce::String(state.session.liveWetDb,1)+" dB | protection "+(state.liveReady?juce::String(state.liveAttenuationDb,1)+" dB":"pending preparation")+protection
        +"\nMarker "+juce::String(state.session.sensitivity*100,1)+"% | Regeneration "+juce::String(state.session.regenerationSensitivity*100,1)+"%",juce::dontSendNotification);
    juce::String text="Selection / render\n"+state.message;
    if(w)
    {
        text="Selection / render\n"+w->name+": "+(w->phase==m3::Phase::ready && !state.shaped?juce::String("Loading / preparing edited preview"):w->status);
        if(w->phase==m3::Phase::rendering)text<<" ("<<juce::String(w->progress*100,0)<<"%)";
        text<<"\nIR: "<<(w->ir.present()?w->irName:juce::String("not assigned"))<<" | source ["<<juce::String(w->sourceStart)<<", "<<juce::String(w->sourceEnd)<<")";
        if(musical){const auto& g=w->calibration;text<<"\nRequested Level "<<juce::String(w->shape.gainDb,1)<<" dB | Calibration ";
            if(g.state==m3::CalibrationState::unavailable)text<<"unavailable (no automatic boost)";
            else text<<juce::String(g.baselineDb,2)<<" dB"<<(g.state==m3::CalibrationState::capped?" (capped)":"");
            if(m3::isEnergyRevision(g.revision))text<<" | RMS dry "<<peakText(g.dryRms)<<" / wet "<<peakText(g.wetRms)
                <<" | window/hop "<<juce::String(g.windowFrames)<<"/"<<juce::String(g.hopFrames)<<" frames";}
        if(state.raw)text<<" | full wet ["<<juce::String(state.raw->start)<<", "<<juce::String(state.raw->end)<<") | "<<juce::String(w->render.frames)<<" frames";
        if(state.raw && state.shaped)text<<"\nRaw peak "<<peakText(state.raw->peak)<<" | shaped "<<peakText(state.shaped->peak)<<" | audition "<<peakText(state.monitorPeak)<<" (aggregate wet guard "<<juce::String(state.attenuationDb,1)<<" dB; sum guard "<<juce::String(state.blendAttenuationDb,1)<<" dB)";
    }
    if(w && state.message.isNotEmpty())text<<"\n"<<state.message;
    detail.setText(text,juce::dontSendNotification);detail.setTooltip(text);
    const auto position=[&](int64_t sample){return sampleUnitsButton.getToggleState()?juce::String(sample):m3::musicalPosition(sample,time);};
    std::array<juce::String,fieldCount> exactValues;
    juce::String placement;
    if(w)
    {
        exactValues[name]=w->name;
        exactValues[arrival]=position(w->arrival);
        exactValues[sourceA]=position(w->sourceStart);
        exactValues[sourceB]=position(w->sourceEnd);
        int64_t a,b;
        if(state.raw && m3::requestedShapeBounds(*w,state.raw->start,a,b))
        {
            exactValues[audibleA]=position(a);
            exactValues[audibleB]=position(b);
        }
        if(time.rate>0)
        {
            exactValues[fadeIn]=juce::String(w->shape.fadeIn*1000.0/time.rate,3);
            exactValues[fadeOut]=juce::String(w->shape.fadeOut*1000.0/time.rate,3);
            placement=juce::String(w->placement*1000.0/time.rate,3);
        }
    }
    // Do not disturb a selection being copied while the readout is unchanged.
    for(size_t i=0;i<fields.size();++i)
        if(fields[i].getText()!=exactValues[i])fields[i].setText(exactValues[i],false);
    if(placementEditor.getText()!=placement)placementEditor.setText(placement,false);
    // Measure the refresh interval from completion. Slow native text layout
    // must leave a quiet interval for input instead of making the timer due
    // again before its previous callback has returned.
    diagnosticsRefreshedAt=juce::Time::getMillisecondCounterHiRes();

    }
    const auto arm=processor.getRecaptureSnapshot();cancelButton.setEnabled(arm.active);recaptureButton.setEnabled(hasSelection && !arm.active);
    juce::String recapture=arm.status;
    if(arm.active)
    {
        recapture="Range ["+juce::String(arm.start)+", "+juce::String(arm.end)+"): fresh "+juce::String(arm.covered)+" / "+juce::String(arm.end-arm.start)+" samples. ";
        if(time.valid){const auto barLength=time.rate*60/time.bpm*time.numerator*4/time.denominator,origin=time.sample-time.ppq*time.rate*60/time.bpm;recapture<<"Move playhead to bar "<<juce::String(1+(arm.start-origin)/barLength,3)<<" and press Play.";}
        else recapture<<arm.status;
    }
    recaptureStatus.setText(recapture,juce::dontSendNotification);recaptureStatus.setTooltip(recapture);
    const auto flow=processor.getWorkflowSnapshot();const auto workflowReady=processor.getWorkflowReadiness();
    const auto caps=workflow::capabilities(flow,workflowReady);
    const auto loopStatus=loopSettings.enabled?processor.loopCaptureStatus():juce::String{};
    workflowHeader.display(m3::ui::presentWorkflow(flow,workflowReady),loopSettings.enabled?loopStatus:capture.status);
    if(loopPerformLayout)
    {
        juce::String accepted="Accepted Loop";
        if(const auto& phrase=capture.manifest.loop; phrase && phrase->meterNumerator>0)
        {
            const auto wholeBars=phrase->beats%phrase->meterNumerator==0;
            const auto count=wholeBars?phrase->beats/phrase->meterNumerator:phrase->beats;
            accepted<<" | "<<count<<(wholeBars?(count==1?" bar":" bars"):(count==1?" beat":" beats"));
        }
        workflowHeader.modeInformation().display(accepted);
    }
    noticeBar.display(m3::ui::workspaceNotice(flow,capture,state,irPreparationNotice.update(state,
        processor.swellEngine().assignmentEpoch(),juce::Time::getMillisecondCounterHiRes()),loopStatus,songTimingIssue));
    recaptureExisting.setToggleState(flow.recapture,juce::dontSendNotification);recaptureExisting.setEnabled(flow.mode==workflow::Mode::capture && !flow.locked());
    recaptureExisting.setTooltip("Off fills unmapped positions only. On replaces encountered positions as one pass committed on DAW Stop. Independent of Automatic Swells.");
    recaptureButton.setEnabled(flow.mode==workflow::Mode::capture && !flow.locked());cancelButton.setEnabled(flow.mode==workflow::Mode::capture && (flow.locked() || flow.recapture));
    recaptureStatus.setText(flow.recapture?"Recapture armed for encountered positions; commit on DAW Stop":"Recapture off; mapped source protected",juce::dontSendNotification);
    mappingButton.setEnabled(!flow.locked());mappingButton.setTooltip("Legacy control requests Learn or Perform; workflow guards always apply.");
    mappingButton.setUnavailableReason("Wait for the current capture pass to finish.");
    clearButton.setEnabled(flow.mode==workflow::Mode::capture && !flow.locked());
    regenerationKnob.setEnabled(false);regenerationKnob.setTooltip("Saved for compatibility. Source replacement requires Learn with explicit Recapture.");
    const bool edit=caps.edit;timeline.setEditingEnabled(edit);
    if(!edit)for(auto& control:valueControls)if(control.editing())control.cancel();
    inspectorContent.setEnabled(edit);addButton.setEnabled(edit);removeButton.setEnabled(edit && hasSelection);
    for(auto* button:{&generateButton,&allButton,&analyzeButton,&confirmButton,&timingButton})button->setEnabled(edit && button->isEnabled());
    using LoopState=m3::LoopCueScheduler::State;
    const auto loopPlayback=processor.loopPlaybackStatus().playback;
    auditionWetOnly.setEnabled(flow.mode==workflow::Mode::live && !flow.locked());
    const auto capturedRate=capture.manifest.sampleRate;
    const bool auditionRateMismatch=loopSource && capturedRate>0 && diagnostics.sampleRate>0
        && capturedRate!=diagnostics.sampleRate;
    juce::String unavailable, auditionHelp;
    if(!w)unavailable="Select a swell to audition";
    else if(auditionRateMismatch)
    {
        const auto requiredRate=sampleRateText(capturedRate);
        unavailable="Audition needs "+requiredRate;
        auditionHelp="Set the host sample rate to "+requiredRate+" to audition this Loop.";
    }
    else if(flow.mode!=workflow::Mode::live)unavailable="Open Perform to audition";
    else if(loopSource && loopPlayback==LoopState::queued)unavailable="Cancel the queued Loop to audition";
    else if(loopSource && loopPlayback==LoopState::stopping)unavailable="Wait for Loop playback to stop";
    else if(loopSource && loopPlayback!=LoopState::standby)unavailable="Stop Loop to audition";
    else if(!loopSource && state.session.liveEnabled && diagnostics.isPlaying)
        unavailable="Stop DAW playback to audition";
    else if(!caps.preview)
        unavailable=workflowReady.compatible?"Capture audio in Learn to audition":"Restore compatible timing to audition";
    else if(!state.auditionReady)
    {
        if(state.auditionPreparing && w->phase==m3::Phase::ready)unavailable="Preparing audition";
        else if(state.dryStatus.isNotEmpty())unavailable=state.dryStatus.contains("Dry source not captured")?
        "Unavailable: capture the dry listening window in Learn":"Unavailable: "+state.dryStatus;
        else if(w->phase==m3::Phase::empty || w->phase==m3::Phase::error || w->phase==m3::Phase::stale)
            unavailable="Unavailable: "+w->status;
        else unavailable="Preparing audition";
    }
    else if(juce::Time::getMillisecondCounterHiRes()<auditionRetryAfter)unavailable="Audition couldn't start. Try again shortly.";
    auditionButton.setEnabled(unavailable.isEmpty());
    if(auditionHelp.isEmpty())auditionHelp=unavailable.isEmpty()?"Loop the selected audition window until Stop.":unavailable;
    auditionButton.setTooltip(auditionHelp);auditionButton.setHelpText(auditionHelp);
    stopButton.setEnabled(caps.preview && (state.playing || state.auditionReady));
    juce::String monitoring=unavailable.isNotEmpty()?unavailable:(state.playing?"Looping":"Ready");
    if(w && unavailable.isEmpty()){
        if(state.auditionReady)monitoring<<" | "<<juce::String(static_cast<int>(state.auditionMembers.size()))<<" swell"<<(state.auditionMembers.size()==1?"":"s")<<(state.session.isolateSelected?" (isolated)":"");
        if(state.cachedWetOnly && state.auditionReady)monitoring<<" | cached wet";
        if(!state.session.auditionFollowsLive)monitoring<<" | saved balance";
        if(state.attenuationDb<0)monitoring<<" | wet protection "<<juce::String(state.attenuationDb,1)<<" dB";
        if(state.blendAttenuationDb<0)monitoring<<" | mix protection "<<juce::String(state.blendAttenuationDb,1)<<" dB";}
    monitorStatus.setText(monitoring,juce::dontSendNotification);
    monitorStatus.setTooltip(monitoring+"\nSelected mode: "+(wetOnly?"Wet only":"Dry + wet")+"\nManual audition loops captured audio until Stop; DAW transport is independent.\n"+(state.session.auditionFollowsLive?"Unity captured dry; Live Wet "+juce::String(state.session.liveWetDb,1)+" dB. Legacy trim is inactive.":"Legacy saved wet blend "+juce::String(state.session.wetBlend*100,1)+"%; trim "+juce::String(state.session.auditionDb,1)+" dB. Toggle Wet only to adopt live levels.")+"\n2 ms wet edges / 5 ms replay edges; wet and combined-mix protection can reduce audition level.\n"+state.dryStatus);
    if(flow.mode==workflow::Mode::capture)monitorStatus.setText("Learn | dry passthrough | editing and audition available in Perform",juce::dontSendNotification);
    if(flow.mode==workflow::Mode::finalise)monitorStatus.setText("Export | DAW wet bypassed | explicit preview only | editing locked",juce::dontSendNotification);
    automaticButton.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());markerButton.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());markerKnob.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());separationSlider.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());
    automaticButton.setUnavailableReason(flow.locked()?"Wait for the current capture pass to finish.":"Available in Learn or Perform.");
    gainPolicyBox.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());defaultIRButton.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());suppressionButton.setEnabled(edit);
    toneButton.setEnabled(flow.mode==workflow::Mode::live);recaptureButton.setButtonText("Arm Recapture pass");
    toneButton.setUnavailableReason("The test tone is available in Perform.");
    if(learningLayout)
    {
        learnWorkspace.displayLoop(loopSettings,processor.canArmLoopCapture(),
            processor.loopCaptureArmed(),flow.locked(),processor.loopFinishing(),
            processor.loopRecaptureEnabled(),loopStatus);
        learnWorkspace.display(capture,state,time,flow,songPlayheadAvailable,processor.getCaptureProgress(),songTimingIssue.has_value());
    }
    if(exportLayout){
        if(loopSource && capture.manifest.loop){
            const auto& loop=*capture.manifest.loop;const auto options=state.session.exportState.loop.options;
            const auto bar=capture.manifest.sampleRate*240*loop.meterNumerator/(loop.tempoBpm*loop.meterDenominator);
            exportWorkspace.setLoopPresentation(m3::ui::ExportWorkspace::LoopPresentation{{options.repeats,options.gridBeats},static_cast<double>(loop.endSample),bar,loop.meterNumerator});
        }else exportWorkspace.setLoopPresentation({});
        exportWorkspace.setPreviewDeviceRate(diagnostics.sampleRate);
        exportWorkspace.display(state);noticeBar.display(exportWorkspace.notice());
    }
    if(learningLayout || exportLayout)for(auto* c:workspaceRoot().getChildren())if(!isShared(c))c->setVisible(false);
    learnWorkspace.setVisible(learningLayout);exportWorkspace.setVisible(exportLayout);
    refreshPerformNavigation();
    if(hoverHelpSource && !hoverHelpSource->isShowing())hoverHelpSource=nullptr;
    refreshHoverHelp();
    if(valueFeedbackChanged && !learningLayout && !exportLayout)
    {
        valueFeedbackChanged=false;
        if(responsivePresentation)layoutResponsiveValues();
        else resized();
    }
    if(responsivePresentation)refreshResponsiveSections();
    updating=false;if(initialFrame)publishPerformNavigation();
}
bool PreverbAudioProcessorEditor::navigationIntentCurrent()const
{
    const auto flow=processor.getWorkflowSnapshot();
    return !destroying && !updating && navigationInvocation && flow.mode==workflow::Mode::live
        && navigationInvocation->epoch==processor.swellEngine().assignmentEpoch()
        && navigationInvocation->pass==flow.pass && navigationInvocation->primary==processor.swellEngine().savedSession().primary;
}
void PreverbAudioProcessorEditor::refreshPerformNavigation()
{
    const auto fraction=timeline.zoomFraction();const bool active=!learningLayout && !exportLayout;
    performZoom.setEnabled(active);performZoom.display({fraction,fraction<1-1.e-9,fraction>1.e-9,hasPerformWorkspace});
    snapButton.setEnabled(active);snapButton.display(timeline.effectiveSnap());
    snapButton.setTooltip("Snap timing to the selected grid. Hold Ctrl to bypass; Shift gives fine drag. Ctrl+Shift combines both.");
    gridBox.setEnabled(active);
    for(size_t n=0;n<m3::gridChoices.size();++n)if(m3::gridChoices[n].division==timeline.selectedGrid())gridBox.setSelectedId(static_cast<int>(n)+1,juce::dontSendNotification);
}
void PreverbAudioProcessorEditor::updateHoverHelp(const juce::MouseEvent& event)
{
    if(destroying || !event.eventComponent)return;
    if(event.eventComponent==this && hoverHelp.getBounds().contains(event.position.toInt()))return;
    hoverHelpSource=event.eventComponent;refreshHoverHelp();
}
void PreverbAudioProcessorEditor::refreshHoverHelp()
{
    juce::String message,fallback;
    for(auto* component=hoverHelpSource.getComponent();component && component!=this;component=component->getParentComponent())
    {
        message=component->getProperties()["contextHelp"].toString();if(message.isNotEmpty())break;
        if(auto* client=dynamic_cast<juce::TooltipClient*>(component)){message=client->getTooltip();if(message.isNotEmpty())break;}
        if(fallback.isEmpty() && (dynamic_cast<juce::Button*>(component) || dynamic_cast<juce::Slider*>(component) || dynamic_cast<juce::ComboBox*>(component) || dynamic_cast<juce::TextEditor*>(component)))
        {
            fallback=component->getTitle();if(fallback.isEmpty())if(auto* button=dynamic_cast<juce::Button*>(component))fallback=button->getButtonText();
            if(fallback.isEmpty())fallback=component->getName();
        }
    }
    if(message.isEmpty())message=fallback;
    message=message.replace("\r"," ").replace("\n"," ").trim();
    hoverHelp.setText(message,juce::dontSendNotification);hoverHelp.setDescription(message);
}
void PreverbAudioProcessorEditor::mouseExit(const juce::MouseEvent& event)
{
    if(!destroying && !hoverHelp.getBounds().contains(getLocalPoint(event.eventComponent,event.position).toInt()))
    {hoverHelpSource=nullptr;hoverHelp.setText({},juce::dontSendNotification);hoverHelp.setDescription({});}
}
void PreverbAudioProcessorEditor::modifierKeysChanged(const juce::ModifierKeys& modifiers)
{
    valueModifierFlags=modifiers.getRawFlags();
    timeline.setGestureModifiers(modifiers.isShiftDown(),modifiers.isCtrlDown());
    for(auto& cell:valueControls)cell.setGestureModifiers(modifiers);
    snapButton.display(timeline.effectiveSnap());
    snapButton.setDescription(timeline.requestedSnap() && !timeline.effectiveSnap()?"Snap temporarily off while Ctrl is held; release Ctrl to restore":"Snap requested state; hold Ctrl for temporary bypass and Shift for fine drag");
    if(timeline.effectiveSnap() && !timeline.gridAvailable())snapButton.setDescription("Snap requested On, but compatible fixed host timing is unavailable. Hold Ctrl or turn Snap off for sample edits.");
}
void PreverbAudioProcessorEditor::publishPerformNavigation()
{
    if(destroying || updating || !navigationDirty || learningLayout || exportLayout)return;
    const auto view=timeline.viewport();m3::PerformNavigationState preferences;
    preferences.hasView=performFitted;preferences.first=view.first;preferences.last=view.last;
    preferences.snap=timeline.requestedSnap();preferences.grid=timeline.selectedGrid();
    if(processor.swellEngine().setPerformNavigation(preferences,navigationEpoch))navigationDirty=false;
}
