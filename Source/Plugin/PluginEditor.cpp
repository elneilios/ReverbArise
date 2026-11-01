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
    setResizable(true,true);setResizeLimits(1100,850,2000,1400);
    title.setText("Preverb | Swell editor",juce::dontSendNotification);title.setFont(juce::FontOptions(22.0f,juce::Font::bold));
    for(auto* label:{&title,&status,&detail,&listLabel,&auditionLabel,&sensitivityLabel,&separationLabel,&spanLabel,&curveInLabel,&curveOutLabel,&gainLabel,&barStartLabel,&barEndLabel,&recaptureStatus,&toneStartLabel,&toneEndLabel})
    {label->setColour(juce::Label::textColourId,juce::Colour(0xffc5d3d8));label->setJustificationType(juce::Justification::centredLeft);addAndMakeVisible(label);}
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
    list.setName("swellList");list.setMultipleSelectionEnabled(true);list.setRowHeight(44);list.setColour(juce::ListBox::backgroundColourId,juce::Colour(0xff10181c));
    addAndMakeVisible(list);addAndMakeVisible(timeline);
    addAndMakeVisible(songOverview);timeline.setInlineOverview(false);timeline.onEnvelopeEdited=[this]{refresh();};
    const std::array<juce::String,fieldCount> labels{"Name","Arrival (sample)","Source start (sample)","Source end exclusive","Audible start (sample)","Audible end exclusive","Fade in (ms)","Fade out (ms)"};
    const std::array<juce::String,fieldCount> names{"swellName","arrivalSample","sourceSampleStart","sourceSampleEnd","audibleStart","audibleEnd","fadeInMs","fadeOutMs"};
    for(int i=0;i<fieldCount;++i)
    {
        fieldLabels[i].setText(labels[i],juce::dontSendNotification);fieldLabels[i].setColour(juce::Label::textColourId,juce::Colour(0xffb7c4c9));fieldLabels[i].setFont(juce::FontOptions(12.0f));addAndMakeVisible(fieldLabels[i]);
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
        chooser=std::make_unique<juce::FileChooser>("Load mono/stereo WAV or WavPack IR",juce::File{},"*.wav;*.wv");
        chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
            [safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(this)](const juce::FileChooser& dialog)
            {if(safe && dialog.getResult()!=juce::File{})safe->processor.swellEngine().loadIR(dialog.getResult(),true);});
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
    {addAndMakeVisible(label);label->setColour(juce::Label::textColourId,juce::Colour(0xffcbd5d5));}
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
    blendSlider.onValueChange=[this]{if(!updating)processor.swellEngine().setMonitoring(processor.swellEngine().savedSession().auditionMode,static_cast<float>(blendSlider.getValue()/100));};
    trimResetButton.onClick=[this]{const auto& s=state.session;processor.swellEngine().setPreferences(0,s.sensitivity,s.separationSeconds,s.autoRegenerate);refresh();};
    auditionButton.setButtonText("Audition");auditionLabel.setText("Monitoring trim",juce::dontSendNotification);
    inspectorViewport.setViewedComponent(&inspectorContent,false);diagnosticsViewport.setViewedComponent(&diagnosticsContent,false);
    inspectorViewport.setName("inspectorViewport");diagnosticsViewport.setName("diagnosticsViewport");
    inspectorViewport.setScrollBarsShown(true,false);diagnosticsViewport.setScrollBarsShown(true,false);
    addAndMakeVisible(inspectorViewport);addAndMakeVisible(diagnosticsViewport);
    for(auto* control:std::array<juce::Component*,14>{&selectedStatus,&irStatus,&loadIRButton,&enabledButton,&muteButton,&soloButton,&fullButton,&resetButton,&inCurveBox,&outCurveBox,&curveInLabel,&curveOutLabel,&gainLabel,&gainSlider})inspectorContent.addAndMakeVisible(control);
    inspectorContent.addAndMakeVisible(fadeInKnob);inspectorContent.addAndMakeVisible(fadeOutKnob);
    for(auto* label:{&transitionStatus,&supportStatus,&inTensionLabel,&outTensionLabel}){inspectorContent.addAndMakeVisible(label);label->setColour(juce::Label::textColourId,juce::Colour(0xffcbd5d5));label->setFont(juce::FontOptions(12));}
    inTensionLabel.setText("In tension",juce::dontSendNotification);outTensionLabel.setText("Out tension",juce::dontSendNotification);
    slider(inTensionSlider,-1,1,.001,"");slider(outTensionSlider,-1,1,.001,"");
    inTensionSlider.setName("inTension");outTensionSlider.setName("outTension");
    for(auto* control:{&inTensionSlider,&outTensionSlider})inspectorContent.addAndMakeVisible(control);
    inTensionSlider.onValueChange=[this]{if(!updating)processor.swellEngine().edit(state.session.primary,[this](auto& w,auto&){w.shape.inTension=inTensionSlider.getValue();return true;});};
    outTensionSlider.onValueChange=[this]{if(!updating)processor.swellEngine().edit(state.session.primary,[this](auto& w,auto&){w.shape.outTension=outTensionSlider.getValue();return true;});};
    slider(tailSlider,0,5,.01," s");tailSlider.setName("auditionTail");tailSlider.setTooltip("Captured Dry + Wet context after the later of the original arrival and wet endpoint. Dry audio is unchanged.");
    tailSlider.onValueChange=[this]{if(!updating)processor.swellEngine().setPostRoll(tailSlider.getValue());};
    addAndMakeVisible(tailLabel);tailLabel.setText("Listen after",juce::dontSendNotification);
    for(auto* label:{&markerLabel,&markerState}){addAndMakeVisible(label);label->setColour(juce::Label::textColourId,juce::Colour(0xffcbd5d5));}
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
    for(int i=0;i<fieldCount;++i){inspectorContent.addAndMakeVisible(fieldLabels[i]);inspectorContent.addAndMakeVisible(fields[i]);}
    for(auto* control:std::array<juce::Component*,30>{&status,&detail,&generateButton,&allButton,&analyzeButton,&confirmButton,&candidateBox,&sensitivityLabel,&sensitivitySlider,&separationLabel,&separationSlider,&autoButton,&recaptureButton,&cancelButton,&recaptureStatus,&mappingButton,&toneButton,&toneStartLabel,&toneStartSlider,&toneEndLabel,&toneEndSlider,&clearButton,&assetInfoButton,&barStartLabel,&barEndLabel,&barStartEditor,&barEndEditor,&setBarsButton,&auditionLabel,&auditionSlider})diagnosticsContent.addAndMakeVisible(control);
    for(auto* control:std::array<juce::Component*,4>{&automaticButton,&sampleUnitsButton,&defaultIRButton,&suppressionButton})diagnosticsContent.addAndMakeVisible(control);
    list.setRowHeight(64);setSize(1400,900);refresh();startTimerHz(10);
}
PreverbAudioProcessorEditor::~PreverbAudioProcessorEditor(){destroying=true;stopTimer();list.setModel(nullptr);}
void PreverbAudioProcessorEditor::paint(juce::Graphics& g)
{g.fillAll(juce::Colour(0xff172126));g.setColour(juce::Colour(0xff37d6cf));g.fillRect(0,0,getWidth(),3);
    g.setColour(juce::Colour(0xff10181c));g.fillRoundedRectangle(headerStatus.getBounds().toFloat(),7);
    g.setColour(juce::Colour(0xff526064));g.drawRoundedRectangle(headerStatus.getBounds().toFloat(),7,1);
    g.setColour(state.defaultIRError.isNotEmpty()?juce::Colour(0xffe9987f):state.session.automaticProcessing?juce::Colour(0xff37d6cf):juce::Colour(0xffa5b3b0));
    g.fillEllipse(headerStatus.getX()+8.0f,headerStatus.getBounds().getCentreY()-4.0f,8.0f,8.0f);
    g.setColour(juce::Colour(0xff526064));g.drawRoundedRectangle(inspectorViewport.getBounds().toFloat(),6,1);g.drawRoundedRectangle(list.getBounds().toFloat(),6,1);g.drawRoundedRectangle(timeline.getBounds().toFloat(),6,1);}
int PreverbAudioProcessorEditor::getNumRows(){return static_cast<int>(state.session.swells.size());}
void PreverbAudioProcessorEditor::paintListBoxItem(int row,juce::Graphics& g,int width,int height,bool selected)
{
    if(row<0 || row>=getNumRows())return;const auto& w=state.session.swells[static_cast<size_t>(row)];
    if(selected){g.setColour(juce::Colour(0xff334650));g.fillAll();}
    g.setColour(juce::Colour(0xffdce6eb));g.setFont(14.0f);g.drawText(w.name,32,2,width-40,20,juce::Justification::left);
    g.drawRect(8,8,15,15);if(w.enabled){g.setColour(juce::Colour(0xff37d6cf));g.fillRect(11,11,9,9);}
    g.setFont(12.0f);g.drawText(w.irName.isNotEmpty()?w.irName:"No IR",32,23,width-40,18,juce::Justification::left);
    g.setFont(11.0f);g.setColour(w.phase==m3::Phase::error?juce::Colour(0xffedaa91):juce::Colour(0xffaebdc4));
    g.drawText((w.phase==m3::Phase::ready?phaseName(w.phase):w.status)+(w.muted?" | mute":"")+(!w.enabled?" | off":"")+(w.solo?" | solo":"")+(w.frozen?" | frozen":""),32,43,width-40,height-43,juce::Justification::left);
}
void PreverbAudioProcessorEditor::listBoxItemClicked(int row,const juce::MouseEvent& event)
{if(row>=0 && row<getNumRows() && event.x<28){processor.swellEngine().edit(state.session.swells[static_cast<size_t>(row)].id,[](auto& w,auto&){w.enabled=!w.enabled;return true;});refresh();}}
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
    auto area=getLocalBounds().reduced(16);
    auto top=area.removeFromTop(34);title.setBounds(top.removeFromLeft(360));headerStatus.setBounds(top);
    auto markerRow=area.removeFromTop(76);auto controls=markerRow.removeFromRight(490);auto markerArea=controls.removeFromTop(38);
    regenerationLabel.setBounds(controls.removeFromLeft(240));regenerationKnob.setBounds(controls.removeFromLeft(145));
    markerButton.setBounds(markerArea.removeFromLeft(125));markerLabel.setBounds(markerArea.removeFromLeft(115));markerKnob.setBounds(markerArea.removeFromLeft(145));markerState.setBounds(markerArea);
    helper.setBounds(markerRow.removeFromTop(18));liveButton.setBounds(markerRow.removeFromLeft(120));liveRoutingBox.setBounds(markerRow.removeFromLeft(125).reduced(1,4));liveLevelLabel.setBounds(markerRow.removeFromLeft(55));liveLevel.setBounds(markerRow.reduced(4,0));area.removeFromTop(8);
    auto fold=area.removeFromBottom(30);diagnosticsButton.setBounds(fold.removeFromLeft(140).reduced(2));
    diagnosticsButton.setButtonText(diagnosticsOpen?"Hide Diagnostics":"Diagnostics");
    diagnosticsViewport.setVisible(diagnosticsOpen);
    if(diagnosticsOpen){diagnosticsViewport.setBounds(area.removeFromBottom(190));area.removeFromBottom(8);}
    else diagnosticsViewport.setBounds(0,0,0,0);
    songOverview.setBounds(area.removeFromBottom(132).reduced(0,4));
    auto audition=area.removeFromBottom(84);auto contextRow=audition.removeFromBottom(28);tailSlider.setBounds(contextRow.removeFromRight(190));tailLabel.setBounds(contextRow.removeFromRight(80));isolateButton.setBounds(contextRow.removeFromLeft(140));isolateDryButton.setBounds(contextRow.removeFromLeft(155));monitorStatus.setBounds(contextRow);
    auto auditionRow=audition.removeFromTop(40);
    auditionButton.setBounds(auditionRow.removeFromLeft(100).reduced(2));stopButton.setBounds(auditionRow.removeFromLeft(65).reduced(2));
    wetOnlyButton.setBounds(auditionRow.removeFromLeft(95).reduced(2));dryWetButton.setBounds(auditionRow.removeFromLeft(105).reduced(2));
    trimResetButton.setBounds(auditionRow.removeFromRight(165).reduced(2));blendSlider.setBounds(auditionRow.reduced(10,0));
    auto listArea=area.removeFromLeft(220);auto listTop=listArea.removeFromTop(36);
    listLabel.setBounds(listTop.removeFromLeft(90));removeButton.setBounds(listTop.removeFromRight(60).reduced(2));addButton.setBounds(listTop.reduced(2));addButton.setButtonText("+");
    list.setBounds(listArea.reduced(0,4));area.removeFromLeft(10);
    const auto inspectorWidth=std::clamp(getWidth()/4,290,360);
    inspectorViewport.setBounds(area.removeFromRight(inspectorWidth));area.removeFromRight(10);
    auto navigation=area.removeFromTop(34);
    for(auto* button:{&previousButton,&nextButton,&zoomOutButton,&zoomInButton,&fitButton})button->setBounds(navigation.removeFromLeft(52).reduced(2));
    spanLabel.setBounds(navigation.removeFromLeft(30));spanBox.setBounds(navigation.removeFromLeft(65).reduced(2));snapButton.setBounds(navigation.removeFromLeft(65));
    timeline.setBounds(area);
    const auto width=std::max(270,inspectorWidth-20);inspectorContent.setSize(width,advancedTiming?820:710);
    auto inspector=inspectorContent.getLocalBounds().reduced(10);
    selectedStatus.setBounds(inspector.removeFromTop(24));irStatus.setBounds(inspector.removeFromTop(20));loadIRButton.setBounds(inspector.removeFromTop(30).reduced(1));
    auto fieldRow=[&](int field,int height=30)
    {auto row=inspector.removeFromTop(height);fieldLabels[field].setBounds(row.removeFromLeft(92));fields[field].setBounds(row.reduced(1,4));};
    fieldRow(name);auto activity=inspector.removeFromTop(26);enabledButton.setBounds(activity.removeFromLeft(90));muteButton.setBounds(activity.removeFromLeft(65));soloButton.setBounds(activity);freezeButton.setBounds(inspector.removeFromTop(28));
    fieldRow(arrival);fieldRow(audibleB);auto placementRow=inspector.removeFromTop(30);placementLabel.setBounds(placementRow.removeFromLeft(92));nudgeRight.setBounds(placementRow.removeFromRight(24));nudgeLeft.setBounds(placementRow.removeFromRight(24));placementEditor.setBounds(placementRow.reduced(1,4));transitionStatus.setBounds(inspector.removeFromTop(24));
    for(int field:{fadeIn,fadeOut})
    {auto row=inspector.removeFromTop(44);fieldLabels[field].setBounds(row.removeFromLeft(56));(field==fadeIn?fadeInKnob:fadeOutKnob).setBounds(row.removeFromLeft(32));
        (field==fadeIn?inCurveBox:outCurveBox).setBounds(row.removeFromRight(100).reduced(1,8));fields[field].setBounds(row.reduced(1,8));}
    for(auto pair:{std::pair{&inTensionLabel,&inTensionSlider},std::pair{&outTensionLabel,&outTensionSlider}})
    {auto tensionRow=inspector.removeFromTop(26);pair.first->setBounds(tensionRow.removeFromLeft(75));pair.second->setBounds(tensionRow);}
    supportStatus.setBounds(inspector.removeFromTop(42));
    curveInLabel.setVisible(false);curveOutLabel.setVisible(false);
    auto row=inspector.removeFromTop(44);gainLabel.setBounds(row.removeFromLeft(80));gainSlider.setBounds(row);
    row=inspector.removeFromTop(36);fullButton.setBounds(row.removeFromLeft(row.getWidth()/2).reduced(1));resetButton.setBounds(row.reduced(1));
    timingButton.setBounds(inspector.removeFromTop(34).reduced(1));
    for(int field:{sourceA,sourceB,audibleA})
    {fieldLabels[field].setVisible(advancedTiming);fields[field].setVisible(advancedTiming);if(advancedTiming)fieldRow(field);}
    const auto diagnosticsWidth=std::max(1000,getWidth()-50);diagnosticsContent.setSize(diagnosticsWidth,430);
    auto diagnostic=diagnosticsContent.getLocalBounds().reduced(4);
    status.setBounds(diagnostic.removeFromTop(52));detail.setBounds(diagnostic.removeFromTop(82));
    row=diagnostic.removeFromTop(34);
    for(auto* button:{&generateButton,&allButton,&analyzeButton,&confirmButton})button->setBounds(row.removeFromLeft(150).reduced(2));candidateBox.setBounds(row.reduced(2));
    row=diagnostic.removeFromTop(34);sensitivityLabel.setBounds(row.removeFromLeft(85));sensitivitySlider.setBounds(row.removeFromLeft(190));separationLabel.setBounds(row.removeFromLeft(105));separationSlider.setBounds(row.removeFromLeft(180));
    automaticButton.setBounds(row.removeFromLeft(180));autoButton.setBounds(row);
    row=diagnostic.removeFromTop(34);mappingButton.setBounds(row.removeFromLeft(170));clearButton.setBounds(row.removeFromLeft(95).reduced(2));recaptureButton.setBounds(row.removeFromLeft(205).reduced(2));cancelButton.setBounds(row.removeFromLeft(140).reduced(2));sampleUnitsButton.setBounds(row.removeFromLeft(195));assetInfoButton.setBounds(row.reduced(2));
    row=diagnostic.removeFromTop(34);toneButton.setBounds(row.removeFromLeft(160));toneStartLabel.setBounds(row.removeFromLeft(90));toneStartSlider.setBounds(row.removeFromLeft(180));toneEndLabel.setBounds(row.removeFromLeft(95));toneEndSlider.setBounds(row.removeFromLeft(180));auditionLabel.setBounds(row.removeFromLeft(100));auditionSlider.setBounds(row);
    row=diagnostic.removeFromTop(34);barStartLabel.setBounds(row.removeFromLeft(65));barStartEditor.setBounds(row.removeFromLeft(70).reduced(2));barEndLabel.setBounds(row.removeFromLeft(65));barEndEditor.setBounds(row.removeFromLeft(70).reduced(2));setBarsButton.setBounds(row.removeFromLeft(175).reduced(2));defaultIRButton.setBounds(row.removeFromLeft(200).reduced(2));suppressionButton.setBounds(row.removeFromLeft(175).reduced(2));
    recaptureStatus.setBounds(diagnostic);
}
void PreverbAudioProcessorEditor::refresh()
{
    if(destroying)return;updating=true;state=processor.swellEngine().snapshot();const auto capture=processor.getCaptureSnapshot();const auto time=processor.getSongTime();const auto diagnostics=processor.getHostPositionDiagnostics();
    timeline.setData(capture,state,time,diagnostics.samplePositionAvailable);
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
        setText(placementEditor,juce::String(w->placement*1000.0/rate,6));
        const auto delta=(static_cast<long double>(w->shape.endOffset)+w->placement)*1000.0/rate;
        transitionStatus.setText((delta<0?"Gap: ":"Wet overlap: ")+juce::String(static_cast<double>(std::abs(delta)),1)+" ms | dry unchanged",juce::dontSendNotification);
        const auto end=m3::renderInputEnd(*w);
        supportStatus.setText(end>w->sourceEnd?"Extra captured input through "+position(end)+" contributes to wet audio (may include later notes)":"Wet support uses the selected source",juce::dontSendNotification);
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
    if(!blendSlider.isMouseButtonDown() && !blendSlider.hasKeyboardFocus(true))blendSlider.setValue(state.session.wetBlend*100,juce::dontSendNotification);
    wetOnlyButton.setToggleState(state.session.auditionMode==m3::AuditionMode::wetOnly,juce::dontSendNotification);
    dryWetButton.setToggleState(state.session.auditionMode==m3::AuditionMode::dryWet,juce::dontSendNotification);
    trimResetButton.setButtonText("Trim "+juce::String(state.session.auditionDb,1)+" dB"+(state.session.auditionDb!=0?" | reset":""));
    monitorStatus.setText(state.dryStatus.isNotEmpty()?state.dryStatus:(state.session.isolateSelected?juce::String("Isolated"):juce::String("Overlaps"))+" | "+juce::String(static_cast<int>(state.auditionMembers.size()))+" wet | "+(state.session.isolateDry?"selected dry":"dry context")+" | live dry unchanged",juce::dontSendNotification);monitorStatus.setTooltip(state.dryStatus);
    selectedStatus.setText(w?phaseName(w->phase)+" | "+w->name:"Select a swell",juce::dontSendNotification);
    selectedStatus.setTooltip(w?w->status:state.message);
    irStatus.setText(w?w->irName:"No swell selected",juce::dontSendNotification);
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
    auditionButton.setEnabled(state.auditionReady && !(state.session.liveEnabled && diagnostics.isPlaying));stopButton.setEnabled(state.playing || state.auditionReady);fullButton.setEnabled(w && state.raw);resetButton.setEnabled(w!=nullptr);
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
