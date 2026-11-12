#pragma once

#include <Capture/LoopBeatClock.h>
#include <Capture/LoopMidiObservations.h>
#include <optional>

namespace m3
{
// Rack sends beat pulses without a transport position. Count the first pulse
// of each clock run as beat one. A patch can mark a different downbeat.
class LoopClockGrid
{
public:
    void reset(double rate) noexcept
    {
        clock.begin(rate);
        last = -1;
        beatInBar = 0;
        beatsSincePhrase.reset();
    }

    bool observe(int64_t sample, uint16_t roles, double rate, bool mayAlign) noexcept
    {
        bool invalid = (roles & loop_capture::reset) || clock.expired(sample);
        if (invalid) reset(rate);
        if (!(roles & loop_capture::captureClock)) return invalid;
        seenClock = true;
        if (!clock.observe(sample))
        {
            reset(rate);
            clock.observe(sample);
            invalid = true;
        }
        if (last >= 0)
        {
            beatInBar = (beatInBar + 1) % 4;
            if (beatsSincePhrase) ++*beatsSincePhrase;
        }
        if (mayAlign && !invalid && (roles & loop_capture::phrase)) beatsSincePhrase = 0;
        const auto annotatedBeat = (roles & loop_capture::rackBeatMask) >> loop_capture::rackBeatShift;
        if (annotatedBeat) beatInBar = annotatedBeat - 1;
        else if (mayAlign && (roles & (loop_capture::phrase | loop_capture::bar)))
            beatInBar = 0;
        last = sample;
        return invalid;
    }

    bool hasClock() const noexcept { return seenClock; }
    bool ready() const noexcept { return clock.verified(); }
    int beatNumber() const noexcept { return beatInBar + 1; }

    std::optional<int64_t> nextDownbeat(int64_t sample, int64_t lead, int phraseBeats = 0) const noexcept
    {
        if (!ready() || lead < 0 || sample > INT64_MAX - std::max(int64_t{1},lead)) return {};
        const auto earliest = sample + std::max(int64_t{1},lead);
        const auto period = clock.estimatedPeriod();
        const auto steps = beatsSincePhrase && phraseBeats > 0 ? phraseBeats : 4;
        const auto nextBoundary = steps - (steps == phraseBeats && beatsSincePhrase ? *beatsSincePhrase % steps : beatInBar);
        const auto ahead = std::max(0.0,std::ceil(((earliest - last - .5) / period - nextBoundary) / steps));
        const auto delta = std::round(static_cast<long double>(period) * (nextBoundary + steps * ahead));
        if (delta >= 0x1p63L || delta > INT64_MAX - last) return {};
        const auto boundary = last + static_cast<int64_t>(delta);
        return boundary >= earliest ? std::optional{boundary} : std::nullopt;
    }

private:
    loop_capture::BeatClock clock;
    int64_t last = -1;
    int beatInBar = 0;
    std::optional<int64_t> beatsSincePhrase;
    bool seenClock = false;
};
}
