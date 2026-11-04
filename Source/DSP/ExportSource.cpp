#include "ExportSource.h"
#include <algorithm>
#include <cmath>
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
    plan=std::move(input);readable.resize(plan->source.ranges.size(),false);
    if(plan->wet==1){error.clear();return true;}
    auto cursor=plan->origin;
    for(size_t i=0;i<plan->source.ranges.size();++i)
    {
        if(cancel && cancel()){error="Export source preparation cancelled";plan.reset();return false;}
        const auto& range=plan->source.ranges[i];const auto a=std::max(plan->origin,range.startSample);
        const auto b=std::min(plan->end,range.startSample+static_cast<int64_t>(range.frameCount));if(a>=b)continue;
        warn(cursor,a,ExportCoverage::gap);
        if(range.kind==CaptureCoverageKind::audio){
            if(range.frameCount>allowance/(4*sizeof(float))){error="Export dry chunk exceeds available working memory";plan.reset();return false;}
            juce::String failure;readable[i]=load(i,failure);if(!readable[i])warn(a,b,ExportCoverage::unavailable);
        }else readable[i]=true;
        cursor=b;
    }
    warn(cursor,plan->end,ExportCoverage::gap);error.clear();return true;
}
bool ExportSourceReader::read(int64_t start,std::span<float> stereo,juce::String& error)
{
    int64_t end;
    if(!plan || stereo.empty() || stereo.size()%2 || stereo.size()/2>exportPageFrames
        || !checkedAdd(start,static_cast<int64_t>(stereo.size()/2),end) || start<plan->origin || end>plan->end)
    {error="Invalid Export dry page";return false;}
    std::fill(stereo.begin(),stereo.end(),0.f);
    if(plan->wet==1){error.clear();return true;}
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
