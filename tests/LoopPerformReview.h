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
    auto& start=require<controls::ActionButton>(editor,"setLoopStart");
    auto& startPosition=require<juce::Label>(editor,"loopStartPosition");
    const auto checkHeadings=[&]{
        const auto section=m3::ui::textStyle(m3::ui::TextRole::sectionHeading);
        for(const auto* name:{"auditionHeading","loopHeading"})
        {
            auto& heading=require<juce::Label>(editor,name);
            const auto font=heading.getFont();
            REQUIRE(font.getHeight()==section.height && font.getTypefacePtr()==m3::ui::interTypeface(section.face));
        }
    };
    checkHeadings();
    REQUIRE(editor.setPresentationSize(1100,850));
    checkHeadings();
    REQUIRE(editor.setPresentationSize(900,600));
    REQUIRE(find(editor,"stopAudition")==nullptr);
    REQUIRE(start.isVisible() && !start.isEnabled() && start.getTooltip()=="Requires a host song position");
    REQUIRE(startPosition.getText()=="No start set");
    auto& readout=require<controls::TransportReadout>(editor,"hostInfo");
    const auto& values=readout.presentation().values;
    const auto positionUnavailable=[&]{return values[0]=="unavailable" && values[1]=="unavailable";};
    REQUIRE(readout.presentation().loopRelative && positionUnavailable());
    REQUIRE(values[2]=="137" && values[3]=="4 / 4");
    REQUIRE(cue.isVisible() && status.isVisible() && find(editor,"loopStop")==nullptr);
    REQUIRE(!cue.isEnabled() && cue.getButtonText()=="Cue Loop" && status.getText().isNotEmpty());
    until([&]{editor.refresh();return audition.isEnabled();});
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
    audition.invokeCommand();
    juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    fixture.tick(true);
    REQUIRE(engine.snapshot().playing);
    editor.refresh();
    REQUIRE(audition.getTitle()=="Stop audition" && audition.getVariant()==controls::CommandVariant::destructive);
    audition.invokeCommand();
    juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    for(int block=0;block<3;++block)fixture.tick();
    editor.refresh();
    REQUIRE(audition.getTitle()=="Play audition" && audition.getVariant()==controls::CommandVariant::primary);
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
    const auto lastSimulatedBlock=std::chrono::steady_clock::now();
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
    // These blocks represent continuous sample time; painting runs serially
    // here, whereas a host continues audio callbacks while the editor paints.
    if(std::chrono::steady_clock::now()-lastSimulatedBlock>std::chrono::milliseconds(500))
        std::cout<<"Continuing simulated Loop callbacks after slow waveform paint.\n";
    processor.resumeContinuousLoopPlaybackForTesting();
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

static void loopExportBindingCases()
{
    LoopCaptureFixture fixture;fixture.simpleClock=true;fixture.percussive=true;
    auto& processor=*fixture.processor;auto& engine=processor.swellEngine();engine.setAutomaticProcessing(false);
    auto settings=processor.getLoopCapturePreference();settings.count=1;
    REQUIRE(processor.setLoopCapturePreference(settings) && processor.armLoopCapture());
    fixture.through(fixture.marker(4),true);
    REQUIRE(processor.getCaptureSnapshot().manifest.loop);
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));
    const auto until=[&](const auto& predicate){
        const auto deadline=juce::Time::getMillisecondCounter()+15000;
        while(!predicate() && juce::Time::getMillisecondCounter()<deadline){fixture.tick();juce::Thread::sleep(2);}
        if(!predicate())throw std::runtime_error(engine.snapshot().message.toStdString());
    };
    until([&]{return engine.savedSession().defaultIR.present();});
    REQUIRE(engine.addTarget(0));const auto id=engine.savedSession().primary;
    const auto ir=fixture.root.file.getChildFile("export-binding.wav");writeTestWav(ir,1,48000,std::vector<float>(201,.002f));
    engine.loadIR(ir);until([&]{return m3::find(engine.savedSession(),id)->irName==ir.getFileName();});
    engine.setLivePlayback(true,m3::LiveRouting::wetOnly,-6);
    REQUIRE(engine.generate(true).accepted);until([&]{return m3::find(engine.savedSession(),id)->phase==m3::Phase::ready;});
    using Playing=m3::LoopCueScheduler::State;
    using Prep=m3::LoopPlaybackPublication::Preparation;
    until([&]{const auto status=processor.loopPlaybackStatus();return status.preparation==Prep::ready
        && status.phase==m3::LoopLiveReference::State::ready;});
    REQUIRE(processor.requestLoopCue());
    float livePeak=0;
    int64_t wetPosition=0;
    juce::AudioBuffer<float> liveWet(2,fixture.frames);
    const auto cueDeadline=fixture.marker(fixture.nextBeat+12);
    while(fixture.cursor<cueDeadline){
        fixture.tick(true);
        const auto status=processor.loopPlaybackStatus();
        if(status.playback==Playing::repeating)break;
        if(status.playback!=Playing::leadIn || !status.playhead)continue;
        const auto peak=fixture.audio.getMagnitude(0,fixture.frames);
        if(peak>livePeak){
            livePeak=peak;
            wetPosition=*status.playhead-fixture.frames+1;
            liveWet.makeCopyOf(fixture.audio);
        }
    }
    REQUIRE(livePeak>1e-5f && wetPosition<0);
    fixture.clockEnabled=false;fixture.through(fixture.marker(fixture.nextBeat+8));
    REQUIRE(!processor.requestLoopCue());
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::finalise));
    until([&]{return engine.snapshot().songExport.phase==m3::ExportPhase::ready;});
    {
        const auto file=engine.snapshot().songExport.prepared->file;
        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(
            engine.assetDirectory().getChildFile(file.path).createInputStream().release(),true));
        juce::AudioBuffer<float> decoded(2,fixture.frames);
        REQUIRE(reader && wetPosition>=file.origin
            && reader->read(&decoded,0,fixture.frames,wetPosition-file.origin,true,true));
        for(int channel=0;channel<2;++channel)
            for(int frame=0;frame<fixture.frames;++frame)
                REQUIRE(std::abs(decoded.getSample(channel,frame)-liveWet.getSample(channel,frame))<1e-6f);
    }
    PreverbAudioProcessorEditor editor(processor);editor.enableResponsiveReview({},PresentationFitContext{{1920,1080},{16,80}});
    REQUIRE(editor.setPresentationSize(900,600));editor.setVisible(true);editor.refresh();
    auto& workspace=require<ExportWorkspace>(editor,"exportWorkspace");auto& mix=workspace.mixControl();
    REQUIRE(workspace.isVisible() && workspace.loopOptions().isVisible() && mix.isVisible());
    REQUIRE(mix.selector.getText()=="Wet Layer" && mix.dry.getValue()==0 && mix.wet.getValue()==100);
    engine.holdExportForTesting(true);
    workspace.loopOptions().beforeInvocation();workspace.loopOptions().repeats.onStep(1);
    until([&]{return engine.exportWaitingForTesting();});editor.refresh();
    REQUIRE(workspace.displayedLoopRepeats()==1 && workspace.loopOptions().settings().repeats==2);
    REQUIRE(!require<controls::ActionButton>(editor,"exportSave").isEnabled());
    engine.holdExportForTesting(false);
    until([&]{return engine.snapshot().songExport.phase==m3::ExportPhase::ready;});editor.refresh();
    REQUIRE(workspace.displayedLoopRepeats()==2);
    REQUIRE(engine.snapshot().songExport.prepared->file.loop->options.repeats==2);
    mix.beforeInvocation();mix.selector.setSelectedId(5,juce::sendNotificationSync);
    until([&]{return engine.snapshot().songExport.phase==m3::ExportPhase::ready;});editor.refresh();
    REQUIRE(engine.snapshot().songExport.prepared->file.dry==1 && engine.snapshot().songExport.prepared->file.wet==1);
    REQUIRE(workspace.placementText().contains("Mute the original dry"));
    REQUIRE(engine.savedSession().exportState.dry==0 && engine.savedSession().exportState.wet==1);
    processor.prepareToPlay(44100,fixture.frames);fixture.tick();
    until([&]{return engine.snapshot().songExport.phase==m3::ExportPhase::ready;});editor.refresh();
    auto& play=require<controls::ActionButton>(editor,"exportPlay");auto& save=require<controls::ActionButton>(editor,"exportSave");
    REQUIRE(!play.isEnabled() && play.getTooltip().contains("48.0 kHz") && save.isEnabled());
    REQUIRE(engine.snapshot().songExport.prepared->file.rate==48000);
    processor.prepareToPlay(48000,fixture.frames);fixture.tick();editor.refresh();
    REQUIRE(play.isEnabled());

    REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));
    REQUIRE(engine.setFrozen(id,true));
    juce::MemoryBlock saved;
    processor.getStateInformation(saved);
    const auto captured=processor.getCaptureSnapshot();
    REQUIRE(saved.getSize()>0 && !captured.manifest.ranges.empty());
    const auto dry=engine.assetDirectory().getChildFile(
        juce::String(captured.manifest.ranges.front().assetPath));
    REQUIRE(dry.deleteFile());

    auto reopened=std::make_unique<PreverbAudioProcessor>(fixture.root.file);
    auto& restored=*reopened;
    restored.prepareToPlay(48000,fixture.frames);
    restored.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
    const auto waitFor=[&](const auto& condition){
        const auto deadline=juce::Time::getMillisecondCounter()+15000;
        while(!condition() && juce::Time::getMillisecondCounter()<deadline)juce::Thread::sleep(2);
        REQUIRE(condition());
    };
    waitFor([&]{const auto capture=restored.getCaptureSnapshot();return capture.unavailable
        && capture.manifestKnown && restored.getWorkflowReadiness().readySwell;});
    REQUIRE(!restored.getSourceEditTime(restored.getCaptureSnapshot()).valid);
    REQUIRE(!restored.requestWorkflowMode(workflow::Mode::live));
    REQUIRE(restored.requestWorkflowMode(workflow::Mode::finalise));
    auto& restoredEngine=restored.swellEngine();
    waitFor([&]{return restoredEngine.snapshot().songExport.phase==m3::ExportPhase::ready;});
    REQUIRE(!restoredEngine.snapshot().songExport.prepared->plan->warnings.empty());
    REQUIRE(restoredEngine.setExportLevels(0,1,restoredEngine.assignmentEpoch()));
    waitFor([&]{return restoredEngine.snapshot().songExport.phase==m3::ExportPhase::ready;});
    REQUIRE(restoredEngine.snapshot().songExport.prepared->plan->warnings.empty());
}
}
