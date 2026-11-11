#include "SwellEngine.h"

namespace m3
{
void SwellEngine::requestExportDetail(int64_t first,int64_t end,size_t bins,uint64_t expectedSession,const juce::String& composition)
{
    const juce::ScopedLock guard(lock);if(expectedSession!=sessionGeneration.load() || !workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise)return;
    auto accepted=exportState.prepared;
    if(!accepted || accepted->plan->identity!=composition)accepted=exportState.previous;
    if(!accepted || accepted->plan->identity!=composition)accepted=exportState.preview;
    if(!accepted || accepted->plan->identity!=composition)return;
    first=std::max(first,accepted->plan->origin);end=std::min(end,accepted->plan->end);bins=std::clamp(bins,size_t{1},size_t{4096});if(first>=end)return;
    if(exportState.detail && exportState.detail->composition==composition && exportState.detail->first==first && exportState.detail->end==end && exportState.detail->bins==bins)return;
    if(detailWork && detailWork->accepted==accepted && detailWork->first==first && detailWork->end==end && detailWork->bins==bins)return;
    detailWork=DetailRequest{expectedSession,detailSerial.fetch_add(1)+1,exportRequest.load(),workflowState->snapshot().pass,first,end,bins,
        juce::Time::getMillisecondCounterHiRes()+100,std::move(accepted)};notify();
}
void SwellEngine::serviceExportDetail()
{
    std::optional<DetailRequest> job;
    {const juce::ScopedLock guard(lock);if(!detailWork || juce::Time::getMillisecondCounterHiRes()<detailWork->due)return;
        job=std::move(detailWork);detailWork.reset();}
    const auto cancelled=[&]{return threadShouldExit() || sessionGeneration.load()!=job->session || exportRequest.load()!=job->mix
        || detailSerial.load()!=job->serial || !workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise || workflowState->snapshot().pass!=job->pass;};
    if(cancelled())return;
    constexpr uint64_t scratch=512*1024;const auto budget=available();juce::String error;
    const auto fail=[&]{const juce::ScopedLock guard(lock);if(!cancelled())exportState.detailMessage=error;};
    if(budget<=scratch){error="Waveform detail working budget unavailable";fail();return;}
    struct Reservation{SwellEngine& engine;~Reservation(){const juce::ScopedLock guard(engine.lock);engine.activeJobBytes=0;}} reservation{*this};
    {const juce::ScopedLock guard(lock);activeJobBytes=budget;}
    SongMix mix(job->accepted->storage?job->accepted->storage->projectsRoot.getChildFile(job->accepted->plan->source.projectId).getChildFile(job->accepted->plan->source.instanceId):instanceRoot(job->accepted->plan->source));auto detail=std::make_shared<SongMixDetail>();
    const auto service=[&]{serviceExportPreview();return cancelled();};
    if(!mix.prepare(job->accepted->plan,budget-scratch,error,service)
        || !mix.adoptMeasurements(job->accepted->metrics,job->accepted->plan->identity,error)
        || !mix.summarize(job->first,job->end,job->bins,*detail,error,service)){fail();return;}
    const juce::ScopedLock guard(lock);if(cancelled())return;
    activeJobBytes=mix.workingBytes()+scratch;exportState.detail=std::move(detail);exportState.detailMessage.clear();
}
bool SwellEngine::hasExportableWet(const SongTime& time)const
{
    auto context=contextProvider();context.time=time;const juce::ScopedLock guard(lock);
    const bool solo=std::any_of(model.swells.begin(),model.swells.end(),[](const auto& w){return w.solo;});
    for(const auto& w:model.swells){
        if(!w.enabled || w.muted || (solo && !w.solo))continue;
        if(context.capture.sourceKind()==CaptureSourceKind::loop && context.capture.unavailable){
            const auto resolved=runtime.find(w.id);
            if(w.frozen && resolved!=runtime.end() && resolved->second.verifiedRender==w.render
               && hasCompatibleReadyRender(w,time))return true;
            continue;
        }
        if(hasCompatibleReadyRender(w,time))return true;
        if(const auto kept=retainedIR.find(w.id);kept!=retainedIR.end() && retainedCompatible(kept->second,w,context)
            && hasCompatibleReadyRender(kept->second.accepted,time))return true;
    }return false;
}

juce::File SwellEngine::readyExportFile(uint64_t expectedSession,uint64_t expectedPass,const ExportFile& file) const
{
    const juce::ScopedLock guard(lock);
    if(!workflowState || expectedSession!=sessionGeneration.load())return {};
    const auto flow=workflowState->snapshot();
    if(flow.mode!=workflow::Mode::finalise || flow.pass!=expectedPass || exportState.pass!=expectedPass
       || exportState.session!=expectedSession || exportState.phase!=ExportPhase::ready || exportState.delivering
       || !exportState.prepared || exportState.prepared->file!=file)return {};
    const auto active=storageLocation.load();
    const auto location=exportState.prepared->storage?exportState.prepared->storage:active;
    if(!active->writable || !location->writable)return {};
    const auto& source=exportState.prepared->plan->source;
    return location->projectsRoot.getChildFile(source.projectId).getChildFile(source.instanceId).getChildFile(file.path);
}

uint64_t SwellEngine::deliverExport(uint64_t expectedSession,const ExportFile& file,const juce::File& destination,bool overwrite,bool drag,uint64_t expectedPass)
{
    const juce::ScopedLock guard(lock);
    if(expectedSession!=sessionGeneration.load() || !workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise
        || exportState.pass!=workflowState->snapshot().pass || exportState.session!=sessionGeneration.load()
        || exportState.phase!=ExportPhase::ready || !exportState.prepared || exportState.prepared->file!=file || (!drag && destination==juce::File{}))return 0;
    if(expectedPass!=UINT64_MAX && workflowState->snapshot().pass!=expectedPass)return 0;
    const auto activeLocation=storageLocation.load();const auto location=exportState.prepared->storage?exportState.prepared->storage:activeLocation;
    if(!activeLocation->writable || !location->writable){exportState.deliveryMessage="Asset working folder unavailable";return 0;}
    const auto inside=[&](const juce::File& root){return destination==root || destination.isAChildOf(root);};
    if(!drag && (inside(location->managedRoot) || inside(activeLocation->managedRoot) || inside(legacyProjectsRoot)))
    {exportState.deliveryMessage="Choose a Save location outside managed plugin storage";return 0;}
    const auto id=++exportDeliverySerial;exportVoice->stop();
    exportDelivery=ExportDelivery{id,expectedSession,exportRequest.load(),workflowState->snapshot().pass,file,
        location->projectsRoot.getChildFile(exportState.prepared->plan->source.projectId).getChildFile(exportState.prepared->plan->source.instanceId),destination,overwrite,drag,location};
    exportState.delivery=id;exportState.delivering=true;exportState.dragReady=false;exportState.deliveryPath.clear();
    exportState.deliveryMessage=drag?"Checking WAV for dragging":"Saving WAV; preview stopped";notify();return id;
}
void SwellEngine::serviceExportDelivery()
{
    std::optional<ExportDelivery> job;
    {const juce::ScopedLock guard(lock);job=std::move(exportDelivery);exportDelivery.reset();}
    if(!job)return;
    const auto cancelled=[&]{const juce::ScopedLock guard(lock);
        return threadShouldExit() || sessionGeneration.load()!=job->session || exportRequest.load()!=job->request
            || exportDeliverySerial!=job->id || !workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise
            || workflowState->snapshot().pass!=job->pass || !exportState.prepared || exportState.prepared->file!=job->file;};
    juce::String error;
    if(cancelled())return;
    constexpr uint64_t scratch=1024*1024;
    if(available()<scratch){const juce::ScopedLock guard(lock);if(!cancelled()){exportState.delivering=false;exportState.deliveryMessage="Export delivery working budget unavailable";}return;}
    struct Reservation{SwellEngine& engine;~Reservation(){const juce::ScopedLock guard(engine.lock);engine.activeJobBytes=0;}} reservation{*this};
    {const juce::ScopedLock guard(lock);activeJobBytes=scratch;}
    const auto commit=[&](const std::function<bool()>& publish){const juce::ScopedLock guard(lock);return !cancelled() && publish();};
    if(!verifyExportFile(job->directory,job->file,error,cancelled)){
        const juce::ScopedLock guard(lock);if(!cancelled()){
            exportState.phase=ExportPhase::error;exportState.message=error;exportState.deliveryMessage=error;exportState.delivering=false;exportState.dragReady=false;
            exportState.prepared.reset();exportAttempt.clear();
        }return;
    }
    const auto success=job->drag?true
        :saveExportFile(job->directory,job->file,job->destination,job->overwrite,error,cancelled,{},commit);
    const juce::ScopedLock guard(lock);if(cancelled())return;
    exportState.delivering=false;exportState.dragReady=success && job->drag;
    exportState.deliveryPath=exportState.dragReady?job->directory.getChildFile(job->file.path).getFullPathName():juce::String{};
    exportState.deliveryMessage=success?(job->drag?"WAV verified; Save remains available":"WAV saved to "+job->destination.getFileName()):error;
}
bool SwellEngine::playExport(uint64_t expectedSession,double deviceRate)
{
    const juce::ScopedLock guard(lock);
    if(expectedSession!=sessionGeneration.load() || !workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise
        || exportState.pass!=workflowState->snapshot().pass || exportState.session!=sessionGeneration.load()
        || exportState.phase!=ExportPhase::ready || !exportState.prepared || !exportState.prepared->metrics.audibleStart)return false;
    if(exportState.prepared->plan->loop && deviceRate>0 && deviceRate!=exportState.prepared->plan->timing.rate)return false;
    exportState.message="Export WAV ready";
    exportPreviewPin=exportState.prepared;exportPreviewSession=expectedSession;exportPreviewPass=exportState.pass;exportPreviewRevision=liveRevision;exportPreviewWetAvailable=true;
    exportPreviewLoop.store(exportPreviewPin->plan->loop.has_value());
    setExpected(exportState.prepared->plan->timing);const auto ticket=exportVoice->requestStart();
    const auto levels=model.exportState.levels(exportPreviewPin->plan->loop.has_value());
    exportPreviewDryRequested=levels.dry>0;exportPreviewResume.reset();
    exportVoice->setLevels(ticket,levels.dry,levels.wet,*exportPreviewPin->metrics.audibleEnd);notify();return true;
}
bool SwellEngine::seekExport(int64_t sample,uint64_t expectedSession,uint64_t expectedPass,const juce::String& composition)
{
    const juce::ScopedLock guard(lock);
    if(expectedSession!=sessionGeneration.load() || !workflowState)return false;
    const auto flow=workflowState->snapshot();
    if(flow.mode!=workflow::Mode::finalise || flow.pass!=expectedPass || flow.locked()
        || exportState.session!=expectedSession || exportState.pass!=expectedPass
        || exportState.phase!=ExportPhase::ready || !exportState.prepared
        || exportState.prepared->plan->identity!=composition)return false;
    const auto& metrics=exportState.prepared->metrics;
    if(!metrics.audibleStart || !metrics.audibleEnd)return false;
    exportState.previewPosition=std::clamp(sample,*metrics.audibleStart,*metrics.audibleEnd-1);
    if(exportVoice->requested(exportVoice->request())){const auto ticket=exportVoice->requestStart();
        exportPreviewPin=exportState.prepared;exportPreviewSession=expectedSession;exportPreviewPass=expectedPass;exportPreviewRevision=liveRevision;exportPreviewWetAvailable=true;
        exportPreviewLoop.store(exportPreviewPin->plan->loop.has_value());
        const auto levels=model.exportState.levels(exportPreviewPin->plan->loop.has_value());
        exportPreviewDryRequested=levels.dry>0;exportPreviewResume.reset();
        exportVoice->setLevels(ticket,levels.dry,levels.wet,*metrics.audibleEnd);}
    notify();return true;
}
void SwellEngine::serviceExportPreview()
{
    const auto ticket=exportVoice->request();
    const auto discard=[&]{exportPreviewReader.reset();exportPreviewBytes=0;exportPreviewTicket=0;};
    if(!workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise || !exportVoice->requested(ticket)){discard();return;}
    std::shared_ptr<const PreparedExport> pin;uint64_t session,pass,revision;std::optional<int64_t> parked;bool needsDry;
    {const juce::ScopedLock guard(lock);pin=exportPreviewPin;session=exportPreviewSession;pass=exportPreviewPass;revision=exportPreviewRevision;
        parked=exportPreviewResume?exportPreviewResume:exportState.previewPosition;needsDry=exportPreviewDryRequested;}
    const auto cancelled=[&]{
        if(threadShouldExit() || !exportVoice->requested(ticket) || !workflowState)return true;
        const auto flow=workflowState->snapshot();
        const juce::ScopedLock guard(lock);return sessionGeneration.load()!=session || flow.mode!=workflow::Mode::finalise || flow.pass!=pass || liveRevision!=revision;
    };
    juce::String error;
    const auto fail=[&]{{const juce::ScopedLock guard(lock);if(exportVoice->requested(ticket)){
        exportState.message="Preview stopped: "+error;exportVoice->stop(true);}}discard();};
    if(cancelled()){error="Preview dependencies changed";fail();return;}
    const auto context=contextProvider();
    const auto time=context.exportTime();
    if(!pin || context.capture.reference!=pin->plan->sourceReference || (time.valid && !transportMappingMatches(pin->plan->timing,time))){error="Preview source or timing changed";fail();return;}
    if(exportPreviewTicket!=ticket){
        discard();if(!pin->metrics.audibleStart || !pin->metrics.audibleEnd){exportVoice->stop();return;}
        auto layerPlan=std::make_shared<SongExportPlan>(*pin->plan);layerPlan->dry=layerPlan->wet=1;
        const auto dry=pin->plan->loop && !needsDry?0.f:1.f;
        layerPlan->dry=dry;
        bool wetAvailable=!layerPlan->members.empty();
        float liveWetDb=0;
        {const juce::ScopedLock guard(lock);
            liveWetDb=model.liveWetDb;
            const bool solo=std::any_of(model.swells.begin(),model.swells.end(),[](const auto& w){return w.solo;});
            for(const auto& w:model.swells)if(w.enabled && !w.muted && (!solo || w.solo)
                && std::none_of(layerPlan->members.begin(),layerPlan->members.end(),[&](const auto& m){return m.accepted.id==w.id;}))wetAvailable=false;
        }
        if(!wetAvailable)layerPlan->wet=0;
        if(wetAvailable && pin->plan->loop){
            // Warm the complete accepted layer window for fader edits. The
            // delivered dry-only file still starts at zero and has no wet tail.
            Session fixed;fixed.timing=pin->plan->timing;fixed.gainPolicy=pin->plan->gainPolicy;fixed.gainRevision=pin->plan->gainRevision;
            for(const auto& member:pin->plan->members)fixed.swells.push_back(member.accepted);
            std::shared_ptr<const SongExportPlan> expanded;
            if(prepareLoopExportPlan(fixed,pin->plan->source,pin->plan->sourceReference,*pin->plan->loop,dry,1,expanded,error)){
                layerPlan=std::make_shared<SongExportPlan>(*expanded);
                if(pin->plan->wet>0)layerPlan->loopWetGain=pin->plan->loopWetGain;
                else if(!prepareLoopExportWetGain(*layerPlan,liveWetDb,instanceRoot(layerPlan->source),error,cancelled)){
                    if(cancelled()){fail();return;}
                    error.clear();wetAvailable=false;layerPlan->wet=0;
                }
            }
            else if(pin->plan->wet>0){fail();return;}
            else {error.clear();wetAvailable=false;layerPlan->wet=0;}
        }
        uint64_t largestChunk=0;
        if(layerPlan->dry>0)for(const auto& range:layerPlan->source.ranges)largestChunk=std::max(largestChunk,range.frameCount);
        const auto budget=uint64_t{1024*1024}+layerPlan->members.size()*16384+largestChunk*4*sizeof(float);
        if(budget>available()){if(activeJobBytes)return;error="Preview layer working budget unavailable";fail();return;}
        exportPreviewBytes=budget;auto reader=std::make_unique<SongMix>(pin->storage?pin->storage->projectsRoot.getChildFile(layerPlan->source.projectId).getChildFile(layerPlan->source.instanceId):instanceRoot(layerPlan->source));
        if(!reader->prepare(layerPlan,budget,error,cancelled)){
            // A dry-only preview has no wet-file dependency. Keep that path usable,
            // but revoke playback if a later edit requires the unavailable layer.
            if(pin->plan->wet!=0 || cancelled()){fail();return;}
            layerPlan->wet=0;wetAvailable=false;
            if(!reader->prepare(layerPlan,budget,error,cancelled)){fail();return;}
        }
        {const juce::ScopedLock guard(lock);exportPreviewWetAvailable=wetAvailable;
            if(!wetAvailable && model.exportState.levels(pin->plan->loop.has_value()).wet>0){error="Accepted wet layer unavailable";}}
        if(error.isNotEmpty()){fail();return;}
        if(pin->plan->dry>0 && reader->plan()->warnings!=pin->plan->warnings){error="Captured dry availability changed";fail();return;}
        if(cancelled()){error="Preview superseded";fail();return;}
        exportPreviewReader=std::move(reader);exportPreviewTicket=ticket;
        exportPreviewStart=std::clamp(parked.value_or(*pin->metrics.audibleStart),pin->plan->origin,pin->plan->end-1);exportPreviewNext=exportPreviewStart;
    }
    const auto end=exportPreviewReader->plan()->end;if(exportPreviewNext==end){exportVoice->arm(ticket);return;}
    std::array<float,exportPageFrames*2> dry{},wet{};size_t supplied=0;
    while(exportPreviewNext<end && !cancelled() && exportVoice->hasRoom(ticket)){
        const auto frames=static_cast<size_t>(std::min(int64_t{exportPageFrames},end-exportPreviewNext));
        if(!exportPreviewReader->readLayers(exportPreviewNext,{dry.data(),frames*2},{wet.data(),frames*2},error)){fail();return;}
        if(!exportVoice->pushLayers(ticket,exportPreviewNext,exportPreviewStart,end,pin->plan->timing.rate,{dry.data(),frames*2},{wet.data(),frames*2}))break;
        exportPreviewNext+=static_cast<int64_t>(frames);++supplied;
    }
    if(supplied || exportPreviewNext==end)exportVoice->arm(ticket);
}
void SwellEngine::prepareExport(const PreviewContext& sourceContext)
{
    auto context=sourceContext;
    context.time=context.exportTime();
    if(!workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise){
        const juce::ScopedLock guard(lock);exportAttempt.clear();exportRetryFile.reset();exportState={};detailWork.reset();detailSerial.fetch_add(1);return;
    }
    // A coherent host-time read can temporarily be unavailable during callback
    // publication. Wait as inspect/prepareTransport do; do not turn that miss
    // into a new dependency key which cancels a verified current preview.
    if(!context.time.valid)return;
    std::shared_ptr<const PreparedExport> previous;std::shared_ptr<const SongMixDetail> previousDetail;
    Session accepted;ExportFile retryFile;ExportRequest owner;uint64_t pass=0,revision=0,failedDelivery=0;juce::String notice,key,error,deliveryFailure;
    const auto directory=instanceRoot(context.capture.manifest);
    {
        const juce::ScopedLock guard(lock);
        const auto flow=workflowState->snapshot();if(flow.mode!=workflow::Mode::finalise || flow.locked())return;
        owner={sessionGeneration.load(),exportRequest.load(),context.capture.manifest.projectId,context.capture.manifest.instanceId};
        pass=flow.pass;revision=liveRevision;
        int64_t origin=0;const auto mapped=resolvePpqSample(context.time,0,origin);
        key=juce::String(owner.session)+":"+juce::String(owner.generation)+":"+juce::String(pass)+":"+juce::String(revision)
            +":"+context.capture.reference+":"+juce::String(context.captureGeneration)+":"+juce::String(static_cast<int>(mapped))+":"+juce::String(origin)
            +":"+juce::String(context.time.rate,12)+":"+juce::String(context.time.bpm,12)+":"+juce::String(context.time.numerator)+":"+juce::String(context.time.denominator);
        if(key==exportAttempt)return;
        const auto levels=model.exportState.levels(context.capture.sourceKind()==CaptureSourceKind::loop);
        if(levels.dry==0 && levels.wet==0){
            exportAttempt=key;exportVoice->stop();exportRetryFile.reset();detailWork.reset();detailSerial.fetch_add(1);
            exportState={ExportPhase::idle,"Silent: Dry 0% / Wet 0%",{}};exportState.session=owner.session;exportState.pass=pass;return;
        }
        if(exportRetryFile && exportRetryFile->first!=key)exportRetryFile.reset();
        if(exportRetryFile)retryFile=exportRetryFile->second;
        if(exportAttempt.isEmpty() && exportState.phase==ExportPhase::error && exportState.session==owner.session && exportState.pass==pass){failedDelivery=exportState.delivery;deliveryFailure=exportState.deliveryMessage;}
        previous=exportState.prepared?exportState.prepared:exportState.previous;previousDetail=exportState.detail;
        if(exportState.session!=owner.session || exportState.pass!=pass){previous.reset();previousDetail.reset();}
        exportAttempt=key;if(!exportPreviewPin || exportPreviewSession!=owner.session || exportPreviewPass!=pass || exportPreviewRevision!=revision)exportVoice->stop();exportState={ExportPhase::preparing,"Preparing export mix",{}};exportState.session=owner.session;exportState.pass=pass;exportState.previous=previous;exportState.detail=previousDetail;exportState.delivery=failedDelivery;exportState.deliveryMessage=deliveryFailure;accepted=model;
        juce::StringArray retained;
        const bool solo=std::any_of(accepted.swells.begin(),accepted.swells.end(),[](const auto& w){return w.solo;});
        for(auto& w:accepted.swells){
            if(context.capture.sourceKind()==CaptureSourceKind::loop && context.capture.unavailable
               && (!w.frozen || (solo && !w.solo))){
                if(w.enabled && !w.muted && (!solo || w.solo))retained.add(w.name+": dry source unavailable");
                w.enabled=false;w.solo=false;
            }
            if(levels.wet==0 || !w.enabled || w.muted || (solo && !w.solo))continue;
            if(const auto kept=retainedIR.find(w.id);kept!=retainedIR.end() && retainedCompatible(kept->second,w,context)
                && (w.phase!=Phase::ready || (w.render==kept->second.accepted.render && w.ir.content!=kept->second.accepted.ir.content))){
                const auto enabled=w.enabled,muted=w.muted,isSolo=w.solo;w=kept->second.accepted;
                w.enabled=enabled;w.muted=muted;w.solo=isSolo;w.phase=Phase::ready;
                retained.add(w.name+": "+w.irName+" (replacement pending)");
            }else if(const auto r=runtime.find(w.id);r==runtime.end() || r->second.verifiedRender!=w.render){
                error="Waiting for verified wet: "+w.name;break;
            }
        }
        notice=retained.joinIntoString("; ");
    }
    const auto cancelled=[&]{
        if(threadShouldExit() || offlineActive.load() || sessionGeneration.load()!=owner.session || exportRequest.load()!=owner.generation)return true;
        const auto flow=workflowState->snapshot();if(flow.mode!=workflow::Mode::finalise || flow.pass!=pass)return true;
        const juce::ScopedLock guard(lock);return liveRevision!=revision;
    };
    const auto fail=[&](const juce::String& reason){const juce::ScopedLock guard(lock);
        if(!cancelled()){auto summary=exportState.preview;exportState={ExportPhase::error,reason,{}};exportState.preview=std::move(summary);exportState.session=owner.session;exportState.pass=pass;exportState.previous=previous;exportState.detail=previousDetail;exportState.delivery=failedDelivery;exportState.deliveryMessage=deliveryFailure;}};
    if(error.isNotEmpty()){fail(error);return;}
    std::shared_ptr<const SongExportPlan> plan;
    const auto levels=accepted.exportState.levels(context.capture.sourceKind()==CaptureSourceKind::loop);
    const auto planned=context.capture.sourceKind()==CaptureSourceKind::loop
        ?prepareLoopExportPlan(accepted,context.capture.manifest,context.capture.reference,accepted.exportState.loop.options,levels.dry,levels.wet,plan,error)
        :prepareSongExportPlan(accepted,context.capture.manifest,context.capture.reference,context.time,levels.dry,levels.wet,plan,error);
    if(!planned){
        fail(error);return;
    }
    if(plan->loop && plan->wet>0){
        auto matched=std::make_shared<SongExportPlan>(*plan);
        if(!prepareLoopExportWetGain(*matched,accepted.liveWetDb,directory,error,cancelled)){fail(error);return;}
        plan=std::move(matched);
    }
    // The same worker owns transport, render and composition preparation. Reserve
    // the remaining shared budget, including publication scratch, for this job.
    constexpr uint64_t publicationScratch=1024*1024;
    const auto budget=available();if(budget<=publicationScratch){fail("Export working budget unavailable");return;}
    struct Reservation{
        SwellEngine& engine;
        ~Reservation(){const juce::ScopedLock guard(engine.lock);engine.activeJobBytes=0;}
    } reservation{*this};
    {const juce::ScopedLock guard(lock);activeJobBytes=budget;}
    const auto service=[&]{serviceExportPreview();return cancelled();};
    SongMix mix(directory);
    if(!mix.prepare(plan,budget-publicationScratch,error,service) || !mix.measure(error,service)){
        fail(error);return;
    }
    plan=mix.plan();
    {
        const juce::ScopedLock guard(lock);if(cancelled())return;
        activeJobBytes=mix.workingBytes()+publicationScratch;
        auto preview=std::make_shared<PreparedExport>();preview->plan=plan;preview->storage=context.capture.location;preview->metrics=mix.metrics();preview->acceptedNotice=notice;exportState.preview=std::move(preview);
        if(plan->dry>0 && exportPreviewReader && exportPreviewReader->plan()->warnings!=plan->warnings){
            exportVoice->stop(true);exportState.message="Preview stopped: captured dry availability changed";
        }
        if(exportVoice->requested(exportVoice->request()) && exportPreviewSession==owner.session && exportPreviewPass==pass && exportPreviewRevision==revision)
            exportVoice->setLevels(exportVoice->request(),plan->dry,plan->wet,mix.metrics().audibleEnd.value_or(plan->origin));
        if(model.exportState.desiredIdentity!=plan->identity){model.exportState.desiredIdentity=plan->identity;persistentChangedLocked();}
    }
    exportWaiting.store(true);
    while(exportHold.load() && !cancelled()){serviceExportPreview();wait(2);}
    exportWaiting.store(false);if(cancelled())return;
    ExportFile file;
    // Reuse only independently verified immutable bytes; missing/corrupt history
    // is retained, and recovery publishes to a new unique path.
    for(auto it=accepted.exportState.history.rbegin();it!=accepted.exportState.history.rend();++it)
        if(it->composition==plan->identity && verifyExportFile(directory,*it,error,service)){file=*it;break;}
    if(file.path.isEmpty() && retryFile.composition==plan->identity && verifyExportFile(directory,retryFile,error,service))file=retryFile;
    if(cancelled())return;
    if(file.path.isEmpty()){
        if(accepted.exportState.history.size()>=maximumExportRevisions){fail("Export revision history is full; earlier WAV files retained");return;}
        if(!publishSongExport(mix,directory,file,error,service)){fail(error);return;}
    }
    const auto latest=contextProvider();
    if(latest.capture.reference!=context.capture.reference || latest.capture.manifest.projectId!=owner.project.toStdString()
        || latest.capture.manifest.instanceId!=owner.instance.toStdString() || !transportMappingMatches(context.time,latest.exportTime())){
        const juce::ScopedLock guard(lock);
        // Retry a missed publication check without accepting stale timing or
        // writing another immutable WAV for this exact request. Keep it private
        // until the existing ownership and compatibility checks all succeed.
        if(!cancelled() && exportAttempt==key){exportRetryFile=std::make_pair(key,file);exportAttempt.clear();}
        return;
    }
    auto prepared=std::make_shared<PreparedExport>();prepared->plan=plan;prepared->storage=context.capture.location;prepared->metrics=mix.metrics();prepared->file=file;prepared->acceptedNotice=notice;
    {
        const juce::ScopedLock guard(lock);if(cancelled())return;
        exportRetryFile.reset();
        const auto before=model.exportState;
        if(std::none_of(model.exportState.history.begin(),model.exportState.history.end(),[&](const auto& f){return f.path==file.path;}))model.exportState.history.push_back(file);
        model.exportState.completedPath=file.path;model.exportState.desiredIdentity=plan->identity;
        activeJobBytes=mix.workingBytes()+publicationScratch;
        exportState={ExportPhase::ready,"Export WAV ready",std::move(prepared)};exportState.session=owner.session;exportState.pass=pass;exportState.previous=previous;exportState.detail=previousDetail;exportState.delivery=failedDelivery;exportState.deliveryMessage=deliveryFailure;
        if(before!=model.exportState)persistentChangedLocked();
    }
}
}

