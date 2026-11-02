#pragma once
inline void liveProcessorCases()
{
    TestDirectory root;auto p=std::make_unique<PreverbAudioProcessor>(root.file);auto& engine=p->swellEngine();
    REQUIRE(engine.setGainPolicy(m3::GainPolicy::legacy)); // Original live sample oracle.
    engine.setAutomaticProcessing(false);p->prepareToPlay(1000,1000);TestPlayHead head;p->setPlayHead(&head);
    head.position.setBpm(240);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    const auto position=[&](int64_t at,bool playing){head.position.setIsPlaying(playing);head.position.setTimeInSamples(at);head.position.setPpqPosition(at/250.0);};
    juce::AudioBuffer<float> buffer(2,1000);juce::MidiBuffer midi;position(0,true);
    for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),1000,.125f);
    p->processBlock(buffer,midi);REQUIRE(p->flushCaptureForTesting());finishCaptureFixture(*p,head.position);position(0,false);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(engine.addTarget(500));const auto first=engine.savedSession().primary;REQUIRE(engine.addTarget(520));const auto second=engine.savedSession().primary;
    REQUIRE(engine.selectTargets({first,second},first));std::vector<float> ir(101,.001f);auto file=root.file.getChildFile("live-ir.wav");writeTestWav(file,1,1000,ir);engine.loadIR(file);
    waitM3([&]{return engine.savedSession().swells[0].irName=="live-ir.wav" && engine.savedSession().swells[1].irName=="live-ir.wav";});engine.generate(true);
    waitM3([&]{return engine.snapshot().liveReady && engine.snapshot().auditionReady && engine.savedSession().swells[1].phase==m3::Phase::ready;});
    REQUIRE(engine.audition()); // Pending manual voice must lose priority on host Play.
    buffer.setSize(2,16);position(450,true);buffer.clear();
    {AudioAllocationGuard guard;p->processBlock(buffer,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    for(int n=5;n<16;++n){const auto expected=((450+n-400+1)+(450+n-420+1))*.000125f;
        REQUIRE(std::abs(buffer.getSample(0,n)-expected)<1e-6f && std::abs(buffer.getSample(1,n)-expected)<1e-6f);}
    REQUIRE(!engine.audition()); // Live Song playback has priority.
    REQUIRE(!engine.snapshot().playing);
    REQUIRE(engine.edit(first,[](auto& w,auto&){w.muted=true;return true;}));position(466,true);buffer.clear();p->processBlock(buffer,midi);
    for(int n=5;n<16;++n)REQUIRE(std::abs(buffer.getSample(0,n)-(466+n-420+1)*.000125f)<1e-6f);
    const auto originalShape=engine.savedSession().swells[1].shape;
    REQUIRE(engine.edit(second,[](auto& w,auto& error){auto shape=w.shape;shape.endOffset=-99;shape.fadeIn=shape.fadeOut=0;return m3::setShape(w,shape,w.render.start,w.render.sourceEnd,error);}));
    waitM3([&]{return engine.snapshot().liveReady;});position(416,true);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(buffer.getMagnitude(5,11)==0); // One-frame crop is silent after the old wet's <=5 ms discharge.
    REQUIRE(engine.edit(second,[&](auto& w,auto&){w.shape=originalShape;return true;}));waitM3([&]{return engine.snapshot().liveReady;});
    armCaptureFixture(*p);position(450,true);buffer.clear();p->processBlock(buffer,midi);REQUIRE(buffer.getMagnitude(5,11)==0);
    finishCaptureFixture(*p,head.position);
    {const auto zeroMap=p->getCaptureSnapshot().manifest;CaptureAssetStore zeroStore(root.file.getChildFile(zeroMap.projectId).getChildFile(zeroMap.instanceId));SourceInterval zero;juce::String error;
        REQUIRE(readSourceInterval(zeroStore,zeroMap,450,466,1000,zero,error));for(auto value:zero.samples)REQUIRE(value==0);}
    // Dry passthrough and capture are independent of Wet Only routing.
    engine.setLivePlayback(true,m3::LiveRouting::wetOnly,0);armCaptureFixture(*p);position(450,true);
    for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),16,.25f);
    p->processBlock(buffer,midi);finishCaptureFixture(*p,head.position);
    REQUIRE(buffer.getSample(0,15)==.25f); // Capture always passes original dry.
    const auto map=p->getCaptureSnapshot().manifest;
    CaptureAssetStore store(root.file.getChildFile(map.projectId).getChildFile(map.instanceId));SourceInterval dry;juce::String error;
    REQUIRE(readSourceInterval(store,map,450,466,1000,dry,error));for(auto value:dry.samples)REQUIRE(value==.25f);
    finishCaptureFixture(*p,head.position);engine.setLivePlayback(false,m3::LiveRouting::dryWet,0);position(466,false);
    for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),16,.125f);p->processBlock(buffer,midi);
    REQUIRE(buffer.getSample(0,15)==.125f);
    REQUIRE(engine.residentBytes()<=previewMemoryBudget);
    REQUIRE(engine.edit(second,[](auto& w,auto& error){return m3::moveWet(w,40,error);}));
    engine.setLivePlayback(true,m3::LiveRouting::dryWet,0);position(490,false);buffer.clear();p->processBlock(buffer,midi);
    waitM3([&]{return engine.snapshot().liveReady;});REQUIRE(engine.selectTargets({first},first));position(490,true);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(std::abs(buffer.getSample(0,10)-41*.000125f)<1e-6f);
    // Session solo filtering matches existing editing/audition semantics.
    REQUIRE(engine.edit(first,[](auto& w,auto&){w.solo=true;w.muted=false;return true;}));
    position(450,false);buffer.clear();p->processBlock(buffer,midi);waitM3([&]{return engine.snapshot().liveReady;});position(450,true);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(std::abs(buffer.getSample(0,10)-61*.000125f)<1e-6f);
    REQUIRE(engine.edit(first,[](auto& w,auto&){w.enabled=false;return true;}));
    position(490,false);buffer.clear();p->processBlock(buffer,midi);waitM3([&]{return engine.snapshot().liveReady;});position(490,true);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(buffer.getSample(0,10)==0); // Disabled solo still excludes non-solo neighbours.
    REQUIRE(engine.edit(first,[](auto& w,auto&){w.solo=false;return true;}));
    position(490,false);buffer.clear();p->processBlock(buffer,midi);waitM3([&]{return engine.snapshot().liveReady;});
    const auto count=engine.snapshot().renderCount;
    armCaptureFixture(*p);buffer.setSize(2,250);position(500,true);
    for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),250,.125f);p->processBlock(buffer,midi);finishCaptureFixture(*p,head.position);
    waitM3([&]{return engine.snapshot().liveReady && engine.savedSession().swells[1].phase==m3::Phase::ready;});
    REQUIRE(engine.snapshot().renderCount==count);
    armCaptureFixture(*p);position(500,true);for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),250,.25f);
    p->processBlock(buffer,midi);finishCaptureFixture(*p,head.position);
    waitM3([&]{const auto s=engine.snapshot();return s.liveReady && s.renderCount>count && s.session.swells[1].phase==m3::Phase::ready;});
    buffer.setSize(2,16);position(490,true);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(std::abs(buffer.getSample(0,10)-41*.00025f)<1e-6f);
    const auto identicalCount=engine.snapshot().renderCount;
    REQUIRE(engine.selectTargets({second},second));engine.loadIR(file,true);position(490,true);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(std::abs(buffer.getSample(0,15)-46*.00025f)<1e-6f); // Retain the accepted contribution.
    waitM3([&]{return engine.snapshot().liveReady && engine.savedSession().swells[1].phase==m3::Phase::ready;});
    REQUIRE(engine.snapshot().renderCount==identicalCount);
    // Coalesce rapid edits; cancelled generations cannot return an older placement.
    for(int64_t offset=1;offset<=40;++offset)REQUIRE(engine.edit(second,[&](auto& w,auto& error){return m3::moveWet(w,offset,error);}));
    waitM3([&]{return engine.snapshot().liveReady;});position(490,true);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(std::abs(buffer.getSample(0,10)-41*.00025f)<1e-6f);
    // A cold destination reports a miss then hydrates around current time.
    position(10000,true);buffer.clear();p->processBlock(buffer,midi);REQUIRE(engine.snapshot().liveMissing);
    waitM3([&]{buffer.clear();p->processBlock(buffer,midi);return !engine.snapshot().liveMissing;});
    head.position.setLoopPoints(juce::AudioPlayHead::LoopPoints{1.96,2.4});head.position.setIsLooping(true);buffer.clear();p->processBlock(buffer,midi);
    waitM3([&]{return engine.snapshot().liveReady && engine.snapshot().liveLoopDestination==490;});
    position(490,true);buffer.clear();p->processBlock(buffer,midi);REQUIRE(!engine.snapshot().liveMissing && std::abs(buffer.getSample(0,10)-41*.00025f)<1e-6f);
    head.position.setIsLooping(false);
    // Returning from incompatible timing cannot revive a formerly published voice.
    head.position.setBpm(241);buffer.clear();p->processBlock(buffer,midi);REQUIRE(engine.snapshot().liveIncompatible);
    waitM3([&]{return engine.savedSession().swells[1].phase==m3::Phase::stale;});
    head.position.setBpm(240);position(490,true);buffer.clear();p->processBlock(buffer,midi);REQUIRE(buffer.getMagnitude(0,16)==0);
    // Clearing/restoring cannot expose an old publication before validation.
    engine.generate(true);position(490,false);buffer.clear();p->processBlock(buffer,midi);waitM3([&]{return engine.snapshot().liveReady && engine.savedSession().swells[1].phase==m3::Phase::ready;});
    REQUIRE(engine.selectTargets({second},second));auto saved=engine.savedSession();REQUIRE(p->requestWorkflowMode(workflow::Mode::capture));p->requestClearCapture();REQUIRE(p->flushCaptureForTesting());
    engine.restore(saved);position(490,true);buffer.clear();p->processBlock(buffer,midi);REQUIRE(buffer.getMagnitude(0,16)==0);
    waitM3([&]{return engine.snapshot().cachedWetOnly;});buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(buffer.getMagnitude(0,16)==0); // Labeled manual recovery never enters live playback.
}
