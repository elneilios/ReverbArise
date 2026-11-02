#include "CaptureWorker.h"
#include <algorithm>
#include <cmath>

CaptureWorker::CaptureWorker(PluginCaptureQueue& q, juce::File root)
    : Thread("Preverb Capture"), queue(q), session(std::move(root)), published(session.snapshot()) {}

CaptureWorker::~CaptureWorker() { stopThread(-1); }

void CaptureWorker::cancelWorkflowPass()
{
    const juce::ScopedLock guard(commandLock);
    if(workflowState)workflowState->cancel();
    // Pass identity invalidates queued/provisional work. The accepted source
    // epoch must stay fixed: cancelling is not Clear or a source revision.
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
    if (loading.load())
    {
        result = {};
        result.manifest.projectId = restore.project;
        result.manifest.instanceId = restore.instance;
        result.manifest.revision = restore.revision;
        result.reference = restore.reference;
        result.status = "Loading saved capture assets; mapping paused";
    }
    else if (hasPendingCapture.load() && ! result.unavailable && ! result.status.startsWith("Stale:"))
        result.status = "Capturing / committing; project save references the last committed revision";
    return result;
}

void CaptureWorker::requestRestore(const std::string& project, const std::string& instance,
                                    const juce::String& reference, uint64_t revision,workflow::Mode mode)
{
    recapture.cancel();
    const juce::ScopedLock lock(commandLock);
    if(workflowState)workflowState->reset(mode);
    mappingAllowed.store(false);
    generation.fetch_add(1);
    loading.store(true);
    restore = {project, instance, reference, revision};
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
                         && ! rateStale && ! current.unavailable,
                         std::memory_order_release);
    const juce::ScopedLock lock(snapshotLock);
    published = current;
    if (rateStale) published.status = "Stale: host sample rate changed; mapping paused. Clear and recapture.";
}

void CaptureWorker::run()
{
    AudioCaptureBlock block;
    auto lastBlock = juce::Time::getMillisecondCounter();
    while (! threadShouldExit())
    {
        if(workerHold.load()){wait(2);continue;}
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
            session.restore(command.project, command.instance, command.reference, command.revision);
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
            if(workflowState)handleWorkflowBlock(block);else handleBlock(block);
            lastBlock = juce::Time::getMillisecondCounter();
        }
        if(workflowState)settleWorkflowPass();
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
        if(!session.beginPass(activePass,activeEpoch,state.recapture)){failedPass.store(activePass);return;}
    }
    hasPendingCapture.store(true);
    if(failedPass.load()==activePass)return;
    if(!std::isfinite(block.sampleRate) || std::abs(block.sampleRate-hostRate.load())>.001)
    {failedPass.store(activePass);return;}
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
    {
        const juce::ScopedLock guard(commandLock);
        state=workflowState->snapshot();if(state.phase!=workflow::Phase::draining || loading.load() || restoreRequested.load() || clearRequested.load())return;
        if(!workflowState->validating(state.pass))return;
        committedGeneration=generation.load();
    }
    while(validationHold.load() && !threadShouldExit() && workflowState->snapshot().pass==state.pass)wait(2);
    if(threadShouldExit())return;
    flushPending();
    if(activePass==state.pass && activeEpoch==committedGeneration && failedPass.load()!=state.pass)
        result=session.preparePass(state.pass,activeEpoch);
    else if(failedPass.load()==state.pass)result.error=session.passFailure().isNotEmpty()?session.passFailure():juce::String("Capture queue/data failure; previous source retained");
    else {result.success=true;result.error="No source captured";}
    {
        const juce::ScopedLock guard(commandLock);
        const auto current=workflowState->snapshot();
        if(generation.load()!=committedGeneration || current.pass!=state.pass || current.phase!=workflow::Phase::validating)
        {session.abortPass();pending.clear();activePass=0;hasPendingCapture=false;return;}
        if(result.success && activePass==state.pass)result.success=session.acceptPreparedPass(state.pass,activeEpoch);
        else session.abortPass();
        activePass=0;hasPendingCapture=false;handler=commitHandler;
        updateSnapshot();
        if(!result.success){const juce::ScopedLock snapshotGuard(snapshotLock);published.status=result.error;}
    }
    if(result.success && handler)for(const auto [a,b]:result.changedIntervals)handler(a,b,committedGeneration,result.replaced);
    {
        const juce::ScopedLock guard(commandLock);
        if(generation.load()==committedGeneration && workflowState->complete(state.pass,result.success,result.useful,result.replaced)
            && result.success && (result.changed || result.useful))acceptedHandler=acceptedStateHandler;
    }
    if(acceptedHandler)acceptedHandler(); // Never call host code under publication locks or on the audio thread.
}

void CaptureWorker::handleBlock(const AudioCaptureBlock& block)
{
    if (block.generation != generation.load() || loading.load() || rateStale
        || session.snapshot().unavailable || block.frameCount == 0
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
        if(failedPass.load()!=activePass && !session.stagePass(activePass,pendingStart,pendingRate,pending.data(),frames))failedPass.store(activePass);
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
