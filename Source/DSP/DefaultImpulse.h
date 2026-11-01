#pragma once
#include "SwellAssetStore.h"

namespace m3
{
bool bundledImpulse(ImpulseResponse&, juce::String& error);
bool decodeDefaultImpulse(const void*,size_t,ImpulseResponse&,juce::String& error);
bool installBundledImpulse(SwellAssetStore&, Asset&, juce::String& error);
}
