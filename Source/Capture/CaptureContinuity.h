#pragma once

#include <cstdint>
#include <limits>

class CaptureContinuity
{
public:
    bool beginBlock(int64_t startSample, uint32_t frameCount) noexcept
    {
        const auto discontinuity = hasExpectedPosition && startSample != expectedSample;
        if (frameCount == 0 || startSample > std::numeric_limits<int64_t>::max() - static_cast<int64_t>(frameCount))
        {
            reset();
            return discontinuity;
        }

        expectedSample = startSample + static_cast<int64_t>(frameCount);
        hasExpectedPosition = true;
        return discontinuity;
    }

    void reset() noexcept
    {
        expectedSample = 0;
        hasExpectedPosition = false;
    }

private:
    int64_t expectedSample = 0;
    bool hasExpectedPosition = false;
};