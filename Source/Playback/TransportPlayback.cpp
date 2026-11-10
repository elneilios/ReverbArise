#include "TransportPlayback.h"
#include <set>

namespace m3
{
uint64_t LivePublication::bytes() const noexcept
{
    uint64_t total=sizeof(*this)+entries.capacity()*sizeof(LiveEntry)+segments.capacity()*sizeof(LiveSegment);
    for(const auto& e:entries)if(e.audio)total+=e.audio->samples.capacity()*sizeof(float);
    for(const auto& e:entries)if(e.loopAudio)total+=e.loopAudio->samples.capacity()*sizeof(float);
    return total;
}
bool prepareLiveSegments(LivePublication& pub,juce::String& error)
{
    struct Edge{int64_t at;size_t entry;bool start;};std::vector<Edge> edges;
    if(pub.end<=pub.start || pub.entries.size()>maximumSwells){error="Invalid live window/capacity";return false;}
    std::set<size_t> identities;
    for(size_t i=0;i<pub.entries.size();++i){const auto& e=pub.entries[i];
        if(e.slot>=maximumSwells || e.end<=e.start || !std::isfinite(e.peak) || e.peak<0){error="Invalid live member";return false;}
        if(!identities.insert(e.slot).second){error="Duplicate live member identity";return false;}
        const auto a=e.start,b=e.end;
        if(a<b){edges.push_back({a,i,true});edges.push_back({b,i,false});}}
    std::sort(edges.begin(),edges.end(),[](const auto& a,const auto& b){return a.at<b.at || (a.at==b.at && a.start<b.start);});
    const auto coverageStart=pub.loopEnd>pub.loopStart?std::min(pub.start,pub.loopStart):pub.start;
    const auto coverageEnd=pub.loopEnd>pub.loopStart?std::max(pub.end,pub.loopEnd):pub.end;
    pub.segments.clear();std::set<size_t> members;double peak=0,bound=0;int64_t at=edges.empty()?coverageStart:std::min(coverageStart,edges.front().at);size_t cursor=0;
    const auto finish=edges.empty()?coverageEnd:std::max(coverageEnd,edges.back().at);
    while(at<finish){
        while(cursor<edges.size() && edges[cursor].at==at){const auto& edge=edges[cursor++];
            if(edge.start){members.insert(edge.entry);peak+=pub.entries[edge.entry].peak;}
            else {members.erase(edge.entry);peak-=pub.entries[edge.entry].peak;}}
        if(members.size()>liveMaximumOverlap){error="Live playback unavailable: more than 64 simultaneous swells";return false;}
        bound=std::max(bound,peak);const auto end=cursor<edges.size()?edges[cursor].at:finish;
        if(end<=at){error="Invalid live edge ordering";return false;}
        LiveSegment s;s.start=std::max(at,coverageStart);s.end=std::min(end,coverageEnd);s.count=members.size();size_t n=0;
        for(auto id:members)s.members[n++]=static_cast<uint16_t>(id);
        if(s.start<s.end)pub.segments.push_back(s);at=end;
    }
    if(!pub.measuredAttenuation)
        pub.attenuation=bound>0?static_cast<float>(std::min(1.0,std::pow(10.0,-6.0/20)/bound)):1;
    else if(!std::isfinite(pub.attenuation) || pub.attenuation<0 || pub.attenuation>1)
    {error="Invalid measured live attenuation";return false;}
    error.clear();return true;
}
void TransportPlayback::configure(bool on,LiveRouting mode,float db) noexcept
{enabled.store(on);wetOnly.store(mode==LiveRouting::wetOnly);level.store(std::isfinite(db)?std::pow(10.0f,std::clamp(db,-60.0f,12.0f)/20):0);}
void TransportPlayback::collect()
{for(auto& s:slots)if(s.state.load(std::memory_order_acquire)==retired){s.data.reset();s.state.store(free,std::memory_order_release);}}
uint64_t TransportPlayback::pinnedBytes() const noexcept
{uint64_t bytes=0;for(const auto& s:slots)if(s.data)bytes+=s.data->bytes();return bytes;}
bool TransportPlayback::publish(std::unique_ptr<LivePublication> pub)
{
    collect();for(size_t i=0;i<slots.size();++i)if(slots[i].state.load(std::memory_order_acquire)==free){
        slots[i].data=std::move(pub);slots[i].state.store(ready,std::memory_order_release);
        const auto old=pending.exchange(static_cast<int>(i),std::memory_order_acq_rel);
        if(old>=0)slots[old].state.store(retired,std::memory_order_release);
        return true;}return false;
}
void TransportPlayback::observe(const LivePublication* pub,int count,const TransportTime& time,
    bool workflowAllows,bool on,bool validCount,bool running,bool missing,LiveGate mappingGate,const LiveClockSnapshot* clock) noexcept
{
    LiveCallbackObservation d;d.block=++observedBlocks;d.observed=time.song;d.count=count;
    d.hasPublication=pub!=nullptr;d.playing=time.playing;d.workflowAllows=workflowAllows;
    d.hostLooping=time.looping;d.hostLoopStart=time.loopStart;d.hostLoopEnd=time.loopEnd;
    d.enabled=on;d.wetOnly=wetOnly.load();d.wetGain=level.load();
    if(clock){d.usesLiveClock=true;d.liveRun=clock->run;d.liveSession=clock->session;d.liveReset=clock->reset;d.liveReference=clock->reference;d.clockGate=clock->gate;}
    d.validCount=validCount;d.running=running;d.missing=missing;
    if(pub){d.published=pub->mapping;d.revision=pub->revision;d.start=pub->start;d.end=pub->end;
        d.loopStart=pub->loopStart;d.loopEnd=pub->loopEnd;
        long double a,b;d.originsValid=songOrigin(pub->mapping,a) && songOrigin(time.song,b);
        if(d.originsValid){d.publishedOrigin=static_cast<double>(a);d.observedOrigin=static_cast<double>(b);d.originDelta=static_cast<double>(b-a);}}
    d.gate=!workflowAllows?LiveGate::workflow:!on?LiveGate::disabled:!time.playing?LiveGate::stopped:
        !pub?LiveGate::noPublication:!validCount?LiveGate::invalidRange:mappingGate;
    if(d.gate==LiveGate::admitted)d.gate=!running?LiveGate::reset:missing?LiveGate::missingCoverage:LiveGate::admitted;
    callbackObservation.publish(d);
    if(clock)causalPublication.publish(causalHistory.observe(d));
    if(workflowAllows && on && time.playing && d.gate!=LiveGate::admitted)rejectionObservation.publish(d);
}
void TransportPlayback::process(float* left,float* right,int count,const TransportTime& time,bool workflowAllows,
                                const LiveClockSnapshot* clock,bool protectLiveOutput) noexcept
{
    if(count<=0)return;
    const auto next=pending.exchange(-1,std::memory_order_acq_rel);bool publicationChanged=false;
    if(next>=0){
        // Remove every old raw view before retiring its publication.
        for(auto& v:voices)v.entry=nullptr;
        if(current>=0)slots[current].state.store(retired,std::memory_order_release);
        current=next;slots[current].state.store(activeSlot,std::memory_order_release);publicationChanged=true;
    }
    const auto* pub=current>=0?slots[current].data.get():nullptr;
    const auto newEpoch=epoch.load();const auto resetRequested=newEpoch!=seenEpoch;seenEpoch=newEpoch;
    const auto mappingGate=!pub?LiveGate::noPublication:clock?clock->accepts(pub->mapping,time.song):liveMappingGate(pub->mapping,time.song);
    const auto mapping=mappingGate==LiveGate::admitted;
    int64_t end;
    const auto validCount=count<=1048576 && checkedAdd(time.song.sample,count,end);
    const auto on=enabled.load();
    const auto running=workflowAllows && on && time.playing && mapping && validCount && !resetRequested;
    const auto discontinuity=!wasRunning || !running || expectedPosition!=time.song.sample || resetRequested;
    const auto wetOnlyRoute=wetOnly.load();
    const auto guardMix=protectLiveOutput && !wetOnlyRoute;
    if(guardMix){
        outputGuard.prepare(time.song.rate);
        const auto wraps=time.looping && time.loopEnd>time.loopStart
            && expectedPosition==time.loopEnd && time.song.sample==time.loopStart;
        if(running && discontinuity && (!wasRunning || !wraps))outputGuard.reset();
    }else outputGuard.reset();
    const auto ramp=static_cast<int>(std::clamp(std::round(std::isfinite(time.song.rate)?time.song.rate*.005:1),1.0,9600.0));
    for(auto& v:voices)if(v.remaining>ramp){v.remaining=ramp;v.fromL=v.lastL;v.fromR=v.lastR;}
    const auto wantedDry=workflowAllows && on && time.playing && wetOnlyRoute?0.0f:1.0f;
    if(wantedDry!=dryTarget){dryTarget=wantedDry;dryRemaining=ramp;dryStep=(dryTarget-dry)/ramp;}
    const auto wantedGain=level.load();if(wantedGain!=gainTarget){gainTarget=wantedGain;gainRemaining=ramp;gainStep=(gainTarget-gain)/ramp;}
    badMapping.store(workflowAllows && enabled.load() && time.playing && pub && (!mapping || !validCount));
    bool anyMissing=workflowAllows && enabled.load() && time.playing && !pub;int segmentIndex=-1;
    if(!running && dryRemaining==0 && gainRemaining==0 && std::all_of(voices.begin(),voices.end(),[](const auto& v){return v.remaining==0 && v.lastL==0 && v.lastR==0;})){
        outputGuard.reset();
        if(dry==0){std::fill_n(left,count,0.0f);std::fill_n(right,count,0.0f);}
        for(auto& v:voices){v.audible=false;v.entry=nullptr;}
        missed=anyMissing;liveActive=false;wasRunning=false;expectedPosition=validCount?end:time.song.sample;
        observe(pub,count,time,workflowAllows,on,validCount,running,anyMissing,mappingGate,clock);return;
    }
    for(int frame=0;frame<count;++frame){
        const auto p=validCount?time.song.sample+frame:0;
        const LiveSegment* segment=nullptr;
        if(running && ((p>=pub->start && p<pub->end) || (p>=pub->loopStart && p<pub->loopEnd))){
            if(segmentIndex<0 || p>=pub->segments[static_cast<size_t>(segmentIndex)].end){
                const auto it=std::upper_bound(pub->segments.begin(),pub->segments.end(),p,[](int64_t at,const LiveSegment& s){return at<s.start;});
                segmentIndex=it==pub->segments.begin()?-1:static_cast<int>(it-pub->segments.begin()-1);
            }
            if(segmentIndex>=0)segment=&pub->segments[static_cast<size_t>(segmentIndex)];
        }else if(workflowAllows && enabled.load() && time.playing && mapping)anyMissing=true;
        std::array<const LiveEntry*,liveMaximumOverlap> wanted{};
        if(segment){for(size_t n=0;n<segment->count;++n){const auto& e=pub->entries[segment->members[n]];
            if(e.generation!=generations[e.slot].load(std::memory_order_acquire)){anyMissing=true;continue;}
            const auto covered=[&](const auto& audio){return audio && p>=audio->start && static_cast<uint64_t>(p)-static_cast<uint64_t>(audio->start)<audio->frames();};
            if(!covered(e.audio) && !covered(e.loopAudio)){anyMissing=true;continue;}
            wanted[n]=&e;}}
        // Preserve each member's channel across publication and segment changes.
        std::array<bool,liveMaximumOverlap> assigned{};
        for(const auto* e:wanted)if(e && channels[e->slot]>=0)assigned[static_cast<size_t>(channels[e->slot])]=true;
        for(const auto* e:wanted)if(e){size_t channel=channels[e->slot]>=0?static_cast<size_t>(channels[e->slot]):liveMaximumOverlap;
            if(channel==liveMaximumOverlap)for(size_t n=0;n<voices.size();++n)if(!assigned[n]){channel=n;break;}
            if(channel<voices.size()){auto& v=voices[channel];assigned[channel]=true;
                const auto changed=v.slot!=e->slot || v.generation!=e->generation || v.version!=e->version || v.attenuation!=pub->attenuation || !v.audible || (frame==0 && discontinuity);
                if(changed && v.remaining==0){v.fromL=v.lastL;v.fromR=v.lastR;v.remaining=ramp;}
                if(v.slot<maximumSwells && v.slot!=e->slot)channels[v.slot]=-1;
                v.slot=e->slot;channels[e->slot]=static_cast<int>(channel);v.generation=e->generation;v.version=e->version;v.entry=e;v.attenuation=pub->attenuation;v.audible=true;}}
        float l=0,r=0;
        bool wetPresent=false;
        for(size_t n=0;n<voices.size();++n){auto& v=voices[n];float a=0,b=0;
            if(!assigned[n]){if(v.audible && v.remaining==0){v.fromL=v.lastL;v.fromR=v.lastR;v.remaining=ramp;}v.audible=false;v.entry=nullptr;}
            if(v.entry){const auto* audio=v.entry->audio.get();if(!audio || p<audio->start || static_cast<uint64_t>(p-audio->start)>=audio->frames())audio=v.entry->loopAudio.get();
                const auto at=static_cast<size_t>(p-audio->start)*2;a=audio->samples[at]*pub->attenuation;b=audio->samples[at+1]*pub->attenuation;}
            if(v.remaining>0){const auto blend=static_cast<float>(v.remaining-1)/ramp;a=a*(1-blend)+v.fromL*blend;b=b*(1-blend)+v.fromR*blend;--v.remaining;}
            wetPresent |= v.entry || a != 0.0f || b != 0.0f;
            v.lastL=a;v.lastR=b;l+=a;r+=b;
        }
        if(gainRemaining>0){gain+=gainStep;if(--gainRemaining==0)gain=gainTarget;}
        if(dryRemaining>0){dry+=dryStep;if(--dryRemaining==0)dry=dryTarget;}
        const auto wetLeft=l*gain,wetRight=r*gain;
        left[frame]=left[frame]*dry+wetLeft;right[frame]=right[frame]*dry+wetRight;
        if(guardMix)outputGuard.protectMix(left[frame],right[frame],wetLeft,wetRight,wetPresent);
        else outputGuard.reset();
    }
    observe(pub,count,time,workflowAllows,on,validCount,running,anyMissing,mappingGate,clock);
    (void)publicationChanged;missed.store(anyMissing);liveActive.store(running);wasRunning=running;
    expectedPosition=validCount?end:time.song.sample;
}
}
