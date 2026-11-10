#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

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
    double estimatedPeriod() const noexcept { return verified() ? (earliest + latest) * .5 : 0; }

    double tempo(int64_t extent, unsigned beats) const noexcept
    {
        if (!verified() || beats == 0)
            return 0;
        auto fit = *this;
        fit.constrain(extent, beats);
        return fit.earliest <= fit.latest ? rate * 60 / ((fit.earliest + fit.latest) * 0.5) : 0;
    }

private:
    void constrain(int64_t distance, unsigned beats) noexcept
    {
        // Rounding the two pulse timestamps introduces less than one sample
        // of interval error. Keep the supported tempo limits inclusive.
        constexpr auto infinity = std::numeric_limits<double>::infinity();
        earliest = std::max(earliest, std::nextafter((distance - 1.0) / beats, infinity));
        latest = std::min(latest, std::nextafter((distance + 1.0) / beats, -infinity));
    }

    double rate = 0, earliest = 0, latest = 0;
    int64_t origin = -1, last = -1;
    unsigned intervals = 0;
};
}
