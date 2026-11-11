#include "LearnGenerationSupport.h"
#include "AudioTestFiles.h"
#include "ControlLibraryCases.h"
#include "LoopCaptureFixture.h"
#include <Plugin/PluginEditor.h>
#include <UI/InitialPerformView.h>
#include <Core/BuildInformation.h>
#include <Capture/SourceRangeReader.h>
#include <DSP/SwellShape.h>

static juce::Component* named(juce::Component& root,const juce::String& name)
{
    if(root.getName()==name)return &root;
    for(auto* child:root.getChildren())if(auto* result=named(*child,name))return result;
    return nullptr;
}
static bool visible(juce::Component* value)
{if(!value)return false;for(;value;value=value->getParentComponent())if(!value->isVisible())return false;return true;}
static bool sameView(const SwellViewport& a,const SwellViewport& b)
{return a.first==b.first && a.last==b.last;}
static void sharedSoundBindings()
{
    GenerationFixture fixture;auto& engine=fixture.engine();
    const auto fileA=fixture.root.file.getChildFile("shared-a.wav"),fileB=fixture.root.file.getChildFile("shared-b.wav");
    writeTestWav(fileA,1,48000,{.5f,.1f,.03f});writeTestWav(fileB,1,48000,{.4f,-.1f,.02f,0});
    REQUIRE(engine.assignDefaultIR(fileA));await([&]{return !engine.snapshot().defaultIRPending;});const auto irA=engine.savedSession().defaultIR;
    REQUIRE(engine.assignDefaultIR(fileB));await([&]{return !engine.snapshot().defaultIRPending;});const auto irB=engine.savedSession().defaultIR;
    fixture.record();fixture.ready();
    PreverbAudioProcessorEditor editor(*fixture.processor);editor.setSize(900,600);editor.refresh();
    auto* defaults=dynamic_cast<m3::ui::controls::ConvolutionIrEditor*>(named(editor,"learnIrEditor"));
    auto* selected=dynamic_cast<m3::ui::controls::ConvolutionIrEditor*>(named(editor,"selectedIrEditor"));
    REQUIRE(defaults && selected && defaults!=selected);
    const auto choose=[&](auto& field,const Asset& ir)
    {
        editor.refresh();int id=0;const auto choices=engine.irChoices();
        for(size_t i=0;i<choices.size();++i)if(choices[i].content==ir.content)id=static_cast<int>(i+1);
        REQUIRE(id>0);field.selector.beforeInvocation();field.selector.setSelectedId(id,juce::sendNotificationSync);
    };
    choose(*defaults,irA);await([&]{return !engine.snapshot().defaultIRPending;});
    REQUIRE(engine.savedSession().defaultIR==irA && engine.savedSession().swells.empty());
    REQUIRE(fixture.generate());await([&]{return !engine.snapshot().learnGeneration.generating;});
    await([&]{const auto s=engine.savedSession();return s.swells.size()>2 && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==Phase::ready;});});
    const auto generated=engine.savedSession();for(const auto& w:generated.swells)REQUIRE(w.ir==irA && w.provenance==Provenance::automatic);
    // Presentation alone must preserve accepted dry, raw wet and shaped wet
    // exactly, as well as their durable references and render-job count.
    const auto captured=fixture.processor->getCaptureSnapshot();CaptureAssetStore sourceStore(engine.assetDirectory());
    SwellAssetStore renderStore(engine.assetDirectory());SourceInterval dryBefore,dryAfter;WetRender rawBefore,rawAfter,shapeBefore,shapeAfter;juce::String error;
    REQUIRE(readSourceInterval(sourceStore,captured.manifest,0,72*4096,72*4096,dryBefore,error));
    REQUIRE(renderStore.readRender(generated.swells.front().render,previewMemoryBudget,rawBefore,error));
    REQUIRE(prepareShape(generated.swells.front(),rawBefore,previewMemoryBudget,shapeBefore,error));
    const auto jobs=engine.snapshot().renderCount;
    for(int i=0;i<5;++i)editor.refresh();
    const auto inspected=engine.savedSession();
    REQUIRE(engine.snapshot().renderCount==jobs && fixture.processor->getCaptureSnapshot().manifest.revision==captured.manifest.revision);
    for(const auto& old:generated.swells)REQUIRE(sameSavedSwell(old,*find(inspected,old.id)));
    REQUIRE(readSourceInterval(sourceStore,captured.manifest,0,72*4096,72*4096,dryAfter,error));
    REQUIRE(renderStore.readRender(inspected.swells.front().render,previewMemoryBudget,rawAfter,error));
    REQUIRE(prepareShape(inspected.swells.front(),rawAfter,previewMemoryBudget,shapeAfter,error));
    REQUIRE(dryBefore.samples==dryAfter.samples && rawBefore.samples==rawAfter.samples && shapeBefore.samples==shapeAfter.samples);
    std::cout<<"Shared IR inspection: exact dry/raw/shaped PCM equality ("<<dryBefore.samples.size()<<"/"<<rawBefore.samples.size()<<"/"<<shapeBefore.samples.size()<<" values); no extra renders.\n";
    const auto frozen=generated.swells.front().id,edited=generated.swells[1].id;
    REQUIRE(engine.setFrozen(frozen,true));REQUIRE(engine.addTarget(15000));const auto manual=engine.savedSession().primary;
    await([&]{return find(engine.savedSession(),manual)->phase==Phase::ready;});
    REQUIRE(find(engine.savedSession(),manual)->ir==irA && find(engine.savedSession(),manual)->provenance==Provenance::manualCreated);
    REQUIRE(fixture.processor->requestWorkflowMode(workflow::Mode::capture));editor.refresh();
    const auto beforeDefault=engine.savedSession();choose(*defaults,irB);await([&]{return !engine.snapshot().defaultIRPending;});
    const auto afterDefault=engine.savedSession();REQUIRE(afterDefault.defaultIR==irB);
    for(const auto& old:beforeDefault.swells)REQUIRE(sameSavedSwell(old,*find(afterDefault,old.id)));
    REQUIRE(fixture.processor->requestWorkflowMode(workflow::Mode::live));REQUIRE(engine.selectTargets({edited},edited));editor.refresh();
    choose(*selected,irB);await([&]{return find(engine.savedSession(),edited)->ir==irB && find(engine.savedSession(),edited)->phase==Phase::ready;});
    REQUIRE(find(engine.savedSession(),edited)->provenance==Provenance::manualEdited && engine.savedSession().defaultIR==irB);
    const auto protectedState=engine.savedSession();
    REQUIRE(fixture.processor->requestWorkflowMode(workflow::Mode::capture));editor.refresh();
    REQUIRE(fixture.generate());await([&]{return !engine.snapshot().learnGeneration.generating;});
    const auto regenerated=engine.savedSession();
    for(const auto& id:{frozen,edited,manual})REQUIRE(sameSavedSwell(*find(protectedState,id),*find(regenerated,id)));
    for(const auto& w:regenerated.swells)if(w.provenance==Provenance::automatic)REQUIRE(w.ir==irB);
    // The opted-in Stop path takes the next accepted default through the same
    // control. Recapture makes this a genuinely eligible new source pass.
    REQUIRE(fixture.processor->requestWorkflowMode(workflow::Mode::capture));editor.refresh();choose(*defaults,irA);
    await([&]{return !engine.snapshot().defaultIRPending;});
    REQUIRE(engine.setAutoGenerateOnStop(true,engine.assignmentEpoch(),fixture.processor->getWorkflowSnapshot().pass));
    REQUIRE(fixture.processor->setRecaptureExisting(true));fixture.record();
    await([&]{const auto s=engine.savedSession();return !engine.snapshot().learnGeneration.generating && std::all_of(s.swells.begin(),s.swells.end(),[&](const auto& w){return w.provenance!=Provenance::automatic || w.ir==irA;});});
    for(const auto& id:{frozen,edited,manual})
    {const auto current=engine.savedSession();const auto* kept=find(current,id);const auto* old=find(protectedState,id);REQUIRE(kept && kept->ir==old->ir && kept->shape==old->shape && kept->provenance==old->provenance && kept->frozen==old->frozen);}
}
static void pendingProtectedFraming()
{
    GenerationFixture fixture;const auto accepted=fixture.engine().savedSession();
    const auto time=fixture.processor->getSongTime();Swell pending;pending.ir=accepted.defaultIR;
    pending.arrival=pending.sourceStart=-2*static_cast<int64_t>(pending.ir.frames);pending.sourceEnd=pending.arrival+32;
    pending.phase=Phase::checking;REQUIRE(pending.generationSource.isEmpty() && pending.provenance==Provenance::manualCreated);
    EditorSnapshot shown;shown.session=accepted;shown.session.swells={pending};shown.session.primary=pending.id;shown.session.selected={pending.id};
    SwellTimeline timeline(fixture.engine());timeline.setSize(600,400);timeline.setData({},shown,time,true);timeline.revealInitialSelection();
    const auto bounds=m3::ui::initialSwellBounds(pending,time);const auto view=timeline.viewport();
    REQUIRE(view.first<=bounds.first && view.last>=bounds.second);
    REQUIRE(view.first>=timeline.navigationBounds().first && view.last<=timeline.navigationBounds().last);
    REQUIRE(fixture.engine().savedSession().swells.empty() && fixture.engine().snapshot().renderCount==0);
}
static void controlBindings()
{
    GenerationFixture fixture;auto& engine=fixture.engine();PreverbAudioProcessorEditor editor(*fixture.processor);editor.addToDesktop(0);editor.setVisible(true);editor.refresh();
    auto* generate=dynamic_cast<m3::ui::controls::ActionButton*>(named(editor,"learnGenerateSwells"));
    auto* automatic=dynamic_cast<m3::ui::controls::Switch*>(named(editor,"learnAutomaticSwells"));
    auto* stop=dynamic_cast<m3::ui::controls::Switch*>(named(editor,"learnAutoGenerateOnStop"));
    auto* helper=dynamic_cast<juce::Label*>(named(editor,"learnManualSwellsHelp"));
    REQUIRE(generate && automatic && stop && helper && !generate->isEnabled() && visible(helper));
    REQUIRE(generate->getTooltip().contains(engine.snapshot().learnGeneration.reason) && generate->getAccessibilityHandler());
    stop->keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(engine.savedSession().autoGenerateOnStop);
    controlReview::pointerClick(*automatic);REQUIRE(!visible(stop) && !visible(generate) && engine.savedSession().autoGenerateOnStop);
    REQUIRE(visible(helper) && helper->getY()>named(editor,"learnAutomaticHelp")->getBottom());
    {
        PreverbAudioProcessorEditor reopened(*fixture.processor);reopened.setVisible(true);reopened.refresh();
        auto* retained=named(reopened,"learnManualSwellsHelp");
        REQUIRE(visible(retained) && !retained->getBounds().isEmpty() && retained->getParentComponent()->getLocalBounds().contains(retained->getBounds()));
        REQUIRE(!visible(named(reopened,"learnGenerateSwells")));
    }
    controlReview::pointerClick(*automatic);REQUIRE(visible(stop) && stop->getToggleState());
    stop->beforeInvocation();engine.restore(engine.savedSession(),fixture.processor->getCaptureSnapshot().reference);
    stop->setToggleState(false,juce::dontSendNotification);stop->onClick();REQUIRE(engine.savedSession().autoGenerateOnStop);
    REQUIRE(engine.setAutoGenerateOnStop(false,engine.assignmentEpoch(),fixture.processor->getWorkflowSnapshot().pass));
    fixture.record();fixture.ready();editor.refresh();REQUIRE(generate->isEnabled());
    generate->beforeInvocation();engine.restore(engine.savedSession(),fixture.processor->getCaptureSnapshot().reference);generate->onClick();
    REQUIRE(engine.savedSession().swells.empty() && !engine.snapshot().initialPerformHandoff);
    fixture.ready();editor.refresh();engine.holdLearnRendersForTesting(true);
    generate->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    // Keyboard activation posts a click. Keep dispatching while waiting rather
    // than relying on it having fired within the first 30 ms under native load.
    try{await([&]{juce::MessageManager::getInstance()->runDispatchLoopUntil(10);return fixture.processor->getWorkflowSnapshot().mode==workflow::Mode::live;});}
    catch(...){const auto view=engine.snapshot().learnGeneration;std::cerr<<"Keyboard Generate: "<<view.reason<<" | "<<view.notice<<'\n';throw;}
    editor.refresh();
    REQUIRE(engine.savedSession().primary==m3::ui::firstChronologicalSwell(engine.savedSession()));
}
static void captureImage(juce::Component& editor,const juce::String& tag,float scale=1)
{
    const auto folder=juce::SystemStats::getEnvironmentVariable("PREVERB_UI_M3_OUTPUT",{});if(folder.isEmpty())return;
    const juce::File directory(folder);REQUIRE(directory.createDirectory().wasOk());
    auto output=directory.getChildFile(tag+".png").createOutputStream();REQUIRE(output);
    REQUIRE(juce::PNGImageFormat().writeImageToStream(editor.createComponentSnapshot(editor.getLocalBounds(),true,scale),*output));
}

static void songHostTimingFeedback(bool review=false)
{
    LoopCaptureFixture fixture;
    auto& processor=*fixture.processor;
    auto preference=processor.getLoopCapturePreference();
    preference.enabled=false;
    REQUIRE(processor.setLoopCapturePreference(preference));
    await([&]{return processor.swellEngine().savedSession().defaultIR.present();});
    auto preview=std::make_unique<PreverbAudioProcessorEditor>(processor);
    auto& editor=*preview;
    editor.setVisible(true);
    auto* learn=dynamic_cast<m3::ui::LearnWorkspace*>(named(editor,"learnWorkspace"));
    auto* notice=dynamic_cast<m3::ui::controls::NoticeBar*>(named(editor,"workspaceNotice"));
    REQUIRE(learn && notice);
    processor.setPlayHead(&fixture.host);
    fixture.host.position.setTimeInSamples(128);
    fixture.host.position.setBpm(120);
    fixture.tick();
    editor.refresh();
    REQUIRE(processor.getHostPositionDiagnostics().samplePositionAvailable && !learn->songPlayhead());
    REQUIRE(!learn->timelineStatusText().contains("Ready"));
    REQUIRE(notice->currentMessage() && notice->currentMessage()->text.contains("Song Mode")
            && notice->currentMessage()->detail.contains("Loop Mode"));
    captureImage(editor,"song-host-timing-unavailable");
    if(review)
    {
        controlReview::run(std::move(preview));
        return;
    }

    preference.enabled=true;
    REQUIRE(processor.setLoopCapturePreference(preference));
    editor.refresh();
    REQUIRE(!notice->currentMessage());
    preference.enabled=false;
    REQUIRE(processor.setLoopCapturePreference(preference));
    fixture.host.position.setTimeInSamples(256);
    fixture.host.position.setPpqPosition(256./24000);
    fixture.host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    fixture.tick();
    editor.refresh();
    REQUIRE(learn->songPlayhead()==256 && !notice->currentMessage());
    processor.setPlayHead(nullptr);
}

static void loopUiCases(bool review)
{
    LoopCaptureFixture fixture;
    fixture.percussive=true;
    auto& processor=*fixture.processor;
    auto initial=processor.getLoopCapturePreference();
    initial.enabled=false;
    REQUIRE(processor.setLoopCapturePreference(initial));
    PreverbAudioProcessorEditor editor(processor);
    editor.enableResponsiveReview({},m3::ui::PresentationFitContext{{1920,1080},{16,80}});
    editor.setVisible(true);
    editor.refresh();
    auto* toggle=dynamic_cast<m3::ui::controls::LatchingActionButton*>(named(editor,"learnLoopMode"));
    auto* arm=dynamic_cast<m3::ui::controls::LatchingActionButton*>(named(editor,"learnLoopArm"));
    auto* method=dynamic_cast<m3::ui::controls::ChoiceSelector*>(named(editor,"learnLoopMethod"));
    auto* unit=dynamic_cast<m3::ui::controls::ChoiceSelector*>(named(editor,"learnLoopUnit"));
    auto* count=dynamic_cast<m3::ui::controls::FieldNumericInput*>(named(editor,"learnLoopCount"));
    auto* status=dynamic_cast<juce::Label*>(named(editor,"learnLoopStatus"));
    auto* automatic=dynamic_cast<m3::ui::controls::Switch*>(named(editor,"learnAutomaticSwells"));
    auto* recapture=dynamic_cast<m3::ui::controls::Switch*>(named(editor,"recaptureExisting"));
    auto* generate=dynamic_cast<m3::ui::controls::ActionButton*>(named(editor,"learnGenerateSwells"));
    auto* onStop=named(editor,"learnAutoGenerateOnStop");
    auto* sensitivity=named(editor,"learnMarkerSensitivity");
    auto* policies=dynamic_cast<m3::ui::controls::FocusViewport*>(named(editor,"learnLoopPolicies"));
    auto* workspace=dynamic_cast<m3::ui::LearnWorkspace*>(named(editor,"learnWorkspace"));
    REQUIRE(toggle && arm && method && unit && count && status && automatic && recapture && generate && onStop && sensitivity && policies && workspace);
    const auto activate=[](m3::ui::controls::CommandButton& button){
        button.invokeCommand();
        juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    };
    const auto buttonFill=[](m3::ui::controls::ActionButton& button){
        juce::Image image(juce::Image::ARGB,button.getWidth(),button.getHeight(),true,juce::SoftwareImageType{});
        juce::Graphics graphics(image);
        button.paintButton(graphics,false,false);
        return image.getPixelAt(12,button.getHeight()/2);
    };
    REQUIRE(visible(toggle) && toggle->getButtonText()=="Loop Mode" && !toggle->getToggleState());
    REQUIRE(visible(named(editor,"recaptureExisting")) && !visible(arm));
    const auto controlBounds=[&](const char* name){
        auto* control=named(editor,name);
        REQUIRE(control);
        return workspace->getLocalArea(control,control->getLocalBounds());
    };
    const auto emptyLearnGeometry=[&]{
        const auto recaptureBounds=controlBounds("recaptureExisting");
        return std::make_tuple(editor.getLocalBounds(),workspace->timelineBounds(),
                               controlBounds("learnSections"),controlBounds("learnSourceCaptureHeading"),
                               controlBounds("learnLoopMode"),recaptureBounds.getX(),recaptureBounds.getWidth());
    };
    const auto songGeometry=emptyLearnGeometry();
    captureImage(editor,"issue133-empty-song");
    activate(*toggle);
    REQUIRE(processor.getLoopCapturePreference().enabled && visible(recapture) && !recapture->getToggleState());
    REQUIRE(toggle->getToggleState());
    REQUIRE(emptyLearnGeometry()==songGeometry);
    captureImage(editor,"issue133-empty-loop");
    activate(*toggle);
    REQUIRE(!processor.getLoopCapturePreference().enabled && !toggle->getToggleState() && !visible(arm));
    REQUIRE(emptyLearnGeometry()==songGeometry);
    activate(*toggle);
    REQUIRE(processor.getLoopCapturePreference().enabled && toggle->getToggleState());
    REQUIRE(visible(arm) && arm->isEnabled() && visible(count));
    REQUIRE(!visible(status));
    REQUIRE(buttonFill(*arm)==m3::ui::background && buttonFill(*toggle)==m3::ui::aqua);
    REQUIRE(visible(automatic) && automatic->isEnabled());
    automatic->setToggleState(true,juce::dontSendNotification);
    automatic->onClick();
    REQUIRE(processor.swellEngine().savedSession().autoMarkersEnabled);
    REQUIRE(visible(sensitivity) && visible(generate) && visible(onStop) && !onStop->isEnabled());
    method->setSelectedId(2,juce::sendNotificationSync);
    REQUIRE(method->getText()=="End on Signal");
    REQUIRE(arm->isEnabled() && !visible(count));
    REQUIRE(!visible(status) && arm->getTooltip()=="Arm the next recording.");
    REQUIRE(method->getTooltip().contains("Stop MIDI note") && method->getTooltip().contains("next bar line"));
    REQUIRE(controlBounds("recaptureExisting").getY()-controlBounds("learnLoopArm").getBottom()==m3::ui::spacing::tight);
    captureImage(editor,"issue126-learn-end-signal");
    activate(*arm);
    REQUIRE(arm->getToggleState() && processor.getWorkflowSnapshot().phase==workflow::Phase::armed);
    REQUIRE(visible(status) && controlBounds("recaptureExisting").getY()>controlBounds("learnLoopStatus").getBottom());
    activate(*arm); // The preview has no audio callbacks between these clicks.
    REQUIRE(!arm->getToggleState() && toggle->isEnabled());
    REQUIRE(processor.getWorkflowSnapshot().result==workflow::Result::cancelled);
    method->setSelectedId(3,juce::sendNotificationSync);
    REQUIRE(processor.getLoopCapturePreference().method==LoopCapturePreference::Method::endOnRepeat);
    REQUIRE(!visible(count) && arm->getTooltip().contains("one pass"));
    method->setSelectedId(1,juce::sendNotificationSync);
    REQUIRE(visible(count) && arm->getTooltip().contains("bar line"));
    unit->setSelectedId(2,juce::sendNotificationSync);
    count->onStep(1);
    REQUIRE(processor.getLoopCapturePreference().count==2);
    REQUIRE(processor.getLoopCapturePreference().unit==LoopCapturePreference::Unit::beats);
    fixture.through(fixture.marker(2));
    editor.refresh();
    REQUIRE(arm->isEnabled());
    activate(*arm);
    REQUIRE(processor.loopCaptureArmed() && arm->getButtonText()=="Arm" && arm->getToggleState() && !count->isEnabled());
    REQUIRE(buttonFill(*arm)==m3::ui::danger);
    REQUIRE(status->getText().contains("Waiting") && !toggle->isEnabled());
    REQUIRE(workspace->coverageTotalsText()=="No accepted source");
    activate(*arm);
    REQUIRE(!processor.loopCaptureArmed() && !arm->getToggleState() && toggle->isEnabled());
    fixture.tick();
    editor.refresh();
    REQUIRE(!processor.loopCaptureArmed() && !arm->getToggleState() && toggle->isEnabled());
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));
    auto previewPreference=processor.getLoopCapturePreference();
    previewPreference.unit=LoopCapturePreference::Unit::bars;
    previewPreference.count=1;
    REQUIRE(processor.setLoopCapturePreference(previewPreference));
    editor.refresh();
    activate(*arm);
    REQUIRE(processor.loopCaptureArmed());
    fixture.through(fixture.marker(16));
    editor.refresh();
    REQUIRE(arm->getToggleState() && status->getText().contains("Recording Loop"));
    fixture.through(fixture.marker(20)-fixture.frames,true);
    processor.holdCaptureValidationForTesting(true);
    fixture.through(fixture.marker(20));
    await([&]{return processor.getWorkflowSnapshot().phase==workflow::Phase::validating;});
    editor.refresh();
    REQUIRE(!arm->getToggleState() && !arm->isEnabled() && status->getText().contains("Checking captured audio"));
    processor.holdCaptureValidationForTesting(false);
    REQUIRE(processor.flushCaptureForTesting());
    const auto accepted=processor.getCaptureSnapshot();
    REQUIRE(accepted.sourceKind()==CaptureSourceKind::loop);
    const auto markers=[&]{return processor.swellEngine().snapshot().learnGeneration;};
    await([&]{
        const auto view=markers();
        return view.displayed && view.displayed->identity.loop==accepted.manifest.loop && !view.updating;
    });
    const auto view=markers();
    REQUIRE(!view.displayed->positions.empty() && !view.displayed->identity.provisional);
    REQUIRE(view.displayed->identity.sourceReference==accepted.reference);
    for(const auto& marker:view.displayed->positions)
        REQUIRE(marker.sample>=0 && marker.sample<accepted.manifest.loop->endSample);
    editor.refresh();
    REQUIRE(workspace->coverageTotalsText().startsWith("Accepted Loop"));
    REQUIRE(!arm->getToggleState() && !arm->isEnabled() && toggle->getToggleState());
    REQUIRE(status->getText().contains("Recapture audio"));
    REQUIRE(!onStop->isEnabled());
    REQUIRE(workspace->dryViewport()==std::make_pair(0.,static_cast<double>(accepted.manifest.loop->endSample)));
    // Shared P3 wrapping must allocate the full Loop status and move following
    // controls inside the existing scroll region, including long explanations.
    const auto longStatus=juce::String::repeatedString("Wait for the accepted Loop and its timing reference before recording again. ",12);
    workspace->displayLoop(processor.getLoopCapturePreference(),false,false,false,false,false,longStatus);
    workspace->resized();
    const auto* wrappedStatus=dynamic_cast<m3::ui::controls::WrappedLabel*>(status);REQUIRE(wrappedStatus);
    REQUIRE(status->getHeight()==wrappedStatus->preferredHeight(status->getWidth()));
    REQUIRE(recapture->getY()>=status->getBottom() && policies->getViewedComponent()->getHeight()>=generate->getBottom());
    REQUIRE(status->getTooltip()==longStatus);
    editor.refresh();
    // Main's waveform navigation must use the accepted phrase, even when the
    // host has no Song position. Refresh retains inspection; a new source resets it.
    const auto fullLoopView=workspace->dryViewport();
    const auto point=workspace->timelineBounds().getCentre().toFloat();
    const auto stamp=juce::Time::getCurrentTime();
    const auto pointer=[&](juce::Point<float> at,juce::ModifierKeys keys){
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),at,keys,
            0,0,0,0,0,workspace,workspace,stamp,point,stamp,1,at!=point);
    };
    juce::MouseWheelDetails wheel{};wheel.deltaY=.5f;
    workspace->mouseWheelMove(pointer(point,{}),wheel);
    const auto zoomed=workspace->dryViewport();
    REQUIRE(zoomed.second-zoomed.first<fullLoopView.second-fullLoopView.first);
    workspace->mouseDown(pointer(point,juce::ModifierKeys::leftButtonModifier));
    workspace->mouseUp(pointer(point,{}));
    const auto pin=workspace->inspectedSample();
    REQUIRE(pin && workspace->hasPinnedInspection());
    REQUIRE(workspace->inspectionText().contains("Bar ") && !workspace->inspectionText().contains("Timing unavailable"));
    workspace->mouseDown(pointer(point,juce::ModifierKeys::middleButtonModifier));
    workspace->mouseDrag(pointer(point.translated(40,0),juce::ModifierKeys::middleButtonModifier));
    workspace->mouseUp(pointer(point.translated(40,0),{}));
    const auto panned=workspace->dryViewport();
    REQUIRE(panned!=zoomed && panned.first>=0 && panned.second<=fullLoopView.second);
    editor.refresh();
    REQUIRE(workspace->dryViewport()==panned && workspace->inspectedSample()==pin);
    auto replacement=accepted;replacement.reference+="-replacement";++replacement.manifest.revision;
    workspace->display(replacement,processor.swellEngine().snapshot(),processor.getSongTime(),processor.getWorkflowSnapshot(),false);
    REQUIRE(workspace->dryViewport()==fullLoopView && !workspace->hasPinnedInspection());
    editor.refresh();
    REQUIRE(workspace->dryViewport()==fullLoopView && processor.getCaptureSnapshot().reference==accepted.reference);
    for (const auto scale : {100,125})
    {
        REQUIRE(editor.setPresentationScale(scale));
        editor.refresh();
        policies->setViewPosition(0,0);
        REQUIRE(visible(status) && workspace->getLocalBounds().contains(workspace->getLocalArea(status,status->getLocalBounds())));
        REQUIRE(arm->getBottom()<=status->getY() && count->getRight()<unit->getRight());
        REQUIRE(m3::ui::controls::commandBodyBounds(method->getBounds()).getX()==count->getX());
        REQUIRE(m3::ui::controls::commandBodyBounds(arm->getBounds()).getX()==status->getX());
        REQUIRE(m3::ui::controls::commandBodyBounds(toggle->getBounds()).getX()==status->getX());
        REQUIRE(count->getX()==status->getX());
        captureImage(editor,"issue124-loop-markers-"+juce::String(scale));
        policies->reveal(*generate);
        REQUIRE(policies->getViewArea().contains(generate->getBounds()));
        policies->setViewPosition(0,policies->getViewedComponent()->getHeight());
        captureImage(editor,"issue124-loop-options-"+juce::String(scale));
    }
    policies->setViewPosition(0,0);
    recapture->setToggleState(true,juce::dontSendNotification);
    recapture->onClick();
    REQUIRE(processor.loopRecaptureEnabled() && arm->isEnabled());
    activate(*arm);
    REQUIRE(arm->getToggleState() && !generate->isEnabled() && markers().displayed->serial==view.displayed->serial);
    for(const auto scale : {100,125})
    {
        REQUIRE(editor.setPresentationScale(scale));
        captureImage(editor,"issue124-loop-armed-"+juce::String(scale));
    }
    activate(*arm);
    fixture.tick();
    editor.refresh();
    REQUIRE(!processor.loopCaptureArmed() && !arm->getToggleState());
    if (!review)
    {
        auto& engine=processor.swellEngine();
        const auto ir=fixture.root.file.getChildFile("Loop generation impulse.wav");
        writeTestWav(ir,1,48000,{.4f,0,.2f,0,.1f});
        REQUIRE(engine.assignDefaultIR(ir,engine.assignmentEpoch(),processor.getWorkflowSnapshot().pass));
        await([&]{return markers().canGenerate;});
        const auto ready=markers();
        REQUIRE(!engine.generateLearn(engine.assignmentEpoch(),processor.getWorkflowSnapshot().pass,ready.displayed->serial+1));
        editor.refresh();REQUIRE(generate->isEnabled());
        activate(*generate);
        await([&]{return processor.getWorkflowSnapshot().mode==workflow::Mode::live;});
        const auto swells=engine.savedSession().swells;
        REQUIRE(!swells.empty() && processor.getCaptureSnapshot().reference==accepted.reference);
        for(const auto& swell:swells) REQUIRE(swell.sourceStart>=0 && swell.sourceEnd<=accepted.manifest.loop->endSample);
        return;
    }

    REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));
    fixture.simpleClock=true;
    fixture.through(fixture.marker(fixture.nextBeat));
    previewPreference.method=LoopCapturePreference::Method::endSignal;
    REQUIRE(processor.setLoopCapturePreference(previewPreference));
    REQUIRE(processor.setLoopRecapture(false));
    editor.refresh();
    REQUIRE(editor.setPresentationScale(100));
    class Window final : public juce::DocumentWindow, private juce::Timer
    {
    public:
        Window(PreverbAudioProcessorEditor& view,LoopCaptureFixture& input)
            : juce::DocumentWindow("ReverbArise #133 - Learn preview",m3::ui::background,closeButton), fixture(input)
        {
            setUsingNativeTitleBar(true);
            setContentNonOwned(&view,true);
            centreWithSize(getWidth(),getHeight());
            setVisible(true);
            startTimerHz(100);
        }
        void closeButtonPressed() override {juce::MessageManager::getInstance()->stopDispatchLoop();}
    private:
        void timerCallback() override {for(int i=0;i<4;++i)fixture.tick();}
        LoopCaptureFixture& fixture;
    };
    Window window(editor,fixture);
    juce::MessageManager::getInstance()->runDispatchLoop();
}
static void candidateOverlay(PreverbAudioProcessorEditor& editor,GenerationFixture& fixture)
{
    auto* workspace=dynamic_cast<m3::ui::LearnWorkspace*>(named(editor,"learnWorkspace"));REQUIRE(workspace);
    const auto snapshot=fixture.engine().snapshot();REQUIRE(snapshot.learnGeneration.displayed && !snapshot.learnGeneration.displayed->positions.empty());
    const auto before=fixture.engine().savedSession();const auto renderCount=snapshot.renderCount;
    const auto bounds=workspace->timelineBounds();const auto view=workspace->dryViewport();
    const auto sample=snapshot.learnGeneration.displayed->positions.front().sample;
    const auto x=juce::roundToInt(bounds.getX()+(sample-view.first)/(view.second-view.first)*bounds.getWidth());
    const auto y=bounds.getY()+m3::ui::spacing::tight+2;
    const auto settled=workspace->createComponentSnapshot(workspace->getLocalBounds());
    REQUIRE(settled.getPixelAt(x,y)==m3::ui::separator);
    const auto event=[&](juce::ModifierKeys mods){const auto point=juce::Point<float>{static_cast<float>(x),static_cast<float>(y)};const auto time=juce::Time::getCurrentTime();
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,mods,0,0,0,0,0,workspace,workspace,time,point,time,1,false);};
    workspace->mouseMove(event({}));
    const auto hovered=workspace->createComponentSnapshot(workspace->getLocalBounds());REQUIRE(hovered.getPixelAt(x,y)==m3::ui::timing);
    captureImage(editor,"issue38-event-hover");
    workspace->mouseDown(event(juce::ModifierKeys::leftButtonModifier));workspace->mouseExit(event({}));
    REQUIRE(workspace->hasPinnedInspection() && workspace->diagnosticDetails().contains("Sample"));
    REQUIRE(!workspace->visibleInspectionText().contains("Sample") && !workspace->getDescription().contains("samples"));
    const auto pinned=workspace->createComponentSnapshot(workspace->getLocalBounds()).getPixelAt(x,bounds.getY()+21);
    REQUIRE(pinned.getRed()>pinned.getBlue()+64);
    captureImage(editor,"issue38-event-pinned");
    workspace->setVisible(false);workspace->setVisible(true);
    auto updating=snapshot;updating.learnGeneration.updating=true;
    workspace->display(fixture.processor->getCaptureSnapshot(),updating,fixture.processor->getSongTime(),fixture.processor->getWorkflowSnapshot(),false);
    const auto stale=workspace->createComponentSnapshot(workspace->getLocalBounds());
    REQUIRE(stale.getPixelAt(x,y)!=m3::ui::timing && workspace->timelineStatusText().contains("updating"));
    const auto waveformPixels=[&](float amplitude)
    {
        auto shown=snapshot;shown.session.automaticProcessing=false;
        auto signal=fixture.processor->getCaptureSnapshot();signal.manifest.ranges={{0,48000,CaptureCoverageKind::audio}};
        signal.overview={{signal.manifest.ranges.front(),true,{{0,48000,-amplitude,amplitude}}}};
        workspace->display(signal,shown,fixture.processor->getSongTime(),fixture.processor->getWorkflowSnapshot(),false);workspace->fitDrySource();
        const auto lane=workspace->timelineBounds();const auto viewport=workspace->dryViewport();
        const auto at=juce::roundToInt(lane.getX()+(24000-viewport.first)/(viewport.second-viewport.first)*lane.getWidth());
        const auto raster=workspace->createComponentSnapshot(workspace->getLocalBounds());int pixels=0;
        for(int row=lane.getY()+1;row<lane.getBottom()-1;++row)if(raster.getPixelAt(at,row)==m3::ui::text)++pixels;
        return pixels;
    };
    const auto quiet=waveformPixels(.25f),loud=waveformPixels(.5f);
    REQUIRE(quiet>0 && std::abs(loud-2*quiet)<=3); // Display is not normalized or secretly amplified per event.
    REQUIRE(fixture.engine().snapshot().renderCount==renderCount);
    juce::String a,b,error;REQUIRE(encode(before,a,error) && encode(fixture.engine().savedSession(),b,error) && a==b);
    editor.refresh();
}
static void selected(PreverbAudioProcessorEditor& editor,SwellEngine& engine,const juce::String& id)
{
    const auto snapshot=engine.snapshot();REQUIRE(snapshot.session.primary==id && snapshot.session.selected==std::vector<juce::String>{id});
    auto* list=dynamic_cast<m3::ui::controls::SwellList*>(named(editor,"swellList"));
    REQUIRE(list && list->primaryId()==id && list->isRowSelected(list->rowFor(id)));
    REQUIRE(visible(named(editor,"inspectorViewport")));
    const auto* swell=find(snapshot.session,id);REQUIRE(swell);
    const auto bounds=m3::ui::initialSwellBounds(*swell,SongTime{true,0,0,48000,120,4,4});
    const auto& view=editor.timelineComponent().viewport();REQUIRE(view.first<=bounds.first && view.last>=bounds.second);
    auto* overview=dynamic_cast<SwellOverview*>(named(editor,"songOverview"));REQUIRE(overview && !overview->detailBounds().isEmpty());
}
static void aboutBindings(bool review=false)
{
    GenerationFixture fixture;auto& engine=fixture.engine();
    juce::String copiedInformation;
    std::function<void(const juce::String&)> writer;
    if(!review)writer=[&](const juce::String& information){copiedInformation=information;};
    auto editor=std::make_unique<PreverbAudioProcessorEditor>(*fixture.processor,std::move(writer));editor->setVisible(true);editor->refresh();
    const auto showAbout=[&]{auto* action=dynamic_cast<juce::Button*>(named(*editor,"diagnostics"));REQUIRE(action);action->onClick();
        editor->settingsPanel()->selectPage(m3::ui::SettingsPage::about);};
    if(review)
    {
        juce::Timer::callAfterDelay(200,[safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(editor.get())]{if(!safe)return;
            auto* action=dynamic_cast<juce::Button*>(named(*safe,"diagnostics"));REQUIRE(action);action->onClick();
            safe->settingsPanel()->selectPage(m3::ui::SettingsPage::about);});
        controlReview::run(std::move(editor));return;
    }
    juce::String before,after,error;REQUIRE(encode(engine.savedSession(),before,error));
    const auto renders=engine.snapshot().renderCount;const auto capture=fixture.processor->getCaptureSnapshot().manifest.revision;
    showAbout();auto* panel=editor->settingsPanel();
    const auto metadata=juce::JSON::parse(juce::File(PREVERB_BUILD_INFO_FILE).loadFileAsString());REQUIRE(metadata.isObject());
    REQUIRE(metadata["version"].toString()==juce::String(m3::build::version.data()));
    REQUIRE(metadata["buildIdentifier"].toString()==juce::String(m3::build::identifier.data()));
    REQUIRE(metadata["gitSha"].toString()==juce::String(m3::build::gitSha.data()));
    REQUIRE(static_cast<bool>(metadata["worktreeDirty"])==m3::build::worktreeDirty);
    const juce::StringArray expectedDisplay{metadata["version"].toString(),metadata["buildIdentifier"].toString(),
        metadata["gitSha"].toString()+(static_cast<bool>(metadata["worktreeDirty"])?" (modified)":"")};
    for(int i=0;i<3;++i){auto* value=dynamic_cast<juce::Label*>(named(*panel,"aboutBuildValue"+juce::String(i)));REQUIRE(value && visible(value));
        REQUIRE(value->getText()==expectedDisplay[i] && value->getTooltip()==expectedDisplay[i]);
        REQUIRE(value->getParentComponent()->getLocalBounds().contains(value->getBounds()) && value->getAccessibilityHandler());}
    auto* copy=dynamic_cast<m3::ui::controls::ActionButton*>(named(*panel,"copyBuildInformation"));REQUIRE(copy && copy->isEnabled() && copy->getAccessibilityHandler());
    auto* status=dynamic_cast<juce::Label*>(named(*panel,"buildInformationCopyStatus"));REQUIRE(status && visible(status));
    REQUIRE(!status->getText().contains("copied to clipboard"));
    const auto fill=[&](bool over,bool down){juce::Image image(juce::Image::ARGB,copy->getWidth(),copy->getHeight(),true);
        {juce::Graphics graphics(image);copy->paintButton(graphics,over,down);}return image.getPixelAt(8,8);};
    const auto normal=fill(false,false),hover=fill(true,false),pressed=fill(true,true);
    REQUIRE(normal==m3::ui::aqua && hover==m3::ui::hover && pressed==m3::ui::aqua && normal!=hover);
    copy->setEnabled(false);REQUIRE(fill(false,false)!=normal);copy->setEnabled(true);
    controlReview::pointerClick(*copy);REQUIRE(copiedInformation==juce::String(m3::build::report()));
    REQUIRE(status->getText().contains("Build information copied to clipboard."));
    REQUIRE(status->getDescription()==status->getText() && copy->isEnabled());
    REQUIRE(status->getParentComponent()->getLocalBounds().contains(status->getBounds()));
    copiedInformation.clear();copy->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    await([&]{juce::MessageManager::getInstance()->runDispatchLoopUntil(10);return copiedInformation==juce::String(m3::build::report());});
    REQUIRE(status->getText().contains("copied to clipboard") && copy->isEnabled());
    REQUIRE(copiedInformation.contains(metadata["gitSha"].toString()) && copiedInformation.contains(metadata["buildIdentifier"].toString()));
    REQUIRE(encode(engine.savedSession(),after,error) && before==after);
    REQUIRE(engine.snapshot().renderCount==renders && fixture.processor->getCaptureSnapshot().manifest.revision==capture);
    captureImage(*panel,"alpha-about-settings");
}
static void alphaGuide()
{
    GenerationFixture fixture;auto& engine=fixture.engine();auto& processor=*fixture.processor;
    PreverbAudioProcessorEditor editor(processor);
    editor.enableResponsivePresentation(std::make_shared<m3::ui::TransientPresentationStorage>(),m3::ui::PresentationFitContext{{4000,3000},{0,0}});
    editor.setSize(1400,1000);editor.setVisible(true);editor.refresh();
    REQUIRE(processor.getWorkflowSnapshot().mode==workflow::Mode::capture && engine.savedSession().swells.empty());
    REQUIRE(!named(editor,"learnGenerateSwells")->isEnabled());captureImage(editor,"alpha-empty-learn");
    fixture.record();const auto markers=fixture.ready();editor.refresh();
    REQUIRE(markers.learnGeneration.displayed && !markers.learnGeneration.displayed->positions.empty());
    REQUIRE(named(editor,"learnGenerateSwells")->isEnabled());captureImage(editor,"alpha-captured-markers");
    auto* generate=dynamic_cast<m3::ui::controls::ActionButton*>(named(editor,"learnGenerateSwells"));REQUIRE(generate);
    controlReview::pointerClick(*generate);
    await([&]{const auto s=engine.snapshot();return processor.getWorkflowSnapshot().mode==workflow::Mode::live && !s.session.swells.empty()
        && std::all_of(s.session.swells.begin(),s.session.swells.end(),[](const auto& swell){return swell.phase==Phase::ready;});});
    editor.refresh();selected(editor,engine,m3::ui::firstChronologicalSwell(engine.savedSession()));
    await([&]{editor.refresh();return named(editor,"audition")->isEnabled();});
    captureImage(editor,"alpha-ready-perform");
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::finalise));
    await([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});editor.refresh();
    REQUIRE(visible(named(editor,"exportWorkspace")) && named(editor,"exportSave")->isEnabled() && named(editor,"exportDrag")->isEnabled());
    REQUIRE(engine.snapshot().songExport.prepared);captureImage(editor,"alpha-ready-export");
}
int main(int argc,char** argv){return runTest([&]
{
    const auto option=argc==2 ? juce::String(argv[1]) : juce::String{};
    if(option=="--review-song-timing")
    {
        juce::ScopedJuceInitialiser_GUI gui;
        songHostTimingFeedback(true);
        return;
    }
    if (option=="--review-empty-loop" || option=="--review-end-signal" || option=="--review-end-on-repeat")
    {
        juce::ScopedJuceInitialiser_GUI gui;
        LoopCaptureFixture fixture;
        auto& processor=*fixture.processor;
        auto preference=processor.getLoopCapturePreference();
        preference.enabled=option!="--review-empty-loop";
        if (preference.enabled) preference.method=option=="--review-end-on-repeat"
            ? LoopCapturePreference::Method::endOnRepeat : LoopCapturePreference::Method::endSignal;
        REQUIRE(processor.setLoopCapturePreference(preference));
        auto editor=std::make_unique<PreverbAudioProcessorEditor>(processor);
        editor->enableResponsiveReview({},m3::ui::PresentationFitContext{{1920,1080},{16,80}});
        controlReview::run(std::move(editor));
        return;
    }
    if (argc==2 && juce::String(argv[1])=="--review-loop")
    {
        juce::ScopedJuceInitialiser_GUI gui;
        loopUiCases(true);
        return;
    }
    if (argc==2 && juce::String(argv[1])=="--test-loop")
    {
        juce::ScopedJuceInitialiser_GUI gui;
        loopUiCases(false);
        return;
    }
    juce::ScopedJuceInitialiser_GUI gui;
    if(argc>1 && juce::String(argv[1])=="--guide-only"){alphaGuide();aboutBindings();return;}
    if(argc>1 && juce::String(argv[1])=="--review-about"){aboutBindings(true);return;}
    const bool review=argc>1 && juce::String(argv[1])=="--review";
    if(!review){songHostTimingFeedback();loopUiCases(false);}
    if(!review)aboutBindings();
    if(!review)controlBindings();
    if(!review)sharedSoundBindings();
    if(!review)pendingProtectedFraming();
    for(bool automatic:{false,true})
    {
        GenerationFixture fixture;auto& engine=fixture.engine();auto& processor=*fixture.processor;
        auto editor=std::make_unique<PreverbAudioProcessorEditor>(processor);editor->setVisible(true);editor->setSize(1100,850);editor->refresh();
        const SwellViewport original{1200000,1584000};editor->timelineComponent().setViewport(original);
        fixture.tick(4096,true);fixture.tick(8192,false);REQUIRE(processor.flushCaptureForTesting());
        await([&]{return processor.getWorkflowReadiness().usefulSource && !processor.getWorkflowSnapshot().locked();});
        // A useful-source, empty Perform visit does not establish an editing position.
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));editor->refresh();
        const auto emptyView=editor->timelineComponent().viewport();
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));editor->refresh();
        REQUIRE(sameView(emptyView,editor->timelineComponent().viewport()));
        auto seed=engine.savedSession();Swell late;late.arrival=late.sourceStart=200000;late.sourceEnd=204000;
        late.provenance=Provenance::manualCreated;late.name="Protected later swell";late.enabled=false;
        seed.swells.push_back(late);seed.primary.clear();seed.selected.clear();engine.restore(seed,processor.getCaptureSnapshot().reference);
        editor->refresh(); // Empty selection in a restored Learn session is still eligible.
        engine.holdLearnRendersForTesting(true);engine.holdLearnGenerationForTesting(!review);
        if(automatic)REQUIRE(engine.setAutoGenerateOnStop(true,engine.assignmentEpoch(),processor.getWorkflowSnapshot().pass));
        fixture.record();auto ready=automatic?engine.snapshot():fixture.ready();editor->refresh();
        if(!automatic)
        {
            candidateOverlay(*editor,fixture);
            auto* button=dynamic_cast<m3::ui::controls::ActionButton*>(named(*editor,"learnGenerateSwells"));REQUIRE(button && button->isEnabled());
            auto* helper=dynamic_cast<m3::ui::controls::WrappedLabel*>(named(*editor,"learnManualSwellsHelp"));REQUIRE(helper && visible(helper));
            REQUIRE(helper->getText().replace("\n"," ")=="Want to place swells yourself? Add manual swells in Perform.");
            REQUIRE(helper->textLayout(helper->getWidth()).getNumLines()>1 && helper->getFont().getHeight()==m3::ui::learnTypography::secondarySize);
            REQUIRE(!helper->getText().contains("?\n") && helper->getText().contains("? Add"));
            for(auto size:{juce::Point{1100,850},juce::Point{1400,1000},juce::Point{1600,1100}})
            {
                editor->setSize(size.x,size.y);editor->refresh();
                REQUIRE(helper->getWidth()>0 && helper->getHeight()>=helper->preferredHeight(helper->getWidth()));
                const auto* reason=named(*editor,"learnGenerateReason");REQUIRE(reason);
                REQUIRE(reason->getY()-button->getBottom()==m3::ui::spacing::labelHelp);
                REQUIRE(helper->getY()-reason->getBottom()==m3::ui::spacing::labelHelp);
                REQUIRE(named(*editor,"learnAutoGenerateOnStop")->getY()-named(*editor,"learnAutomaticSwells")->getBottom()==m3::ui::spacing::related);
                REQUIRE(helper->getParentComponent()->getLocalBounds().contains(helper->getBounds()));
                auto* learn=dynamic_cast<m3::ui::LearnWorkspace*>(named(*editor,"learnWorkspace"));REQUIRE(learn);
                REQUIRE(learn->timelineBounds().getHeight()<=learn->timelineBounds().getWidth()/2);
                REQUIRE(learn->timelineStatusText().contains("events") && !learn->timelineStatusText().contains("Mapped"));
                for(float scale:{1.f,1.25f,1.5f,2.f})captureImage(*editor,"issue38-learn-"+juce::String(size.x)+"-"+juce::String(scale),scale);
            }
            editor->setSize(1100,850);editor->refresh();
            if(review){engine.holdLearnRendersForTesting(false);controlReview::run(std::move(editor));return;}
            controlReview::pointerClick(*button);
        }
        // Pin preparation so neither fast explicit nor automatic completion can
        // be consumed by a refresh before the editor-closed assertion begins.
        await([&]{return engine.learnGenerationWaitingForTesting();});editor.reset();engine.holdLearnGenerationForTesting(false);
        try{await([&]{return processor.getWorkflowSnapshot().mode==workflow::Mode::live;});}
        catch(...){const auto view=engine.snapshot().learnGeneration;std::cerr<<"Initial handoff automatic="<<automatic<<" notice="<<view.notice<<" reason="<<view.reason
            <<" generating="<<view.generating<<" pass="<<processor.getWorkflowSnapshot().pass<<'\n';throw;}
        const auto committed=engine.snapshot();REQUIRE(committed.workflowState.mode==workflow::Mode::live && committed.initialPerformHandoff && committed.session.swells.size()>2);
        const auto first=m3::ui::firstChronologicalSwell(committed.session);
        REQUIRE(first!=committed.session.swells.front().id && committed.session.swells.front().id==late.id);
        // Commit with no editor refresh/reopen yet retains the pending handoff.
        editor=std::make_unique<PreverbAudioProcessorEditor>(processor);editor->setVisible(true);editor->refresh();
        selected(*editor,engine,first);REQUIRE(!engine.snapshot().initialPerformHandoff);
        const auto initialNavigation=engine.savedSession().performUi;
        REQUIRE(initialNavigation.hasView && sameView({initialNavigation.first,initialNavigation.last},editor->timelineComponent().viewport()));
        REQUIRE(!find(engine.savedSession(),late.id)->enabled && processor.getWorkflowSnapshot().mode==workflow::Mode::live);
        REQUIRE(!fixture.host.position.getIsPlaying());
        captureImage(*editor,automatic?"issue38-auto-first-pending":"issue38-explicit-first-pending");
        // Rendering cannot steal a newer selection or viewport, even across mode return.
        const auto second=committed.session.swells.back().id;REQUIRE(engine.selectTargets({second},second,engine.assignmentEpoch()));editor->refresh();
        const SwellViewport editing{90000,220000};editor->timelineComponent().setViewport(editing);
        editor->timelineComponent().onViewportChanged(); // Publish through the accepted #48 navigation binding.
        engine.holdLearnRendersForTesting(false);
        await([&]{const auto s=engine.snapshot();const auto* firstSwell=find(s.session,first);return firstSwell && firstSwell->phase==Phase::ready;});
        editor->refresh();REQUIRE(engine.savedSession().primary==second && sameView(editing,editor->timelineComponent().viewport()));
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));editor->refresh();fixture.ready();
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));editor->refresh();
        REQUIRE(engine.savedSession().primary==second && sameView(editing,editor->timelineComponent().viewport()));
        // Restoring a valid selection keeps the existing viewport and never offers a handoff.
        engine.restore(engine.savedSession(),processor.getCaptureSnapshot().reference);editor->refresh();
        REQUIRE(engine.savedSession().primary==second && !engine.snapshot().initialPerformHandoff && sameView(editing,editor->timelineComponent().viewport()));
        await([&]{const auto session=engine.savedSession();return std::all_of(session.swells.begin(),session.swells.end(),[](const auto& swell)
            {return swell.provenance!=Provenance::automatic || swell.phase==Phase::ready;});});
        editor->refresh();REQUIRE(sameView(editing,editor->timelineComponent().viewport()));
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));editor->refresh();fixture.ready();
        REQUIRE(fixture.generate());await([&]{return !engine.snapshot().learnGeneration.generating;});editor->refresh();
        REQUIRE(processor.getWorkflowSnapshot().mode==workflow::Mode::capture && sameView(editing,editor->timelineComponent().viewport()));
        REQUIRE(engine.assetDirectory().getChildFile(engine.savedSession().defaultIR.path).replaceWithText("Injected UI generation failure"));
        REQUIRE(fixture.generate());
        try{await([&]{const auto view=engine.snapshot().learnGeneration;return !view.generating && view.failed;});}
        catch(...){const auto view=engine.snapshot().learnGeneration;std::cerr<<"UI failure outcome: "<<view.notice<<" reason="<<view.reason<<" failed="<<view.failed<<" generating="<<view.generating<<'\n';throw;}
        editor->refresh();
        REQUIRE(processor.getWorkflowSnapshot().mode==workflow::Mode::capture && !engine.snapshot().initialPerformHandoff);
        REQUIRE(engine.savedSession().primary==second && sameView(editing,editor->timelineComponent().viewport()));
    }
});}
