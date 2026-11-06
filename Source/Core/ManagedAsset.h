#pragma once
#include <juce_core/juce_core.h>

namespace m3
{
// Shared durable metadata contract, independent of capture/session ownership.
struct Asset
{
    bool operator==(const Asset&) const = default;
    juce::String path, checksum, content, dependency, irContent;
    double rate = 0, bpm = 0;
    int channels = 0, engine = 1, numerator = 4, denominator = 4;
    int64_t frames = 0, start = 0, sourceStart = 0, sourceEnd = 0;
    bool present() const noexcept { return path.isNotEmpty(); }
};
juce::var assetMetadata(const Asset&);
bool parseAssetMetadata(const juce::var&,Asset&,juce::String& error);
}
