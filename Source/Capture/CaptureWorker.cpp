#include "CaptureWorker.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

CaptureWorker::CaptureWorker(PluginCaptureQueue& q, juce::File root)
    : Thread("Preverb Capture"), queue(q), session(std::move(root)), published(session.snapshot()) {}

CaptureWorker::~CaptureWorker() { stopThread(-1); }

void CaptureWorker::failPass(CaptureFailureEvent event) noexcept
{
    if(event.epoch!=generation.load() || (workflowState && event.pass!=workflowState->snapshot().pass))return;
    event.depth=queue.depth();event.capacity=PluginCaptureQueue::capacity;
    callbackFinalDrops.store(event.dropped,std::memory_order_relaxed);
    if(callbackFailedPass!=event.pass || callbackFailedEpoch!=event.epoch)
    {
        callbackFailedPass=event.pass;callbackFailedEpoch=event.epoch;
        event.order=failureOrder.fetch_add(1,std::memory_order_relaxed)+1;
        if(!callbackFailures.push(event))lostFailureEvents.fetch_add(1,std::memory_order_relaxed);
    }
    // Publish the callback-owned failure fence after its evidence is ready.
    // A late worker failure from an older pass cannot overwrite this latch.
    callbackFinalPass.store(event.pass,std::memory_order_release);
}

void CaptureWorker::rememberFailure(const CaptureFailureEvent& event,const juce::String& detail)
{
    const auto state=workflowState?workflowState->snapshot():workflow::Snapshot{};
    if(event.epoch!=generation.load() || event.pass!=state.pass || !state.locked())return;
    if(currentFailure && currentFailure.event.pass==event.pass && currentFailure.event.epoch==event.epoch && currentFailure.event.order<=event.order)return;
    currentFailure={};currentFailure.event=event;currentFailure.detail=detail.substring(0,512);
    const auto& accepted=session.snapshot();
    currentFailure.reference=accepted.reference;currentFailure.revision=accepted.manifest.revision;
    currentFailure.project=accepted.manifest.projectId;currentFailure.instance=accepted.manifest.instanceId;
    currentFailure.hadCommittedSource=!accepted.manifest.ranges.empty();
    // A pass may remain recording until the next explicit DAW Stop. Publish the
    // cause now: invalidating peaks must not leave the UI claiming healthy capture.
    const juce::ScopedLock lock(snapshotLock);
    published.failure=currentFailure;published.status=captureFailureFeedback(currentFailure);
}

void CaptureWorker::recordWorkerFailure(CaptureFailureReason reason,const juce::String& detail,int64_t start,int frames)
{
    CaptureFailureEvent event;event.reason=reason;event.pass=activePass;event.epoch=activeEpoch;
    event.order=failureOrder.fetch_add(1,std::memory_order_relaxed)+1;
    event.start=start;event.frames=frames;event.timing=lastTiming;event.baseline=passBaseline;
    event.depth=queue.depth();event.capacity=PluginCaptureQueue::capacity;
    event.queued=passFirstOrdinal?queue.totalQueued()-passFirstOrdinal+1:workerQueued;
    if(callbackFinalPass.load(std::memory_order_acquire)==activePass)event.dropped=callbackFinalDrops.load();
    rememberFailure(event,detail);failedPass.store(activePass,std::memory_order_release);
}

void CaptureWorker::collectCallbackFailures()
{
    CaptureFailureEvent event;
    while(callbackFailures.pop(event))rememberFailure(event,captureFailureReasonText(event.reason));
}

void CaptureWorker::recoverWorkerException(const char* message)
{
    int length=0;while(length<512 && message[length])++length;
    recordWorkerFailure(CaptureFailureReason::workerException,juce::String::fromUTF8(message,length),lastBlockStart,lastBlockFrames);
    collectCallbackFailures();
    // Keep ownership/failure metadata; settlement will reject and abort the pass.
    session.abortPass();pending.clear();
    const juce::ScopedLock guard(commandLock);
    const auto state=workflowState?workflowState->snapshot():workflow::Snapshot{};
    if(state.phase==workflow::Phase::validating && state.pass==activePass && generation.load()==activeEpoch)
    {
        lastFailure=currentFailure;
        if(callbackFinalPass.load(std::memory_order_acquire)==activePass)lastFailure.finalDropped=callbackFinalDrops.load();
        workflowState->complete(activePass,false,false,false);activePass=0;hasPendingCapture=false;updateSnapshot();
    }
}

void CaptureWorker::cancelWorkflowPass()
{
    const juce::ScopedLock guard(commandLock);
    if(workflowState)workflowState->cancel();
    if(recoveryCaptureArmed.exchange(false))mappingAllowed.store(false);refreshAdmission=true;notify();
    // Pass identity invalidates queued/provisional work. The accepted source
    // epoch must stay fixed: cancelling is not Clear or a source revision.
}

bool CaptureWorker::armRecovery(std::vector<std::pair<int64_t,int64_t>> required)
{
    if(required.empty() || required.size()>65536)return false;
    for(const auto [a,b]:required)if(b<=a)return false;
    const juce::ScopedLock guard(commandLock);
    if(isLoading() || !workflowState || workflowState->snapshot().locked())return false;
    {const juce::ScopedLock data(snapshotLock);if(published.location && !published.location->writable)return false;}
    workflowState->reset(workflow::Mode::capture);recoveryRequired=std::move(required);recoveryCaptureArmed=true;mappingAllowed.store(true);
    refreshAdmission=true;notify();return true;
}
void CaptureWorker::setHostRate(double rate) noexcept
{
    hostRate.store(rate);
    const auto recorded = committedRate.load();
    if (recorded > 0 && std::abs(recorded - rate) > 0.001)
        mappingAllowed.store(false, std::memory_order_release);
}

CaptureSessionSnapshot CaptureWorker::snapshot() const
{
    const juce::ScopedLock commandGuard(commandLock);
    const juce::ScopedLock lock(snapshotLock);
    auto result = published;
    if(result.failure && result.failure.event.epoch!=generation.load())result.failure={};
    if (loading.load())
    {
        result = {};
        result.manifest.projectId = restore.project;
        result.manifest.instanceId = restore.instance;
        result.manifest.revision = restore.revision;
        result.reference = restore.reference;
        result.status = "Loading saved capture assets; mapping paused";
    }
    else if (hasPendingCapture.load() && ! result.unavailable && ! result.status.startsWith("Stale:")
        && !(result.failure && workflowState && result.failure.event.pass==workflowState->snapshot().pass))
        result.status = "Capturing / committing; project save references the last committed revision";
    return result;
}

CaptureProgress CaptureWorker::progressSnapshot() const
{
    const juce::ScopedLock commandGuard(commandLock);
    const auto flow=workflowState?workflowState->snapshot():workflow::Snapshot{};
    const juce::ScopedLock lock(snapshotLock);
    if(loading.load() || clearRequested.load() || !flow.locked() || publishedProgress.pass!=flow.pass
        || publishedProgress.epoch!=generation.load() || passFailed(flow.pass))return {};
    return publishedProgress;
}
std::optional<ProvisionalCaptureSnapshot> CaptureWorker::provisionalSnapshot() const
{
    const juce::ScopedLock commandGuard(commandLock);
    const auto flow=workflowState?workflowState->snapshot():workflow::Snapshot{};
    const juce::ScopedLock guard(snapshotLock);
    if(loading.load() || clearRequested.load() || !flow.locked() || !publishedProvisional
        || publishedProvisional->pass!=flow.pass || publishedProvisional->epoch!=generation.load()
        || passFailed(flow.pass))return {};
    return publishedProvisional;
}

void CaptureWorker::publishProgress()
{
    if(!progressDirty)return;
    const auto now=juce::Time::getMillisecondCounter();
    if(progressPublishedAt && now-progressPublishedAt<100)return;
    const juce::ScopedLock lock(snapshotLock);publishedProgress=progress;
    progressPublishedAt=now;progressDirty=false;
}

void CaptureWorker::requestRestore(const std::string& project, const std::string& instance,
                                    const juce::String& reference, uint64_t revision,workflow::Mode mode,bool preserveOnFailure,std::optional<StorageRestore> storage)
{
    recapture.cancel();
    const juce::ScopedLock lock(commandLock);
    if(workflowState)workflowState->reset(mode);
    mappingAllowed.store(false);
    generation.fetch_add(1);
    loading.store(true);
    recoveryCaptureArmed=false;recoveryRequired.clear();
    restore = {project, instance, reference, revision,preserveOnFailure,std::move(storage)};
    restoreRequested.store(true, std::memory_order_release);
}

void CaptureWorker::requestClear()
{
    const juce::ScopedLock guard(commandLock);recapture.cancel();
    if(workflowState)workflowState->reset();
    mappingAllowed=false;usefulSource=false;generation.fetch_add(1);clearRequested=true;
}

bool CaptureWorker::flush(int timeout)
{
    flushFinished.reset();
    flushRequested.store(true, std::memory_order_release);
    return flushFinished.wait(timeout);
}

void CaptureWorker::updateSnapshot()
{
    const auto rate = hostRate.load();
    const auto& current = session.snapshot();
    usefulSource.store(!current.unavailable && std::any_of(current.overview.begin(),current.overview.end(),[](const auto& r){return r.available && r.range.kind==CaptureCoverageKind::audio;}),std::memory_order_release);
    committedRate.store(current.manifest.ranges.empty() ? 0 : current.manifest.sampleRate);
    if (! current.manifest.ranges.empty() && rate > 0
        && std::abs(current.manifest.sampleRate - rate) > 0.001)
        rateStale = true;
    mappingAllowed.store(! loading.load() && ! restoreRequested.load() && ! clearRequested.load()
                         && ! rateStale && (! current.unavailable || recoveryCaptureArmed.load()),
                         std::memory_order_release);
    const juce::ScopedLock lock(snapshotLock);
    published = current;
    if(lastFailure && lastFailure.event.epoch==generation.load())published.failure=lastFailure;
    if(workflowState && currentFailure && currentFailure.event.epoch==generation.load()
        && currentFailure.event.pass==workflowState->snapshot().pass && workflowState->snapshot().locked())published.failure=currentFailure;
    publishedMetrics=workingMetrics;
    if(workflowState && workflowState->snapshot().phase==workflow::Phase::failed)
        published.status=published.failure && published.failure.event.pass==workflowState->snapshot().pass
            ?captureFailureFeedback(published.failure):"Capture pass rejected; failure details unavailable. Stop and retry.";
    if (rateStale) published.status = "Stale: host sample rate changed; mapping paused. Clear and recapture.";
}

void CaptureWorker::run()
{
    AudioCaptureBlock block;
    auto lastBlock = juce::Time::getMillisecondCounter();
    while (! threadShouldExit())
    {
        if(workerHold.load()){workerHeld.store(true,std::memory_order_release);wait(2);continue;}
        workerHeld.store(false,std::memory_order_release);
        if(refreshAdmission.exchange(false))updateSnapshot();
        if (restoreRequested.exchange(false, std::memory_order_acq_rel))
        {
            Restore command;
            uint64_t commandGeneration;
            { const juce::ScopedLock lock(commandLock); command = restore; commandGeneration = generation.load(); }
            session.abortPass();activePass=0;pending.clear();
            hasPendingCapture.store(false);
            while (queue.tryPop(block)) {}
            continuity.reset();
            rateStale = false;
            if(command.storage){
                auto selected=command.storage->location;
                try{if(command.storage->resolve)selected=command.storage->resolve();}catch(const std::exception&){selected.reset();}
                if(!selected)selected=std::make_shared<const CaptureStorageLocation>(CaptureStorageLocation{{},{},{},false});
                CaptureSession candidate(selected->projectsRoot,selected);
                const bool available=candidate.restore(command.project,command.instance,command.reference,command.revision,command.storage->savedManifest);
                const juce::ScopedLock fence(commandLock);
                if(generation.load()==commandGeneration && (available || command.storage->allowMissingSource || !command.preserveOnFailure))session=std::move(candidate);
            }else if(command.preserveOnFailure)session.reload(command.project,command.instance,command.reference,command.revision);
            else session.restore(command.project, command.instance, command.reference, command.revision);
            { const juce::ScopedLock lock(commandLock);
              if (generation.load() == commandGeneration) loading.store(false); }
            updateSnapshot();
        }
        if (clearRequested.exchange(false))
        {
            session.abortPass();activePass=0;pending.clear();
            hasPendingCapture.store(false);
            while (queue.tryPop(block)) {}
            if (session.clear(hostRate.load()))
            {
                rateStale = false;
                continuity.reset();
                discontinuityCount.store(0);
            }
            if (! restoreRequested.load()) loading.store(false);
            updateSnapshot();
        }
        if (! rateStale && ! session.snapshot().manifest.ranges.empty() && hostRate.load() > 0
            && std::abs(session.snapshot().manifest.sampleRate - hostRate.load()) > 0.001)
            updateSnapshot();
        while (queue.tryPop(block))
        {
            if(workflowState)
            {
                try {handleWorkflowBlock(block);}
                catch(const std::exception& error){recoverWorkerException(error.what());}
                catch(...){recoverWorkerException("Unknown capture worker exception");}
            }
            else handleBlock(block);
            lastBlock = juce::Time::getMillisecondCounter();
        }
        publishProgress();
        if(workflowState)
        {
            collectCallbackFailures();
            try {settleWorkflowPass();}
            catch(const std::exception& error){recoverWorkerException(error.what());settleWorkflowPass();}
            catch(...){recoverWorkerException("Unknown capture settlement exception");settleWorkflowPass();}
        }
        // A second restore can arrive while the first is reading assets. Do not acknowledge
        // a flush barrier until the latest queued restore/clear command has completed.
        if (! loading.load() && ! restoreRequested.load() && ! clearRequested.load()
            && flushRequested.exchange(false, std::memory_order_acq_rel))
        {
            if(!workflowState)flushPending();
            updateSnapshot();
            flushFinished.signal();
        }
        else if (!workflowState && ! pending.empty() && juce::Time::getMillisecondCounter() - lastBlock >= 250)
            flushPending();
        wait(2);
    }
    if(workflowState){session.abortPass();return;}
    while (queue.tryPop(block)) handleBlock(block);
    flushPending();
}

void CaptureWorker::handleWorkflowBlock(const AudioCaptureBlock& block)
{
    const auto state=workflowState->snapshot();
    if(block.generation!=generation.load() || loading.load() || rateStale
        || block.recaptureToken!=state.pass || (state.phase!=workflow::Phase::recording && state.phase!=workflow::Phase::draining))return;
    if(activePass!=state.pass || activeEpoch!=block.generation)
    {
        session.abortPass();pending.clear();continuity.reset();activePass=state.pass;activeEpoch=block.generation;
        if(currentFailure.event.pass!=activePass || currentFailure.event.epoch!=activeEpoch)currentFailure={};
        passBaseline=block.timing;passBaseline.rate=block.sampleRate;lastTiming=passBaseline;workerQueued=0;passFirstOrdinal=block.queueOrdinal;
        lastBlockStart=block.startSample;lastBlockFrames=static_cast<int>(block.frameCount);
        passStartFailure.clear();
        progress={};progress.pass=activePass;progress.epoch=activeEpoch;progressDirty=true;
        std::function<std::optional<CaptureGenerationContext>(uint64_t)> provider;
        {const juce::ScopedLock guard(commandLock);provider=passContextProvider;}
        // Engine locks and metadata copies stay outside the command fence and
        // outside the callback. The same atomic pass still owns publication.
        const auto context=provider?provider(activePass):std::nullopt;
        const auto latest=workflowState->snapshot();
        if(latest.pass!=activePass || !latest.locked() || generation.load()!=activeEpoch)return;
        if(provider && !context){passStartFailure="Unable to pin capture generation settings";recordWorkerFailure(CaptureFailureReason::context,passStartFailure,block.startSample,static_cast<int>(block.frameCount));return;}
        std::vector<std::pair<int64_t,int64_t>> required;{const juce::ScopedLock guard(commandLock);if(recoveryCaptureArmed.load())required=recoveryRequired;}
        if(!(required.empty()?session.beginPass(activePass,activeEpoch,state.recapture,context?&*context:nullptr)
            :session.beginRecoveryPass(activePass,activeEpoch,required,context?&*context:nullptr))){recordWorkerFailure(CaptureFailureReason::rejected,"Capture pass could not begin",block.startSample,static_cast<int>(block.frameCount));return;}
    }
    hasPendingCapture.store(true);
    ++workerQueued;lastTiming=block.timing;lastTiming.rate=block.sampleRate;lastBlockStart=block.startSample;lastBlockFrames=static_cast<int>(block.frameCount);
    if(passFailed(activePass))return;
    if(!std::isfinite(block.sampleRate) || std::abs(block.sampleRate-hostRate.load())>.001)
    {recordWorkerFailure(CaptureFailureReason::sampleRate,"Capture sample rate is incompatible",block.startSample,static_cast<int>(block.frameCount));return;}
    if(!block.frameCount || block.frameCount>maxCaptureBlockFrames || block.startSample>INT64_MAX-block.frameCount)
    {recordWorkerFailure(CaptureFailureReason::range,"Invalid capture block range",block.startSample,static_cast<int>(block.frameCount));return;}
    if(block.frameCount && block.frameCount<=maxCaptureBlockFrames && block.startSample<=INT64_MAX-block.frameCount)
    {progress.add(block.startSample,block.frameCount,block.interleavedSamples.data(),state.recapture,session.snapshot().manifest);progressDirty=true;}
    handleBlock(block); // Reuse bounded five-second/silence/discontinuity batching.
}

void CaptureWorker::settleWorkflowPass()
{
    auto state=workflowState->snapshot();
    if(activePass && (activePass!=state.pass || activeEpoch!=generation.load()))
    {session.abortPass();pending.clear();activePass=0;hasPendingCapture=false;}
    if(state.phase!=workflow::Phase::draining)return;
    // Validate and write immutable assets outside the short command fence;
    // cancel/restore can supersede the operation without waiting for disk I/O.
    CaptureSession::PassResult result;uint64_t committedGeneration=0;
    std::function<void(int64_t,int64_t,uint64_t,bool)> handler;
    std::function<void()> acceptedHandler;
    bool usefulMaterial=false;std::function<void(uint64_t,uint64_t,bool)> settled;
    {
        const juce::ScopedLock guard(commandLock);
        state=workflowState->snapshot();if(state.phase!=workflow::Phase::draining || loading.load() || restoreRequested.load() || clearRequested.load())return;
        if(!workflowState->validating(state.pass))return;
        committedGeneration=generation.load();
    }
    while(validationHold.load() && !threadShouldExit() && workflowState->snapshot().pass==state.pass){publishProgress();wait(2);}
    if(threadShouldExit())return;
    collectCallbackFailures();
    flushPending();
    if(activePass==state.pass && activeEpoch==committedGeneration && !passFailed(state.pass))
    {
        const auto began=juce::Time::getMillisecondCounterHiRes();
        result=session.preparePass(state.pass,activeEpoch);
        workingMetrics.maxValidationMs=std::max(workingMetrics.maxValidationMs,juce::Time::getMillisecondCounterHiRes()-began);
        if(!result.success)recordWorkerFailure(session.passFailureReason()==CaptureFailureReason::none?CaptureFailureReason::rejected:session.passFailureReason(),result.error,lastBlockStart,lastBlockFrames);
    }
    else if(passFailed(state.pass))result.error=session.passFailure().isNotEmpty()?session.passFailure():passStartFailure.isNotEmpty()?passStartFailure:juce::String("Capture queue/data failure; previous source retained");
    else {result.success=true;result.error="No source captured";}
    {
        const juce::ScopedLock guard(commandLock);
        const auto current=workflowState->snapshot();
        if(generation.load()!=committedGeneration || current.pass!=state.pass || current.phase!=workflow::Phase::validating)
        {session.abortPass();pending.clear();activePass=0;hasPendingCapture=false;return;}
        if(!result.success && currentFailure && currentFailure.event.pass==state.pass && currentFailure.event.epoch==committedGeneration)
        {
            lastFailure=currentFailure;
            if(callbackFinalPass.load(std::memory_order_acquire)==state.pass)lastFailure.finalDropped=callbackFinalDrops.load();
            result.error=captureFailureFeedback(lastFailure);
        }
        if(result.success && activePass==state.pass)result.success=session.acceptPreparedPass(state.pass,activeEpoch);
        else session.abortPass();
        if(result.success && recoveryCaptureArmed.load()){recoveryCaptureArmed=false;recoveryRequired.clear();}
        activePass=0;hasPendingCapture=false;handler=commitHandler;
        updateSnapshot();
        if(!result.success){const juce::ScopedLock snapshotGuard(snapshotLock);published.status=result.error;}
    }
    if(result.success && handler)for(const auto [a,b]:result.changedIntervals)handler(a,b,committedGeneration,result.replaced);
    {
        const juce::ScopedLock guard(commandLock);
        usefulMaterial=result.useful || (state.recapture && result.replaced && usefulSource.load());
        if(generation.load()==committedGeneration && result.success)settled=settlementHandler;
    }
    // Pin optional generation intent while the pass is still locked. Engine
    // metadata/locks never enter the source publication fence or audio callback.
    if(settled)settled(state.pass,committedGeneration,usefulMaterial);
    {
        const juce::ScopedLock guard(commandLock);
        // Explicit replacement may deliberately remove a note. Other accepted
        // useful source still qualifies; ordinary mapped-only/empty passes do not.
        if(generation.load()==committedGeneration && workflowState->complete(state.pass,result.success,usefulMaterial,result.replaced)
            && result.success && (result.changed || usefulMaterial))acceptedHandler=acceptedStateHandler;
    }
    if(acceptedHandler)acceptedHandler(); // Never call host code under publication locks or on the audio thread.
}

void CaptureWorker::handleBlock(const AudioCaptureBlock& block)
{
    if (block.generation != generation.load() || loading.load() || rateStale
        || (session.snapshot().unavailable && !recoveryCaptureArmed.load()) || block.frameCount == 0
        || ! std::isfinite(block.sampleRate) || block.sampleRate <= 0) return;
    const auto& map = session.snapshot().manifest;
    if ((! map.ranges.empty() && std::abs(map.sampleRate - block.sampleRate) > 0.001)
        || (hostRate.load() > 0 && std::abs(hostRate.load() - block.sampleRate) > 0.001))
    {
        flushPending();
        rateStale = true;
        updateSnapshot();
        return;
    }
    const auto discontinuous = continuity.beginBlock(block.startSample, block.frameCount);
    const auto silent = std::all_of(block.interleavedSamples.begin(),
        block.interleavedSamples.begin() + block.frameCount * 2, [](float v) { return v == 0; });
    if (discontinuous) discontinuityCount.fetch_add(1);
    if (discontinuous || (! pending.empty() && (pendingRate != block.sampleRate || pendingSilence != silent || pendingRecaptureToken != block.recaptureToken)))
        flushPending();
    const auto limit = static_cast<uint32_t>(std::clamp(block.sampleRate * 5.0, 1.0, 1920000.0));
    for (uint32_t offset = 0; offset < block.frameCount;)
    {
        if (pending.empty())
        {
            pendingStart = block.startSample + offset;
            pendingRate = block.sampleRate;
            pendingSilence = silent;
            pendingRecaptureToken = block.recaptureToken;
        }
        const auto count = std::min(limit - static_cast<uint32_t>(pending.size() / 2), block.frameCount - offset);
        const auto begin = block.interleavedSamples.begin() + offset * 2;
        pending.insert(pending.end(), begin, begin + count * 2);
        hasPendingCapture.store(true);
        offset += count;
        if (pending.size() / 2 == limit) flushPending();
    }
}

void CaptureWorker::flushPending()
{
    if (pending.empty()) return;
    const auto frames=static_cast<uint32_t>(pending.size()/2);
    if(workflowState)
    {
        if(!passFailed(activePass))
        {
            if(const auto delay=stageDelay.exchange(0))wait(static_cast<int>(std::min(delay,1000u)));
            if(stageThrow.exchange(false))throw std::runtime_error("Injected capture storage exception");
            const auto began=juce::Time::getMillisecondCounterHiRes();
            const auto staged=session.stagePass(activePass,pendingStart,pendingRate,pending.data(),frames);
            ++workingMetrics.stages;workingMetrics.maxStageMs=std::max(workingMetrics.maxStageMs,juce::Time::getMillisecondCounterHiRes()-began);
            workingMetrics.maxWriteMs=std::max(workingMetrics.maxWriteMs,session.passWriteMilliseconds());
            if(!staged)recordWorkerFailure(session.passFailureReason(),session.passFailure(),pendingStart,static_cast<int>(frames));
            if(staged && !passFailed(activePass))if(auto manifest=session.stagedManifest())
            {
                const juce::ScopedLock guard(snapshotLock);
                publishedProvisional=ProvisionalCaptureSnapshot{std::move(*manifest),activePass,activeEpoch,++provisionalSequence};
            }
        }
        pending.clear();return;
    }
    const auto armBefore=recapture.snapshot();
    const auto commitGeneration=generation.load();
    const auto previousRevision=session.snapshot().manifest.revision;
    const auto committed=session.commit(pendingStart, pendingRate, pending.data(), frames,pendingRecaptureToken?0:changeTolerance.load());
    const auto changed=committed && session.snapshot().manifest.revision!=previousRevision;
    bool completed=false;
    if(committed) completed=recapture.committed(pendingStart,frames,pendingRecaptureToken);
    else recapture.failed(pendingRecaptureToken);
    pending.clear();
    hasPendingCapture.store(false);
    updateSnapshot();
    if(changed)
    {
        std::function<void(int64_t,int64_t,uint64_t,bool)> handler;
        {const juce::ScopedLock guard(commandLock);handler=commitHandler;}
        if(handler)handler(pendingStart,pendingStart+frames,commitGeneration,false);
    }
    if(completed)
    {
        std::function<void(int64_t,int64_t)> handler;
        {const juce::ScopedLock guard(commandLock);handler=recaptureHandler;}
        if(handler)handler(armBefore.start,armBefore.end);
    }
}
