#include "SparseSongMap.h"

#include <algorithm>
#include <limits>
#include <utility>

int64_t SparseSongMap::Segment::endSample() const noexcept
{
    return startSample + static_cast<int64_t>(interleavedSamples.size() / 2);
}

void SparseSongMap::write(int64_t startSample, const float* interleavedSamples, uint32_t frameCount)
{
    if (frameCount == 0 || interleavedSamples == nullptr
        || startSample > std::numeric_limits<int64_t>::max() - static_cast<int64_t>(frameCount))
        return;

    const auto endSample = startSample + static_cast<int64_t>(frameCount);
    std::vector<Segment> updated;
    updated.reserve(segments.size() + 2);

    for (const auto& segment : segments)
    {
        if (segment.endSample() <= startSample || segment.startSample >= endSample)
        {
            updated.push_back(segment);
            continue;
        }

        if (segment.startSample < startSample)
        {
            const auto leftFrames = static_cast<size_t>(startSample - segment.startSample);
            Segment left;
            left.startSample = segment.startSample;
            left.interleavedSamples.assign(segment.interleavedSamples.begin(),
                                           segment.interleavedSamples.begin() + static_cast<std::ptrdiff_t>(leftFrames * 2));
            updated.push_back(std::move(left));
        }

        if (segment.endSample() > endSample)
        {
            const auto rightOffset = static_cast<size_t>(endSample - segment.startSample) * 2;
            Segment right;
            right.startSample = endSample;
            right.interleavedSamples.assign(segment.interleavedSamples.begin() + static_cast<std::ptrdiff_t>(rightOffset),
                                            segment.interleavedSamples.end());
            updated.push_back(std::move(right));
        }
    }

    Segment replacement;
    replacement.startSample = startSample;
    replacement.interleavedSamples.assign(interleavedSamples,
                                           interleavedSamples + static_cast<size_t>(frameCount) * 2);
    updated.push_back(std::move(replacement));

    std::sort(updated.begin(), updated.end(), [](const auto& left, const auto& right)
    {
        return left.startSample < right.startSample;
    });

    std::vector<Segment> merged;
    merged.reserve(updated.size());
    for (auto& segment : updated)
    {
        if (! merged.empty() && merged.back().endSample() == segment.startSample)
        {
            auto& destination = merged.back().interleavedSamples;
            destination.insert(destination.end(), segment.interleavedSamples.begin(), segment.interleavedSamples.end());
        }
        else
        {
            merged.push_back(std::move(segment));
        }
    }

    segments = std::move(merged);
}

bool SparseSongMap::isCovered(int64_t samplePosition) const noexcept
{
    const auto segment = std::upper_bound(segments.begin(), segments.end(), samplePosition,
        [](int64_t position, const Segment& candidate)
        {
            return position < candidate.startSample;
        });

    if (segment == segments.begin())
        return false;

    const auto& candidate = *std::prev(segment);
    return samplePosition < candidate.endSample();
}

bool SparseSongMap::getSample(int64_t samplePosition, float& left, float& right) const noexcept
{
    const auto segment = std::upper_bound(segments.begin(), segments.end(), samplePosition,
        [](int64_t position, const Segment& candidate)
        {
            return position < candidate.startSample;
        });

    if (segment == segments.begin())
        return false;

    const auto& candidate = *std::prev(segment);
    if (samplePosition >= candidate.endSample())
        return false;

    const auto frameOffset = static_cast<size_t>(samplePosition - candidate.startSample) * 2;
    left = candidate.interleavedSamples[frameOffset];
    right = candidate.interleavedSamples[frameOffset + 1];
    return true;
}

size_t SparseSongMap::segmentCount() const noexcept
{
    return segments.size();
}

void SparseSongMap::clear() noexcept
{
    segments.clear();
}