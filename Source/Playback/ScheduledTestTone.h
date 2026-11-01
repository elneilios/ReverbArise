#pragma once

#include <cstdint>

class ScheduledTestTone
{
public:
    static bool resolveInterval(int64_t blockStartSample,
                                double blockStartPpq,
                                double bpm,
                                int numerator,
                                int denominator,
                                double sampleRate,
                                int startBar,
                                int endBar,
                                int64_t& eventStartSample,
                                int64_t& eventEndSample) noexcept;

    static float sampleAt(int64_t samplePosition,
                          int64_t eventStartSample,
                          int64_t eventEndSample,
                          double sampleRate) noexcept;

    // Returns the peak of the generated contribution, excluding dry input.
    static float addToStereo(float* left,
                            float* right,
                            int frameCount,
                            int64_t blockStartSample,
                            double blockStartPpq,
                            double bpm,
                            int numerator,
                            int denominator,
                            double sampleRate,
                            int startBar,
                            int endBar) noexcept;
};
