#include "SwellEngine.h"
#include <Core/MonitoringSafety.h>

namespace m3
{
namespace
{
juce::String memberKey(const Swell& w)
{return w.render.content+"/"+juce::String(w.inputGeneration)+"/"+juce::String(w.shapeGeneration)+"/"+juce::String(w.placement);}

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
            const auto factor=envelope(w.shape,position,a,b)*monitorEdgeGain(static_cast<size_t>(position-a),static_cast<size_t>(b-a),w.render.rate,.002);
            for(int c=0;c<2;++c){const auto value=static_cast<float>(scratch.getSample(c,n)*factor);
                if(!std::isfinite(value)){error="Non-finite live render";return false;}
                peak=std::max(peak,std::abs(value));if(output)output->samples[static_cast<size_t>(position-from)*2+c]=value;}}
        at+=count;
    }
    if(output)output->peak=peak;return true;
}
}
void SwellEngine::invalidateLiveLocked(const juce::String& id)
{
    ++liveRevision;
    if(id.isEmpty()){for(const auto& [key,slot]:liveSlots)live.invalidate(slot,++liveGenerations[slot]);}
    else if(const auto at=liveSlots.find(id);at!=liveSlots.end())live.invalidate(at->second,++liveGenerations[at->second]);
    notify();
}
void SwellEngine::setLivePlayback(bool enabled,LiveRouting routing,float db)
{
    if(!std::isfinite(db) || db< -60 || db>0 || static_cast<int>(routing)<0 || static_cast<int>(routing)>1)return;
    const juce::ScopedLock guard(lock);model.liveEnabled=enabled;model.liveRouting=routing;model.liveWetDb=db;
    liveEnabled=enabled;live.configure(enabled,routing,db);notify();
}
void SwellEngine::setRegenerationSensitivity(float value)
{
    if(!std::isfinite(value) || value<0 || value>1)return;
    const juce::ScopedLock guard(lock);model.regenerationSensitivity=value;regeneration=value;
}
bool SwellEngine::setFrozen(const juce::String& id,bool frozen)
{
    const juce::ScopedLock guard(lock);auto w=find(model,id);if(!w)return false;
    if(w->frozen==frozen)return true;
    if(frozen && (w->phase!=Phase::ready || !w->render.present())){message="Wait for a ready render before freezing";return false;}
    w->frozen=frozen;w->provenance=Provenance::manualEdited;
    if(frozen){w->status="Frozen render";forceRefresh.erase(id);}
    else {++w->inputGeneration;w->phase=Phase::checking;w->status="Checking unfrozen source";forceRefresh.insert(id);invalidateLiveLocked(id);}
    ++monitoringGeneration;notify();return true;
}
void SwellEngine::processOfflineAudio(float* left,float* right,int frames,const TransportTime& hostTime)
{
    if(frames<=0)return;
    offlineBlockCount.fetch_add(1,std::memory_order_relaxed);
    const auto starting=!offlineActive.exchange(true);
    const juce::ScopedLock work(workLock);
    const auto fail=[&](const juce::String& error){const juce::ScopedLock guard(lock);
        offlineError="Export incomplete: "+error;message=offlineError;offlineStatus=offlineError;
        if(offlineSession.liveEnabled && offlineSession.liveRouting==LiveRouting::wetOnly){std::fill_n(left,frames,0.0f);std::fill_n(right,frames,0.0f);}};
    try{
        if(starting || hostTime.song.sample<offlineLastPosition){const juce::ScopedLock guard(lock);
            offlinePlayer.reset();offlineEntries.clear();offlineMembers.clear();offlineError.clear();offlineWarning.clear();offlineStatus="Preparing offline export";offlineWindowStart=offlineWindowEnd=0;offlineSession=model;}
        offlineLastPosition=hostTime.song.sample;
        if(offlineError.isNotEmpty()){if(offlineSession.liveEnabled && offlineSession.liveRouting==LiveRouting::wetOnly){std::fill_n(left,frames,0.0f);std::fill_n(right,frames,0.0f);}return;}
        if(!offlinePlayer){
            offlineEntries.clear();offlineMembers.clear();
            const auto context=contextProvider();long double origin;
            if(!songOrigin(hostTime.song,origin)){fail("valid absolute Song timing is required");return;}
            collectCompletedPass(context);inspect(context);prepareDefault(context);dispatchAutomatic(context);
            Session pending;uint64_t generation;
            {const juce::ScopedLock guard(lock);pending=model;generation=sessionGeneration.load();}
            const auto solo=std::any_of(pending.swells.begin(),pending.swells.end(),[](const auto& w){return w.solo;});
            if(pending.liveEnabled)for(const auto& w:pending.swells)if(w.enabled && !w.muted && (!solo || w.solo))renderJob(w,context,generation,true);
            {const juce::ScopedLock guard(lock);offlineSession=model;offlineRoot=instanceRoot(context.capture.manifest);}
            if(offlineSession.liveEnabled && !transportMappingMatches(offlineSession.timing,hostTime.song) && !offlineSession.swells.empty()){
                fail("prepared assets have incompatible timing");return;}
            SwellAssetStore store(offlineRoot);juce::String error;juce::StringArray skipped;
            const auto skip=[&](const Swell& w,const juce::String& reason){if(skipped.size()<8)skipped.add(w.name+": "+reason);};int skippedCount=0;
            if(offlineSession.liveEnabled)for(const auto& w:offlineSession.swells)if(w.enabled && !w.muted && (!solo || w.solo)){
                if(w.phase!=Phase::ready){++skippedCount;skip(w,w.status);continue;}
                int64_t a,b;float peak=0;
                if(!shapeBounds(w,w.render.start,w.render.sourceEnd,a,b,error) || (!w.frozen && !store.verify(w.ir,error)) || !store.verify(w.render,error)
                    || !readShaped(w,store,a,b,nullptr,peak,error,{})){++skippedCount;skip(w,error);continue;}
                offlineEntries.push_back({offlineEntries.size(),1,a,b,peak,{},{}});offlineMembers.push_back(w);
            }
            {const juce::ScopedLock guard(lock);
                if(skippedCount)offlineWarning="Export warning: "+juce::String(skippedCount)+(skippedCount==1?" swell skipped | ":" swells skipped | ")+skipped.joinIntoString("; ");
                offlinePlayer=std::make_unique<TransportPlayback>();
                offlinePlayer->configure(offlineSession.liveEnabled,offlineSession.liveRouting,offlineSession.liveWetDb);
                offlinePlayer->primeForOffline();
                for(const auto& entry:offlineEntries)offlinePlayer->invalidate(entry.slot,1);}
        }
        if(!transportMappingMatches(offlineSession.timing,hostTime.song) && !offlineEntries.empty()){
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
            const auto expectedPpq=offlineSession.timing.ppq+(static_cast<long double>(hostTime.song.sample)-offlineSession.timing.sample)
                *offlineSession.timing.bpm/(60*offlineSession.timing.rate);
            fail("Song mapping changed during export | sample="+juce::String(hostTime.song.sample)
                +" PPQ="+juce::String(hostTime.song.ppq,6)+" expected="+juce::String(static_cast<double>(expectedPpq),6)
                +" last wet end="+juce::String(lastWetEnd));return;
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
            offlinePlayer->process(left+offset,right+offset,count,time);
            if(offlinePlayer->missing() || offlinePlayer->incompatible()){fail("required prepared wet was unavailable");return;}
            offset+=count;
        }
        {const juce::ScopedLock guard(lock);liveStatus=offlineWarning.isNotEmpty()?offlineWarning:juce::String("Offline export: all available wet rendered");offlineStatus=liveStatus;}
    }catch(const std::exception& e){fail(e.what());}
}
void SwellEngine::prepareTransport(const PreviewContext& context)
{
    if(!hostObserved.load(std::memory_order_acquire) || !context.time.valid || !liveEnabled.load())return;
    Session session;uint64_t revision,generation;
    int64_t loopFrom=0,loopTo=0;
    for(int attempt=0;attempt<8;++attempt){const auto sequence=loopSequence.load(std::memory_order_acquire);if(sequence&1)continue;
        const auto on=loopHint.load();const auto a=loopHintStart.load(),b=loopHintEnd.load();
        if(sequence==loopSequence.load(std::memory_order_acquire)){if(on && b>a){loopFrom=a;loopTo=b;}break;}}
    {
        const juce::ScopedLock guard(lock);live.collect();
        if(!transportMappingMatches(model.timing,context.time))return;
        // Some hosts wrap Song playback without advertising cycle points.
        // Keep the observed completed pass start warm as the next destination.
        if(loopTo<=loopFrom && !completedPasses.empty()){
            const auto& pass=completedPasses.back();
            if(context.time.sample>=pass.first && context.time.sample<=pass.second){loopFrom=pass.first;loopTo=pass.second;}}
        const auto margin=static_cast<int64_t>(std::min(context.time.rate,48000.0)*.5);
        if(preparedLiveRevision==liveRevision && preparedLoopStart==loopFrom && preparedLoopEnd==loopTo && context.time.sample>=liveWindowStart && context.time.sample<liveWindowEnd-margin)return;
        session=model;revision=liveRevision;generation=sessionGeneration.load();liveStatus="Preparing live window";
        for(auto it=liveSlots.begin();it!=liveSlots.end();){if(!find(model,it->first)){live.invalidate(it->second,++liveGenerations[it->second]);liveCache.erase(it->first);it=liveSlots.erase(it);}else ++it;}
        std::array<bool,maximumSwells> used{};for(const auto& [id,slot]:liveSlots)used[slot]=true;
        for(const auto& w:session.swells)if(!liveSlots.contains(w.id)){size_t slot=0;while(slot<used.size() && used[slot])++slot;
            if(slot==used.size()){liveStatus="Live identity capacity unavailable";return;}used[slot]=true;liveSlots[w.id]=slot;live.invalidate(slot,++liveGenerations[slot]);}
    }
    const auto cancel=[&]{const juce::ScopedLock guard(lock);return threadShouldExit() || generation!=sessionGeneration.load() || revision!=liveRevision;};
    auto pub=std::make_unique<LivePublication>();pub->mapping=session.timing;pub->revision=revision;
    // At most 2.25 seconds at 48 kHz; higher rates shorten time coverage to
    // keep one fully overlapping 64-voice window near 53 MiB of PCM.
    const auto frames=static_cast<int64_t>(std::min(context.time.rate*2,96000.0));
    const auto behind=std::max(int64_t{1},frames/8);
    if(!checkedSubtract(context.time.sample,behind,pub->start) || !checkedAdd(context.time.sample,frames,pub->end))return;
    if(loopTo>loopFrom && (loopFrom<pub->start || loopFrom>=pub->end)){
        if(!checkedSubtract(loopFrom,behind,pub->loopStart) || !checkedAdd(loopFrom,frames,pub->loopEnd))return;}
    const auto anySolo=std::any_of(session.swells.begin(),session.swells.end(),[](const auto& w){return w.solo;});
    SwellAssetStore store(instanceRoot(context.capture.manifest));juce::String error;uint64_t reserved=8ULL*1024*1024;size_t unavailable=0;
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
        if(cached.key!=key){
            if(available(reserved)<512ULL*1024){error="256 MiB budget unavailable for bounded live verification scratch";break;}
            float peak=0;
            if((!w.frozen && !store.verify(w.ir,error)) || !store.verify(w.render,error) || !readShaped(w,store,a,b,nullptr,peak,error,cancel)){
                if(!cancel())report(w,generation,Phase::error,error);return;}
            cached={key,peak};const juce::ScopedLock guard(lock);if(cancel())return;liveCache[w.id]=cached;
        }
        LiveEntry entry{slot,memberGeneration,a,b,cached.peak,{}};
        const auto first=std::max(a,pub->start),last=std::min(b,pub->end);
        if(first<last){const auto bytes=static_cast<uint64_t>(last-first)*2*sizeof(float);
            if(bytes>available(reserved+65536)){error="256 MiB budget unavailable for live pages";break;}
            entry.audio=cached.main.lock();
            if(!entry.audio || entry.audio->start!=first || entry.audio->frames()!=static_cast<size_t>(last-first)){
                auto audio=std::make_shared<WetRender>();float ignored=0;
                if(!readShaped(w,store,first,last,audio.get(),ignored,error,cancel)){if(!cancel())report(w,generation,Phase::error,error);return;}entry.audio=std::move(audio);}
            reserved+=entry.audio->samples.capacity()*sizeof(float);cached.main=entry.audio;
        }
        const auto loopFirst=std::max(a,pub->loopStart),loopLast=std::min(b,pub->loopEnd);
        if(pub->loopEnd>pub->loopStart && loopFirst<loopLast){const auto bytes=static_cast<uint64_t>(loopLast-loopFirst)*2*sizeof(float);
            if(bytes>available(reserved+65536)){error="256 MiB budget unavailable for loop destination pages";break;}
            entry.loopAudio=cached.loop.lock();
            if(!entry.loopAudio || entry.loopAudio->start!=loopFirst || entry.loopAudio->frames()!=static_cast<size_t>(loopLast-loopFirst)){
                auto audio=std::make_shared<WetRender>();float ignored=0;
                if(!readShaped(w,store,loopFirst,loopLast,audio.get(),ignored,error,cancel)){if(!cancel())report(w,generation,Phase::error,error);return;}entry.loopAudio=std::move(audio);}
            reserved+=entry.loopAudio->samples.capacity()*sizeof(float);cached.loop=entry.loopAudio;}
        {const juce::ScopedLock guard(lock);if(cancel())return;liveCache[w.id]=cached;}
        pub->entries.push_back(std::move(entry));
    }
    if(cancel())return;
    if(error.isEmpty() && !prepareLiveSegments(*pub,error)){}
    const auto latest=contextProvider();
    if(latest.captureGeneration!=context.captureGeneration || !transportMappingMatches(session.timing,latest.time))return;
    for(const auto& w:session.swells)if(!w.frozen && w.render.present()
        && sourceDependency(latest.capture.manifest,w.sourceStart,renderInputEnd(w))!=sourceDependency(context.capture.manifest,w.sourceStart,renderInputEnd(w)))return;
    const juce::ScopedLock guard(lock);if(cancel())return;
    if(error.isNotEmpty()){liveStatus=error;return;}
    const auto start=pub->start,end=pub->end;const auto attenuation=pub->attenuation;
    if(pub->bytes()>available()){liveStatus="256 MiB live publication budget exceeded";return;}
    if(live.publish(std::move(pub))){preparedLiveRevision=revision;liveWindowStart=start;liveWindowEnd=end;
        preparedLoopStart=loopFrom;preparedLoopEnd=loopTo;
        liveAttenuationDb=juce::Decibels::gainToDecibels(attenuation);liveStatus=unavailable?"Live window | "+juce::String(static_cast<int>(unavailable))+" swells unavailable":"Prepared live window";}
    else liveStatus="Live publication queue busy";
}
}
