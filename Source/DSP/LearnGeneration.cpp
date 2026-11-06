#include "SwellEngine.h"
#include <Capture/SourceVisitor.h>

namespace m3
{
CandidateIdentity SwellEngine::learnIdentity(const PreviewContext& context,bool provisional,uint64_t pass)const
{
    CandidateIdentity identity;identity.session=sessionGeneration.load();identity.captureGeneration=context.captureGeneration;
    identity.sourceRevision=context.capture.manifest.revision;identity.sourceReference=context.capture.reference;
    identity.scope=juce::String(context.capture.manifest.projectId)+"/"+context.capture.manifest.instanceId;
    identity.settings=markerGeneration.load();identity.sensitivity=model.sensitivity;identity.separationSeconds=model.separationSeconds;
    identity.boundaries=acceptedPassSequence;
    identity.timing=context.time;identity.provisional=provisional;identity.pass=provisional?pass:0;return identity;
}
void SwellEngine::useProvisionalSource(std::function<std::optional<ProvisionalCandidateSource>()> provider)
{const juce::ScopedLock guard(lock);provisionalSource=std::move(provider);}
void SwellEngine::cancelLearnGenerationLocked()
{
    ++learnGenerationCancellation;learnRequest.reset();stopIntent.reset();
}
void SwellEngine::invalidateLearnLocked()
{
    ++learnAnalysisCancellation;cancelLearnGenerationLocked();learnAttempt.reset();learnUpdating=true;
    learnNotice.clear();learnFailed=false;
}
LearnGenerationView SwellEngine::learnViewLocked()const
{
    LearnGenerationView view;view.displayed=learnCandidates;view.updating=learnUpdating;
    view.generating=learnGenerating || learnRequest.has_value();view.notice=learnNotice;view.failed=learnFailed;
    const auto flow=workflowState?workflowState->snapshot():workflow::Snapshot{};
    if(!workflowState || flow.mode!=workflow::Mode::capture)view.reason="Generate Swells is available in Learn.";
    else if(flow.locked() || transportActive.load())view.reason="Finish recording before generating swells.";
    else if(view.generating)view.reason="Generating swells...";
    else if(!model.automaticProcessing || !model.autoMarkersEnabled)view.reason="Enable Automatic Swells to generate.";
    else if(!learnUsable)view.reason=learnReason.isNotEmpty()?learnReason:"Capture usable audio before generating swells.";
    else if(view.updating || !learnCandidates || learnCandidates->identity.provisional
        || learnCandidates->identity.session!=sessionGeneration.load() || learnCandidates->identity.settings!=markerGeneration.load())
        view.reason=learnReason.isNotEmpty()?learnReason:"Analysing captured audio... markers updating.";
    else if(defaultPending)view.reason="Finish preparing the Default IR before generating swells.";
    else if(!model.defaultIR.present())view.reason="Choose a valid Default IR before generating swells.";
    else {view.canGenerate=true;view.reason=juce::String(static_cast<int>(learnCandidates->positions.size()))+" automatic markers found. Generate Swells to apply.";}
    return view;
}
bool SwellEngine::setAutoGenerateOnStop(bool enabled,uint64_t expectedSession,uint64_t expectedPass)
{
    const juce::ScopedLock guard(lock);const auto flow=workflowState?workflowState->snapshot():workflow::Snapshot{};
    if((expectedSession && expectedSession!=sessionGeneration.load()) || (expectedPass!=UINT64_MAX && flow.pass!=expectedPass)
        || !workflowState || flow.mode!=workflow::Mode::capture || (enabled && flow.locked()))return false;
    if(model.autoGenerateOnStop==enabled)return true;
    model.autoGenerateOnStop=enabled;if(!enabled)cancelLearnGenerationLocked();
    persistentChangedLocked();notify();return true;
}
bool SwellEngine::requestLearnLocked(const PreviewContext& context,uint64_t session,uint64_t pass,uint64_t serial,bool automatic)
{
    const auto flow=workflowState?workflowState->snapshot():workflow::Snapshot{};
    const auto view=learnViewLocked();
    if(session!=sessionGeneration.load() || flow.pass!=pass || !view.canGenerate || !learnCandidates
        || learnCandidates->serial!=serial || !sameCandidateIdentity(learnCandidates->identity,learnIdentity(context)))
    {
        learnNotice=view.canGenerate?"Automatic markers changed. Wait for current analysis and try again.":view.reason;
        learnFailed=false;return false;
    }
    actionError.clear();LearnRequest request;request.candidates=learnCandidates;request.expected=model;request.source=context.capture.manifest;
    request.flow=flow;request.revision=modelRevision;request.cancellation=learnGenerationCancellation.load();request.automatic=automatic;
    learnRequest=std::move(request);++learnRenderCancellation;learnNotice.clear();learnFailed=false;notify();return true;
}
bool SwellEngine::generateLearn(uint64_t expectedSession,uint64_t expectedPass,uint64_t serial)
{
    const auto context=contextProvider();const juce::ScopedLock guard(lock);
    return requestLearnLocked(context,expectedSession,expectedPass,serial,false);
}
void SwellEngine::captureSettled(uint64_t pass,uint64_t capture,bool useful)
{
    const auto context=contextProvider();collectCompletedPass(context,true);const juce::ScopedLock guard(lock);
    const auto flow=workflowState?workflowState->snapshot():workflow::Snapshot{};
    if(!learnCandidates || !sameCandidateIdentity(learnCandidates->identity,learnIdentity(context)))
    {learnUpdating=true;learnAttempt.reset();}
    if(!useful || !workflowState || flow.pass!=pass || flow.phase!=workflow::Phase::validating
        || context.captureGeneration!=capture || !model.autoGenerateOnStop || !model.automaticProcessing || !model.autoMarkersEnabled)return;
    StopIntent intent;intent.source=learnIdentity(context);intent.pass=pass;intent.configuration=configurationRevision;
    intent.revision=learnGenerationCancellation.load();intent.ir=model.defaultIR;stopIntent=std::move(intent);notify();
}
bool SwellEngine::claimInitialPerform(uint64_t session,uint64_t handoff)
{
    const juce::ScopedLock guard(lock);
    if(session!=sessionGeneration.load() || !handoff || handoff!=initialPerformOffer || performPositionEstablished
        || !workflowState || workflowState->snapshot().mode!=workflow::Mode::live || model.swells.empty())return false;
    initialPerformOffer=0;performPositionEstablished=true;return true;
}
void SwellEngine::establishPerformPosition(uint64_t session)
{
    const juce::ScopedLock guard(lock);
    if(session==sessionGeneration.load() && workflowState && workflowState->snapshot().mode==workflow::Mode::live && !model.swells.empty())
    {performPositionEstablished=true;initialPerformOffer=0;}
}
bool SwellEngine::selectInitialPerform(uint64_t session,uint64_t handoff,const juce::String& id)
{
    const juce::ScopedLock guard(lock);
    if(!find(model,id) || !claimInitialPerform(session,handoff))return false;
    return selectTargets({id},id,session);
}
bool SwellEngine::learnGenerationStep(const PreviewContext& accepted)
{
    if(!workflowState)return false;
    std::optional<LearnRequest> generation;
    {
        const juce::ScopedLock guard(lock);
        if(learnRequest){generation=std::move(learnRequest);learnRequest.reset();learnGenerating=true;}
    }
    if(generation){performLearnGeneration(std::move(*generation));return true;}
    const auto flow=workflowState->snapshot();
    if(flow.mode!=workflow::Mode::capture)
    {const juce::ScopedLock guard(lock);stopIntent.reset();return false;}
    PreviewContext context=accepted;
    const bool provisional=flow.locked();
    std::function<std::optional<ProvisionalCandidateSource>()> provider;
    {const juce::ScopedLock guard(lock);provider=provisionalSource;}
    if(provisional && provider)if(const auto staged=provider();staged && staged->pass==flow.pass && staged->epoch==accepted.captureGeneration)
        context.capture.manifest=staged->manifest;
    CandidateIdentity identity;CompletedPasses completed;uint64_t cancellation;
    {
        const juce::ScopedLock guard(lock);
        identity=learnIdentity(context,provisional,flow.pass);
        if(!provisional && learnCandidates && learnCandidates->identity.provisional)
        {learnCandidates=settledLearnCandidates;learnAttempt.reset();}
        learnUsable=!context.capture.unavailable && !context.capture.status.startsWith("Loading") && context.time.valid
            && context.capture.manifest.sampleRate==context.time.rate
            && (!model.timing.valid || transportMappingMatches(model.timing,context.time))
            && std::any_of(context.capture.manifest.ranges.begin(),context.capture.manifest.ranges.end(),[](const auto& range)
                {return range.kind==CaptureCoverageKind::audio;});
        if(!model.automaticProcessing || !model.autoMarkersEnabled)
        {learnReason="Automatic Swells is off.";learnUpdating=true;stopIntent.reset();return false;}
        if(!learnUsable)
        {learnReason="Capture usable audio with compatible timing before generating swells.";learnUpdating=true;stopIntent.reset();return false;}
        if(stopIntent)
        {
            if(stopIntent->pass!=flow.pass || stopIntent->configuration!=configurationRevision
                || stopIntent->revision!=learnGenerationCancellation.load() || stopIntent->ir!=model.defaultIR
                || !sameCandidateIdentity(stopIntent->source,learnIdentity(accepted)) || !model.autoGenerateOnStop)
                stopIntent.reset();
            else if(!flow.locked() && flow.result==workflow::Result::accepted && !learnUpdating && learnCandidates
                && sameCandidateIdentity(learnCandidates->identity,identity))
            {
                requestLearnLocked(accepted,identity.session,flow.pass,learnCandidates->serial,true);stopIntent.reset();return true;
            }
        }
        if(learnCandidates && sameCandidateIdentity(learnCandidates->identity,identity))
        {learnUpdating=false;learnReason.clear();return false;}
        learnUpdating=true;learnReason="Analysing captured audio... markers updating.";
        if(juce::Time::getMillisecondCounterHiRes()<nextMarkerAnalysis || (learnAttempt && sameCandidateIdentity(*learnAttempt,identity)))return false;
        learnAttempt=identity;completed=applicableCompletedPasses(context.capture.manifest);cancellation=learnAnalysisCancellation.load();
    }
    const auto cancel=[&]{return threadShouldExit() || identity.session!=sessionGeneration.load()
        || identity.settings!=markerGeneration.load() || cancellation!=learnAnalysisCancellation.load()
        || (captureEpochProvider && captureEpochProvider()!=identity.captureGeneration)
        || workflowState->snapshot().pass!=flow.pass || workflowState->snapshot().mode!=workflow::Mode::capture;};
    while(learnAnalysisHold.load() && !cancel()){learnAnalysisWaiting=true;wait(2);}learnAnalysisWaiting=false;
    LearnCandidateSnapshot scanned;juce::String error;CaptureAssetStore store(instanceRoot(context.capture.manifest));
    bool ok=false;
    try{ok=!cancel() && scanLearnCandidates(store,context.capture.manifest,identity,completed,scanned,error,cancel);}
    catch(const std::exception& failure){error="Could not analyse captured source: "+juce::String(failure.what());}
    auto latest=contextProvider();
    if(provisional && provider)if(const auto staged=provider();staged && staged->pass==flow.pass && staged->epoch==latest.captureGeneration)
        latest.capture.manifest=staged->manifest;
    const juce::ScopedLock guard(lock);
    if(cancel() || !sameCandidateIdentity(identity,learnIdentity(latest,provisional,flow.pass)))
    {
        // A cancelled attempt is not a cached analysis result. In particular,
        // leaving Learn and returning with the same source/settings must retry.
        if(learnAttempt && sameCandidateIdentity(*learnAttempt,identity))learnAttempt.reset();
        return true;
    }
    if(!ok){learnReason=error.isNotEmpty()?error:"Candidate analysis was cancelled; previous markers retained.";learnNotice=learnReason;learnFailed=true;return true;}
    scanned.serial=++learnCandidateSerial;learnCandidates=std::make_shared<const LearnCandidateSnapshot>(std::move(scanned));
    if(!provisional)settledLearnCandidates=learnCandidates;
    ++automaticAnalysisCount;learnUpdating=false;learnReason.clear();learnNotice.clear();learnFailed=false;notify();return true;
}
void SwellEngine::performLearnGeneration(LearnRequest request)
{
    struct Finish{SwellEngine& engine;~Finish(){const juce::ScopedLock guard(engine.lock);engine.learnGenerating=false;engine.learnGenerationWaiting=false;}}finish{*this};
    {
        const juce::ScopedLock guard(lock);
        // This worker may have finished an earlier render after request admission.
        // Keep its accepted assets/progress without accepting a newer musical edit.
        if(request.candidates->identity.session==sessionGeneration.load() && request.revision==modelRevision
            && model.selected==request.expected.selected && model.primary==request.expected.primary)
            request.expected=model;
    }
    const auto cancel=[&]
    {
        const auto flow=workflowState->snapshot();
        return threadShouldExit() || request.candidates->identity.session!=sessionGeneration.load()
            || request.cancellation!=learnGenerationCancellation.load() || transportActive.load()
            || flow.mode!=workflow::Mode::capture || flow.locked() || flow.pass!=request.flow.pass
            || flow.configurationSlot!=request.flow.configurationSlot
            || (captureEpochProvider && captureEpochProvider()!=request.candidates->identity.captureGeneration);
    };
    while(learnGenerationHold.load() && !cancel()){learnGenerationWaiting=true;wait(2);}learnGenerationWaiting=false;
    AutomaticGenerationPlan plan;juce::String error;
    try
    {
        SwellAssetStore assets(instanceRoot(request.source));
        ImpulseResponse impulse;
        if(cancel())plan.outcome=GenerationOutcome::cancelled;
        else if(!assets.readIR(request.expected.defaultIR,available(),impulse,error))plan.message="Could not generate swells. Existing swells were kept. "+error;
        else
        {
            CaptureAssetStore source(instanceRoot(request.source));
            bool valid=true;
            for(const auto& run:request.candidates->mappedRuns)
                if(!visitSource(source,request.source,run.first,run.second,false,[](int64_t,const float*,int,bool){return true;},error,cancel))
                {valid=false;break;}
            if(!valid)plan.message="Could not generate swells. Existing swells were kept. "+error;
            else plan=prepareAutomaticGeneration(request.expected,request.candidates->positions,request.source,request.candidates->identity.timing,
                    request.candidates->completedPasses,[&](int64_t first,int64_t last,juce::String& identity,juce::String& failure)
                        {return sourceFingerprint(source,request.source,first,last,identity,failure,cancel);},cancel);
        }
    }
    catch(const std::exception& failure){plan.outcome=GenerationOutcome::failed;plan.message="Could not generate swells. Existing swells were kept. "+juce::String(failure.what());}
    const auto latest=contextProvider();const juce::ScopedLock guard(lock);
    if(request.candidates->identity.session!=sessionGeneration.load())return;
    if(cancel() || request.revision!=modelRevision || model.selected!=request.expected.selected || model.primary!=request.expected.primary
        || !sameCandidateIdentity(request.candidates->identity,learnIdentity(latest)))
    {learnNotice="Generation cancelled after a newer change. Existing swells were kept.";learnFailed=false;return;}
    if(plan.outcome!=GenerationOutcome::generated)
    {learnNotice=plan.message.isNotEmpty()?plan.message:"Generation cancelled. Existing swells were kept.";learnFailed=plan.outcome==GenerationOutcome::failed;return;}
    // This is the only reserved work: prepared vector/string ownership swaps.
    // Asset verification, allocation, invalidation and host notifications occur outside it.
    if(!workflowState->publishGenerated(request.flow,[&]()noexcept{std::swap(model,plan.replacement);}))
    {learnNotice="Generation was superseded. Existing swells were kept.";learnFailed=false;return;}
    for(const auto& id:plan.changed)
    {
        if(const auto old=find(plan.replacement,id))retainIRLocked(*old,latest,model.defaultIRName);
        forceRefresh.insert(id);invalidateLiveLocked(id);
    }
    for(const auto& id:plan.retired){runtime.erase(id);retainedIR.erase(id);forceRefresh.erase(id);invalidateLiveLocked(id);}
    for(const auto& w:model.swells)if(w.provenance!=Provenance::automatic)protectedGenerationCalibration.insert(w.id);
    dirty.clear();passDirty.clear();passRenderRequests.clear();analyzedContent.clear();automaticBoundsDirty=false;performWindowsRequested=false;
    ++successfulGeneration;if(!performPositionEstablished)initialPerformOffer=successfulGeneration;
    learnNotice.clear();learnFailed=false;message=plan.message;persistentChangedLocked();notify();
}
}
