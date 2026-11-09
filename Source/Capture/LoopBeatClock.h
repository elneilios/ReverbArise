#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace loop_capture
{
// Intersect the periods consistent with each rounded interval and with the
// distance from the first observed beat. No predicted beat can close a take.
class BeatClock
{
public:
    void begin(double sampleRate) noexcept
    {
        *this = {};
        rate = sampleRate;
        earliest = rate * 60 / 300;
        latest = rate * 60 / 30;
    }

    bool observe(int64_t at) noexcept
    {
        if (origin < 0)
        {
            origin = last = at;
            return true;
        }
        if (at <= last || expired(at))
            return false;
        ++intervals;
        constrain(at - last, 1);
        constrain(at - origin, intervals);
        last = at;
        return earliest <= latest;
    }

    bool expired(int64_t at) const noexcept
    {
        return last >= 0 && (at - last > std::ceil(latest)
            || at - origin > std::ceil(latest * (intervals + 1)));
    }

    bool verified() const noexcept { return intervals >= 2 && earliest <= latest; }

    double tempo(int64_t extent, unsigned beats) const noexcept
    {
        if (!verified() || beats == 0)
            return 0;
        const auto low = std::max(earliest, (extent - 1.0) / beats);
        const auto high = std::min(latest, (extent + 1.0) / beats);
        return low <= high ? rate * 60 / ((low + high) * 0.5) : 0;
    }

private:
    void constrain(int64_t distance, unsigned beats) noexcept
    {
        earliest = std::max(earliest, (distance - 1.0) / beats);
        latest = std::min(latest, (distance + 1.0) / beats);
    }

    double rate = 0, earliest = 0, latest = 0;
    int64_t origin = -1, last = -1;
    unsigned intervals = 0;
};
}
