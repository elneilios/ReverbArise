#pragma once
#include <juce_core/juce_core.h>

namespace m3
{
inline constexpr auto sourceEdgeRevision = "source-edges-1ms-v1:";

// Generation planning compares source PCM, independent of wet processing revision.
inline juce::String renderSourceDependency(const juce::String& dependency)
{
    return dependency.startsWith(sourceEdgeRevision)
        ? dependency.substring(static_cast<int>(juce::String(sourceEdgeRevision).length())) : dependency;
}
}
