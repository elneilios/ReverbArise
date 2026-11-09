#pragma once

#include <cstdint>

// Settings for the next arm. No transient state belongs in this value.
struct LoopCapturePreference
{
    enum class Method { duration, endSignal };
    enum class Unit { bars, beats };
    static constexpr unsigned maximumCount = 64;
    bool enabled = false;
    Method method = Method::duration;
    Unit unit = Unit::bars;
    unsigned count = 1;

    constexpr uint32_t packed() const noexcept
    {
        return (count << 3) | (unit == Unit::beats ? 4u : 0u)
            | (method == Method::endSignal ? 2u : 0u) | (enabled ? 1u : 0u);
    }
    static constexpr LoopCapturePreference unpack(uint32_t value) noexcept
    {
        return { (value & 1) != 0, (value & 2) != 0 ? Method::endSignal : Method::duration,
                 (value & 4) != 0 ? Unit::beats : Unit::bars, value >> 3 };
    }
    constexpr bool valid() const noexcept
    {
        return count > 0 && count <= maximumCount && (method == Method::duration || method == Method::endSignal)
            && (unit == Unit::bars || unit == Unit::beats);
    }
    bool operator==(const LoopCapturePreference&) const = default;
};
