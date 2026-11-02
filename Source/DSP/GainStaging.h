#pragma once
#include "ReverseConvolution.h"
#include <Capture/SourceRangeReader.h>
#include <Core/SwellSession.h>
#include <functional>

namespace m3
{
bool prepareSourcePeakCalibration(const Session&,const Swell&,const WetRender&,const SourceInterval&,
                                  int64_t referenceEnd,const juce::String& renderIdentity,
                                  GainCalibration&,juce::String& note);
GainCalibration fixedCalibration(const Session&,const Swell&,float baselineDb,const juce::String& renderIdentity) noexcept;
bool prepareSourceEnergyCalibration(const Session&,const Swell&,const WetRender&,const SourceInterval&,
                                    int64_t referenceEnd,const juce::String& renderIdentity,
                                    GainCalibration&,juce::String& note,const RenderCancellation& cancel={},float targetDb=0);
using AggregateAdd = std::function<bool(int64_t start,int frames,float* interleaved,juce::String& error)>;
bool measureAggregateGain(int64_t start,int64_t end,float ceilingDb,float pathGainDb,uint64_t budget,
                          const AggregateAdd&,const RenderCancellation&,float& attenuation,float& peak,
                          juce::String& error);
}
