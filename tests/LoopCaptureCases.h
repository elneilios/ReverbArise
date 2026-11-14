#pragma once
#include <thread>
inline void coherentHostTimingCases()
{
    struct SlowHead:juce::AudioPlayHead{
        PositionInfo position;std::atomic<bool> slow{false},entered{false},release{false};
        juce::Optional<PositionInfo> getPosition() const override{
            auto& self=*const_cast<SlowHead*>(this);
            if(slow.load()){self.entered=true;while(!release.load())std::this_thread::yield();}
            return position;}
    } head;
    TestDirectory root;auto p=std::make_unique<PreverbAudioProcessor>(root.file);p->prepareToPlay(48000,512);p->setPlayHead(&head);
    head.position.setTimeInSamples(0);head.position.setPpqPosition(0);head.position.setBpm(120);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    juce::AudioBuffer<float> buffer(2,512);buffer.clear();juce::MidiBuffer midi;p->processBlock(buffer,midi);REQUIRE(p->getSongTime().valid);
    head.slow=true;std::thread audio([&]{p->processBlock(buffer,midi);});
    const auto deadline=juce::Time::getMillisecondCounter()+2000;
    while(!head.entered.load() && juce::Time::getMillisecondCounter()<deadline)juce::Thread::sleep(1);
    bool valid=head.entered.load();for(int n=0;n<100;++n)valid=valid && p->getSongTime().valid;
    head.release=true;audio.join();REQUIRE(valid);
}
inline void loopCaptureCases()
{
    TestDirectory root;auto p=std::make_unique<PreverbAudioProcessor>(root.file);auto& engine=p->swellEngine();
    // This is a loop/readiness oracle, not an asynchronous factory-load race.
    // The latter is tested explicitly by generation_configuration_test.
    waitM3([&]{return engine.savedSession().defaultIR.present();});
    p->prepareToPlay(48000,4096);TestPlayHead head;p->setPlayHead(&head);head.position.setIsPlaying(true);
    head.position.setBpm(120);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    std::vector<float> source(96000*2);
    for(int onset:{12000,48000,90000})for(int n=0;n<6000;++n){const auto value=static_cast<float>(.25*std::exp(-n/1200.0)*std::sin(n*.17));source[(onset+n)*2]=value;source[(onset+n)*2+1]=value*.6f;}
    juce::AudioBuffer<float> buffer(2,4096);juce::MidiBuffer midi;
    const auto block=[&](int at,int frames){buffer.setSize(2,frames);for(int c=0;c<2;++c)for(int n=0;n<frames;++n)buffer.setSample(c,n,source[(at+n)*2+c]);
        head.position.setTimeInSamples(at);head.position.setPpqPosition(at/24000.0);
        {AudioAllocationGuard guard;p->processBlock(buffer,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);};
    REQUIRE(engine.setAutoGenerateOnStop(true));
    const auto pass=[&](int stride){head.position.setIsPlaying(true);for(int at=0;at<96000;at+=stride){block(at,std::min(stride,96000-at));juce::Thread::sleep(1);}
        head.position.setIsPlaying(false);block(0,512);REQUIRE(p->flushCaptureForTesting());};
    REQUIRE(setFixtureCaptureArmed(*p));
    pass(4096);
    waitM3([&]{const auto s=engine.savedSession();return s.swells.size()>=3 && s.swells.back().phase==m3::Phase::ready;});
    const auto finalId=engine.savedSession().swells.back().id;REQUIRE(m3::find(engine.savedSession(),finalId)->sourceEnd==96000);
    // Stop now closes the atomic source pass; later Live loops do not recapture.
    block(0,512);REQUIRE(p->flushCaptureForTesting());
    waitM3([&]{const auto s=engine.snapshot();return s.liveReady && !s.analyzing && s.pendingIntervals==0 && std::all_of(s.session.swells.begin(),s.session.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
    const auto initial=engine.savedSession();const auto captured=p->getCaptureSnapshot();const auto renders=engine.snapshot().renderCount;const auto analyses=engine.snapshot().analysisCount;
    REQUIRE(m3::find(initial,finalId)->sourceEnd==96000);
    for(int repeat=0;repeat<3;++repeat){pass(repeat%2?1536:4096);juce::Thread::sleep(30);
        const auto s=engine.snapshot();REQUIRE(s.renderCount==renders && s.analysisCount==analyses && s.session.swells.size()==initial.swells.size());
        REQUIRE(p->getCaptureSnapshot().reference==captured.reference);
        for(const auto& w:initial.swells){const auto current=m3::find(s.session,w.id);REQUIRE(current && current->phase==m3::Phase::ready && current->render==w.render && current->inputGeneration==w.inputGeneration);}}
    head.position.setIsPlaying(false);block(0,512);juce::Thread::sleep(120);
    REQUIRE(engine.snapshot().renderCount==renders && engine.snapshot().analysisCount==analyses);
    for(const auto& w:engine.savedSession().swells)REQUIRE(w.phase==m3::Phase::ready);
    {juce::MemoryBlock saved;p->getStateInformation(saved);auto reopened=std::make_unique<PreverbAudioProcessor>(root.file);
        reopened->prepareToPlay(48000,4096);reopened->setPlayHead(&head);reopened->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
        REQUIRE(reopened->flushCaptureForTesting());buffer.clear();reopened->processBlock(buffer,midi);
        auto& restored=reopened->swellEngine();
        waitM3([&]{const auto s=restored.savedSession();return !s.swells.empty() && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
        REQUIRE(m3::find(restored.savedSession(),finalId)->sourceEnd==96000);
        restored.setAutoMarkers(false);restored.setAutoMarkers(true);
        waitM3([&]{const auto s=restored.snapshot();return !s.analyzing && s.pendingIntervals==0;});
        REQUIRE(m3::find(restored.savedSession(),finalId)->sourceEnd==96000 && m3::find(restored.savedSession(),finalId)->phase==m3::Phase::ready);}
    // Source changes now require explicit Capture/Recapture, not sensitivity.
    engine.setRegenerationSensitivity(1);
    for(int n=48100;n<48120;++n){source[n*2]*=.5f;source[n*2+1]*=.5f;}
    armCaptureFixture(*p);head.position.setIsPlaying(true);pass(2048);
    waitM3([&]{const auto s=engine.snapshot();return s.renderCount>renders && !s.analyzing && s.pendingIntervals==0 && std::all_of(s.session.swells.begin(),s.session.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
    for(const auto& old:initial.swells)if(old.sourceStart>49000 || old.sourceEnd<48000)REQUIRE(m3::find(engine.savedSession(),old.id)->render==old.render);
    // A useful later take triggers Generate on Stop. Its contiguous coverage
    // removes the old inferred cap and allows the full automatic source window.
    REQUIRE(p->requestWorkflowMode(workflow::Mode::capture) && setFixtureCaptureArmed(*p));head.position.setIsPlaying(true);
    for(int at=96000;at<120000;at+=4096){const auto count=std::min(4096,120000-at);buffer.setSize(2,count);buffer.clear();
        // Keep this below the preceding onset and introduce no new attack.
        for(int channel=0;channel<2;++channel)std::fill_n(buffer.getWritePointer(channel),count,.00001f);
        head.position.setTimeInSamples(at);head.position.setPpqPosition(at/24000.0);p->processBlock(buffer,midi);}
    head.position.setIsPlaying(false);block(0,512);REQUIRE(p->flushCaptureForTesting());
    try{waitM3([&]{const auto state=engine.savedSession();const auto last=m3::find(state,finalId);return last && last->sourceEnd==last->arrival+24000 && last->phase==m3::Phase::ready;});}
    catch(...){const auto state=engine.snapshot();const auto last=m3::find(state.session,finalId);
        const auto snapshot=p->getCaptureSnapshot();
        std::cerr<<"Extension mode="<<static_cast<int>(p->getWorkflowSnapshot().mode)
                 <<" result="<<static_cast<int>(p->getWorkflowSnapshot().result)
                 <<" revision="<<snapshot.manifest.revision<<" capture="<<snapshot.status<<" message="<<state.message
                 <<" generation="<<state.learnGeneration.notice;
        for(const auto& range:snapshot.manifest.ranges)std::cerr<<" range="<<range.startSample<<"+"<<range.frameCount;
        if(last)std::cerr<<" arrival="<<last->arrival<<" end="<<last->sourceEnd<<" status="<<last->status;
        std::cerr<<'\n';throw;}
}
