#include "GainStaging.h"
#include "SwellShape.h"
#include <Core/MonitoringSafety.h>
#include <algorithm>
#include <cmath>

namespace m3
{
namespace
{
float peakBetween(const SourceInterval& source,int64_t start,int64_t end) noexcept
{
    float peak=0;
    for(auto at=start;at<end;++at)
        for(int channel=0;channel<2;++channel)
            peak=std::max(peak,std::abs(source.samples[static_cast<size_t>(at-source.start)*2+channel]));
    return peak;
}

template<class Sample> bool maximumRms(int64_t frames,int64_t window,int64_t hop,const Sample& sample,
                                     const RenderCancellation& cancel,float& rms,float& peak)
{
    double energy[2]{};double maximum=0;peak=0;
    for(int64_t n=0;n<frames;++n){if(n%4096==0 && cancel && cancel())return false;
        for(int c=0;c<2;++c){const double value=sample(n,c);if(!std::isfinite(value))return false;
            peak=std::max(peak,static_cast<float>(std::abs(value)));energy[c]+=value*value;
            if(n>=window){const double old=sample(n-window,c);energy[c]-=old*old;}}
        const auto first=n+1-window;
        if(first>=0 && (first%hop==0 || n==frames-1))maximum=std::max({maximum,energy[0]/window,energy[1]/window});
    }
    rms=static_cast<float>(std::sqrt(std::max(0.0,maximum)));return true;
}
}

bool prepareSourceEnergyCalibration(const Session& session,const Swell& swell,const WetRender& raw,
                                    const SourceInterval& source,int64_t referenceLimit,const juce::String& renderIdentity,
                                    GainCalibration& output,juce::String& note,const RenderCancellation& cancel,float targetDb)
{
    output={};note.clear();if(session.gainPolicy!=GainPolicy::musical)return true;
    const auto unavailable=[&](const juce::String& why){output={};note="Energy matching unavailable: "+why;return true;};
    if(!std::isfinite(raw.rate) || raw.rate<=0 || raw.rate>1e9 || !std::isfinite(targetDb) || std::abs(targetDb)>24
        || source.rate!=raw.rate || source.start>swell.arrival || source.end<=swell.arrival || source.end<=source.start
        || source.samples.size()%2 || source.samples.size()/2!=static_cast<uint64_t>(source.end)-static_cast<uint64_t>(source.start))
        return unavailable("invalid or missing local reference");
    int64_t end;if(!checkedAdd(swell.arrival,std::max<int64_t>(1,std::llround(raw.rate*candidateAttackSeconds)),end))return unavailable("reference overflow");
    end=std::min({end,referenceLimit,swell.sourceEnd,source.end});if(end<=swell.arrival)return unavailable("empty reference");
    auto neutral=swell;neutral.shape=initialShape(raw.rate);neutral.placement=0;neutral.calibration={};int64_t rawEnd,a,b;juce::String error;
    if(raw.frames()>INT64_MAX || !checkedAdd(raw.start,static_cast<int64_t>(raw.frames()),rawEnd)
        || !shapeBounds(neutral,raw.start,rawEnd,a,b,error))return unavailable("default wet support missing");
    const auto dryFrames=end-swell.arrival,wetFrames=b-a;
    GainCalibration result;result.windowFrames=std::min({std::max<int64_t>(1,std::llround(raw.rate*candidateRmsSeconds)),dryFrames,wetFrames});
    result.hopFrames=std::max<int64_t>(1,std::llround(raw.rate*candidateRmsHopSeconds));
    if(result.windowFrames<1)return unavailable("empty measurement window");
    if(!maximumRms(dryFrames,result.windowFrames,result.hopFrames,[&](int64_t n,int c){return source.samples[static_cast<size_t>(swell.arrival-source.start+n)*2+c];},cancel,result.dryRms,result.dryPeak)
        || !maximumRms(wetFrames,result.windowFrames,result.hopFrames,[&](int64_t n,int c){
            return raw.samples[static_cast<size_t>(a-raw.start+n)*2+c]*envelope(neutral.shape,a+n,a,b)*monitorEdgeGain(static_cast<size_t>(n),static_cast<size_t>(wetFrames),raw.rate,.002);
        },cancel,result.wetRms,result.wetPeak))return unavailable("cancelled or non-finite audio");
    result.referenceStart=swell.arrival;result.referenceEnd=end;result.sourceIdentity=juce::String(source.dependency);
    result.irIdentity=swell.ir.content;result.renderIdentity=renderIdentity;result.revision=session.gainRevision;
    const auto floor=juce::Decibels::decibelsToGain(candidateMatchFloorDb);
    if(result.dryRms<floor || result.wetRms<floor){note="Energy matching unavailable: dry or wet RMS below -60 dBFS";output=result;return true;}
    const auto requested=juce::Decibels::gainToDecibels(result.dryRms/result.wetRms)+targetDb;
    if(!std::isfinite(requested))return unavailable("non-finite RMS ratio");
    result.baselineDb=std::clamp(requested,-60.0f,candidateMaximumBoostDb);
    result.state=requested!=result.baselineDb?CalibrationState::capped:CalibrationState::matched;
    note=result.state==CalibrationState::capped?"Source energy matched (calibration capped)":"Source energy matched";
    output=std::move(result);return true;
}

GainCalibration fixedCalibration(const Session& session,const Swell& swell,float baselineDb,const juce::String& renderIdentity) noexcept
{
    GainCalibration result;
    if(session.gainPolicy!=GainPolicy::musical || !std::isfinite(baselineDb))return result;
    result.state=CalibrationState::fixed;result.baselineDb=std::clamp(baselineDb,-60.0f,candidateMaximumBoostDb);
    result.referenceStart=swell.arrival;result.referenceEnd=swell.sourceEnd;
    result.sourceIdentity=juce::String(swell.render.dependency);result.irIdentity=swell.ir.content;
    result.renderIdentity=renderIdentity;result.revision=session.gainRevision;return result;
}

bool measureAggregateGain(int64_t start,int64_t end,float ceilingDb,float pathGainDb,uint64_t budget,
                          const AggregateAdd& add,const RenderCancellation& cancel,float& attenuation,float& peak,
                          juce::String& error)
{
    if(end<=start || !std::isfinite(ceilingDb) || !std::isfinite(pathGainDb) || !add
        || budget<4096*2*sizeof(float)){error="Invalid aggregate gain scan";return false;}
    const auto pathGain=juce::Decibels::decibelsToGain(pathGainDb);
    const auto ceiling=juce::Decibels::decibelsToGain(ceilingDb);
    if(!std::isfinite(pathGain) || !std::isfinite(ceiling) || ceiling<=0){error="Invalid aggregate gain limits";return false;}
    std::vector<float> sum(4096*2);float measured=0;
    for(auto at=start;at<end;){if(cancel && cancel()){error="Aggregate gain scan superseded";return false;}
        const auto count=static_cast<int>(std::min<int64_t>(4096,end-at));std::fill_n(sum.data(),count*2,0.0f);
        if(!add(at,count,sum.data(),error))return false;
        for(int i=0;i<count*2;++i){const auto value=sum[static_cast<size_t>(i)]*pathGain;
            if(!std::isfinite(value)){error="Non-finite aggregate wet sample";return false;}measured=std::max(measured,std::abs(value));}
        at+=count;
    }
    peak=measured;attenuation=measured>ceiling?ceiling/measured:1.0f;error.clear();return true;
}

bool prepareSourcePeakCalibration(const Session& session,const Swell& swell,const WetRender& raw,
                                  const SourceInterval& source,int64_t referenceLimit,
                                  const juce::String& renderIdentity,GainCalibration& output,juce::String& note)
{
    GainCalibration result;
    if(session.gainPolicy!=GainPolicy::musical){output=result;note.clear();return true;}
    if(raw.rate<=0 || source.rate!=raw.rate || source.start>swell.arrival || source.end<=swell.arrival
        || source.samples.size()/2!=static_cast<size_t>(source.end-source.start))
    {note="Matching unavailable: local dry reference is not captured";output=result;return true;}
    const auto requestedFrames=raw.rate*candidateAttackSeconds;
    if(!std::isfinite(requestedFrames) || requestedFrames<1 || requestedFrames>=static_cast<double>(INT64_MAX))
    {note="Matching unavailable: invalid sample rate";output=result;return true;}
    int64_t attackEnd;
    if(!checkedAdd(swell.arrival,static_cast<int64_t>(std::llround(requestedFrames)),attackEnd))
    {note="Matching unavailable: reference overflow";output=result;return true;}
    attackEnd=std::min({attackEnd,source.end,swell.sourceEnd,referenceLimit});
    if(attackEnd<=swell.arrival){note="Matching unavailable: local dry reference is empty";output=result;return true;}
    const auto dryPeak=peakBetween(source,swell.arrival,attackEnd);
    auto neutral=swell;neutral.shape=initialShape(raw.rate);neutral.placement=0;neutral.calibration={};
    WetRender wet;juce::String error;
    if(!prepareShape(neutral,raw,previewMemoryBudget,wet,error))
    {note="Matching unavailable: "+error;output=result;return true;}
    const auto wetPeak=wet.peak;
    const auto floor=juce::Decibels::decibelsToGain(candidateMatchFloorDb);
    result.referenceStart=swell.arrival;result.referenceEnd=attackEnd;result.dryPeak=dryPeak;result.wetPeak=wetPeak;
    result.sourceIdentity=juce::String(source.dependency);result.irIdentity=swell.ir.content;
    result.renderIdentity=renderIdentity;result.revision=session.gainRevision;
    if(dryPeak<floor || wetPeak<floor)
    {note="Matching unavailable: local dry or default wet is below -60 dBFS";output=result;return true;}
    const auto requested=juce::Decibels::gainToDecibels(dryPeak/wetPeak);
    if(!std::isfinite(requested)){note="Matching unavailable: non-finite peak ratio";output={};return true;}
    result.baselineDb=std::clamp(requested,-60.0f,candidateMaximumBoostDb);
    result.state=requested>candidateMaximumBoostDb?CalibrationState::capped:CalibrationState::matched;
    note=result.state==CalibrationState::capped?"Source matched (boost capped at +24 dB)":"Source peak matched";
    output=std::move(result);return true;
}
}
