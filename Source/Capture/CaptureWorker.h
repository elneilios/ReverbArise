#pragma once

#include "AudioCaptureQueue.h"
#include "CaptureContinuity.h"
#include "ScopedRecapture.h"
#include "CaptureSession.h"
#include <Core/WorkflowState.h>

using PluginCaptureQueue = SpscAudioCaptureQueue<32>;

class CaptureWorker final : public juce::Thread
{
public:
    CaptureWorker(PluginCaptureQueue& queue, juce::File projectsRoot);
    ~CaptureWorker() override;
    void setHostRate(double rate) noexcept;
    void useWorkflow(workflow::State& state) noexcept {workflowState=&state;}
    void failPass(uint64_t token) noexcept {failedPass.store(token,std::memory_order_release);}
    // Non-RT cancellation fences the actual source publication as well as mode.
    void cancelWorkflowPass();
    void holdValidationForTesting(bool hold) noexcept {validationHold.store(hold);}
    void holdWorkerForTesting(bool hold) noexcept {workerHold.store(hold);}
    bool canMap() const noexcept { return mappingAllowed.load(std::memory_order_acquire); }
    bool hasUsefulSource() const noexcept {return usefulSource.load(std::memory_order_acquire);}
    void setChangeTolerance(double tolerance) noexcept {changeTolerance.store(tolerance);}
    bool isLoading() const noexcept { return loading.load() || restoreRequested.load() || clearRequested.load(); }
    void requestClear();
    bool requestRecapture(int64_t start,int64_t end,double rate) { return canMap() && recapture.begin(start,end,rate); }
    void cancelRecapture() { recapture.cancel(); }
    m3::CaptureAdmission recaptureAdmission(int64_t start,int frames,double rate) const noexcept { return recapture.admit(start,frames,rate); }
    m3::RecaptureSnapshot recaptureSnapshot() const { return recapture.snapshot(); }
    void setRecaptureHandler(std::function<void(int64_t,int64_t)> handler) { const juce::ScopedLock guard(commandLock);recaptureHandler=std::move(handler); }
    void setCommitHandler(std::function<void(int64_t,int64_t,uint64_t,bool)> handler) {const juce::ScopedLock guard(commandLock);commitHandler=std::move(handler);}
    void setAcceptedStateHandler(std::function<void()> handler) {const juce::ScopedLock guard(commandLock);acceptedStateHandler=std::move(handler);}
    uint64_t captureGeneration() const noexcept { return generation.load(); }
    void requestRestore(const std::string& project, const std::string& instance,
                        const juce::String& reference, uint64_t revision,workflow::Mode mode=workflow::Mode::capture);
    CaptureSessionSnapshot snapshot() const;
    uint64_t discontinuities() const noexcept { return discontinuityCount.load(); }
    // Non-audio-thread barrier: capture must be disabled before calling.
    bool flush(int timeoutMilliseconds = 10000);

private:
    void run() override;
    void handleBlock(const AudioCaptureBlock& block);
    void flushPending();
    void updateSnapshot();
    void handleWorkflowBlock(const AudioCaptureBlock&);
    void settleWorkflowPass();
    PluginCaptureQueue& queue;
    CaptureSession session;
    mutable juce::CriticalSection snapshotLock;
    CaptureSessionSnapshot published;
    mutable juce::CriticalSection commandLock;
    struct Restore { std::string project, instance; juce::String reference; uint64_t revision = 0; };
    Restore restore;
    std::atomic<bool> restoreRequested{false}, clearRequested{false}, loading{false};
    std::atomic<bool> mappingAllowed{true}, flushRequested{false};
    std::atomic<bool> usefulSource{false};
    std::atomic<bool> hasPendingCapture{false};
    std::atomic<double> hostRate{0};
    std::atomic<double> changeTolerance{0};
    std::atomic<double> committedRate{0};
    std::atomic<uint64_t> discontinuityCount{0};
    std::atomic<uint64_t> generation{0};
    juce::WaitableEvent flushFinished;
    CaptureContinuity continuity;
    std::vector<float> pending;
    int64_t pendingStart = 0;
    double pendingRate = 0;
    bool pendingSilence = false, rateStale = false;
    uint64_t pendingRecaptureToken = 0;
    m3::ScopedRecapture recapture;
    std::function<void(int64_t,int64_t)> recaptureHandler;
    std::function<void(int64_t,int64_t,uint64_t,bool)> commitHandler;
    std::function<void()> acceptedStateHandler;
    workflow::State* workflowState=nullptr;
    uint64_t activePass=0,activeEpoch=0;
    std::atomic<uint64_t> failedPass{0};
    std::atomic<bool> validationHold{false},workerHold{false};
};
