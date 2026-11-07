#pragma once
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

struct SongTime
{
    bool valid = false;
    int64_t sample = 0;
    double ppq = 0, rate = 0, bpm = 0;
    int numerator = 0, denominator = 0;
};

enum class SourceBarFields { none, start, end, both };

inline bool resolveSourceBars(const SongTime& time, double captureRate, int64_t firstBar,
                             int64_t endBar, int64_t& first, int64_t& end, std::string& error,
                             SourceBarFields* affected = nullptr)
{
    if (affected) *affected = SourceBarFields::none;
    if (! time.valid || ! std::isfinite(time.ppq) || ! std::isfinite(time.bpm)
        || ! std::isfinite(time.rate) || time.rate <= 0 || time.bpm <= 0
        || time.numerator <= 0 || time.denominator <= 0)
    { error = "Host Song timing unavailable (fixed tempo/meter required)"; return false; }
    if (! std::isfinite(captureRate) || std::abs(captureRate - time.rate) > 0.001)
    { error = "Capture and host sample rates differ; clear and recapture"; return false; }
    if (firstBar < 1 || endBar <= firstBar)
    {
        if (affected) *affected = firstBar < 1 ? SourceBarFields::start : SourceBarFields::end;
        error = "Source end bar must be after start bar";
        return false;
    }
    const auto perQuarter = static_cast<long double>(time.rate) * 60 / time.bpm;
    const auto perBar = static_cast<long double>(time.numerator) * 4 / time.denominator;
    const auto boundary = [&](int64_t bar, int64_t& result)
    {
        const auto value = std::round(static_cast<long double>(time.sample)
            + ((static_cast<long double>(bar) - 1) * perBar - time.ppq) * perQuarter);
        // Windows long double is double; use a strict upper bound to avoid casting 2^63.
        if (! std::isfinite(value) || value < -9223372036854775808.0L || value >= 9223372036854775808.0L)
            return false;
        result = static_cast<int64_t>(value); return true;
    };
    if (! boundary(firstBar, first) || ! boundary(endBar, end) || end <= first)
    {
        if (affected) *affected = SourceBarFields::both;
        error = "Source sample interval overflows";
        return false;
    }
    error.clear(); return true;
}

inline bool sameMusicalMapping(const SongTime& a, const SongTime& b) noexcept
{
    return a.valid && b.valid && a.rate == b.rate && a.bpm == b.bpm
        && a.numerator == b.numerator && a.denominator == b.denominator;
}
