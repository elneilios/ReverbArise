#pragma once
#include "ReverseConvolution.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace m3
{
inline bool applySourceCut(std::vector<float>& stereo, double rate, int hz, bool highPass,
                           const RenderCancellation& cancel)
{
    if (hz == 0) return true;
    const auto cutoff = std::min(static_cast<double>(hz), rate * .475);
    const auto angle = 2 * std::numbers::pi * cutoff / rate;
    const auto cosine = std::cos(angle);
    const auto alpha = std::sin(angle) / std::numbers::sqrt2;
    const auto b0 = (highPass ? 1 + cosine : 1 - cosine) / (2 * (1 + alpha));
    const auto b1 = (highPass ? -2 : 2) * b0;
    const auto a1 = -2 * cosine / (1 + alpha);
    const auto a2 = (1 - alpha) / (1 + alpha);
    double first[2]{}, second[2]{};
    for (size_t i = 0; i < stereo.size(); i += 2)
    {
        if (i % 8192 == 0 && cancel && cancel()) return false;
        for (size_t ch = 0; ch < 2; ++ch)
        {
            const auto input = stereo[i + ch];
            const auto output = b0 * input + first[ch];
            first[ch] = b1 * input - a1 * output + second[ch];
            second[ch] = b0 * input - a2 * output;
            stereo[i + ch] = static_cast<float>(output);
        }
    }
    return true;
}

inline bool dampFullResponse(ImpulseResponse& ir, int damping, const RenderCancellation& cancel)
{
    if (damping == 0 || ir.frames() < 2) return true;
    const auto channels = static_cast<size_t>(ir.channels);
    const auto energy = [&](size_t frame)
    {
        double value = 0;
        for (size_t ch = 0; ch < channels; ++ch)
        {
            const double sample = ir.samples[frame * channels + ch];
            value += sample * sample;
        }
        return value;
    };
    double total = 0, peak = 0;
    for (size_t frame = 0; frame < ir.frames(); ++frame)
    {
        if (frame % 4096 == 0 && cancel && cancel()) return false;
        const auto value = energy(frame);
        total += value;
        peak = std::max(peak, value);
    }
    if (peak == 0) return true;
    size_t onset = ir.frames(), reference = 0;
    double accumulated = 0;
    for (size_t frame = 0; frame < ir.frames(); ++frame)
    {
        if (frame % 4096 == 0 && cancel && cancel()) return false;
        const auto value = energy(frame);
        if (onset == ir.frames() && value > peak * .0001) onset = frame;
        accumulated += value;
        if (onset != ir.frames() && accumulated >= total * .9) { reference = frame; break; }
    }
    if (onset == ir.frames()) return true;
    const auto span = std::max(std::round(.045 * ir.rate), static_cast<double>(reference - onset));
    const auto protectedEnd = onset + static_cast<size_t>(std::llround(.002 * ir.rate));
    const auto reduction = damping <= 50 ? 18.0 * damping / 50 : 18.0 + 47.0 * (damping - 50) / 50;
    for (size_t frame = protectedEnd; frame < ir.frames(); ++frame)
    {
        if ((frame - protectedEnd) % 4096 == 0 && cancel && cancel()) return false;
        const auto gain = static_cast<float>(std::pow(10.0, -reduction * (frame - protectedEnd) / (20 * span)));
        for (size_t ch = 0; ch < channels; ++ch) ir.samples[frame * channels + ch] *= gain;
    }
    return true;
}
}
