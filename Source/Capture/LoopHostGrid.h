#pragma once

#include <Core/TransportTime.h>
#include "LoopPhraseClock.h"
#include <optional>

namespace loop_capture
{
// Capture timing from a continuous host playhead, including declared repeats.
// Perform keeps its own playback origin.
class HostGrid
{
public:
    struct Beat { int offset; bool downbeat; };
    struct Block
    {
        ClockProfile clock;
        std::optional<Beat> beat;
        bool starting = false;
        bool repeatStart = false, sameRepeatRange = false;
    };

    static bool supports(const SongTime& time) noexcept
    {
        return time.valid && time.rate == 48000 && time.numerator == 4 && time.denominator == 4
            && ClockProfile{time.rate, time.bpm}.valid() && time.sample >= 0
            && time.ppq >= 0 && time.ppq < 0x1p52;
    }

    Block observe(const TransportTime& host, int frames) noexcept
    {
        const auto& time = host.song;
        const bool starting = host.playing && !wasPlaying;
        wasPlaying = host.playing;
        const bool hasRepeatRange = host.looping && host.loopStart >= 0 && host.loopStart < host.loopEnd;
        const bool reachesRepeatStart = hasRepeatRange && frames > 0 && time.sample <= host.loopStart
            && time.sample > host.loopStart - frames;
        auto quarter = std::ceil(time.ppq);
        int64_t repeatSample = -1;
        const bool repeatDownbeat = reachesRepeatStart && std::fmod(quarter, 4.0) == 0
            && resolvePpqSample(time,quarter,repeatSample) && repeatSample == host.loopStart;
        auto boundaryTime = time;
        boundaryTime.sample = repeatSample;
        boundaryTime.ppq = quarter;
        if (!host.playing || (!supports(time) && !(repeatDownbeat && supports(boundaryTime)))
            || frames <= 0 || time.sample > INT64_MAX - frames)
        {
            previous = {};
            return {};
        }

        const bool prerollWrap = repeatDownbeat && previous.playing && previous.looping
            && previous.loopStart >= 0 && previous.loopEnd > previous.loopStart
            && host.loopEnd - host.loopStart == previous.loopEnd - previous.loopStart
            && previousEnd >= 0 && previousEnd - previous.loopEnd == time.sample - host.loopStart
            && sameRelativeMapping(previous, host);
        const bool sameRepeatRange = sameDeclaredRepeat(previous, host) || prerollWrap;
        const bool wrapped = prerollWrap || isDeclaredRepeatWrap(previous, previousEnd, host);
        const bool continuous = prerollWrap || ((time.sample == previousEnd || wrapped)
            && transportMappingMatches(previous.song, time));
        previous = host;
        previousEnd = time.sample + frames;
        if (!continuous && !starting)
            return {};

        Block result{continuous ? ClockProfile{time.rate, time.bpm} : ClockProfile{}, {}, starting,
            hasRepeatRange && (starting || wrapped) && (repeatDownbeat || time.sample == host.loopStart), sameRepeatRange};
        if (result.repeatStart && repeatDownbeat)
        {
            result.beat = Beat{static_cast<int>(repeatSample - time.sample),true};
            return result;
        }
        quarter = std::floor(time.ppq);
        int64_t sample = 0;
        if (!resolvePpqSample(time, quarter, sample))
            return result;
        if (sample < time.sample && !resolvePpqSample(time, ++quarter, sample))
            return result;
        const bool downbeat = std::fmod(quarter, 4.0) == 0;
        // Capture may tentatively start on the first downbeat. The next
        // callback still has to confirm continuity before the take can finish.
        if (sample >= time.sample && sample < previousEnd
            && (continuous || (downbeat && sample == time.sample)))
            result.beat = Beat{static_cast<int>(sample - time.sample), downbeat};
        return result;
    }

private:
    static bool sameRelativeMapping(const TransportTime& before, const TransportTime& after) noexcept
    {
        // FL Studio can shift its sample origin by the pre-roll length at the
        // wrap. Compare the PPQ maps relative to the declared start.
        auto previous = before.song;
        auto current = after.song;
        if (previous.sample < 0) return false;
        previous.sample -= before.loopStart;
        current.sample -= after.loopStart;
        return transportMappingMatches(previous,current);
    }

    TransportTime previous{};
    int64_t previousEnd = -1;
    bool wasPlaying = false;
};
}
