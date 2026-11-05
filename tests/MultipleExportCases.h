#pragma once
#include <future>
inline void multipleExportCases()
{
    TestDirectory root;const auto irFile=root.file.getChildFile("two-track-ir.wav");writeTestWav(irFile,1,1000,std::vector<float>(101,.001f));
    std::array<std::unique_ptr<PreverbAudioProcessor>,2> processors;
    std::array<TestPlayHead,2> heads;juce::MidiBuffer midi;std::array<juce::String,2> finalIds;juce::String waitingId;
    const auto position=[&](int i,int64_t at,bool playing){auto& h=heads[i].position;h.setIsPlaying(playing);h.setTimeInSamples(at);h.setPpqPosition(at/500.0);};
    for(int i=0;i<2;++i){auto& p=processors[i];p=std::make_unique<PreverbAudioProcessor>(root.file);p->prepareToPlay(1000,1024);p->setPlayHead(&heads[i]);
        auto& e=p->swellEngine();e.setAutomaticProcessing(false);e.setAutoMarkers(false);
        REQUIRE(e.setGainPolicy(m3::GainPolicy::legacy)); // Musical adoption is exercised separately below.
        heads[i].position.setBpm(120);heads[i].position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
        juce::AudioBuffer<float> b(2,1000);for(int at=0;at<4001;at+=1000){const auto n=std::min(1000,4001-at);b.setSize(2,n);position(i,at,true);
            for(int c=0;c<2;++c)std::fill_n(b.getWritePointer(c),n,i?.25f:.125f);p->processBlock(b,midi);REQUIRE(p->flushCaptureForTesting());}
        finishCaptureFixture(*p,heads[i].position);position(i,0,false);b.clear();p->processBlock(b,midi);
        REQUIRE(e.addTarget(500));const auto first=e.savedSession().primary;REQUIRE(e.addTarget(3900));finalIds[i]=e.savedSession().primary;
        REQUIRE(e.selectTargets({first,finalIds[i]},first));e.loadIR(irFile);
        waitM3([&]{return e.savedSession().swells.back().irName=="two-track-ir.wav";});e.generate(true);
        waitM3([&]{const auto s=e.savedSession();return std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
        e.setLivePlayback(true,m3::LiveRouting::wetOnly,0);
        if(i){REQUIRE(e.edit(finalIds[i],[](auto& w,auto&){w.provenance=m3::Provenance::automatic;return true;}));e.setAutomaticProcessing(true);
            waitM3([&]{return m3::find(e.savedSession(),finalIds[i])->phase==m3::Phase::ready;});
            // Stop now bounds the real final event. Use a genuinely unmapped,
            // protected source range to exercise available-member warnings.
            auto session=e.savedSession();auto waiting=session.swells.front();waiting.id=juce::Uuid().toDashedString();waitingId=waiting.id;
            waiting.arrival=waiting.sourceStart=6000;waiting.sourceEnd=6500;waiting.render={};waiting.provenance=m3::Provenance::manualEdited;waiting.phase=m3::Phase::stale;
            session.swells.push_back(waiting);e.restore(session);
            waitM3([&]{return m3::find(e.savedSession(),waitingId)->status=="Waiting for source" && m3::find(e.savedSession(),finalIds[i])->phase==m3::Phase::ready;});}}
    REQUIRE(processors[0]->getCaptureSnapshot().manifest.instanceId!=processors[1]->getCaptureSnapshot().manifest.instanceId);
    REQUIRE(processors[0]->swellEngine().snapshot().offlineBlocks==0 && processors[1]->swellEngine().snapshot().offlineBlocks==0);
    // A waiting marker warns, while already audible contributions still export.
    for(int i=0;i<2;++i){if(i)processors[i]->swellEngine().setLivePlayback(true,m3::LiveRouting::dryWet,0);
        processors[i]->setNonRealtime(true);position(i,450,false);juce::AudioBuffer<float> b(2,16);b.clear();if(i)for(int c=0;c<2;++c)std::fill_n(b.getWritePointer(c),16,.4f);processors[i]->processBlock(b,midi);
        if(i){REQUIRE(processors[i]->swellEngine().snapshot().liveStatus.startsWith("Export warning"));REQUIRE(std::abs(b.getSample(0,10)-(.4f+61*.00025f))<1e-6);processors[i]->swellEngine().setLivePlayback(true,m3::LiveRouting::wetOnly,0);}
        REQUIRE(b.getMagnitude(0,16)>0);processors[i]->setNonRealtime(false);}
    REQUIRE(processors[1]->swellEngine().selectTargets({waitingId},waitingId));processors[1]->swellEngine().removeSelected();
    // An explicit [0,4000) loop has real captured data through 4001 due to rounding.
    heads[1].position.setIsLooping(true);heads[1].position.setLoopPoints(juce::AudioPlayHead::LoopPoints{0,8});position(1,0,false);
    juce::AudioBuffer<float> tick(2,64);tick.clear();processors[1]->processBlock(tick,midi);
    // This fixture learned with Automatic Swells off. A new extraction window
    // needs explicit Perform generation rather than implicit Off-pass work.
    waitM3([&]{const auto saved=processors[1]->swellEngine().savedSession();const auto w=m3::find(saved,finalIds[1]);return w && w->phase==m3::Phase::stale && w->sourceEnd==4000;});
    processors[1]->swellEngine().generate(true);
    try {waitM3([&]{const auto saved=processors[1]->swellEngine().savedSession();const auto w=m3::find(saved,finalIds[1]);return w && w->phase==m3::Phase::ready && w->sourceEnd==4000;});}
    catch(...){const auto state=processors[1]->swellEngine().snapshot();const auto w=m3::find(state.session,finalIds[1]);const auto timing=processors[1]->getHostPositionDiagnostics();
        std::cerr<<"Final loop fit: end="<<w->sourceEnd<<" provenance="<<static_cast<int>(w->provenance)<<" status="<<w->status<<" automatic="<<state.session.automaticProcessing<<" loop="<<timing.loopStart<<":"<<timing.loopEnd<<" | "<<state.message<<"\n";throw;}
    REQUIRE(m3::find(processors[1]->swellEngine().savedSession(),finalIds[1])->sourceEnd==4000);
    // Saved final-tail fitting survives reopen with loop flags no longer supplied.
    auto reopened=processors[1]->swellEngine().savedSession();heads[1].position.setIsLooping(false);
    processors[1]->swellEngine().restore(reopened);position(1,0,false);tick.clear();processors[1]->processBlock(tick,midi);
    waitM3([&]{return m3::find(processors[1]->swellEngine().savedSession(),finalIds[1])->phase==m3::Phase::ready;});
    REQUIRE(m3::find(processors[1]->swellEngine().savedSession(),finalIds[1])->sourceEnd==4000);
    const auto captured0=processors[0]->getCaptureSnapshot().reference,captured1=processors[1]->getCaptureSnapshot().reference;
    const auto bounce=[&](int i,int stride,double origin=0,bool playing=false){auto& p=*processors[i];p.prepareToPlay(1000,stride);p.setNonRealtime(true);juce::MidiBuffer messages;std::vector<float> output(4000);juce::AudioBuffer<float> b(2,stride);
        for(int at=0;at<4000;at+=stride){const auto n=std::min(stride,4000-at);position(i,at,playing);heads[i].position.setPpqPosition((at-origin)/500.);b.setSize(2,n);b.clear();p.processBlock(b,messages);
            REQUIRE(!p.swellEngine().snapshot().liveStatus.startsWith("Export incomplete"));std::copy_n(b.getReadPointer(0),n,output.data()+at);}return output;};
    const auto serial0=bounce(0,17),serial1=bounce(1,1023);
    auto future0=std::async(std::launch::async,[&]{return bounce(0,8192);});auto future1=std::async(std::launch::async,[&]{return bounce(1,31);});
    const auto parallel0=future0.get(),parallel1=future1.get();
    REQUIRE(processors[0]->swellEngine().snapshot().offlineBlocks>1 && processors[1]->swellEngine().snapshot().offlineBlocks>1);REQUIRE(serial0==parallel0 && serial1==parallel1);
    for(int i=0;i<2;++i){const auto& samples=i?serial1:serial0;const auto session=processors[i]->swellEngine().savedSession();
        for(int at=0;at<4000;++at){double expected=0;for(const auto& w:session.swells){const auto a=w.arrival-100,b=w.arrival+20;if(at<a || at>=b)continue;
            const auto age=at-a,source=w.render.sourceEnd-w.render.sourceStart;
            const auto terms=std::min({age+1,int64_t{101},source,source+100-age});
            const auto u=std::min(1.0,(b-1-at)/20.0),fade=u*u*(3-2*u);
            expected+=terms*.001*(i?.25:.125)*fade*std::min({1.0,age/2.0,(b-1-at)/2.0})*std::min(1.0,(age+1)/5.0);}
            REQUIRE(std::abs(samples[at]-expected)<1e-6);}}
    REQUIRE(serial0[440]>0 && serial1[440]>serial0[440] && serial1[3850]>0);
    // #73: same accepted sample coordinates, real-time zero-input wet versus
    // offline output. The first five samples are the independently defined join
    // ramp; subsequent changing samples must agree, not just an energy flag.
    for(int i=0;i<2;++i){auto& p=*processors[i];p.setNonRealtime(false);p.prepareToPlay(1000,64);
        const auto savedBeforeLive=p.swellEngine().savedSession();
        position(i,440,false);tick.clear();p.processBlock(tick,midi);
        waitM3([&]{return p.swellEngine().snapshot().liveReady;});
        position(i,440,true);
        waitM3([&]{tick.clear();p.processBlock(tick,midi);const auto s=p.swellEngine().snapshot();return s.liveActive && !s.liveMissing && !s.liveIncompatible;});
        // Confirmed Stop removes previous voices; wait through its bounded ramp.
        position(i,440,false);tick.clear();p.processBlock(tick,midi);
        position(i,440,true);std::vector<float> live;
        for(int stride:{7,19,38}){juce::AudioBuffer<float> block(2,stride);block.clear();p.processBlock(block,midi);
            for(int n=0;n<stride;++n)live.push_back(block.getSample(0,n));position(i,440+static_cast<int64_t>(live.size()),true);}
        const auto& offline=i?serial1:serial0;REQUIRE(live.size()==64 && live[10]>0);
        for(size_t n=0;n<live.size();++n){const auto ramp=std::min(1.0f,static_cast<float>(n+1)/5);
            REQUIRE(std::abs(live[n]-offline[440+n]*ramp)<1e-6f);}
        const auto savedAfterLive=p.swellEngine().savedSession();REQUIRE(savedBeforeLive.swells.size()==savedAfterLive.swells.size());
        for(size_t n=0;n<savedBeforeLive.swells.size();++n)REQUIRE(m3::sameSavedSwell(savedBeforeLive.swells[n],savedAfterLive.swells[n]));
    }

    REQUIRE(processors[0]->getLatencySamples()==0 && processors[1]->getLatencySamples()==0);
    const auto accepted0=processors[0]->swellEngine().savedSession(),accepted1=processors[1]->swellEngine().savedSession();
    REQUIRE(bounce(0,37,16)==serial0 && bounce(1,509,-31)==serial1);
    REQUIRE(bounce(0,513,16,true)==serial0);
    auto offset0=std::async(std::launch::async,[&]{return bounce(0,8192,16);});
    auto offset1=std::async(std::launch::async,[&]{return bounce(1,31,-31);});
    REQUIRE(offset0.get()==serial0 && offset1.get()==serial1);
    for(int i=0;i<2;++i){const auto after=processors[i]->swellEngine().savedSession();const auto& before=i?accepted1:accepted0;
        for(size_t n=0;n<before.swells.size();++n)REQUIRE(m3::sameSavedSwell(before.swells[n],after.swells[n]));}
    REQUIRE(processors[0]->getCaptureSnapshot().reference==captured0 && processors[1]->getCaptureSnapshot().reference==captured1);
    // Musical exports snapshot each instance's calibrated accepted schedule;
    // irregular and independently interleaved callbacks remain deterministic.
    for(int i=0;i<2;++i){processors[i]->setNonRealtime(false);position(i,0,false);tick.clear();processors[i]->processBlock(tick,midi);}
    for(auto& processor:processors){REQUIRE(processor->swellEngine().setGainPolicy(m3::GainPolicy::musical));
        waitM3([&]{const auto session=processor->swellEngine().savedSession();return std::all_of(session.swells.begin(),session.swells.end(),[](const auto& w){return w.phase!=m3::Phase::checking && w.phase!=m3::Phase::rendering;});});}
    const auto musical0=bounce(0,37),musical1=bounce(1,509);
    auto musicalFuture0=std::async(std::launch::async,[&]{return bounce(0,2048);});
    auto musicalFuture1=std::async(std::launch::async,[&]{return bounce(1,19);});
    REQUIRE(musical0==musicalFuture0.get() && musical1==musicalFuture1.get());
    for(const auto& processor:processors){const auto metrics=processor->swellEngine().snapshot();
        REQUIRE(metrics.gainPlanBuilds>0 && metrics.gainPlanPreparationMs>=0 && metrics.gainPlanPeak>=0);
        std::cout<<"Musical gain plan: builds="<<metrics.gainPlanBuilds<<" cacheHits="<<metrics.gainPlanCacheHits
            <<" lastMs="<<metrics.gainPlanPreparationMs<<" peak="<<metrics.gainPlanPeak<<"\n";}
    for(auto& processor:processors)REQUIRE(processor->swellEngine().setGainPolicy(m3::GainPolicy::legacy));
    // Hosts can reset/freeze PPQ while processing an offline tail after every
    // prepared wet interval has already ended. That cannot invalidate prior wet.
    for(const auto routing:{m3::LiveRouting::dryWet,m3::LiveRouting::wetOnly}){
        processors[0]->swellEngine().setLivePlayback(true,routing,0);bounce(0,64);
        position(0,4000,false);heads[0].position.setPpqPosition(0);juce::AudioBuffer<float> tail(2,64);
        for(int c=0;c<2;++c)std::fill_n(tail.getWritePointer(c),64,.4f);processors[0]->processBlock(tail,midi);
        REQUIRE(!processors[0]->swellEngine().snapshot().liveStatus.startsWith("Export incomplete"));
        for(int n=0;n<64;++n)REQUIRE(tail.getSample(0,n)==(routing==m3::LiveRouting::wetOnly?0.0f:.4f));
    }
    // A mapping change while future/current wet remains required is still fatal.
    processors[0]->prepareToPlay(1000,64);position(0,440,false);heads[0].position.setPpqPosition((440.-16)/500.);
    tick.clear();processors[0]->processBlock(tick,midi);REQUIRE(tick.getMagnitude(0,64)>0);
    position(0,504,false);heads[0].position.setPpqPosition((504.-19)/500.);tick.clear();processors[0]->processBlock(tick,midi);
    REQUIRE(processors[0]->swellEngine().snapshot().offlineStatus.contains("run origin delta=3.000000000 samples"));
    processors[0]->prepareToPlay(1000,64);position(0,440,false);tick.clear();processors[0]->processBlock(tick,midi);
    position(0,504,false);heads[0].position.setPpqPosition(0);tick.clear();processors[0]->processBlock(tick,midi);
    REQUIRE(processors[0]->swellEngine().snapshot().liveStatus.startsWith("Export incomplete"));
    const auto realError=processors[0]->swellEngine().snapshot().liveStatus;
    position(0,4000,false);heads[0].position.setPpqPosition(0);tick.clear();processors[0]->processBlock(tick,midi);
    REQUIRE(processors[0]->swellEngine().snapshot().liveStatus==realError); // A later tail never erases an earlier failure.
    processors[0]->prepareToPlay(1000,64);position(0,440,false);tick.clear();processors[0]->processBlock(tick,midi);
    position(0,4000,false);heads[0].position.setBpm(121);tick.clear();processors[0]->processBlock(tick,midi);
    REQUIRE(processors[0]->swellEngine().snapshot().liveStatus.startsWith("Export incomplete"));heads[0].position.setBpm(120);
    // Failure stays local to its instance, including later bounce attempts.
    auto& failed=processors[1]->swellEngine();REQUIRE(failed.setFrozen(finalIds[1],true));const auto asset=m3::find(failed.savedSession(),finalIds[1])->render;REQUIRE(failed.assetDirectory().getChildFile(asset.path).deleteFile());
    processors[1]->prepareToPlay(1000,64);position(1,440,false);tick.clear();processors[1]->processBlock(tick,midi);
    REQUIRE(failed.snapshot().liveStatus.startsWith("Export warning"));REQUIRE(tick.getMagnitude(5,50)>0);
    position(1,3850,false);tick.clear();processors[1]->processBlock(tick,midi);REQUIRE(tick.getMagnitude(5,50)==0);
    position(1,4000,false);heads[1].position.setPpqPosition(0);tick.clear();processors[1]->processBlock(tick,midi);
    REQUIRE(failed.snapshot().liveStatus.startsWith("Export warning")); // Tail processing must preserve skip warnings too.
    REQUIRE(bounce(0,777)==serial0);
}
