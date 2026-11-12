#include "SwellEngine.h"
#include "GainStaging.h"
#include <Core/MonitoringSafety.h>

namespace m3
{
namespace
{
bool isAudible(const Swell& swell, bool anySolo) noexcept
{
    return swell.enabled && !swell.muted && (!anySolo || swell.solo);
}

bool usesRetainedRender(const Swell& swell, const Swell& accepted, bool loop)
{
    return swell.phase != Phase::ready
        || (swell.render == accepted.render && (loop || swell.ir.content != accepted.ir.content));
}

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
    +"/"+juce::String(w.generatedGain.value_or(1.0f),17)
    +"/"+w.calibration.sourceIdentity+"/"+w.calibration.irIdentity+"/"+w.calibration.renderIdentity+"/"+w.render.path+"/"+w.render.checksum+"/"+w.ir.path;}

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

struct HeadroomMember
{
    Swell swell;
    int64_t start, end;
    bool pending;
};

bool measureSongHeadroom(const std::vector<HeadroomMember>& members, const SwellAssetStore& store,
                         double ceiling, const RenderCancellation& cancel, double& gain, juce::String& error)
{
    std::vector<std::pair<int64_t,int64_t>> intervals;
    for (const auto& member : members) intervals.emplace_back(member.start,member.end);
    std::sort(intervals.begin(),intervals.end());
    std::vector<std::pair<int64_t,int64_t>> occupied;
    for (const auto& interval : intervals)
    {
        if (!occupied.empty() && interval.first < occupied.back().second)
            occupied.back().second = std::max(occupied.back().second,interval.second);
        else occupied.push_back(interval);
    }

    struct Sample
    {
        double direct=0, ramped=0, absolute=0;
    };
    constexpr int64_t chunk = 4096;
    std::vector<Sample> retained(chunk * 2), generated(chunk * 2);
    for (const auto& [start,end] : occupied)
    {
        for (auto first = start; first < end;)
        {
            if (cancel()) return false;
            const auto frames = std::min(chunk,end-first);
            const auto last = first + frames;
            std::fill_n(retained.begin(),frames*2,Sample{});
            std::fill_n(generated.begin(),frames*2,Sample{});
            for (const auto& member : members)
            {
                if (member.end <= first || member.start >= last) continue;
                WetRender part;
                float peak = 0;
                if (!readShaped(member.swell,store,first,last,&part,peak,error,cancel)) return false;
                auto& sum = member.pending ? generated : retained;
                const auto offset = static_cast<size_t>(part.start-first)*2;
                const auto ramp = std::clamp(std::round(member.swell.render.rate*.005),1.0,9600.0);
                for (size_t i = 0; i < part.samples.size(); ++i)
                {
                    auto& sample = sum[offset+i];
                    const auto value = part.samples[i];
                    const auto age = part.start-member.start + static_cast<int64_t>(i/2);
                    sample.direct += value;
                    sample.ramped += value * std::min(1.0,(age+1)/ramp);
                    sample.absolute += std::abs(value);
                }
            }
            for (int64_t i = 0; i < frames*2; ++i)
            {
                const auto& old = retained[static_cast<size_t>(i)];
                const auto& added = generated[static_cast<size_t>(i)];
                constexpr auto rounding = std::numeric_limits<float>::epsilon() * (liveMaximumOverlap+4);
                const auto cap = ceiling - old.absolute * rounding;
                const auto constrain = [&](double accepted, double pending)
                {
                    if (std::abs(accepted) > cap) return false;
                    const auto errorPerGain = added.absolute * rounding;
                    const auto positive = pending + errorPerGain;
                    const auto negative = -pending + errorPerGain;
                    if (positive > 0) gain = std::min(gain,(cap-accepted)/positive);
                    if (negative > 0) gain = std::min(gain,(cap+accepted)/negative);
                    return true;
                };
                if (!constrain(old.direct,added.direct) || !constrain(old.ramped,added.ramped))
                {
                    error = "Retained swells exceed the generated wet ceiling";
                    return false;
                }
            }
            first = last;
        }
    }
    return true;
}

}
void SwellEngine::prepareGeneratedHeadroom(const PreviewContext& context)
{
    std::vector<HeadroomMember> members;
    std::vector<std::pair<juce::String,bool>> pending;
    std::set<juce::String> staged;
    uint64_t revision, session;
    bool hasAudiblePending = false;
    juce::String error;
    {
        const juce::ScopedLock guard(lock);
        auto swells=model.swells;
        const auto acceptingRenders=std::any_of(swells.begin(),swells.end(),
            [](const auto& swell){return swell.phase==Phase::headroom;});
        if(acceptingRenders)
        {
            // Ordinary renders publish immediately. Measure them with the
            // accepted mix, then measure staged swaps against those gains.
            resetStagedIRHeadroomLocked();
        }
        else
        {
            for(auto& swell:swells)
            {
                const auto replacement=irReplacements.find(swell.id);
                if(replacement!=irReplacements.end() && replacement->second.candidate
                    && irReplacementCurrentLocked(*replacement->second.candidate))
                {
                    swell=*replacement->second.candidate;
                    staged.insert(swell.id);
                }
            }
        }
        if(!staged.empty() && std::any_of(swells.begin(),swells.end(),[](const auto& swell){return swell.phase==Phase::headroom;}))
            for(auto& swell:swells)
                if(staged.contains(swell.id))swell.phase=Phase::headroom;
        const auto anySolo = std::any_of(swells.begin(),swells.end(),[](const auto& w){return w.solo;});
        for (const auto& swell : swells)
        {
            if (swell.phase == Phase::headroom)
            {
                const auto audible = isAudible(swell,anySolo);
                pending.emplace_back(swell.id,audible);
                hasAudiblePending |= audible;
            }
            else if (swell.phase == Phase::queued || swell.phase == Phase::rendering
                     || swell.phase == Phase::checking || swell.phase == Phase::restoring) return;
        }
        if (pending.empty()) return;
        revision = liveRevision;
        session = sessionGeneration.load();
        const bool loop = context.capture.manifest.loop.has_value();
        const auto clock = loop ? std::nullopt : liveClock.snapshot(session);
        const auto* liveContext = clock && clock->playing && clock->accepts(model.timing,context.time)==LiveGate::admitted
            ? &*clock : nullptr;
        for (auto swell : swells)
        {
            if (!hasAudiblePending || !isAudible(swell,anySolo)) continue;
            const auto isPending = swell.phase == Phase::headroom;
            if (!isPending)
            {
                const auto kept = retainedIR.find(swell.id);
                if (kept != retainedIR.end() && usesRetainedRender(swell,kept->second.accepted,loop)
                    && retainedCompatible(kept->second,swell,context,liveContext))
                    swell = kept->second.accepted;
                else if (swell.phase != Phase::ready) continue;
            }
            if (model.gainPolicy == GainPolicy::legacy) swell.calibration = {};
            swell.shape.gainDb = defaultSwellLevelDb;
            if (isPending) swell.generatedGain.reset();
            int64_t start, end;
            if (!shapeBounds(swell,swell.render.start,swell.render.sourceEnd,start,end,error)) break;
            members.push_back({std::move(swell),start,end,isPending});
        }
    }
    const auto cancel = [&]{
        const juce::ScopedLock guard(lock);
        return threadShouldExit() || offlineActive.load() || sessionGeneration.load() != session || liveRevision != revision;
    };
    SwellAssetStore store(instanceRoot(context.capture.manifest));
    const auto ceiling = juce::Decibels::decibelsToGain(static_cast<double>(candidateWetCeilingDb));
    double gain = 1;
    const auto started = juce::Time::getMillisecondCounterHiRes();
    if (error.isEmpty() && hasAudiblePending)
    {
        if (available() < 512ULL*1024) error = "Working budget unavailable for generated headroom";
        else if (const auto& loop = context.capture.manifest.loop)
        {
            LoopWetPlan plan;
            std::vector<float> analysisGains(members.size(),1);
            uint64_t reserved = 512ULL*1024;
            for (const auto& member : members)
            {
                const auto frames = static_cast<uint64_t>(member.end-member.start);
                if (frames > available(reserved)/(2*sizeof(float)))
                { error = "Working budget unavailable for Loop generated headroom"; break; }
                auto shaped = std::make_shared<WetRender>();
                float peak = 0;
                if (!readShaped(member.swell,store,member.start,member.end,shaped.get(),peak,error,cancel)) break;
                reserved += shaped->samples.capacity()*sizeof(float);
                plan.audio.entries.push_back({plan.audio.entries.size(),0,member.start,member.end,peak,std::move(shaped),{},0});
            }
            const auto measure = [&](float candidate, double& peak)
            {
                for (size_t i = 0; i < members.size(); ++i)
                    analysisGains[i] = members[i].pending ? candidate : 1.0f;
                const auto analysis = fixedLoopGrid ? LoopWetPlan::PeakAnalysis::measure : LoopWetPlan::PeakAnalysis::skip;
                if (!plan.prepare(loop->endSample,available(),error,cancel,analysisGains,analysis)) return false;
                peak = fixedLoopGrid ? plan.fixedGridPeak : plan.peakBound;
                return true;
            };
            double peak = 0;
            if (error.isEmpty() && measure(1,peak) && peak > ceiling)
            {
                if (std::all_of(members.begin(),members.end(),[](const auto& member){return member.pending;}))
                    gain = ceiling/peak;
                else if (measure(0,peak))
                {
                    if (peak > ceiling) error = "Retained swells exceed the generated wet ceiling";
                    else
                    {
                        // The peak is convex in the new batch's common gain.
                        // Since zero fits, find the highest tested gain that fits.
                        float low = 0, high = 1;
                        for (int step = 0; step < 14; ++step)
                        {
                            const auto mid = (low+high)*0.5f;
                            if (!measure(mid,peak)) break;
                            if (peak <= ceiling) low = mid;
                            else high = mid;
                        }
                        gain = low;
                    }
                }
            }
        }
        else if (!measureSongHeadroom(members,store,ceiling,cancel,gain,error) && error.isEmpty())
            error = "Generated headroom preparation superseded";
    }
    if (error.isEmpty() && !(gain > 0)) error = "No headroom available for generated wet";
    const auto acceptedGain = gain < 1 ? static_cast<float>(gain * 0.9999) : 1.0f;
    if (error.isEmpty() && !(acceptedGain > 0)) error = "No representable gain fits the generated wet ceiling";
    const auto latest = contextProvider();
    const juce::ScopedLock guard(lock);
    if (cancel() || latest.captureGeneration != context.captureGeneration
        || latest.capture.manifest.loop != context.capture.manifest.loop
        || latest.capture.location != context.capture.location) return;
    for (const auto& member : members)
    {
        const auto& swell = member.swell;
        if (!swell.frozen && sourceDependency(latest.capture.manifest,swell.sourceStart,renderInputEnd(swell))
            != sourceDependency(context.capture.manifest,swell.sourceStart,renderInputEnd(swell))) return;
    }
    for (const auto& [id,audible] : pending)
    {
        auto& swell = staged.contains(id)?*irReplacements.at(id).candidate:*find(model,id);
        if (!audible || error.isEmpty())
        {
            swell.generatedGain = audible ? acceptedGain : 1.0f;
            swell.phase = Phase::ready;
            swell.status = "Ready | Generated wet trim " + juce::String(20*std::log10(*swell.generatedGain),1) + " dB";
            if(staged.contains(id))
            {
                swell.progress=1;
                if(irReplacements.at(id).fitted)swell.status="Ready | Wet timing adjusted to fit this IR";
            }
        }
        else
        {
            if(staged.contains(id))failIRReplacementLocked(id,error);
            else
            {
                swell.phase = Phase::error;
                swell.status = error;
            }
        }
    }
    if (error.isEmpty() && hasAudiblePending)
    {
        gainPlanPreparationMs = juce::Time::getMillisecondCounterHiRes()-started;
        ++gainPlanBuilds;
    }
    if (error.isNotEmpty()) message = error;
    if (std::any_of(pending.begin(),pending.end(),[&](const auto& item)
        {return !staged.contains(item.first) && (error.isEmpty() || !item.second);}))
        persistentChangedLocked(false);
    ++liveRevision;
    ++monitoringGeneration;
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
void SwellEngine::acceptIdleIRReplacements(const PreviewContext& context)
{
    const auto scheduledPlayback=[&]
    {
        const auto inLive=!workflowState || workflowState->snapshot().mode==workflow::Mode::live;
        const auto loop=context.capture.sourceKind()==CaptureSourceKind::loop;
        const auto clock=liveClock.snapshot(sessionGeneration.load());
        return inLive && liveEnabled.load() && (loop
            ? loopPlayback.status().playback!=LoopCueScheduler::State::standby
            : clock && clock->playing && clock->accepts(model.timing,context.time)==LiveGate::admitted);
    };
    Session candidate;
    std::vector<juce::String> replacements;
    uint64_t session, revision, monitorRevision, musicalRevision;
    bool auditioning;
    {
        const juce::ScopedLock guard(lock);
        if(offlineActive.load() || irReplacements.empty() || scheduledPlayback())return;
        candidate=model;
        replacements=overlayReadyIRReplacementsLocked(candidate,context);
        if(replacements.empty())return;
        session=sessionGeneration.load();
        revision=liveRevision;
        monitorRevision=monitoringGeneration;
        musicalRevision=modelRevision;
        auditioning=audioAllowed.load() && voice.isActiveOrPending();
        if(auditioning && !voice.canReplaceNow())return;
    }
    const auto cancel=[&]
    {
        const juce::ScopedLock guard(lock);
        return threadShouldExit() || offlineActive.load() || session!=sessionGeneration.load()
            || revision!=liveRevision || musicalRevision!=modelRevision || monitorRevision!=monitoringGeneration
            || scheduledPlayback();
    };
    Runtime monitor;
    juce::String error;
    const auto* anchor=find(candidate,candidate.primary);
    const auto ready=!auditioning || (anchor && prepareAuditionMonitor(candidate,*anchor,context,cancel,monitor,error));
    const auto latest=contextProvider();
    const juce::ScopedLock guard(lock);
    if(cancel())return;
    if(!sameMusicalMapping(context.time,latest.time) || latest.captureGeneration!=context.captureGeneration)return;
    for(const auto& id:replacements)
    {
        const auto& swell=*find(candidate,id);
        if(!irReplacementCurrentLocked(swell) || irReplacements.at(id).sourceRevision
            !=juce::String(sourceDependency(latest.capture.manifest,swell.sourceStart,renderInputEnd(swell))))return;
    }
    const auto stillAuditioning=audioAllowed.load() && voice.isActiveOrPending();
    if(stillAuditioning && !auditioning)return; // A new Play won the race. Prepare against its mix.
    if(stillAuditioning && !ready)
    {
        for(const auto& id:replacements)
            failIRReplacementLocked(id,"Could not prepare the new Audition mix. "+error+" Try again or choose another response.");
        return;
    }
    if(stillAuditioning && !voice.replace(monitor.monitor,true))return;
    commitIRReplacementsLocked(replacements,std::move(candidate.importedIRs));
    if(stillAuditioning)
    {
        auto& state=runtime[candidate.primary];
        state.monitor=std::move(monitor.monitor);
        state.monitorRevision=std::move(monitor.monitorRevision);
        state.dryStatus=std::move(monitor.dryStatus);
        state.dryAvailable=monitor.dryAvailable;
        state.members=std::move(monitor.members);
        state.preparedMonitor=monitoringGeneration;
    }
    ++liveRevision;
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
bool SwellEngine::setFrozen(const juce::String& id,bool frozen,uint64_t expectedSession)
{
    if(!permitCreative())return false;
    const juce::ScopedLock guard(lock);
    if(expectedSession!=0 && sessionGeneration.load()!=expectedSession)return false;
    auto w=find(model,id);if(!w)return false;
    if(w->frozen==frozen)return true;
    if(frozen && (w->phase!=Phase::ready || !w->render.present())){message="Wait for a ready render before freezing";return false;}
    cancelIRReplacementLocked(id);
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
            acceptedAtEntry.emplace(id,std::move(audible));pendingAtEntry.add(current->name+": "+kept.accepted.irName+" (requested "+kept.requestedName+")");}
        for(const auto& [id,request]:irReplacements)
            if(const auto* current=find(model,id);current && current->phase==Phase::ready
                && current->inputGeneration==request.input)
            {
                acceptedAtEntry.emplace(id,*current);
                pendingAtEntry.add(current->name+": "+current->irName+" (requested "+request.name
                    +(request.error.isEmpty()?" is preparing)":" failed)"));
            }
    }
    const juce::ScopedLock work(workLock);
    if(starting){offlineAccepted=std::move(acceptedAtEntry);offlineIRNotice=pendingAtEntry.isEmpty()?juce::String{}:"Export uses accepted IRs | "+pendingAtEntry.joinIntoString("; ");}
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
                int64_t a,b;
                if(!shapeBounds(w,w.render.start,w.render.sourceEnd,a,b,error) || (!w.frozen && !store.verify(w.ir,error)) || !store.verify(w.render,error))
                {++skippedCount;skip(w,error);continue;}
                offlineEntries.push_back({offlineEntries.size(),1,a,b,0,{},{}});offlineMembers.push_back(w);
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
void SwellEngine::prepareTransport(const PreviewContext& context,bool* budgetFailure)
{
    if(budgetFailure)*budgetFailure=false;
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
    std::vector<juce::String> staged;
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
            if(usesRetainedRender(w,kept->second.accepted,false)){
                const auto enabled=w.enabled,muted=w.muted,solo=w.solo;w=kept->second.accepted;w.phase=Phase::ready;
                w.enabled=enabled;w.muted=muted;w.solo=solo;held.insert(w.id);
            }else replacements.insert(w.id);}
        if(clock->playing && (!workflowState || workflowState->snapshot().mode==workflow::Mode::live))
            staged=overlayReadyIRReplacementsLocked(session,context);
        for(auto& swell:session.swells)
            if(const auto request=irReplacements.find(swell.id);swell.phase==Phase::ready
                && request!=irReplacements.end() && request->second.input==swell.inputGeneration
                && std::find(staged.begin(),staged.end(),swell.id)==staged.end())
            {
                // Advancing the request token must not retrigger the audible accepted voice.
                swell.inputGeneration=request->second.acceptedInput;
                held.insert(swell.id);
            }
        revision=liveRevision;
        for(auto it=liveSlots.begin();it!=liveSlots.end();){if(!find(model,it->first)){live.invalidate(it->second,++liveGenerations[it->second]);liveCache.erase(it->first);it=liveSlots.erase(it);}else ++it;}
        std::array<bool,maximumSwells> used{};for(const auto& [id,slot]:liveSlots)used[slot]=true;
        for(const auto& w:session.swells)if(!liveSlots.contains(w.id)){size_t slot=0;while(slot<used.size() && used[slot])++slot;
            if(slot==used.size()){liveStatus="Live identity capacity unavailable";observation.result=liveStatus;return;}used[slot]=true;liveSlots[w.id]=slot;live.invalidate(slot,++liveGenerations[slot]);}
    }
    if(session.gainPolicy==GainPolicy::legacy)for(auto& w:session.swells)w.calibration={};
    const auto cancel=[&]{const juce::ScopedLock guard(lock);return threadShouldExit() || offlineActive.load() || generation!=sessionGeneration.load() || revision!=liveRevision;};
    const auto candidateFailure=[&](const Swell& swell,const juce::String& reason)
    {
        const juce::ScopedLock guard(lock);
        if(cancel())return;
        if(std::find(staged.begin(),staged.end(),swell.id)!=staged.end())
            failIRReplacementLocked(swell.id,reason);
        else report(swell,generation,Phase::error,reason);
    };
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
    bool pageBudgetLimited=false;
    for(const auto& w:session.swells){if(cancel())return;
        if(!isAudible(w,anySolo) || !w.render.present())continue;
        LiveCache cached;size_t slot;uint64_t memberGeneration;bool recovered=false;
        {const juce::ScopedLock guard(lock);slot=liveSlots.at(w.id);memberGeneration=liveGenerations[slot];cached=liveCache[w.id];
            recovered=runtime[w.id].cachedWetOnly && std::find(staged.begin(),staged.end(),w.id)==staged.end();}
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
            if(available(reserved)<512ULL*1024){pageBudgetLimited=true;error="256 MiB budget unavailable for bounded live verification scratch";break;}
            if((!w.frozen && !store.verify(w.ir,error)) || !store.verify(w.render,error)){
                if(!cancel()){
                    if(held.contains(w.id)){const juce::ScopedLock guard(lock);invalidateLiveLocked(w.id);if(auto current=find(model,w.id)){current->phase=Phase::error;current->status="Retained audio unavailable: "+error;}}
                    else candidateFailure(w,error);
                    if(error.isNotEmpty())observation.result=error;}
                return;}
            cached={key,0};const juce::ScopedLock guard(lock);if(cancel())return;liveCache[w.id]=cached;
        }
        LiveEntry entry{slot,memberGeneration,a,b,cached.peak,{}};
        entry.version=w.inputGeneration;
        const auto first=std::max(a,pub->start),last=std::min(b,pub->end);
        if(first<last){const auto bytes=static_cast<uint64_t>(last-first)*2*sizeof(float);
            if(bytes>available(reserved+65536)){pageBudgetLimited=true;error="256 MiB budget unavailable for live pages";break;}
            entry.audio=cached.main.lock();
            if(!entry.audio || entry.audio->start!=first || entry.audio->frames()!=static_cast<size_t>(last-first)){
                auto audio=std::make_shared<WetRender>();float ignored=0;
                if(!readShaped(w,store,first,last,audio.get(),ignored,error,cancel)){candidateFailure(w,error);if(error.isNotEmpty())observation.result=error;return;}entry.audio=std::move(audio);}
            reserved+=entry.audio->samples.capacity()*sizeof(float);cached.main=entry.audio;
        }
        const auto loopFirst=std::max(a,pub->loopStart),loopLast=std::min(b,pub->loopEnd);
        if(pub->loopEnd>pub->loopStart && loopFirst<loopLast){const auto bytes=static_cast<uint64_t>(loopLast-loopFirst)*2*sizeof(float);
            if(bytes>available(reserved+65536)){pageBudgetLimited=true;error="256 MiB budget unavailable for loop destination pages";break;}
            entry.loopAudio=cached.loop.lock();
            if(!entry.loopAudio || entry.loopAudio->start!=loopFirst || entry.loopAudio->frames()!=static_cast<size_t>(loopLast-loopFirst)){
                auto audio=std::make_shared<WetRender>();float ignored=0;
                if(!readShaped(w,store,loopFirst,loopLast,audio.get(),ignored,error,cancel)){candidateFailure(w,error);if(error.isNotEmpty())observation.result=error;return;}entry.loopAudio=std::move(audio);}
            reserved+=entry.loopAudio->samples.capacity()*sizeof(float);cached.loop=entry.loopAudio;}
        {const juce::ScopedLock guard(lock);if(cancel())return;liveCache[w.id]=cached;}
        pub->entries.push_back(std::move(entry));
    }
    if(cancel())return;
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
    if(error.isNotEmpty()){if(budgetFailure)*budgetFailure=pageBudgetLimited;
        for(const auto& id:staged)failIRReplacementLocked(id,error);liveStatus=error;observation.result=error;return;}
    const auto start=pub->start,end=pub->end;
    if(pub->bytes()>available()){if(budgetFailure)*budgetFailure=true;liveStatus="256 MiB live publication budget exceeded";
        for(const auto& id:staged)failIRReplacementLocked(id,liveStatus);observation.result=liveStatus;return;}
    if(livePublicationFail.exchange(false)){++livePublicationFailures;observation.result="Injected transient live publication miss";return;}
    if(live.publish(std::move(pub))){commitIRReplacementsLocked(staged,std::move(session.importedIRs));for(const auto& id:replacements)retainedIR.erase(id);preparedLiveRevision=revision;observation.preparedRevision=revision;observation.result="Published complete window";liveWindowStart=start;liveWindowEnd=end;
        preparedLoopStart=loopFrom;preparedLoopEnd=loopTo;
        liveStatus=unavailable?"Live window | "+juce::String(static_cast<int>(unavailable))+" swells unavailable":"Prepared live window";}
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
    std::vector<juce::String> staged;
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
            // During the edit debounce the replacement is pending even though
            // reconciliation has not moved it from Stale to Checking yet.
            if (w.phase == Phase::stale && w.ir.present() && forceRefresh.contains(w.id))
                w.phase = Phase::checking;
            const auto kept = retainedIR.find(w.id);
            if (kept == retainedIR.end()) continue;
            if (!retainedCompatible(kept->second,w,context))
            {
                invalidateLiveLocked(w.id);
                return;
            }
            if (usesRetainedRender(w,kept->second.accepted,true))
            {
                const auto enabled = w.enabled, muted = w.muted, solo = w.solo;
                w = kept->second.accepted;
                w.phase = Phase::ready;
                w.enabled = enabled; w.muted = muted; w.solo = solo;
                retained.insert(w.id);
            }
            else replacements.push_back(w.id);
        }
        if(loopPlayback.status().playback!=LoopCueScheduler::State::standby)
            staged=overlayReadyIRReplacementsLocked(snapshot,context);
        revision=liveRevision;
    }

    using Preparation = LoopPlaybackPublication::Preparation;
    const auto cancel = [&]{
        const juce::ScopedLock guard(lock);
        return threadShouldExit() || offlineActive.load() || liveRevision != revision || sessionGeneration.load() != session
            || publication->dependencies != loopPlayback.dependencies();
    };
    const auto anySolo = std::any_of(snapshot.swells.begin(),snapshot.swells.end(),[](const auto& w){return w.solo;});
    SwellAssetStore store(instanceRoot(manifest));
    juce::String error;
    auto result = Preparation::noWet;
    bool hasAudibleMember = false;
    uint64_t reserved = 8ULL * 1024 * 1024;
    for (auto& w : snapshot.swells)
    {
        if (cancel()) return;
        if (!isAudible(w,anySolo)) continue;
        hasAudibleMember = true;
        if (w.phase != Phase::ready || !w.render.present())
        {
            if (w.phase == Phase::queued || w.phase == Phase::restoring || w.phase == Phase::checking || w.phase == Phase::rendering || w.phase == Phase::headroom)
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
            if (!retained.contains(w.id) && std::find(staged.begin(),staged.end(),w.id)==staged.end()
                && (resolved == runtime.end() || resolved->second.cachedWetOnly
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
        result = publication->plan.prepare(manifest.loop->endSample,available(),error,cancel,{},LoopWetPlan::PeakAnalysis::skip)
            ? Preparation::ready : Preparation::budget;
        if (cancel()) return;
        if (result == Preparation::ready)
        {
            const auto requested = juce::Decibels::decibelsToGain(snapshot.liveWetDb);
            publication->gain = requested;
            if (!publication->prepareLeadInPeaks(cancel)) return;
            if (publication->bytes() > available()) result = Preparation::budget;
        }
    }
    publication->preparation = result;
    publication->intentionallySilent = !hasAudibleMember;
    publication->wetOnly = snapshot.liveRouting == LiveRouting::wetOnly;
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
    if (!staged.empty() && result != Preparation::ready && (result != Preparation::noWet || hasAudibleMember))
    {
        if(result!=Preparation::waiting)
            for(const auto& id:staged)failIRReplacementLocked(id,error.isNotEmpty()?error:"Replacement could not fit the Loop playback budget. Try a shorter response.");
        return;
    }
    if (result == Preparation::ready && !replacements.empty())
    {
        loopPlayback.invalidatePrepared();
        publication->dependencies = loopPlayback.dependencies();
    }
    if (!loopPlayback.publish(std::move(publication))) return;
    commitIRReplacementsLocked(staged,std::move(snapshot.importedIRs));
    if (result == Preparation::ready) for (const auto& id : replacements) retainedIR.erase(id);
    preparedLoopRender = revision;
}
}
