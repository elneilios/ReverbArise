#pragma once
#include "CaptureAssetStore.h"
#include <functional>

struct SourceInterval
{
    int64_t start = 0, end = 0;
    double rate = 0;
    std::vector<float> samples; // interleaved stereo
    std::string dependency;
};

// Revision-based intersections are stable when unrelated recapture splits an immutable chunk.
std::string sourceDependency(const CaptureManifest&, int64_t start, int64_t end);
bool readSourceInterval(CaptureAssetStore&, const CaptureManifest&, int64_t start, int64_t end,
                        uint64_t maximumFrames, SourceInterval&, juce::String& error,
                        const std::function<bool()>& cancelled = {});
