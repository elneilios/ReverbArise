#pragma once

#include "AudioCaptureQueue.h"
#include "CaptureContinuity.h"
#include "ScopedRecapture.h"
#include "CaptureSession.h"
#include "CaptureProgress.h"
#include <Core/WorkflowState.h>

using PluginCaptureQueue = SpscAudioCaptureQueue<32>;

class CaptureWorker final : public juce::Thread
{
public:
    CaptureWorker(PluginCaptureQueue& queue, juce::File projectsRoot);
    ~CaptureWorker() override;
    void setHostRate(double rate) noexcept;
    void useWorkflow(workflow::State& state) noexcept {workflowState=&state;}
    // Single callback producer. Metadata publication is bounded; no worker/UI lock.
    void failPass(CaptureFailureEvent event) noexcept;
    void failPass(uint64_t token) noexcept {CaptureFailureEvent e;e.pass=token;e.epoch=generation.load();e.reason=CaptureFailureReason::queueFull;failPass(e);}
    struct Metrics {uint64_t stages=0;double maxStageMs=0,maxWriteMs=0,maxValidationMs=0;};
    Metrics metrics() const {const juce::ScopedLock lock(snapshotLock);return publishedMetrics;}
    void delayNextStageForTesting(unsigned milliseconds) noexcept {stageDelay.store(milliseconds);}
    void throwNextStageForTesting() noexcept {stageThrow.store(true);}
    // Non-RT cancellation fences the actual source publication as well as mode.
    void cancelWorkflowPass();
    bool armRecovery(std::vector<std::pair<int64_t,int64_t>> required);
    bool recoveryArmed()const noexcept{return recoveryCaptureArmed.load();}
    void holdValidationForTesting(bool hold) noexcept {validationHold.store(hold);}
    void holdWorkerForTesting(bool hold) noexcept {workerHold.store(hold);}
    bool workerHeldForTesting() const noexcept {return workerHeld.load(std::memory_order_acquire);}
    bool canMap() const noexcept { return mappingAllowed.load(std::memory_order_acquire); }
    bool hasUsefulSource() const noexcept {return usefulSource.load(std::memory_order_acquire);}
    void setChangeTolerance(double tolerance) noexcept {changeTolerance.store(tolerance);}
    bool isRestoring() const noexcept {return restoring.load();}
    bool isLoading() const noexcept { return loading.load() || restoreRequested.load() || clearRequested.load(); }
    void requestClear();
    bool requestRecapture(int64_t start,int64_t end,double rate) { return canMap() && recapture.begin(start,end,rate); }
    void cancelRecapture() { recapture.cancel(); }
    m3::CaptureAdmission recaptureAdmission(int64_t start,int frames,double rate) const noexcept { return recapture.admit(start,frames,rate); }
    m3::RecaptureSnapshot recaptureSnapshot() const { return recapture.snapshot(); }
    void setRecaptureHandler(std::function<void(int64_t,int64_t)> handler) { const juce::ScopedLock guard(commandLock);recaptureHandler=std::move(handler); }
    void setCommitHandler(std::function<void(int64_t,int64_t,uint64_t,bool)> handler) {const juce::ScopedLock guard(commandLock);commitHandler=std::move(handler);}
    void setAcceptedStateHandler(std::function<void()> handler) {const juce::ScopedLock guard(commandLock);acceptedStateHandler=std::move(handler);}
    void setSettlementHandler(std::function<void(uint64_t,uint64_t,bool)> handler)
    {const juce::ScopedLock guard(commandLock);settlementHandler=std::move(handler);}
    void setPassContextProvider(std::function<std::optional<CaptureGenerationContext>(uint64_t)> provider)
    {const juce::ScopedLock guard(commandLock);passContextProvider=std::move(provider);}
    uint64_t captureGeneration() const noexcept { return generation.load(); }
    struct StorageRestore
    {
        std::shared_ptr<const CaptureStorageLocation> location;
        std::function<std::shared_ptr<const CaptureStorageLocation>()> resolve;
        juce::String savedManifest;
        bool allowMissingSource=false;
    };
    void requestRestore(const std::string& project, const std::string& instance,
                        const juce::String& reference, uint64_t revision,workflow::Mode mode=workflow::Mode::capture,bool preserveOnFailure=false,std::optional<StorageRestore> storage=std::nullopt);
    // Worker/control only. Verified location adoption does not reset workflow,
    // advance source generation, or drain/drop callback packets.
    bool relocateStorage(const CaptureSessionSnapshot&,uint64_t,std::shared_ptr<const CaptureStorageLocation>,
                         std::unique_ptr<CaptureSession>,std::function<bool(const std::function<bool()>&)>,juce::String&);
    CaptureSessionSnapshot snapshot() const;
    CaptureProgress progressSnapshot() const;
    std::optional<ProvisionalCaptureSnapshot> provisionalSnapshot() const;
    uint64_t discontinuities() const noexcept { return discontinuityCount.load(); }
    uint64_t omittedFailureEvents() const noexcept {return lostFailureEvents.load(std::memory_order_relaxed);}
    // Non-audio-thread barrier: capture must be disabled before calling.
    bool flush(int timeoutMilliseconds = 10000);

private:
    void run() override;
    void handleBlock(const AudioCaptureBlock& block);
    void flushPending();
    void updateSnapshot();
    void handleWorkflowBlock(const AudioCaptureBlock&);
    void settleWorkflowPass();
    void collectCallbackFailures();
    void recordWorkerFailure(CaptureFailureReason,const juce::String&,int64_t,int);
    void rememberFailure(const CaptureFailureEvent&,const juce::String&);
    void recoverWorkerException(const char*);
    bool passFailed(uint64_t token) const noexcept
    {return token && (failedPass.load(std::memory_order_acquire)==token || callbackFinalPass.load(std::memory_order_acquire)==token);}
    CaptureFailureMailbox callbackFailures;
    std::atomic<uint64_t> failureOrder{0},callbackFinalPass{0},callbackFinalDrops{0},lostFailureEvents{0};
    uint64_t callbackFailedPass=0,callbackFailedEpoch=UINT64_MAX;
    CaptureFailureRecord currentFailure,lastFailure;
    CaptureHostTiming passBaseline,lastTiming;
    uint64_t workerQueued=0,passFirstOrdinal=0;
    int64_t lastBlockStart=0;int lastBlockFrames=0;
    Metrics workingMetrics,publishedMetrics;
    std::atomic<unsigned> stageDelay{0};
    std::atomic<bool> stageThrow{false};
    PluginCaptureQueue& queue;
    CaptureSession session;
    mutable juce::CriticalSection snapshotLock;
    CaptureSessionSnapshot published;
    CaptureProgress progress,publishedProgress;
    uint32_t progressPublishedAt=0;
    bool progressDirty=false;
    void publishProgress();
    mutable juce::CriticalSection commandLock;
    struct Restore { std::string project, instance; juce::String reference; uint64_t revision = 0; bool preserveOnFailure=false;std::optional<StorageRestore> storage; };
    std::vector<std::pair<int64_t,int64_t>> recoveryRequired;
    std::atomic<bool> recoveryCaptureArmed{false},refreshAdmission{false};
    struct Relocation {
        CaptureSessionSnapshot expected;uint64_t generation=0;std::shared_ptr<const CaptureStorageLocation> location;
        std::unique_ptr<CaptureSession> recovered;std::function<bool(const std::function<bool()>&)> publication;
        juce::WaitableEvent finished;std::atomic<bool> cancelled{false};bool accepted=false;juce::String error;
    };
    std::shared_ptr<Relocation> relocation;
    void applyStorageRelocation();
    Restore restore;
    std::atomic<bool> restoreRequested{false}, clearRequested{false}, loading{false},restoring{false};
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
    std::function<void(uint64_t,uint64_t,bool)> settlementHandler;
    std::optional<ProvisionalCaptureSnapshot> publishedProvisional;
    uint64_t provisionalSequence=0;
    std::function<std::optional<CaptureGenerationContext>(uint64_t)> passContextProvider;
    juce::String passStartFailure;
    workflow::State* workflowState=nullptr;
    uint64_t activePass=0,activeEpoch=0;
    std::atomic<uint64_t> failedPass{0};
    std::atomic<bool> validationHold{false},workerHold{false},workerHeld{false};
};
