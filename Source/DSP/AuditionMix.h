#pragma once
#include "ReverseConvolution.h"
#include <Core/SwellSession.h>
#include <Capture/SourceRangeReader.h>

namespace m3
{
bool prepareAuditionMix(const WetRender& shaped,const SourceInterval* dry,AuditionMode mode,float blend,
                        uint64_t budget,WetRender& output,juce::String& error,
                        bool wetEdges=true,std::optional<float> liveWetDb={});
}
