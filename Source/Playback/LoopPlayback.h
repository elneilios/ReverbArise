#pragma once

#include "LoopWetPlan.h"
#include <Capture/CaptureSession.h>
#include <Core/TransportTime.h>

namespace m3
{
struct LoopPlaybackPublication
{
    enum class Preparation { waiting, noWet, budget, ready };
    uint64_t captureGeneration = 0, sourceRevision = 0, dependencies = 0;
    double rate = 0;
    LoopPhraseTiming timing;
    Preparation preparation = Preparation::waiting;
    float gain = 1, fixedGridGain = 1;
    bool wetOnly = false, intentionallySilent = false;
    LoopWetPlan plan;
    std::shared_ptr<const std::vector<CapturePeak>> leadInPeaks;
    bool prepareLeadInPeaks(const std::function<bool()>& cancelled);
    uint64_t bytes() const noexcept
    {
        return plan.audio.bytes() + (leadInPeaks ? leadInPeaks->capacity() * sizeof(CapturePeak) : 0);
    }
};

class LoopPlayback
{
public:
    enum class Access { disabled, observe, perform };
    struct Status
    {
        LoopLiveReference::State phase;
        LoopCueScheduler::State playback;
        LoopPlaybackPublication::Preparation preparation;
        std::optional<int64_t> playhead;
        std::optional<int64_t> nextLeadInStart;
    };
    struct LeadInDisplay
    {
        std::optional<int64_t> start;
        std::shared_ptr<const std::vector<CapturePeak>> peaks;
        float gain = 1;
        bool operator==(const LeadInDisplay&) const = default;
    };

    bool publish(std::unique_ptr<LoopPlaybackPublication>);
    void discardPending();
    void collect();
    uint64_t pinnedBytes() const noexcept;
    void invalidatePrepared() noexcept { dependencyEpoch.fetch_add(1,std::memory_order_release); }
    uint64_t dependencies() const noexcept { return dependencyEpoch.load(std::memory_order_acquire); }

    // Configuration has a serialized control writer. The callback reads one bounded snapshot.
    void setReference(const LoopReferenceSetup&, bool externalCommands) noexcept;
    void invalidateReference() noexcept { resetEpoch.fetch_add(1,std::memory_order_release); }
    bool requestCue() noexcept;
    // Storage holds this from the final playback check through adoption.
    juce::SpinLock::ScopedTryLockType tryFenceCue() const noexcept { return juce::SpinLock::ScopedTryLockType(cueFence); }
    void requestStop() noexcept { commands.fetch_or(loop_capture::stop,std::memory_order_release); }
    void requestFinish() noexcept { commands.fetch_or(LoopCueScheduler::finishCommand,std::memory_order_release); }
    Status status() const noexcept;
    bool performUsesFixedWetGrid() const noexcept { return fixedWetGrid.load(std::memory_order_acquire); }
    // Call under the same control lock as publish and collect.
    LeadInDisplay leadInDisplay() const noexcept;
    void process(float* left, float* right, int frames, const loop_capture::MidiObservations&,
                 double rate, uint64_t generation, uint64_t revision, Access access, bool discontinuity,
                 const LoopReferenceSetup& observedReference = {},
                 std::optional<SongTime> playlist = {}, bool protectLiveOutput = true) noexcept;

private:
    enum SlotState { free, published, retained, retired };
    struct Slot
    {
        std::atomic<SlotState> state{free};
        std::unique_ptr<LoopPlaybackPublication> data;
    };
    std::array<Slot,4> slots;
    std::atomic<int> pending{-1};
    int latest = -1, held = -1, fadingFrom = -1;
    int editFadeRemaining = 0, editFadeLength = 0;
    void takePublication() noexcept;
    void releaseEditFade() noexcept;
    void releaseHeld() noexcept;
    void retireLatestWithoutSource(uint64_t revision) noexcept;

    std::atomic<uint64_t> dependencyEpoch{0}, resetEpoch{0}, configSequence{0};
    uint64_t seenReset = 0, seenGeneration = 0, seenRevision = 0;
    std::atomic<double> referenceRate{0}, referenceTempo{0};
    std::atomic<int> referenceBeats{0}, referenceNumerator{0}, referenceDenominator{0};
    std::atomic<bool> phraseInput{false}, external{false};
    std::atomic<uint16_t> commands{0};
    juce::SpinLock cueFence;
    // A supported phrase fits in 23 bits. A signed cursor also covers
    // up to 64 phrases of pre-roll. A nonzero lead-in encodes the display's
    // slot and gain route in the same word.
    static constexpr int playheadShift = 8, leadInShift = 38, displaySlotShift = 61;
    static constexpr uint64_t fixedGridDisplay = uint64_t{1} << 63;
    static constexpr uint64_t displaySlotMask = 3;
    static constexpr int64_t playheadBias = int64_t{1} << (leadInShift - playheadShift - 1);
    static constexpr uint64_t playheadMask = (uint64_t{1} << (leadInShift - playheadShift)) - 1;
    static constexpr uint64_t leadInMask = (uint64_t{1} << (displaySlotShift - leadInShift)) - 1;
    static std::optional<int64_t> decodeLeadIn(uint64_t word) noexcept;
    uint64_t currentStatusWord() const noexcept;
    std::atomic<uint64_t> statusWord{0};
    std::atomic<uint64_t> statusResetEpoch{0};
    std::atomic<bool> fixedWetGrid{true};

    LoopCueScheduler scheduler;
    std::optional<double> playlistPhrasePpq;
    int64_t cursor = 0;
    LoopWetSample lastWet;
    LiveOutputPeakGuard outputGuard;
    bool lastWetOnly = false;
    float dry = 1;
    bool wasObserving = false;
};
}
