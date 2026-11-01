#include "ScheduledTestTone.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

namespace
{
constexpr double frequencyHz = 440.0;
constexpr double peakAmplitude = 0.12589254117941673;
constexpr double fadeDurationSeconds = 0.005;
}

bool ScheduledTestTone::resolveInterval(int64_t blockStartSample,
                                        double blockStartPpq,
                                        double bpm,
                                        int numerator,
                                        int denominator,
                                        double sampleRate,
                                        int startBar,
                                        int endBar,
                                        int64_t& eventStartSample,
                                        int64_t& eventEndSample) noexcept
{
    eventStartSample = 0;
    eventEndSample = 0;

    if (! std::isfinite(blockStartPpq) || ! std::isfinite(bpm) || ! std::isfinite(sampleRate)
        || bpm <= 0.0 || sampleRate <= 0.0 || numerator <= 0 || denominator <= 0
        || startBar < 1 || endBar <= startBar)
        return false;

    const auto quarterNotesPerBar = static_cast<double>(numerator) * 4.0 / static_cast<double>(denominator);
    const auto samplesPerQuarterNote = sampleRate * 60.0 / bpm;
    const auto startPpq = static_cast<double>(startBar - 1) * quarterNotesPerBar;
    const auto endPpq = static_cast<double>(endBar - 1) * quarterNotesPerBar;
    const auto startPosition = static_cast<long double>(blockStartSample)
        + std::round(static_cast<long double>((startPpq - blockStartPpq) * samplesPerQuarterNote));
    const auto endPosition = static_cast<long double>(blockStartSample)
        + std::round(static_cast<long double>((endPpq - blockStartPpq) * samplesPerQuarterNote));
    const auto minimumPosition = static_cast<long double>(std::numeric_limits<int64_t>::min());
    const auto maximumPosition = static_cast<long double>(std::numeric_limits<int64_t>::max());

    if (startPosition < minimumPosition || startPosition > maximumPosition
        || endPosition < minimumPosition || endPosition > maximumPosition)
        return false;

    eventStartSample = static_cast<int64_t>(startPosition);
    eventEndSample = static_cast<int64_t>(endPosition);
    return eventEndSample > eventStartSample;
}

float ScheduledTestTone::sampleAt(int64_t samplePosition,
                                  int64_t eventStartSample,
                                  int64_t eventEndSample,
                                  double sampleRate) noexcept
{
    if (samplePosition < eventStartSample || samplePosition >= eventEndSample
        || sampleRate <= 0.0 || eventEndSample <= eventStartSample)
        return 0.0f;

    const auto fadeFrames = std::max<int64_t>(1, static_cast<int64_t>(std::llround(sampleRate * fadeDurationSeconds)));
    const auto offset = samplePosition - eventStartSample;
    const auto framesRemaining = eventEndSample - samplePosition;
    auto fadeIn = 1.0;
    auto fadeOut = 1.0;

    if (offset < fadeFrames)
    {
        const auto phase = static_cast<double>(offset) / static_cast<double>(fadeFrames);
        fadeIn = 0.5 - 0.5 * std::cos(std::numbers::pi * phase);
    }

    if (framesRemaining <= fadeFrames)
    {
        const auto phase = static_cast<double>(framesRemaining) / static_cast<double>(fadeFrames);
        fadeOut = 0.5 - 0.5 * std::cos(std::numbers::pi * phase);
    }

    const auto oscillatorPhase = 2.0 * std::numbers::pi * frequencyHz
        * static_cast<double>(offset) / sampleRate;
    return static_cast<float>(peakAmplitude * fadeIn * fadeOut * std::sin(oscillatorPhase));
}

float ScheduledTestTone::addToStereo(float* left,
                                    float* right,
                                    int frameCount,
                                    int64_t blockStartSample,
                                    double blockStartPpq,
                                    double bpm,
                                    int numerator,
                                    int denominator,
                                    double sampleRate,
                                    int startBar,
                                    int endBar) noexcept
{
    if (left == nullptr || right == nullptr || frameCount <= 0)
        return 0.0f;

    int64_t eventStartSample = 0;
    int64_t eventEndSample = 0;
    if (! resolveInterval(blockStartSample, blockStartPpq, bpm, numerator, denominator,
                          sampleRate, startBar, endBar, eventStartSample, eventEndSample))
        return 0.0f;

    auto peak = 0.0f;
    for (int frame = 0; frame < frameCount; ++frame)
    {
        const auto tone = sampleAt(blockStartSample + frame, eventStartSample, eventEndSample, sampleRate);
        peak = std::max(peak, std::abs(tone));
        left[frame] += tone;
        right[frame] += tone;
    }
    return peak;
}
