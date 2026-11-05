#pragma once
#include "ReverseConvolution.h"
#include <Core/RenderDependency.h>

namespace m3
{
// Worker-only: modifies an owned wet-source copy, never captured PCM/dry references.
// Cancellation may leave this disposable copy partially conditioned.
bool conditionWetSourceEdges(std::vector<float>& stereo, double rate, juce::String& error,
                             const RenderCancellation& cancelled = {});
}
