#include "SourceVisitor.h"
#include <Core/SwellSession.h>
#include <Core/ContentHash.h>
#include <array>
#include <cmath>

namespace m3
{
bool visitSource(CaptureAssetStore& store,const CaptureManifest& manifest,int64_t start,int64_t end,bool allowGaps,
                 const SourceBlockVisitor& visitor,juce::String& error,const std::function<bool()>& cancelled)
{
    if(end<=start || !isValidCaptureManifest(manifest,error)) { if(error.isEmpty()) error="Invalid source interval"; return false; }
    std::array<float,8192> zero{}; auto cursor=start; bool first=true; juce::String warnings;
    for(const auto& range:manifest.ranges)
    {
        if(cancelled && cancelled()) { error="Cancelled"; return false; }
        int64_t rangeEnd;
        if(range.frameCount>INT64_MAX || !checkedAdd(range.startSample,static_cast<int64_t>(range.frameCount),rangeEnd))
        { error="Source range overflow"; return false; }
        const auto a=std::max(start,range.startSample),b=std::min(end,rangeEnd); if(b<=a) continue;
        if(!allowGaps && a!=cursor) { error="Uncaptured source gap at "+juce::String(cursor); return false; }
        CaptureAudioChunk chunk;
        if(range.kind==CaptureCoverageKind::audio && !store.readAudioChunk(range,manifest.sampleRate,chunk,error))
        { if(!allowGaps) return false; warnings=error; first=true; cursor=b; continue; }
        auto newRun=first || a!=cursor;
        for(auto at=a;at<b;)
        {
            if(cancelled && cancelled()) { error="Cancelled"; return false; }
            const auto count=static_cast<int>(std::min(int64_t{4096},b-at));
            const auto samples=range.kind==CaptureCoverageKind::silence ? zero.data()
                : chunk.interleavedSamples.data()+static_cast<size_t>(at-range.startSample)*2;
            if(!std::all_of(samples,samples+count*2,[](float v){return std::isfinite(v);}))
            { error="Non-finite source audio"; return false; }
            if(!visitor(at,samples,count,newRun)) { error="Source operation cancelled or capacity exceeded"; return false; }
            at+=count; newRun=false;
        }
        cursor=b; first=false;
    }
    if(!allowGaps && cursor!=end) { error="Uncaptured source gap at "+juce::String(cursor); return false; }
    error=warnings; return true;
}
bool sourceFingerprint(CaptureAssetStore& store,const CaptureManifest& map,int64_t start,int64_t end,
                       juce::String& result,juce::String& error,const std::function<bool()>& cancelled)
{
    try
    {
        ContentHash hash; hash.update(&map.sampleRate,sizeof(map.sampleRate));
        hash.update(&start,sizeof(start)); hash.update(&end,sizeof(end));
        if(!visitSource(store,map,start,end,false,[&](int64_t,const float* samples,int frames,bool)
            { hash.samples(samples,static_cast<size_t>(frames)*2); return true; },error,cancelled)) return false;
        result=hash.finish(); return true;
    }
    catch(const std::exception& e) { error=e.what(); return false; }
}
}
