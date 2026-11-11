#include "ExportSource.h"
#include <algorithm>
#include <cmath>
#include <Playback/LoopCycles.h>
namespace m3
{
void ExportSourceReader::warn(int64_t a,int64_t b,ExportCoverage kind)
{
    if(a>=b)return;
    if(!missing.empty() && missing.back().end==a && missing.back().kind==kind)missing.back().end=b;
    else missing.push_back({a,b,kind});
}
bool ExportSourceReader::load(size_t index,juce::String& error)
{
    if(cached==index)return true;
    cache={};cached=SIZE_MAX;
    const auto& range=plan->source.ranges[index];
    // Existing chunk validation keeps reads <= five seconds and <=1,920,000
    // frames. Account for both the planar decode scratch and returned stereo.
    if(range.frameCount>allowance/(4*sizeof(float))){error="Export dry chunk exceeds available working memory";return false;}
    if(!store.readAudioChunk(range,plan->source.sampleRate,cache,error))return false;
    if(!std::all_of(cache.interleavedSamples.begin(),cache.interleavedSamples.end(),[](float v){return std::isfinite(v);}))
    {cache={};error="Non-finite Export dry asset";return false;}
    cached=index;return true;
}
bool ExportSourceReader::prepare(std::shared_ptr<const SongExportPlan> input,uint64_t availableBytes,juce::String& error,const std::function<bool()>& cancel)
{
    plan.reset();cache={};cached=SIZE_MAX;missing.clear();readable.clear();allowance=availableBytes;
    if(!input || input->end<=input->origin || !isValidCaptureManifest(input->source,error))return false;
    if(input->loop){
        const auto count=input->loop->repeats;
        if(!input->source.loop || count<1 || count>LoopExportOptions::maximumRepeats)
        {error="Invalid finite Loop export plan";return false;}
        const auto scheduled=loopCycleAnchor(0,count,input->source.loop->endSample);
        const auto last=loopCycleAnchor(0,count-1,input->source.loop->endSample);
        int64_t frames;
        if(!scheduled || !last || input->origin>0 || input->end<*scheduled
            || !checkedSubtract(input->end,input->origin,frames) || frames<=0 || static_cast<uint64_t>(frames)>(UINT32_MAX-1024ull)/8)
        {error="Invalid finite Loop export bounds";return false;}
        if(input->wet>0)for(const auto& member:input->members){int64_t end;
            if(member.start<input->origin || member.start>=member.end || !checkedAdd(member.end,*last,end) || end>input->end)
            {error="Invalid finite Loop wet bounds";return false;}
        }
    }
    plan=std::move(input);readable.resize(plan->source.ranges.size(),false);
    if(plan->dry==0){error.clear();return true;}
    const auto first=plan->loop?int64_t{0}:plan->origin;
    const auto last=plan->loop?plan->source.loop->endSample:plan->end;
    auto cursor=first;
    for(size_t i=0;i<plan->source.ranges.size();++i)
    {
        if(cancel && cancel()){error="Export source preparation cancelled";plan.reset();return false;}
        const auto& range=plan->source.ranges[i];const auto a=std::max(first,range.startSample);
        const auto b=std::min(last,range.startSample+static_cast<int64_t>(range.frameCount));if(a>=b)continue;
        warn(cursor,a,ExportCoverage::gap);
        if(range.kind==CaptureCoverageKind::audio){
            if(range.frameCount>allowance/(4*sizeof(float))){error="Export dry chunk exceeds available working memory";plan.reset();return false;}
            juce::String failure;readable[i]=load(i,failure);if(!readable[i])warn(a,b,ExportCoverage::unavailable);
        }else readable[i]=true;
        cursor=b;
    }
    warn(cursor,last,ExportCoverage::gap);
    if(plan->loop && !missing.empty()){
        const auto count=static_cast<size_t>(plan->loop->repeats);
        if(!count || missing.size()>allowance/(4*sizeof(ExportWarning))/count)
        {error="Loop dry warning metadata exceeds available working memory";plan.reset();return false;}
        auto local=std::move(missing);missing.clear();missing.reserve(local.size()*count);
        allowance-=local.size()*count*sizeof(ExportWarning);
        for(size_t cycle=0;cycle<count;++cycle){
            if(cancel && cancel()){error="Export source preparation cancelled";plan.reset();return false;}
            const auto offset=loopCycleAnchor(0,static_cast<int64_t>(cycle),last);
            if(!offset){error="Loop dry extent overflows";plan.reset();return false;}
            for(const auto& warning:local)warn(warning.start+*offset,warning.end+*offset,warning.kind);
        }
    }
    error.clear();return true;
}
bool ExportSourceReader::read(int64_t start,std::span<float> stereo,juce::String& error)
{
    int64_t end;
    if(!plan || stereo.empty() || stereo.size()%2 || stereo.size()/2>exportPageFrames
        || !checkedAdd(start,static_cast<int64_t>(stereo.size()/2),end) || start<plan->origin || end>plan->end)
    {error="Invalid Export dry page";return false;}
    std::fill(stereo.begin(),stereo.end(),0.f);
    if(plan->dry==0){error.clear();return true;}
    if(plan->loop){
        const auto period=plan->source.loop->endSample;
        const auto scheduledEnd=loopCycleAnchor(0,plan->loop->repeats,period);
        if(!scheduledEnd){error="Loop dry extent overflows";return false;}
        for(auto at=std::max(start,int64_t{0});at<std::min(end,*scheduledEnd);){
            const auto local=at%period;
            const auto frames=std::min(period-local,std::min(end,*scheduledEnd)-at);
            if(!readCoverage(local,stereo.subspan(static_cast<size_t>(at-start)*2,static_cast<size_t>(frames)*2),error))return false;
            at+=frames;
        }
        error.clear();return true;
    }
    return readCoverage(start,stereo,error);
}
bool ExportSourceReader::readCoverage(int64_t start,std::span<float> stereo,juce::String& error)
{
    const auto end=start+static_cast<int64_t>(stereo.size()/2);
    for(size_t i=0;i<plan->source.ranges.size();++i)
    {
        const auto& range=plan->source.ranges[i];const auto a=std::max(start,range.startSample);
        const auto b=std::min(end,range.startSample+static_cast<int64_t>(range.frameCount));
        if(a>=b || range.kind==CaptureCoverageKind::silence || !readable[i])continue;
        // Availability was fixed at preparation. A later read failure is fatal,
        // not permission to alter a revision's samples or warning plan mid-write.
        if(!load(i,error))return false;
        std::copy_n(cache.interleavedSamples.data()+static_cast<size_t>(a-range.startSample)*2,
                    static_cast<size_t>(b-a)*2,stereo.data()+static_cast<size_t>(a-start)*2);
    }
    error.clear();return true;
}
}
