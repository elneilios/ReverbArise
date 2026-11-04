#include "PluginEditor.h"
#include <UI/MusicalPosition.h>
#include <DSP/DefaultImpulse.h>

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


}
PreverbAudioProcessorEditor::PreverbAudioProcessorEditor(PreverbAudioProcessor& p)
    :AudioProcessorEditor(p),processor(p),learnWorkspace(p.swellEngine(),[&p]{return p.getWorkflowSnapshot();}),exportWorkspace(p.swellEngine()),timeline(p.swellEngine())
{
    setLookAndFeel(&workspaceTheme);
    addChildComponent(learnWorkspace);addChildComponent(exportWorkspace);
    exportWorkspace.onReturn=[this]{processor.requestWorkflowMode(workflow::Mode::live);refresh();};
    learnWorkspace.onRecapture=[this](bool enabled){processor.setRecaptureExisting(enabled);};learnWorkspace.onRefresh=[this]{refresh();};
    addAndMakeVisible(workflowHeader);addAndMakeVisible(recaptureExisting);
    noticeBar.setName("workspaceNotice");noticeBar.setHeaderAppearance(true);addChildComponent(noticeBar);
    recaptureExisting.setName("legacyRecaptureExisting");
    workflowHeader.onModeRequested=[this](workflow::Mode mode){if(!updating && !destroying){processor.requestWorkflowMode(mode);refresh();}};
    recaptureExisting.onClick=[this]{processor.setRecaptureExisting(recaptureExisting.getToggleState());refresh();};
    setResizable(true,true);setResizeLimits(1100,850,2000,1400);
    for(auto* label:{&status,&detail,&listLabel,&auditionLabel,&separationLabel,&spanLabel,&barStartLabel,&barEndLabel,&recaptureStatus,&toneStartLabel,&toneEndLabel})
    {label->setColour(juce::Label::textColourId,m3::ui::text);label->setJustificationType(juce::Justification::centredLeft);addAndMakeVisible(label);}
    status.setFont(juce::FontOptions(m3::ui::smallSize));detail.setFont(juce::FontOptions(m3::ui::smallSize));detail.setJustificationType(juce::Justification::topLeft);
    listLabel.setText("Swells",juce::dontSendNotification);auditionLabel.setText("Audition level",juce::dontSendNotification);
    separationLabel.setText("Min. separation",juce::dontSendNotification);
    spanLabel.setText("Bars",juce::dontSendNotification);
    barStartLabel.setText("Bar start",juce::dontSendNotification);barEndLabel.setText("End excl.",juce::dontSendNotification);
    toneStartLabel.setText("Tone start bar",juce::dontSendNotification);toneEndLabel.setText("End exclusive",juce::dontSendNotification);
    for(auto* button:{&addButton,&removeButton,&loadIRButton,&generateButton,&allButton,&auditionButton,&stopButton,&analyzeButton,&confirmButton,&previousButton,&nextButton,&zoomInButton,&zoomOutButton,&fitButton,&recaptureButton,&cancelButton,&fullButton,&resetButton,&setBarsButton,&clearButton})addAndMakeVisible(button);
    addAndMakeVisible(assetInfoButton);assetInfoButton.setTooltip("Copy the selected swell's managed asset paths for backup or validation.");
    assetInfoButton.onClick=[this]
    {
        const auto directory=processor.swellEngine().assetDirectory();juce::String info="Preverb working assets: "+directory.getFullPathName();
        if(const auto w=m3::find(state.session,state.session.primary))
        {info<<"\nSwell: "<<w->name<<"\nIR: "<<(w->ir.present()?directory.getChildFile(w->ir.path).getFullPathName():juce::String("not assigned"))
            <<"\nRender: "<<(w->render.present()?directory.getChildFile(w->render.path).getFullPathName():juce::String("not generated"));}
        juce::SystemClipboard::copyTextToClipboard(info);
    };
    for(auto* button:std::array<juce::Button*,7>{&snapButton,&autoButton,&enabledButton,&muteButton,&soloButton,&mappingButton,&toneButton})addAndMakeVisible(button);
    for(auto* box:{&candidateBox,&spanBox,&inCurveBox,&outCurveBox})addAndMakeVisible(box);
    spanBox.setName("detailBars");inCurveBox.setName("fadeInCurve");outCurveBox.setName("fadeOutCurve");
    loadIRButton.setName("loadIR");generateButton.setName("generate");auditionButton.setName("audition");stopButton.setName("stopAudition");detail.setName("previewStatus");
    list.setName("swellList");list.setMultipleSelectionEnabled(true);list.setRowHeight(44);list.setColour(juce::ListBox::backgroundColourId,m3::ui::background);
    addAndMakeVisible(list);addAndMakeVisible(timeline);
    addAndMakeVisible(songOverview);timeline.setInlineOverview(false);timeline.onEnvelopeEdited=[this]{refresh();};
    const std::array<juce::String,fieldCount> labels{"Name","Arrival (sample)","Source start (sample)","Source end exclusive","Audible start (sample)","Audible end exclusive","Fade in (ms)","Fade out (ms)"};
    const std::array<juce::String,fieldCount> names{"swellName","arrivalSample","sourceSampleStart","sourceSampleEnd","audibleStart","audibleEnd","fadeInMs","fadeOutMs"};
    for(int i=0;i<fieldCount;++i)
    {
        fieldLabels[i].setText(labels[i],juce::dontSendNotification);fieldLabels[i].setColour(juce::Label::textColourId,m3::ui::secondary);fieldLabels[i].setFont(juce::FontOptions(m3::ui::smallSize));addAndMakeVisible(fieldLabels[i]);
        fields[i].setName(names[i]);fields[i].setInputRestrictions(i==name?256:40,i==name?juce::String{}:(i>=fadeIn?juce::String("0123456789."):juce::String("-0123456789.")));
        fields[i].setFont(m3::ui::fontFor(*this,m3::ui::controlSize));fields[i].setReadOnly(true);addAndMakeVisible(fields[i]);
    }
    barStartEditor.setName("sourceStart");barEndEditor.setName("sourceEnd");barStartEditor.setText("1",false);barEndEditor.setText("2",false);
    for(auto* editor:{&barStartEditor,&barEndEditor,&placementEditor})editor->setFont(m3::ui::fontFor(*this,m3::ui::controlSize));
    for(auto* field:{&barStartEditor,&barEndEditor}){field->setInputRestrictions(12,"0123456789");addAndMakeVisible(field);field->onReturnKey=[this]{applyBarRange();};}
    const auto slider=[&](juce::Slider& control,double a,double b,double step,const juce::String& suffix)
    {control.setRange(a,b,step);control.setSliderStyle(juce::Slider::LinearHorizontal);control.setTextBoxStyle(juce::Slider::TextBoxRight,false,78,22);control.setTextValueSuffix(suffix);addAndMakeVisible(control);};
    slider(auditionSlider,-60,0,0.1," dB");auditionSlider.setName("auditionLevel");
    slider(separationSlider,1,2000,1," ms");slider(toneStartSlider,1,511,1,"");slider(toneEndSlider,2,512,1,"");
    for(int bars:{1,2,4,8,16})spanBox.addItem(juce::String(bars),bars);spanBox.setSelectedId(4,juce::dontSendNotification);
    for(auto* box:{&inCurveBox,&outCurveBox}){box->addItem("Linear",1);box->addItem("Smooth",2);box->addItem("Exponential",3);}
    snapButton.setToggleState(true,juce::dontSendNotification);
    addButton.onClick=[this]{processor.swellEngine().addTarget(processor.getSongTime().sample);refresh();};
    addButton.setTooltip("Move FL Studio's playhead to captured audio, or double-click the source waveform to add a target.");
    removeButton.onClick=[this]{processor.swellEngine().removeSelected();refresh();};
    loadIRButton.onClick=[this]
    {
        const auto w=m3::find(state.session,state.session.primary);if(!w || w->frozen)return;
        const auto id=w->id;const auto generation=w->inputGeneration;const auto epoch=processor.swellEngine().assignmentEpoch();
        chooser=std::make_unique<juce::FileChooser>("Load mono/stereo WAV or WavPack IR",juce::File{},"*.wav;*.wv");
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
            [safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(this),id,generation,epoch](const juce::FileChooser& dialog)
            {if(safe && dialog.getResult()!=juce::File{})safe->processor.swellEngine().assignIR(id,generation,dialog.getResult(),false,epoch);});
    };
    generateButton.onClick=[this]{processor.swellEngine().generate();refresh();};allButton.onClick=[this]{processor.swellEngine().generate(true);refresh();};
    auditionButton.onClick=[this]{processor.swellEngine().audition();};stopButton.onClick=[this]{processor.swellEngine().stop();};
    auditionSlider.onValueChange=[this]{if(!updating)setPreferences();};
    separationSlider.beforeInvocation=[this]{separationPass=processor.getWorkflowSnapshot().pass;};
    separationSlider.onValueChange=[this]{if(!updating){const auto s=processor.swellEngine().savedSession();processor.swellEngine().setPreferences(s.auditionDb,s.sensitivity,separationSlider.getValue()/1000,s.autoRegenerate,separationPass.value_or(processor.getWorkflowSnapshot().pass));}};
    autoButton.beforeInvocation=[this]{refreshPass=processor.getWorkflowSnapshot().pass;};
    autoButton.onClick=[this]{const auto s=processor.swellEngine().savedSession();processor.swellEngine().setPreferences(s.auditionDb,s.sensitivity,s.separationSeconds,autoButton.getToggleState(),refreshPass.value_or(processor.getWorkflowSnapshot().pass));};
    analyzeButton.onClick=[this]
    {
        const auto capture=processor.getCaptureSnapshot();
        if(capture.manifest.ranges.empty()){processor.swellEngine().diagnostic("Capture source before suggesting onsets");return;}
        const auto& last=capture.manifest.ranges.back();processor.swellEngine().analyze(capture.manifest.ranges.front().startSample,last.startSample+static_cast<int64_t>(last.frameCount));
    };
    candidateBox.onChange=[this]{if(updating)return;const auto index=candidateBox.getSelectedId()-1;if(index>=0 && index<static_cast<int>(state.candidates.size()))chosenCandidate=state.candidates[static_cast<size_t>(index)].sample;};
    confirmButton.onClick=[this]{if(chosenCandidate)processor.swellEngine().confirmCandidateAt(*chosenCandidate);refresh();};
    timeline.onCandidateSelected=[this](int index)
    {if(index<0 || index>=static_cast<int>(state.candidates.size()))return;chosenCandidate=state.candidates[static_cast<size_t>(index)].sample;
        if(candidateBox.indexOfItemId(index+1)<0)candidateBox.addItem("Sample "+juce::String(*chosenCandidate),index+1);candidateBox.setSelectedId(index+1,juce::dontSendNotification);};
    previousButton.onClick=[this]{timeline.previous();};nextButton.onClick=[this]{timeline.next();};zoomInButton.onClick=[this]{timeline.zoom(0.5);};zoomOutButton.onClick=[this]{timeline.zoom(2);};fitButton.onClick=[this]{timeline.fitSelection();};
    spanBox.onChange=[this]{if(!updating)timeline.page(spanBox.getSelectedId());};snapButton.onClick=[this]{timeline.setSnap(snapButton.getToggleState());};
    recaptureButton.onClick=[this]{armSelected();};cancelButton.onClick=[this]{processor.cancelRecapture();refresh();};
    fullButton.onClick=[this]{shapeReset(true);};resetButton.onClick=[this]{shapeReset(false);};setBarsButton.onClick=[this]{applyBarRange();};
    inCurveBox.onChange=[this]{curveChanged(true);};outCurveBox.onChange=[this]{curveChanged(false);};
    const auto activity=[this]
    {if(updating)return;processor.swellEngine().edit(state.session.primary,[&](auto& w,auto&){w.enabled=enabledButton.getToggleState();w.muted=muteButton.getToggleState();w.solo=soloButton.getToggleState();return true;});};
    enabledButton.onClick=activity;muteButton.onClick=activity;soloButton.onClick=activity;
    mappingButton.onClick=[this]{processor.setMappingEnabled(mappingButton.getToggleState());};clearButton.onClick=[this]{processor.requestClearCapture();refresh();};
    toneButton.onClick=[this]{processor.setTestToneEnabled(toneButton.getToggleState());};
    toneStartSlider.onValueChange=[this]{if(!updating)processor.setToneStartBar(static_cast<int>(toneStartSlider.getValue()));};
    toneEndSlider.onValueChange=[this]{if(!updating)processor.setToneEndBar(static_cast<int>(toneEndSlider.getValue()));};
    for(auto* label:{&headerStatus,&selectedStatus,&irStatus,&monitorStatus,&helper})
    {addAndMakeVisible(label);label->setColour(juce::Label::textColourId,m3::ui::secondary);}
    headerStatus.setName("processingStatus");selectedStatus.setName("selectedStatus");irStatus.setName("irAssignmentStatus");
    headerStatus.setBorderSize(juce::BorderSize<int>(0,24,0,8));
    helper.setText("Live playback: awaiting prepared Song audio",juce::dontSendNotification);
    liveButton.setName("livePlayback");liveRoutingBox.setName("liveRouting");liveLevel.setName("liveWetLevel");
    liveRoutingBox.addItem("Dry + Wet",1);liveRoutingBox.addItem("Wet Only",2);
    liveLevelLabel.setText("Live wet",juce::dontSendNotification);
    liveLevel.setRange(-60,0,.1);liveLevel.setTextValueSuffix(" dB");liveLevel.setSliderStyle(juce::Slider::LinearHorizontal);
    liveLevel.setTextBoxStyle(juce::Slider::TextBoxRight,false,75,24);
    gainPolicyLabel.setText("Gain staging",juce::dontSendNotification);gainPolicyBox.setName("gainPolicy");
    gainPolicyBox.addItem("Legacy (original projects)",1);gainPolicyBox.addItem("Musical (source energy +6 dB)",2);
    gainPolicyBox.addItem("Saved musical prototype",3);gainPolicyBox.setItemEnabled(3,false);
    gainPolicyBox.onChange=[this]{if(!updating && gainPolicyBox.getSelectedId()!=3){processor.swellEngine().setGainPolicy(gainPolicyBox.getSelectedId()==2?m3::GainPolicy::musical:m3::GainPolicy::legacy);refresh();}};
    addAndMakeVisible(liveButton);addAndMakeVisible(liveRoutingBox);addAndMakeVisible(liveLevel);addAndMakeVisible(liveLevelLabel);
    const auto updateLive=[this]{if(!updating)processor.swellEngine().setLivePlayback(liveButton.getToggleState(),
        liveRoutingBox.getSelectedId()==2?m3::LiveRouting::wetOnly:m3::LiveRouting::dryWet,static_cast<float>(liveLevel.getValue()));};
    liveButton.onClick=updateLive;liveRoutingBox.onChange=updateLive;liveLevel.onValueChange=updateLive;
    helper.setFont(juce::FontOptions(m3::ui::smallSize));
    for(auto* button:{&trimResetButton,&defaultIRButton,&suppressionButton})addAndMakeVisible(button);
    for(auto* button:std::array<juce::Button*,2>{&automaticButton,&sampleUnitsButton})addAndMakeVisible(button);
    sampleUnitsButton.setName("sampleUnits");automaticButton.setName("automaticProcessing");
    trimResetButton.setName("resetAuditionTrim");
    blendSlider.setName("auditionBlend");slider(blendSlider,0,100,.1," % Wet");
    workflowHeader.onSettings=[this]{if(!destroying)setSettingsOpen(!settingsWindow || !settingsWindow->isVisible());};
    automaticButton.beforeInvocation=[this]{automaticPass=processor.getWorkflowSnapshot().pass;};
    automaticButton.onClick=[this]{processor.swellEngine().setAutomaticProcessing(automaticButton.getToggleState(),automaticPass.value_or(processor.getWorkflowSnapshot().pass));refresh();};
    sampleUnitsButton.onClick=[this]{refresh();};
    suppressionButton.onClick=[this]{processor.swellEngine().resetSuppression();refresh();};
    defaultIRButton.onClick=[this]
    {
        const auto epoch=processor.swellEngine().assignmentEpoch();const auto pass=processor.getWorkflowSnapshot().pass;
        chooser=std::make_unique<juce::FileChooser>("Default IR for future swells",juce::File{},"*.wav;*.wv");
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
            [safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(this),epoch,pass](const juce::FileChooser& file)
            {if(safe && file.getResult()!=juce::File{})safe->processor.swellEngine().assignDefaultIR(file.getResult(),epoch,pass);});
    };
    blendSlider.onValueChange=[this]{if(!updating){const auto value=static_cast<float>(blendSlider.getValue()/100);const auto saved=processor.swellEngine().savedSession();processor.swellEngine().setMonitoring(value>=1?m3::AuditionMode::wetOnly:m3::AuditionMode::dryWet,value>=1?saved.wetBlend:value);}};
    trimResetButton.onClick=[this]{const auto& s=state.session;processor.swellEngine().setPreferences(0,s.sensitivity,s.separationSeconds,s.autoRegenerate);refresh();};
    auditionButton.setTooltip("Play audition preview; does not start the DAW");stopButton.setTooltip("Stop audition preview; does not stop the DAW");auditionButton.setButtonText("Audition");auditionLabel.setText("Monitoring trim",juce::dontSendNotification);
    inspectorViewport.setViewedComponent(&inspectorContent,false);diagnosticsViewport.setViewedComponent(&diagnosticsContent,false);
    inspectorViewport.setName("inspectorViewport");diagnosticsViewport.setName("diagnosticsViewport");
    inspectorViewport.setScrollBarsShown(true,false);diagnosticsViewport.setScrollBarsShown(true,false);
    addAndMakeVisible(inspectorViewport);diagnosticsViewport.setVisible(true);
    for(auto* control:std::array<juce::Component*,10>{&selectedStatus,&irStatus,&loadIRButton,&enabledButton,&muteButton,&soloButton,&fullButton,&resetButton,&inCurveBox,&outCurveBox,})inspectorContent.addAndMakeVisible(control);
    slider(tailSlider,0,5,.01," s");tailSlider.setName("auditionTail");tailSlider.setTooltip("Captured Dry + Wet context after the later of the original arrival and wet endpoint. Dry audio is unchanged.");
    tailSlider.onValueChange=[this]{if(!updating)processor.swellEngine().setPostRoll(tailSlider.getValue());};
    addAndMakeVisible(tailLabel);tailLabel.setText("Listen after",juce::dontSendNotification);
    for(auto* label:{&markerLabel,&markerState}){addAndMakeVisible(label);label->setColour(juce::Label::textColourId,m3::ui::secondary);}
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
    freezeButton.onClick=[this]{processor.swellEngine().setFrozen(state.session.primary,freezeButton.getToggleState());refresh();};
    markerButton.beforeInvocation=[this]{markingPass=processor.getWorkflowSnapshot().pass;};
    markerButton.onClick=[this]{processor.swellEngine().setAutoMarkers(markerButton.getToggleState(),markingPass.value_or(processor.getWorkflowSnapshot().pass));refresh();};
    addAndMakeVisible(isolateDryButton);isolateDryButton.setName("isolateDry");
    isolateDryButton.setTooltip("Replay captured dry only within the selected source interval; overlapping mixed notes inside it cannot be separated. Live input is unchanged. Off includes the listening context.");
    isolateDryButton.onClick=[this]{processor.swellEngine().setIsolateDry(isolateDryButton.getToggleState());refresh();};
    isolateButton.onClick=[this]{processor.swellEngine().setIsolateSelected(isolateButton.getToggleState());refresh();};
    diagnosticsContent.addAndMakeVisible(placementLabel);diagnosticsContent.addAndMakeVisible(placementEditor);
    placementLabel.setText("Move wet (ms)",juce::dontSendNotification);placementEditor.setName("legacy_wetPlacement");placementEditor.setReadOnly(true);
    diagnosticsContent.addAndMakeVisible(timingButton);timingButton.setName("advancedTiming");
    timingButton.onClick=[this]{advancedTiming=!advancedTiming;resized();};
    // The swell name is renamed from the list row menu, not the Selected panel.
    for(int i=arrival;i<fieldCount;++i){inspectorContent.addAndMakeVisible(fieldLabels[i]);inspectorContent.addAndMakeVisible(fields[i]);}
    fields[name].setVisible(false);fieldLabels[name].setVisible(false);
    for(auto* control:std::array<juce::Component*,28>{&status,&detail,&generateButton,&allButton,&analyzeButton,&confirmButton,&candidateBox,&separationLabel,&separationSlider,&autoButton,&recaptureButton,&cancelButton,&recaptureStatus,&mappingButton,&toneButton,&toneStartLabel,&toneStartSlider,&toneEndLabel,&toneEndSlider,&clearButton,&assetInfoButton,&barStartLabel,&barEndLabel,&barStartEditor,&barEndEditor,&setBarsButton,&auditionLabel,&auditionSlider})diagnosticsContent.addAndMakeVisible(control);
    for(auto* control:std::array<juce::Component*,4>{&automaticButton,&sampleUnitsButton,&defaultIRButton,&suppressionButton})diagnosticsContent.addAndMakeVisible(control);
    for(auto* control:std::array<juce::Component*,10>{&removeButton,&liveButton,&liveRoutingBox,&liveLevel,&liveLevelLabel,&isolateButton,&isolateDryButton,&tailSlider,&tailLabel,&helper})diagnosticsContent.addAndMakeVisible(control);
    for(auto* label:{&inspectorHeading,&hostInfo,&listEmpty,&inspectorEmpty,&auditionHeading,&editorReadout})
    {addAndMakeVisible(label);label->setColour(juce::Label::textColourId,m3::ui::text);label->setInterceptsMouseClicks(false,false);}
    inspectorHeading.setText("Selected Swell",juce::dontSendNotification);inspectorHeading.setFont(juce::FontOptions(m3::ui::headingSize,juce::Font::bold));
    auditionHeading.setText("Audition",juce::dontSendNotification);auditionHeading.setFont(juce::FontOptions(m3::ui::headingSize,juce::Font::bold));
    hostInfo.setName("hostInfo");hostInfo.setFont(juce::FontOptions(15));hostInfo.setJustificationType(juce::Justification::centred);
    listEmpty.setText("No swells yet\n\nUse Learn to capture source.\nIn Perform, add a swell with +.",juce::dontSendNotification);listEmpty.setJustificationType(juce::Justification::centred);listEmpty.setFont(juce::FontOptions(18));
    inspectorEmpty.setText("No swell selected\n\nSelect a swell to adjust\nits settings.",juce::dontSendNotification);inspectorEmpty.setJustificationType(juce::Justification::centred);inspectorEmpty.setFont(juce::FontOptions(18));
    listLabel.setFont(juce::FontOptions(m3::ui::headingSize,juce::Font::bold));
    headerStatus.setBorderSize(juce::BorderSize<int>(0));headerStatus.setFont(juce::FontOptions(m3::ui::smallSize));
    headerStatus.setJustificationType(juce::Justification::centredRight);
    workflowHeader.settingsAction().setTooltip("Settings: playback, processing, selected swell, audition and recovery");
    markerButton.setButtonText("Auto-marker");
    const std::array<juce::String,valueCount> valueTitles{"Target","Source Start","Source End","Pre-roll (s)","End Offset (s)","Fade In (ms)","Fade Out (ms)","Level (dB)","Curve In","Curve Out","Move Wet (ms)"};
    const std::array<juce::String,valueCount> valueNames{"arrivalSample","sourceSampleStart","sourceSampleEnd","preRoll","endOffset","fadeInMs","fadeOutMs","swellLevel","curveInTension","curveOutTension","wetPlacement"};
    // Keep the older exact-bound tools available in Settings until its dedicated
    // cleanup stage; only the new cells own the primary timing/shape names.
    for(int field:{arrival,sourceA,sourceB,fadeIn,fadeOut})fields[field].setName("legacy_"+fields[field].getName());
    placementEditor.setName("legacy_wetPlacement");
    for(int index=0;index<valueCount;++index)
    {
        auto& cell=valueControls[index];inspectorContent.addAndMakeVisible(cell);
        cell.configure(valueTitles[index],valueNames[index],index<=sourceEndValue?"Absolute Playlist position. Choose exact sample units in Settings. Numeric entry is never snapped.":
            index==preRollValue?"Seconds before the fixed dry target. Negative values start after it. Placement is inverted when editing.":
            index==endOffsetValue?"Signed seconds from the dry target. Negative creates a gap; positive overlaps unchanged dry audio.":
            index==placementValue?"Move wet audio and envelope together; dry stays fixed. Exact sample alternative is in Settings.":
            index==inCurveValue || index==outCurveValue?"Independent curve tension, -1 to +1. Curve preset is in Settings.":"Type an exact value, or drag the knob. Shift-drag for fine adjustment.");
        cell.onBegin=[this,index]{beginValueEdit(index);};cell.onEnd=[this,index]{valueAnchors[index].reset();};
        cell.onDelta=[this,index](double delta){commitValueEdit(index,{},delta);};
        cell.onCommit=[this,index](const auto& text){return commitValueEdit(index,text);};
    }
    irBox.setName("selectedIR");inspectorContent.addAndMakeVisible(irBox);
    irBox.setTooltip("Choose a bundled response, or load WAV/WavPack for this swell only. Monumental is deliberately intense; Shimmer is static coloration.");
    irBox.beforeInvocation=[this]{const auto session=processor.swellEngine().savedSession();if(const auto w=m3::find(session,session.primary))irInvocation=IRInvocation{w->id,w->inputGeneration,processor.swellEngine().assignmentEpoch(),irEntries};else irInvocation.reset();};
    irBox.onChange=[this]{if(updating)return;const auto invocation=std::exchange(irInvocation,std::nullopt);
        const auto index=irBox.getSelectedId()-1;const auto& choices=invocation?invocation->choices:irEntries;
        if(index<0 || static_cast<size_t>(index)>=choices.size())return;const auto& entry=choices[static_cast<size_t>(index)];
        const auto assign=[&](const auto& id,uint64_t input,uint64_t epoch){
            if(entry.presetId.isNotEmpty())processor.swellEngine().assignBundledIR(id,input,entry.presetId,epoch);
            else processor.swellEngine().assignManagedIR(id,input,entry.managed,entry.name,epoch);};
        if(invocation)assign(invocation->id,invocation->input,invocation->epoch);
        else if(const auto w=m3::find(state.session,irOwner))assign(w->id,w->inputGeneration,processor.swellEngine().assignmentEpoch());};
    loadIRButton.setButtonText({});loadIRButton.setTooltip("Load a mono/stereo WAV or WavPack IR file for this swell");
    resetButton.setButtonText("Reset");resetButton.setName("resetWetShape");
    resetButton.setTooltip("Reset only the wet shape: start/end, fades, curves, level and placement return to defaults. Source points, IR and activity stay unchanged.");
    inspectorContent.addAndMakeVisible(resetMenu);resetMenu.setName("resetSwellMenu");resetMenu.setButtonText("Reset All");
    resetMenu.setTooltip("Reset this whole swell back to creation defaults: wet shape plus dry source points, IR, enabled/solo/freeze and activity.");
    resetMenu.onClick=[this]{const auto id=state.session.primary;processor.swellEngine().resetSwell(id,true);refresh();};
    freezeButton.setButtonText("Freeze");
    for(auto* label:{&captureDiagnostic,&playbackDiagnostic,&policyDiagnostic}){diagnosticsContent.addAndMakeVisible(label);label->setFont(juce::FontOptions(m3::ui::controlSize));label->setJustificationType(juce::Justification::topLeft);}
    status.setName("diagnosticTiming");captureDiagnostic.setName("diagnosticCapture");playbackDiagnostic.setName("diagnosticPlayback");policyDiagnostic.setName("diagnosticPolicy");
    toneButton.setName("scheduledTone");
    initialiseSettings();
    // setResizeLimits may already have brought the editor to the minimum.
    // Lay out once after every child is attached even if setSize is a no-op.
    list.setRowHeight(64);setSize(1100,850);resized();refresh();startTimerHz(10);
}
PreverbAudioProcessorEditor::~PreverbAudioProcessorEditor(){destroying=true;stopTimer();diagnosticsWindow.reset();settingsWindow.reset();list.setModel(nullptr);setLookAndFeel(nullptr);}
void PreverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(m3::ui::background);const m3::ui::WorkspaceLayout layout(getLocalBounds());g.setColour(m3::ui::separator);
    if(learningLayout || exportLayout){g.drawHorizontalLine(layout.header.getBottom(),static_cast<float>(layout.header.getX()),static_cast<float>(layout.header.getRight()));return;}
    for(const auto y:{layout.header.getBottom(),layout.globals.getBottom(),layout.workspace.getBottom(),layout.audition.getBottom(),layout.overview.getBottom()})
        g.drawHorizontalLine(y,static_cast<float>(layout.header.getX()),static_cast<float>(layout.header.getRight()));
    for(const auto x:{layout.swells.getRight()+10,layout.inspector.getX()-10})
        g.drawVerticalLine(x,static_cast<float>(layout.workspace.getY()+10),static_cast<float>(layout.workspace.getBottom()-10));
}
int PreverbAudioProcessorEditor::getNumRows(){return static_cast<int>(state.session.swells.size());}
void PreverbAudioProcessorEditor::paintListBoxItem(int row,juce::Graphics& g,int width,int height,bool selected)
{
    if(row<0 || row>=getNumRows())return;const auto& w=state.session.swells[static_cast<size_t>(row)];
    if(selected){g.setColour(m3::ui::aqua.withAlpha(.06f));g.fillRoundedRectangle(juce::Rectangle<float>(1,1,static_cast<float>(width-2),static_cast<float>(height-2)),4);g.setColour(m3::ui::aqua);g.drawRoundedRectangle(juce::Rectangle<float>(1,1,static_cast<float>(width-2),static_cast<float>(height-2)),4,1);}
    g.setColour(m3::ui::text);g.setFont(m3::ui::fontFor(*this,m3::ui::bodySize));g.drawText(w.name,32,2,width-62,20,juce::Justification::left);
    g.drawRect(8,8,15,15);if(w.enabled){g.setColour(m3::ui::aqua);g.fillRect(11,11,9,9);}
    g.setFont(m3::ui::fontFor(*this,m3::ui::smallSize));g.drawText(w.irName.isNotEmpty()?w.irName:"No IR",32,23,width-62,18,juce::Justification::left);
    g.setFont(m3::ui::fontFor(*this,m3::ui::captionSize));g.setColour(w.phase==m3::Phase::error?m3::ui::warning:m3::ui::secondary);
    g.drawText((w.phase==m3::Phase::ready?phaseName(w.phase):w.status)+(w.muted?" | mute":"")+(!w.enabled?" | off":"")+(w.solo?" | solo":"")+(w.frozen?" | frozen":""),32,43,width-62,height-43,juce::Justification::left);
    g.setColour(m3::ui::secondary);
    for(int n=0;n<3;++n)g.fillEllipse(juce::Rectangle<float>(3,3).withCentre({static_cast<float>(width-13),static_cast<float>(height/2-7+n*7)}));
}
void PreverbAudioProcessorEditor::listBoxItemClicked(int row,const juce::MouseEvent& event)
{
    if(row<0 || row>=getNumRows())return;
    const auto& w=state.session.swells[static_cast<size_t>(row)];
    if(event.mods.isPopupMenu() || (event.eventComponent && event.x>=event.eventComponent->getWidth()-26))
    {
        juce::PopupMenu menu;menu.addItem(1,"Rename...");menu.addItem(2,"Delete");
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(event.eventComponent).withTargetScreenArea(swellMenuAnchor(event)),
            [safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(this),id=w.id,name=w.name](int result)
        {
            if(!safe)return;
            if(result==2){safe->processor.swellEngine().selectTargets({id},id);safe->processor.swellEngine().removeSelected();safe->refresh();return;}
            if(result!=1)return;
            auto* dialog=new juce::AlertWindow("Rename swell","",juce::MessageBoxIconType::NoIcon);
            dialog->addTextEditor("name",name);dialog->getTextEditor("name")->selectAll();
            dialog->addButton("OK",1,juce::KeyPress(juce::KeyPress::returnKey));dialog->addButton("Cancel",0,juce::KeyPress(juce::KeyPress::escapeKey));
            dialog->enterModalState(true,juce::ModalCallbackFunction::create([safe,id,dialog](int button)
            {
                if(!safe || button!=1)return;
                const auto text=dialog->getTextEditorContents("name");
                if(text.isNotEmpty())safe->processor.swellEngine().edit(id,[&](auto& swell,auto&){swell.name=text;return true;});
                safe->refresh();
            }),true);
        });
        return;
    }
    if(event.x<28){processor.swellEngine().edit(w.id,[](auto& swell,auto&){swell.enabled=!swell.enabled;return true;});refresh();}
}
void PreverbAudioProcessorEditor::selectedRowsChanged(int row)
{
    if(updating || destroying)return;std::vector<juce::String> ids;
    for(int i=0;i<getNumRows();++i)if(list.isRowSelected(i))ids.push_back(state.session.swells[static_cast<size_t>(i)].id);
    const auto primary=row>=0 && row<getNumRows()?state.session.swells[static_cast<size_t>(row)].id:(ids.empty()?juce::String{}:ids.front());
    processor.swellEngine().selectTargets(ids,primary);refresh();
}
void PreverbAudioProcessorEditor::setPreferences()
{const auto s=processor.swellEngine().savedSession();processor.swellEngine().setPreferences(static_cast<float>(auditionSlider.getValue()),s.sensitivity,s.separationSeconds,s.autoRegenerate);}
void PreverbAudioProcessorEditor::curveChanged(bool in)
{
    if(updating || destroying)return;const auto curve=static_cast<m3::Curve>((in?inCurveBox:outCurveBox).getSelectedId()-1);
    processor.swellEngine().edit(state.session.primary,[&](auto& w,auto&){if(in)w.shape.inCurve=curve;else w.shape.outCurve=curve;return true;});
}
void PreverbAudioProcessorEditor::applyBarRange()
{processor.setSourceBars(barStartEditor.getText().getLargeIntValue(),barEndEditor.getText().getLargeIntValue());refresh();}
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
    const m3::ui::WorkspaceLayout layout(getLocalBounds());
    workflowHeader.setBounds(layout.header);
    noticeBar.setBounds(layout.notice);
    if(learningLayout || exportLayout){auto body=getLocalBounds().reduced(m3::ui::workspaceGeometry::outerInset(getWidth()));body.setTop(layout.header.getBottom()+m3::ui::spacing::group);
        learnWorkspace.setBounds(body);exportWorkspace.setBounds(body);return;}
    auto globals=layout.globals.reduced(0,8);
    markerButton.setBounds(globals.removeFromLeft(160));globals.removeFromLeft(16);
    markerLabel.setBounds(globals.removeFromLeft(118));markerKnob.setBounds(globals.removeFromLeft(150));globals.removeFromLeft(24);
    regenerationLabel.setBounds({});regenerationKnob.setBounds({});
    recaptureExisting.setBounds(globals.removeFromLeft(198));
    markerState.setBounds({});headerStatus.setBounds(globals);
    auto listArea=layout.swells.reduced(0,10);auto listTop=listArea.removeFromTop(32);
    addButton.setBounds(listTop.removeFromRight(32).reduced(1));addButton.setButtonText("+");listLabel.setBounds(listTop);
    list.setBounds(listArea.reduced(0,4));listEmpty.setBounds(list.getBounds().reduced(8,12));
    auto inspectorArea=layout.inspector.reduced(0,10);inspectorHeading.setBounds(inspectorArea.removeFromTop(32));
    inspectorViewport.setBounds(inspectorArea);inspectorEmpty.setBounds(inspectorArea.reduced(10));
    const auto inspectorWidth=layout.inspector.getWidth();
    auto area=layout.editor.reduced(0,10);auto navigation=area.removeFromBottom(32);
    editorReadout.setBounds(area.removeFromBottom(26));editorReadout.setName("editorReadout");
    for(auto* button:{&previousButton,&nextButton,&zoomOutButton,&zoomInButton,&fitButton})button->setBounds(navigation.removeFromLeft(43).reduced(1));
    spanLabel.setBounds(navigation.removeFromLeft(40));spanBox.setBounds(navigation.removeFromLeft(52).reduced(1));snapButton.setBounds(navigation.removeFromLeft(65));
    timeline.setBounds(area);
    auto audition=layout.audition.reduced(0,8);auto auditionRow=audition.removeFromTop(36);
    auditionHeading.setBounds(auditionRow.removeFromLeft(112));auditionButton.setBounds(auditionRow.removeFromLeft(50).reduced(1));stopButton.setBounds(auditionRow.removeFromLeft(50).reduced(1));
    trimResetButton.setBounds(auditionRow.removeFromRight(138).reduced(1));blendSlider.setBounds(auditionRow.reduced(24,0));
    monitorStatus.setBounds(audition.withTrimmedLeft(116));
    songOverview.setBounds(layout.overview.reduced(0,8));hostInfo.setBounds(layout.info.reduced(0,4));
    const auto width=std::max(280,inspectorWidth-14);inspectorContent.setSize(width,560);
    auto inspector=inspectorContent.getLocalBounds().reduced(2,4);
    selectedStatus.setBounds(inspector.removeFromTop(24));
    fields[name].setBounds({});fieldLabels[name].setBounds({});
    auto row=inspector.removeFromTop(30);enabledButton.setBounds(row.removeFromLeft(100));soloButton.setBounds(row.removeFromLeft(80));freezeButton.setBounds(row);
    irStatus.setBounds(inspector.removeFromTop(21));
    row=inspector.removeFromTop(30);loadIRButton.setBounds(row.removeFromRight(30).reduced(1));irBox.setBounds(row.reduced(1));
    inspector.removeFromTop(6);
    const std::array<int,valueCount> controlOrder{targetValue,placementValue,sourceStartValue,sourceEndValue,preRollValue,endOffsetValue,fadeInValue,fadeOutValue,inCurveValue,outCurveValue,levelValue};
    for(int index=0;index<valueCount;index+=2)
    {
        row=inspector.removeFromTop(50);auto left=row.removeFromLeft((row.getWidth()-10)/2);row.removeFromLeft(10);
        valueControls[controlOrder[index]].setBounds(left);if(index+1<valueCount)valueControls[controlOrder[index+1]].setBounds(row);
    }
    // Destructive resets sit beneath all the knob controls.
    inspector.removeFromTop(6);
    row=inspector.removeFromTop(32);auto resetLeft=row.removeFromLeft((row.getWidth()-10)/2);row.removeFromLeft(10);
    resetButton.setBounds(resetLeft.reduced(1));resetMenu.setBounds(row.reduced(1));
    // Read-only precision/asset observations; musician edits live in the inspector.
    for(int field=arrival;field<fieldCount;++field){diagnosticsContent.addAndMakeVisible(fieldLabels[field]);diagnosticsContent.addAndMakeVisible(fields[field]);}
    diagnosticsContent.setSize(960,850);auto diagnostic=diagnosticsContent.getLocalBounds().reduced(20);
    status.setBounds(diagnostic.removeFromTop(72));captureDiagnostic.setBounds(diagnostic.removeFromTop(72));detail.setBounds(diagnostic.removeFromTop(140));
    playbackDiagnostic.setBounds(diagnostic.removeFromTop(80));policyDiagnostic.setBounds(diagnostic.removeFromTop(96));
    row=diagnostic.removeFromTop(36);timingButton.setBounds(row.removeFromLeft(290).reduced(1));assetInfoButton.setBounds(row.removeFromLeft(180).reduced(1));
    for(int field=arrival;field<fieldCount;++field){row=diagnostic.removeFromTop(32);fieldLabels[field].setBounds(row.removeFromLeft(200));fields[field].setBounds(row.removeFromLeft(300).reduced(1,3));}
    row=diagnostic.removeFromTop(34);placementLabel.setBounds(row.removeFromLeft(200));placementEditor.setBounds(row.removeFromLeft(300));
    helper.setBounds({});
    layoutSettings();

}
void PreverbAudioProcessorEditor::refresh()
{
    if(destroying)return;updating=true;state=processor.swellEngine().snapshot();const auto capture=processor.getCaptureSnapshot();const auto time=processor.getSongTime();const auto diagnostics=processor.getHostPositionDiagnostics();
    const auto mode=processor.getWorkflowSnapshot().mode;
    const auto nextLearning=mode==workflow::Mode::capture,nextExport=mode==workflow::Mode::finalise;
    if(exportLayout && !nextExport)exportWorkspace.resetView();
    const auto wasSpecial=learningLayout || exportLayout,nextSpecial=nextLearning || nextExport;
    const auto leavingLearn=wasSpecial && !nextSpecial;
    const auto isShared=[this](juce::Component* c){return c==&workflowHeader || c==&noticeBar || c==&learnWorkspace || c==&exportWorkspace
        || dynamic_cast<juce::ResizableCornerComponent*>(c)!=nullptr || dynamic_cast<juce::ResizableBorderComponent*>(c)!=nullptr;};
    if(nextLearning!=learningLayout || nextExport!=exportLayout)
    {
        if(nextSpecial && !wasSpecial){if(hasPerformWorkspace)performViewport=timeline.viewport();performVisibility.clear();for(auto* c:getChildren())if(!isShared(c))performVisibility[c]=c->isVisible();}
        else if(!nextSpecial){for(const auto& [c,visible]:performVisibility)if(c->getParentComponent()==this)c->setVisible(visible);performVisibility.clear();}
        learningLayout=nextLearning;exportLayout=nextExport;resized();
    }
    if(!nextSpecial){timeline.setData(capture,state,time,diagnostics.samplePositionAvailable);if(leavingLearn && performViewport)timeline.setViewport(*performViewport);hasPerformWorkspace=true;}
    hostInfo.setText((diagnostics.samplePositionAvailable && time.valid?m3::musicalPosition(time.sample,time):juce::String("Position unavailable"))
        +"    |    "+(diagnostics.tempoAvailable?juce::String(diagnostics.bpm,2)+" BPM":juce::String("BPM unavailable"))
        +"    |    "+(diagnostics.meterAvailable?juce::String(diagnostics.numerator)+" / "+juce::String(diagnostics.denominator):juce::String("Meter unavailable")),juce::dontSendNotification);
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
    refreshValues(w,time);
    listLabel.setText("Swells ("+juce::String(static_cast<int>(state.session.swells.size()))+")",juce::dontSendNotification);
    const auto key=w?w->id:juce::String{};
    if(!learningLayout && !exportLayout && key!=fittedKey && state.raw){if(!leavingLearn)timeline.fitSelection();fittedKey=key;}
    if(!learningLayout && !exportLayout){list.updateContent();juce::SparseSet<int> selectedRows;
    for(size_t i=0;i<state.session.swells.size();++i)if(std::find(state.session.selected.begin(),state.session.selected.end(),state.session.swells[i].id)!=state.session.selected.end())selectedRows.addRange({static_cast<int>(i),static_cast<int>(i+1)});
    list.setSelectedRows(selectedRows,juce::dontSendNotification);list.repaint();}
    enabledButton.setToggleState(w && w->enabled,juce::dontSendNotification);muteButton.setToggleState(w && w->muted,juce::dontSendNotification);
    soloButton.setToggleState(w && w->solo,juce::dontSendNotification);freezeButton.setToggleState(w && w->frozen,juce::dontSendNotification);
    if(w){inCurveBox.setSelectedId(static_cast<int>(w->shape.inCurve)+1,juce::dontSendNotification);outCurveBox.setSelectedId(static_cast<int>(w->shape.outCurve)+1,juce::dontSendNotification);}
    if(!markerKnob.isMouseButtonDown() && !markerKnob.hasKeyboardFocus(true))markerKnob.setValue(state.session.sensitivity*100,juce::dontSendNotification);
    if(!regenerationKnob.isMouseButtonDown() && !regenerationKnob.hasKeyboardFocus(true))regenerationKnob.setValue(state.session.regenerationSensitivity*100,juce::dontSendNotification);
    markerButton.setToggleState(state.session.autoMarkersEnabled,juce::dontSendNotification);
    markerState.setText(!state.session.autoMarkersEnabled?"Off":!state.session.automaticProcessing?"Master paused":state.analyzing?"Analysing":"On",juce::dontSendNotification);
    isolateDryButton.setToggleState(state.session.isolateDry,juce::dontSendNotification);isolateDryButton.setEnabled(state.session.auditionMode==m3::AuditionMode::dryWet);
    isolateButton.setToggleState(state.session.isolateSelected,juce::dontSendNotification);
    if(!tailSlider.isMouseButtonDown() && !tailSlider.hasKeyboardFocus(true))tailSlider.setValue(state.session.postRollSeconds,juce::dontSendNotification);
    if(!auditionSlider.isMouseButtonDown() && !auditionSlider.hasKeyboardFocus(true))auditionSlider.setValue(state.session.auditionDb,juce::dontSendNotification);
    if(!separationSlider.isMouseButtonDown())separationSlider.setValue(state.session.separationSeconds*1000,juce::dontSendNotification);
    autoButton.setToggleState(state.session.autoRegenerate,juce::dontSendNotification);
    automaticButton.setToggleState(state.session.automaticProcessing,juce::dontSendNotification);
    if(!blendSlider.isMouseButtonDown() && !blendSlider.hasKeyboardFocus(true))blendSlider.setValue(state.session.auditionMode==m3::AuditionMode::wetOnly?100:state.session.wetBlend*100,juce::dontSendNotification);
    trimResetButton.setButtonText("Trim "+juce::String(state.session.auditionDb,1)+" dB"+(state.session.auditionDb!=0?" | reset":""));
    juce::String monitoring="Preview only | "+(state.dryStatus.isNotEmpty()?state.dryStatus:(state.session.isolateSelected?juce::String("Isolated"):juce::String("Overlaps"))+" | "+juce::String(static_cast<int>(state.auditionMembers.size()))+" wet | "+(state.session.isolateDry?"selected dry":"dry context"));
    if(state.attenuationDb<0)monitoring<<" | wet safety "<<juce::String(state.attenuationDb,1)<<" dB";
    if(state.blendAttenuationDb<0)monitoring<<" | sum safety "<<juce::String(state.blendAttenuationDb,1)<<" dB";
    monitorStatus.setText(monitoring,juce::dontSendNotification);monitorStatus.setTooltip(monitoring+". Live playback routing and gain are independent.");
    selectedStatus.setText(w?phaseName(w->phase)+" | "+w->name:"Select a swell",juce::dontSendNotification);
    selectedStatus.setTooltip(w?w->status:state.message);
    if(w && musical){const auto& g=w->calibration;const auto calibrationText=g.state==m3::CalibrationState::unavailable?
            juce::String("Calibration unavailable"):"Calibration "+juce::String(g.baselineDb,1)+" dB"+(g.state==m3::CalibrationState::capped?" (capped)":"");
        selectedStatus.setTooltip(w->status+"\n"+calibrationText+" | Requested Level "+juce::String(w->shape.gainDb,1)+" dB");
        if(w->phase==m3::Phase::ready && (g.state==m3::CalibrationState::unavailable || g.state==m3::CalibrationState::capped))
            selectedStatus.setText(phaseName(w->phase)+" | "+(g.state==m3::CalibrationState::unavailable?"Calibration unavailable":"Calibration capped"),juce::dontSendNotification);}
    irStatus.setText("Impulse Response (IR)",juce::dontSendNotification);
    if(state.retainedIRName.isNotEmpty()){
        const auto text=(state.liveActive?juce::String("Playing "):juce::String("Retained "))+state.retainedIRName+" | Requested "+state.requestedIRName;
        irStatus.setText(text,juce::dontSendNotification);irStatus.setTooltip(text+" | "+(w?w->status:state.message));}
    const auto choice=w?w->ir.content+"|"+w->irName:juce::String{};
    auto entries=processor.swellEngine().irChoices();juce::String catalogue;
    for(const auto& entry:entries)catalogue<<entry.id<<"|"<<entry.name<<"\n";
    if(irOwner!=(w?w->id:juce::String{}) || irChoice!=choice || irCatalogue!=catalogue)
    {
        irBox.hidePopup();irOwner=w?w->id:juce::String{};irChoice=choice;irBox.clear(juce::dontSendNotification);int selected=0,index=0;
        irCatalogue=catalogue;irEntries=std::move(entries);
        for(const auto& entry:irEntries){irBox.addItem(entry.name,++index);
            if(w && w->ir.present() && w->ir.content==entry.content){selected=index;irBox.setTooltip(entry.description);}}
        if(selected)irBox.setSelectedId(selected,juce::dontSendNotification);else irBox.setTextWhenNothingSelected("Choose an IR");
    }
    irBox.setEnabled(w && !w->frozen);loadIRButton.setEnabled(w && !w->frozen);
    resetButton.setEnabled(w && time.valid && capture.manifest.sampleRate==time.rate);resetMenu.setEnabled(w && time.valid);
    const std::array<juce::String,5> timingLabels{"Arrival","Source start","Source end","Wet start","Wet end"};
    for(int field=arrival;field<=audibleB;++field)fieldLabels[field].setText(timingLabels[static_cast<size_t>(field-arrival)]+(sampleUnitsButton.getToggleState()?" sample":""),juce::dontSendNotification);
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
    candidateBox.clear(juce::dontSendNotification);int selectedCandidate=0;const auto viewport=timeline.viewport();
    for(size_t i=0;i<state.candidates.size();++i)
    {
        const auto sample=state.candidates[i].sample;const auto chosen=chosenCandidate && sample==*chosenCandidate;
        if(chosen || (candidateBox.getNumItems()<128 && sample>=viewport.first && sample<=viewport.last))
        {candidateBox.addItem("Sample "+juce::String(sample),static_cast<int>(i+1));if(chosen)selectedCandidate=static_cast<int>(i+1);}
    }
    if(!selectedCandidate && candidateBox.getNumItems()){selectedCandidate=candidateBox.getItemId(0);chosenCandidate=state.candidates[static_cast<size_t>(selectedCandidate-1)].sample;}
    if(selectedCandidate)candidateBox.setSelectedId(selectedCandidate,juce::dontSendNotification);else chosenCandidate.reset();
    candidateBox.setTextWhenNothingSelected("Click a suggestion in the source lane");
    confirmButton.setEnabled(chosenCandidate.has_value());analyzeButton.setEnabled(!capture.manifest.ranges.empty() && !state.analyzing);
    const auto hasSelection=!state.session.selected.empty();bool selectedIR=false,selectedImport=false,anyIR=false;
    for(const auto& swell:state.session.swells)
    {
        anyIR=anyIR || swell.ir.present();
        if(std::find(state.session.selected.begin(),state.session.selected.end(),swell.id)!=state.session.selected.end())
        {selectedIR=selectedIR || swell.ir.present();selectedImport=selectedImport || swell.status=="Importing IR";}
    }
    loadIRButton.setEnabled(hasSelection && w && !w->frozen);removeButton.setEnabled(hasSelection);generateButton.setEnabled(hasSelection && selectedIR && !selectedImport);allButton.setEnabled(anyIR && !selectedImport);
    auditionButton.setEnabled(state.auditionReady && !(state.session.liveEnabled && diagnostics.isPlaying));stopButton.setEnabled(state.playing || state.auditionReady);fullButton.setEnabled(w && state.raw);
    freezeButton.setEnabled(w && (w->frozen || w->phase==m3::Phase::ready));
    for(auto* control:std::array<juce::Component*,6>{&enabledButton,&muteButton,&soloButton,&inCurveBox,&outCurveBox,&setBarsButton})control->setEnabled(w!=nullptr);
    if(w && w->frozen){for(int field:{arrival,sourceA,sourceB})fields[field].setEnabled(false);setBarsButton.setEnabled(false);}
    timingButton.setEnabled(w!=nullptr);
    mappingButton.setToggleState(processor.isMappingEnabled(),juce::dontSendNotification);toneButton.setToggleState(processor.isTestToneEnabled(),juce::dontSendNotification);
    toneStartSlider.setValue(processor.getToneStartBar(),juce::dontSendNotification);toneEndSlider.setValue(processor.getToneEndBar(),juce::dontSendNotification);
    const auto now=juce::Time::getMillisecondCounterHiRes();
    if(diagnosticsOpen && (diagnosticsRefreshedAt<0 || now-diagnosticsRefreshedAt>=200))
    {
    diagnosticsRefreshedAt=now;
    juce::String diagnostic="Transport: ";diagnostic<<(!diagnostics.positionAvailable?"unavailable":diagnostics.isPlaying?"playing":"stopped")<<" | sample "<<(diagnostics.samplePositionAvailable?juce::String(time.sample):"unavailable")<<" | PPQ "<<(diagnostics.ppqPositionAvailable?juce::String(time.ppq,3):"unavailable")
        <<" | "<<(diagnostics.tempoAvailable?juce::String(time.bpm,2):"unavailable")<<" BPM | "<<(diagnostics.meterAvailable?juce::String(time.numerator)+"/"+juce::String(time.denominator):"meter unavailable")<<" | "<<juce::String(time.rate,0)<<" Hz | block "<<diagnostics.blockSize;
    status.setText("Timing\n"+diagnostic,juce::dontSendNotification);
    juce::String captured="Capture / assets\n";captured<<capture.status<<" | revision "<<juce::String(static_cast<juce::int64>(capture.manifest.revision))<<" | dropped "<<juce::String(static_cast<juce::int64>(processor.getDroppedCaptureBlockCount()))<<" | discontinuities "<<juce::String(static_cast<juce::int64>(processor.getTimelineDiscontinuityCount()));
    captureDiagnostic.setText(captured,juce::dontSendNotification);captureDiagnostic.setTooltip(captured);
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
    for(auto& field:fields)field.clear();placementEditor.clear();
    if(w){fields[name].setText(w->name,false);fields[arrival].setText(position(w->arrival),false);fields[sourceA].setText(position(w->sourceStart),false);fields[sourceB].setText(position(w->sourceEnd),false);
        int64_t a,b;if(state.raw && m3::requestedShapeBounds(*w,state.raw->start,a,b)){fields[audibleA].setText(position(a),false);fields[audibleB].setText(position(b),false);}
        if(time.rate>0){fields[fadeIn].setText(juce::String(w->shape.fadeIn*1000.0/time.rate,3),false);fields[fadeOut].setText(juce::String(w->shape.fadeOut*1000.0/time.rate,3),false);placementEditor.setText(juce::String(w->placement*1000.0/time.rate,3),false);}}

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
    workflowHeader.display(m3::ui::presentWorkflow(flow,workflowReady),capture.status);
    noticeBar.display(m3::ui::workspaceNotice(flow,capture,state,irPreparationNotice.update(state,
        processor.swellEngine().assignmentEpoch(),juce::Time::getMillisecondCounterHiRes())));
    recaptureExisting.setToggleState(flow.recapture,juce::dontSendNotification);recaptureExisting.setEnabled(flow.mode==workflow::Mode::capture && !flow.locked());
    recaptureExisting.setTooltip("Off fills unmapped positions only. On replaces encountered positions as one pass committed on DAW Stop. Independent of Automatic Swells.");
    recaptureButton.setEnabled(flow.mode==workflow::Mode::capture && !flow.locked());cancelButton.setEnabled(flow.mode==workflow::Mode::capture && (flow.locked() || flow.recapture));
    recaptureStatus.setText(flow.recapture?"Recapture armed for encountered positions; commit on DAW Stop":"Recapture off; mapped source protected",juce::dontSendNotification);
    mappingButton.setEnabled(!flow.locked());mappingButton.setTooltip("Legacy control requests Learn or Perform; workflow guards always apply.");
    clearButton.setEnabled(flow.mode==workflow::Mode::capture && !flow.locked());
    regenerationKnob.setEnabled(false);regenerationKnob.setTooltip("Saved for compatibility. Source replacement requires Learn with explicit Recapture.");
    const bool edit=caps.edit;timeline.setEditingEnabled(edit);
    if(!edit)for(auto& control:valueControls)if(control.editing())control.cancel();
    inspectorContent.setEnabled(edit);addButton.setEnabled(edit);removeButton.setEnabled(edit && hasSelection);
    for(auto* button:{&generateButton,&allButton,&analyzeButton,&confirmButton,&timingButton})button->setEnabled(edit && button->isEnabled());
    auditionButton.setEnabled(caps.preview && state.auditionReady && !(flow.mode==workflow::Mode::live && state.session.liveEnabled && diagnostics.isPlaying));
    stopButton.setEnabled(caps.preview && (state.playing || state.auditionReady));
    if(flow.mode==workflow::Mode::capture)monitorStatus.setText("Learn | dry passthrough | editing and audition available in Perform",juce::dontSendNotification);
    if(flow.mode==workflow::Mode::finalise)monitorStatus.setText("Export | DAW wet bypassed | explicit preview only | editing locked",juce::dontSendNotification);
    automaticButton.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());markerButton.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());markerKnob.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());separationSlider.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());
    gainPolicyBox.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());defaultIRButton.setEnabled(flow.mode!=workflow::Mode::finalise && !flow.locked());suppressionButton.setEnabled(edit);
    toneButton.setEnabled(flow.mode==workflow::Mode::live);recaptureButton.setButtonText("Arm Recapture pass");
    if(learningLayout)learnWorkspace.display(capture,state,time,flow,diagnostics.samplePositionAvailable,processor.getCaptureProgress());
    if(exportLayout){exportWorkspace.display(state);noticeBar.display(exportWorkspace.notice());}
    if(learningLayout || exportLayout)for(auto* c:getChildren())if(!isShared(c))c->setVisible(false);
    learnWorkspace.setVisible(learningLayout);exportWorkspace.setVisible(exportLayout);
    updating=false;
}
