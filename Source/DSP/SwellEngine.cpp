#include "SwellEngine.h"
#include <Capture/SourceRangeReader.h>
#include <Core/ContentHash.h>
#include "DefaultImpulse.h"
#include "GainStaging.h"
#include "SourceEdgeConditioning.h"
#include <cmath>

namespace m3
{
bool SwellEngine::permitCreative(juce::String* rejection)
{
    if(!workflowState || workflowState->snapshot().mode==workflow::Mode::live)return true;
    const juce::ScopedLock guard(lock);
    message="Creative editing is available in Perform mode";
    if(rejection) *rejection=message;
    return false;
}
SwellEngine::SwellEngine(juce::File path,std::function<PreviewContext()> context,std::function<void()> changed)
    :Thread("Preverb Swells"),persistentChange(std::move(changed)),legacyProjectsRoot(path),storageLocation(std::make_shared<const CaptureStorageLocation>(CaptureStorageLocation{path,path,{}})),contextProvider(std::move(context))
{ if(const auto initial=contextProvider().capture.location)storageLocation.store(initial);voice.setLevel(1);startThread(); }
SwellEngine::~SwellEngine() { signalThreadShouldExit(); sessionGeneration.fetch_add(1); stopThread(-1); }
void SwellEngine::stopLocked() { audioAllowed.store(false); voice.stop(true);exportVoice->stop(true); }
void SwellEngine::setExpected(const SongTime& time)
{ expectedRate=time.rate; expectedBpm=time.bpm; expectedNumerator=time.numerator; expectedDenominator=time.denominator; }
juce::File SwellEngine::instanceRoot(const CaptureManifest& map) const
{ return storageLocation.load()->projectsRoot.getChildFile(map.projectId).getChildFile(map.instanceId); }
bool SwellEngine::storageReady(const PreviewContext& context)
{
    if(context.capture.status.startsWith("Loading"))return false;
    const juce::ScopedLock guard(lock);
    if(context.capture.location && context.capture.location!=storageLocation.load()){
        storageLocation.store(context.capture.location);invalidateLiveLocked();liveCache.clear();stopLocked();
    }
    if(context.capture.location && !context.capture.location->writable){
        stopLocked();message="Asset working folder unavailable; locate, restore or explicitly create a recovery workspace";
        for(auto& swell:model.swells){swell.phase=Phase::stale;swell.status=message;}
        if(workflowState && workflowState->snapshot().mode==workflow::Mode::finalise){
            exportState={ExportPhase::error,message,{}};exportState.session=sessionGeneration.load();exportState.pass=workflowState->snapshot().pass;
        }
        return false;
    }
    return true;
}
Session SwellEngine::savedSession() const { const juce::ScopedLock guard(lock); return model; }
bool SwellEngine::acceptedStorageSnapshot(CaptureSessionSnapshot& capture,Session& session,juce::File& directory,uint64_t& epoch,juce::String& error,bool allowUnavailable) const
{
    const juce::ScopedLock guard(lock);
    const auto before=contextProvider();
    const bool knownSource=(before.capture.reference==captureReference && before.captureGeneration==captureGeneration)
        || (storageRestoreReference.isNotEmpty() && before.capture.reference==storageRestoreReference)
        || (storageRestoreReference.isEmpty() && before.capture.reference.isEmpty() && before.capture.manifest.revision==0 && before.capture.manifest.ranges.empty());
    if((before.capture.unavailable && !allowUnavailable) || before.capture.status.startsWith("Loading") || !knownSource
        || (before.capture.location && before.capture.location!=storageLocation.load()))
    {error="Accepted source is unavailable or still settling. Retry after it settles.";return false;}
    auto accepted=model;const auto after=contextProvider();
    if(before.captureGeneration!=after.captureGeneration || before.capture.reference!=after.capture.reference
        || before.capture.manifest.revision!=after.capture.manifest.revision || before.capture.manifest.projectId!=after.capture.manifest.projectId
        || before.capture.manifest.instanceId!=after.capture.manifest.instanceId || before.capture.location!=after.capture.location)
    {error="Accepted source changed while its creative state was pinned. Retry.";return false;}
    capture=before.capture;session=std::move(accepted);directory=instanceRoot(before.capture.manifest);epoch=sessionGeneration.load();error.clear();return true;
}
bool SwellEngine::reloadStorageAssets(uint64_t expectedSession,juce::String expectedReference)
{
    const juce::ScopedLock guard(lock);
    if(sessionGeneration.load()!=expectedSession || (workflowState && workflowState->snapshot().locked()))return false;
    auto current=model;restore(std::move(current),std::move(expectedReference));return true;
}
void SwellEngine::beginSourceReplacement()
{const juce::ScopedLock guard(lock);awaitingCaptureReference.clear();}
void SwellEngine::useWorkflow(workflow::State& state,std::function<uint64_t()> epoch)
{
    const juce::ScopedLock guard(lock);workflowState=&state;captureEpochProvider=std::move(epoch);
    publishGenerationLocked(model,UINT64_MAX,true);
}
bool SwellEngine::publishGenerationLocked(const Session& next,uint64_t expectedPass,bool restoring,std::optional<uint64_t> configurationSession)
{
    if(!workflowState)return true;
    const auto flow=workflowState->snapshot();
    if(flow.locked() || (!restoring && flow.mode==workflow::Mode::finalise) || (expectedPass!=UINT64_MAX && flow.pass!=expectedPass))
    {message="Generation settings are locked or the edit was superseded";return false;}
    const auto slot=static_cast<uint8_t>(1-flow.configurationSlot);
    generationConfigurations[slot]={next.automaticProcessing && next.autoMarkersEnabled,next.sensitivity,next.separationSeconds,
        next.defaultIR,next.defaultIRName,++configurationRevision,configurationSession.value_or(sessionGeneration.load())};
    if(!workflowState->publishConfiguration(slot,flow.pass))
    {message="Generation setting lost admission to playback; previous setting retained";return false;}
    cancelLearnGenerationLocked();
    return true;
}
std::optional<GenerationConfiguration> SwellEngine::passConfiguration(uint64_t pass) const
{
    const juce::ScopedLock guard(lock);if(!workflowState)return {};
    const auto flow=workflowState->snapshot();const auto& config=generationConfigurations[flow.configurationSlot];
    if(!flow.locked() || flow.pass!=pass || config.session!=sessionGeneration.load())return {};
    return config;
}
EditorSnapshot SwellEngine::snapshot() const
{
    const juce::ScopedLock guard(lock); EditorSnapshot s;
    s.workflowState=workflowState?workflowState->snapshot():workflow::Snapshot{};
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::finalise){
        const auto pass=workflowState->snapshot().pass;s.songExport=exportState;
        if(exportState.session!=sessionGeneration.load() || exportState.pass!=pass)s.songExport={ExportPhase::preparing,"Preparing export mix",{}};
        s.songExport.session=sessionGeneration.load();s.songExport.pass=pass;s.songExport.cursor=exportVoice->position();s.songExport.previewRequested=exportVoice->requested(exportVoice->request());
        if(s.songExport.previewRequested){const auto levels=exportVoice->acceptedLevels();if(levels.revision)s.songExport.previewLevels=levels;}
        if(exportVoice->starved())s.songExport.message="Preview stopped: audio pages unavailable; WAV unchanged";
        if(exportVoice->invalidAudio())s.songExport.message="Preview stopped: non-finite mixed audio; WAV unchanged";}
    s.sessionEpoch=sessionGeneration.load();
    s.gridClock=transportClockSnapshot();
    s.learnGeneration=learnViewLocked();s.initialPerformHandoff=initialPerformOffer;
    s.session=model; s.candidates=candidates; s.actionError=actionError;s.message=actionError.isEmpty()?message:actionError; s.auditionSample=audioAllowed.load()?voice.position():std::nullopt;s.playing=s.auditionSample.has_value(); s.analyzing=analyzing; s.renderCount=renderCount;
    s.defaultIRError=defaultError;
    s.defaultIRPending=defaultPending;s.requestedDefaultIRName=requestedDefaultName;s.defaultIRStatus=defaultStatus;
    s.livePreparation=livePreparation;
    s.liveObservation=live.observation();s.liveLastRejection=live.lastRejection();
    s.liveCausal=live.causalObservation();
    s.liveActive=live.active();s.liveMissing=live.missing();s.liveIncompatible=live.incompatible();s.liveStatus=liveStatus;s.liveAttenuationDb=liveAttenuationDb;
    s.gainPlanPeak=gainPlanPeak;s.gainPlanPreparationMs=gainPlanPreparationMs;s.gainPlanBuilds=gainPlanBuilds;s.gainPlanCacheHits=gainPlanCacheHits;
    s.liveReady=preparedLiveRevision>0 && preparedLiveRevision==liveRevision;
    s.liveLoopDestination=preparedLoopStart;
    if(offlineError.isNotEmpty())s.liveStatus=offlineError;else if(offlineWarning.isNotEmpty())s.liveStatus=offlineWarning;
    s.offlineBlocks=offlineBlockCount.load();s.offlineStatus=offlineStatus;
    s.analysisCount=automaticAnalysisCount;s.pendingIntervals=dirty.size()+passDirty.size();s.analyzing=analyzing || automaticBusy.load();
    for(const auto& [id,r]:runtime)if(r.rawWave)s.wetSummaries.emplace(id,r.rawWave);
    const auto w=find(model,model.primary); const auto at=runtime.find(model.primary);
    if(w)if(const auto kept=retainedIR.find(w->id);kept!=retainedIR.end()){
        s.retainedIRName=kept->second.accepted.irName;s.requestedIRName=kept->second.requestedName;}
    if(at!=runtime.end())
    {
        s.auditionPreparing=at->second.preparedMonitor!=monitoringGeneration;
        s.raw=at->second.rawWave;s.sourceDetail=at->second.sourceWave;
        if(w && at->second.preparedShape==w->shapeGeneration && at->second.preparedRender==w->render.content)s.shaped=at->second.shapeWave;
        if(w && at->second.shaped && at->second.preparedShape==w->shapeGeneration && at->second.preparedRender==w->render.content)
        {
            auto playable=*w;if(at->second.cachedWetOnly)playable.phase=Phase::ready;
            s.auditionReady=playable.phase==Phase::ready && at->second.monitor && at->second.preparedMonitor==monitoringGeneration
                && (model.auditionMode==AuditionMode::wetOnly || at->second.dryAvailable);
            s.auditionMembers=at->second.members;s.dryAvailable=at->second.dryAvailable;s.dryStatus=at->second.dryStatus;s.cachedWetOnly=at->second.cachedWetOnly;
            if(at->second.monitor){s.auditionDryStart=at->second.monitor->sourceStart;s.auditionDryEnd=at->second.monitor->sourceEnd;}
            s.blendAttenuationDb=at->second.blendAttenuation;
            if(at->second.monitor)s.monitorPeak=at->second.monitor->peak*juce::Decibels::decibelsToGain(model.auditionFollowsLive?0.0f:model.auditionDb);
            s.attenuationDb=at->second.wetAttenuation;
        }
    }
    return s;
}
void SwellEngine::restore(Session session,juce::String expectedCaptureReference)
{
    juce::String error; if(!validate(session,error)) { const juce::ScopedLock guard(lock); message=error; return; }
    const juce::ScopedLock guard(lock); stopLocked(); sessionGeneration.fetch_add(1); analysisRequest.fetch_add(1);
    initialiseDefaultIfMissing=false;
    invalidateLearnLocked();learnCandidates.reset();settledLearnCandidates.reset();learnUsable=false;
    initialPerformOffer=0;performPositionEstablished=find(session,session.primary)!=nullptr;
    protectedGenerationCalibration.clear();for(const auto& w:session.swells)if(w.provenance!=Provenance::automatic)protectedGenerationCalibration.insert(w.id);
    exportRequest.fetch_add(1);++exportDeliverySerial;exportDelivery.reset();detailSerial.fetch_add(1);detailWork.reset();exportState={};exportAttempt.clear();exportRetryFile.reset();
    imports.clear(); resets.clear(); actionError.clear();analysis.reset(); candidates.clear(); analyzing=false; runtime.clear(); forceRefresh.clear();
    dirty.clear();passDirty.clear();acceptedSource.reset();passRenderRequests.clear();analyzedContent.clear();trailing.reset();defaultAttemptScope.clear();defaultError.clear();
    ++defaultRequest;defaultPending=false;requestedDefaultName.clear();defaultStatus.clear();
    model=std::move(session);publishGenerationLocked(model,UINT64_MAX,true);regeneration=model.regenerationSensitivity; for(auto& w:model.swells) { w.phase=Phase::restoring; w.status="Restoring dependencies"; }
    invalidateLiveLocked();live.reset();liveCache.clear();hostObserved=false;
    completedPasses.clear();automaticBoundsDirty=true;performWindowsRequested=false;
    recoverPassBounds=true;
    liveEnabled=model.liveEnabled;live.configure(model.liveEnabled,model.liveRouting,model.liveWetDb);
    storageRestoreReference=expectedCaptureReference;awaitingCaptureReference=std::move(expectedCaptureReference);
    captureReference.clear(); captureScope.clear(); voice.setLevel(juce::Decibels::decibelsToGain(model.auditionFollowsLive?0.0f:model.auditionDb));
    message="Restoring saved swell session; audition stopped";
}
void SwellEngine::clear()
{
    if(workflowState && (workflowState->snapshot().mode!=workflow::Mode::capture || workflowState->snapshot().locked()))
    {diagnostic("Clear requires stopped idle Learn mode");return;}
    const juce::ScopedLock guard(lock); stopLocked(); sessionGeneration.fetch_add(1); analysisRequest.fetch_add(1);
    invalidateLearnLocked();learnCandidates.reset();settledLearnCandidates.reset();learnUsable=false;initialPerformOffer=0;
    publishGenerationLocked(model,UINT64_MAX,true);
    exportRequest.fetch_add(1);++exportDeliverySerial;exportDelivery.reset();detailSerial.fetch_add(1);detailWork.reset();exportState={};exportAttempt.clear();exportRetryFile.reset();
    imports.clear(); resets.clear(); actionError.clear();analysis.reset(); candidates.clear(); analyzing=false; forceRefresh.clear();
    dirty.clear();passDirty.clear();acceptedSource.reset();passRenderRequests.clear();analyzedContent.clear();trailing.reset();
    ++defaultRequest;defaultPending=false;requestedDefaultName.clear();defaultStatus.clear();
    for(auto& w:model.swells) { ++w.inputGeneration; w.phase=w.frozen?Phase::restoring:Phase::stale; w.status=w.frozen?"Restoring frozen render":"Capture cleared; recapture source"; }
    runtime.clear(); message="Capture cleared; saved assets retained";
    invalidateLiveLocked();live.reset();liveCache.clear();
    completedPasses.clear();automaticBoundsDirty=true;performWindowsRequested=false;
    recoverPassBounds=false;
}
bool SwellEngine::addTarget(int64_t arrival,juce::String* rejection)
{
    if(!permitCreative(rejection))return false;
    const auto context=contextProvider(); const juce::ScopedLock guard(lock);
    std::vector<int64_t> arrivals; for(const auto& c:candidates) arrivals.push_back(c.sample);
    const auto& targetTime=sameMusicalMapping(model.timing,context.time)?model.timing:context.time;
    if(!add(model,arrival,context.capture.manifest,targetTime,arrivals,message))
    {
        if(rejection) *rejection=message;
        return false;
    }
    if(workflowState){performPositionEstablished=true;initialPerformOffer=0;}
    protectedGenerationCalibration.clear();
    ++monitoringGeneration;auto& w=model.swells.back();w.detectorAnchor=arrival;w.ir=model.defaultIR;w.irName=model.defaultIRName;
    w.phase=Phase::stale;w.status=w.ir.present()?"Target added":"Waiting for default IR";
    requestManualPreparationLocked(w,context);
    nextAutomaticEdit=juce::Time::getMillisecondCounterHiRes()+150;
    stopLocked(); message="Target added";persistentChangedLocked();return true;
}
bool SwellEngine::confirmCandidate(size_t index)
{
    int64_t arrival;
    { const juce::ScopedLock guard(lock); if(index>=candidates.size()) return false; arrival=candidates[index].sample; }
    return confirmCandidateAt(arrival);
}
bool SwellEngine::confirmCandidateAt(int64_t arrival)
{
    {const juce::ScopedLock guard(lock);if(std::none_of(candidates.begin(),candidates.end(),[&](const auto& c){return c.sample==arrival;})){message="Suggestion changed; choose a current candidate";return false;}}
    if(!addTarget(arrival)) return false;
    const juce::ScopedLock guard(lock); std::erase_if(candidates,[&](const auto& c){return c.sample==arrival;}); return true;
}
void SwellEngine::removeSelected()
{
    if(!permitCreative())return;
    const juce::ScopedLock guard(lock); stopLocked();
    ++monitoringGeneration;const auto selected=model.selected;
    for(const auto& id:selected) if(suppressAndErase(model,id,message)) {protectedGenerationCalibration.clear();invalidateLiveLocked(id);runtime.erase(id); forceRefresh.erase(id);persistentChangedLocked(); }
}
bool SwellEngine::removeTarget(const juce::String& id,uint64_t expectedSession)
{
    // Keep the existing select-then-delete policy atomic with session restore.
    const juce::ScopedLock guard(lock);
    if(expectedSession!=sessionGeneration.load() || !find(model,id) || !permitCreative())return false;
    if(!selectTargets({id},id,expectedSession))return false;
    removeSelected();return find(model,id)==nullptr;
}
bool SwellEngine::selectTargets(const std::vector<juce::String>& ids,const juce::String& primary,uint64_t expectedSession)
{
    const juce::ScopedLock guard(lock);
    if(expectedSession && expectedSession!=sessionGeneration.load())return false;
    if(ids==model.selected && primary==model.primary)return true;
    if(!select(model,ids,primary)) return false;
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::live && !model.swells.empty())
    {performPositionEstablished=true;initialPerformOffer=0;}
    stopLocked(); return true;
}
bool SwellEngine::selectTargets(const std::vector<juce::String>& ids,const juce::String& primary)
{return selectTargets(ids,primary,0);}
bool SwellEngine::edit(const juce::String& id,const std::function<bool(Swell&,juce::String&)>& operation)
{return editChecked(id,operation);}
bool SwellEngine::edit(const juce::String& id,const std::function<bool(Swell&,juce::String&)>& operation,
                       uint64_t expectedSession,juce::String* rejection)
{return editChecked(id,operation,false,expectedSession,rejection);}
bool SwellEngine::setGainPolicy(GainPolicy policy)
{
    if(workflowState && (workflowState->snapshot().mode==workflow::Mode::finalise || workflowState->snapshot().locked()))
    {diagnostic("Gain policy needs idle Learn or Perform mode");return false;}
    const juce::ScopedLock guard(lock);actionError.clear();
    if(policy!=GainPolicy::legacy && policy!=GainPolicy::musical){actionError="Unknown gain policy";return false;}
    if(policy==model.gainPolicy && (policy==GainPolicy::legacy || model.gainRevision==approvedGainRevision))return true;
    if(policy==GainPolicy::legacy && model.liveWetDb>0){actionError="Lower Live wet to 0 dB or below before selecting Legacy gain";return false;}
    Session trial=model;trial.gainPolicy=policy;trial.gainRevision=policy==GainPolicy::musical?approvedGainRevision:legacyGainRevision;
    if(policy==GainPolicy::musical)for(auto& w:trial.swells)if(w.calibration.revision!=approvedGainRevision)w.calibration={};
    juce::String error;if(!validate(trial,error)){actionError=error;return false;}
    // Publish the future epoch through the same admission CAS before changing
    // compatibility mode. A worker copy waits for this engine lock, so it sees
    // either the whole old edit or the whole new one, never a stale slot epoch.
    if(!publishGenerationLocked(trial,UINT64_MAX,false,sessionGeneration.load()+1)){actionError=message;return false;}
    stopLocked();sessionGeneration.fetch_add(1);analysisRequest.fetch_add(1);model.gainPolicy=trial.gainPolicy;model.gainRevision=trial.gainRevision;
    if(policy==GainPolicy::musical)for(auto& w:model.swells)if(w.calibration.revision!=approvedGainRevision)w.calibration={};
    if(policy==GainPolicy::musical)for(auto& w:model.swells){
        if(w.calibration.state!=CalibrationState::unavailable && w.calibration.revision==model.gainRevision)continue;
        if(w.frozen){w.status="Musical calibration unavailable: unfreeze and restore source";continue;}
        // Checking with an unchanged raw dependency reuses the verified render;
        // forceRefresh would incorrectly request another convolution.
        w.phase=Phase::checking;w.status="Preparing musical source calibration";
    }
    ++monitoringGeneration;invalidateLiveLocked();live.configure(model.liveEnabled,model.liveRouting,model.liveWetDb);
    for(auto& [id,r]:runtime){r.preparedShape=UINT64_MAX;r.preparedMonitor=UINT64_MAX;}
    message=policy==GainPolicy::musical?"Musical gain staging selected; preparing calibrated swells":"Legacy gain staging selected";
    persistentChangedLocked();notify();return true;
}
bool SwellEngine::editChecked(const juce::String& id,const std::function<bool(Swell&,juce::String&)>& operation,
                             bool allowReset,uint64_t expectedSession,juce::String* rejection)
{
    if(!permitCreative(rejection))return false;
    const auto context=contextProvider();
    const juce::ScopedLock guard(lock);
    const auto reject=[rejection](const juce::String& reason)
    {
        if(rejection) *rejection=reason;
        return false;
    };
    if(expectedSession && expectedSession!=sessionGeneration.load())
        return reject("The swell session changed");
    auto w=find(model,id);
    if(!w) return reject("The selected swell is unavailable");
    actionError.clear();auto candidate=*w; if(!operation(candidate,message)) return reject(message);
    if(candidate.id!=id) { message="Swell identity cannot be edited"; return reject(message); }
    if(candidate.arrival==w->arrival && (candidate.sourceStart!=w->sourceStart || candidate.sourceEnd!=w->sourceEnd))candidate.supportOffset=0;
    const auto support=candidate.arrival==w->arrival && candidate.sourceStart==w->sourceStart && candidate.sourceEnd==w->sourceEnd
        && renderInputEnd(candidate)!=renderInputEnd(*w);
    const auto input=(allowReset && w->frozen && !candidate.frozen) || support || candidate.arrival!=w->arrival || candidate.sourceStart!=w->sourceStart || candidate.sourceEnd!=w->sourceEnd || candidate.ir!=w->ir;
    if(!allowReset && (candidate.frozen!=w->frozen || (w->frozen && input))){message="Unfreeze before changing source, IR or render support";return reject(message);}
    const auto shape=candidate.shape!=w->shape || candidate.placement!=w->placement;
    const auto activity=candidate.enabled!=w->enabled || candidate.muted!=w->muted || candidate.solo!=w->solo;
    if(input || shape || activity || candidate.name!=w->name)
        if(candidate.provenance==Provenance::automatic)candidate.provenance=Provenance::manualEdited;
    Session trial=model; *find(trial,id)=candidate; if(!validate(trial,message)) return reject(message);
    candidate.inputGeneration=w->inputGeneration+(input?1:0);
    candidate.shapeGeneration=w->shapeGeneration+(shape?1:0);
    if(input)
    { candidate.inputGeneration=w->inputGeneration+1; candidate.phase=Phase::stale; candidate.status="Inputs changed; preparing";candidate.calibration={};runtime[id].cachedWetOnly=false; }
    if(shape)
    {
        candidate.shapeGeneration=w->shapeGeneration+1;
        if(runtime[id].shapeError) { candidate.phase=Phase::ready; candidate.status="Preparing edited preview"; runtime[id].shapeError=false; }
        if(candidate.render.present() && candidate.ir.content!=candidate.render.irContent){
            candidate.phase=Phase::checking;candidate.status="Checking requested IR with edited shape";forceRefresh.insert(id);}
    }
    if(input || shape || activity) stopLocked();
    if(input || shape)invalidateLiveLocked(id);
    if(activity){const auto soloFilterChanged=candidate.solo!=w->solo || (w->solo && (candidate.enabled!=w->enabled || candidate.muted!=w->muted));
        if(soloFilterChanged)invalidateLiveLocked();else invalidateLiveLocked(id);}
    if(support){candidate.phase=Phase::checking;candidate.status="Preparing extended wet support";forceRefresh.insert(id);}
    if(allowReset && input){candidate.phase=Phase::checking;candidate.status="Checking Reset all dependencies";forceRefresh.insert(id);}
    if(input){protectedGenerationCalibration.clear();requestManualPreparationLocked(candidate,context);
        nextAutomaticEdit=juce::Time::getMillisecondCounterHiRes()+150;}
    if(!sameSavedSwell(*w,candidate))persistentChangedLocked();
    ++monitoringGeneration;*w=std::move(candidate);notify(); return true;
}
void SwellEngine::loadIR(const juce::File& file,bool primaryOnly)
{
    if(!permitCreative())return;
    if(file==juce::File{}) return;
    const auto context=contextProvider();
    const juce::ScopedLock guard(lock); if(model.selected.empty()) { message="Add or select a swell before loading its IR"; return; }
    Import command; command.file=file; command.session=sessionGeneration.load(); stopLocked();
    for(const auto& id:model.selected) if(auto w=find(model,id);w && !w->frozen && (!primaryOnly || id==model.primary))
    { if(!retainIRLocked(*w,context,file.getFileName()))invalidateLiveLocked(id);else ++liveRevision;
      ++w->inputGeneration; if(w->provenance==Provenance::automatic)w->provenance=Provenance::manualEdited;w->phase=Phase::checking; w->status="Importing IR"; command.targets.push_back({id,w->inputGeneration}); }
    if(command.targets.empty()){message="Unfreeze the selected swell before changing its IR";return;}
    // Latest assignment wins for overlapping target sets; obsolete imports cannot install.
    imports.push_back(std::move(command));
    while(imports.size()>32)
    {
        const auto dropped=std::move(imports.front());imports.pop_front();
        for(const auto& [id,generation]:dropped.targets)if(!cancelled(id,generation,dropped.session))
        {auto& w=*find(model,id);w.phase=Phase::error;w.status="IR import queue capacity reached; retry";}
    }
}
RegenerationResult SwellEngine::generate(bool all)
{
    const auto context=contextProvider(); const juce::ScopedLock guard(lock);
    RegenerationResult result;result.sessionEpoch=sessionGeneration.load();
    if(!permitCreative())return result;
    result.accepted=true;stopLocked();
    if(context.time.valid && context.capture.manifest.sampleRate==context.time.rate && !sameMusicalMapping(model.timing,context.time)) model.timing=context.time;
    for(auto& w:model.swells)
    {
        if(!all && std::find(model.selected.begin(),model.selected.end(),w.id)==model.selected.end())continue;
        if(w.frozen){++result.frozen;continue;}
        if(w.status=="Importing IR"){++result.importing;continue;}
        protectedGenerationCalibration.erase(w.id);forceRefresh.insert(w.id);
        ++w.inputGeneration; w.phase=Phase::queued; w.status="Queued"; w.progress=0;++result.queued;
    }
    notify();return result;
}
bool SwellEngine::assignIR(const juce::String& id,uint64_t expectedInput,const juce::File& file,bool bundled,uint64_t expectedSession)
{return queueIR(id,expectedInput,file,bundled?juce::String(defaultImpulseId):juce::String{},expectedSession);}
bool SwellEngine::assignBundledIR(const juce::String& id,uint64_t expectedInput,const juce::String& presetId,uint64_t expectedSession)
{
    if(!findBundledImpulse(presetId)){const juce::ScopedLock guard(lock);message="Unknown bundled IR identity";return false;}
    return queueIR(id,expectedInput,{},presetId,expectedSession);
}
bool SwellEngine::assignManagedIR(const juce::String& id,uint64_t expectedInput,const Asset& asset,const juce::String& name,uint64_t expectedSession)
{return queueIR(id,expectedInput,{},{},expectedSession,asset,name);}
std::vector<IRChoice> SwellEngine::irChoices() const
{
    const juce::ScopedLock guard(lock);std::vector<IRChoice> choices;
    for(const auto& entry:bundledImpulses())choices.push_back({"bundled:"+juce::String(entry.id),entry.name,entry.description,entry.id,entry.decodedHash,{}});
    const auto append=[&](const Asset& asset,const juce::String& name)
    {
        if(!asset.present() || asset.content.isEmpty() || std::any_of(choices.begin(),choices.end(),[&](const auto& item){return item.content==asset.content;}))return;
        choices.push_back({"managed:"+asset.content,name,"Imported into this instance",{},asset.content,asset});
    };
    for(const auto& entry:model.importedIRs)append(entry.asset,entry.name);
    append(model.defaultIR,model.defaultIRName);for(const auto& w:model.swells)append(w.ir,w.irName);
    for(const auto& [id,retained]:retainedIR)if(find(model,id))append(retained.accepted.ir,retained.accepted.irName);
    return choices;
}
bool SwellEngine::queueIR(const juce::String& id,uint64_t expectedInput,const juce::File& file,const juce::String& presetId,uint64_t expectedSession,const Asset& managed,const juce::String& label)
{
    if(!permitCreative())return false;
    const auto context=contextProvider();
    const juce::ScopedLock guard(lock);actionError.clear();auto w=find(model,id);
    if(!w || w->inputGeneration!=expectedInput || (expectedSession!=0 && expectedSession!=sessionGeneration.load())){message="IR assignment superseded; select the swell again";return false;}
    if(w->frozen){message="Unfreeze this swell before changing its IR";return false;}
    if((presetId.isEmpty() && file==juce::File{} && !managed.present()) || imports.size()>=32){message="IR assignment unavailable; retry";return false;}
    Import command;command.file=file;command.presetId=presetId;command.session=sessionGeneration.load();
    command.managed=managed;command.label=label;
    stopLocked();const auto* choice=presetId.isNotEmpty()?findBundledImpulse(presetId):nullptr;
    if(!retainIRLocked(*w,context,choice?juce::String(choice->name):managed.present()?label:file.getFileName()))invalidateLiveLocked(id);else ++liveRevision;
    ++w->inputGeneration;
    if(w->provenance==Provenance::automatic)w->provenance=Provenance::manualEdited;
    w->phase=Phase::checking;w->status="Importing IR";command.targets.emplace_back(id,w->inputGeneration);
    imports.push_back(std::move(command));notify();return true;
}
bool SwellEngine::resetSwell(const juce::String& id,bool allSettings)
{
    if(!permitCreative())return false;
    const auto context=contextProvider();const juce::ScopedLock guard(lock);const auto w=find(model,id);
    actionError.clear();
    if(!w)return false;
    if(!context.time.valid || context.capture.manifest.sampleRate!=context.time.rate
        || (w->render.present() && w->render.rate!=context.time.rate))
    {message="Reset needs compatible captured sample rate and host timing";return false;}
    if(!allSettings)return editChecked(id,[&](auto& candidate,auto& error){return resetWetDefaults(candidate,context.time.rate,error);});
    Reset job;job.expected=*w;job.time=context.time;job.session=sessionGeneration.load();
    if(!resetSwellDefaults(model,id,context.capture.manifest,context.time,job.candidate,message))return false;
    job.captureScope=instanceRoot(context.capture.manifest).getFullPathName();
    job.sourceRevision=sourceDependency(context.capture.manifest,job.candidate.sourceStart,job.candidate.sourceEnd);
    std::erase_if(resets,[&](const auto& previous){return previous.expected.id==id;});
    if(resets.size()>=32){message="Reset queue capacity reached; retry";return false;}
    resets.push_back(std::move(job));message="Checking captured source and default IR for Reset all";notify();return true;
}
void SwellEngine::resetJob(const Reset& job)
{
    const auto context=contextProvider();juce::String error;
    auto valid=instanceRoot(context.capture.manifest).getFullPathName()==job.captureScope
        && sameMusicalMapping(context.time,job.time)
        && sourceDependency(context.capture.manifest,job.candidate.sourceStart,job.candidate.sourceEnd)==job.sourceRevision;
    if(valid)
    {
        CaptureAssetStore captures(instanceRoot(context.capture.manifest));SourceInterval source;
        valid=readSourceInterval(captures,context.capture.manifest,job.candidate.sourceStart,job.candidate.sourceEnd,
            available()/(sizeof(float)*2),source,error,[&]{return cancelled(job.expected.id,job.expected.inputGeneration,job.session);});
    }
    if(valid){SwellAssetStore assets(instanceRoot(context.capture.manifest));ImpulseResponse impulse;valid=assets.readIR(job.candidate.ir,available(),impulse,error);}
    const auto latest=contextProvider();const juce::ScopedLock guard(lock);const auto current=find(model,job.expected.id);
    if(cancelled(job.expected.id,job.expected.inputGeneration,job.session) || !current)return;
    const auto& old=job.expected;
    // Definition edits supersede the transaction; background phase/render progress does not.
    if(current->shapeGeneration!=old.shapeGeneration || current->shape!=old.shape || current->placement!=old.placement
        || current->name!=old.name || current->frozen!=old.frozen || current->enabled!=old.enabled || current->muted!=old.muted
        || current->solo!=old.solo || current->ir!=old.ir || current->arrival!=old.arrival || current->sourceStart!=old.sourceStart
        || current->sourceEnd!=old.sourceEnd || model.defaultIR!=job.candidate.ir)
    {actionError=message="Reset all superseded by a newer edit";return;}
    valid=valid && instanceRoot(latest.capture.manifest).getFullPathName()==job.captureScope
        && sameMusicalMapping(latest.time,job.time)
        && sourceDependency(latest.capture.manifest,job.candidate.sourceStart,job.candidate.sourceEnd)==job.sourceRevision;
    if(!valid){actionError=message=error.isNotEmpty()?"Reset all: "+error:"Reset all needs unchanged compatible capture and timing";return;}
    editChecked(old.id,[&](auto& value,auto&){auto candidate=job.candidate;candidate.render=value.render;value=std::move(candidate);return true;},true);
}
void SwellEngine::analyze(int64_t start,int64_t end)
{
    if(!permitCreative())return;
    const juce::ScopedLock guard(lock);
    if(end<=start) { message="Analysis needs a positive covered range"; return; }
    const auto token=analysisRequest.fetch_add(1)+1;
    analysis=Analysis{start,end,model.sensitivity,model.separationSeconds,sessionGeneration.load(),token};
    analyzing=true; message="Analyzing covered source";
}
void SwellEngine::setPreferences(float db,float sensitivity,double separation,bool automatic,uint64_t expectedPass)
{
    const juce::ScopedLock guard(lock);
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::finalise && (sensitivity!=model.sensitivity || separation!=model.separationSeconds || automatic!=model.autoRegenerate))
    {message="Creative preferences are locked in Export";return;}
    if(!std::isfinite(db) || !std::isfinite(sensitivity) || !std::isfinite(separation)) { message="Preferences must be finite"; return; }
    sensitivity=std::clamp(sensitivity,0.0f,1.0f);separation=std::clamp(separation,0.001,2.0);
    const auto changed=sensitivity!=model.sensitivity || separation!=model.separationSeconds;
    if(changed || automatic!=model.autoRegenerate)
    {
        auto next=model;next.sensitivity=sensitivity;next.separationSeconds=separation;
        if(!publishGenerationLocked(next,expectedPass)){actionError=message;return;}actionError.clear();
    }
    const auto savedChanged=model.auditionDb!=std::clamp(db,-60.0f,0.0f) || model.sensitivity!=std::clamp(sensitivity,0.0f,1.0f)
        || model.separationSeconds!=std::clamp(separation,0.001,2.0) || model.autoRegenerate!=automatic;
    const auto learning=workflowState && workflowState->snapshot().mode==workflow::Mode::capture;
    if(changed){++markerGeneration;invalidateLearnLocked();nextMarkerAnalysis=juce::Time::getMillisecondCounterHiRes()+200;}
    if(changed && !learning && model.automaticProcessing && model.autoMarkersEnabled){const auto context=contextProvider();for(const auto& r:context.capture.manifest.ranges)
        {int64_t end;if(r.frameCount<=INT64_MAX && checkedAdd(r.startSample,static_cast<int64_t>(r.frameCount),end))dirtyLocked(r.startSample,end);}}
    model.auditionDb=std::clamp(db,-60.0f,0.0f); model.sensitivity=std::clamp(sensitivity,0.0f,1.0f);
    model.separationSeconds=std::clamp(separation,0.001,2.0); model.autoRegenerate=automatic;
    if(savedChanged)persistentChangedLocked();
    voice.setLevel(juce::Decibels::decibelsToGain(model.auditionFollowsLive?0.0f:model.auditionDb));notify();
}
bool SwellEngine::initializeExport()
{
    const juce::ScopedLock guard(lock);
    if(!workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise || workflowState->snapshot().locked())return false;
    const auto before=model.exportState;
    if(!m3::initializeExport(model,message))return false;
    if(model.exportState!=before){persistentChangedLocked();notify();}return true;
}
bool SwellEngine::setPerformNavigation(PerformNavigationState preferences,uint64_t expectedSession)
{
    const juce::ScopedLock guard(lock);
    if(expectedSession!=sessionGeneration.load())return false;
    preferences=normalizedPerformNavigation(preferences);
    if(preferences!=model.performUi){model.performUi=preferences;persistentChangedLocked();notify();}
    return true;
}
bool SwellEngine::setExportMix(float wet,uint64_t expectedSession,uint64_t expectedPass)
{return setExportLevels(1-wet,wet,expectedSession,expectedPass);}
bool SwellEngine::setExportLevels(float dry,float wet,uint64_t expectedSession,uint64_t expectedPass)
{
    const juce::ScopedLock guard(lock);
    if(expectedSession!=sessionGeneration.load() || !workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise || workflowState->snapshot().locked())return false;
    if(expectedPass!=UINT64_MAX && workflowState->snapshot().pass!=expectedPass)return false;
    const auto before=model.exportState;
    if(!m3::setExportLevels(model,dry,wet,message))return false;
    if(before!=model.exportState){auto previous=exportState.prepared?exportState.prepared:exportState.previous;auto detail=exportState.detail;exportRequest.fetch_add(1);detailSerial.fetch_add(1);detailWork.reset();if((dry==0 && wet==0) || (wet>0 && !exportPreviewWetAvailable))exportVoice->stop();else if(exportVoice->requested(exportVoice->request()) && exportPreviewPin && exportPreviewSession==sessionGeneration.load() && exportPreviewPass==workflowState->snapshot().pass && exportPreviewRevision==liveRevision)exportVoice->setLevels(exportVoice->request(),dry,wet,exportPreviewPin->plan->end);else exportVoice->stop();exportState={dry==0 && wet==0?ExportPhase::idle:ExportPhase::preparing,dry==0 && wet==0?"Silent: Dry 0% / Wet 0%":"Preparing export mix",{}};exportState.previous=std::move(previous);exportState.detail=std::move(detail);exportState.session=sessionGeneration.load();exportState.pass=workflowState->snapshot().pass;persistentChangedLocked();notify();}return true;
}
bool SwellEngine::audition()
{
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::finalise)return playExport(sessionGeneration.load());
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::capture)return false;
    if((!workflowState || workflowState->snapshot().mode==workflow::Mode::live) && liveEnabled.load() && transportActive.load())return false;
    const juce::ScopedLock guard(lock); const auto w=find(model,model.primary); const auto at=runtime.find(model.primary);
    if(!w || at==runtime.end() || !at->second.monitor || at->second.preparedMonitor!=monitoringGeneration
        || at->second.preparedShape!=w->shapeGeneration || at->second.preparedRender!=w->render.content) return false;
    auto playable=*w;if(at->second.cachedWetOnly && model.auditionMode==AuditionMode::wetOnly)playable.phase=Phase::ready;
    if(playable.phase!=Phase::ready || (model.auditionMode==AuditionMode::dryWet && !at->second.dryAvailable))return false;
    const auto context=contextProvider();
    if(!sameMusicalMapping(model.timing,context.time) || context.capture.manifest.sampleRate!=context.time.rate) return false;
    setExpected(context.time); audioAllowed=true;
    return voice.start(at->second.monitor,1,true);
}
void SwellEngine::stop() { const juce::ScopedLock guard(lock);
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::finalise && (exportState.prepared || exportPreviewPin))
        if(const auto at=exportVoice->position())exportState.previewPosition=*at;
    voice.stop();exportVoice->stop(); }
void SwellEngine::processAudio(float* left,float* right,int frames,const SongTime& time) noexcept
{
    auto allowed=audioAllowed.load() && (!workflowState || workflowState->snapshot().mode!=workflow::Mode::capture);
    if(allowed && (!time.valid || time.rate!=expectedRate.load() || time.bpm!=expectedBpm.load()
        || time.numerator!=expectedNumerator.load() || time.denominator!=expectedDenominator.load()))
    { if(time.valid)timingChanged=true; allowed=false; audioAllowed=false; }
    voice.process(left,right,frames,time.rate,allowed);
    const auto exportAllowed=workflowState && workflowState->snapshot().mode==workflow::Mode::finalise
        && time.valid && time.rate==expectedRate.load() && time.bpm==expectedBpm.load()
        && time.numerator==expectedNumerator.load() && time.denominator==expectedDenominator.load();
    exportVoice->process(left,right,frames,time.rate,exportAllowed);
}
void SwellEngine::processTransportAudio(float* left,float* right,int frames,const TransportTime& time) noexcept
{
    transportActive.store(time.playing,std::memory_order_relaxed);
    hostObserved.store(true,std::memory_order_release);
    const auto session=sessionGeneration.load(std::memory_order_acquire);
    const auto clock=liveClock.observe(time,frames,session);
    if(previousTransportSession!=session){previousTransportPlaying=false;previousTransportSession=session;}
    const auto publishPass=[&](int64_t start,int64_t end){if(start>=end)return;
        if(passSession.load()==session && passStart.load()==start && passEnd.load()==end)return;
        passSequence.fetch_add(1,std::memory_order_acq_rel);passStart=start;passEnd=end;passSession=session;passSequence.fetch_add(1,std::memory_order_release);};
    int64_t blockEnd;
    if(time.song.valid && checkedAdd(time.song.sample,frames,blockEnd)){
        if(time.looping && time.loopEnd>time.loopStart)publishPass(time.loopStart,time.loopEnd);
        if(time.playing && previousTransportPlaying && time.song.sample<previousTransportTime.sample
            && transportMappingMatches(previousTransportTime,time.song)){
            if(!time.looping || time.loopEnd<=time.loopStart)publishPass(currentPassStart,previousTransportEnd);currentPassStart=time.song.sample;
        }else if(time.playing && (!previousTransportPlaying || time.song.sample!=previousTransportEnd))currentPassStart=time.song.sample;
        previousTransportTime=time.song;previousTransportEnd=blockEnd;previousTransportPlaying=time.playing;
    }else previousTransportPlaying=false;
    loopSequence.fetch_add(1,std::memory_order_acq_rel);loopHint=time.looping;loopHintStart=time.loopStart;loopHintEnd=time.loopEnd;loopSequence.fetch_add(1,std::memory_order_release);
    const auto modeAllows=!workflowState || workflowState->snapshot().mode==workflow::Mode::live;
    const auto priority=modeAllows && liveEnabled.load() && time.playing;
    if(priority && !songPriority)voice.stop();songPriority=priority;
    live.process(left,right,frames,time,modeAllows,&clock);
    processAudio(left,right,frames,time.song);
}
void SwellEngine::captureStopped() noexcept
{
    if(!previousTransportPlaying || currentPassStart>=previousTransportEnd)return;
    passSequence.fetch_add(1,std::memory_order_acq_rel);passStart=currentPassStart;passEnd=previousTransportEnd;
    passSession=sessionGeneration.load(std::memory_order_acquire);passSequence.fetch_add(1,std::memory_order_release);
}
void SwellEngine::recaptureCompleted(int64_t a,int64_t b)
{
    const auto context=contextProvider();const juce::ScopedLock guard(lock);
    for(auto& w:model.swells) if(!w.frozen && w.sourceStart<b && renderInputEnd(w)>a
        && captureAllowsAutomatic(context.capture.manifest,std::max(a,w.sourceStart),std::min(b,renderInputEnd(w))))
    { forceRefresh.insert(w.id); if(w.ir.present()){w.phase=Phase::checking;w.status="Checking completed recapture";} if(w.id==model.primary)stopLocked(); }
}
uint64_t SwellEngine::occupiedLocked() const
{
    uint64_t bytes=voice.pinnedBytes()+live.pinnedBytes()+(offlinePlayer?offlinePlayer->pinnedBytes():0); std::set<const WetRender*> counted;
    for(const auto& [id,r]:runtime)
    {
        if(r.raw && counted.insert(r.raw.get()).second) bytes+=r.raw->samples.capacity()*sizeof(float);
        // Voice pins are deliberately conservative: double-counting avoids oversubscription.
        if(r.shaped && counted.insert(r.shaped.get()).second) bytes+=r.shaped->samples.capacity()*sizeof(float);
        if(r.monitor && counted.insert(r.monitor.get()).second)bytes+=r.monitor->samples.capacity()*sizeof(float);
        if(r.rawWave) bytes+=r.rawWave->peaks.capacity()*sizeof(CapturePeak);
        if(r.shapeWave) bytes+=r.shapeWave->peaks.capacity()*sizeof(CapturePeak);
        if(r.sourceWave) bytes+=r.sourceWave->peaks.capacity()*sizeof(CapturePeak);
    }
    std::set<const PreparedExport*> countedExports;
    const auto countExport=[&](const std::shared_ptr<const PreparedExport>& p){if(p && countedExports.insert(p.get()).second)bytes+=(p->metrics.dryPeaks.capacity()+p->metrics.wetPeaks.capacity())*sizeof(CapturePeak);};
    countExport(exportState.prepared);countExport(exportState.previous);countExport(exportState.preview);countExport(exportPreviewPin);if(detailWork)countExport(detailWork->accepted);
    if(exportState.detail)bytes+=(exportState.detail->dry.capacity()+exportState.detail->wet.capacity())*sizeof(CapturePeak);
    // Reserve bounded metadata still held by an editor between snapshot refreshes.
    return bytes+activeJobBytes+exportPreviewBytes.load()+exportVoice->workingBytes()+exportPageFrames*4*sizeof(float)+16384+1024*1024
        +(workflowState?maximumStateBytes*3:0); // Learn metadata, including an editor-held snapshot; absent in standalone engines.
}
uint64_t SwellEngine::residentBytes() const { const juce::ScopedLock guard(lock); return occupiedLocked(); }
uint64_t SwellEngine::available(uint64_t extra)
{
    const juce::ScopedLock guard(lock); voice.collect();
    live.collect();
    uint64_t peakBytes=0;std::vector<std::pair<uint64_t,juce::String>> peakOrder;
    for(auto& [id,r]:runtime)
    {
        if(id!=model.primary){r.raw.reset();r.shaped.reset();r.monitor.reset();r.sourceWave.reset();peakOrder.push_back({r.lastUse,id});}
        if(r.rawWave)peakBytes+=r.rawWave->peaks.capacity()*sizeof(CapturePeak);
        if(r.shapeWave)peakBytes+=r.shapeWave->peaks.capacity()*sizeof(CapturePeak);
    }
    if(peakBytes>16ULL*1024*1024)
    {
        std::sort(peakOrder.begin(),peakOrder.end());
        for(const auto& [age,id]:peakOrder)
        {
            auto& r=runtime[id];if(r.rawWave){peakBytes-=r.rawWave->peaks.capacity()*sizeof(CapturePeak);r.rawWave.reset();}
            if(r.shapeWave){peakBytes-=r.shapeWave->peaks.capacity()*sizeof(CapturePeak);r.shapeWave.reset();}
            if(peakBytes<=16ULL*1024*1024)break;
        }
    }
    const auto occupied=occupiedLocked();
    return occupied>=previewMemoryBudget || extra>=previewMemoryBudget-occupied ? 0 : previewMemoryBudget-occupied-extra;
}
bool SwellEngine::cancelled(const juce::String& id,uint64_t input,uint64_t session) const
{
    if(threadShouldExit() || sessionGeneration.load()!=session) return true;
    const juce::ScopedLock guard(lock); const auto w=find(model,id); return !w || w->inputGeneration!=input;
}
void SwellEngine::report(const Swell& requested,uint64_t session,Phase phase,const juce::String& status,float progress)
{
    const juce::ScopedLock guard(lock);
    if(cancelled(requested.id,requested.inputGeneration,session)) return;
    auto& w=*find(model,requested.id); w.phase=phase; w.status=status; w.progress=progress;
    if(phase==Phase::error || phase==Phase::stale || phase==Phase::rendering){
        if(retainedIR.contains(requested.id))++liveRevision;else invalidateLiveLocked(requested.id);}
}
std::shared_ptr<Waveform> SwellEngine::waveform(const WetRender& audio)
{
    auto wave=std::make_shared<Waveform>(); wave->start=audio.start; wave->end=audio.start+static_cast<int64_t>(audio.frames()); wave->peak=audio.peak;
    const auto stride=std::max(size_t{1},(audio.frames()+2047)/2048);
    for(size_t at=0;at<audio.frames();at+=stride)
    {
        const auto count=std::min(stride,audio.frames()-at);
        const auto bounds=std::minmax_element(audio.samples.begin()+at*2,audio.samples.begin()+(at+count)*2);
        wave->peaks.push_back({audio.start+static_cast<int64_t>(at),static_cast<uint32_t>(count),*bounds.first,*bounds.second});
    }
    return wave;
}
void SwellEngine::inspect(const PreviewContext& context)
{
    const juce::ScopedLock guard(lock); voice.collect();
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::finalise && !model.exportState.initialized)initializeExport();
    if(context.capture.status.startsWith("Loading") || (awaitingCaptureReference.isNotEmpty() && context.capture.reference!=awaitingCaptureReference)) return;
    awaitingCaptureReference.clear();
    // A reader can temporarily miss the host's coherent timing publication.
    // That gates playback/preparation, but does not change any captured asset.
    long double origin;
    if(!songOrigin(context.time,origin))return;
    if(timingChanged.exchange(false) || (model.timing.valid && !sameMusicalMapping(model.timing,context.time)))
    {
        invalidateLiveLocked();liveCache.clear();
        stopLocked(); for(auto& w:model.swells) if(w.phase==Phase::ready || w.phase==Phase::rendering || w.phase==Phase::queued || w.phase==Phase::checking || w.phase==Phase::restoring)
        { ++w.inputGeneration; w.phase=Phase::stale; w.status="Host timing changed; checking dependencies"; }
        if(!context.capture.unavailable && context.capture.manifest.sampleRate==context.time.rate
            && std::any_of(model.swells.begin(),model.swells.end(),[](const auto& w){return !w.frozen;}))model.timing=context.time;
    }
    const auto scope=juce::String(context.capture.manifest.projectId)+"/"+context.capture.manifest.instanceId;
    // A newly confirmed/removed neighbour can change the local dry reference
    // without changing capture PCM or this swell's extraction definition.
    if(model.gainPolicy==GainPolicy::musical && isEnergyRevision(model.gainRevision)){
        std::vector<int64_t> arrivals;arrivals.reserve(model.swells.size());for(const auto& w:model.swells)arrivals.push_back(w.arrival);
        std::sort(arrivals.begin(),arrivals.end());
        for(auto& w:model.swells)if(!w.frozen && !protectedGenerationCalibration.contains(w.id) && w.phase==Phase::ready && isEnergyRevision(w.calibration.revision)){
            int64_t end;if(!checkedAdd(w.arrival,std::max<int64_t>(1,std::llround(context.time.rate*candidateAttackSeconds)),end))continue;
            end=std::min(end,w.sourceEnd);const auto next=std::upper_bound(arrivals.begin(),arrivals.end(),w.arrival);if(next!=arrivals.end())end=std::min(end,*next);
            if(end!=w.calibration.referenceEnd){w.calibration={};w.phase=Phase::checking;w.status="Checking changed energy reference";
                ++monitoringGeneration;runtime[w.id].preparedShape=UINT64_MAX;invalidateLiveLocked(w.id);stopLocked();}
        }
    }
    // Timing can recover without a new capture revision (including after Clear).
    // Re-admit only matching frozen assets to the existing worker verification;
    // metadata compatibility alone must never mark their audio Ready.
    for(auto& w:model.swells)
        if(w.frozen && w.phase==Phase::stale && w.render.rate==context.time.rate && w.render.bpm==context.time.bpm
            && w.render.numerator==context.time.numerator && w.render.denominator==context.time.denominator)
        {w.phase=Phase::checking;w.status="Checking frozen render";}
    if(captureReference==context.capture.reference && captureScope==scope && captureGeneration==context.captureGeneration) return;
    const auto cleared=captureScope.isNotEmpty() && (captureScope!=scope || captureGeneration!=context.captureGeneration);
    acceptedSourceLocked(context.capture.manifest);
    captureReference=context.capture.reference; captureScope=scope; captureGeneration=context.captureGeneration;
    for(auto& w:model.swells)
    {
        auto& r=runtime[w.id]; const auto revision=juce::String(sourceDependency(context.capture.manifest,w.sourceStart,renderInputEnd(w)));
        if(w.frozen){r.revision=revision;
            int64_t dryEnd;
            if(w.id==model.primary && model.auditionMode==AuditionMode::dryWet && r.monitorRevision.isNotEmpty()
                && auditionDryEnd(w,context.time.rate,model.postRollSeconds,model.isolateDry,dryEnd)
                && r.monitorRevision!=juce::String(sourceDependency(context.capture.manifest,w.sourceStart,dryEnd))){
                r.preparedMonitor=UINT64_MAX;++monitoringGeneration;stopLocked();}
            continue;}
        if(r.revision!=revision){if(retainedIR.contains(w.id))invalidateLiveLocked(w.id);const auto anchor=runtime.find(model.primary);
            if(anchor!=runtime.end() && std::find(anchor->second.members.begin(),anchor->second.members.end(),w.id)!=anchor->second.members.end()){++monitoringGeneration;stopLocked();}}
        // Import owns an explicit assignment, not the source revision. Its
        // input generation already cancelled previous render publication.
        if(w.status=="Importing IR"){r.revision=revision;continue;}
        if(w.phase!=Phase::restoring && (cleared || r.revision!=revision))
        {++w.inputGeneration;w.phase=Phase::stale;w.status="Accepted source changed; checking dependencies";if(w.id==model.primary)stopLocked();}
        r.revision=revision;
        int64_t dryEnd;
        if(w.id==model.primary && r.monitorRevision.isNotEmpty() && auditionDryEnd(w,context.time.rate,model.postRollSeconds,model.isolateDry,dryEnd)
            && r.monitorRevision!=juce::String(sourceDependency(context.capture.manifest,w.sourceStart,dryEnd))){r.preparedMonitor=UINT64_MAX;++monitoringGeneration;stopLocked();}
    }
}
void SwellEngine::importJob(const Import& job)
{
    if(job.futureDefault)
    {
        defaultImportWaiting.store(true);
        while(defaultImportHold.load() && !threadShouldExit() && job.session==sessionGeneration.load())wait(1);
        defaultImportWaiting.store(false);
        const juce::ScopedLock guard(lock);if(threadShouldExit() || job.session!=sessionGeneration.load() || job.defaultRequest!=defaultRequest)return;
    }
    const auto context=contextProvider(); SwellAssetStore store(instanceRoot(context.capture.manifest)); Asset asset; juce::String error;
    const auto budget=available();
    const auto bundled=job.presetId.isNotEmpty();
    ImpulseResponse managedIR;
    auto ok=bundled?installBundledImpulse(store,job.presetId,budget,asset,error):job.managed.present()
        ?store.readIR(job.managed,budget,managedIR,error):store.importIR(job.file,budget,asset,error);
    if(ok && job.managed.present())asset=job.managed;
    const auto* descriptor=bundled?findBundledImpulse(job.presetId):nullptr;
    const auto label=descriptor?juce::String(descriptor->name):job.managed.present()?job.label:job.file.getFileName();
    const juce::ScopedLock guard(lock);
    if(job.session!=sessionGeneration.load())return;
    // Admit history only together with a still-current successful assignment.
    // Bundled responses already have permanent catalogue entries.
    const auto remember=[&](Session& target){
        if(bundled || std::any_of(target.importedIRs.begin(),target.importedIRs.end(),[&](const auto& entry){return entry.asset.content==asset.content;}))return true;
        if(target.importedIRs.size()>=maximumImportedIRs){error="Imported IR collection is full; previous IR retained";return false;}
        target.importedIRs.push_back({asset,label});juce::String encoded;
        if(!encode(target,encoded,error)){target.importedIRs.pop_back();return false;}return true;
    };
    if(job.futureDefault)
    {
        if(job.defaultRequest!=defaultRequest)return;defaultPending=false;
        if(workflowState){const auto flow=workflowState->snapshot();if(flow.locked() || flow.mode==workflow::Mode::finalise || flow.pass!=job.workflowPass)
            {defaultStatus="Default IR request superseded by workflow; previous default retained";message=defaultStatus;return;}}
        auto next=model;next.defaultIR=asset;next.defaultIRName=label;
        if(ok)ok=remember(next);
        if(ok && !publishGenerationLocked(next,job.workflowPass))
        {defaultStatus="Default IR request superseded by playback; previous default retained";message=defaultStatus;return;}
        if(ok){if(model.defaultIR!=asset || model.defaultIRName!=label || model.importedIRs.size()!=next.importedIRs.size())persistentChangedLocked();
            model=std::move(next);defaultError.clear();defaultStatus="Default IR for the next Generate updated";}
        else{defaultError=error;defaultStatus="Unable to change Default IR; previous default retained";}
        message=ok?defaultStatus:defaultStatus+": "+error;return;
    }
    const auto hasTarget=std::any_of(job.targets.begin(),job.targets.end(),[&](const auto& target){
        return !cancelled(target.first,target.second,job.session) && !find(model,target.first)->frozen;});
    const auto oldSize=model.importedIRs.size();if(ok && hasTarget)ok=remember(model);
    if(model.importedIRs.size()!=oldSize)persistentChangedLocked();
    for(const auto& [id,generation]:job.targets)
    {
        if(cancelled(id,generation,job.session)) continue; auto& w=*find(model,id);if(w.frozen)continue;
        if(ok) {
            const auto kept=retainedIR.find(id);
            const auto identical=kept!=retainedIR.end() && retainedCompatible(kept->second,w,context)
                && kept->second.accepted.ir.content==asset.content && w.render==kept->second.accepted.render;
            if(w.ir!=asset || w.irName!=label)persistentChangedLocked();
            w.ir=asset;w.irName=label;w.phase=identical?Phase::ready:Phase::stale;
            w.status=identical?"Ready (identical IR content)":"IR: "+label+"; preparing";if(identical)++liveRevision;
            else requestManualPreparationLocked(w,context);}
        else { w.phase=Phase::error; w.status=error; }
    }
    message=ok ? "Managed IR imported: "+label : error;
    nextAutomaticEdit=juce::Time::getMillisecondCounterHiRes()+150;
}
void SwellEngine::analysisJob(const Analysis& job)
{
    const auto context=contextProvider(); CaptureAssetStore store(instanceRoot(context.capture.manifest)); juce::String error; std::vector<Candidate> found;
    const auto cancel=[&]{return threadShouldExit() || job.session!=sessionGeneration.load() || job.request!=analysisRequest.load();};
    // Analysis decode/FFT scratch is bounded below the same working allowance.
    const auto ok=available()>=64ULL*1024*1024 && suggestOnsets(store,context.capture.manifest,job.start,job.end,job.sensitivity,job.separation,found,error,cancel);
    const auto latest=contextProvider(); const juce::ScopedLock guard(lock);
    if(cancel()) return;
    analyzing=false;
    if(!ok || latest.capture.reference!=context.capture.reference) { message=error.isEmpty()?"Analysis superseded or working budget unavailable":error; return; }
    const auto tolerance=static_cast<int64_t>(context.capture.manifest.sampleRate*job.separation);
    std::erase_if(found,[&](const auto& c){return std::any_of(model.swells.begin(),model.swells.end(),[&](const auto& w){return std::abs(static_cast<long double>(c.sample)-w.arrival)<tolerance;});});
    candidates=std::move(found); message=juce::String(static_cast<int>(candidates.size()))+" onset suggestions; confirm or add targets";
}
void SwellEngine::renderJob(Swell requested,const PreviewContext& context,uint64_t session,bool restoreOrCheck)
{
    const auto renderEpoch=learnRenderCancellation.load();
    bool pendingAtStart;
    {const juce::ScopedLock guard(lock);pendingAtStart=learnRequest.has_value();}
    const auto superseded=[&]{return requested.provenance==Provenance::automatic && !requested.frozen
        && (pendingAtStart || renderEpoch!=learnRenderCancellation.load());};
    struct Finish
    {
        SwellEngine& engine;const Swell& requested;uint64_t session,epoch;bool pending;
        ~Finish()
        {
            engine.renderPublicationWaiting=false;
            if(requested.provenance!=Provenance::automatic || requested.frozen || (!pending && epoch==engine.learnRenderCancellation.load()))return;
            const juce::ScopedLock guard(engine.lock);auto* current=find(engine.model,requested.id);
            if(session!=engine.sessionGeneration.load() || !current || current->inputGeneration!=requested.inputGeneration
                || current->provenance!=Provenance::automatic || current->frozen)return;
            if(current->phase==Phase::rendering || current->phase==Phase::checking)
            {current->phase=Phase::checking;current->progress=0;current->status="Queued after Generate Swells";}
        }
    }finish{*this,requested,session,renderEpoch,pendingAtStart};
    if(superseded())return;
    // Cooperative page servicing keeps accepted audio and its neighbours resident
    // during long resampling/FFT work. Reserve the complete in-flight workspace
    // while nested page preparation runs; it must not spend the FFT's memory.
    uint64_t reservation=previewMemoryBudget;int64_t sourceFrames=0;juce::String reservationError;
    const auto preparedFrames=std::round(requested.ir.frames*context.time.rate/requested.ir.rate);
    uint64_t required=0;
    if(checkedSubtract(renderInputEnd(requested),requested.sourceStart,sourceFrames) && sourceFrames>0
        && std::isfinite(preparedFrames) && preparedFrames>0 && preparedFrames<previewMemoryBudget/8
        && estimateRenderMemory(static_cast<uint64_t>(sourceFrames),static_cast<uint64_t>(preparedFrames),0,required,reservationError))
        reservation=std::min(previewMemoryBudget,required+static_cast<uint64_t>(std::max(int64_t{0},requested.ir.frames))*requested.ir.channels*sizeof(float));
    double lastPageService=0;bool pagePressure=false;
    const auto cancel=[&]{
        if(superseded() || pagePressure || cancelled(requested.id,requested.inputGeneration,session))return true;
        const auto now=juce::Time::getMillisecondCounterHiRes();
        if(!offlineActive.load() && now-lastPageService>=40){lastPageService=now;
            {const juce::ScopedLock guard(lock);activeJobBytes=reservation;}
            try{prepareTransport(contextProvider());}
            catch(...){const juce::ScopedLock guard(lock);activeJobBytes=0;throw;}
            {const juce::ScopedLock guard(lock);activeJobBytes=0;
                if(retainedIR.contains(requested.id) && liveStatus.containsIgnoreCase("budget")){
                    pagePressure=true;report(requested,session,Phase::error,"IR replacement budget unavailable; accepted audio retained. Retry stopped or with fewer overlapping swells.");}}}
        return superseded() || pagePressure || cancelled(requested.id,requested.inputGeneration,session);};
    const auto failure=[&](const juce::String& error){report(requested,session,Phase::error,error);};
    if(requested.frozen){
        const auto& asset=requested.render;
        if(!context.time.valid || asset.rate!=context.time.rate || asset.bpm!=context.time.bpm
            || asset.numerator!=context.time.numerator || asset.denominator!=context.time.denominator){
            report(requested,session,Phase::stale,"Frozen render needs original host timing");return;}
        SwellAssetStore assets(instanceRoot(context.capture.manifest));juce::String error;
        if(!assets.verify(asset,error)){failure("Frozen render unavailable: "+error);return;}
        auto raw=std::make_shared<WetRender>();
        if(!assets.readRender(asset,available(),*raw,error)){failure(error);return;}
        const juce::ScopedLock guard(lock);if(cancel())return;
        auto& r=runtime[requested.id];r.verifiedRender=asset;r.cachedWetOnly=false;r.rawWave=waveform(*raw);
        if(requested.id==model.primary)r.raw=std::move(raw);
        r.preparedShape=UINT64_MAX;r.preparedMonitor=UINT64_MAX;
        r.revision=juce::String(sourceDependency(context.capture.manifest,requested.sourceStart,renderInputEnd(requested)));
        if(!sameMusicalMapping(model.timing,context.time))model.timing=context.time;
        auto& w=*find(model,requested.id);w.phase=Phase::ready;w.progress=1;w.status="Frozen render";
        if(model.gainPolicy==GainPolicy::musical && w.calibration.state==CalibrationState::unavailable)
            w.status+=" | Calibration unavailable: unfreeze and restore source";
        ++liveRevision;forceRefresh.erase(w.id);return;
    }
    if(!requested.ir.present()) { failure("Assign an IR before generation"); return; }
    if(!context.time.valid || context.time.rate!=context.capture.manifest.sampleRate)
    { report(requested,session,Phase::stale,"Awaiting compatible host timing / capture rate"); return; }
    if(restoreOrCheck)
    {
        const juce::ScopedLock guard(lock);
        if(model.timing.valid && !sameMusicalMapping(model.timing,context.time))
        { report(requested,session,Phase::stale,"Saved timing differs; verify and regenerate"); return; }
    }
    CaptureAssetStore sourceStore(instanceRoot(context.capture.manifest)); SwellAssetStore assets(instanceRoot(context.capture.manifest));
    {
        const juce::ScopedLock guard(lock);int64_t covered=requested.sourceStart;
        for(const auto& range:context.capture.manifest.ranges)
        {int64_t end;if(range.frameCount>INT64_MAX || !checkedAdd(range.startSample,static_cast<int64_t>(range.frameCount),end))continue;
            if(range.startSample>covered)break;covered=std::max(covered,end);if(covered>=renderInputEnd(requested))break;}
        if((model.automaticProcessing || renderInputEnd(requested)>requested.sourceEnd) && covered<renderInputEnd(requested) && (!requested.render.present()
            || requested.render.sourceStart!=requested.sourceStart || requested.render.sourceEnd!=renderInputEnd(requested)))
        {report(requested,session,Phase::stale,"Waiting for source");return;}
    }
    juce::String error,dependency;
    // Release inactive selected buffers before a replacement; voice pins retire separately.
    if(!restoreOrCheck)
    {
        { const juce::ScopedLock guard(lock); auto& r=runtime[requested.id]; r.raw.reset(); r.shaped.reset(); }
        int64_t sourceLength;
        const auto preparedLength=std::round(requested.ir.frames*context.time.rate/requested.ir.rate);
        uint64_t required;
        if(!checkedSubtract(renderInputEnd(requested),requested.sourceStart,sourceLength) || sourceLength<=0
            || !std::isfinite(preparedLength) || preparedLength<1 || preparedLength>=static_cast<double>(INT64_MAX)
            || !estimateRenderMemory(static_cast<uint64_t>(sourceLength),static_cast<uint64_t>(preparedLength),
                previewMemoryBudget-available(),required,error))
        { failure(error.isEmpty()?"Invalid source/IR length":error); return; }
    }
    if(available()<64ULL*1024*1024) { failure("256 MiB working budget: source verification needs decode scratch"); return; }
    report(requested,session,Phase::checking,"Verifying source and IR",0.02f);
    if(!sourceFingerprint(sourceStore,context.capture.manifest,requested.sourceStart,renderInputEnd(requested),dependency,error,cancel))
    {
        WetRender recovered;juce::String verifyError;
        if(restoreOrCheck && requested.render.present() && requested.render.rate==context.time.rate
            && requested.render.irContent==requested.ir.content && requested.render.sourceStart==requested.sourceStart
            && requested.render.sourceEnd==renderInputEnd(requested) && assets.verify(requested.ir,verifyError)
            && assets.readRender(requested.render,available(),recovered,verifyError))
        {
            const juce::ScopedLock guard(lock);if(cancel())return;
            auto& r=runtime[requested.id];r.cachedWetOnly=true;r.raw=std::make_shared<WetRender>(std::move(recovered));
            r.rawWave=waveform(*r.raw);r.preparedShape=UINT64_MAX;r.preparedRender.clear();
            r.revision=juce::String(sourceDependency(context.capture.manifest,requested.sourceStart,renderInputEnd(requested)));
            if(!sameMusicalMapping(model.timing,context.time))model.timing=context.time;
            auto& w=*find(model,requested.id);w.phase=Phase::error;w.status="Missing source: last-committed Wet Only available";
        }
        else if(!cancel())failure(error);return;
    }
    if(!assets.verify(requested.ir,error)){if(!cancel())failure("Missing IR / invalid asset: "+error);return;}
    if(cancel()) return;
    const auto renderDependency=juce::String(sourceEdgeRevision)+dependency;
    // Legacy assets retain their saved sound until an explicit regeneration.
    const auto matching=requested.render.present()
        && (requested.render.dependency==dependency || requested.render.dependency==renderDependency)
        && requested.render.irContent==requested.ir.content && requested.render.sourceStart==requested.sourceStart
        && requested.render.sourceEnd==renderInputEnd(requested) && requested.render.rate==context.time.rate
        && requested.render.bpm==context.time.bpm && requested.render.numerator==context.time.numerator
        && requested.render.denominator==context.time.denominator;
    bool alreadyVerified=false;
    {const juce::ScopedLock guard(lock);const auto at=runtime.find(requested.id);
        const auto calibrationReady=model.gainPolicy==GainPolicy::legacy
            || (requested.calibration.state!=CalibrationState::unavailable
                && requested.calibration.irIdentity==requested.ir.content
                && requested.calibration.renderIdentity==requested.render.content
                && requested.calibration.revision==model.gainRevision);
        alreadyVerified=matching && calibrationReady && at!=runtime.end() && !at->second.cachedWetOnly && at->second.verifiedRender==requested.render;}
    if(restoreOrCheck && alreadyVerified && assets.verify(requested.render,error)){
        const auto latest=contextProvider();
        if(latest.captureGeneration!=context.captureGeneration || !sameMusicalMapping(context.time,latest.time)
            || sourceDependency(latest.capture.manifest,requested.sourceStart,renderInputEnd(requested))!=sourceDependency(context.capture.manifest,requested.sourceStart,renderInputEnd(requested)))return;
        const juce::ScopedLock guard(lock);if(cancel())return;auto& current=*find(model,requested.id);
        current.phase=Phase::ready;current.progress=1;current.status="Ready (unchanged source)";
        ++liveRevision;
        runtime[requested.id].revision=juce::String(sourceDependency(latest.capture.manifest,current.sourceStart,renderInputEnd(current)));
        forceRefresh.erase(requested.id);return;
    }
    auto raw=std::make_shared<WetRender>(); Asset committed=requested.render;
    bool cached=false;
    if(restoreOrCheck && matching) cached=assets.readRender(requested.render,available(),*raw,error);
    if(cancel()) return;
    if(!cached)
    {
        report(requested,session,Phase::rendering,"Reading source",0.1f);
        SourceInterval source; const auto budget=available();
        if(budget<=64ULL*1024*1024 || !readSourceInterval(sourceStore,context.capture.manifest,requested.sourceStart,renderInputEnd(requested),
            (budget-64ULL*1024*1024)/(sizeof(float)*2),source,error,cancel)) { failure(error.isEmpty()?"256 MiB working budget exceeded":error); return; }
        if(!conditionWetSourceEdges(source.samples,source.rate,error,cancel)) {if(!cancel())failure(error);return;}
        ImpulseResponse ir,prepared;
        auto remaining=available(source.samples.capacity()*sizeof(float));
        if(!assets.readIR(requested.ir,remaining,ir,error)) { failure(error); return; }
        remaining=available((source.samples.capacity()+ir.samples.capacity())*sizeof(float));
        if(!prepareImpulseResponse(ir,source.rate,remaining,prepared,error,cancel)) { if(!cancel()) failure(error); return; }
        const auto overhead=previewMemoryBudget-available()+ir.samples.capacity()*sizeof(float);
        if(!renderReverseConvolution(source.samples,requested.sourceStart,source.rate,prepared,overhead,*raw,error,cancel,
            [&](float p){report(requested,session,Phase::rendering,"Convolving",0.3f+p*0.5f);}))
        { if(!cancel()) failure(error); return; }
        if(cancel()) return;
        if(!assets.writeRender(*raw,requested.ir,renderDependency,context.time,committed,error)) { failure(error); return; }
    }
    {const juce::ScopedLock guard(lock);if(retainedIR.contains(requested.id)){
        int64_t a,b;if(!shapeBounds(requested,raw->start,raw->sourceEnd,a,b,error)){
            failure("New IR cannot support saved wet shape; choose Full response or Reset explicitly: "+error);return;}}}
    const auto latest=contextProvider(); juce::String currentDependency;
    CaptureAssetStore latestStore(instanceRoot(latest.capture.manifest));
    if(!sameMusicalMapping(context.time,latest.time) || latest.captureGeneration!=context.captureGeneration
        || !sourceFingerprint(latestStore,latest.capture.manifest,requested.sourceStart,renderInputEnd(requested),currentDependency,error,cancel)
        || currentDependency!=dependency)
    { report(requested,session,Phase::stale,"Source/timing changed during generation; retry"); return; }
    std::shared_ptr<Waveform> sourceWave;
    bool selected=false;{const juce::ScopedLock guard(lock);selected=model.primary==requested.id;}
    if(selected && !prepareSourceWave(requested,latest,raw->samples.capacity()*sizeof(float),sourceWave,error)) {failure(error);return;}
    Session calibrationSession;int64_t referenceEnd=requested.sourceEnd;
    {const juce::ScopedLock guard(lock);calibrationSession=model;
        for(const auto& other:model.swells)if(other.id!=requested.id && other.arrival>requested.arrival)referenceEnd=std::min(referenceEnd,other.arrival);}
    GainCalibration calibration;juce::String calibrationNote;
    if(calibrationSession.gainPolicy==GainPolicy::musical)
    {
        const auto requestedReference=std::max(int64_t{1},static_cast<int64_t>(std::llround(context.time.rate*candidateAttackSeconds)));
        int64_t attackEnd;if(!checkedAdd(requested.arrival,requestedReference,attackEnd))attackEnd=requested.arrival;
        referenceEnd=std::min(referenceEnd,attackEnd);
        SourceInterval reference;
        if(referenceEnd<=requested.arrival || !readSourceInterval(sourceStore,latest.capture.manifest,requested.arrival,referenceEnd,
            static_cast<uint64_t>(std::max(int64_t{0},referenceEnd-requested.arrival)),reference,error,cancel))
            calibrationNote="Matching unavailable: local dry reference is not captured";
        else if(isEnergyRevision(calibrationSession.gainRevision))
            prepareSourceEnergyCalibration(calibrationSession,requested,*raw,reference,referenceEnd,committed.content,calibration,calibrationNote,cancel,
                calibrationSession.gainRevision==approvedGainRevision?approvedEnergyTargetDb:0.0f);
        else prepareSourcePeakCalibration(calibrationSession,requested,*raw,reference,referenceEnd,committed.content,calibration,calibrationNote);
    }
    while(renderPublicationHold.load() && !cancel()){renderPublicationWaiting=true;wait(2);}renderPublicationWaiting=false;
    const auto finalContext=contextProvider();
    if(finalContext.captureGeneration!=latest.captureGeneration || !sameMusicalMapping(latest.time,finalContext.time)
        || sourceDependency(finalContext.capture.manifest,requested.sourceStart,renderInputEnd(requested))!=sourceDependency(latest.capture.manifest,requested.sourceStart,renderInputEnd(requested)))
    {report(requested,session,Phase::checking,"Source changed during waveform preparation; checking again");return;}
    auto wave=waveform(*raw);
    const juce::ScopedLock guard(lock); if(cancel()) return;
    auto& w=*find(model,requested.id); ++monitoringGeneration;
    const auto support=std::max(int64_t{0},committed.sourceEnd-w.sourceEnd);
    if(w.render!=committed || w.calibration!=calibration || w.supportOffset!=support)persistentChangedLocked(false);
    w.render=committed;w.calibration=calibration;w.supportOffset=support;w.phase=Phase::ready; w.progress=1; w.status=cached?"Ready (verified saved render)":"Ready (committed render)";
    if(calibrationNote.isNotEmpty())w.status+=" | "+calibrationNote;
    if(!cached && !retainedIR.contains(w.id))invalidateLiveLocked(w.id);
    else ++liveRevision;
    if(!sameMusicalMapping(model.timing,context.time))model.timing=context.time;
    auto& runtimeState=runtime[w.id]; runtimeState.rawWave=std::move(wave);
    runtimeState.verifiedRender=committed;
    runtimeState.sourceWave=std::move(sourceWave);runtimeState.lastUse=++useSerial;
    if(w.id==model.primary) runtimeState.raw=std::move(raw);
    runtimeState.preparedRender.clear(); runtimeState.preparedShape=UINT64_MAX;
    runtimeState.cachedWetOnly=false;runtimeState.preparedMonitor=UINT64_MAX;runtimeState.monitor.reset();
    runtimeState.revision=juce::String(sourceDependency(latest.capture.manifest,w.sourceStart,renderInputEnd(w)));
    if(!cached) ++renderCount;
    forceRefresh.erase(w.id);
}
bool SwellEngine::prepareSourceWave(const Swell& requested,const PreviewContext& context,uint64_t extra,std::shared_ptr<Waveform>& output,juce::String& error)
{
    const auto budget=available(extra+64ULL*1024*1024);
    if(!budget){error="Working budget unavailable for source detail";return false;}
    CaptureAssetStore assets(instanceRoot(context.capture.manifest));SourceInterval source;
    if(!readSourceInterval(assets,context.capture.manifest,requested.sourceStart,renderInputEnd(requested),budget/8,source,error))return false;
    WetRender audio;audio.start=source.start;audio.rate=source.rate;audio.samples=std::move(source.samples);
    for(auto value:audio.samples)audio.peak=std::max(audio.peak,std::abs(value));output=waveform(audio);return true;
}
void SwellEngine::shapeJob(Swell requested,uint64_t session)
{
    const auto cancel=[&]
    { if(cancelled(requested.id,requested.inputGeneration,session)) return true; const juce::ScopedLock guard(lock); const auto w=find(model,requested.id); return !w || w->shapeGeneration!=requested.shapeGeneration || model.primary!=requested.id; };
    std::shared_ptr<WetRender> raw;
    { const juce::ScopedLock guard(lock); raw=runtime[requested.id].raw; }
    juce::String error;
    if(!raw)
    {
        const auto context=contextProvider(); SwellAssetStore assets(instanceRoot(context.capture.manifest)); raw=std::make_shared<WetRender>();
        if(!assets.readRender(requested.render,available(),*raw,error)) { report(requested,session,Phase::error,error); return; }
        const juce::ScopedLock guard(lock); if(cancel()) return; runtime[requested.id].raw=raw;
    }
    bool needsSource;
    { const juce::ScopedLock guard(lock); runtime[requested.id].shaped.reset();runtime[requested.id].monitor.reset();needsSource=!requested.frozen && !runtime[requested.id].sourceWave && !runtime[requested.id].cachedWetOnly; }
    if(needsSource)
    {
        std::shared_ptr<Waveform> wave;
        if(!prepareSourceWave(requested,contextProvider(),0,wave,error)){report(requested,session,Phase::error,error);return;}
        const juce::ScopedLock guard(lock);if(cancel())return;runtime[requested.id].sourceWave=std::move(wave);
    }
    auto shaped=std::make_shared<WetRender>();
    auto audible=requested;{const juce::ScopedLock guard(lock);if(model.gainPolicy==GainPolicy::legacy)audible.calibration={};}
    if(!prepareShape(audible,*raw,available(),*shaped,error,cancel))
    {
        if(!cancel()) { const juce::ScopedLock guard(lock); runtime[requested.id].shapeError=true; report(requested,session,Phase::error,error); }
        return;
    }
    auto rawWave=waveform(*raw),shapeWave=waveform(*shaped);
    const auto latest=contextProvider();
    const juce::ScopedLock guard(lock); if(cancel()) return;
    if(!requested.frozen && runtime[requested.id].revision!=juce::String(sourceDependency(latest.capture.manifest,requested.sourceStart,renderInputEnd(requested))))
    {auto& w=*find(model,requested.id);w.phase=Phase::checking;w.status="Checking source changed during preview preparation";stopLocked();return;}
    auto& r=runtime[requested.id]; r.rawWave=std::move(rawWave); r.shapeWave=std::move(shapeWave); r.shaped=std::move(shaped);
    r.preparedShape=requested.shapeGeneration; r.preparedRender=requested.render.content;
    r.preparedMonitor=UINT64_MAX;
    r.lastUse=++useSerial;
}
void SwellEngine::run()
{
    while(!threadShouldExit())
    {
        if(offlineActive.load()){wait(2);continue;}
        const juce::ScopedLock work(workLock);
        if(offlineActive.load())continue;
        {const juce::ScopedLock guard(lock);if(offlinePlayer || !offlineAccepted.empty()){offlinePlayer.reset();offlineEntries.clear();offlineMembers.clear();offlineAccepted.clear();offlineIRNotice.clear();offlineSession={};}}
        try
        {
            const auto context=contextProvider();
            if(!storageReady(context)){wait(2);continue;}
            collectCompletedPass(context);inspect(context);prepareDefault(context);
            if(learnGenerationStep(context))continue;
            reconcileRenders(context);prepareTransport(context);serviceExportPreview();
            long double origin;const auto validTime=songOrigin(context.time,origin);
            if(analysisTurn && automaticJob()){analysisTurn=false;continue;}
            analysisTurn=true;
            std::optional<Import> import; std::optional<Reset> reset; std::optional<Analysis> analyzingJob; std::optional<Swell> work,shape,monitor;
            bool restoreOrCheck=false; uint64_t session,settings=0;
            {
                const juce::ScopedLock guard(lock); session=sessionGeneration.load();
                if(!resets.empty()){reset=std::move(resets.front());resets.pop_front();}
                else if(!imports.empty()) { import=std::move(imports.front()); imports.pop_front(); }
                else if(analysis) { analyzingJob=analysis; analysis.reset(); }
                else if(validTime && !context.capture.status.startsWith("Loading")
                    && (awaitingCaptureReference.isEmpty() || context.capture.reference==awaitingCaptureReference))
                {
                    for(const auto& w:model.swells) if(w.phase==Phase::queued || w.phase==Phase::restoring || w.phase==Phase::checking)
                    {
                        // An IR import marks Checking too; only its import job can finish it.
                        if(w.status=="Importing IR") continue;
                        if(learnRenderHold.load() && w.status=="Queued after Generate Swells")continue;
                        if(w.status=="Preparing extended wet support" && juce::Time::getMillisecondCounterHiRes()<nextAutomaticEdit)continue;
                        if(!context.time.valid && w.phase==Phase::restoring) continue;
                        work=w;restoreOrCheck=w.phase!=Phase::queued; break;
                    }
                    if(!work) if(const auto w=find(model,model.primary); w && (w->phase==Phase::ready || runtime[w->id].cachedWetOnly))
                    {
                        const auto& r=runtime[w->id]; if(!r.shaped || r.preparedShape!=w->shapeGeneration || r.preparedRender!=w->render.content) shape=*w;
                        else if(r.preparedMonitor!=monitoringGeneration){monitor=*w;settings=monitoringGeneration;}
                    }
                }
            }
            if(reset)resetJob(*reset);
            else if(import) importJob(*import);
            else if(analyzingJob) analysisJob(*analyzingJob);
            else if(work) renderJob(*work,context,session,restoreOrCheck);
            else if(shape) shapeJob(*shape,session);
            else if(monitor)monitorJob(*monitor,session,settings);
            else { prepareExport(context);serviceExportDelivery();serviceExportDetail();available(); wait(10); }
        }
        catch(const std::exception& e)
        {
            const juce::ScopedLock guard(lock); message="Background operation failed: "+juce::String(e.what()); stopLocked();
            if(exportState.phase==ExportPhase::preparing)exportState={ExportPhase::error,message,{}};
            for(auto& w:model.swells) if(w.phase==Phase::rendering || w.phase==Phase::checking) {w.phase=Phase::error;w.status=message;}
            wait(10);
        }
    }
}
}
