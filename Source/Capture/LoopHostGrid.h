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
        if (!host.playing || !supports(time) || frames <= 0 || time.sample > INT64_MAX - frames)
        {
            previous = {};
            return {};
        }

        const bool hasRepeatRange = host.looping && host.loopStart < host.loopEnd;
        const bool sameRepeatRange = sameDeclaredRepeat(previous, host);
        const bool wrapped = isDeclaredRepeatWrap(previous, previousEnd, host);
        const bool continuous = (time.sample == previousEnd || wrapped)
            && transportMappingMatches(previous.song, time);
        previous = host;
        previousEnd = time.sample + frames;
        if (!continuous && !starting)
            return {};

        Block result{continuous ? ClockProfile{time.rate, time.bpm} : ClockProfile{}, {}, starting,
            hasRepeatRange && (starting || wrapped) && time.sample == host.loopStart, sameRepeatRange};
        auto quarter = std::floor(time.ppq);
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
    TransportTime previous{};
    int64_t previousEnd = -1;
    bool wasPlaying = false;
};
}
