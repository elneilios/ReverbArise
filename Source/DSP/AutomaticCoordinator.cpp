#include "SwellEngine.h"
#include "DefaultImpulse.h"
#include <Capture/SourceVisitor.h>
#include <cmath>

namespace m3
{
void SwellEngine::setLearningAutomatic(bool enabled,uint64_t expectedPass)
{
    const juce::ScopedLock guard(lock);
    if(workflowState && workflowState->snapshot().mode!=workflow::Mode::capture)
    {message="Learning preferences require Learn mode";return;}
    auto next=model;next.automaticProcessing=enabled;
    if(enabled)next.autoMarkersEnabled=true;
    if(next.automaticProcessing==model.automaticProcessing && next.autoMarkersEnabled==model.autoMarkersEnabled)return;
    if(!publishGenerationLocked(next,expectedPass)){actionError=message;return;}actionError.clear();
    model.automaticProcessing=next.automaticProcessing;model.autoMarkersEnabled=next.autoMarkersEnabled;
    ++markerGeneration;invalidateLearnLocked();persistentChangedLocked();automaticBoundsDirty=true;notify();
}
void SwellEngine::setAutomaticProcessing(bool enabled,uint64_t expectedPass)
{
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::finalise){diagnostic("Automatic settings are locked in Export");return;}
    const auto context=contextProvider();const juce::ScopedLock guard(lock);
    if(model.automaticProcessing==enabled)return;
    auto next=model;next.automaticProcessing=enabled;if(!publishGenerationLocked(next,expectedPass)){actionError=message;return;}actionError.clear();
    if(model.automaticProcessing!=enabled){++markerGeneration;invalidateLearnLocked();persistentChangedLocked();}model.automaticProcessing=enabled;
    automaticBoundsDirty=true;
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::live)performWindowsRequested=true;
    if(enabled && model.autoMarkersEnabled && (!workflowState || workflowState->snapshot().mode!=workflow::Mode::capture))for(const auto& r:context.capture.manifest.ranges)
    {int64_t end;if(r.frameCount<=INT64_MAX && checkedAdd(r.startSample,static_cast<int64_t>(r.frameCount),end))dirtyLocked(r.startSample,end);}
    notify();
}
void SwellEngine::setAutoMarkers(bool enabled,uint64_t expectedPass)
{
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::finalise){diagnostic("Marker settings are locked in Export");return;}
    const auto context=contextProvider();const juce::ScopedLock guard(lock);
    if(model.autoMarkersEnabled==enabled)return;auto next=model;next.autoMarkersEnabled=enabled;if(!publishGenerationLocked(next,expectedPass)){actionError=message;return;}actionError.clear();model.autoMarkersEnabled=enabled;persistentChangedLocked();++markerGeneration;
    invalidateLearnLocked();
    if(enabled && model.automaticProcessing && (!workflowState || workflowState->snapshot().mode!=workflow::Mode::capture))for(const auto& r:context.capture.manifest.ranges)
    {int64_t end;if(r.frameCount<=INT64_MAX && checkedAdd(r.startSample,static_cast<int64_t>(r.frameCount),end))dirtyLocked(r.startSample,end);}
    notify();
}
void SwellEngine::resetSuppression()
{if(!permitCreative())return;const juce::ScopedLock guard(lock);if(!model.suppressed.empty())persistentChangedLocked();model.suppressed.clear();analyzedContent.clear();message="Deleted target suppression reset; replay to rediscover";}
void SwellEngine::loadDefaultIR(const juce::File& file)
{assignDefaultIR(file);}
bool SwellEngine::assignDefaultIR(const juce::File& file,uint64_t expectedSession,uint64_t expectedWorkflowPass)
{if(file==juce::File{})return false;return queueDefaultIR(file,{},{},file.getFileName(),expectedSession,expectedWorkflowPass);}
bool SwellEngine::assignDefaultBundledIR(const juce::String& presetId,uint64_t expectedSession,uint64_t expectedWorkflowPass)
{
    const auto* choice=findBundledImpulse(presetId);
    if(!choice){const juce::ScopedLock guard(lock);defaultError="Unknown bundled Default IR";return false;}
    return queueDefaultIR({},presetId,{},choice->name,expectedSession,expectedWorkflowPass);
}
bool SwellEngine::assignDefaultManagedIR(const Asset& asset,const juce::String& name,uint64_t expectedSession,uint64_t expectedWorkflowPass)
{if(!asset.present())return false;return queueDefaultIR({},{},asset,name,expectedSession,expectedWorkflowPass);}
bool SwellEngine::queueDefaultIR(const juce::File& file,const juce::String& presetId,const Asset& asset,const juce::String& label,uint64_t expectedSession,uint64_t expectedWorkflowPass)
{
    const juce::ScopedLock guard(lock);const auto flow=workflowState?workflowState->snapshot():workflow::Snapshot{};
    if((expectedSession && expectedSession!=sessionGeneration.load()) || (expectedWorkflowPass!=UINT64_MAX && expectedWorkflowPass!=flow.pass))
    {message="Default IR request superseded";if(!defaultPending)defaultStatus=message;return false;}
    if(workflowState && (flow.mode==workflow::Mode::finalise || flow.locked())){message="Default IR is locked for this pass";return false;}
    std::erase_if(imports,[](const auto& job){return job.futureDefault;});
    if(imports.size()>=32){message="IR import queue full";return false;}
    Import job;job.file=file;job.presetId=presetId;job.managed=asset;job.label=label;job.session=sessionGeneration.load();job.futureDefault=true;
    cancelLearnGenerationLocked();
    job.defaultRequest=++defaultRequest;job.workflowPass=flow.pass;imports.push_back(std::move(job));
    defaultPending=true;requestedDefaultName=label;defaultError.clear();defaultStatus="Preparing requested Default IR";notify();return true;
}
void SwellEngine::dirtyLocked(int64_t a,int64_t b,bool acceptedPass)
{
    auto& dirty=acceptedPass?passDirty:this->dirty;
    if(a>=b)return;
    for(auto i=dirty.begin();i!=dirty.end();)
    {if(i->second>=a && i->first<=b){a=std::min(a,i->first);b=std::max(b,i->second);i=dirty.erase(i);}else ++i;}
    dirty.push_back({a,b});std::sort(dirty.begin(),dirty.end());
    // A bounding dirty summary is safe: analysis still walks only real coverage.
    if(dirty.size()>128){const auto first=dirty.front().first,last=dirty.back().second;dirty={{first,last}};}
}
void SwellEngine::acceptedSourceLocked(const CaptureManifest& map)
{
    if(workflowState){acceptedSource=map;return;}
    // Durable provenance is the queue's recovery journal. Only new/changed
    // accepted On spans are admitted; future preference edits cannot backfill Off.
    if(map.schemaVersion>=2)for(const auto& span:map.contextSpans)
    {
        const auto* owner=captureContextAt(map,span.start);
        if(!owner || !owner->automatic || owner->origin!=CaptureContextOrigin::pass)continue;
        bool unchanged=false;
        if(acceptedSource && acceptedSource->projectId==map.projectId && acceptedSource->instanceId==map.instanceId)
        {
            const auto* old=captureContextAt(*acceptedSource,span.start);
            const auto* last=captureContextAt(*acceptedSource,span.end-1);
            unchanged=old && last && old->id==owner->id && last->id==owner->id
                && std::find(acceptedSource->contextSpans.begin(),acceptedSource->contextSpans.end(),span)!=acceptedSource->contextSpans.end()
                && sourceDependency(*acceptedSource,span.start,span.end)==sourceDependency(map,span.start,span.end);
        }
        if(!unchanged){dirtyLocked(span.start,span.end,true);
            for(const auto& w:model.swells)if(!w.frozen && w.sourceStart<span.end && renderInputEnd(w)>span.start
                && captureAllowsAutomatic(map,w.sourceStart,renderInputEnd(w)))passRenderRequests.insert(w.id);}

    }
    acceptedSource=map;
}
void SwellEngine::captureCommitted(int64_t a,int64_t b,uint64_t generation,bool explicitRecapture)
{
    const auto context=contextProvider();if(context.captureGeneration!=generation)return;
    const juce::ScopedLock guard(lock);if(captureEpochProvider && captureEpochProvider()!=generation)return;
    if(workflowState)
    {
        invalidateLearnLocked();automaticBoundsDirty=false;
        for(auto& w:model.swells)if(!w.frozen && w.sourceStart<b && renderInputEnd(w)>a)
        {
            ++w.inputGeneration;w.phase=Phase::stale;w.status="Source changed; waiting for explicit generation";
            deferredLearnDependencies.insert(w.id);forceRefresh.erase(w.id);passRenderRequests.erase(w.id);invalidateLiveLocked(w.id);
        }
        notify();return;
    }
    automaticBoundsDirty=true;if(context.capture.manifest.schemaVersion<2 && model.automaticProcessing && model.autoMarkersEnabled)dirtyLocked(a,b);
    for(auto& w:model.swells)if(!w.frozen && w.sourceStart<b && renderInputEnd(w)>a)
    {
        if(!captureAllowsAutomatic(context.capture.manifest,std::max(a,w.sourceStart),std::min(b,renderInputEnd(w))))
        {
            // Fence an older queued/in-flight automatic render, including forced
            // Recapture and legacy refresh paths. Keep definitions/assets intact.
            ++w.inputGeneration;w.phase=Phase::stale;w.status="Dry-only capture changed source; generate in Perform";
            forceRefresh.erase(w.id);passRenderRequests.erase(w.id);invalidateLiveLocked(w.id);
            if(w.id==model.primary)stopLocked();
        }
        else if(explicitRecapture){invalidateLiveLocked(w.id);forceRefresh.insert(w.id);
            if(w.ir.present()){w.phase=Phase::checking;w.status="Checking completed recapture";}if(w.id==model.primary)stopLocked();}
    }
    notify();
}
void SwellEngine::collectCompletedPass(const PreviewContext& context,bool acceptedCapture)
{
    {const juce::ScopedLock guard(lock);
        if(recoverPassBounds && !context.capture.status.startsWith("Loading") && (awaitingCaptureReference.isEmpty() || context.capture.reference==awaitingCaptureReference)){
            recoverPassBounds=false;CompletedPasses runs;
            for(const auto& range:context.capture.manifest.ranges){int64_t end;if(range.frameCount>INT64_MAX || !checkedAdd(range.startSample,static_cast<int64_t>(range.frameCount),end))continue;
                if(!runs.empty() && range.startSample<=runs.back().second)runs.back().second=std::max(runs.back().second,end);else runs.push_back({range.startSample,end});}
            const auto quarter=model.timing.rate*60/model.timing.bpm;
            if(model.timing.valid && std::isfinite(quarter) && quarter>=1 && quarter<static_cast<double>(INT64_MAX))for(const auto& w:model.swells){int64_t ordinary;
                if(w.provenance!=Provenance::automatic || !checkedAdd(w.arrival,static_cast<int64_t>(std::llround(quarter)),ordinary) || w.sourceEnd>=ordinary)continue;
                for(const auto& run:runs)if(run.first<=w.arrival && (run.second==w.sourceEnd || (w.sourceEnd<INT64_MAX && run.second==w.sourceEnd+1)) && completedPasses.size()<128){
                    const auto recovered=std::pair{run.first,w.sourceEnd};
                    if(std::find(completedPasses.begin(),completedPasses.end(),recovered)==completedPasses.end())completedPasses.push_back(recovered);}}
        }}
    // In Learn, an RT Stop boundary is still provisional until source settlement
    // accepts it. It must never become legacy reconciliation work, including
    // while a fresh instance still exposes an empty schema-1 manifest.
    if(workflowState && !acceptedCapture && workflowState->snapshot().mode!=workflow::Mode::live)return;
    for(int attempt=0;attempt<8;++attempt){const auto version=passSequence.load(std::memory_order_acquire);if(version&1)continue;
        const auto first=passStart.load(),last=passEnd.load();const auto session=passSession.load();
        if(version!=passSequence.load(std::memory_order_acquire))continue;
        const juce::ScopedLock guard(lock);
        if(version==acceptedPassSequence || session!=sessionGeneration.load() || first>=last)return;
        acceptedPassSequence=version;
        auto found=std::find_if(completedPasses.begin(),completedPasses.end(),[&](const auto& pass){return pass.first==first;});
        if(found!=completedPasses.end()){
            const auto unchanged=found->second==last;completedPasses.erase(found);completedPasses.push_back({first,last});
            if(unchanged)return;}
        else {if(completedPasses.size()>=128){message="Completed-pass boundary capacity reached; capture extra tail or clear";return;}completedPasses.push_back({first,last});}
        automaticBoundsDirty=true;
        if(workflowState && workflowState->snapshot().mode==workflow::Mode::live)performWindowsRequested=true;
        if(!workflowState && context.capture.manifest.schemaVersion<2 && model.automaticProcessing && model.autoMarkersEnabled && context.time.valid){int64_t before;
            if(!checkedSubtract(last,static_cast<int64_t>(analysisWindow(context.time.rate))*100,before))before=first;
            dirtyLocked(std::max(first,before),last);}
        return;
    }
}
CompletedPasses SwellEngine::applicableCompletedPasses(const CaptureManifest& map) const
{
    return usableCompletedPasses(map,completedPasses);
}
void SwellEngine::prepareDefault(const PreviewContext& context)
{
    if(context.capture.status.startsWith("Loading") || context.capture.unavailable || recoveryGenerationHold.load() || recoveryReloadOnly.load())return;
    const auto scope=juce::String(context.capture.manifest.projectId)+"/"+context.capture.manifest.instanceId;
    uint64_t generation;
    {const juce::ScopedLock guard(lock);if((workflowState && workflowState->snapshot().locked()) || !awaitingCaptureReference.isEmpty() || model.defaultIR.present() || defaultAttemptScope==scope)return;
        defaultAttemptScope=scope;generation=sessionGeneration.load();}
    Asset asset;juce::String error;SwellAssetStore store(instanceRoot(context.capture.manifest));
    const auto ok=installBundledImpulse(store,defaultImpulseId,available(),asset,error);
    const juce::ScopedLock guard(lock);if(generation!=sessionGeneration.load())return;
    if(!ok){defaultError="Missing default IR: "+error;message=defaultError;return;}
    auto next=model;next.defaultIR=asset;next.defaultIRName="Preverb Hall";
    if(!publishGenerationLocked(next)){defaultAttemptScope.clear();return;}
    if(model.defaultIR!=asset || model.defaultIRName!="Preverb Hall")persistentChangedLocked();model.defaultIR=asset;model.defaultIRName="Preverb Hall";
    if(!workflowState)for(auto& w:model.swells)if(!w.ir.present() && w.status=="Waiting for default IR")
    {w.ir=asset;w.irName=model.defaultIRName;w.phase=Phase::stale;w.status="Waiting for source";}
}
void SwellEngine::requestManualPreparationLocked(const Swell& w,const PreviewContext& context)
{
    // Explicit Perform intent grants work for this target without changing the
    // saved automatic policy. Pin its current source revision so inspect does
    // not mistake a new/edited interval for a later dry-only capture change.
    deferredLearnDependencies.erase(w.id);
    forceRefresh.insert(w.id);
    runtime[w.id].revision=juce::String(sourceDependency(context.capture.manifest,w.sourceStart,renderInputEnd(w)));
}
void SwellEngine::dispatchAutomatic(const PreviewContext& context)
{
    if(recoveryGenerationHold.load() || recoveryReloadOnly.load())return;
    const juce::ScopedLock guard(lock);
    if(workflowState && workflowState->snapshot().mode==workflow::Mode::capture)return;
    if((!model.automaticProcessing && passRenderRequests.empty() && forceRefresh.empty()) || !context.time.valid || context.time.rate!=context.capture.manifest.sampleRate
       || (model.timing.valid && !transportMappingMatches(model.timing,context.time)) || juce::Time::getMillisecondCounterHiRes()<nextAutomaticEdit)return;
    if(automaticBoundsDirty && (!workflowState || performWindowsRequested)){auto trial=model;juce::String error;
        if(!updateAutomaticWindows(trial,context.time,error,applicableCompletedPasses(context.capture.manifest))){message=error;return;}
        for(const auto& old:model.swells)if(const auto* now=find(trial,old.id);now && (now->inputGeneration!=old.inputGeneration || now->shapeGeneration!=old.shapeGeneration))invalidateLiveLocked(old.id);
        model=std::move(trial);automaticBoundsDirty=false;performWindowsRequested=false;}
    for(auto& w:model.swells)
    {
        if(deferredLearnDependencies.contains(w.id) && !forceRefresh.contains(w.id))continue;
        if(w.frozen || (!model.automaticProcessing && !passRenderRequests.contains(w.id) && !forceRefresh.contains(w.id)) || (w.phase!=Phase::stale && w.phase!=Phase::empty))continue;
        if(!captureAllowsAutomatic(context.capture.manifest,w.sourceStart,renderInputEnd(w)) && !passRenderRequests.contains(w.id) && !forceRefresh.contains(w.id))continue;
        if(!w.ir.present()){w.status="Missing IR: assign an impulse response";continue;}
        // Coverage alone is cheap admission; decoding/checksum validation stays in renderJob.
        int64_t covered=w.sourceStart;
        for(const auto& r:context.capture.manifest.ranges)
        {
            int64_t end;if(r.frameCount>INT64_MAX || !checkedAdd(r.startSample,static_cast<int64_t>(r.frameCount),end))continue;
            if(r.startSample>covered)break;if(end>covered)covered=end;if(covered>=renderInputEnd(w))break;
        }
        if(covered<renderInputEnd(w)){w.status="Waiting for source";continue;}
        w.phase=Phase::checking;w.status="Queued for dependency check";
    }
}
bool SwellEngine::automaticJob()
{
    const auto workflowPass=workflowState?workflowState->snapshot().pass:0;
    if(workflowState && workflowState->snapshot().mode!=workflow::Mode::live)return false;
    PreviewContext context;
    int64_t a=0,b=0;uint64_t generation,controls;float sensitivity;double separation;bool playing,acceptedPass=false;juce::String contextId;
    {
        const juce::ScopedLock guard(lock);
        // Snapshot after acquiring notification ownership, never consume newer dirty work
        // against the previous run-loop snapshot (which can omit the committed region).
        context=contextProvider();
        playing=transportActive.load();
        if(wasTransportActive && !playing && trailing){dirtyLocked(trailing->first,trailing->second);trailing.reset();}
        wasTransportActive=playing;
        acceptedPass=!passDirty.empty();
        if((!acceptedPass && (!model.automaticProcessing || !model.autoMarkersEnabled || dirty.empty())) || juce::Time::getMillisecondCounterHiRes()<nextMarkerAnalysis || context.capture.status.startsWith("Loading") || !context.time.valid || context.time.rate!=context.capture.manifest.sampleRate
            || (model.timing.valid && !transportMappingMatches(model.timing,context.time)))return false;
        generation=sessionGeneration.load();controls=markerGeneration.load();sensitivity=model.sensitivity;separation=model.separationSeconds;
        // One bounded cell per turn, allowing rendering to run between detection batches.
        const auto cell=static_cast<int64_t>(context.time.rate*8);
        auto& pending=acceptedPass?passDirty:dirty;
        a=pending.front().first;b=pending.front().second;
        if(acceptedPass){
            const auto span=std::find_if(context.capture.manifest.contextSpans.begin(),context.capture.manifest.contextSpans.end(),[&](const auto& item){return item.end>a;});
            if(span==context.capture.manifest.contextSpans.end() || span->start>=b){pending.erase(pending.begin());return true;}
            a=std::max(a,span->start);b=std::min(b,span->end);
            const auto* owner=captureContextAt(context.capture.manifest,a);
            if(!owner || !owner->automatic || owner->origin!=CaptureContextOrigin::pass){
                if(b==pending.front().second)pending.erase(pending.begin());else pending.front().first=b;return true;}
            contextId=owner->id;sensitivity=owner->sensitivity;separation=owner->separationSeconds;
        }
        const auto remainder=((a%cell)+cell)%cell;
        int64_t cellEnd;if(checkedAdd(a,cell-remainder,cellEnd))b=std::min(b,cellEnd);
        if(b==pending.front().second)pending.erase(pending.begin());else pending.front().first=b;
        automaticBusy=true;
    }
    struct BusyReset {std::atomic<bool>& flag;~BusyReset(){flag=false;}} busy{automaticBusy};
    const auto cancel=[&]{return threadShouldExit() || generation!=sessionGeneration.load() || (!acceptedPass && controls!=markerGeneration.load())
        || (workflowState && (workflowState->snapshot().mode!=workflow::Mode::live || workflowState->snapshot().pass!=workflowPass));};
    const auto window=analysisWindow(context.time.rate);const auto history=static_cast<int64_t>(window)*100;
    const auto tolerance=targetTolerance(context.time.rate,separation);
    CaptureAssetStore store(instanceRoot(context.capture.manifest));
    std::vector<std::pair<int64_t,int64_t>> runs;
    for(const auto& r:context.capture.manifest.ranges)
    {
        int64_t end;if(r.frameCount>INT64_MAX || !checkedAdd(r.startSample,static_cast<int64_t>(r.frameCount),end))continue;
        if(!runs.empty() && r.startSample<=runs.back().second)runs.back().second=std::max(runs.back().second,end);
        else runs.push_back({r.startSample,end});
    }
    for(const auto& run:runs)
    {
        if(cancel())return true;
        if(run.second<=a || run.first>=b)continue;
        // Expand around a dirty boundary, but reconcile only the complete affected body.
        int64_t before,after;if(!checkedSubtract(a,history,before))before=INT64_MIN;
        if(!checkedAdd(b,history,after))after=INT64_MAX;
        auto scanStart=std::max(run.first,before);const auto scanEnd=std::min(run.second,after);
        int64_t offset;
        if(checkedSubtract(scanStart,run.first,offset))scanStart-=offset%(window/4);
        int64_t bodyBefore,bodyAfter;
        if(!checkedSubtract(a,tolerance,bodyBefore))bodyBefore=INT64_MIN;
        if(!checkedAdd(b,tolerance,bodyAfter))bodyAfter=INT64_MAX;
        int64_t bodyStart=std::max(run.first,acceptedPass?a:bodyBefore),bodyEnd=std::min(run.second,acceptedPass?b:bodyAfter);
        bool closedRun=false;
        {const juce::ScopedLock guard(lock);for(const auto& pass:applicableCompletedPasses(context.capture.manifest))if(pass.second==run.second)closedRun=true;
            if(closedRun && trailing && trailing->second==run.second)trailing.reset();}
        if(playing && scanEnd==run.second && !closedRun)
        {
            const auto safeEnd=std::max(run.first,run.second-window);
            if(bodyEnd>safeEnd)
            {const juce::ScopedLock guard(lock);trailing=std::pair{safeEnd,run.second};bodyEnd=safeEnd;}
        }
        if(bodyEnd<=bodyStart)continue;
        juce::String hash,error;
        if(available()<64ULL*1024*1024)
        {{const juce::ScopedLock guard(lock);message="Detection waiting for working memory";dirtyLocked(a,b,acceptedPass);}wait(20);return true;}
        if(!sourceFingerprint(store,context.capture.manifest,scanStart,scanEnd,hash,error,cancel))
        {const juce::ScopedLock guard(lock);message=error;return true;}
        const auto key=std::pair{scanStart,scanEnd};
        const auto identity=contextId+"/"+juce::String(attackPolicyVersion)+"/"+juce::String(sensitivity,9)+"/"+juce::String(separation,9)+"/"+hash+"/"+juce::String(bodyStart)+"/"+juce::String(bodyEnd);
        {const juce::ScopedLock guard(lock);const auto old=analyzedContent.find(key);if(old!=analyzedContent.end() && old->second==identity)continue;}
        std::vector<Candidate> detected;
        if(!suggestOnsets(store,context.capture.manifest,scanStart,scanEnd,sensitivity,separation,detected,error,cancel))
        {const juce::ScopedLock guard(lock);message=error;return true;}
        const auto latest=contextProvider();
        if(cancel() || latest.captureGeneration!=context.captureGeneration
            || sourceDependency(latest.capture.manifest,scanStart,scanEnd)!=sourceDependency(context.capture.manifest,scanStart,scanEnd)
            || (acceptedPass && (!captureContextAt(latest.capture.manifest,a) || !captureContextAt(latest.capture.manifest,b-1)
                || captureContextAt(latest.capture.manifest,a)->id!=contextId || captureContextAt(latest.capture.manifest,b-1)->id!=contextId)))
        {const juce::ScopedLock guard(lock);dirtyLocked(a,b,acceptedPass);return true;}
        const juce::ScopedLock guard(lock);if(cancel() || (!acceptedPass && (!model.automaticProcessing || !model.autoMarkersEnabled)))return true;
        if(!acceptedPass && (model.sensitivity!=sensitivity || model.separationSeconds!=separation)){dirtyLocked(a,b,acceptedPass);return true;}
        ++automaticAnalysisCount;
        const auto previous=model.swells;
        if(!reconcileTargets(model,detected,bodyStart,bodyEnd,context.time,error,applicableCompletedPasses(context.capture.manifest),&context.capture.manifest,separation)){message=error;return true;}
        if(acceptedPass || context.capture.manifest.schemaVersion>=2)for(const auto& w:model.swells)
            if(std::none_of(previous.begin(),previous.end(),[&](const auto& old){return old.id==w.id;}))passRenderRequests.insert(w.id);
        if(previous.size()!=model.swells.size() || !std::equal(previous.begin(),previous.end(),model.swells.begin(),sameSavedSwell))persistentChangedLocked();
        for(const auto& old:previous){const auto* now=find(model,old.id);
            if(!now || now->inputGeneration!=old.inputGeneration || now->shapeGeneration!=old.shapeGeneration)invalidateLiveLocked(old.id);}
        if(analyzedContent.size()>=512)analyzedContent.erase(analyzedContent.begin());
        // Body bounds are part of identity: stop finalization still runs with unchanged PCM.
        analyzedContent[key]=identity;
        // Removed targets cannot leave PCM or pending render state behind.
        std::erase_if(runtime,[&](const auto& entry){return !find(model,entry.first);});
        std::erase_if(passRenderRequests,[&](const auto& id){return !find(model,id);});
        std::erase_if(forceRefresh,[&](const auto& id){return !find(model,id);});
        message="Automatic preparation: "+juce::String(static_cast<int>(model.swells.size()))+" swells; audition to listen";
    }
    return true;
}
}
