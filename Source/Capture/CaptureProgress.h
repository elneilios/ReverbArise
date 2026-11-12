#pragma once
#include "CaptureSession.h"
#include "LoopTake.h"
#include <algorithm>
#include <cmath>

// Display telemetry only. Never used by save, render, admission or playback.
// Built on the capture worker; no PCM survives add(). A Loop keeps a separate
// min/max waveform and recorded extent even if detailed coverage reaches its limit.
struct CaptureProgress
{
    static constexpr size_t maxRanges=4096,maxPeaks=65536;
    static constexpr uint32_t binFrames=512;
    static constexpr size_t maxLoopPeaks=(loop_capture::Take::maximumFrames+binFrames-1)/binFrames;
    uint64_t pass=0,epoch=0;
    CaptureSourceKind kind=CaptureSourceKind::song;
    double sampleRate=0;
    bool freshDomain=false;
    bool limited=false;
    std::vector<CaptureOverviewRange> ranges;
    CaptureOverviewRange loopWaveform;
    int64_t recordedLoopEnd() const noexcept {return static_cast<int64_t>(loopWaveform.range.frameCount);}
    size_t bytes() const noexcept
    {
        size_t n=sizeof(*this)+ranges.size()*sizeof(CaptureOverviewRange)+loopWaveform.peaks.size()*sizeof(CapturePeak);
        for(const auto& r:ranges)n+=r.peaks.size()*sizeof(CapturePeak);
        return n;
    }
    void add(int64_t start,uint32_t count,const float* pcm,bool replace,const CaptureManifest& committed)
    {
        if((limited && kind!=CaptureSourceKind::loop) || !count || start>INT64_MAX-count
            || !std::all_of(pcm,pcm+size_t(count)*2,[](float x){return std::isfinite(x);}))return;
        const auto end=start+count;
        if(kind==CaptureSourceKind::loop)
        {
            if(start!=recordedLoopEnd() || end>loop_capture::Take::maximumFrames)return;
            appendPeaks(loopWaveform.peaks,start,end,pcm);
            loopWaveform.range.frameCount=static_cast<uint64_t>(end);
        }
        if(limited)return;
        const bool appendOnly=ranges.empty() || start>=ranges.back().range.startSample+static_cast<int64_t>(ranges.back().range.frameCount);
        std::vector<std::pair<int64_t,int64_t>> admitted{{start,end}};
        const auto subtract=[&](int64_t a,int64_t b)
        {
            std::vector<std::pair<int64_t,int64_t>> next;
            for(auto [x,y]:admitted){if(y<=a || x>=b)next.emplace_back(x,y);else{if(x<a)next.emplace_back(x,a);if(y>b)next.emplace_back(b,y);}}
            admitted=std::move(next);
        };
        if(!replace)
        {
            // The exact same first-received/protected-source rule as stagePass.
            for(const auto& r:committed.ranges)
            {
                if(r.startSample>=end || admitted.empty())break;
                const auto last=r.startSample+static_cast<int64_t>(r.frameCount);
                if(last>start)subtract(r.startSample,last);
            }
            if(!appendOnly)for(const auto& r:ranges)subtract(r.range.startSample,r.range.startSample+static_cast<int64_t>(r.range.frameCount));
        }
        if(admitted.empty())return;
        const auto append=[&](auto& destination,int64_t a,int64_t b)
        {
            const auto* input=pcm+static_cast<size_t>(a-start)*2;
            const auto coverage=std::all_of(input,input+static_cast<size_t>(b-a)*2,[](float x){return x==0;})
                ? CaptureCoverageKind::silence : CaptureCoverageKind::audio;
            if(!appendOnly || destination.empty() || destination.back().range.kind!=coverage
                || destination.back().range.startSample+static_cast<int64_t>(destination.back().range.frameCount)!=a)
            {
                CaptureOverviewRange region;
                region.range={a,0,coverage};
                destination.push_back(std::move(region));
            }
            auto& region=destination.back();
            region.range.frameCount+=static_cast<uint64_t>(b-a);
            if(coverage==CaptureCoverageKind::audio)appendPeaks(region.peaks,a,b,input);
        };
        if(appendOnly)
        {
            // Most blocks extend the take. Keep the existing bins in place and
            // fill the last bin across callback boundaries.
            for(auto [a,b]:admitted)append(ranges,a,b);
            enforceLimit();
            return;
        }
        std::vector<CaptureOverviewRange> next;
        if(replace)
        {
            for(const auto& old:ranges)
            {
                const auto a=old.range.startSample,b=a+static_cast<int64_t>(old.range.frameCount);
                if(b<=start || a>=end){next.push_back(old);continue;}
                const auto retain=[&](int64_t x,int64_t y)
                {
                    CaptureOverviewRange part{old.range,old.available,{}};
                    part.range.startSample=x;part.range.frameCount=static_cast<uint64_t>(y-x);
                    // A bin crossing a replacement edge cannot truthfully be
                    // split without PCM. Omit it; never reuse overwritten peaks.
                    const auto first=std::lower_bound(old.peaks.begin(),old.peaks.end(),x,
                        [](const CapturePeak& peak,int64_t sample){return peak.startSample<sample;});
                    const auto last=std::partition_point(first,old.peaks.end(),
                        [y](const CapturePeak& peak){return peak.startSample+peak.frameCount<=y;});
                    part.peaks.assign(first,last);
                    next.push_back(std::move(part));
                };
                if(a<start)retain(a,start);if(b>end)retain(end,b);
            }
        }
        else next=ranges;
        for(auto [a,b]:admitted)append(next,a,b);
        std::sort(next.begin(),next.end(),[](const auto& a,const auto& b){return a.range.startSample<b.range.startSample;});
        std::vector<CaptureOverviewRange> merged;
        for(auto& r:next)
        {
            if(!merged.empty() && merged.back().range.kind==r.range.kind && merged.back().range.startSample+static_cast<int64_t>(merged.back().range.frameCount)==r.range.startSample)
            {
                auto& back=merged.back();
                back.range.frameCount+=r.range.frameCount;
                auto first=r.peaks.begin();
                if(!back.peaks.empty() && first!=r.peaks.end()
                    && peakCapacity(back.peaks.back(),first->startSample)>=first->frameCount)
                {
                    extendPeak(back.peaks.back(),*first);
                    ++first;
                }
                back.peaks.insert(back.peaks.end(),first,r.peaks.end());
            }
            else merged.push_back(std::move(r));
        }
        ranges=std::move(merged);
        enforceLimit();
    }
private:
    static uint32_t peakCapacity(const CapturePeak& peak,int64_t start) noexcept
    {
        return peak.frameCount<binFrames && peak.startSample+peak.frameCount==start
            ? binFrames-peak.frameCount : 0;
    }
    static void extendPeak(CapturePeak& peak,const CapturePeak& next) noexcept
    {
        peak.frameCount+=next.frameCount;
        peak.minimum=std::min(peak.minimum,next.minimum);
        peak.maximum=std::max(peak.maximum,next.maximum);
    }
    static void appendPeaks(std::vector<CapturePeak>& peaks,int64_t start,int64_t end,const float* pcm)
    {
        for(auto x=start;x<end;)
        {
            const auto capacity=peaks.empty()?0:peakCapacity(peaks.back(),x);
            const auto frames=static_cast<uint32_t>(std::min<int64_t>(capacity?capacity:binFrames,end-x));
            const auto* at=pcm+static_cast<size_t>(x-start)*2;
            const auto [low,high]=std::minmax_element(at,at+size_t(frames)*2);
            const CapturePeak peak{x,frames,*low,*high};
            if(capacity)extendPeak(peaks.back(),peak);
            else peaks.push_back(peak);
            x+=frames;
        }
    }
    void enforceLimit()
    {
        size_t peaks=0;for(const auto& r:ranges)peaks+=r.peaks.size();
        if(ranges.size()>maxRanges || peaks>maxPeaks){limited=true;ranges.clear();}
    }
};
static_assert(sizeof(CaptureProgress)+CaptureProgress::maxRanges*sizeof(CaptureOverviewRange)
    +(CaptureProgress::maxPeaks+CaptureProgress::maxLoopPeaks)*sizeof(CapturePeak)<4*1024*1024);
