#pragma once

#include <atomic>
#include <cmath>
#include <algorithm>

namespace m3
{
class PlaybackPeaks
{
public:
    struct Values { float wet, output; };

    Values read() const noexcept
    {
        return { wet.load(std::memory_order_relaxed), output.load(std::memory_order_relaxed) };
    }

    void publish(float wetPeak, float outputPeak, int frames, double rate) noexcept
    {
        // Hold short peaks long enough for the UI to observe them.
        const auto hold = std::isfinite(rate) && rate > 0 ? rate * 0.5 : 0.0;
        update(wet,wetRemaining,wetPeak,frames,hold);
        update(output,outputRemaining,outputPeak,frames,hold);
    }

private:
    static void update(std::atomic<float>& value, double& remaining, float peak, int frames, double hold) noexcept
    {
        remaining -= frames;
        if (remaining <= 0 || peak >= value.load(std::memory_order_relaxed))
        {
            value.store(peak,std::memory_order_relaxed);
            remaining = hold;
        }
    }

    std::atomic<float> wet{0}, output{0};
    double wetRemaining = 0, outputRemaining = 0;
};
}
