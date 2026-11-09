#pragma once

#include "LoopWetPlan.h"

namespace m3
{
struct LoopPlaybackPublication
{
    enum class Preparation { waiting, noWet, budget, ready };
    uint64_t captureGeneration = 0, sourceRevision = 0, dependencies = 0;
    double rate = 0;
    LoopPhraseTiming timing;
    Preparation preparation = Preparation::waiting;
    float gain = 1;
    bool wetOnly = false;
    LoopWetPlan plan;
};

class LoopPlayback
{
public:
    struct Status
    {
        LoopLiveReference::State phase;
        LoopCueScheduler::State playback;
        LoopPlaybackPublication::Preparation preparation;
    };

    bool publish(std::unique_ptr<LoopPlaybackPublication>);
    void collect();
    uint64_t pinnedBytes() const noexcept;
    void invalidatePrepared() noexcept { dependencyEpoch.fetch_add(1,std::memory_order_release); }
    uint64_t dependencies() const noexcept { return dependencyEpoch.load(std::memory_order_acquire); }

    // Configuration has a serialized control writer. The callback reads one bounded snapshot.
    void setReference(const LoopReferenceSetup&, bool externalCommands) noexcept;
    void invalidateReference() noexcept { resetEpoch.fetch_add(1,std::memory_order_release); }
    void requestCue() noexcept { commands.fetch_or(loop_capture::cue,std::memory_order_release); }
    void requestStop() noexcept { commands.fetch_or(loop_capture::stop,std::memory_order_release); }
    Status status() const noexcept;
    void process(float* left, float* right, int frames, const loop_capture::MidiObservations&,
                 double rate, uint64_t generation, uint64_t revision, bool allowed, bool discontinuity) noexcept;

private:
    enum SlotState { free, published, retained, retired };
    struct Slot
    {
        std::atomic<SlotState> state{free};
        std::unique_ptr<LoopPlaybackPublication> data;
    };
    std::array<Slot,4> slots;
    std::atomic<int> pending{-1};
    int latest = -1, held = -1;
    void takePublication() noexcept;
    void releaseHeld() noexcept;

    std::atomic<uint64_t> dependencyEpoch{0}, resetEpoch{0}, configSequence{0};
    uint64_t seenReset = 0;
    std::atomic<double> referenceRate{0}, referenceTempo{0};
    std::atomic<int> referenceBeats{0}, referenceNumerator{0}, referenceDenominator{0};
    std::atomic<bool> originConfirmed{false}, external{false};
    std::atomic<uint16_t> commands{0};
    std::atomic<unsigned> statusWord{0};

    LoopCueScheduler scheduler;
    int64_t cursor = 0;
    LoopWetSample lastWet;
    float dry = 1;
    bool wasAllowed = false;
};
}
