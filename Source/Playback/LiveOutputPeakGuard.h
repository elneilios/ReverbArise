#pragma once

#include <algorithm>
#include <cmath>

namespace m3
{
class LiveOutputPeakGuard
{
public:
    static constexpr float ceiling = 0.891250938f; // -1 dBFS sample peak

    void prepare(double sampleRate) noexcept
    {
        if (sampleRate == rate) return;
        rate = sampleRate;
        releaseStep = 1.0 / (std::max(1.0, rate) * 0.050);
        reset();
    }

    void reset() noexcept { gain = 1.0; }
    bool recovering() const noexcept { return gain < 1.0; }

    void protectMix(float& left, float& right, float wetLeft, float wetRight, bool wetPresent) noexcept
    {
        if (wetLeft != 0.0f || wetRight != 0.0f || (wetPresent && recovering()))
            apply(left, right);
        else
            reset();
    }

    void apply(float& left, float& right) noexcept
    {
        const auto peak = std::max(std::abs(left), std::abs(right));
        const auto allowed = peak > ceiling ? static_cast<double>(ceiling) / peak : 1.0;
        gain = std::min(allowed, gain + releaseStep);
        if (recovering())
        {
            left = static_cast<float>(left * gain);
            right = static_cast<float>(right * gain);
        }
    }

private:
    double rate = 0.0, releaseStep = 1.0, gain = 1.0;
};
}
