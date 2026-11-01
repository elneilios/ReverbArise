#include "Playback/ScheduledTestTone.h"

#include <array>
#include <cmath>

int main()
{
    constexpr double sampleRate = 48000.0;
    int64_t eventStart = 0;
    int64_t eventEnd = 0;

    if (! ScheduledTestTone::resolveInterval(108000, 4.5, 120.0, 4, 4, sampleRate, 2, 3,
                                             eventStart, eventEnd)
        || eventStart != 96000 || eventEnd != 192000)
        return 1;

    if (ScheduledTestTone::resolveInterval(0, 0.0, 120.0, 4, 4, sampleRate, 10, 8,
                                           eventStart, eventEnd))
        return 2;

    constexpr int64_t testEventEnd = 96000;
    if (ScheduledTestTone::sampleAt(-1, 0, testEventEnd, sampleRate) != 0.0f
        || ScheduledTestTone::sampleAt(0, 0, testEventEnd, sampleRate) != 0.0f
        || ScheduledTestTone::sampleAt(testEventEnd, 0, testEventEnd, sampleRate) != 0.0f)
        return 3;

    auto peak = 0.0f;
    int firstPositiveCrossing = -1;
    int lastPositiveCrossing = -1;
    int positiveCrossingCount = 0;
    auto previous = ScheduledTestTone::sampleAt(11999, 0, testEventEnd, sampleRate);

    for (int sample = 12000; sample < 36000; ++sample)
    {
        const auto value = ScheduledTestTone::sampleAt(sample, 0, testEventEnd, sampleRate);
        peak = std::max(peak, std::abs(value));
        if (previous <= 0.0f && value > 0.0f)
        {
            if (firstPositiveCrossing < 0)
                firstPositiveCrossing = sample;
            lastPositiveCrossing = sample;
            ++positiveCrossingCount;
        }
        previous = value;
    }

    if (std::abs(peak - 0.12589254f) > 0.0001f || positiveCrossingCount < 2)
        return 4;

    const auto measuredFrequency = static_cast<double>(positiveCrossingCount - 1) * sampleRate
        / static_cast<double>(lastPositiveCrossing - firstPositiveCrossing);
    if (std::abs(measuredFrequency - 440.0) > 1.0)
        return 5;

    std::array<float, 8> left;
    std::array<float, 8> right;
    left.fill(0.25f);
    right.fill(-0.25f);
    ScheduledTestTone::addToStereo(left.data(), right.data(), static_cast<int>(left.size()),
                                   0, 0.0, 120.0, 4, 4, sampleRate, 2, 3);
    for (size_t sample = 0; sample < left.size(); ++sample)
        if (left[sample] != 0.25f || right[sample] != -0.25f)
            return 6;

    std::array<float, 3> insideLeft{};
    std::array<float, 3> insideRight{};
    ScheduledTestTone::addToStereo(insideLeft.data(), insideRight.data(), static_cast<int>(insideLeft.size()),
                                   96000, 4.0, 120.0, 4, 4, sampleRate, 2, 3);
    if (insideLeft[0] != 0.0f || insideLeft[1] <= 0.0f
        || insideRight[1] != insideLeft[1])
        return 7;

    std::array<float, 64> seekLeft{};
    std::array<float, 64> seekRight{};
    std::array<float, 64> replayLeft{};
    std::array<float, 64> replayRight{};
    ScheduledTestTone::addToStereo(seekLeft.data(), seekRight.data(), static_cast<int>(seekLeft.size()),
                                   120000, 5.0, 120.0, 4, 4, sampleRate, 2, 3);
    ScheduledTestTone::addToStereo(replayLeft.data(), replayRight.data(), static_cast<int>(replayLeft.size()),
                                   120000, 5.0, 120.0, 4, 4, sampleRate, 2, 3);
    for (size_t sample = 0; sample < seekLeft.size(); ++sample)
    {
        const auto expected = ScheduledTestTone::sampleAt(120000 + static_cast<int64_t>(sample),
                                                          96000, 192000, sampleRate);
        if (seekLeft[sample] != expected || replayLeft[sample] != expected
            || seekRight[sample] != expected || replayRight[sample] != expected)
            return 8;
    }

    return 0;
}