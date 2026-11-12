#pragma once
#include "SwellAssetStore.h"
#include <Core/SoundPreset.h>
#include <span>

namespace m3
{
struct BundledImpulse
{
    const char* id;
    uint32_t revision;
    const char* name;
    const char* description;
    const char* license;
    const char* generator;
    double rate;
    int channels, bits;
    int64_t frames;
    const void* bytes;
    size_t size;
    const char* wavHash;
    const char* decodedHash; // Canonical sampleHash, including rate/channels/frames.
};
std::span<const BundledImpulse> bundledImpulses();
const BundledImpulse* findBundledImpulse(const juce::String& id);
bool decodeBundledImpulse(const BundledImpulse&,uint64_t budget,ImpulseResponse&,juce::String& error);
bool installBundledImpulse(SwellAssetStore&,const juce::String& id,uint64_t budget,Asset&,juce::String& error);
bool bundledImpulse(ImpulseResponse&, juce::String& error);
bool decodeDefaultImpulse(const void*,size_t,ImpulseResponse&,juce::String& error);
bool installBundledImpulse(SwellAssetStore&, Asset&, juce::String& error);
}
