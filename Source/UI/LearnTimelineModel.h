#pragma once
#include <Capture/CaptureProgress.h>
#include <Core/SwellSession.h>
#include <Core/TransportTime.h>
#include <algorithm>
#include <cmath>

namespace m3::ui
{
// Dry extent only: no selection, wet bounds, renderer or host-seek dependencies.
struct LearnDryView
{
    double first=0,last=1,originAnchor=0;bool initialized=false;
    void reset()noexcept{initialized=false;}
    void update(const CaptureSessionSnapshot& source,const CaptureProgress& progress,const SongTime& time)
    {
        if(progress.freshDomain && progress.kind==CaptureSourceKind::loop)
        {
            first=0;
            last=static_cast<double>(std::max<int64_t>(1,progress.recordedLoopEnd()));
            initialized=false;
            return;
        }
        if(source.manifest.loop)
        {
            first=0;
            last=static_cast<double>(source.manifest.loop->endSample);
            initialized=false;
            return;
        }
        long double origin=0;const auto musical=songOrigin(time,origin);
        const auto rate=std::isfinite(time.rate) && time.rate>0?time.rate:source.manifest.sampleRate>0?source.manifest.sampleRate:48000.;
        const auto bar=musical?rate*60/time.bpm*time.numerator*4/time.denominator:rate*4;
        const auto quantum=std::isfinite(bar) && bar>0?bar:rate*4;
        auto anchor=musical?static_cast<double>(origin):initialized?originAnchor:0.;
        if(initialized && std::abs(anchor-originAnchor)<=1)anchor=originAnchor;
        double lo=std::min(0.,anchor),hi=std::max(0.,anchor);
        if(!initialized)hi+=quantum*4;
        originAnchor=anchor;
        const auto include=[&](const auto& r){lo=std::min(lo,static_cast<double>(r.startSample));hi=std::max(hi,static_cast<double>(r.startSample)+static_cast<double>(r.frameCount));};
        for(const auto& r:source.manifest.ranges)include(r);
        for(const auto& r:progress.ranges)include(r.range);
        // Quantized headroom avoids block-by-block rescaling. Never shrink for
        // loops, cancellation or shorter recapture while this workspace is open.
        if(!initialized || lo<first || hi>last){
            const auto wantedFirst=anchor+std::floor((lo-anchor)/quantum)*quantum;
            const auto wantedLast=anchor+(std::ceil((hi-anchor)/quantum)+1)*quantum;
            first=initialized?std::min(first,wantedFirst):wantedFirst;
            last=initialized?std::max(last,wantedLast):wantedLast;
            if(!(last>first)){first=0;last=rate*16;}initialized=true;
        }
    }
};
struct LearnCoverageInfo
{
    juce::String kind;int64_t start=0,end=0;bool provisional=false;
    juce::String description()const{return (provisional?"Recording ":"")+kind+" | samples ["+juce::String(start)+", "+juce::String(end)+")";}
};
inline LearnCoverageInfo learnCoverageAt(const CaptureSessionSnapshot& source,const CaptureProgress& progress,int64_t sample,int64_t first,int64_t last)
{
    const auto lookup=[&](const auto& regions,bool pending)->std::optional<LearnCoverageInfo>{
        for(const auto& r:regions){int64_t end;
            if(r.range.frameCount>INT64_MAX || !checkedAdd(r.range.startSample,static_cast<int64_t>(r.range.frameCount),end))continue;
            if(sample>=r.range.startSample && sample<end)return LearnCoverageInfo{!r.available?"Unavailable":r.range.kind==CaptureCoverageKind::silence?"Silence":"Audio",r.range.startSample,end,pending};}
        return {};};
    if(const auto p=lookup(progress.ranges,true))return *p;
    if(progress.limited && sample>=0 && sample<progress.recordedLoopEnd())
        return {"Coverage detail unavailable",0,progress.recordedLoopEnd(),true};
    if(const auto r=lookup(source.overview,false))return *r;
    // Missing overview telemetry is not proof of silence or an uncaptured gap.
    for(const auto& r:source.manifest.ranges){int64_t end;if(r.frameCount>INT64_MAX || !checkedAdd(r.startSample,static_cast<int64_t>(r.frameCount),end))continue;
        if(sample>=r.startSample && sample<end)return {"Unavailable",r.startSample,end,false};}
    auto a=first,b=last;
    const auto edges=[&](const auto& r){int64_t end;if(r.frameCount>INT64_MAX || !checkedAdd(r.startSample,static_cast<int64_t>(r.frameCount),end))return;
        if(end<=sample)a=std::max(a,end);if(r.startSample>sample)b=std::min(b,r.startSample);};
    for(const auto& r:source.manifest.ranges)edges(r);for(const auto& r:progress.ranges)edges(r.range);
    return {"Gap (uncaptured)",a,b,false};
}
struct LearnInspectionReadout
{
    juce::String text,details,musical;bool timingAvailable=false;
};
inline LearnInspectionReadout learnInspectionReadout(int64_t sample,const LearnCoverageInfo& coverage,const SongTime& time,double captureRate)
{
    LearnInspectionReadout result;long double origin=0;
    const auto compatible=std::isfinite(captureRate) && captureRate>=0 && (captureRate==0 || std::abs(captureRate-time.rate)<=.001);
    if(compatible && songOrigin(time,origin))
    {
        const auto seconds=(static_cast<long double>(sample)-origin)/time.rate;
        const auto beats=(static_cast<long double>(sample)-origin)/(time.rate*60.0L/time.bpm*4/time.denominator);
        const auto roundedBeats=std::round(beats*1000)/1000;
        const auto bar=std::floor(roundedBeats/time.numerator);
        const auto milliseconds=std::round(std::abs(seconds)*1000);
        if(std::isfinite(bar) && std::abs(bar)<1.e9L && std::isfinite(milliseconds) && milliseconds<9223372036854775808.0L)
        {
            auto beat=juce::String(static_cast<double>(roundedBeats-bar*time.numerator+1),3);
            while(beat.endsWithChar('0'))beat=beat.dropLastCharacters(1);
            if(beat.endsWithChar('.'))beat=beat.dropLastCharacters(1);
            const auto ms=static_cast<int64_t>(milliseconds),hours=ms/3600000,minutes=(ms/60000)%60;
            auto elapsed=hours>0?juce::String(hours)+":"+juce::String(minutes).paddedLeft('0',2):juce::String(minutes);
            elapsed+=":"+juce::String((ms/1000)%60).paddedLeft('0',2)+"."+juce::String(ms%1000).paddedLeft('0',3);
            if(seconds<0 && ms>0)elapsed="-"+elapsed;
            result.text="Bar "+juce::String(static_cast<int64_t>(bar)+1)+" Beat "+beat+" | "+elapsed;
            result.timingAvailable=true;
        }
    }
    if(!result.timingAvailable)result.text="Timing unavailable";
    result.musical=result.text;
    result.text+=" | Sample "+juce::String(sample)+" | "+(coverage.provisional?"Recording ":"")+coverage.kind;
    result.details=result.text+" | "+coverage.description();return result;
}
inline long double learnExtentFrames(const CaptureSessionSnapshot& source,const CaptureProgress& progress,const SongTime& time)
{long double origin=0;if(!songOrigin(time,origin))origin=0;
    auto first=origin,last=origin;
    const auto include=[&](const auto& r){first=std::min(first,static_cast<long double>(r.startSample));last=std::max(last,static_cast<long double>(r.startSample)+r.frameCount);};
    for(const auto& r:source.manifest.ranges)include(r);for(const auto& r:progress.ranges)include(r.range);return last-first;}
inline long double learnMappedFrames(const CaptureManifest& source)
{long double frames=0;for(const auto& r:source.ranges)frames+=r.frameCount;return frames;}
inline long double learnProvisionalFrames(const CaptureProgress& progress)
{
    if(progress.kind==CaptureSourceKind::loop)return progress.recordedLoopEnd();
    long double frames=0;for(const auto& r:progress.ranges)frames+=r.range.frameCount;return frames;
}
}
