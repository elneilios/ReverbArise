#include "PluginEditor.h"
#include <UI/MusicalPosition.h>

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
    :AudioProcessorEditor(p),processor(p),timeline(p.swellEngine())
{
    setLookAndFeel(&workspaceTheme);
    setResizable(true,true);setResizeLimits(1100,850,2000,1400);
    title.setText("Preverb | Swell editor",juce::dontSendNotification);title.setFont(juce::FontOptions(22.0f,juce::Font::bold));
    for(auto* label:{&title,&status,&detail,&listLabel,&auditionLabel,&sensitivityLabel,&separationLabel,&spanLabel,&curveInLabel,&curveOutLabel,&gainLabel,&barStartLabel,&barEndLabel,&recaptureStatus,&toneStartLabel,&toneEndLabel})
    {label->setColour(juce::Label::textColourId,m3::ui::text);label->setJustificationType(juce::Justification::centredLeft);addAndMakeVisible(label);}
    status.setFont(juce::FontOptions(12.0f));detail.setFont(juce::FontOptions(12.0f));detail.setJustificationType(juce::Justification::topLeft);
    listLabel.setText("Swells",juce::dontSendNotification);auditionLabel.setText("Audition level",juce::dontSendNotification);
    sensitivityLabel.setText("Sensitivity",juce::dontSendNotification);separationLabel.setText("Min. separation",juce::dontSendNotification);
    spanLabel.setText("Bars",juce::dontSendNotification);curveInLabel.setText("Fade-in curve",juce::dontSendNotification);curveOutLabel.setText("Fade-out curve",juce::dontSendNotification);
    gainLabel.setText("Swell gain",juce::dontSendNotification);barStartLabel.setText("Bar start",juce::dontSendNotification);barEndLabel.setText("End excl.",juce::dontSendNotification);
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
    for(auto* button:{&snapButton,&autoButton,&enabledButton,&muteButton,&soloButton,&mappingButton,&toneButton})addAndMakeVisible(button);
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
        fieldLabels[i].setText(labels[i],juce::dontSendNotification);fieldLabels[i].setColour(juce::Label::textColourId,m3::ui::secondary);fieldLabels[i].setFont(juce::FontOptions(12.0f));addAndMakeVisible(fieldLabels[i]);
        fields[i].setName(names[i]);fields[i].setInputRestrictions(i==name?256:40,i==name?juce::String{}:(i>=fadeIn?juce::String("0123456789."):juce::String("-0123456789.")));
        fields[i].onReturnKey=[this,i]{commitField(i);};fields[i].onFocusLost=[this,i]{commitField(i);};addAndMakeVisible(fields[i]);
    }
    barStartEditor.setName("sourceStart");barEndEditor.setName("sourceEnd");barStartEditor.setText("1",false);barEndEditor.setText("2",false);
    for(auto* field:{&barStartEditor,&barEndEditor}){field->setInputRestrictions(12,"0123456789");addAndMakeVisible(field);field->onReturnKey=[this]{applyBarRange();};}
    const auto slider=[&](juce::Slider& control,double a,double b,double step,const juce::String& suffix)
    {control.setRange(a,b,step);control.setSliderStyle(juce::Slider::LinearHorizontal);control.setTextBoxStyle(juce::Slider::TextBoxRight,false,78,22);control.setTextValueSuffix(suffix);addAndMakeVisible(control);};
    slider(auditionSlider,-60,0,0.1," dB");auditionSlider.setName("auditionLevel");slider(gainSlider,-60,12,0.1," dB");gainSlider.setName("swellGain");
    slider(sensitivitySlider,0,1,0.01,"");slider(separationSlider,1,2000,1," ms");slider(toneStartSlider,1,511,1,"");slider(toneEndSlider,2,512,1,"");
    for(int bars:{1,2,4,8,16})spanBox.addItem(juce::String(bars),bars);spanBox.setSelectedId(4,juce::dontSendNotification);
    for(auto* box:{&inCurveBox,&outCurveBox}){box->addItem("Linear",1);box->addItem("Smooth",2);box->addItem("Exponential",3);}
    snapButton.setToggleState(true,juce::dontSendNotification);
    addButton.onClick=[this]{processor.swellEngine().addTarget(processor.getSongTime().sample);refresh();};
    addButton.setTooltip("Move FL Studio's playhead to captured audio, or double-click the source waveform to add a target.");
    removeButton.onClick=[this]{processor.swellEngine().removeSelected();refresh();};
    loadIRButton.onClick=[this]
    {
        const auto w=m3::find(state.session,state.session.primary);if(!w || w->frozen)return;
        const auto id=w->id;const auto generation=w->inputGeneration;
        chooser=std::make_unique<juce::FileChooser>("Load mono/stereo WAV or WavPack IR",juce::File{},"*.wav;*.wv");
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
            [safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(this),id,generation](const juce::FileChooser& dialog)
            {if(safe && dialog.getResult()!=juce::File{})safe->processor.swellEngine().assignIR(id,generation,dialog.getResult());});
    };
    generateButton.onClick=[this]{processor.swellEngine().generate();refresh();};allButton.onClick=[this]{processor.swellEngine().generate(true);refresh();};
    auditionButton.onClick=[this]{processor.swellEngine().audition();};stopButton.onClick=[this]{processor.swellEngine().stop();};
    auditionSlider.onValueChange=[this]{if(!updating)setPreferences();};sensitivitySlider.onValueChange=[this]{if(!updating)setPreferences();};
    separationSlider.onValueChange=[this]{if(!updating)setPreferences();};autoButton.onClick=[this]{setPreferences();};
    gainSlider.onValueChange=[this]
    {if(updating)return;const auto id=state.session.primary;const auto gain=static_cast<float>(gainSlider.getValue());processor.swellEngine().edit(id,[&](auto& w,auto&){w.shape.gainDb=gain;return true;});};
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
    headerStatus.setName("processingStatus");selectedStatus.setName("selectedStatus");
    headerStatus.setBorderSize(juce::BorderSize<int>(0,24,0,8));
    title.setText("Preverb - Reverse Reverb",juce::dontSendNotification);
    helper.setText("Live playback: awaiting prepared Song audio",juce::dontSendNotification);
    liveButton.setName("livePlayback");liveRoutingBox.setName("liveRouting");liveLevel.setName("liveWetLevel");
    liveRoutingBox.addItem("Dry + Wet",1);liveRoutingBox.addItem("Wet Only",2);
    liveLevelLabel.setText("Live wet",juce::dontSendNotification);
    liveLevel.setRange(-60,0,.1);liveLevel.setTextValueSuffix(" dB");liveLevel.setSliderStyle(juce::Slider::LinearHorizontal);
    liveLevel.setTextBoxStyle(juce::Slider::TextBoxRight,false,75,24);
    addAndMakeVisible(liveButton);addAndMakeVisible(liveRoutingBox);addAndMakeVisible(liveLevel);addAndMakeVisible(liveLevelLabel);
    const auto updateLive=[this]{if(!updating)processor.swellEngine().setLivePlayback(liveButton.getToggleState(),
        liveRoutingBox.getSelectedId()==2?m3::LiveRouting::wetOnly:m3::LiveRouting::dryWet,static_cast<float>(liveLevel.getValue()));};
    liveButton.onClick=updateLive;liveRoutingBox.onChange=updateLive;liveLevel.onValueChange=updateLive;
    helper.setFont(juce::FontOptions(12));
    for(auto* button:{&diagnosticsButton,&wetOnlyButton,&dryWetButton,&trimResetButton,&defaultIRButton,&suppressionButton})addAndMakeVisible(button);
    for(auto* button:{&automaticButton,&sampleUnitsButton})addAndMakeVisible(button);
    sampleUnitsButton.setName("sampleUnits");automaticButton.setName("automaticProcessing");diagnosticsButton.setName("diagnostics");
    wetOnlyButton.setName("wetOnly");dryWetButton.setName("dryWet");trimResetButton.setName("resetAuditionTrim");
    blendSlider.setName("auditionBlend");slider(blendSlider,0,100,.1," % Wet");
    for(auto* knob:{&fadeInKnob,&fadeOutKnob})
    {knob->setRange(0,60000,0.01);knob->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knob->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);addAndMakeVisible(knob);}
    fadeInKnob.onValueChange=[this]{if(!updating){fields[fadeIn].setText(juce::String(fadeInKnob.getValue(),3),false);commitField(fadeIn);}};
    fadeOutKnob.onValueChange=[this]{if(!updating){fields[fadeOut].setText(juce::String(fadeOutKnob.getValue(),3),false);commitField(fadeOut);}};
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    diagnosticsButton.onClick=[this]{diagnosticsOpen=!diagnosticsOpen;resized();refresh();};
    automaticButton.onClick=[this]{processor.swellEngine().setAutomaticProcessing(automaticButton.getToggleState());refresh();};
    sampleUnitsButton.onClick=[this]{refresh();};
    suppressionButton.onClick=[this]{processor.swellEngine().resetSuppression();refresh();};
    defaultIRButton.onClick=[this]
    {
        chooser=std::make_unique<juce::FileChooser>("Default IR for future swells",juce::File{},"*.wav;*.wv");
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
            [safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(this)](const juce::FileChooser& file)
            {if(safe && file.getResult()!=juce::File{})safe->processor.swellEngine().loadDefaultIR(file.getResult());});
    };
    wetOnlyButton.onClick=[this]{processor.swellEngine().setMonitoring(m3::AuditionMode::wetOnly,processor.swellEngine().savedSession().wetBlend);};
    dryWetButton.onClick=[this]{processor.swellEngine().setMonitoring(m3::AuditionMode::dryWet,processor.swellEngine().savedSession().wetBlend);};
    blendSlider.onValueChange=[this]{if(!updating){const auto value=static_cast<float>(blendSlider.getValue()/100);const auto saved=processor.swellEngine().savedSession();processor.swellEngine().setMonitoring(value>=1?m3::AuditionMode::wetOnly:m3::AuditionMode::dryWet,value>=1?saved.wetBlend:value);}};
    trimResetButton.onClick=[this]{const auto& s=state.session;processor.swellEngine().setPreferences(0,s.sensitivity,s.separationSeconds,s.autoRegenerate);refresh();};
    auditionButton.setButtonText("Audition");auditionLabel.setText("Monitoring trim",juce::dontSendNotification);
    inspectorViewport.setViewedComponent(&inspectorContent,false);diagnosticsViewport.setViewedComponent(&diagnosticsContent,false);
    inspectorViewport.setName("inspectorViewport");diagnosticsViewport.setName("diagnosticsViewport");
    inspectorViewport.setScrollBarsShown(true,false);diagnosticsViewport.setScrollBarsShown(true,false);
    addAndMakeVisible(inspectorViewport);addAndMakeVisible(diagnosticsViewport);
    for(auto* control:std::array<juce::Component*,14>{&selectedStatus,&irStatus,&loadIRButton,&enabledButton,&muteButton,&soloButton,&fullButton,&resetButton,&inCurveBox,&outCurveBox,&curveInLabel,&curveOutLabel,&gainLabel,&gainSlider})inspectorContent.addAndMakeVisible(control);
    inspectorContent.addAndMakeVisible(fadeInKnob);inspectorContent.addAndMakeVisible(fadeOutKnob);
    for(auto* label:{&inTensionLabel,&outTensionLabel}){inspectorContent.addAndMakeVisible(label);label->setColour(juce::Label::textColourId,m3::ui::secondary);label->setFont(juce::FontOptions(12));}
    inTensionLabel.setText("In tension",juce::dontSendNotification);outTensionLabel.setText("Out tension",juce::dontSendNotification);
    slider(inTensionSlider,-1,1,.001,"");slider(outTensionSlider,-1,1,.001,"");
    inTensionSlider.setName("inTension");outTensionSlider.setName("outTension");
    for(auto* control:{&inTensionSlider,&outTensionSlider})inspectorContent.addAndMakeVisible(control);
    inTensionSlider.onValueChange=[this]{if(!updating)processor.swellEngine().edit(state.session.primary,[this](auto& w,auto&){w.shape.inTension=inTensionSlider.getValue();return true;});};
    outTensionSlider.onValueChange=[this]{if(!updating)processor.swellEngine().edit(state.session.primary,[this](auto& w,auto&){w.shape.outTension=outTensionSlider.getValue();return true;});};
    slider(tailSlider,0,5,.01," s");tailSlider.setName("auditionTail");tailSlider.setTooltip("Captured Dry + Wet context after the later of the original arrival and wet endpoint. Dry audio is unchanged.");
    tailSlider.onValueChange=[this]{if(!updating)processor.swellEngine().setPostRoll(tailSlider.getValue());};
    addAndMakeVisible(tailLabel);tailLabel.setText("Listen after",juce::dontSendNotification);
    for(auto* label:{&markerLabel,&markerState}){addAndMakeVisible(label);label->setColour(juce::Label::textColourId,m3::ui::secondary);}
    markerLabel.setText("Marker sensitivity",juce::dontSendNotification);markerKnob.setName("markerSensitivity");markerButton.setName("autoMarkers");isolateButton.setName("isolateSelected");
    slider(markerKnob,0,100,.1," %");markerKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    markerKnob.setTooltip("Lower: fewer, stronger attacks. Higher: finer detail. Changes reanalyse captured audio and preserve manual edits. This does not detect musical keys.");
    addAndMakeVisible(markerButton);addAndMakeVisible(isolateButton);
    markerKnob.onValueChange=[this]{if(!updating){const auto s=processor.swellEngine().savedSession();processor.swellEngine().setPreferences(s.auditionDb,static_cast<float>(markerKnob.getValue()/100),s.separationSeconds,s.autoRegenerate);}};
    addAndMakeVisible(regenerationLabel);regenerationLabel.setText("Regeneration sensitivity",juce::dontSendNotification);
    slider(regenerationKnob,0,100,.1," %");regenerationKnob.setName("regenerationSensitivity");regenerationKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    regenerationKnob.setTooltip("Lower tolerates more performance variation against the retained capture; 100% compares exact samples. Separate from marker detection. Explicit recapture remains exact.");
    regenerationKnob.onValueChange=[this]{if(!updating)processor.swellEngine().setRegenerationSensitivity(static_cast<float>(regenerationKnob.getValue()/100));};
    inspectorContent.addAndMakeVisible(freezeButton);freezeButton.setName("freezeRender");
    freezeButton.setTooltip("Keep this rendered wet audio fixed against source changes. Unfreeze to change source/IR or extend render support. Envelope, gain and placement remain editable.");
    freezeButton.onClick=[this]{processor.swellEngine().setFrozen(state.session.primary,freezeButton.getToggleState());refresh();};
    markerButton.onClick=[this]{processor.swellEngine().setAutoMarkers(markerButton.getToggleState());refresh();};
    addAndMakeVisible(isolateDryButton);isolateDryButton.setName("isolateDry");
    isolateDryButton.setTooltip("Replay captured dry only within the selected source interval; overlapping mixed notes inside it cannot be separated. Live input is unchanged. Off includes the listening context.");
    isolateDryButton.onClick=[this]{processor.swellEngine().setIsolateDry(isolateDryButton.getToggleState());refresh();};
    isolateButton.onClick=[this]{processor.swellEngine().setIsolateSelected(isolateButton.getToggleState());refresh();};
    for(auto* c:std::array<juce::Component*,4>{&placementLabel,&placementEditor,&nudgeLeft,&nudgeRight})inspectorContent.addAndMakeVisible(c);
    placementLabel.setText("Move wet (ms)",juce::dontSendNotification);placementEditor.setName("wetPlacement");placementEditor.setInputRestrictions(40,"-0123456789.");
    placementEditor.onReturnKey=[this]{if(updating)return;const auto text=placementEditor.getText();const auto rate=processor.getSongTime().rate;
        const auto value=text.getDoubleValue()*rate/1000;
        if(!std::isfinite(rate) || rate<=0 || text.isEmpty() || text=="-" || text=="." || text=="-." || text.indexOfChar('.')!=text.lastIndexOfChar('.') || text.lastIndexOfChar('-')>0 || !std::isfinite(value) || value<=static_cast<double>(INT64_MIN) || value>=static_cast<double>(INT64_MAX))
        {processor.swellEngine().diagnostic("Enter a finite signed wet placement in milliseconds");return;}
        processor.swellEngine().edit(state.session.primary,[&](auto& w,auto& error){return m3::moveWet(w,static_cast<int64_t>(std::llround(value)),error);});refresh();};
    placementEditor.onFocusLost=placementEditor.onReturnKey;
    nudgeLeft.onClick=[this]{processor.swellEngine().edit(state.session.primary,[](auto& w,auto& error){int64_t value;return m3::checkedAdd(w.placement,-1,value) && m3::moveWet(w,value,error);});refresh();};
    nudgeRight.onClick=[this]{processor.swellEngine().edit(state.session.primary,[](auto& w,auto& error){int64_t value;return m3::checkedAdd(w.placement,1,value) && m3::moveWet(w,value,error);});refresh();};
    nudgeLeft.setTooltip("Move wet one sample earlier");nudgeRight.setTooltip("Move wet one sample later");
    inspectorContent.addAndMakeVisible(timingButton);timingButton.setName("advancedTiming");
    timingButton.onClick=[this]{advancedTiming=!advancedTiming;resized();};
    // The swell name is renamed from the list row menu, not the Selected panel.
    for(int i=arrival;i<fieldCount;++i){inspectorContent.addAndMakeVisible(fieldLabels[i]);inspectorContent.addAndMakeVisible(fields[i]);}
    fields[name].setVisible(false);fieldLabels[name].setVisible(false);
    for(auto* control:std::array<juce::Component*,30>{&status,&detail,&generateButton,&allButton,&analyzeButton,&confirmButton,&candidateBox,&sensitivityLabel,&sensitivitySlider,&separationLabel,&separationSlider,&autoButton,&recaptureButton,&cancelButton,&recaptureStatus,&mappingButton,&toneButton,&toneStartLabel,&toneStartSlider,&toneEndLabel,&toneEndSlider,&clearButton,&assetInfoButton,&barStartLabel,&barEndLabel,&barStartEditor,&barEndEditor,&setBarsButton,&auditionLabel,&auditionSlider})diagnosticsContent.addAndMakeVisible(control);
    for(auto* control:std::array<juce::Component*,4>{&automaticButton,&sampleUnitsButton,&defaultIRButton,&suppressionButton})diagnosticsContent.addAndMakeVisible(control);
    for(auto* control:std::array<juce::Component*,12>{&removeButton,&liveButton,&liveRoutingBox,&liveLevel,&liveLevelLabel,&wetOnlyButton,&dryWetButton,&isolateButton,&isolateDryButton,&tailSlider,&tailLabel,&helper})diagnosticsContent.addAndMakeVisible(control);
    addAndMakeVisible(wordmark);wordmark.setName("brandWordmark");
    for(auto* label:{&tagline,&inspectorHeading,&hostInfo,&listEmpty,&inspectorEmpty,&auditionHeading,&editorReadout})
    {addAndMakeVisible(label);label->setColour(juce::Label::textColourId,m3::ui::text);label->setInterceptsMouseClicks(false,false);}
    tagline.setText("Reverse Reverb for Creative Swells",juce::dontSendNotification);tagline.setFont(juce::FontOptions(21,juce::Font::italic));
    inspectorHeading.setText("Selected Swell",juce::dontSendNotification);inspectorHeading.setFont(juce::FontOptions(21,juce::Font::bold));
    auditionHeading.setText("Audition",juce::dontSendNotification);auditionHeading.setFont(juce::FontOptions(21,juce::Font::bold));
    hostInfo.setName("hostInfo");hostInfo.setFont(juce::FontOptions(15));hostInfo.setJustificationType(juce::Justification::centred);
    listEmpty.setText("No swells yet\n\nPlay your song to map audio,\nor add a swell with +.",juce::dontSendNotification);listEmpty.setJustificationType(juce::Justification::centred);listEmpty.setFont(juce::FontOptions(18));
    inspectorEmpty.setText("No swell selected\n\nSelect a swell to adjust\nits settings.",juce::dontSendNotification);inspectorEmpty.setJustificationType(juce::Justification::centred);inspectorEmpty.setFont(juce::FontOptions(18));
    listLabel.setFont(juce::FontOptions(21,juce::Font::bold));
    title.setVisible(false);headerStatus.setBorderSize(juce::BorderSize<int>(0));headerStatus.setFont(juce::FontOptions(12));
    headerStatus.setJustificationType(juce::Justification::centredRight);
    diagnosticsButton.setTooltip("Settings and recovery controls (dedicated diagnostics cleanup follows after visual review)");
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
    irBox.setTooltip("This swell's IR. Preverb Hall is bundled; Load file assigns WAV or WavPack to this swell only.");
    irBox.onChange=[this]{if(updating || irBox.getSelectedId()!=1)return;const auto w=m3::find(state.session,irOwner);
        if(w)processor.swellEngine().assignIR(w->id,w->inputGeneration,{},true);};
    loadIRButton.setButtonText({});loadIRButton.setTooltip("Load a mono/stereo WAV or WavPack IR file for this swell");
    resetButton.setButtonText("Reset");resetButton.setName("resetWetShape");
    resetButton.setTooltip("Reset only the wet shape: start/end, fades, curves, level and placement return to defaults. Source points, IR and activity stay unchanged.");
    inspectorContent.addAndMakeVisible(resetMenu);resetMenu.setName("resetSwellMenu");resetMenu.setButtonText("Reset All");
    resetMenu.setTooltip("Reset this whole swell back to creation defaults: wet shape plus dry source points, IR, enabled/solo/freeze and activity.");
    resetMenu.onClick=[this]{const auto id=state.session.primary;processor.swellEngine().resetSwell(id,true);refresh();};
    freezeButton.setButtonText("Freeze");
    list.setRowHeight(64);setSize(1400,1000);refresh();startTimerHz(10);
}
PreverbAudioProcessorEditor::~PreverbAudioProcessorEditor(){destroying=true;stopTimer();list.setModel(nullptr);setLookAndFeel(nullptr);}
void PreverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(m3::ui::background);const m3::ui::WorkspaceLayout layout(getLocalBounds());g.setColour(m3::ui::separator);
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
    g.setColour(m3::ui::text);g.setFont(14.0f);g.drawText(w.name,32,2,width-62,20,juce::Justification::left);
    g.drawRect(8,8,15,15);if(w.enabled){g.setColour(m3::ui::aqua);g.fillRect(11,11,9,9);}
    g.setFont(12.0f);g.drawText(w.irName.isNotEmpty()?w.irName:"No IR",32,23,width-62,18,juce::Justification::left);
    g.setFont(11.0f);g.setColour(w.phase==m3::Phase::error?juce::Colour(0xffedaa91):m3::ui::secondary);
    g.drawText((w.phase==m3::Phase::ready?phaseName(w.phase):w.status)+(w.muted?" | mute":"")+(!w.enabled?" | off":"")+(w.solo?" | solo":"")+(w.frozen?" | frozen":""),32,43,width-62,height-43,juce::Justification::left);
    g.setColour(m3::ui::secondary);
    for(int n=0;n<3;++n)g.fillEllipse(juce::Rectangle<float>(3,3).withCentre({static_cast<float>(width-13),static_cast<float>(height/2-7+n*7)}));
}
void PreverbAudioProcessorEditor::listBoxItemClicked(int row,const juce::MouseEvent& event)
{
    if(row<0 || row>=getNumRows())return;
    const auto& w=state.session.swells[static_cast<size_t>(row)];
    if(event.x>=event.eventComponent->getWidth()-26)
    {
        juce::PopupMenu menu;menu.addItem(1,"Rename...");menu.addItem(2,"Delete");
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(list),
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
{processor.swellEngine().setPreferences(static_cast<float>(auditionSlider.getValue()),static_cast<float>(sensitivitySlider.getValue()),separationSlider.getValue()/1000,autoButton.getToggleState());}
void PreverbAudioProcessorEditor::commitField(int field)
{
    if(updating || destroying)return;const auto text=fields[field].getText();const auto raw=state.raw;const auto time=processor.getSongTime();
    processor.swellEngine().edit(state.session.primary,[&](m3::Swell& w,juce::String& error)
    {
        if(field==name){w.name=text;return true;}
        int64_t value=text.getLargeIntValue();
        if(field<fadeIn)
        {
            if(sampleUnitsButton.getToggleState()) {if(juce::String(value)!=text){error="Enter an exact sample position";return false;}}
            else if(!m3::parseMusicalPosition(text,time,value)){error="Enter bar.beat or bar.beat.fraction (up to nine fractional digits), or use exact samples in Diagnostics";return false;}
        }
        if(field==arrival)return m3::moveArrival(w,value,error);
        if(field==sourceA)return m3::setSource(w,value,w.sourceEnd,error);
        if(field==sourceB)return m3::setSource(w,w.sourceStart,value,error);
        if(!raw){error="Generate before editing audible bounds";return false;}auto shape=w.shape;
        if(field==audibleA){shape.automaticStart=false;int64_t logical;if(!m3::checkedSubtract(value,w.placement,logical) || !m3::checkedSubtract(logical,w.arrival,shape.startOffset))return false;}
        if(field==audibleB){int64_t logical;if(!m3::checkedSubtract(value,w.placement,logical) || !m3::checkedSubtract(logical,w.arrival,shape.endOffset))return false;}
        if(field>=fadeIn)
        {
            const auto ms=text.getDoubleValue(),samples=ms*time.rate/1000;
            if(text.isEmpty() || text.containsOnly(".") || !text.containsOnly("0123456789.") || text.indexOfChar('.')!=text.lastIndexOfChar('.')
                || !std::isfinite(samples) || ms<0 || samples>=static_cast<double>(INT64_MAX)){error="Invalid fade length";return false;}
            if(field==fadeIn)shape.fadeIn=static_cast<int64_t>(std::llround(samples));else shape.fadeOut=static_cast<int64_t>(std::llround(samples));
        }
        return m3::setShape(w,shape,raw->start,raw->end,error);
    });refresh();
}
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
    {if(full){if(!raw)return false;return m3::fullShape(w,raw->start,raw->end,error);}m3::resetShape(w);return true;});refresh();
}
void PreverbAudioProcessorEditor::armSelected()
{
    int64_t a=INT64_MAX,b=INT64_MIN;
    for(const auto& id:state.session.selected)if(const auto w=m3::find(state.session,id)){a=std::min(a,w->sourceStart);b=std::max(b,w->sourceEnd);}
    if(b>a)processor.armRecapture(a,b);refresh();
}
void PreverbAudioProcessorEditor::fieldBounds(juce::Rectangle<int>& row,int field,int labelWidth,int editorWidth)
{fieldLabels[field].setBounds(row.removeFromLeft(labelWidth));fields[field].setBounds(row.removeFromLeft(editorWidth).reduced(2,4));}
void PreverbAudioProcessorEditor::resized()
{
    const m3::ui::WorkspaceLayout layout(getLocalBounds());
    auto top=layout.header;diagnosticsButton.setBounds(top.removeFromRight(42).withSizeKeepingCentre(36,32));
    wordmark.setBounds(top.removeFromLeft(getWidth()<1200?250:290).reduced(0,5));
    top.removeFromLeft(20);tagline.setBounds(top.withSizeKeepingCentre(top.getWidth(),43));
    auto globals=layout.globals.reduced(0,8);
    markerButton.setBounds(globals.removeFromLeft(160));globals.removeFromLeft(16);
    markerLabel.setBounds(globals.removeFromLeft(118));markerKnob.setBounds(globals.removeFromLeft(150));globals.removeFromLeft(24);
    regenerationLabel.setBounds(globals.removeFromLeft(180));regenerationKnob.setBounds(globals.removeFromLeft(150));globals.removeFromLeft(16);
    markerState.setBounds(globals.removeFromLeft(110));headerStatus.setBounds(globals);
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
    diagnosticsButton.setButtonText("Settings");diagnosticsViewport.setVisible(diagnosticsOpen);
    // Interim, real settings/recovery controls remain available while the user
    // reviews the shell. Unlike the old inline fold, this overlay never moves
    // the underlying bands. Task 8 replaces it with the dedicated window.
    diagnosticsViewport.setBounds(diagnosticsOpen?juce::Rectangle<int>(layout.workspace.getX(),layout.workspace.getBottom()-210,layout.workspace.getWidth(),210):juce::Rectangle<int>{});
    if(diagnosticsOpen)diagnosticsViewport.toFront(false);
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
    // Legacy exact-bound and curve-preset tools are temporarily grouped in
    // Settings, never overlaid on the paired main inspector controls.
    for(auto* c:std::array<juce::Component*,17>{&muteButton,&fullButton,&inCurveBox,&outCurveBox,&gainSlider,&gainLabel,&fadeInKnob,&fadeOutKnob,&inTensionLabel,&outTensionLabel,&inTensionSlider,&outTensionSlider,&placementLabel,&placementEditor,&nudgeLeft,&nudgeRight,&timingButton})diagnosticsContent.addAndMakeVisible(c);
    for(int field=arrival;field<fieldCount;++field){diagnosticsContent.addAndMakeVisible(fieldLabels[field]);diagnosticsContent.addAndMakeVisible(fields[field]);}
    curveInLabel.setVisible(false);curveOutLabel.setVisible(false);
    const auto diagnosticsWidth=std::max(1000,getWidth()-50);diagnosticsContent.setSize(diagnosticsWidth,850);
    auto diagnostic=diagnosticsContent.getLocalBounds().reduced(4);
    status.setBounds(diagnostic.removeFromTop(52));detail.setBounds(diagnostic.removeFromTop(82));
    row=diagnostic.removeFromTop(34);
    for(auto* button:{&generateButton,&allButton,&analyzeButton,&confirmButton})button->setBounds(row.removeFromLeft(150).reduced(2));candidateBox.setBounds(row.reduced(2));
    row=diagnostic.removeFromTop(34);sensitivityLabel.setBounds(row.removeFromLeft(85));sensitivitySlider.setBounds(row.removeFromLeft(190));separationLabel.setBounds(row.removeFromLeft(105));separationSlider.setBounds(row.removeFromLeft(180));
    automaticButton.setBounds(row.removeFromLeft(180));autoButton.setBounds(row);
    row=diagnostic.removeFromTop(34);mappingButton.setBounds(row.removeFromLeft(170));clearButton.setBounds(row.removeFromLeft(95).reduced(2));recaptureButton.setBounds(row.removeFromLeft(205).reduced(2));cancelButton.setBounds(row.removeFromLeft(140).reduced(2));sampleUnitsButton.setBounds(row.removeFromLeft(195));assetInfoButton.setBounds(row.reduced(2));
    row=diagnostic.removeFromTop(34);toneButton.setBounds(row.removeFromLeft(160));toneStartLabel.setBounds(row.removeFromLeft(90));toneStartSlider.setBounds(row.removeFromLeft(180));toneEndLabel.setBounds(row.removeFromLeft(95));toneEndSlider.setBounds(row.removeFromLeft(180));auditionLabel.setBounds(row.removeFromLeft(100));auditionSlider.setBounds(row);
    row=diagnostic.removeFromTop(34);barStartLabel.setBounds(row.removeFromLeft(65));barStartEditor.setBounds(row.removeFromLeft(70).reduced(2));barEndLabel.setBounds(row.removeFromLeft(65));barEndEditor.setBounds(row.removeFromLeft(70).reduced(2));setBarsButton.setBounds(row.removeFromLeft(175).reduced(2));defaultIRButton.setBounds(row.removeFromLeft(200).reduced(2));suppressionButton.setBounds(row.removeFromLeft(175).reduced(2));
    row=diagnostic.removeFromTop(34);liveButton.setBounds(row.removeFromLeft(125));liveRoutingBox.setBounds(row.removeFromLeft(125));liveLevelLabel.setBounds(row.removeFromLeft(60));liveLevel.setBounds(row.removeFromLeft(260));removeButton.setBounds(row.removeFromLeft(90));
    row=diagnostic.removeFromTop(34);wetOnlyButton.setBounds(row.removeFromLeft(95));dryWetButton.setBounds(row.removeFromLeft(105));isolateButton.setBounds(row.removeFromLeft(140));isolateDryButton.setBounds(row.removeFromLeft(160));tailLabel.setBounds(row.removeFromLeft(80));tailSlider.setBounds(row);
    row=diagnostic.removeFromTop(34);muteButton.setBounds(row.removeFromLeft(80));fullButton.setBounds(row.removeFromLeft(140));inCurveBox.setBounds(row.removeFromLeft(130));outCurveBox.setBounds(row.removeFromLeft(130));timingButton.setBounds(row.removeFromLeft(210));
    row=diagnostic.removeFromTop(36);gainLabel.setBounds(row.removeFromLeft(85));gainSlider.setBounds(row.removeFromLeft(170));inTensionSlider.setBounds(row.removeFromLeft(180));outTensionSlider.setBounds(row.removeFromLeft(180));
    for(int field=arrival;field<fieldCount;++field){row=diagnostic.removeFromTop(30);fieldLabels[field].setBounds(row.removeFromLeft(180));fields[field].setBounds(row.removeFromLeft(250).reduced(1,3));}
    row=diagnostic.removeFromTop(34);placementLabel.setBounds(row.removeFromLeft(150));placementEditor.setBounds(row.removeFromLeft(180));nudgeLeft.setBounds(row.removeFromLeft(26));nudgeRight.setBounds(row.removeFromLeft(26));
    helper.setBounds(diagnostic.removeFromTop(20));recaptureStatus.setBounds(diagnostic);
}
void PreverbAudioProcessorEditor::refresh()
{
    if(destroying)return;updating=true;state=processor.swellEngine().snapshot();const auto capture=processor.getCaptureSnapshot();const auto time=processor.getSongTime();const auto diagnostics=processor.getHostPositionDiagnostics();
    timeline.setData(capture,state,time,diagnostics.samplePositionAvailable);
    hostInfo.setText((diagnostics.samplePositionAvailable && time.valid?m3::musicalPosition(time.sample,time):juce::String("Position unavailable"))
        +"    |    "+(diagnostics.tempoAvailable?juce::String(diagnostics.bpm,2)+" BPM":juce::String("BPM unavailable"))
        +"    |    "+(diagnostics.meterAvailable?juce::String(diagnostics.numerator)+" / "+juce::String(diagnostics.denominator):juce::String("Meter unavailable")),juce::dontSendNotification);
    liveButton.setToggleState(state.session.liveEnabled,juce::dontSendNotification);
    liveRoutingBox.setSelectedId(state.session.liveRouting==m3::LiveRouting::wetOnly?2:1,juce::dontSendNotification);
    if(!liveLevel.isMouseButtonDown() && !liveLevel.hasKeyboardFocus(true))liveLevel.setValue(state.session.liveWetDb,juce::dontSendNotification);
    const auto liveText=(state.liveStatus.startsWith("Export incomplete") || state.liveStatus.startsWith("Export warning"))?state.liveStatus:
        !state.session.liveEnabled?juce::String("Live off | dry passthrough"):
        state.liveIncompatible?juce::String("Live: awaiting compatible timing/preparation"):
        state.liveMissing?juce::String("Live: missing prepared coverage | ")+state.liveStatus:
        (diagnostics.isPlaying?juce::String("Live Song | "):juce::String("Stopped | "))+state.liveStatus+" | guard "+juce::String(state.liveAttenuationDb,1)+" dB";
    helper.setText(liveText,juce::dontSendNotification);helper.setTooltip(liveText);
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
    curveInLabel.setVisible(false);curveOutLabel.setVisible(false);
    refreshValues(w,time);
    listLabel.setText("Swells ("+juce::String(static_cast<int>(state.session.swells.size()))+")",juce::dontSendNotification);
    const auto key=w?w->id:juce::String{};
    if(key!=fittedKey && state.raw){timeline.fitSelection();fittedKey=key;}
    list.updateContent();juce::SparseSet<int> selectedRows;
    for(size_t i=0;i<state.session.swells.size();++i)if(std::find(state.session.selected.begin(),state.session.selected.end(),state.session.swells[i].id)!=state.session.selected.end())selectedRows.addRange({static_cast<int>(i),static_cast<int>(i+1)});
    list.setSelectedRows(selectedRows,juce::dontSendNotification);list.repaint();
    const auto setText=[](juce::TextEditor& field,const juce::String& text){if(!field.hasKeyboardFocus(true))field.setText(text,false);};
    for(int i=0;i<fieldCount;++i)fields[i].setEnabled(w!=nullptr && (i<audibleA || state.raw!=nullptr));
    if(w)
    {
        const auto position=[&](int64_t sample){return sampleUnitsButton.getToggleState()?juce::String(sample):m3::musicalPosition(sample,time);};
        setText(fields[name],w->name);setText(fields[arrival],position(w->arrival));setText(fields[sourceA],position(w->sourceStart));setText(fields[sourceB],position(w->sourceEnd));
        int64_t a=0,b=0;juce::String error;
        if(state.raw)
        {
            const auto valid=m3::shapeBounds(*w,state.raw->start,state.raw->end,a,b,error);
            if(valid){setText(fields[audibleA],position(a));setText(fields[audibleB],position(b));}
            else
            {
                const auto startValid=w->shape.automaticStart?(a=state.raw->start,true):m3::checkedAdd(w->arrival,w->shape.startOffset,a);if(startValid)m3::checkedAdd(a,w->placement,a);
                const auto endValid=m3::checkedAdd(w->arrival,w->shape.endOffset,b);if(endValid)m3::checkedAdd(b,w->placement,b);
                setText(fields[audibleA],startValid?position(a):juce::String("overflow"));setText(fields[audibleB],endValid?position(b):juce::String("overflow"));
            }
        }
        const auto rate=capture.manifest.sampleRate>0?capture.manifest.sampleRate:48000;
        const auto maximumFade=std::max({1.0,w->shape.fadeIn*1000.0/rate,w->shape.fadeOut*1000.0/rate,state.raw?(state.raw->end-state.raw->start)*1000.0/rate:1.0});
        fadeInKnob.setRange(0,maximumFade,.01);fadeOutKnob.setRange(0,maximumFade,.01);
        setText(fields[fadeIn],juce::String(w->shape.fadeIn*1000.0/rate,3));setText(fields[fadeOut],juce::String(w->shape.fadeOut*1000.0/rate,3));
        if(!fadeInKnob.isMouseButtonDown())fadeInKnob.setValue(w->shape.fadeIn*1000.0/rate,juce::dontSendNotification);
        if(!fadeOutKnob.isMouseButtonDown())fadeOutKnob.setValue(w->shape.fadeOut*1000.0/rate,juce::dontSendNotification);
        if(!inTensionSlider.isMouseButtonDown() && !inTensionSlider.hasKeyboardFocus(true))inTensionSlider.setValue(w->shape.inTension,juce::dontSendNotification);
        if(!outTensionSlider.isMouseButtonDown() && !outTensionSlider.hasKeyboardFocus(true))outTensionSlider.setValue(w->shape.outTension,juce::dontSendNotification);
        setText(placementEditor,juce::String(w->placement*1000.0/rate,3));
        inCurveBox.setSelectedId(static_cast<int>(w->shape.inCurve)+1,juce::dontSendNotification);outCurveBox.setSelectedId(static_cast<int>(w->shape.outCurve)+1,juce::dontSendNotification);
        if(!gainSlider.isMouseButtonDown() && !gainSlider.hasKeyboardFocus(true))gainSlider.setValue(w->shape.gainDb,juce::dontSendNotification);
        freezeButton.setToggleState(w->frozen,juce::dontSendNotification);enabledButton.setToggleState(w->enabled,juce::dontSendNotification);muteButton.setToggleState(w->muted,juce::dontSendNotification);soloButton.setToggleState(w->solo,juce::dontSendNotification);
    }
    else{for(auto& field:fields)if(!field.hasKeyboardFocus(true))field.setText({},false);}
    placementEditor.setEnabled(w && state.raw);nudgeLeft.setEnabled(w && state.raw);nudgeRight.setEnabled(w && state.raw);
    if(!w)placementEditor.setText({},false);
    if(!markerKnob.isMouseButtonDown() && !markerKnob.hasKeyboardFocus(true))markerKnob.setValue(state.session.sensitivity*100,juce::dontSendNotification);
    if(!regenerationKnob.isMouseButtonDown() && !regenerationKnob.hasKeyboardFocus(true))regenerationKnob.setValue(state.session.regenerationSensitivity*100,juce::dontSendNotification);
    markerButton.setToggleState(state.session.autoMarkersEnabled,juce::dontSendNotification);
    markerState.setText(!state.session.autoMarkersEnabled?"Off":!state.session.automaticProcessing?"Master paused":state.analyzing?"Analysing":"On",juce::dontSendNotification);
    isolateDryButton.setToggleState(state.session.isolateDry,juce::dontSendNotification);isolateDryButton.setEnabled(state.session.auditionMode==m3::AuditionMode::dryWet);
    isolateButton.setToggleState(state.session.isolateSelected,juce::dontSendNotification);
    if(!tailSlider.isMouseButtonDown() && !tailSlider.hasKeyboardFocus(true))tailSlider.setValue(state.session.postRollSeconds,juce::dontSendNotification);
    if(!auditionSlider.isMouseButtonDown() && !auditionSlider.hasKeyboardFocus(true))auditionSlider.setValue(state.session.auditionDb,juce::dontSendNotification);
    if(!sensitivitySlider.isMouseButtonDown())sensitivitySlider.setValue(state.session.sensitivity,juce::dontSendNotification);
    if(!separationSlider.isMouseButtonDown())separationSlider.setValue(state.session.separationSeconds*1000,juce::dontSendNotification);
    autoButton.setToggleState(state.session.autoRegenerate,juce::dontSendNotification);
    automaticButton.setToggleState(state.session.automaticProcessing,juce::dontSendNotification);
    if(!blendSlider.isMouseButtonDown() && !blendSlider.hasKeyboardFocus(true))blendSlider.setValue(state.session.auditionMode==m3::AuditionMode::wetOnly?100:state.session.wetBlend*100,juce::dontSendNotification);
    wetOnlyButton.setToggleState(state.session.auditionMode==m3::AuditionMode::wetOnly,juce::dontSendNotification);
    dryWetButton.setToggleState(state.session.auditionMode==m3::AuditionMode::dryWet,juce::dontSendNotification);
    trimResetButton.setButtonText("Trim "+juce::String(state.session.auditionDb,1)+" dB"+(state.session.auditionDb!=0?" | reset":""));
    monitorStatus.setText(state.dryStatus.isNotEmpty()?state.dryStatus:(state.session.isolateSelected?juce::String("Isolated"):juce::String("Overlaps"))+" | "+juce::String(static_cast<int>(state.auditionMembers.size()))+" wet | "+(state.session.isolateDry?"selected dry":"dry context")+" | live dry unchanged",juce::dontSendNotification);monitorStatus.setTooltip(state.dryStatus);
    selectedStatus.setText(w?phaseName(w->phase)+" | "+w->name:"Select a swell",juce::dontSendNotification);
    selectedStatus.setTooltip(w?w->status:state.message);
    irStatus.setText("Impulse Response (IR)",juce::dontSendNotification);
    const auto choice=w?w->ir.content+"|"+w->irName:juce::String{};
    if(irOwner!=(w?w->id:juce::String{}) || irChoice!=choice)
    {
        irOwner=w?w->id:juce::String{};irChoice=choice;irBox.clear(juce::dontSendNotification);irBox.addItem("Preverb Hall",1);
        if(w && w->ir.present() && w->irName!="Preverb Hall"){irBox.addItem(w->irName,2);irBox.setSelectedId(2,juce::dontSendNotification);}
        else if(w && w->ir.present())irBox.setSelectedId(1,juce::dontSendNotification);
        else irBox.setTextWhenNothingSelected("Choose an IR");
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
    else if(state.defaultIRError.isNotEmpty())aggregate="Missing default IR | choose a replacement in Diagnostics";
    aggregate<<" | "<<ready<<" ready";if(updatingCount)aggregate<<" | "<<updatingCount<<" updating";if(waiting)aggregate<<" | "<<waiting<<" waiting";if(failed)aggregate<<" | "<<failed<<" need attention";
    if(!state.session.automaticProcessing)aggregate<<" | Manual processing";
    if((state.liveStatus.startsWith("Export incomplete") || state.liveStatus.startsWith("Export warning")))aggregate=state.liveStatus;
    if(state.liveStatus.startsWith("Export warning"))aggregate=state.liveStatus.upToFirstOccurrenceOf(" | ",false,false);
    if(!state.session.liveEnabled)aggregate<<" | Live off";
    if(state.actionError.isNotEmpty())aggregate=state.actionError;
    headerStatus.setText(aggregate,juce::dontSendNotification);headerStatus.setTooltip((state.liveStatus.startsWith("Export incomplete") || state.liveStatus.startsWith("Export warning"))?state.liveStatus:state.message);
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
    for(auto* control:std::array<juce::Component*,7>{&gainSlider,&enabledButton,&muteButton,&soloButton,&inCurveBox,&outCurveBox,&setBarsButton})control->setEnabled(w!=nullptr);
    if(w && w->frozen){for(int field:{arrival,sourceA,sourceB})fields[field].setEnabled(false);setBarsButton.setEnabled(false);}
    inTensionSlider.setEnabled(w && state.raw);outTensionSlider.setEnabled(w && state.raw);
    fadeInKnob.setEnabled(w && state.raw);fadeOutKnob.setEnabled(w && state.raw);timingButton.setEnabled(w!=nullptr);
    mappingButton.setToggleState(processor.isMappingEnabled(),juce::dontSendNotification);toneButton.setToggleState(processor.isTestToneEnabled(),juce::dontSendNotification);
    toneStartSlider.setValue(processor.getToneStartBar(),juce::dontSendNotification);toneEndSlider.setValue(processor.getToneEndBar(),juce::dontSendNotification);
    juce::String diagnostic="Transport: ";diagnostic<<(!diagnostics.positionAvailable?"unavailable":diagnostics.isPlaying?"playing":"stopped")<<" | sample "<<(diagnostics.samplePositionAvailable?juce::String(time.sample):"unavailable")<<" | PPQ "<<(diagnostics.ppqPositionAvailable?juce::String(time.ppq,3):"unavailable")
        <<" | "<<(diagnostics.tempoAvailable?juce::String(time.bpm,2):"unavailable")<<" BPM | "<<(diagnostics.meterAvailable?juce::String(time.numerator)+"/"+juce::String(time.denominator):"meter unavailable")<<" | "<<juce::String(time.rate,0)<<" Hz | block "<<diagnostics.blockSize
        <<"\nCapture: "<<capture.status<<" | revision "<<juce::String(static_cast<juce::int64>(capture.manifest.revision))<<" | dropped "<<juce::String(static_cast<juce::int64>(processor.getDroppedCaptureBlockCount()))<<" | discontinuities "<<juce::String(static_cast<juce::int64>(processor.getTimelineDiscontinuityCount()));
    diagnostic<<"\nOffline blocks: "<<juce::String(static_cast<juce::int64>(state.offlineBlocks))<<" | "<<state.offlineStatus;
    status.setText(diagnostic,juce::dontSendNotification);status.setTooltip(diagnostic);
    juce::String text=state.message;
    if(w)
    {
        text=w->name+": "+(w->phase==m3::Phase::ready && !state.shaped?juce::String("Loading / preparing edited preview"):w->status);
        if(w->phase==m3::Phase::rendering)text<<" ("<<juce::String(w->progress*100,0)<<"%)";
        text<<"\nIR: "<<(w->ir.present()?w->irName:juce::String("not assigned"))<<" | source ["<<juce::String(w->sourceStart)<<", "<<juce::String(w->sourceEnd)<<")";
        if(state.raw)text<<" | full wet ["<<juce::String(state.raw->start)<<", "<<juce::String(state.raw->end)<<") | "<<juce::String(w->render.frames)<<" frames";
        if(state.raw && state.shaped)text<<"\nRaw peak "<<peakText(state.raw->peak)<<" | shaped "<<peakText(state.shaped->peak)<<" | audition "<<peakText(state.monitorPeak)<<" (aggregate wet guard "<<juce::String(state.attenuationDb,1)<<" dB; sum guard "<<juce::String(state.blendAttenuationDb,1)<<" dB)";
    }
    if(w && state.message.isNotEmpty())text<<"\n"<<state.message;
    detail.setText(text,juce::dontSendNotification);detail.setTooltip(text);
    const auto arm=processor.getRecaptureSnapshot();cancelButton.setEnabled(arm.active);recaptureButton.setEnabled(hasSelection && !arm.active);
    juce::String recapture=arm.status;
    if(arm.active)
    {
        recapture="Range ["+juce::String(arm.start)+", "+juce::String(arm.end)+"): fresh "+juce::String(arm.covered)+" / "+juce::String(arm.end-arm.start)+" samples. ";
        if(time.valid){const auto barLength=time.rate*60/time.bpm*time.numerator*4/time.denominator,origin=time.sample-time.ppq*time.rate*60/time.bpm;recapture<<"Move playhead to bar "<<juce::String(1+(arm.start-origin)/barLength,3)<<" and press Play.";}
        else recapture<<arm.status;
    }
    recaptureStatus.setText(recapture,juce::dontSendNotification);recaptureStatus.setTooltip(recapture);updating=false;
}
