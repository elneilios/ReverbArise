#pragma once
#include <Core/ContentHash.h>
#include <Capture/SourceVisitor.h>

// Real managed render/source identities with synthetic PCM, not an owner FLP.
static void runLiveClockEngineCases()
{
    using namespace m3;
    // Exercise the reported counts with current assets, plus frozen assets.
    for(const auto scenario:{0,1,2}){
        const auto members=scenario==0?7:61;const auto frozen=scenario==2;
        TestDirectory directory;CaptureSession capture(directory.file);juce::String error;
        std::vector<float> dry(300000*2,.125f);
        REQUIRE(capture.commit(0,48000,dry.data(),150000));
        REQUIRE(capture.commit(150000,48000,dry.data()+300000,150000));
        juce::CriticalSection mutex;
        PreviewContext context{capture.snapshot(),{true,12000,12000.*122/(48000*60),48000,122,4,4},0};
        workflow::State flow;flow.reset(workflow::Mode::live);
        auto engine=std::make_unique<SwellEngine>(directory.file,[&]{const juce::ScopedLock guard(mutex);return context;});
        engine->useWorkflow(flow);
        engine->setAutomaticProcessing(false);waitFor([&]{return engine->savedSession().defaultIR.present();});
        SwellAssetStore assets(engine->assetDirectory());
        ImpulseResponse ir;ir.rate=48000;ir.channels=1;ir.samples.resize(2001);ir.samples[0]=.008f;
        Asset response,render;REQUIRE(assets.writeIR(ir,response,error));
        WetRender raw;raw.start=10000;raw.sourceStart=12000;raw.sourceEnd=220000;raw.rate=48000;
        raw.samples.resize(static_cast<size_t>(raw.sourceEnd-raw.start)*2,.001f);raw.peak=.001f;
        CaptureAssetStore sourceStore(engine->assetDirectory());juce::String fingerprint;
        REQUIRE(sourceFingerprint(sourceStore,context.capture.manifest,12000,220000,fingerprint,error));
        REQUIRE(assets.writeRender(raw,response,fingerprint,context.time,render,error));
        auto saved=engine->savedSession();saved.timing=context.time;saved.swells.clear();saved.selected.clear();
        saved.gainPolicy=GainPolicy::legacy;saved.gainRevision=legacyGainRevision;saved.automaticProcessing=false;saved.autoRegenerate=false;
        for(int n=0;n<members;++n){Swell w;w.name="Live clock fixture "+juce::String(n);w.arrival=12000;w.sourceStart=12000;w.sourceEnd=220000;
            w.ir=response;w.irName="Fixture";w.render=render;w.frozen=frozen;w.shape.automaticStart=false;
            w.shape.startOffset=-2000;w.shape.endOffset=208000;w.phase=Phase::ready;saved.swells.push_back(w);}
        saved.primary=saved.swells.front().id;saved.selected={saved.primary};engine->restore(saved);
        const auto allReady=[&]{const auto s=engine->savedSession();for(const auto& w:s.swells)
            if(w.phase==Phase::error || w.phase==Phase::stale)throw std::runtime_error("Live clock fixture restore: "+w.status.toStdString());
            return std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==Phase::ready;});};
        waitFor(allReady);
        float left[512],right[512];
        const auto block=[&](int64_t sample,bool playing,int offset=0,bool valid=true,bool looping=false){
            TransportTime time;
            {const juce::ScopedLock guard(mutex);context.time={valid,sample,(sample-(playing?offset:0))*122./(48000*60),48000,122,4,4};
                time={context.time,playing,looping,12000,16000};}
            std::fill_n(left,512,.125f);std::fill_n(right,512,.125f);
            {AudioAllocationGuard guard;engine->processTransportAudio(left,right,512,time);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        };
        const auto readyAt=[&](int64_t p){const auto s=engine->snapshot();return s.liveReady && s.livePreparation.start<=p
            && s.livePreparation.end>p+512 && (s.livePreparation.result=="Published complete window" || s.livePreparation.result=="Prepared window retained");};
        const auto wet=[&]{REQUIRE(engine->snapshot().liveActive && !engine->snapshot().liveIncompatible);
            REQUIRE(std::abs(left[511]-(.125f+members*.001f))<1e-6f);};
        block(12000,false);waitFor([&]{return readyAt(12000);});
        juce::String before,after;REQUIRE(encode(engine->savedSession(),before,error));const auto captureBefore=capture.snapshot().reference;
        block(12000,true,0);wet();block(12512,true,0);wet();block(12000,false);
        block(12000,true,16);wet();REQUIRE(left[0]>.125f);
        REQUIRE(std::abs(engine->snapshot().liveObservation->originDelta-16)<.00001);
        block(12512,true,16);wet();
        // A later cold position must prepare despite the source/playing delta.
        block(180000,true,16);REQUIRE(engine->snapshot().liveMissing);
        waitFor([&]{return readyAt(180000);});block(180512,true,16);wet();
        // Prepare a distant loop destination, then join its existing absolute PCM.
        block(181024,true,16,true,true);waitFor([&]{return engine->snapshot().liveLoopDestination==12000;});
        block(12512,true,16,true,true);wet();REQUIRE(!engine->snapshot().liveMissing);
        // Invalid stopped-looking data is not a confirmed Stop/rebase.
        block(13024,true,18);REQUIRE(engine->snapshot().liveIncompatible && left[511]==.125f);
        block(13024,false,0,false);block(13024,true,18);REQUIRE(engine->snapshot().liveIncompatible);
        engine->setLivePlayback(false,LiveRouting::dryWet,0);block(13024,true,18);
        engine->setLivePlayback(true,LiveRouting::dryWet,0);block(13024,true,18);REQUIRE(engine->snapshot().liveIncompatible);
        block(13024,true,16);wet(); // Freshly validated return to the retained run.
        block(13024,false);block(13024,true,64);wet(); // No hard-coded 16-sample correction.
        REQUIRE(encode(engine->savedSession(),after,error) && before==after && capture.snapshot().reference==captureBefore);
        flow.reset(workflow::Mode::finalise);block(13024,true,64);REQUIRE(left[511]==.125f);
        flow.reset(workflow::Mode::live);block(13024,true,64);wet();
        flow.reset(workflow::Mode::capture);block(13024,true,64);REQUIRE(left[511]==.125f);
        flow.reset(workflow::Mode::live);block(13024,true,64);wet();
        // Publication crossing Stop/Play must reject the old clock, then retry.
        const auto clockRejects=engine->liveClockRejectionsForTesting();
        engine->holdLivePublicationForTesting(true);block(330000,true,64);waitFor([&]{return engine->livePublicationWaitingForTesting();});
        block(330000,false);block(330000,true,32);engine->holdLivePublicationForTesting(false);
        waitFor([&]{return engine->liveClockRejectionsForTesting()>clockRejects && readyAt(330000);});
        // A transient publish failure must not certify the candidate revision/window.
        const auto failures=engine->livePublicationFailuresForTesting();const auto attempt=engine->snapshot().livePreparation.attempt;
        engine->failNextLivePublicationForTesting();block(440000,true,32);
        waitFor([&]{return engine->livePublicationFailuresForTesting()>failures && readyAt(440000);});
        REQUIRE(engine->snapshot().livePreparation.attempt>=attempt+2);
        block(440512,true,32);REQUIRE(left[511]==.125f); // No elapsed wet replay.
        // An edited render/shape revision supersedes held work; only new gain may publish.
        engine->holdLivePublicationForTesting(true);block(600000,true,32);waitFor([&]{return engine->livePublicationWaitingForTesting();});
        REQUIRE(engine->edit(saved.primary,[](auto& w,auto&){w.shape.gainDb=-3;return true;}));
        engine->holdLivePublicationForTesting(false);waitFor([&]{return readyAt(600000);});
        block(15000,true,32);waitFor([&]{return readyAt(15000);});block(15512,true,32);
        REQUIRE(std::abs(left[511]-(.125f+(members-1)*.001f+.001f*juce::Decibels::decibelsToGain(-3.f)))<1e-6f);
        // Source generation supersedes work even when the content can be reused.
        const auto sourceAttempt=engine->snapshot().livePreparation.attempt;
        engine->holdLivePublicationForTesting(true);block(750000,true,32);waitFor([&]{return engine->livePublicationWaitingForTesting();});
        {const juce::ScopedLock guard(mutex);++context.captureGeneration;}
        engine->holdLivePublicationForTesting(false);waitFor([&]{return readyAt(750000);});
        REQUIRE(engine->snapshot().livePreparation.attempt>=sourceAttempt+2);
        // Restore supersedes both session ownership and a paused preparation.
        engine->holdLivePublicationForTesting(true);block(900000,true,32);waitFor([&]{return engine->livePublicationWaitingForTesting();});
        engine->restore(saved);block(12000,false);engine->holdLivePublicationForTesting(false);
        waitFor(allReady);waitFor([&]{return readyAt(12000);});block(12000,true,16);wet();
        REQUIRE(engine->savedSession().swells.front().id==saved.swells.front().id);
        REQUIRE(engine->savedSession().swells.front().render==render && capture.snapshot().reference==captureBefore && assets.verify(render,error));
        // Another instance uses its own run mapping and cannot repair/rebase this one.
        juce::CriticalSection otherMutex;PreviewContext otherContext;
        {const juce::ScopedLock guard(mutex);otherContext=context;}
        otherContext.time={true,12000,12000.*122/(48000*60),48000,122,4,4};
        auto second=std::make_unique<SwellEngine>(directory.file,[&]{const juce::ScopedLock guard(otherMutex);return otherContext;});
        second->setAutomaticProcessing(false);second->restore(saved);waitFor([&]{const auto s=second->savedSession();return std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==Phase::ready;});});
        TransportTime other{otherContext.time,false};second->processTransportAudio(left,right,512,other);
        waitFor([&]{return second->snapshot().liveReady;});
        {const juce::ScopedLock guard(otherMutex);otherContext.time.ppq=(12000-64)*122./(48000*60);other={otherContext.time,true};}
        std::fill_n(left,512,.125f);std::fill_n(right,512,.125f);
        {AudioAllocationGuard guard;second->processTransportAudio(left,right,512,other);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && second->snapshot().liveActive);
        REQUIRE(std::abs(left[511]-(.125f+members*.001f))<1e-6f);
        block(12512,true,16);wet();
        REQUIRE(engine->snapshot().liveObservation->liveReference.ppq!=second->snapshot().liveObservation->liveReference.ppq);
        // A late window must be rejected before it can consume a publication
        // fault or certify readiness. Unchanged run, new absolute destination.
        const auto lateAttempt=engine->snapshot().livePreparation.attempt;
        const auto lateFailures=engine->livePublicationFailuresForTesting();
        engine->holdLivePublicationForTesting(true);block(1000000,true,16);
        waitFor([&]{return engine->livePublicationWaitingForTesting();});
        block(1200000,true,16);engine->failNextLivePublicationForTesting();engine->holdLivePublicationForTesting(false);
        waitFor([&]{return engine->livePublicationFailuresForTesting()>lateFailures && readyAt(1200000);});
        REQUIRE(engine->snapshot().livePreparation.attempt>=lateAttempt+3);

    }
}
