#pragma once
#include <Capture/SourceVisitor.h>

// Synthetic processor lifecycle evidence, not a trace of FL Studio callbacks.
inline void processorReactivationCases()
{
    using namespace m3;
    TestDirectory root;
    auto processor=std::make_unique<PreverbAudioProcessor>(root.file);
    CaptureFailureHost host;
    processor->setPlayHead(&host);processor->prepareToPlay(1000,64);
    auto& engine=processor->swellEngine();engine.setLearningAutomatic(false);engine.setAutomaticProcessing(false);
    const auto wait=[&](auto predicate){
        const auto deadline=juce::Time::getMillisecondCounter()+10000;
        while(!predicate() && juce::Time::getMillisecondCounter()<deadline)juce::Thread::sleep(2);
        if(!predicate()){
            const auto state=engine.snapshot();std::cerr<<"#103 fixture wait: "<<state.livePreparation.result<<"\n";
            for(const auto& w:engine.savedSession().swells)std::cerr<<"swell "<<int(w.phase)<<" "<<w.status<<"\n";
            REQUIRE(predicate());
        }
    };
    wait([&]{return engine.savedSession().defaultIR.present();});
    juce::MidiBuffer midi;juce::AudioBuffer<float> buffer(2,64);
    const auto process=[&](int64_t at,bool playing,double origin=0,bool bypass=false,int count=64){
        buffer.setSize(2,count);
        host.at(at,playing);host.position.setPpqPosition((at-origin)/500.0);
        for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),count,.125f);
        {AudioAllocationGuard guard;
            if(bypass)processor->processBlockBypassed(buffer,midi);else processor->processBlock(buffer,midi);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    };
    REQUIRE(setFixtureCaptureArmed(*processor));
    for(int64_t at=0;at<8192;at+=64){process(at,true);REQUIRE(processor->flushCaptureForTesting());}
    process(8192,false);REQUIRE(processor->flushCaptureForTesting());
    REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));
    const auto capture=processor->getCaptureSnapshot();
    SwellAssetStore assets(engine.assetDirectory());CaptureAssetStore sources(engine.assetDirectory());
    ImpulseResponse ir;ir.rate=1000;ir.channels=1;ir.samples.resize(101);ir.samples[0]=.01f;
    Asset response;juce::String error;REQUIRE(assets.writeIR(ir,response,error));
    auto saved=engine.savedSession();saved.swells.clear();saved.selected.clear();saved.timing=processor->getSongTime();
    saved.gainPolicy=GainPolicy::legacy;saved.gainRevision=legacyGainRevision;
    saved.automaticProcessing=false;saved.autoRegenerate=false;
    const auto sample=[](int64_t index,int channel){return channel==0?.01f+float(index%113)*.0001f:-.02f-float(index%79)*.0001f;};
    for(const int64_t arrival:{1000,5000}){
        WetRender raw;raw.rate=1000;raw.start=arrival-100;raw.sourceStart=arrival;raw.sourceEnd=arrival+2000;raw.peak=.04f;
        raw.samples.resize(4200);for(int64_t i=0;i<2100;++i)for(int c=0;c<2;++c)raw.samples[size_t(i*2+c)]=sample(i,c);
        juce::String fingerprint;REQUIRE(sourceFingerprint(sources,capture.manifest,arrival,arrival+2000,fingerprint,error));
        Asset render;REQUIRE(assets.writeRender(raw,response,fingerprint,saved.timing,render,error));
        Swell swell;swell.arrival=swell.sourceStart=arrival;swell.sourceEnd=arrival+2000;swell.ir=response;swell.render=render;
        swell.shape.automaticStart=false;swell.shape.startOffset=-100;swell.shape.endOffset=2000;swell.phase=Phase::ready;
        saved.swells.push_back(swell);
    }
    saved.primary=saved.swells.front().id;saved.selected={saved.primary};engine.restore(saved);
    wait([&]{const auto state=engine.savedSession();return state.swells.size()==2 && std::all_of(state.swells.begin(),state.swells.end(),[](const auto& w){return w.phase==Phase::ready;});});
    const auto ready=[&](int64_t at){const auto state=engine.snapshot();return state.liveReady && state.livePreparation.start<=at && state.livePreparation.end>=at+64;};
    const auto wet=[&](int64_t at){
        const auto state=engine.snapshot();
        if(!state.liveActive || state.liveIncompatible)std::cerr<<"reactivation gate="<<int(state.liveObservation->gate)<<" at="<<at<<"\n";
        REQUIRE(state.liveActive && !state.liveIncompatible);
        const int64_t start=at<4000?900:4900;
        for(int c=0;c<2;++c)for(int n=5;n<buffer.getNumSamples();++n)REQUIRE(std::abs(buffer.getSample(c,n)-(.125f+sample(at+n-start,c)))<1e-6f);
    };
    process(1200,false);wait([&]{return ready(1200);});
    juce::String before,after;REQUIRE(encode(engine.savedSession(),before,error));
    for(int path=0;path<4;++path){
        process(1200,false);wait([&]{return ready(1200);});process(1200,true);wet(1200);
        // No processing callbacks from 1264 to 1600 while the host advances.
        if(path==1){processor->releaseResources();processor->prepareToPlay(1000,64);}
        if(path==2){process(1400,true,0,true);REQUIRE(buffer.getSample(0,63)==.125f);}
        if(path==3){AudioAllocationGuard guard;processor->reset();}
        process(1600,true);wet(1600);
        process(5200,true);if(!ready(5200))wait([&]{return ready(5200);});process(5264,true);wet(5264);
        std::cout<<"#103 stable-origin lifecycle path="<<path<<" interior/later wet passed\n";
    }
    REQUIRE(encode(engine.savedSession(),after,error) && before==after);
    REQUIRE(processor->getCaptureSnapshot().reference==capture.reference);
    // VST3 setProcessing(false) calls reset(), without release/prepare. A fresh
    // host processing epoch may supply a new valid sample-to-PPQ mapping.
    process(1200,false);wait([&]{return ready(1200);});process(1200,true);wet(1200);
    {AudioAllocationGuard guard;processor->reset();}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    process(1600,true,16);wet(1600);
    process(5200,true,16);wait([&]{return ready(5200);});process(5264,true,16);wet(5264);
    std::cout<<"#103 reset/new-origin interior/later wet passed\n";
    // Repeated real reset requests establish distinct ephemeral authority.
    for(int cycle=0;cycle<3;++cycle){
        const auto previous=engine.transportClockSnapshot();REQUIRE(previous);
        {AudioAllocationGuard guard;processor->reset();processor->reset();}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && !engine.transportClockSnapshot());
        const auto origin=32.+cycle*16;
        process(1700,true,origin);wait([&]{return ready(1700);});process(1764,true,origin,false,37);wet(1764);
        const auto current=engine.transportClockSnapshot();REQUIRE(current && !previous->sameRun(*current));
        process(1900,true,origin,false,127);wet(1900); // Ordinary forward seek.
        host.position.setIsLooping(true);process(1800,true,origin,false,23);wet(1800); // Backward seek/loop, same mapping.
        // A gap/seek plus changed origin still cannot rebase an established run.
        process(2100,true,origin+2);REQUIRE(engine.snapshot().liveIncompatible);
        engine.setLivePlayback(false,LiveRouting::dryWet,0);process(2164,true,origin+2);
        engine.setLivePlayback(true,LiveRouting::dryWet,0);process(2228,true,origin+2);
        REQUIRE(engine.snapshot().liveIncompatible);
        process(2292,true,origin);wet(2292);
    }
    host.position.setIsLooping(false);
    // Pending work must not publish under authority superseded by reset.
    const auto rejects=engine.liveClockRejectionsForTesting();
    engine.holdLivePublicationForTesting(true);process(7600,true,64);
    wait([&]{return engine.livePublicationWaitingForTesting();});
    processor->reset();process(1600,true,80);engine.holdLivePublicationForTesting(false);
    wait([&]{return engine.liveClockRejectionsForTesting()>rejects && ready(1600);});
    process(1664,true,80,false,97);wet(1664);
    // Session restore also revokes a held publication, preserving accepted assets.
    engine.holdLivePublicationForTesting(true);process(7600,true,80);
    wait([&]{return engine.livePublicationWaitingForTesting();});engine.restore(saved);
    process(1200,false);engine.holdLivePublicationForTesting(false);
    wait([&]{const auto state=engine.savedSession();return std::all_of(state.swells.begin(),state.swells.end(),[](const auto& w){return w.phase==Phase::ready;}) && ready(1200);});
    process(1200,true,96);wet(1200);
    // A reset while timing is missing cannot admit stale wet or a false Stop.
    processor->reset();host.available=false;buffer.clear();
    {AudioAllocationGuard guard;processor->processBlock(buffer,midi);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && !engine.snapshot().liveActive);
    process(1400,true,112);wet(1400);
    REQUIRE(encode(engine.savedSession(),after,error) && before==after);
    REQUIRE(processor->getCaptureSnapshot().reference==capture.reference);
    for(const auto& swell:saved.swells)REQUIRE(assets.verify(swell.render,error));
    processor->setPlayHead(nullptr);
}
