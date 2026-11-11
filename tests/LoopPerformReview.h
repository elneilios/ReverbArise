#pragma once
#include <chrono>
#include <source_location>
#include <thread>

namespace preview46
{
static void writeLoopPreview(const juce::File& png,const juce::Image& image)
{
    if(png.existsAsFile())REQUIRE(png.deleteFile());
    auto stream=png.createOutputStream();
    REQUIRE(stream && juce::PNGImageFormat().writeImageToStream(image,*stream));
}

static void loopPerformReview(bool review,const juce::File& output)
{
    using Phase=m3::LoopLiveReference::State;
    using Prep=m3::LoopPlaybackPublication::Preparation;
    using Playing=m3::LoopCueScheduler::State;
    LoopCaptureFixture fixture;
    fixture.percussive=true;
    fixture.simpleClock=true;
    fixture.simplePhrase=true;
    auto& processor=*fixture.processor;
    processor.setLoopPlaybackReference({{48000,137},16,4,4,true},true);
    auto& engine=processor.swellEngine();
    // The native fixture normally advances audio synchronously. Painting/PNG
    // output can exceed the production callback-gap guard in Debug. Feed the
    // actual processor on an independent callback thread while capturing UI,
    // just as a host continues audio while its message thread paints.
    const auto withAudioCallbacks=[&](auto&& capture)
    {
        std::jthread callbacks([&](std::stop_token stop)
        {
            while(!stop.stop_requested())
            {
                fixture.tick();
                std::this_thread::sleep_for(std::chrono::duration<double>(fixture.frames/fixture.rate));
            }
        });
        return capture(); // jthread stops and joins before the next fixture edit.
    };
    engine.setAutomaticProcessing(false);
    auto preference=processor.getLoopCapturePreference();
    preference.count=4;
    REQUIRE(processor.setLoopCapturePreference(preference));
    REQUIRE(processor.armLoopCapture());
    fixture.through(fixture.marker(16),true);
    const auto capture=processor.getCaptureSnapshot();
    if(!capture.manifest.loop || capture.manifest.loop->beats!=16)
        throw std::runtime_error(processor.loopCaptureStatus().toStdString());
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));
    REQUIRE(!processor.requestWorkflowMode(workflow::Mode::finalise));
    REQUIRE(engine.addTarget(0));
    const auto first=engine.savedSession().primary;
    REQUIRE(engine.addTarget(84088));
    const auto second=engine.savedSession().primary;
    REQUIRE(first!=second && engine.selectTargets({first,second},first));
    const auto ir=fixture.root.file.getChildFile("Loop Perform preview impulse.wav");
    std::vector<float> impulse(14400);
    for(size_t i=0;i<impulse.size();++i)
        impulse[i]=static_cast<float>(.12*std::exp(-static_cast<double>(i)/3200)*std::cos(i*.034));
    writeTestWav(ir,1,48000,impulse);
    engine.loadIR(ir);
    const auto until=[&](const auto& predicate,const std::source_location where=std::source_location::current())
    {
        const auto deadline=juce::Time::getMillisecondCounter()+12000;
        while(!predicate() && juce::Time::getMillisecondCounter()<deadline)
        {fixture.tick();juce::Thread::sleep(2);}
        if(!predicate())
        {
            const auto state=engine.snapshot();const auto playback=processor.loopPlaybackStatus();
            std::cerr<<"Loop readiness timeout at "<<where.line()<<" audition="<<state.auditionReady
                <<" phase="<<static_cast<int>(playback.phase)<<" playback="<<static_cast<int>(playback.playback)
                <<" preparation="<<static_cast<int>(playback.preparation)<<" message="<<state.message<<"\n";
            for(const auto& swell:state.session.swells)std::cerr<<"Swell "<<swell.name<<" phase="<<static_cast<int>(swell.phase)<<" "<<swell.status<<"\n";
            throw std::runtime_error("Loop readiness did not settle");
        }
    };
    until([&]{const auto session=engine.savedSession();return session.swells.size()==2
        && session.swells[0].irName==ir.getFileName() && session.swells[1].irName==ir.getFileName();});
    REQUIRE(engine.selectTargets({first},first));
    engine.generate(true);
    until([&]{return engine.snapshot().auditionReady && processor.loopPlaybackStatus().preparation==Prep::ready;});
    REQUIRE(engine.snapshot().sourceDetail && engine.snapshot().raw && engine.snapshot().raw->start<0);
    REQUIRE(!engine.edit(first,[](auto& swell,auto&){swell.enabled=false;return true;},engine.assignmentEpoch(),nullptr,capture.manifest.revision+1));

    PreverbAudioProcessorEditor editor(processor);
    editor.enableResponsiveReview({},PresentationFitContext{{1920,1080},{16,80}});
    REQUIRE(editor.setPresentationSize(900,600));
    editor.setVisible(true);
    editor.refresh();
    auto& cue=require<controls::ActionButton>(editor,"loopCue");
    auto& status=require<juce::Label>(editor,"loopPlaybackStatus");
    auto& audition=require<controls::ActionButton>(editor,"audition");
    auto& auditionStatus=require<juce::Label>(editor,"auditionStatus");
    auto& readout=require<controls::TransportReadout>(editor,"hostInfo");
    const auto& values=readout.presentation().values;
    const auto positionUnavailable=[&]{return values[0]=="unavailable" && values[1]=="unavailable";};
    REQUIRE(readout.presentation().loopRelative && positionUnavailable());
    REQUIRE(values[2]=="137" && values[3]=="4 / 4");
    REQUIRE(cue.isVisible() && status.isVisible() && find(editor,"loopStop")==nullptr);
    REQUIRE(!cue.isEnabled() && cue.getButtonText()=="Cue Loop" && status.getText().isNotEmpty());
    REQUIRE(audition.isEnabled());
    REQUIRE(editor.timelineComponent().gridAvailable());
    auto& placement=require<KnobValue>(editor,"wetPlacementCell");
    placement.valueField().setText("-25",false);placement.valueField().onReturnKey();
    const auto edited=engine.savedSession();
    REQUIRE(m3::find(edited,first)->placement==-1200 && m3::find(edited,second)->placement==0);
    auto& reset=require<controls::ActionButton>(editor,"resetWetShape");
    reset.invokeCommand();juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    REQUIRE(m3::find(engine.savedSession(),first)->placement==0);
    placement.valueField().setText("-25",false);placement.valueField().onReturnKey();
    REQUIRE(m3::find(engine.savedSession(),first)->placement==-1200);
    REQUIRE(engine.selectTargets({second},second));editor.refresh();
    REQUIRE(engine.savedSession().primary==second && editor.timelineComponent().gridAvailable());
    auto& freeze=require<juce::ToggleButton>(editor,"freezeRender");
    freeze.setToggleState(true,juce::dontSendNotification);freeze.onClick();
    REQUIRE(m3::find(engine.savedSession(),second)->frozen);
    freeze.setToggleState(false,juce::dontSendNotification);freeze.onClick();
    REQUIRE(!m3::find(engine.savedSession(),second)->frozen);
    auto& timeline=editor.timelineComponent();
    timeline.fitSelection();timeline.setSnap(false);
    const auto lane=timeline.sourceLane();
    const auto oldEnd=m3::find(edited,second)->sourceEnd;
    const auto endX=static_cast<float>(timeline.viewport().pixel(oldEnd,lane.getX(),lane.getWidth()));
    const juce::Point<float> handle{endX,lane.getY()+lane.getHeight()*.75f};
    REQUIRE(timeline.beginDrag(handle));
    timeline.dragTo(handle.translated(-8,0));
    REQUIRE(m3::find(engine.savedSession(),second)->sourceEnd<oldEnd);
    timeline.focusLost(juce::Component::focusChangedDirectly);
    timeline.setSnap(true);
    REQUIRE(engine.selectTargets({first},first));editor.refresh();
    REQUIRE(engine.generate(true).accepted);
    until([&]{
        const auto state=engine.snapshot();
        const auto* other=m3::find(state.session,second);
        return other && other->phase==m3::Phase::ready && state.auditionReady
            && processor.loopPlaybackStatus().preparation==Prep::ready;
    });
    editor.refresh();timeline.fitSelection();
    REQUIRE(engine.audition());
    fixture.tick(true);
    REQUIRE(engine.snapshot().playing);
    engine.stop();
    for(int block=0;block<3;++block)fixture.tick();
    if(output!=juce::File{})
    {
        withAudioCallbacks([&]
        {
            REQUIRE(output.createDirectory().wasOk());
            for(const auto scale:{100,125})
            {
                REQUIRE(editor.setPresentationScale(scale));editor.refresh();
                writeLoopPreview(output.getChildFile("loop-blocked-"+juce::String(scale)+".png"),
                    editor.createComponentSnapshot(editor.getLocalBounds(),true));
            }
            REQUIRE(editor.setPresentationScale(100));
        });
    }
    fixture.dryOffset=2171;
    fixture.through(fixture.marker(80));
    editor.refresh();
    REQUIRE(processor.loopPlaybackStatus().phase==Phase::ready && cue.isEnabled());
    REQUIRE(cue.getButtonText()=="Cue Loop" && status.getText()=="Ready");
    const auto readyBounds=cue.getBounds();
    REQUIRE(cue.keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    fixture.tick();
    editor.refresh();
    REQUIRE(processor.loopPlaybackStatus().playback==Playing::queued && cue.isEnabled() && cue.getButtonText()=="Cancel Loop");
    REQUIRE(!audition.isEnabled() && audition.getTooltip()=="Cancel the queued Loop to audition");
    REQUIRE(status.getText()=="Waiting for bar");
    REQUIRE(cue.getBounds()==readyBounds && cue.getTooltip().contains("Cancel"));
    if(output!=juce::File{})
        withAudioCallbacks([&]{writeLoopPreview(output.getChildFile("loop-queued.png"),editor.createComponentSnapshot(editor.getLocalBounds(),true));});
    cue.invokeCommand();
    juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    fixture.tick();editor.refresh();
    if(!(processor.loopPlaybackStatus().playback==Playing::standby && cue.getButtonText()=="Cue Loop" && status.getText()=="Ready"))
        std::cerr<<"Loop cancel state="<<static_cast<int>(processor.loopPlaybackStatus().playback)<<" phase="<<static_cast<int>(processor.loopPlaybackStatus().phase)<<" status="<<status.getText()<<" button="<<cue.getButtonText()<<"\n";
    REQUIRE(processor.loopPlaybackStatus().playback==Playing::standby && cue.getButtonText()=="Cue Loop" && status.getText()=="Ready");
    cue.invokeCommand();
    juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    fixture.through(fixture.marker(96)-4096);
    editor.refresh();
    REQUIRE(processor.loopPlaybackStatus().playback==Playing::leadIn && cue.getButtonText()=="Stop Loop" && cue.isEnabled());
    REQUIRE(status.getText()=="Playing lead-in");
    REQUIRE(positionUnavailable());
    REQUIRE(!processor.loopPlaybackStatus().nextLeadInStart);
    fixture.through(fixture.marker(96));
    editor.refresh();
    REQUIRE(processor.loopPlaybackStatus().playback==Playing::repeating && cue.getButtonText()=="Stop Loop" && cue.isEnabled());
    REQUIRE(status.getText()=="Playing loop");
    REQUIRE(!audition.isEnabled() && audition.getTooltip()=="Stop Loop to audition");
    REQUIRE(values[0]=="1" && values[1].getDoubleValue()>=1 && values[1].getDoubleValue()<2);
    REQUIRE(cue.getBounds()==readyBounds && cue.getTooltip().contains("next lead-in"));
    if(output!=juce::File{})
        withAudioCallbacks([&]{writeLoopPreview(output.getChildFile("loop-stop-active.png"),editor.createComponentSnapshot(editor.getLocalBounds(),true));});
    const auto previousView=timeline.viewport();
    const auto phraseEnd=capture.manifest.loop->endSample;
    const m3::SwellViewport finalBar{phraseEnd * .75,static_cast<double>(phraseEnd)};
    timeline.setViewport(finalBar);
    fixture.through(fixture.marker(112)-4096);
    editor.refresh();
    const auto playing=processor.loopPlaybackStatus();
    REQUIRE(playing.playhead && playing.nextLeadInStart && *playing.playhead > *playing.nextLeadInStart);
    REQUIRE(values[0]=="4");
    const auto leadIn=engine.snapshot().loopLeadIn;
    REQUIRE(leadIn.peaks && std::any_of(leadIn.peaks->begin(),leadIn.peaks->end(),
        [](const auto& peak){return peak.maximum > peak.minimum;}));
    withAudioCallbacks([&]{timeline.createComponentSnapshot(timeline.getLocalBounds(),true);});
    const auto paintedColumns=timeline.wetPaintMetrics().columnsPainted;
    REQUIRE(paintedColumns > 0);
    REQUIRE(engine.selectTargets({second},second));
    editor.refresh();
    timeline.setViewport(finalBar);
    REQUIRE(engine.snapshot().loopLeadIn == leadIn);
    const auto otherSelection=withAudioCallbacks([&]{return editor.createComponentSnapshot(editor.getLocalBounds(),true);});
    REQUIRE(timeline.wetPaintMetrics().columnsPainted == paintedColumns);
    if(output!=juce::File{})
        withAudioCallbacks([&]{writeLoopPreview(output.getChildFile("loop-next-lead-in.png"),otherSelection);});
    REQUIRE(engine.selectTargets({first},first));
    editor.refresh();
    timeline.setViewport(previousView);
    cue.invokeCommand();
    juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    fixture.tick();editor.refresh();
    if(!(processor.loopPlaybackStatus().playback==Playing::finishing && cue.getButtonText()=="Stop Now"))
        std::cerr<<"Loop finish state="<<static_cast<int>(processor.loopPlaybackStatus().playback)<<" phase="<<static_cast<int>(processor.loopPlaybackStatus().phase)<<" status="<<status.getText()<<" button="<<cue.getButtonText()<<"\n";
    REQUIRE(processor.loopPlaybackStatus().playback==Playing::finishing && cue.getButtonText()=="Stop Now");
    REQUIRE(!processor.loopPlaybackStatus().nextLeadInStart);
    // Audio continued during the screenshots, so Stop may now belong to the
    // following pass. Drive through that pass's actual observed boundary.
    const auto stopBoundaryBeat=((fixture.nextBeat+fixture.phraseBeats-1)/fixture.phraseBeats)*fixture.phraseBeats;
    fixture.through(fixture.marker(stopBoundaryBeat)+512);
    until([&]{const auto state=processor.loopPlaybackStatus();
        return state.playback==Playing::standby && state.preparation==Prep::ready && state.phase==Phase::ready;});
    editor.refresh();
    REQUIRE(processor.loopPlaybackStatus().playback==Playing::standby && cue.isEnabled());
    REQUIRE(positionUnavailable() && values[2]=="137");

    if(output!=juce::File{})
    {
        withAudioCallbacks([&]
        {
            REQUIRE(output.createDirectory().wasOk());
            Composition song;
            auto songEditor=song.createComparisonEditor();
            songEditor->enableResponsiveReview({},PresentationFitContext{{1920,1080},{16,80}});
            REQUIRE(songEditor->setPresentationSize(900,600));
            songEditor->setVisible(true);
            for(const auto scale:{100,125})
            {
                REQUIRE(songEditor->setPresentationScale(scale) && editor.setPresentationScale(scale));
                songEditor->refresh();editor.refresh();
                REQUIRE(!require<controls::ActionButton>(*songEditor,"loopCue").isVisible());
                const auto songImage=songEditor->createComponentSnapshot(songEditor->getLocalBounds(),true);
                const auto loopImage=editor.createComponentSnapshot(editor.getLocalBounds(),true);
                juce::Image comparison(juce::Image::RGB,songImage.getWidth()+loopImage.getWidth(),loopImage.getHeight(),true);
                {juce::Graphics g(comparison);g.drawImageAt(songImage,0,0);g.drawImageAt(loopImage,songImage.getWidth(),0);}
                writeLoopPreview(output.getChildFile("song-loop-"+juce::String(scale)+".png"),comparison);
            }
            REQUIRE(editor.setPresentationScale(100));
        });
    }

    const int missedPhraseBeat=((fixture.nextBeat+15)/16)*16;
    fixture.simplePhrase=false;
    fixture.through(fixture.marker(missedPhraseBeat)+128);
    editor.refresh();
    REQUIRE(processor.loopPlaybackStatus().phase==Phase::lost && !cue.isEnabled());
    fixture.simplePhrase=true;
    fixture.through(fixture.marker(missedPhraseBeat+48));
    editor.refresh();
    REQUIRE(processor.loopPlaybackStatus().phase==Phase::ready && cue.isEnabled());
    std::cout<<"Loop Perform native layout, real source and wet, Cue, Stop, loss and re-lock passed.\n";
    if(!review)
    {
        auto& add=require<controls::ActionButton>(editor,"Add at playhead");
        REQUIRE(add.getTitle()=="Add at view start");
        REQUIRE(processor.requestLoopCue());
        fixture.through(fixture.marker(missedPhraseBeat+80)+512);
        editor.refresh();
        REQUIRE(values[0]=="1" && values[1].getDoubleValue()>=1 && values[1].getDoubleValue()<2);
        fixture.through(fixture.marker(missedPhraseBeat+84)+1024);
        editor.refresh();
        const auto playhead=timeline.transportPlayhead();
        REQUIRE(playhead && *playhead>84088 && add.getTitle()=="Add at playhead");
        add.invokeCommand();
        juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
        const auto playingSession=engine.savedSession();
        const auto* inserted=m3::find(playingSession,playingSession.primary);
        REQUIRE(playingSession.swells.size()==3 && inserted && inserted->arrival==*playhead);

        processor.requestLoopPlaybackStop();
        fixture.tick();
        processor.requestLoopPlaybackStop();
        for(int block=0;block<3;++block)fixture.tick();
        editor.refresh();
        timeline.setViewport({42000,126000});
        REQUIRE(add.getTitle()=="Add at view start");
        add.invokeCommand();
        juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
        const auto saved=engine.savedSession();
        inserted=m3::find(saved,saved.primary);
        REQUIRE(saved.swells.size()==4 && inserted && inserted->arrival==42000);

        auto wrongSource=timeline.targetIdentity();
        wrongSource.reference+=".other";
        REQUIRE(!engine.addTarget(50000,nullptr,wrongSource));
        const auto stale=timeline.targetIdentity();
        engine.restore(saved);
        REQUIRE(!engine.addTarget(50000,nullptr,stale));
        REQUIRE(engine.savedSession().swells.size()==saved.swells.size());

        REQUIRE(engine.selectTargets({first},first));
        until([&]{return engine.snapshot().auditionReady;});
        processor.prepareToPlay(44100,fixture.frames);
        fixture.tick();
        editor.refresh();
        REQUIRE(!cue.isEnabled() && status.getText()=="Loop needs 48 kHz");
        REQUIRE(status.getTooltip().contains("Set the host sample rate to 48 kHz"));
        REQUIRE(!audition.isEnabled() && auditionStatus.getText()=="Audition needs 48 kHz");
        REQUIRE(!processor.auditionPreview());
        auto& help=require<juce::Label>(editor,"hoverHelp");
        const auto hover=[&](juce::Component& control){
            const auto now=juce::Time::getCurrentTime();
            const auto point=control.getLocalBounds().getCentre().toFloat();
            editor.mouseMove(juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),
                point,{},0,0,0,0,0,&control,&control,now,point,now,0,false));
        };
        hover(audition);
        REQUIRE(help.getText()=="Set the host sample rate to 48 kHz to audition this Loop.");
        hover(cue);
        REQUIRE(help.getText()=="Set the host sample rate to 48 kHz to play this Loop.");
        processor.prepareToPlay(48000,fixture.frames);
        fixture.tick();
        editor.refresh();
        REQUIRE(audition.isEnabled() && processor.auditionPreview());
        engine.stop();
        return;
    }

    // Open on repeating wet so the active command is visible immediately.
    REQUIRE(processor.requestLoopCue());
    REQUIRE(engine.selectTargets({second},second));
    fixture.through(fixture.marker(missedPhraseBeat+80)-6000);
    editor.refresh();
    timeline.setViewport(finalBar);
    REQUIRE(cue.isEnabled() && cue.getButtonText()=="Stop Loop" && status.getText()=="Playing loop");

    class Window final:public juce::DocumentWindow,private juce::Timer
    {
    public:
        Window(PreverbAudioProcessorEditor& editor,LoopCaptureFixture& source)
            :DocumentWindow("ReverbArise - Loop Perform readout preview",background,closeButton),fixture(source)
        {
            setUsingNativeTitleBar(true);setContentNonOwned(&editor,true);
            centreWithSize(editor.getWidth(),editor.getHeight());setVisible(true);toFront(true);
            startTimer(40);
        }
        void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
    private:
        void timerCallback()override{for(int block=0;block<15;++block)fixture.tick();}
        LoopCaptureFixture& fixture;
    };
    Window window(editor,fixture);
    juce::MessageManager::getInstance()->runDispatchLoop();
}
}
