#include "AuditionMix.h"
#include <Core/MonitoringSafety.h>
#include <cmath>
namespace m3
{
bool prepareAuditionMix(const WetRender& wet,const SourceInterval* dry,AuditionMode mode,float blend,
                        uint64_t budget,WetRender& output,float& wetDb,float& sumDb,juce::String& error,bool wetEdges,GainPolicy policy)
{
    if(wet.frames()==0 || !std::isfinite(blend) || blend<0 || blend>1 || wet.rate<=0
        || (mode!=AuditionMode::wetOnly && mode!=AuditionMode::dryWet)) {error="Invalid audition inputs";return false;}
    const auto mixed=mode==AuditionMode::dryWet;
    if(mixed && (!dry || dry->rate!=wet.rate || dry->samples.empty() || dry->end<=dry->start
        || dry->samples.size()/2!=static_cast<uint64_t>(dry->end-dry->start)))
    {error="Dry source not captured for this span";return false;}
    int64_t wetEnd;if(wet.frames()>INT64_MAX || !checkedAdd(wet.start,static_cast<int64_t>(wet.frames()),wetEnd))
    {error="Audition interval overflow";return false;}
    const auto start=mixed?std::min(wet.start,dry->start):wet.start,end=mixed?std::max(wetEnd,dry->end):wetEnd;
    int64_t frames;if(!checkedSubtract(end,start,frames) || frames<=0 || static_cast<uint64_t>(frames)>budget/8)
    {error="256 MiB working budget exceeded by aligned audition";return false;}
    const auto ceiling=juce::Decibels::decibelsToGain(policy==GainPolicy::musical?candidateWetCeilingDb:-6.0f);
    float peak=0;for(size_t i=0;i<wet.samples.size();++i){const auto v=wet.samples[i]*(wetEdges?monitorEdgeGain(i/2,wet.frames(),wet.rate,.002):1);if(!std::isfinite(v)){error="Non-finite wet audition";return false;}peak=std::max(peak,std::abs(v));}
    const auto wetGuard=peak>ceiling?ceiling/peak:1.0f;
    WetRender result;result.start=start;result.rate=wet.rate;result.sourceStart=wet.sourceStart;result.sourceEnd=wet.sourceEnd;
    result.samples.resize(static_cast<size_t>(frames)*2);
    const auto wetWeight=mixed?blend:1.0f,dryWeight=mixed?1-blend:0.0f;
    for(int64_t frame=0;frame<frames;++frame)
    {
        const auto at=start+frame;
        for(int c=0;c<2;++c)
        {
            float value=0;
            if(at>=wet.start && at<wetEnd)value=wet.samples[static_cast<size_t>(at-wet.start)*2+c]*wetGuard*wetWeight*(wetEdges?monitorEdgeGain(static_cast<size_t>(at-wet.start),wet.frames(),wet.rate,.002):1);
            if(mixed && at>=dry->start && at<dry->end)
            {const auto v=dry->samples[static_cast<size_t>(at-dry->start)*2+c];if(!std::isfinite(v)){error="Non-finite captured dry";return false;}value+=v*dryWeight;}
            result.samples[static_cast<size_t>(frame)*2+c]=value;result.peak=std::max(result.peak,std::abs(value));
        }
    }
    const auto sumGuard=result.peak>ceiling?ceiling/result.peak:1.0f;
    if(sumGuard<1){for(auto& v:result.samples)v*=sumGuard;result.peak*=sumGuard;}
    wetDb=juce::Decibels::gainToDecibels(wetGuard);sumDb=juce::Decibels::gainToDecibels(sumGuard);
    output=std::move(result);error.clear();return true;
}
}
