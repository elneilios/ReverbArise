#pragma once
#include "ReverseConvolution.h"
#include <Core/SwellSession.h>
#include <Capture/SourceRangeReader.h>

namespace m3
{
bool prepareAuditionMix(const WetRender& shaped,const SourceInterval* dry,AuditionMode mode,float blend,
                        uint64_t budget,WetRender& output,float& wetAttenuationDb,float& sumAttenuationDb,juce::String& error,
                        bool wetEdges=true,GainPolicy policy=GainPolicy::legacy,std::optional<float> liveWetDb={});
}
