#pragma once

#include "AudioCaptureQueue.h"
#include "CaptureContinuity.h"
#include "ScopedRecapture.h"
#include "CaptureSession.h"

using PluginCaptureQueue = SpscAudioCaptureQueue<32>;

class CaptureWorker final : public juce::Thread
{
public:
    CaptureWorker(PluginCaptureQueue& queue, juce::File projectsRoot);
    ~CaptureWorker() override;
    void setHostRate(double rate) noexcept;
    bool canMap() const noexcept { return mappingAllowed.load(std::memory_order_acquire); }
    void setChangeTolerance(double tolerance) noexcept {changeTolerance.store(tolerance);}
    bool isLoading() const noexcept { return loading.load() || restoreRequested.load() || clearRequested.load(); }
    void requestClear() { recapture.cancel(); mappingAllowed.store(false); generation.fetch_add(1); clearRequested.store(true); }
    bool requestRecapture(int64_t start,int64_t end,double rate) { return canMap() && recapture.begin(start,end,rate); }
    void cancelRecapture() { recapture.cancel(); }
    m3::CaptureAdmission recaptureAdmission(int64_t start,int frames,double rate) const noexcept { return recapture.admit(start,frames,rate); }
    m3::RecaptureSnapshot recaptureSnapshot() const { return recapture.snapshot(); }
    void setRecaptureHandler(std::function<void(int64_t,int64_t)> handler) { const juce::ScopedLock guard(commandLock);recaptureHandler=std::move(handler); }
    void setCommitHandler(std::function<void(int64_t,int64_t,uint64_t)> handler) {const juce::ScopedLock guard(commandLock);commitHandler=std::move(handler);}
    uint64_t captureGeneration() const noexcept { return generation.load(); }
    void requestRestore(const std::string& project, const std::string& instance,
                        const juce::String& reference, uint64_t revision);
    CaptureSessionSnapshot snapshot() const;
    uint64_t discontinuities() const noexcept { return discontinuityCount.load(); }
    // Non-audio-thread barrier: capture must be disabled before calling.
    bool flush(int timeoutMilliseconds = 10000);

private:
    void run() override;
    void handleBlock(const AudioCaptureBlock& block);
    void flushPending();
    void updateSnapshot();
    PluginCaptureQueue& queue;
    CaptureSession session;
    mutable juce::CriticalSection snapshotLock;
    CaptureSessionSnapshot published;
    mutable juce::CriticalSection commandLock;
    struct Restore { std::string project, instance; juce::String reference; uint64_t revision = 0; };
    Restore restore;
    std::atomic<bool> restoreRequested{false}, clearRequested{false}, loading{false};
    std::atomic<bool> mappingAllowed{true}, flushRequested{false};
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
    std::function<void(int64_t,int64_t,uint64_t)> commitHandler;
};
