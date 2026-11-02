#pragma once
inline void liveResourceCases()
{
    TestDirectory root;auto p=std::make_unique<PreverbAudioProcessor>(root.file);auto& engine=p->swellEngine();engine.setAutomaticProcessing(false);
    p->prepareToPlay(48000,4096);TestPlayHead head;p->setPlayHead(&head);head.position.setBpm(240);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    const auto position=[&](int64_t at,bool playing){head.position.setTimeInSamples(at);head.position.setPpqPosition(at/12000.0);head.position.setIsPlaying(playing);};
    juce::AudioBuffer<float> buffer(2,4096);juce::MidiBuffer midi;
    for(int64_t at=0;at<49152;at+=4096){position(at,true);for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),4096,.05f);p->processBlock(buffer,midi);}
    REQUIRE(p->flushCaptureForTesting());finishCaptureFixture(*p,head.position);position(0,false);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(engine.addTarget(12000));std::vector<float> ir(96000,.00001f);const auto file=root.file.getChildFile("long-ir.wav");writeTestWav(file,1,48000,ir);engine.loadIR(file);
    waitM3([&]{return engine.savedSession().swells[0].irName=="long-ir.wav";});engine.generate();waitM3([&]{return engine.snapshot().auditionReady;});
    auto state=engine.savedSession();const auto original=state.swells[0];state.swells.clear();
    for(int i=0;i<128;++i){auto w=original;w.id=juce::Uuid().toDashedString();w.name="Resource "+juce::String(i);w.placement=i<64?0:960000;state.swells.push_back(w);}
    state.primary=state.swells[0].id;state.selected={state.primary};engine.restore(state);
    head.position.setLoopPoints(juce::AudioPlayHead::LoopPoints{80,81});head.position.setIsLooping(true);position(-48000,false);buffer.clear();p->processBlock(buffer,midi);
    const auto deadline=juce::Time::getMillisecondCounter()+60000;
    while(true){const auto s=engine.snapshot();REQUIRE(engine.residentBytes()<=previewMemoryBudget);
        if(s.liveReady && s.liveLoopDestination==960000 && s.session.swells.back().phase==m3::Phase::ready && s.auditionReady)break;
        if(juce::Time::getMillisecondCounter()>deadline)throw std::runtime_error("Live resource timeout: "+s.liveStatus.toStdString()+" / "+s.message.toStdString());
        juce::Thread::sleep(2);}
    REQUIRE(engine.audition());position(-48000,true);buffer.clear();{AudioAllocationGuard guard;p->processBlock(buffer,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && !engine.snapshot().liveMissing);
    REQUIRE(buffer.getMagnitude(0,4096)>0 && engine.residentBytes()<=previewMemoryBudget);
    const auto denseBytes=engine.residentBytes();
    engine.generate(); // A render competes with pinned playback pages and retiring audition.
    position(-43904,true);buffer.clear();p->processBlock(buffer,midi);waitM3([&]{return engine.snapshot().liveReady && engine.savedSession().swells[0].phase==m3::Phase::ready;});
    REQUIRE(engine.residentBytes()<=previewMemoryBudget);
    // A 65th simultaneous contribution is explicitly refused, not silently lost.
    auto excess=engine.savedSession();auto extra=excess.swells[0];extra.id=juce::Uuid().toDashedString();excess.swells.push_back(extra);engine.restore(excess);buffer.clear();p->processBlock(buffer,midi);
    waitM3([&]{return engine.snapshot().liveStatus.contains("64 simultaneous");});REQUIRE(engine.residentBytes()<=previewMemoryBudget);
    // The same assets can describe months of sparse Song time without resident PCM growth.
    auto sparse=state;for(size_t i=0;i<sparse.swells.size();++i)sparse.swells[i].placement=static_cast<int64_t>(i)*1000000000;
    engine.restore(sparse);head.position.setIsLooping(false);position(0,false);buffer.clear();p->processBlock(buffer,midi);
    waitM3([&]{const auto s=engine.snapshot();return s.liveReady && s.session.swells.back().phase==m3::Phase::ready;});
    REQUIRE(engine.residentBytes()<=previewMemoryBudget);
    waitM3([&]{buffer.clear();p->processBlock(buffer,midi);return engine.residentBytes()<denseBytes/2;});
    std::cout<<"Live resource bytes: dense="<<denseBytes<<" sparse="<<engine.residentBytes()<<" budget="<<previewMemoryBudget<<'\n';
}
