#pragma once

#include "LoopLiveReference.h"
#include <algorithm>
#include <array>

namespace m3
{
class LoopCueScheduler
{
public:
    enum class State { standby, queued, leadIn, repeating, stopping };
    static constexpr int anchorCapacity = 64;

    void configure(const LoopReferenceSetup& setup, double sourceRate,
                   const std::optional<LoopPhraseTiming>& source) noexcept
    {
        if (!reference.configure(setup,sourceRate,source)) return;
        stop();
        anchors.fill({});
    }

    void setProcessingRate(double rate) noexcept
    {
        const auto nextRate = std::isfinite(rate) && rate > 0 ? rate : 0;
        if (nextRate == processingRate) return;
        fadeLength = static_cast<int>(std::clamp(nextRate * .005,1.0,960.0));
        if (fadeRemaining > 0)
        {
            fadeStart *= static_cast<float>(fadeRemaining) / static_cast<float>(fadeTotal);
            const auto remaining = processingRate > 0 ? fadeRemaining * (nextRate / processingRate) : 1.0;
            fadeRemaining = fadeTotal = static_cast<int>(std::clamp(remaining,1.0,static_cast<double>(fadeLength)));
        }
        processingRate = nextRate;
    }

    void processSample(int64_t sample, uint16_t roles, bool prepared, int64_t lead) noexcept
    {
        const auto observed = roles ? reference.observe(sample,roles) : LoopLiveReference::Observation{false,reference.advance(sample)};
        if (observed.lost)
        {
            stop();
            anchors.fill({});
        }
        if (observed.boundary)
        {
            const auto cycle = reference.ordinal();
            anchors[static_cast<size_t>(cycle % anchorCapacity)] = {cycle,sample};
            if (cycle == first && active()) playback = State::repeating;
        }
        if (!prepared || (roles & loop_capture::stop)) stop();
        else if ((roles & loop_capture::cue) && playback == State::standby)
            queue(sample,lead);
        if (playback == State::queued && sample >= leadStart) playback = State::leadIn;
    }

    void invalidateReference() noexcept
    {
        stop();
        reference.revoke();
        anchors.fill({});
    }

    void stop() noexcept
    {
        if (playback == State::leadIn || playback == State::repeating)
        {
            fadeRemaining = fadeTotal = fadeLength;
            fadeStart = 1;
            playback = State::stopping;
        }
        else if (playback == State::queued) playback = State::standby;
        first = -1;
    }

    float fadeWeight() noexcept
    {
        if (playback != State::stopping) return 0;
        const auto weight = fadeStart * static_cast<float>(--fadeRemaining) / static_cast<float>(fadeTotal);
        if (fadeRemaining == 0) playback = State::standby;
        return weight;
    }

    State state() const noexcept { return playback; }
    LoopLiveReference::State phase() const noexcept { return reference.state(); }
    bool active() const noexcept { return playback == State::queued || playback == State::leadIn || playback == State::repeating; }
    int64_t firstCycle() const noexcept { return first; }
    int64_t nearCycle(int64_t sample) const noexcept
    {
        const auto origin = reference.window(0);
        return origin && sample >= origin->earliest ? static_cast<int64_t>((sample - origin->earliest) / reference.period()) : 0;
    }

    std::optional<int64_t> positionFor(int64_t cycle, int64_t sample) const noexcept
    {
        if (first < 0 || cycle < first) return {};
        const auto& anchor = anchors[static_cast<size_t>(cycle % anchorCapacity)];
        if (anchor.cycle == cycle) return sample - anchor.sample;
        // Keep the launch prediction fixed after Cue so later markers cannot shift a lead already in progress.
        const auto forecast = cycle == first ? std::optional{firstWindow} : reference.window(cycle);
        if (!forecast || sample > forecast->latest) return {};
        // A late but valid marker extends only the last negative sample.
        return std::min(int64_t{-1},sample - forecast->earliest);
    }

private:
    void queue(int64_t sample, int64_t lead) noexcept
    {
        const auto origin = reference.window(0);
        if (!origin || lead < 0 || sample > INT64_MAX - std::max(int64_t{1},lead)) return;
        const auto threshold = sample + std::max(int64_t{1},lead);
        auto cycle = std::max(reference.ordinal() + 1,
                              static_cast<int64_t>(std::ceil((threshold - origin->earliest) / reference.period())));
        if (cycle > reference.ordinal() + 1)
            if (const auto earlier = reference.cueWindow(cycle-1); earlier && earlier->earliest >= threshold) --cycle;
        auto candidate = reference.cueWindow(cycle);
        if (candidate && candidate->earliest < threshold) candidate = reference.cueWindow(++cycle);
        if (!candidate || candidate->earliest < threshold) return;
        first = cycle;
        firstWindow = *candidate;
        leadStart = candidate->earliest - lead;
        playback = State::queued;
    }

    struct Anchor { int64_t cycle = -1, sample = 0; };
    LoopLiveReference reference;
    std::array<Anchor,anchorCapacity> anchors{};
    LoopLiveReference::Window firstWindow{};
    int64_t first = -1, leadStart = 0;
    int fadeLength = 1, fadeTotal = 1, fadeRemaining = 0;
    double processingRate = 0;
    float fadeStart = 1;
    State playback = State::standby;
};
}
