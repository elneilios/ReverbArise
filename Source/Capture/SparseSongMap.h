#pragma once

#include <cstdint>
#include <vector>

class SparseSongMap
{
public:
    void write(int64_t startSample, const float* interleavedSamples, uint32_t frameCount);
    bool isCovered(int64_t samplePosition) const noexcept;
    bool getSample(int64_t samplePosition, float& left, float& right) const noexcept;
    size_t segmentCount() const noexcept;
    void clear() noexcept;

private:
    struct Segment
    {
        int64_t startSample = 0;
        std::vector<float> interleavedSamples;

        int64_t endSample() const noexcept;
    };

    std::vector<Segment> segments;
};