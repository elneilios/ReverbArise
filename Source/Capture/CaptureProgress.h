#pragma once
#include "CaptureSession.h"
#include <algorithm>
#include <cmath>

// Display telemetry only. Never used by save, render, admission or playback.
// Built on the capture worker; no PCM survives add(). On overflow, omit new
// telemetry rather than fabricate coverage or reject valid source capture.
struct CaptureProgress
{
    static constexpr size_t maxRanges=4096,maxPeaks=65536;
    uint64_t pass=0,epoch=0;
    bool limited=false;
    std::vector<CaptureOverviewRange> ranges;
    size_t bytes() const noexcept
    {size_t n=sizeof(*this)+ranges.size()*sizeof(CaptureOverviewRange);for(const auto& r:ranges)n+=r.peaks.size()*sizeof(CapturePeak);return n;}
    void add(int64_t start,uint32_t count,const float* pcm,bool replace,const CaptureManifest& committed)
    {
        if(limited || !count || start>INT64_MAX-count || !std::all_of(pcm,pcm+size_t(count)*2,[](float x){return std::isfinite(x);}))return;
        const auto end=start+count;
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
            for(const auto& r:committed.ranges)subtract(r.startSample,r.startSample+static_cast<int64_t>(r.frameCount));
            for(const auto& r:ranges)subtract(r.range.startSample,r.range.startSample+static_cast<int64_t>(r.range.frameCount));
        }
        auto next=ranges;
        if(replace)
        {
            next.clear();
            for(const auto& old:ranges)
            {
                const auto a=old.range.startSample,b=a+static_cast<int64_t>(old.range.frameCount);
                if(b<=start || a>=end){next.push_back(old);continue;}
                const auto retain=[&](int64_t x,int64_t y)
                {
                    auto part=old;part.range.startSample=x;part.range.frameCount=static_cast<uint64_t>(y-x);part.peaks.clear();
                    // A bin crossing a replacement edge cannot truthfully be
                    // split without PCM. Omit it; never reuse overwritten peaks.
                    for(const auto& p:old.peaks)if(p.startSample>=x && p.startSample+p.frameCount<=y)part.peaks.push_back(p);
                    next.push_back(std::move(part));
                };
                if(a<start)retain(a,start);if(b>end)retain(end,b);
            }
        }
        for(auto [a,b]:admitted)
        {
            CaptureOverviewRange r;r.range.startSample=a;r.range.frameCount=static_cast<uint64_t>(b-a);
            const auto* input=pcm+static_cast<size_t>(a-start)*2;
            r.range.kind=std::all_of(input,input+static_cast<size_t>(b-a)*2,[](float x){return x==0;})?CaptureCoverageKind::silence:CaptureCoverageKind::audio;
            if(r.range.kind==CaptureCoverageKind::audio)for(auto x=a;x<b;x+=512)
            {
                const auto frames=static_cast<uint32_t>(std::min<int64_t>(512,b-x));const auto* at=pcm+static_cast<size_t>(x-start)*2;
                const auto bounds=std::minmax_element(at,at+size_t(frames)*2);r.peaks.push_back({x,frames,*bounds.first,*bounds.second});
            }
            next.push_back(std::move(r));
        }
        std::sort(next.begin(),next.end(),[](const auto& a,const auto& b){return a.range.startSample<b.range.startSample;});
        std::vector<CaptureOverviewRange> merged;
        for(auto& r:next)
        {
            if(!merged.empty() && merged.back().range.kind==r.range.kind && merged.back().range.startSample+static_cast<int64_t>(merged.back().range.frameCount)==r.range.startSample)
            {auto& back=merged.back();back.range.frameCount+=r.range.frameCount;back.peaks.insert(back.peaks.end(),r.peaks.begin(),r.peaks.end());}
            else merged.push_back(std::move(r));
        }
        size_t peaks=0;for(const auto& r:merged)peaks+=r.peaks.size();
        if(merged.size()>maxRanges || peaks>maxPeaks){limited=true;ranges.clear();return;}
        ranges=std::move(merged);
    }
};
static_assert(sizeof(CaptureProgress)+CaptureProgress::maxRanges*sizeof(CaptureOverviewRange)+CaptureProgress::maxPeaks*sizeof(CapturePeak)<4*1024*1024);
