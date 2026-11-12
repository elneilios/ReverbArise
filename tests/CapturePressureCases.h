#pragma once
#include <Capture/CaptureWorker.h>
#include <chrono>
#include <thread>

// Uses the production queue, worker and asset store. Accelerated song duration
// is explicitly separate from wall-clock pacing; no substitute capture engine.
inline void capturePressureCase(int seconds,int rate,int frames,bool realtime,bool fault,bool patterns=false,bool probe=false)
{
    TestDirectory root;auto queue=std::make_unique<PluginCaptureQueue>();workflow::State flow;
    CaptureWorker worker(*queue,root.file);worker.useWorkflow(flow);worker.setHostRate(rate);worker.startThread();
    std::vector<float> left(static_cast<size_t>(frames),.125f),right(static_cast<size_t>(frames),-.25f);
    const auto pass=flow.begin(true);uint64_t queued=0,dropped=0;bool injected=false;uint32_t high=0;
    const auto began=std::chrono::steady_clock::now();
    for(int64_t at=0;at<int64_t(seconds)*rate;at+=frames)
    {
        const auto count=static_cast<int>(std::min<int64_t>(frames,int64_t(seconds)*rate-at));
        const auto songAt=at+(patterns && at>=int64_t(30)*rate?int64_t(2)*rate:0)
            -(patterns && at>=int64_t(90)*rate && at<int64_t(100)*rate?int64_t(10)*rate:0);
        const auto silent=patterns && (at/rate)%45==17;
        std::fill(left.begin(),left.end(),silent?0.f:.125f);std::fill(right.begin(),right.end(),silent?0.f:-.25f);
        if(realtime)std::this_thread::sleep_until(began+std::chrono::nanoseconds(at*1000000000LL/rate));
        // Isolated correctness runs pace against actual consumption rather than
        // pretending an arbitrarily fast producer is normal DAW delivery.
        if(!realtime && !injected)
        {
            const auto until=juce::Time::getMillisecondCounter()+30000;
            while(queue->depth()>=16 && juce::Time::getMillisecondCounter()<until)std::this_thread::yield();
            REQUIRE(queue->depth()<16);
        }
        if(fault && !injected && at>=int64_t(61)*rate)
        {
            REQUIRE(worker.flush(30000));worker.holdWorkerForTesting(true);
            const auto until=juce::Time::getMillisecondCounter()+10000;
            while(!worker.workerHeldForTesting() && juce::Time::getMillisecondCounter()<until)juce::Thread::sleep(1);
            REQUIRE(worker.workerHeldForTesting());injected=true;
        }
        CaptureHostTiming timing;timing.position=timing.sample=timing.ppq=timing.tempo=timing.meter=timing.playing=true;
        timing.samplePosition=songAt;timing.ppqPosition=double(songAt)*2/rate;timing.rate=rate;timing.bpm=120;timing.numerator=4;timing.denominator=4;
        if(queue->tryPush(songAt,rate,left.data(),right.data(),count,worker.captureGeneration(),pass,timing))++queued;
        else
        {
            ++dropped;CaptureFailureEvent event;event.reason=CaptureFailureReason::queueFull;event.pass=pass;event.epoch=worker.captureGeneration();
            event.start=songAt;event.frames=count;event.queued=queued;event.dropped=dropped;event.timing=timing;
            worker.failPass(event);
        }
        high=std::max(high,queue->depth());
        if(probe && dropped)break;
        if(injected && dropped){worker.holdWorkerForTesting(false);break;}
        // Controlled sub-budget delay in paced success runs; the separately
        // injected held worker above supplies a deterministic pressure failure.
        if(realtime && !fault && at>rate && at%(rate*5)<frames)worker.delayNextStageForTesting(5);
    }
    REQUIRE(worker.flush(60000));const auto previewLimited=worker.progressSnapshot().limited;
    if(frames==64 && !fault && !probe)
    {
        REQUIRE(!previewLimited);
        awaitCapture([&]{
            const auto preview=worker.progressSnapshot();
            if(preview.ranges.empty())return false;
            const auto& last=preview.ranges.back().range;
            return last.startSample+static_cast<int64_t>(last.frameCount)==int64_t(seconds)*rate;
        });
    }
    REQUIRE(flow.stop(pass));REQUIRE(worker.flush(60000));
    const auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-began).count();
    const auto snapshot=worker.snapshot();const auto metrics=worker.metrics();
    std::cout<<"pressure seconds="<<seconds<<" rate="<<rate<<" frames="<<frames<<" realtime="<<realtime<<" fault="<<fault<<" patterns="<<patterns
        <<" queued="<<queued<<" dropped="<<dropped<<" high="<<high<<" preview-limited="<<previewLimited<<" stages="<<metrics.stages
        <<" stage-max-ms="<<metrics.maxStageMs<<" write-verify-max-ms="<<metrics.maxWriteMs<<" validation-max-ms="<<metrics.maxValidationMs<<" wall-s="<<elapsed<<'\n'<<std::flush;
    if(snapshot.failure)std::cout<<m3::ui::captureFailureText(snapshot.failure,true)<<'\n'<<std::flush;
    if(fault || (probe && dropped))
    {
        REQUIRE(dropped && (probe || injected) && flow.snapshot().phase==workflow::Phase::failed);
        REQUIRE(snapshot.manifest.ranges.empty() && worker.progressSnapshot().ranges.empty());
        REQUIRE(snapshot.failure && snapshot.failure.event.reason==CaptureFailureReason::queueFull);
        if(fault)REQUIRE(snapshot.failure.event.start>=int64_t(61)*rate); // No earlier unexpected pressure is hidden by the injected fault.
        const auto retry=flow.begin(true);REQUIRE(queue->tryPush(0,rate,left.data(),right.data(),frames,worker.captureGeneration(),retry));
        REQUIRE(flow.stop(retry) && worker.flush(30000) && flow.snapshot().result==workflow::Result::accepted);
        REQUIRE(worker.snapshot().failure.event.pass==pass);
    }
    else
    {
        REQUIRE(!dropped && flow.snapshot().result==workflow::Result::accepted && !snapshot.manifest.ranges.empty());
        for(const auto& range:snapshot.manifest.ranges)REQUIRE(range.frameCount<=uint64_t(rate)*5 && range.frameCount<=1920000);
        CaptureSession restored(root.file);REQUIRE(restored.restore(snapshot.manifest.projectId,snapshot.manifest.instanceId,snapshot.reference,snapshot.manifest.revision));
        float l=0,r=0;REQUIRE(restored.readSample(0,l,r) && l==.125f && r==-.25f);
        REQUIRE(restored.readSample(int64_t(seconds)*rate+(patterns?2*rate:0)-1,l,r));
        if(patterns){REQUIRE(!restored.readSample(int64_t(31)*rate,l,r));REQUIRE(restored.readSample(int64_t(17)*rate+rate/2,l,r) && l==0 && r==0);}
    }
    worker.stopThread(-1);
}
inline void capturePressureMatrix()
{
    for(int rate:{44100,48000})for(int frames:{64,512,8192})
    {capturePressureCase(180,rate,frames,false,false);capturePressureCase(300,rate,frames,false,false);capturePressureCase(180,rate,frames,false,true);}
    capturePressureCase(180,48000,512,false,false,true);
}
