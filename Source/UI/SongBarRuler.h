#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

struct SongBarTick
{
    int64_t bar = 1;
    double samplePosition = 0;
};

// Fixed tempo/meter only. The host sample/PPQ pair anchors Playlist bar 1;
// a capture chunk's start is never treated as the beginning of a bar.
inline std::vector<SongBarTick> makeSongBarRuler(double firstSample, double lastSample,
    double width, double sampleRate, double bpm, int numerator, int denominator,
    int64_t hostSample, double hostPpq)
{
    if (! std::isfinite(firstSample) || ! std::isfinite(lastSample) || lastSample <= firstSample
        || ! std::isfinite(width) || width < 1 || ! std::isfinite(sampleRate) || sampleRate <= 0
        || ! std::isfinite(bpm) || bpm <= 0 || ! std::isfinite(hostPpq)
        || numerator <= 0 || denominator <= 0) return {};
    const auto samplesPerQuarter = sampleRate * 60.0 / bpm;
    const auto samplesPerBar = samplesPerQuarter * numerator * 4.0 / denominator;
    const auto origin = static_cast<double>(hostSample) - hostPpq * samplesPerQuarter;
    const auto firstBarOffset = (firstSample - origin) / samplesPerBar;
    const auto lastBarOffset = (lastSample - origin) / samplesPerBar;
    if (! std::isfinite(firstBarOffset) || ! std::isfinite(lastBarOffset)
        || std::abs(firstBarOffset) > 1.0e9 || std::abs(lastBarOffset) > 1.0e9) return {};
    auto stride = int64_t{1};
    while (samplesPerBar * static_cast<double>(stride) * width / (lastSample - firstSample) < 64.0)
        stride *= 2;
    const auto begin = static_cast<int64_t>(std::ceil(firstBarOffset / static_cast<double>(stride) - 1.0e-9)) * stride;
    std::vector<SongBarTick> ticks;
    for (auto offset = begin; static_cast<double>(offset) <= lastBarOffset + 1.0e-9; offset += stride)
        ticks.push_back({offset + 1, origin + static_cast<double>(offset) * samplesPerBar});
    return ticks;
}
