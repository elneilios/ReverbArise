#include "SwellEngine.h"

namespace m3
{
void SwellEngine::requestExportDetail(int64_t first,int64_t end,size_t bins,uint64_t expectedSession,const juce::String& composition)
{
    const juce::ScopedLock guard(lock);if(expectedSession!=sessionGeneration.load() || !workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise)return;
    auto accepted=exportState.prepared;
    if(!accepted || accepted->plan->identity!=composition)accepted=exportState.previous;
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
    SongMix mix(instanceRoot(job->accepted->plan->source));auto detail=std::make_shared<SongMixDetail>();
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
        if(hasCompatibleReadyRender(w,time))return true;
        if(const auto kept=retainedIR.find(w.id);kept!=retainedIR.end() && retainedCompatible(kept->second,w,context)
            && hasCompatibleReadyRender(kept->second.accepted,time))return true;
    }return false;
}

uint64_t SwellEngine::deliverExport(uint64_t expectedSession,const ExportFile& file,const juce::File& destination,bool overwrite,bool drag,uint64_t expectedPass)
{
    const juce::ScopedLock guard(lock);
    if(expectedSession!=sessionGeneration.load() || !workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise
        || exportState.pass!=workflowState->snapshot().pass || exportState.session!=sessionGeneration.load()
        || exportState.phase!=ExportPhase::ready || !exportState.prepared || exportState.prepared->file!=file || (!drag && destination==juce::File{}))return 0;
    if(expectedPass!=UINT64_MAX && workflowState->snapshot().pass!=expectedPass)return 0;
    if(!drag && (destination==root || destination.isAChildOf(root))){exportState.deliveryMessage="Choose a Save location outside managed plugin storage";return 0;}
    const auto id=++exportDeliverySerial;exportVoice->stop();
    exportDelivery=ExportDelivery{id,expectedSession,exportRequest.load(),workflowState->snapshot().pass,file,
        instanceRoot(exportState.prepared->plan->source),destination,overwrite,drag};
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
bool SwellEngine::playExport(uint64_t expectedSession)
{
    const juce::ScopedLock guard(lock);
    if(expectedSession!=sessionGeneration.load() || !workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise
        || exportState.pass!=workflowState->snapshot().pass || exportState.session!=sessionGeneration.load()
        || exportState.phase!=ExportPhase::ready || !exportState.prepared || !exportState.prepared->metrics.audibleStart)return false;
    setExpected(exportState.prepared->plan->timing);exportVoice->requestStart();notify();return true;
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
    if(exportVoice->requested(exportVoice->request()))exportVoice->requestStart();
    notify();return true;
}
void SwellEngine::serviceExportPreview()
{
    const auto ticket=exportVoice->request();
    if(!workflowState || workflowState->snapshot().mode!=workflow::Mode::finalise || !exportVoice->requested(ticket)){
        exportPreviewReader.reset();exportPreviewPin.reset();exportPreviewTicket=0;return;
    }
    juce::String error;
    const auto cancelled=[&]{return threadShouldExit() || !exportVoice->requested(ticket)
        || workflowState->snapshot().mode!=workflow::Mode::finalise;};
    const auto fail=[&]{const juce::ScopedLock guard(lock);if(!cancelled()){
        exportState.message="Preview stopped: "+error;exportVoice->stop(true);}};
    if(exportPreviewTicket!=ticket){
        exportPreviewReader.reset();{const juce::ScopedLock guard(lock);exportPreviewPin=exportState.prepared;
            if(exportPreviewPin)exportPreviewStart=exportState.previewPosition.value_or(exportPreviewPin->metrics.audibleStart.value_or(0));}
        if(!exportPreviewPin || !exportPreviewPin->metrics.audibleStart || !exportPreviewPin->metrics.audibleEnd){exportVoice->stop();return;}
        const auto directory=instanceRoot(exportPreviewPin->plan->source);
        if(!verifyExportFile(directory,exportPreviewPin->file,error,cancelled)){fail();return;}
        auto input=directory.getChildFile(exportPreviewPin->file.path).createInputStream();
        if(!input){error="Export WAV is unavailable";fail();return;}
        juce::WavAudioFormat format;exportPreviewReader.reset(format.createReaderFor(input.release(),true));
        if(!exportPreviewReader){error="Export WAV cannot be decoded";fail();return;}
        exportPreviewTicket=ticket;exportPreviewNext=exportPreviewStart;
    }
    const auto start=exportPreviewStart,end=*exportPreviewPin->metrics.audibleEnd;
    if(exportPreviewNext==end){exportVoice->arm(ticket);return;}
    if(!exportVoice->hasRoom(ticket))return;
    juce::AudioBuffer<float> block(2,exportPageFrames);std::array<float,exportPageFrames*2> samples;
    size_t supplied=0;
    while(exportPreviewNext<end && !cancelled() && exportVoice->hasRoom(ticket)){
        const auto frames=static_cast<int>(std::min(int64_t{exportPageFrames},end-exportPreviewNext));
        if(!exportPreviewReader->read(&block,0,frames,exportPreviewNext-exportPreviewPin->file.origin,true,true)){
            error="Export WAV page read failed";fail();return;
        }
        for(int n=0;n<frames;++n)for(int c=0;c<2;++c){const auto value=block.getSample(c,n);
            if(!std::isfinite(value)){error="Export WAV page contains invalid audio";fail();return;}samples[n*2+c]=value;}
        if(!exportVoice->push(ticket,exportPreviewNext,start,end,exportPreviewPin->file.rate,{samples.data(),static_cast<size_t>(frames)*2}))break;
        exportPreviewNext+=frames;++supplied;
    }
    if(supplied || exportPreviewNext==end)exportVoice->arm(ticket);
}
void SwellEngine::prepareExport(const PreviewContext& context)
{
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
        if(exportRetryFile && exportRetryFile->first!=key)exportRetryFile.reset();
        if(exportRetryFile)retryFile=exportRetryFile->second;
        if(exportAttempt.isEmpty() && exportState.phase==ExportPhase::error && exportState.session==owner.session && exportState.pass==pass){failedDelivery=exportState.delivery;deliveryFailure=exportState.deliveryMessage;}
        previous=exportState.prepared?exportState.prepared:exportState.previous;previousDetail=exportState.detail;
        if(exportState.session!=owner.session || exportState.pass!=pass){previous.reset();previousDetail.reset();}
        exportAttempt=key;exportVoice->stop();exportState={ExportPhase::preparing,"Preparing export mix",{}};exportState.session=owner.session;exportState.pass=pass;exportState.previous=previous;exportState.detail=previousDetail;exportState.delivery=failedDelivery;exportState.deliveryMessage=deliveryFailure;accepted=model;
        juce::StringArray retained;
        const bool solo=std::any_of(accepted.swells.begin(),accepted.swells.end(),[](const auto& w){return w.solo;});
        for(auto& w:accepted.swells){
            if(!w.enabled || w.muted || (solo && !w.solo))continue;
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
        if(!cancelled()){exportState={ExportPhase::error,reason,{}};exportState.session=owner.session;exportState.pass=pass;exportState.previous=previous;exportState.detail=previousDetail;exportState.delivery=failedDelivery;exportState.deliveryMessage=deliveryFailure;}};
    if(error.isNotEmpty()){fail(error);return;}
    std::shared_ptr<const SongExportPlan> plan;
    if(!prepareSongExportPlan(accepted,context.capture.manifest,context.capture.reference,context.time,accepted.exportState.wet,plan,error)){
        fail(error);return;
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
    SongMix mix(directory);
    if(!mix.prepare(plan,budget-publicationScratch,error,cancelled) || !mix.measure(error,cancelled)){
        fail(error);return;
    }
    plan=mix.plan();
    {
        const juce::ScopedLock guard(lock);if(cancelled())return;
        if(model.exportState.desiredIdentity!=plan->identity){model.exportState.desiredIdentity=plan->identity;persistentChangedLocked();}
    }
    exportWaiting.store(true);
    while(exportHold.load() && !cancelled())wait(2);
    exportWaiting.store(false);if(cancelled())return;
    ExportFile file;
    // Reuse only independently verified immutable bytes; missing/corrupt history
    // is retained, and recovery publishes to a new unique path.
    for(auto it=accepted.exportState.history.rbegin();it!=accepted.exportState.history.rend();++it)
        if(it->composition==plan->identity && verifyExportFile(directory,*it,error,cancelled)){file=*it;break;}
    if(file.path.isEmpty() && retryFile.composition==plan->identity && verifyExportFile(directory,retryFile,error,cancelled))file=retryFile;
    if(cancelled())return;
    if(file.path.isEmpty()){
        if(accepted.exportState.history.size()>=maximumExportRevisions){fail("Export revision history is full; earlier WAV files retained");return;}
        if(!publishSongExport(mix,directory,file,error,cancelled)){fail(error);return;}
    }
    const auto latest=contextProvider();
    if(latest.capture.reference!=context.capture.reference || latest.capture.manifest.projectId!=owner.project.toStdString()
        || latest.capture.manifest.instanceId!=owner.instance.toStdString() || !transportMappingMatches(context.time,latest.time)){
        const juce::ScopedLock guard(lock);
        // Retry a missed publication check without accepting stale timing or
        // writing another immutable WAV for this exact request. Keep it private
        // until the existing ownership and compatibility checks all succeed.
        if(!cancelled() && exportAttempt==key){exportRetryFile=std::make_pair(key,file);exportAttempt.clear();}
        return;
    }
    auto prepared=std::make_shared<PreparedExport>();prepared->plan=plan;prepared->metrics=mix.metrics();prepared->file=file;prepared->acceptedNotice=notice;
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

