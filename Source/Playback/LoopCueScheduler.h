#pragma once

#include "LoopLiveReference.h"
#include "LoopClockGrid.h"
#include "LoopCycles.h"
#include <algorithm>
#include <array>

namespace m3
{
class LoopCueScheduler
{
public:
    enum class State { standby, queued, leadIn, repeating, stopping, finishing };
    static constexpr int anchorCapacity = 64;
    static constexpr uint16_t finishCommand = 1 << 11; // Local Stop button only.

    struct Timeline
    {
        int64_t origin, period, last;
    };

    void configure(const LoopReferenceSetup& setup, double sourceRate,
                   const std::optional<LoopPhraseTiming>& source, bool usePlaylist = false,
                   bool observeClock = false) noexcept
    {
        const auto period = (usePlaylist || observeClock) && source && sourceRate == processingRate ? source->endSample : 0;
        phraseBeats = source ? source->beats : 0;
        const auto referenceChanged = reference.configure(setup,sourceRate,source);
        if (!referenceChanged && period == gridLength && usePlaylist == playlistGrid && observeClock == clockGrid) return;
        gridLength = period;
        playlistGrid = usePlaylist;
        clockGrid = observeClock;
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
        rackClock.reset(nextRate);
    }

    void processSample(int64_t sample, uint16_t roles, bool prepared, int64_t lead,
                       std::optional<int64_t> playlistDownbeat = {}, const Timeline* timeline = nullptr,
                       int64_t wetEnd = 0, bool allowTimelineEntry = false) noexcept
    {
        if (clockGrid)
        {
            const auto hadClock = rackClock.hasClock();
            if (rackClock.observe(sample,roles,processingRate,!active()) || hadClock != rackClock.hasClock()) stop();
        }
        if (timeline && allowTimelineEntry && prepared && playback == State::standby
            && !(roles & (finishCommand | loop_capture::finish | loop_capture::stop)))
        {
            const auto last = loopCycleAnchor(timeline->origin,timeline->last,timeline->period);
            if (timeline->last == INT64_MAX || (last && sample - *last < wetEnd))
            {
                gridOrigin = timeline->origin;
                scheduledPeriod = timeline->period;
                scheduledLast = timeline->last;
                first = 0;
                leadStart = gridOrigin - lead;
                playback = State::queued;
            }
        }
        if ((roles & (finishCommand | loop_capture::finish)) && !(roles & loop_capture::stop))
        {
            if (playback == State::leadIn || playback == State::repeating)
            {
                finishCycle = playback == State::leadIn ? first
                    : usesGrid() ? std::min(nearCycle(sample),scheduledLast) : reference.ordinal();
                playback = State::finishing;
            }
            else if (playback != State::finishing || (roles & finishCommand)) stop();
            roles &= ~loop_capture::cue;
        }
        if (usesGrid())
        {
            if (!prepared || (roles & loop_capture::stop)) stop();
            else if ((roles & loop_capture::cue) && playback == State::standby && gridLength > 0)
            {
                const auto downbeat = playlistGrid ? playlistDownbeat : rackClock.nextDownbeat(sample,lead,phraseBeats);
                if (downbeat)
                {
                    first = 0;
                    gridOrigin = *downbeat;
                    scheduledPeriod = gridLength;
                    scheduledLast = INT64_MAX;
                    leadStart = gridOrigin - lead;
                    playback = State::queued;
                }
            }
            if (playback == State::queued && sample >= leadStart) playback = State::leadIn;
            if (playback == State::leadIn && sample >= gridOrigin) playback = State::repeating;
            if (timeline && active() && lastCycle() != INT64_MAX)
            {
                if (const auto end = loopCycleAnchor(gridOrigin,lastCycle(),scheduledPeriod); end && sample - *end >= wetEnd)
                {
                    playback = State::standby;
                    first = -1;
                }
            }
            else if (playback == State::finishing && nearCycle(sample) > finishCycle) stop();
            return;
        }
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
            if (playback == State::finishing && cycle > finishCycle) stop();
            else if (cycle == first && (playback == State::queued || playback == State::leadIn)) playback = State::repeating;
        }
        if (!prepared || (roles & loop_capture::stop)) stop();
        else if ((roles & loop_capture::cue) && playback == State::standby)
            queue(sample,lead);
        if (playback == State::queued && sample >= leadStart) playback = State::leadIn;
    }

    void invalidateReference(bool resetClock = true) noexcept
    {
        stop();
        reference.revoke();
        anchors.fill({});
        if (resetClock) rackClock.reset(processingRate);
    }

    void stop() noexcept
    {
        if (playback == State::leadIn || playback == State::repeating || playback == State::finishing)
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
    LoopLiveReference::State phase() const noexcept
    {
        if (usesGrid())
            return gridLength > 0 && (playlistGrid || rackClock.ready()) ? LoopLiveReference::State::ready
                                                                         : LoopLiveReference::State::waitingForClock;
        return reference.state();
    }
    bool usesClock() const noexcept { return clockGrid && rackClock.hasClock(); }
    bool usesGrid() const noexcept { return playlistGrid || usesClock(); }
    bool active() const noexcept { return playback != State::standby && playback != State::stopping; }
    int64_t firstCycle() const noexcept { return first; }
    int64_t lastCycle() const noexcept { return playback == State::finishing ? finishCycle : scheduledLast; }
    std::optional<int64_t> playheadAt(int64_t sample) const noexcept
    {
        if (playback == State::leadIn) return positionFor(first,sample);
        if (playback == State::repeating) return positionFor(usesGrid() ? std::min(nearCycle(sample),scheduledLast) : reference.ordinal(),sample);
        if (playback == State::finishing) return positionFor(finishCycle,sample);
        return {};
    }

    std::optional<int64_t> nextLeadInStart(int64_t wetStart) const noexcept
    {
        if (playback != State::repeating || wetStart >= 0) return {};
        if (usesGrid())
            return scheduledLast == INT64_MAX ? std::optional{std::max(int64_t{0},scheduledPeriod + wetStart)} : std::nullopt;
        const auto cycle = reference.ordinal();
        const auto next = reference.window(cycle + 1);
        const auto& anchor = anchors[static_cast<size_t>(cycle % anchorCapacity)];
        if (!next || anchor.cycle != cycle) return {};
        return std::max(int64_t{0}, (next->earliest - anchor.sample) + wetStart);
    }

    int64_t nearCycle(int64_t sample) const noexcept
    {
        if (usesGrid()) return scheduledPeriod > 0 ? std::max(int64_t{0},sample - gridOrigin) / scheduledPeriod : 0;
        const auto origin = reference.window(0);
        return origin && sample >= origin->earliest ? static_cast<int64_t>((sample - origin->earliest) / reference.period()) : 0;
    }

    std::optional<int64_t> positionFor(int64_t cycle, int64_t sample) const noexcept
    {
        if (first < 0 || cycle < first) return {};
        if (usesGrid())
        {
            const auto anchor=loopCycleAnchor(gridOrigin,cycle,scheduledPeriod);
            if(!anchor)return {};
            return sample-*anchor;
        }
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
    LoopClockGrid rackClock;
    std::array<Anchor,anchorCapacity> anchors{};
    LoopLiveReference::Window firstWindow{};
    int64_t first = -1, leadStart = 0;
    int64_t gridLength = 0, gridOrigin = 0, scheduledPeriod = 0, scheduledLast = INT64_MAX;
    int phraseBeats = 0;
    bool playlistGrid = false, clockGrid = false;
    int64_t finishCycle = 0;
    int fadeLength = 1, fadeTotal = 1, fadeRemaining = 0;
    double processingRate = 0;
    float fadeStart = 1;
    State playback = State::standby;
};
}
