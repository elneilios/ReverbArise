#pragma once
#include <juce_core/juce_core.h>

namespace m3
{
inline constexpr auto sourceEdgeRevision = "source-edges-1ms-v1:";
inline constexpr auto pingPongExcitationRevision = "ping-pong-excitation-v1:";
inline constexpr auto soundShapingRevision = "sound-shaping-v1:";

// Generation planning compares source PCM, independent of wet processing revision.
inline juce::String renderSourceDependency(const juce::String& dependency)
{
    if (dependency.startsWith(pingPongExcitationRevision) || dependency.startsWith(soundShapingRevision))
        return dependency.fromLastOccurrenceOf(":", false, false);
    if (dependency.startsWith(sourceEdgeRevision)) return dependency.fromFirstOccurrenceOf(":", false, false);
    return dependency;
}
}
