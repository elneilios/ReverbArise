#include "SwellEngine.h"
#include "AuditionMix.h"
#include <Core/MonitoringSafety.h>
namespace m3
{
bool SwellEngine::setAuditionWetOnly(bool wetOnly,uint64_t expectedSession,uint64_t expectedPass)
{
    const juce::ScopedLock guard(lock);
    if(expectedSession!=sessionGeneration.load())return false;
    if(workflowState){const auto flow=workflowState->snapshot();if(flow.mode!=workflow::Mode::live || flow.locked() || (expectedPass!=UINT64_MAX && flow.pass!=expectedPass))return false;}
    const auto mode=wetOnly?AuditionMode::wetOnly:AuditionMode::dryWet;
    if(model.auditionFollowsLive && model.auditionMode==mode)return true;
    model.auditionFollowsLive=true;model.auditionMode=mode;voice.setLevel(1);
    persistentChangedLocked();++monitoringGeneration;notify();return true;
}
void SwellEngine::setPostRoll(double seconds)
{
    const juce::ScopedLock guard(lock);
    if(!std::isfinite(seconds) || seconds<0 || seconds>5){message="Audition tail must be 0 to 5 seconds";return;}
    if(model.postRollSeconds==seconds)return;model.postRollSeconds=seconds;persistentChangedLocked();++monitoringGeneration;notify();
}
void SwellEngine::setMonitoring(AuditionMode mode,float blend)
{
    const juce::ScopedLock guard(lock);
    if((mode!=AuditionMode::wetOnly && mode!=AuditionMode::dryWet) || !std::isfinite(blend) || blend<0 || blend>1)
    {message="Invalid audition mode or percentage";return;}
    if(!model.auditionFollowsLive && model.auditionMode==mode && model.wetBlend==blend)return;
    model.auditionFollowsLive=false;model.auditionMode=mode;model.wetBlend=blend;voice.setLevel(juce::Decibels::decibelsToGain(model.auditionDb));persistentChangedLocked();++monitoringGeneration;notify();
}
void SwellEngine::setIsolateSelected(bool enabled)
{
    const juce::ScopedLock guard(lock);if(model.isolateSelected==enabled)return;
    model.isolateSelected=enabled;persistentChangedLocked();++monitoringGeneration;notify();
}
void SwellEngine::setIsolateDry(bool enabled)
{
    const juce::ScopedLock guard(lock);if(model.isolateDry==enabled)return;
    model.isolateDry=enabled;persistentChangedLocked();++monitoringGeneration;notify();
}
namespace
{
bool memberBounds(const Swell& w,double rate,int64_t& a,int64_t& b)
{
    int64_t rawStart=w.sourceStart;
    if(w.render.present())rawStart=w.render.start;
    else if(w.ir.present())
    {
        const auto frames=std::round(w.ir.frames*rate/w.ir.rate);
        if(!std::isfinite(frames) || frames<1 || frames>=static_cast<double>(INT64_MAX)
            || !checkedSubtract(w.sourceStart,static_cast<int64_t>(frames)-1,rawStart))return false;
    }
    return requestedShapeBounds(w,rawStart,a,b);
}
}
void SwellEngine::monitorJob(Swell requested,uint64_t session,uint64_t settings)
{
    Session saved;std::shared_ptr<WetRender> anchor;
    {const juce::ScopedLock guard(lock);saved=model;anchor=runtime[requested.id].shaped;runtime[requested.id].monitor.reset();}
    if(!anchor)return;
    const auto mode=saved.auditionMode;const auto context=contextProvider();
    const auto cancel=[&]{const juce::ScopedLock guard(lock);return threadShouldExit() || session!=sessionGeneration.load()
        || model.primary!=requested.id || settings!=monitoringGeneration;};
    int64_t wetEnd,dryEnd=requested.sourceEnd,contextEnd=requested.sourceEnd,windowStart=anchor->start,windowEnd;
    juce::String error,dryError;bool dryReady=false;
    if(!checkedAdd(anchor->start,static_cast<int64_t>(anchor->frames()),wetEnd))return;
    windowEnd=wetEnd;
    const auto validContext=auditionContextEnd(requested,anchor->rate,saved.postRollSeconds,contextEnd);
    if (context.capture.manifest.loop) contextEnd=std::min(contextEnd,context.capture.manifest.loop->endSample);
    if(!saved.isolateDry)dryEnd=contextEnd;
    const auto needed=validContext?"Dry source not captured for this span; need samples ["+juce::String(requested.sourceStart)+", "+juce::String(dryEnd)+")":juce::String("Audition context overflow: reduce endpoint or listening tail");
    if(mode==AuditionMode::dryWet){windowStart=std::min(windowStart,requested.sourceStart);windowEnd=std::max(windowEnd,contextEnd);if(!validContext)error=needed;}
    CaptureAssetStore source(instanceRoot(context.capture.manifest));SwellAssetStore assets(instanceRoot(context.capture.manifest));
    bool anyRecovered=false;std::vector<Swell> members;std::vector<juce::String> ids;
    const auto solo=std::any_of(saved.swells.begin(),saved.swells.end(),[](const auto& w){return w.solo;});
    for(const auto& w:saved.swells)
    {
        if(!w.enabled || w.muted || (solo && !w.solo) || (saved.isolateSelected && w.id!=requested.id))continue;
        int64_t a,b;if(!memberBounds(w,anchor->rate,a,b)){error="Cannot establish wet bounds: "+w.name;break;}
        if(a>=windowEnd || b<=windowStart)continue;members.push_back(w);ids.push_back(w.id);
    }
    if(mode==AuditionMode::wetOnly && members.empty() && error.isEmpty())error="No audible swells in this window";
    int64_t frames;WetRender aggregate;
    aggregate.start=windowStart;aggregate.rate=anchor->rate;aggregate.sourceStart=requested.sourceStart;aggregate.sourceEnd=dryEnd;
    if(error.isEmpty() && (!checkedSubtract(windowEnd,windowStart,frames) || frames<=0 || static_cast<uint64_t>(frames)>available(64ULL*1024*1024)/8))error="256 MiB budget: audition window too large";
    if(error.isEmpty())aggregate.samples.resize(static_cast<size_t>(frames)*2);
    for(const auto& w:members)
    {
        if(cancel())return;if(error.isNotEmpty())break;
        bool recovered=false;{const juce::ScopedLock guard(lock);recovered=runtime[w.id].cachedWetOnly;}
        if(w.phase!=Phase::ready && !(mode==AuditionMode::wetOnly && recovered)){error="Unavailable swell: "+w.name+" - "+w.status;break;}
        if(w.render.rate!=anchor->rate || (!w.frozen && (w.render.irContent!=w.ir.content || !assets.verify(w.ir,dryError))))
        {error="Unavailable IR/rate: "+w.name+" - "+dryError;break;}
        WetRender raw,shaped;std::shared_ptr<WetRender> resident;
        {const juce::ScopedLock guard(lock);resident=runtime[w.id].raw;}
        const auto overhead=aggregate.samples.capacity()*sizeof(float);
        if(!resident && !assets.readRender(w.render,available(overhead),raw,dryError)){error="Unavailable render: "+w.name+" - "+dryError;break;}
        const auto& input=resident?*resident:raw;
        auto audible=w;if(saved.gainPolicy==GainPolicy::legacy)audible.calibration={};
        if(!prepareShape(audible,input,available(overhead+raw.samples.capacity()*sizeof(float)),shaped,dryError,cancel))
        {error="Unavailable shape: "+w.name+" - "+dryError;break;}
        int64_t end;if(!checkedAdd(shaped.start,static_cast<int64_t>(shaped.frames()),end)){error="Swell position overflow: "+w.name;break;}
        const auto a=std::max(windowStart,shaped.start),b=std::min(windowEnd,end);
        for(auto at=a;at<b;++at)
        {
            if((at-a)%4096==0 && cancel())return;
            const auto gain=monitorEdgeGain(static_cast<size_t>(at-shaped.start),shaped.frames(),shaped.rate,.002);
            for(int c=0;c<2;++c)aggregate.samples[static_cast<size_t>(at-windowStart)*2+c]+=shaped.samples[static_cast<size_t>(at-shaped.start)*2+c]*gain;
        }
        if(recovered)anyRecovered=true;
        if(recovered)dryError="Last-committed Wet Only recovery (source unavailable)";
    }
    SourceInterval dry;
    if(error.isEmpty() && mode==AuditionMode::dryWet)
    {
        const auto budget=available(aggregate.samples.capacity()*sizeof(float)+64ULL*1024*1024);
        dryReady=budget>0 && readSourceInterval(source,context.capture.manifest,requested.sourceStart,dryEnd,budget/8,dry,dryError,cancel);
        if(!dryReady)error=needed;
    }
    auto prepared=std::make_shared<WetRender>();float branch=0,sum=0;
    if(error.isEmpty() && !prepareAuditionMix(aggregate,dryReady?&dry:nullptr,mode,saved.wetBlend,
        available((aggregate.samples.capacity()+dry.samples.capacity())*sizeof(float)),*prepared,branch,sum,error,false,saved.gainPolicy,saved.auditionFollowsLive?std::optional<float>{saved.liveWetDb}:std::nullopt)){}
    const auto latest=contextProvider();const juce::ScopedLock guard(lock);if(cancel())return;
    bool changed=context.captureGeneration!=latest.captureGeneration || !sameMusicalMapping(context.time,latest.time);
    if(context.capture.manifest.loop)changed=changed || latest.capture.reference!=context.capture.reference;
    if(mode==AuditionMode::dryWet)changed=changed || sourceDependency(context.capture.manifest,requested.sourceStart,dryEnd)!=sourceDependency(latest.capture.manifest,requested.sourceStart,dryEnd);
    for(const auto& member:members)if(!member.frozen)changed=changed || sourceDependency(context.capture.manifest,member.sourceStart,renderInputEnd(member))!=sourceDependency(latest.capture.manifest,member.sourceStart,renderInputEnd(member));
    if(changed){stopLocked();return;}
    auto& r=runtime[requested.id];r.monitorRevision=juce::String(sourceDependency(latest.capture.manifest,requested.sourceStart,dryEnd));
    r.dryAvailable=dryReady;r.dryStatus=error.isNotEmpty()?error:anyRecovered?juce::String("Last-committed Wet Only recovery: source unavailable"):juce::String{};r.members=ids;r.wetAttenuation=branch;r.blendAttenuation=sum;r.preparedMonitor=settings;
    if(error.isEmpty()){r.monitor=std::move(prepared);if(voice.isActiveOrPending() && !voice.replace(r.monitor))message="Audition update queue full; retry";}
    else {r.monitor.reset();message=error;stopLocked();}
}
}
