#include "SwellEngine.h"
#include "GainStaging.h"
#include <Core/MonitoringSafety.h>

namespace m3
{
namespace
{
juce::String offlineTimingEvidence(const SongTime& prepared,const SongTime& reference,const SongTime& host)
{
    const auto describe=[](const SongTime& t){return "sample="+juce::String(t.sample)+" PPQ="+juce::String(t.ppq,15)
        +" Hz="+juce::String(t.rate,6)+" BPM="+juce::String(t.bpm,12)
        +" meter="+juce::String(t.numerator)+"/"+juce::String(t.denominator)+" valid="+juce::String(t.valid?1:0);};
    const auto delta=[&](const SongTime& from){long double a=0,b=0;return songOrigin(from,a) && songOrigin(host,b)
        ?juce::String(static_cast<double>(b-a),9):juce::String("unavailable");};
    return " | Prepared: "+describe(prepared)+" | Reference: "+describe(reference)+" | Host: "+describe(host)
        +" | source origin delta="+delta(prepared)+" samples | run origin delta="+delta(reference)+" samples";
}
juce::String memberKey(const Swell& w)
{return w.render.content+"/"+juce::String(w.inputGeneration)+"/"+juce::String(w.shapeGeneration)+"/"+juce::String(w.placement)
    +"/"+juce::String(w.arrival)+"/"+juce::String(static_cast<int>(w.shape.automaticStart))+"/"+juce::String(w.shape.startOffset)+"/"+juce::String(w.shape.endOffset)
    +"/"+juce::String(w.shape.fadeIn)+"/"+juce::String(w.shape.fadeOut)+"/"+juce::String(static_cast<int>(w.shape.inCurve))+"/"+juce::String(static_cast<int>(w.shape.outCurve))
    +"/"+juce::String(w.shape.inTension,17)+"/"+juce::String(w.shape.outTension,17)+"/"+juce::String(w.shape.gainDb,9)
    +"/"+juce::String(static_cast<int>(w.calibration.state))+"/"+juce::String(w.calibration.baselineDb,9)+"/"+w.calibration.revision
    +"/"+w.calibration.sourceIdentity+"/"+w.calibration.irIdentity+"/"+w.calibration.renderIdentity+"/"+w.render.path+"/"+w.render.checksum+"/"+w.ir.path;}
juce::String waveformKey(const Swell& w){auto unplaced=w;unplaced.placement=0;return memberKey(unplaced);}

bool readShaped(const Swell& w,const SwellAssetStore& store,int64_t from,int64_t to,
    WetRender* output,float& peak,juce::String& error,const RenderCancellation& cancel)
{
    int64_t a,b;if(!shapeBounds(w,w.render.start,w.render.sourceEnd,a,b,error))return false;
    from=std::max(from,a);to=std::min(to,b);if(from>=to)return true;
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(store.directory().getChildFile(w.render.path).createInputStream().release(),true));
    if(!reader || reader->numChannels!=2 || reader->sampleRate!=w.render.rate || reader->lengthInSamples!=w.render.frames)
    {error="Live render missing or header incompatible";return false;}
    if(output){output->start=from;output->rate=w.render.rate;output->samples.resize(static_cast<size_t>(to-from)*2);}
    juce::AudioBuffer<float> scratch(2,4096);
    for(int64_t at=from;at<to;){if(cancel && cancel()){error="Live preparation superseded";return false;}
        const auto count=static_cast<int>(std::min(int64_t{4096},to-at));
        const auto rawAt=at-w.placement-w.render.start;
        if(!reader->read(&scratch,0,count,rawAt,true,true)){error="Live render read failed";return false;}
        for(int n=0;n<count;++n){const auto position=at+n;
            const auto factor=swellEnvelope(w,position,a,b)*monitorEdgeGain(static_cast<size_t>(position-a),static_cast<size_t>(b-a),w.render.rate,.002);
            for(int c=0;c<2;++c){const auto value=static_cast<float>(scratch.getSample(c,n)*factor);
                if(!std::isfinite(value)){error="Non-finite live render";return false;}
                peak=std::max(peak,std::abs(value));if(output)output->samples[static_cast<size_t>(position-from)*2+c]=value;}}
        at+=count;
    }
    if(output)output->peak=peak;return true;
}

juce::String gainKey(const Session& session,const std::vector<Swell>& members)
{
    juce::String key=session.gainRevision+"/"+juce::String(session.liveWetDb,6);
    for(const auto& w:members)key+="|"+memberKey(w)+"/"+juce::String(static_cast<int>(w.enabled))+"/"+juce::String(static_cast<int>(w.muted))+"/"+juce::String(static_cast<int>(w.solo));
    return key;
}

bool prepareMusicalPlan(const Session& session,const std::vector<Swell>& members,const SwellAssetStore& store,
                        uint64_t budget,const RenderCancellation& cancel,float& attenuation,float& peak,juce::String& error)
{
    if(members.empty()){attenuation=1;peak=0;error.clear();return true;}
    struct Group{const Swell* swell;int count;int64_t start,end;};std::vector<Group> groups;
    std::map<juce::String,size_t> grouped;std::vector<std::pair<int64_t,int64_t>> intervals;
    for(const auto& w:members){int64_t a,b;if(!shapeBounds(w,w.render.start,w.render.sourceEnd,a,b,error))return false;
        const auto key=memberKey(w);const auto found=grouped.find(key);
        if(found!=grouped.end())++groups[found->second].count;
        else {grouped[key]=groups.size();groups.push_back({&w,1,a,b});intervals.emplace_back(a,b);}}
    std::sort(intervals.begin(),intervals.end());std::vector<std::pair<int64_t,int64_t>> occupied;
    for(const auto& range:intervals){if(!occupied.empty() && range.first<occupied.back().second)occupied.back().second=std::max(occupied.back().second,range.second);
        else occupied.push_back(range);}
    peak=0;attenuation=1;
    std::map<juce::String,float> isolatedPeaks;
    for(const auto& range:occupied){float measured=0,guard=1;
    const Group* isolated=nullptr;size_t count=0;for(const auto& g:groups)if(g.start<range.second && g.end>range.first){isolated=&g;++count;}
    if(count==1 && isolated->count==1){
        const auto key=waveformKey(*isolated->swell);auto cached=isolatedPeaks.find(key);float memberPeak=0;
        if(cached==isolatedPeaks.end()){
            if(!readShaped(*isolated->swell,store,range.first,range.second,nullptr,memberPeak,error,cancel))return false;
            isolatedPeaks[key]=memberPeak;
        }else memberPeak=cached->second;
        measured=memberPeak*juce::Decibels::decibelsToGain(session.liveWetDb);
        const auto ceiling=juce::Decibels::decibelsToGain(gainCeilingDb(session));guard=measured>ceiling?ceiling/measured:1;
        peak=std::max(peak,measured);attenuation=std::min(attenuation,guard);continue;
    }
    if(!measureAggregateGain(range.first,range.second,gainCeilingDb(session),session.liveWetDb,budget,
        [&](int64_t at,int frames,float* sum,juce::String& addError){
            int64_t end;if(!checkedAdd(at,frames,end)){addError="Aggregate interval overflow";return false;}
            for(const auto& group:groups){if(group.end<=at || group.start>=end)continue;WetRender part;float ignored=0;
                if(!readShaped(*group.swell,store,at,end,&part,ignored,addError,cancel))return false;
                for(size_t n=0;n<part.frames();++n){const auto offset=static_cast<size_t>(part.start-at+n)*2;
                    // Repeat additions preserve the production mixer's float
                    // accumulation for identical voices, while reading once.
                    for(int copy=0;copy<group.count;++copy){sum[offset]+=part.samples[n*2];sum[offset+1]+=part.samples[n*2+1];}}}
            return true;
        },cancel,guard,measured,error))return false;
        peak=std::max(peak,measured);attenuation=std::min(attenuation,guard);
    }
    return true;
}
}
void SwellEngine::invalidateLiveLocked(const juce::String& id)
{
    if(id.isEmpty())retainedIR.clear();else retainedIR.erase(id);
    ++liveRevision;
    loopPlayback.invalidatePrepared();
    if(id.isEmpty()){for(const auto& [key,slot]:liveSlots)live.invalidate(slot,++liveGenerations[slot]);}
    else if(const auto at=liveSlots.find(id);at!=liveSlots.end())live.invalidate(at->second,++liveGenerations[at->second]);
    notify();
}
void SwellEngine::setLivePlayback(bool enabled,LiveRouting routing,float db)
{
    const juce::ScopedLock guard(lock);
    if(!std::isfinite(db) || db< -60 || db>(model.gainPolicy==GainPolicy::musical?12:0) || static_cast<int>(routing)<0 || static_cast<int>(routing)>1)return;
    const auto gainChanged=model.liveWetDb!=db;
    if(model.liveRouting!=routing || gainChanged){loopPlayback.invalidatePrepared();preparedLoopRender=0;}
    if(model.liveEnabled!=enabled || model.liveRouting!=routing || gainChanged)persistentChangedLocked();
    model.liveEnabled=enabled;model.liveRouting=routing;model.liveWetDb=db;
    if(gainChanged && model.auditionFollowsLive)++monitoringGeneration;
    liveEnabled=enabled;live.configure(enabled,routing,db);
    if(gainChanged && model.gainPolicy==GainPolicy::musical)invalidateLiveLocked();else notify();
}
bool SwellEngine::retainedCompatible(const RetainedIR& kept,const Swell& w,const PreviewContext& context,const LiveClockSnapshot* liveContext) const
{
    const auto& a=kept.accepted;
    return w.enabled && !w.frozen && w.sourceStart==a.sourceStart && w.sourceEnd==a.sourceEnd
        && w.arrival==a.arrival && w.shape==a.shape && w.placement==a.placement
        && (liveContext?liveContext->accepts(kept.timing,context.time)==LiveGate::admitted:transportMappingMatches(kept.timing,context.time))
        && kept.revision==juce::String(sourceDependency(context.capture.manifest,a.sourceStart,renderInputEnd(a)));
}
bool SwellEngine::retainIRLocked(const Swell& w,const PreviewContext& context,const juce::String& requestedName)
{
    const auto clock=liveClock.snapshot(sessionGeneration.load());
    const auto useLive=context.capture.sourceKind()!=CaptureSourceKind::loop && !offlineActive.load()
        && (!workflowState || workflowState->snapshot().mode==workflow::Mode::live)
        && clock && clock->playing && clock->accepts(model.timing,context.time)==LiveGate::admitted;
    const auto* liveContext=useLive?&*clock:nullptr;
    if(auto found=retainedIR.find(w.id);found!=retainedIR.end()){
        if(retainedCompatible(found->second,w,context,liveContext)){found->second.requestedName=requestedName;return true;}
        retainedIR.erase(found);}
    const auto r=runtime.find(w.id);int64_t a,b;juce::String error;
    if(w.phase!=Phase::ready || w.frozen || !w.enabled || r==runtime.end() || r->second.cachedWetOnly
        || r->second.verifiedRender!=w.render || !shapeBounds(w,w.render.start,w.render.sourceEnd,a,b,error))return false;
    const auto revision=juce::String(sourceDependency(context.capture.manifest,w.sourceStart,renderInputEnd(w)));
    if(r->second.revision!=revision)return false; // A commit may precede the worker's next inspect pass.
    RetainedIR kept{w,model.timing,revision,requestedName};
    if(!retainedCompatible(kept,w,context,liveContext))return false;
    retainedIR.emplace(w.id,std::move(kept));return true;
}
void SwellEngine::setRegenerationSensitivity(float value)
{
    if(!std::isfinite(value) || value<0 || value>1)return;
    const juce::ScopedLock guard(lock);if(model.regenerationSensitivity!=value)persistentChangedLocked();model.regenerationSensitivity=value;regeneration=value;
}
bool SwellEngine::setFrozen(const juce::String& id,bool frozen)
{
    if(!permitCreative())return false;
    const juce::ScopedLock guard(lock);auto w=find(model,id);if(!w)return false;
    if(w->frozen==frozen)return true;
    if(frozen && (w->phase!=Phase::ready || !w->render.present())){message="Wait for a ready render before freezing";return false;}
    w->frozen=frozen;w->provenance=Provenance::manualEdited;persistentChangedLocked();
    if(frozen){w->status="Frozen render";forceRefresh.erase(id);}
    else {++w->inputGeneration;w->phase=Phase::checking;w->status="Checking unfrozen source";forceRefresh.insert(id);invalidateLiveLocked(id);}
    ++monitoringGeneration;notify();return true;
}
void SwellEngine::processOfflineAudio(float* left,float* right,int frames,const TransportTime& hostTime)
{
    if(workflowState && workflowState->snapshot().mode!=workflow::Mode::live){endOffline();return;}
    if(frames<=0)return;
    offlineBlockCount.fetch_add(1,std::memory_order_relaxed);
    const auto starting=!offlineActive.exchange(true);
    // Offline callbacks need not set Playing. Pin their own clock separately
    // from parked/source PPQ, while preserving the accepted sample schedule.
    LiveClockSnapshot entryClock;entryClock.reference=entryClock.observed=hostTime.song;
    entryClock.playing=true;entryClock.gate=LiveGate::admitted;
    // Capture accepted IR-only fallbacks before waiting for an in-flight worker
    // render. Its completion must not silently change this export's composition.
    std::map<juce::String,Swell> acceptedAtEntry;juce::StringArray pendingAtEntry;
    if(starting){const auto context=contextProvider();const juce::ScopedLock guard(lock);
        for(const auto& [id,kept]:retainedIR)if(const auto* current=find(model,id);current && retainedCompatible(kept,*current,context,&entryClock)){
            auto audible=kept.accepted;audible.phase=Phase::ready;audible.enabled=current->enabled;audible.muted=current->muted;audible.solo=current->solo;
            acceptedAtEntry.emplace(id,std::move(audible));pendingAtEntry.add(current->name+": "+kept.accepted.irName+" (requested "+kept.requestedName+")");}}
    const juce::ScopedLock work(workLock);
    if(starting){offlineAccepted=std::move(acceptedAtEntry);offlineIRNotice=pendingAtEntry.isEmpty()?juce::String{}:"Export uses accepted IRs; replacements pending | "+pendingAtEntry.joinIntoString("; ");}
    const auto fail=[&](const juce::String& error){const juce::ScopedLock guard(lock);
        offlineError="Export incomplete: "+error;message=offlineError;offlineStatus=offlineError;
        if(offlineSession.liveEnabled && offlineSession.liveRouting==LiveRouting::wetOnly){std::fill_n(left,frames,0.0f);std::fill_n(right,frames,0.0f);}};
    try{
        if(starting || hostTime.song.sample<offlineLastPosition){const juce::ScopedLock guard(lock);
            offlinePlayer.reset();offlineEntries.clear();offlineMembers.clear();offlineError.clear();offlineWarning.clear();offlineStatus="Preparing offline export";offlineWindowStart=offlineWindowEnd=0;offlineSession=model;offlineClock=entryClock;}
        offlineLastPosition=hostTime.song.sample;
        if(offlineError.isNotEmpty()){if(offlineSession.liveEnabled && offlineSession.liveRouting==LiveRouting::wetOnly){std::fill_n(left,frames,0.0f);std::fill_n(right,frames,0.0f);}return;}
        if(!offlinePlayer){
            offlineEntries.clear();offlineMembers.clear();
            const auto context=contextProvider();long double origin;
            if(!songOrigin(hostTime.song,origin)){fail("valid absolute Song timing is required"+offlineTimingEvidence(offlineSession.timing,offlineClock.reference,hostTime.song));return;}
            if(!storageReady(context)){fail("asset working folder unavailable or still loading");return;}
            collectCompletedPass(context);inspect(context);prepareDefault(context);reconcileRenders(context);
            Session pending;uint64_t generation;
            {const juce::ScopedLock guard(lock);pending=model;generation=sessionGeneration.load();}
            const auto solo=std::any_of(pending.swells.begin(),pending.swells.end(),[](const auto& w){return w.solo;});
            if(pending.liveEnabled)for(const auto& w:pending.swells)if(w.enabled && !w.muted && (!solo || w.solo) && !offlineAccepted.contains(w.id))renderJob(w,context,generation,true);
            {const juce::ScopedLock guard(lock);offlineSession=model;offlineRoot=instanceRoot(context.capture.manifest);
                for(const auto& [id,accepted]:offlineAccepted)if(auto target=find(offlineSession,id))*target=accepted;}
            if(offlineSession.gainPolicy==GainPolicy::legacy)for(auto& w:offlineSession.swells)w.calibration={};
            if(offlineSession.liveEnabled && offlineClock.accepts(offlineSession.timing,hostTime.song)!=LiveGate::admitted && !offlineSession.swells.empty()){
                fail("prepared assets have incompatible timing"+offlineTimingEvidence(offlineSession.timing,offlineClock.reference,hostTime.song));return;}
            SwellAssetStore store(offlineRoot);juce::String error;juce::StringArray skipped;
            const auto skip=[&](const Swell& w,const juce::String& reason){if(skipped.size()<8)skipped.add(w.name+": "+reason);};int skippedCount=0;
            if(offlineSession.liveEnabled)for(const auto& w:offlineSession.swells)if(w.enabled && !w.muted && (!solo || w.solo)){
                if(w.phase!=Phase::ready){++skippedCount;skip(w,w.status);continue;}
                int64_t a,b;float peak=0;
                if(!shapeBounds(w,w.render.start,w.render.sourceEnd,a,b,error) || (!w.frozen && !store.verify(w.ir,error)) || !store.verify(w.render,error)
                    || !readShaped(w,store,a,b,nullptr,peak,error,{})){++skippedCount;skip(w,error);continue;}
                offlineEntries.push_back({offlineEntries.size(),1,a,b,peak,{},{}});offlineMembers.push_back(w);
            }
            offlineGainAttenuation=1;offlineGainPeak=0;
            if(offlineSession.gainPolicy==GainPolicy::musical){
                const auto key=gainKey(offlineSession,offlineMembers);bool cached=false;const auto started=juce::Time::getMillisecondCounterHiRes();
                {const juce::ScopedLock guard(lock);cached=key==gainPlanKey;if(cached){offlineGainAttenuation=gainPlanAttenuation;offlineGainPeak=gainPlanPeak;++gainPlanCacheHits;}}
                if(!cached && !prepareMusicalPlan(offlineSession,offlineMembers,store,available(),{},offlineGainAttenuation,offlineGainPeak,error))
                {fail(error);return;}
                if(!cached){const juce::ScopedLock guard(lock);gainPlanKey=key;gainPlanAttenuation=offlineGainAttenuation;gainPlanPeak=offlineGainPeak;
                    gainPlanPreparationMs=juce::Time::getMillisecondCounterHiRes()-started;++gainPlanBuilds;}
            }
            {const juce::ScopedLock guard(lock);
                if(skippedCount)offlineWarning="Export warning: "+juce::String(skippedCount)+(skippedCount==1?" swell skipped | ":" swells skipped | ")+skipped.joinIntoString("; ");
                if(offlineIRNotice.isNotEmpty())offlineWarning=offlineWarning.isEmpty()?offlineIRNotice:offlineWarning+" | "+offlineIRNotice;
                offlinePlayer=std::make_unique<TransportPlayback>();
                offlinePlayer->configure(offlineSession.liveEnabled,offlineSession.liveRouting,offlineSession.liveWetDb);
                offlinePlayer->primeForOffline();
                for(const auto& entry:offlineEntries)offlinePlayer->invalidate(entry.slot,1);}
        }
        if(offlineClock.accepts(offlineSession.timing,hostTime.song)!=LiveGate::admitted && !offlineEntries.empty()){
            int64_t lastWetEnd=INT64_MIN;for(const auto& entry:offlineEntries)lastWetEnd=std::max(lastWetEnd,entry.end);
            // Hosts may advance absolute samples through an offline dry/tail
            // while freezing/resetting PPQ after playback has stopped. There
            // is no remaining wet to remap once every admitted interval ended.
            if(hostTime.song.sample>=lastWetEnd && sameMusicalMapping(offlineSession.timing,hostTime.song)){
                if(offlineSession.liveEnabled && offlineSession.liveRouting==LiveRouting::wetOnly){std::fill_n(left,frames,0.0f);std::fill_n(right,frames,0.0f);}
                const juce::ScopedLock guard(lock);
                liveStatus=offlineWarning.isNotEmpty()?offlineWarning:juce::String("Offline export: wet complete | host tail timing reset");offlineStatus=liveStatus;
                return;
            }
            const auto expectedPpq=offlineClock.reference.ppq+(static_cast<long double>(hostTime.song.sample)-offlineClock.reference.sample)
                *offlineClock.reference.bpm/(60*offlineClock.reference.rate);
            fail("Song mapping changed during export | sample="+juce::String(hostTime.song.sample)
                +" PPQ="+juce::String(hostTime.song.ppq,6)+" expected="+juce::String(static_cast<double>(expectedPpq),6)
                +" last wet end="+juce::String(lastWetEnd)+offlineTimingEvidence(offlineSession.timing,offlineClock.reference,hostTime.song));return;
        }
        auto time=hostTime;time.playing=true;
        SwellAssetStore store(offlineRoot);juce::String error;
        // The host may call much faster than wall clock. Prepare synchronously
        // on this explicitly non-real-time path; never wait in the live mixer.
        for(int offset=0;offset<frames;){const auto count=std::min(512,frames-offset);
            int64_t at,end;if(!checkedAdd(hostTime.song.sample,offset,at) || !checkedAdd(at,count,end)){fail("sample interval overflow");return;}
            if(at<offlineWindowStart || end>offlineWindowEnd){
                {const juce::ScopedLock guard(lock);offlinePlayer->collect();}
                int length=16384;std::unique_ptr<LivePublication> pub;
                while(true){pub=std::make_unique<LivePublication>();pub->mapping=offlineMembers.empty()?hostTime.song:offlineSession.timing;pub->start=at;
                    if(!checkedAdd(at,length,pub->end)){fail("page interval overflow");return;}
                    pub->entries=offlineEntries;uint64_t required=8ULL*1024*1024;
                    for(const auto& entry:pub->entries){const auto a=std::max(at,entry.start),b=std::min(pub->end,entry.end);if(a<b)required+=static_cast<uint64_t>(b-a)*8;}
                    if(required<=available())break;
                    if(length<=count){fail("256 MiB budget cannot admit required offline pages");return;}length=std::max(count,length/2);}
                uint64_t reserved=8ULL*1024*1024;
                if(offlineSession.gainPolicy==GainPolicy::musical){pub->attenuation=offlineGainAttenuation;pub->measuredAttenuation=true;}
                size_t index=0;
                for(const auto& w:offlineMembers){
                    auto& entry=pub->entries[index++];const auto a=std::max(at,entry.start),b=std::min(pub->end,entry.end);if(a>=b)continue;
                    if(static_cast<uint64_t>(b-a)*8>available(reserved+65536)){fail("offline page budget exceeded");return;}
                    auto audio=std::make_shared<WetRender>();float peak=0;
                    if(!readShaped(w,store,a,b,audio.get(),peak,error,{})){fail(w.name+": "+error);return;}
                    reserved+=audio->samples.capacity()*sizeof(float);entry.audio=std::move(audio);}
                if(!prepareLiveSegments(*pub,error)){fail(error);return;}
                const auto pageStart=pub->start,pageEnd=pub->end;
                {const juce::ScopedLock guard(lock);if(!offlinePlayer->publish(std::move(pub))){fail("offline publication capacity");return;}}
                offlineWindowStart=pageStart;offlineWindowEnd=pageEnd;
            }
            time.song.sample=at;time.song.ppq=hostTime.song.ppq+offset*hostTime.song.bpm/(60*hostTime.song.rate);
            offlinePlayer->process(left+offset,right+offset,count,time,true,&offlineClock);
            if(offlinePlayer->missing() || offlinePlayer->incompatible()){fail("required prepared wet was unavailable");return;}
            offset+=count;
        }
        {const juce::ScopedLock guard(lock);liveStatus=offlineWarning.isNotEmpty()?offlineWarning:juce::String("Offline export: all available wet rendered");offlineStatus=liveStatus;}
    }catch(const std::exception& e){fail(e.what());}
}
void SwellEngine::prepareTransport(const PreviewContext& context)
{
    if (context.capture.sourceKind() == CaptureSourceKind::loop)
    {
        prepareLoopTransport(context);
        return;
    }
    LivePreparationObservation observation;
    {const juce::ScopedLock guard(lock);observation=livePreparation;
        if (preparedLoopSource)
        {
            loopPlayback.discardPending();
            preparedLoopGeneration = preparedLoopSource = preparedLoopRender = 0;
        }
        ++observation.service;observation.observed=context.time;observation.source=model.timing;
        observation.latest={};observation.clock.reset();observation.revision=liveRevision;observation.preparedRevision=preparedLiveRevision;
        observation.start=liveWindowStart;observation.end=liveWindowEnd;
        observation.loopStart=preparedLoopStart;observation.loopEnd=preparedLoopEnd;}
    observation.servicedAtMs=juce::Time::getMillisecondCounterHiRes();
    observation.result="Preparation interrupted";bool attempted=false;
    struct Recorder {std::function<void()> finish;~Recorder(){finish();}} recorder{[&]{
        const juce::ScopedLock guard(lock);
        if(attempted)observation.lastAttemptResult=observation.result;
        livePreparation=observation;}};
    if(!hostObserved.load(std::memory_order_acquire)){observation.result="No host observation";return;}
    if(!context.time.valid){observation.result="Host timing unavailable";return;}
    if(!liveEnabled.load()){observation.result="Live disabled";return;}
    const auto clock=liveClock.snapshot(sessionGeneration.load());observation.clock=clock;
    if(!clock){observation.result="Awaiting coherent live clock";return;}
    Session session;uint64_t revision,generation;std::set<juce::String> held,replacements;
    int64_t loopFrom=0,loopTo=0;
    for(int attempt=0;attempt<8;++attempt){const auto sequence=loopSequence.load(std::memory_order_acquire);if(sequence&1)continue;
        const auto on=loopHint.load();const auto a=loopHintStart.load(),b=loopHintEnd.load();
        if(sequence==loopSequence.load(std::memory_order_acquire)){if(on && b>a){loopFrom=a;loopTo=b;}break;}}
    {
        const juce::ScopedLock guard(lock);live.collect();
        if(clock->session!=sessionGeneration.load() || clock->accepts(model.timing,context.time)!=LiveGate::admitted){observation.result="Live clock or asset timing incompatible";return;}
        // Restore validates members serially. Avoid rebuilding an ever-growing
        // full-support gain plan after each member; publish once restoration
        // has settled (failed members are still explicitly unavailable).
        if(model.gainPolicy==GainPolicy::musical && std::any_of(model.swells.begin(),model.swells.end(),[](const auto& w){return w.phase==Phase::restoring;})){
            LivePublication admission;admission.start=0;admission.end=1;juce::String admissionError;
            const auto solo=std::any_of(model.swells.begin(),model.swells.end(),[](const auto& w){return w.solo;});
            for(const auto& w:model.swells)if(w.enabled && !w.muted && (!solo || w.solo) && w.render.present()){
                int64_t a,b;if(shapeBounds(w,w.render.start,w.render.sourceEnd,a,b,admissionError))
                    admission.entries.push_back({admission.entries.size(),0,a,b,0,{},{}});}
            if(!prepareLiveSegments(admission,admissionError))liveStatus=admissionError;
            observation.result="Waiting for restore validation";return;
        }
        // Some hosts wrap Song playback without advertising cycle points.
        // Keep the observed completed pass start warm as the next destination.
        if(loopTo<=loopFrom && !completedPasses.empty()){
            const auto& pass=completedPasses.back();
            if(context.time.sample>=pass.first && context.time.sample<=pass.second){loopFrom=pass.first;loopTo=pass.second;}}
        // Service substantially earlier while a long job is between FFT stages.
        // At 96 kHz this leaves ~0.75 s rather than 0.25 s of page headroom.
        const auto margin=activeJobBytes?static_cast<int64_t>(std::min(context.time.rate*2,96000.0)*.75)
            :static_cast<int64_t>(std::min(context.time.rate,48000.0)*.5);
        if(preparedLiveRevision==liveRevision && preparedLoopStart==loopFrom && preparedLoopEnd==loopTo && context.time.sample>=liveWindowStart && context.time.sample<liveWindowEnd-margin){observation.result="Prepared window retained";return;}
        session=model;revision=liveRevision;generation=sessionGeneration.load();liveStatus="Preparing live window";
        attempted=true;++observation.attempt;observation.attemptedAtMs=observation.servicedAtMs;
        observation.result="Preparation cancelled or dependencies changed";observation.loopStart=loopFrom;observation.loopEnd=loopTo;
        for(auto& w:session.swells)if(const auto kept=retainedIR.find(w.id);kept!=retainedIR.end()){
            if(!retainedCompatible(kept->second,w,context,&*clock)){invalidateLiveLocked(w.id);observation.result="Retained dependencies incompatible";return;}
            if(w.phase!=Phase::ready || (w.render==kept->second.accepted.render && w.ir.content!=kept->second.accepted.ir.content)){
                const auto enabled=w.enabled,muted=w.muted,solo=w.solo;w=kept->second.accepted;w.phase=Phase::ready;
                w.enabled=enabled;w.muted=muted;w.solo=solo;held.insert(w.id);
            }else replacements.insert(w.id);}
        for(auto it=liveSlots.begin();it!=liveSlots.end();){if(!find(model,it->first)){live.invalidate(it->second,++liveGenerations[it->second]);liveCache.erase(it->first);it=liveSlots.erase(it);}else ++it;}
        std::array<bool,maximumSwells> used{};for(const auto& [id,slot]:liveSlots)used[slot]=true;
        for(const auto& w:session.swells)if(!liveSlots.contains(w.id)){size_t slot=0;while(slot<used.size() && used[slot])++slot;
            if(slot==used.size()){liveStatus="Live identity capacity unavailable";observation.result=liveStatus;return;}used[slot]=true;liveSlots[w.id]=slot;live.invalidate(slot,++liveGenerations[slot]);}
    }
    if(session.gainPolicy==GainPolicy::legacy)for(auto& w:session.swells)w.calibration={};
    const auto cancel=[&]{const juce::ScopedLock guard(lock);return threadShouldExit() || generation!=sessionGeneration.load() || revision!=liveRevision;};
    auto pub=std::make_unique<LivePublication>();pub->mapping=session.timing;pub->revision=revision;
    // At most 2.25 seconds at 48 kHz; higher rates shorten time coverage to
    // keep one fully overlapping 64-voice window near 53 MiB of PCM.
    const auto frames=static_cast<int64_t>(std::min(context.time.rate*2,96000.0));
    const auto behind=std::max(int64_t{1},frames/8);
    if(!checkedSubtract(context.time.sample,behind,pub->start) || !checkedAdd(context.time.sample,frames,pub->end)){observation.result="Window range overflow";return;}
    observation.start=pub->start;observation.end=pub->end;
    if(loopTo>loopFrom && (loopFrom<pub->start || loopFrom>=pub->end)){
        if(!checkedSubtract(loopFrom,behind,pub->loopStart) || !checkedAdd(loopFrom,frames,pub->loopEnd)){observation.result="Loop range overflow";return;}}
    const auto anySolo=std::any_of(session.swells.begin(),session.swells.end(),[](const auto& w){return w.solo;});
    SwellAssetStore store(instanceRoot(context.capture.manifest));juce::String error;uint64_t reserved=8ULL*1024*1024;size_t unavailable=0;
    std::vector<Swell> gainMembers;
    std::map<juce::String,float> sharedMemberPeaks;
    for(const auto& w:session.swells){if(cancel())return;
        if(!w.enabled || w.muted || (anySolo && !w.solo) || !w.render.present())continue;
        LiveCache cached;size_t slot;uint64_t memberGeneration;bool recovered=false;
        {const juce::ScopedLock guard(lock);slot=liveSlots.at(w.id);memberGeneration=liveGenerations[slot];cached=liveCache[w.id];recovered=runtime[w.id].cachedWetOnly;}
        const auto key=memberKey(w);
        const auto checking=w.phase==Phase::checking && cached.key==key;
        int64_t a,b;if(!shapeBounds(w,w.render.start,w.render.sourceEnd,a,b,error)){
            if(requestedShapeBounds(w,w.render.start,a,b)){pub->entries.push_back({slot,memberGeneration,a,b,0,{},{}});++unavailable;}
            error.clear();continue;}
        if(recovered || (w.phase!=Phase::ready && !checking)){
            pub->entries.push_back({slot,memberGeneration,a,b,0,{},{}});++unavailable;continue;}
        if(held.contains(w.id) && (!store.verify(w.ir,error) || !store.verify(w.render,error))){
            const juce::ScopedLock guard(lock);invalidateLiveLocked(w.id);if(auto current=find(model,w.id)){current->phase=Phase::error;current->status="Retained audio unavailable: "+error;}observation.result=error;return;}
        if(cached.key!=key){
            if(available(reserved)<512ULL*1024){error="256 MiB budget unavailable for bounded live verification scratch";break;}
            float peak=0;const auto shared=sharedMemberPeaks.find(waveformKey(w));
            if(shared!=sharedMemberPeaks.end())peak=shared->second;
            else if((!w.frozen && !store.verify(w.ir,error)) || !store.verify(w.render,error) || !readShaped(w,store,a,b,nullptr,peak,error,cancel)){
                if(!cancel()){
                    if(held.contains(w.id)){const juce::ScopedLock guard(lock);invalidateLiveLocked(w.id);if(auto current=find(model,w.id)){current->phase=Phase::error;current->status="Retained audio unavailable: "+error;}}
                    else report(w,generation,Phase::error,error);
                    if(error.isNotEmpty())observation.result=error;}
                return;}
            sharedMemberPeaks[waveformKey(w)]=peak;
            cached={key,peak};const juce::ScopedLock guard(lock);if(cancel())return;liveCache[w.id]=cached;
        }
        LiveEntry entry{slot,memberGeneration,a,b,cached.peak,{}};
        entry.version=w.inputGeneration;
        const auto first=std::max(a,pub->start),last=std::min(b,pub->end);
        if(first<last){const auto bytes=static_cast<uint64_t>(last-first)*2*sizeof(float);
            if(bytes>available(reserved+65536)){error="256 MiB budget unavailable for live pages";break;}
            entry.audio=cached.main.lock();
            if(!entry.audio || entry.audio->start!=first || entry.audio->frames()!=static_cast<size_t>(last-first)){
                auto audio=std::make_shared<WetRender>();float ignored=0;
                if(!readShaped(w,store,first,last,audio.get(),ignored,error,cancel)){if(!cancel())report(w,generation,Phase::error,error);if(error.isNotEmpty())observation.result=error;return;}entry.audio=std::move(audio);}
            reserved+=entry.audio->samples.capacity()*sizeof(float);cached.main=entry.audio;
        }
        const auto loopFirst=std::max(a,pub->loopStart),loopLast=std::min(b,pub->loopEnd);
        if(pub->loopEnd>pub->loopStart && loopFirst<loopLast){const auto bytes=static_cast<uint64_t>(loopLast-loopFirst)*2*sizeof(float);
            if(bytes>available(reserved+65536)){error="256 MiB budget unavailable for loop destination pages";break;}
            entry.loopAudio=cached.loop.lock();
            if(!entry.loopAudio || entry.loopAudio->start!=loopFirst || entry.loopAudio->frames()!=static_cast<size_t>(loopLast-loopFirst)){
                auto audio=std::make_shared<WetRender>();float ignored=0;
                if(!readShaped(w,store,loopFirst,loopLast,audio.get(),ignored,error,cancel)){if(!cancel())report(w,generation,Phase::error,error);if(error.isNotEmpty())observation.result=error;return;}entry.loopAudio=std::move(audio);}
            reserved+=entry.loopAudio->samples.capacity()*sizeof(float);cached.loop=entry.loopAudio;}
        {const juce::ScopedLock guard(lock);if(cancel())return;liveCache[w.id]=cached;}
        pub->entries.push_back(std::move(entry));
        gainMembers.push_back(w);
    }
    if(cancel())return;
    if(error.isEmpty())prepareLiveSegments(*pub,error);
    if(error.isEmpty() && session.gainPolicy==GainPolicy::musical){
        const auto key=gainKey(session,gainMembers);float attenuation=1,peak=0;bool cached=false;
        const auto started=juce::Time::getMillisecondCounterHiRes();
        {const juce::ScopedLock guard(lock);cached=key==gainPlanKey;if(cached){attenuation=gainPlanAttenuation;peak=gainPlanPeak;++gainPlanCacheHits;}}
        if(!cached && !prepareMusicalPlan(session,gainMembers,store,available(reserved),cancel,attenuation,peak,error)){}
        if(error.isEmpty()){pub->attenuation=attenuation;pub->measuredAttenuation=true;
            if(!cached){const juce::ScopedLock guard(lock);if(!cancel()){gainPlanKey=key;gainPlanAttenuation=attenuation;gainPlanPeak=peak;
                gainPlanPreparationMs=juce::Time::getMillisecondCounterHiRes()-started;++gainPlanBuilds;}}}
    }
    if(error.isEmpty() && !prepareLiveSegments(*pub,error)){}
    if(livePublicationHold.load()){
        livePublicationWaiting=true;
        while(livePublicationHold.load() && !threadShouldExit() && !cancel())juce::Thread::sleep(2);
        livePublicationWaiting=false;
    }
    const auto latest=contextProvider();observation.latest=latest.time;
    const auto latestClock=liveClock.snapshot(sessionGeneration.load());
    if(!latestClock || !clock->sameRun(*latestClock) || latestClock->accepts(session.timing,latest.time)!=LiveGate::admitted){
        ++liveClockRejections;observation.result="Live clock changed during preparation; retry";return;}
    if(latest.captureGeneration!=context.captureGeneration || liveAssetGate(session.timing,latest.time)!=LiveGate::admitted){observation.result="Latest source generation or mapping changed";return;}
    for(const auto& w:session.swells)if(!w.frozen && w.render.present()
        && sourceDependency(latest.capture.manifest,w.sourceStart,renderInputEnd(w))!=sourceDependency(context.capture.manifest,w.sourceStart,renderInputEnd(w))){observation.result="Latest source dependency changed";return;}
    if(!((latest.time.sample>=pub->start && latest.time.sample<pub->end)
        || (latest.time.sample>=pub->loopStart && latest.time.sample<pub->loopEnd))){
        observation.result="Live position left candidate coverage; retry";return;}
    const juce::ScopedLock guard(lock);if(cancel())return;
    if(error.isNotEmpty()){liveStatus=error;observation.result=error;return;}
    const auto start=pub->start,end=pub->end;const auto attenuation=pub->attenuation;
    if(pub->bytes()>available()){liveStatus="256 MiB live publication budget exceeded";observation.result=liveStatus;return;}
    if(livePublicationFail.exchange(false)){++livePublicationFailures;observation.result="Injected transient live publication miss";return;}
    if(live.publish(std::move(pub))){for(const auto& id:replacements)retainedIR.erase(id);preparedLiveRevision=revision;observation.preparedRevision=revision;observation.result="Published complete window";liveWindowStart=start;liveWindowEnd=end;
        preparedLoopStart=loopFrom;preparedLoopEnd=loopTo;
        liveAttenuationDb=juce::Decibels::gainToDecibels(attenuation);liveStatus=unavailable?"Live window | "+juce::String(static_cast<int>(unavailable))+" swells unavailable":"Prepared live window";}
    else {liveStatus="Live publication queue busy";observation.result=liveStatus;}
}

void SwellEngine::prepareLoopTransport(const PreviewContext& context)
{
    const auto& manifest = context.capture.manifest;
    if (!manifest.loop || context.capture.unavailable || !manifest.revision) return;
    const auto flow = workflowState ? workflowState->snapshot() : workflow::Snapshot{workflow::Mode::live};
    const auto inLive = flow.mode == workflow::Mode::live && liveEnabled.load();
    auto publication = std::make_unique<LoopPlaybackPublication>();
    publication->captureGeneration = context.captureGeneration;
    publication->sourceRevision = manifest.revision;
    publication->rate = manifest.sampleRate;
    publication->timing = *manifest.loop;
    uint64_t revision, session;
    Session snapshot;
    std::vector<juce::String> replacements;
    std::set<juce::String> retained;
    {
        const juce::ScopedLock guard(lock);
        loopPlayback.collect();
        const bool newSource = preparedLoopGeneration != context.captureGeneration || preparedLoopSource != manifest.revision;
        publication->dependencies = loopPlayback.dependencies();
        if (newSource)
        {
            if (!loopPlayback.publish(std::make_unique<LoopPlaybackPublication>(*publication))) return;
            preparedLoopGeneration = context.captureGeneration;
            preparedLoopSource = manifest.revision;
            preparedLoopRender = 0;
        }
        if (!inLive || !context.time.valid || preparedLoopRender == liveRevision) return;
        revision = liveRevision;
        session = sessionGeneration.load();
        snapshot = model;
        for (auto& w : snapshot.swells)
        {
            const auto kept = retainedIR.find(w.id);
            if (kept == retainedIR.end()) continue;
            if (!retainedCompatible(kept->second,w,context))
            {
                invalidateLiveLocked(w.id);
                return;
            }
            if (w.phase != Phase::ready || w.render == kept->second.accepted.render)
            {
                const auto enabled = w.enabled, muted = w.muted, solo = w.solo;
                w = kept->second.accepted;
                w.phase = Phase::ready;
                w.enabled = enabled; w.muted = muted; w.solo = solo;
                retained.insert(w.id);
            }
            else replacements.push_back(w.id);
        }
    }

    using Preparation = LoopPlaybackPublication::Preparation;
    const auto cancel = [&]{
        const juce::ScopedLock guard(lock);
        return threadShouldExit() || liveRevision != revision || sessionGeneration.load() != session
            || publication->dependencies != loopPlayback.dependencies();
    };
    const auto anySolo = std::any_of(snapshot.swells.begin(),snapshot.swells.end(),[](const auto& w){return w.solo;});
    SwellAssetStore store(instanceRoot(manifest));
    juce::String error;
    auto result = Preparation::noWet;
    uint64_t reserved = 8ULL * 1024 * 1024;
    for (auto& w : snapshot.swells)
    {
        if (cancel()) return;
        if (!w.enabled || w.muted || (anySolo && !w.solo)) continue;
        if (w.phase != Phase::ready || !w.render.present())
        {
            if (w.phase == Phase::queued || w.phase == Phase::restoring || w.phase == Phase::checking || w.phase == Phase::rendering)
                result = Preparation::waiting;
            if (result == Preparation::waiting) break;
            continue;
        }
        if (snapshot.gainPolicy == GainPolicy::legacy) w.calibration = {};
        int64_t first, end;
        if (w.render.rate != manifest.sampleRate || w.render.bpm != manifest.loop->tempoBpm
            || w.render.numerator != manifest.loop->meterNumerator || w.render.denominator != manifest.loop->meterDenominator
            || !shapeBounds(w,w.render.start,w.render.sourceEnd,first,end,error)) continue;
        const auto frames = static_cast<uint64_t>(end) - static_cast<uint64_t>(first);
        if (frames > available(reserved) / (2 * sizeof(float))) { result = Preparation::budget; break; }
        const auto key = memberKey(w);
        std::shared_ptr<const WetRender> render;
        {
            const juce::ScopedLock guard(lock);
            const auto resolved = runtime.find(w.id);
            if (!retained.contains(w.id) && (resolved == runtime.end() || resolved->second.cachedWetOnly
                || resolved->second.verifiedRender != w.render
                || (!w.frozen && resolved->second.revision != juce::String(sourceDependency(manifest,w.sourceStart,renderInputEnd(w))))))
                continue;
            const auto cached = liveCache.find(w.id);
            if (cached != liveCache.end() && cached->second.key == key) render = cached->second.main.lock();
        }
        if (!render || render->start != first || render->frames() != frames)
        {
            auto shaped = std::make_shared<WetRender>();
            float peak = 0;
            if ((!w.frozen && !store.verify(w.ir,error)) || !store.verify(w.render,error)
                || !readShaped(w,store,first,end,shaped.get(),peak,error,cancel))
            { result = Preparation::noWet; publication->plan.audio.entries.clear(); break; }
            render = std::move(shaped);
            const juce::ScopedLock guard(lock);
            liveCache[w.id] = {key,render->peak,render,{}};
        }
        reserved += render->samples.capacity() * sizeof(float);
        const auto slot = publication->plan.audio.entries.size();
        publication->plan.audio.entries.push_back({slot,0,first,end,render->peak,std::move(render),{},0});
    }
    if (cancel()) return;
    if (result == Preparation::noWet && !publication->plan.audio.entries.empty())
    {
        result = publication->plan.prepare(manifest.loop->endSample,available(),error) ? Preparation::ready : Preparation::budget;
        if (result == Preparation::ready)
        {
            publication->gain = juce::Decibels::decibelsToGain(snapshot.liveWetDb);
            if (publication->plan.peakBound > 0)
                publication->gain = std::min(publication->gain,
                    juce::Decibels::decibelsToGain(gainCeilingDb(snapshot)) / publication->plan.peakBound);
            publication->wetOnly = snapshot.liveRouting == LiveRouting::wetOnly;
            if (!publication->prepareLeadInPeaks(cancel)) return;
            if (publication->bytes() > available()) result = Preparation::budget;
        }
    }
    publication->preparation = result;
    if (result != Preparation::ready)
    {
        publication->plan = {};
        publication->leadInPeaks.reset();
    }
    if (livePublicationHold.load())
    {
        livePublicationWaiting = true;
        while (livePublicationHold.load() && !cancel()) juce::Thread::sleep(2);
        livePublicationWaiting = false;
    }
    const auto latest = contextProvider();
    if (latest.captureGeneration != context.captureGeneration || latest.capture.reference != context.capture.reference) return;
    const juce::ScopedLock guard(lock);
    if (cancel() || (workflowState && workflowState->snapshot().mode != workflow::Mode::live)) return;
    if (result == Preparation::ready && !replacements.empty())
    {
        loopPlayback.invalidatePrepared();
        publication->dependencies = loopPlayback.dependencies();
    }
    if (!loopPlayback.publish(std::move(publication))) return;
    if (result == Preparation::ready) for (const auto& id : replacements) retainedIR.erase(id);
    preparedLoopRender = revision;
}
}
