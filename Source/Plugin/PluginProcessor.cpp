#include "PluginProcessor.h"
#include <Core/BuildInformation.h>

#include "PluginEditor.h"
#include <Playback/ScheduledTestTone.h>
#include <UI/WorkflowPresentation.h>

#include <algorithm>

juce::File PreverbAudioProcessor::defaultProjectsRoot()
{
    return juce::File::getSpecialLocation(juce::File::windowsLocalAppData)
        .getChildFile("K7MA/Preverb/Projects");
}

PreverbAudioProcessor::PreverbAudioProcessor(juce::File projectsRoot)
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      captureWorker(captureQueue, projectsRoot)
{
    captureWorker.useWorkflow(workflowState);
    swells = std::make_unique<m3::SwellEngine>(std::move(projectsRoot), [this]
    {
        auto capture=captureWorker.snapshot();if(captureWorker.isLoading())capture.status="Loading capture state";
        return PreviewContext{std::move(capture), getSongTime(), captureWorker.captureGeneration()};
    },[this]{persistentStateChanged();});
    swells->setGainPolicy(m3::GainPolicy::musical);
    swells->setAuditionWetOnly(false,swells->assignmentEpoch());
    swells->useWorkflow(workflowState,[this]{return captureWorker.captureGeneration();});
    swells->useProvisionalSource([this]{return captureWorker.provisionalSnapshot();});
    captureWorker.setSettlementHandler([this](uint64_t pass,uint64_t generation,bool useful){swells->captureSettled(pass,generation,useful);});
    captureWorker.setPassContextProvider([this](uint64_t pass)->std::optional<CaptureGenerationContext>
    {
        const auto pinned=swells->passConfiguration(pass);if(!pinned)return {};
        CaptureGenerationContext context;context.automatic=pinned->automatic;context.sensitivity=pinned->sensitivity;
        context.separationSeconds=pinned->separationSeconds;context.ir=pinned->ir;context.irName=pinned->irName;
        context.id=captureGenerationContextId(context);return context;
    });
    captureWorker.setRecaptureHandler([this](int64_t a,int64_t b) { swells->recaptureCompleted(a,b); });
    captureWorker.setCommitHandler([this](int64_t a,int64_t b,uint64_t generation,bool recapture) {swells->captureCommitted(a,b,generation,recapture);});
    captureWorker.setAcceptedStateHandler([this]{persistentStateChanged();});
    cancelPendingUpdate(); // Constructor defaults are not user edits.
    hostNotificationsEnabled.store(true);
    captureWorker.startThread();
}

PreverbAudioProcessor::~PreverbAudioProcessor()
{
    hostNotificationsEnabled.store(false);
    captureWorker.stopThread(-1);
    swells.reset();
    cancelPendingUpdate();
}

void PreverbAudioProcessor::prepareToPlay(double newSampleRate, int)
{
    wasOffline=false;swells->endOffline();swells->resetTransportClock();
    sampleRate.store(newSampleRate, std::memory_order_relaxed);
    captureWorker.setHostRate(newSampleRate);
}

void PreverbAudioProcessor::releaseResources()
{
    wasOffline=false;swells->endOffline();swells->resetTransportClock();
}

bool PreverbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void PreverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateHostPositionDiagnostics(buffer.getNumSamples());
    awaitingRestoredPosition.store(false,std::memory_order_release);
    const auto diagnostics = getHostPositionDiagnostics();
    const TransportTime transport{{diagnostics.positionAvailable && diagnostics.samplePositionAvailable
        && diagnostics.ppqPositionAvailable && diagnostics.tempoAvailable && diagnostics.meterAvailable,
        diagnostics.samplePosition,diagnostics.ppqPosition,diagnostics.sampleRate,diagnostics.bpm,
        diagnostics.numerator,diagnostics.denominator},diagnostics.isPlaying,diagnostics.isLooping,diagnostics.loopStart,diagnostics.loopEnd};
    const auto captureGeneration = captureWorker.captureGeneration();
    const auto offline=isNonRealtime();
    if(offline && !wasOffline)offlineWorkflowMode=workflowState.snapshot().mode;
    wasOffline=offline;
    auto workflow=workflowState.snapshot();
    if(!offline && diagnostics.positionAvailable)
    {
        if(diagnostics.isPlaying && workflow.mode==workflow::Mode::capture && !workflow.locked() && (!previousHostPlaying || previousWorkflowMode!=workflow::Mode::capture))
            if(workflowState.begin(transport.song.valid && captureWorker.canMap()))swells->captureStarted(transport.song.sample);
        if(!diagnostics.isPlaying && workflow.phase==workflow::Phase::recording){swells->captureStopped();workflowState.stop(workflow.pass);}
        previousHostPlaying=diagnostics.isPlaying;
        previousWorkflowMode=workflow.mode;
    }
    workflow=workflowState.snapshot();
    const CaptureHostTiming captureTiming{diagnostics.positionAvailable,diagnostics.samplePositionAvailable,
        diagnostics.ppqPositionAvailable,diagnostics.tempoAvailable,diagnostics.meterAvailable,diagnostics.isPlaying,
        diagnostics.isLooping,diagnostics.samplePosition,diagnostics.loopStart,diagnostics.loopEnd,
        diagnostics.ppqPosition,diagnostics.sampleRate,diagnostics.bpm,diagnostics.numerator,diagnostics.denominator};
    if(!offline && workflow.phase==workflow::Phase::recording && observedCapturePass!=workflow.pass)
    {capturePassTime=transport.song;observedCapturePass=workflow.pass;capturePassDiagnosticBaseline=captureTiming;passQueuedBlocks=passDroppedBlocks=0;}
    const auto rejectCapture=[&](CaptureFailureReason reason)
    {
        droppedCaptureBlockCount.fetch_add(1,std::memory_order_relaxed);++passDroppedBlocks;
        CaptureFailureEvent event;event.reason=reason;event.pass=workflow.pass;event.epoch=captureGeneration;
        event.start=diagnostics.samplePosition;event.frames=buffer.getNumSamples();event.queued=passQueuedBlocks;event.dropped=passDroppedBlocks;
        event.timing=captureTiming;event.baseline=capturePassDiagnosticBaseline;captureWorker.failPass(event);
    };
    if(!offline && workflow.phase==workflow::Phase::recording && !diagnostics.positionAvailable && buffer.getNumSamples()>0)
        rejectCapture(CaptureFailureReason::timingUnavailable);
    if (!offline && workflow.mode==workflow::Mode::capture && workflow.phase==workflow::Phase::recording
        && diagnostics.isPlaying)
    {
        const bool compatiblePass=transportMappingMatches(capturePassTime,transport.song);
        const auto channelCount = buffer.getNumChannels();
        const auto count=buffer.getNumSamples();
        CaptureFailureReason reason=CaptureFailureReason::none;
        if(count<0 || count>maxCaptureBlockFrames)reason=CaptureFailureReason::blockSize;
        else if(channelCount<2)reason=CaptureFailureReason::channels;
        else if(!transport.song.valid || !std::isfinite(transport.song.ppq) || !std::isfinite(transport.song.bpm)
            || transport.song.bpm<=0 || transport.song.numerator<=0 || transport.song.denominator<=0)reason=CaptureFailureReason::timingUnavailable;
        else if(!std::isfinite(transport.song.rate) || transport.song.rate<=0 || std::abs(capturePassTime.rate-transport.song.rate)>.001)reason=CaptureFailureReason::sampleRate;
        else if(diagnostics.samplePosition>INT64_MAX-count)reason=CaptureFailureReason::range;
        else if(!compatiblePass)reason=CaptureFailureReason::timingChanged;
        else if(!captureWorker.canMap())reason=CaptureFailureReason::mappingUnavailable;
        if(count!=0)
        {
            if(reason!=CaptureFailureReason::none)rejectCapture(reason);
            else if(captureQueue.tryPush(diagnostics.samplePosition,transport.song.rate,buffer.getReadPointer(0),buffer.getReadPointer(1),count,captureGeneration,workflow.pass,captureTiming))++passQueuedBlocks;
            else rejectCapture(CaptureFailureReason::queueFull);
        }
    }

    auto tonePeak = 0.0f;
    if (workflow.mode==workflow::Mode::live && testToneEnabled.load(std::memory_order_relaxed)
        && diagnostics.positionAvailable
        && diagnostics.isPlaying
        && diagnostics.samplePositionAvailable
        && diagnostics.ppqPositionAvailable
        && diagnostics.tempoAvailable
        && diagnostics.meterAvailable
        && buffer.getNumChannels() >= 2)
    {
        tonePeak = ScheduledTestTone::addToStereo(buffer.getWritePointer(0),
                                        buffer.getWritePointer(1),
                                        buffer.getNumSamples(),
                                        diagnostics.samplePosition,
                                        diagnostics.ppqPosition,
                                        diagnostics.bpm,
                                        diagnostics.numerator,
                                        diagnostics.denominator,
                                        diagnostics.sampleRate,
                                        toneStartBar.load(std::memory_order_relaxed),
                                        toneEndBar.load(std::memory_order_relaxed));
    }
    generatedTonePeak.store(tonePeak, std::memory_order_relaxed);
    if (buffer.getNumChannels() >= 2)
    {
        if(offline){if(offlineWorkflowMode==workflow::Mode::live)swells->processOfflineAudio(buffer.getWritePointer(0),buffer.getWritePointer(1),buffer.getNumSamples(),transport);}
        else {swells->endOffline();swells->processTransportAudio(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples(), transport);}
    }

    for (auto channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());
}

HostPositionDiagnostics PreverbAudioProcessor::getHostPositionDiagnostics() const noexcept
{
    if(awaitingRestoredPosition.load(std::memory_order_acquire))
    {HostPositionDiagnostics unknown;unknown.sampleRate=sampleRate.load();return unknown;}
    for (int attempt = 0; attempt < 8; ++attempt)
    {
    const auto sequence = diagnosticsSequence.load(std::memory_order_acquire);
    if ((sequence & 1) != 0) continue;
    HostPositionDiagnostics diagnostics;
    diagnostics.positionAvailable = positionAvailable.load(std::memory_order_relaxed);
    diagnostics.samplePositionAvailable = samplePositionAvailable.load(std::memory_order_relaxed);
    diagnostics.samplePosition = samplePosition.load(std::memory_order_relaxed);
    diagnostics.ppqPositionAvailable = ppqPositionAvailable.load(std::memory_order_relaxed);
    diagnostics.ppqPosition = ppqPosition.load(std::memory_order_relaxed);
    diagnostics.tempoAvailable = tempoAvailable.load(std::memory_order_relaxed);
    diagnostics.bpm = bpm.load(std::memory_order_relaxed);
    diagnostics.meterAvailable = meterAvailable.load(std::memory_order_relaxed);
    diagnostics.numerator = numerator.load(std::memory_order_relaxed);
    diagnostics.denominator = denominator.load(std::memory_order_relaxed);
    diagnostics.isPlaying = isPlaying.load(std::memory_order_relaxed);
    diagnostics.isRecording = isRecording.load(std::memory_order_relaxed);
    diagnostics.isLooping=isLooping.load(std::memory_order_relaxed);diagnostics.loopStart=loopStart.load(std::memory_order_relaxed);diagnostics.loopEnd=loopEnd.load(std::memory_order_relaxed);
    diagnostics.blockSize = blockSize.load(std::memory_order_relaxed);
    diagnostics.sampleRate = sampleRate.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if (diagnosticsSequence.load(std::memory_order_relaxed) == sequence) return diagnostics;
    }
    HostPositionDiagnostics unavailable;
    unavailable.sampleRate = sampleRate.load(std::memory_order_relaxed);
    return unavailable;
}

SongTime PreverbAudioProcessor::getSongTime() const noexcept
{
    const auto d = getHostPositionDiagnostics();
    return {d.positionAvailable && d.samplePositionAvailable && d.ppqPositionAvailable
                && d.tempoAvailable && d.meterAvailable,
            d.samplePosition, d.ppqPosition, d.sampleRate, d.bpm, d.numerator, d.denominator};
}

void PreverbAudioProcessor::setMappingEnabled(bool enabled)
{
    // Legacy control now requests a guarded workflow transition; never a second
    // independent source-overwrite authority.
    requestWorkflowMode(enabled?workflow::Mode::capture:workflow::Mode::live);
}

bool PreverbAudioProcessor::isMappingEnabled() const noexcept
{
    return workflowState.snapshot().mode==workflow::Mode::capture;
}

void PreverbAudioProcessor::requestClearCapture()
{
    if(workflowState.snapshot().mode!=workflow::Mode::capture || workflowState.snapshot().locked())
    {swells->diagnostic("Clear requires stopped idle Learn mode");return;}
    swells->clear();
    droppedCaptureBlockCount.store(0, std::memory_order_relaxed);
    captureWorker.requestClear();
    persistentStateChanged();
}

bool PreverbAudioProcessor::armRecapture(int64_t start,int64_t end)
{
    if(start>=end){swells->diagnostic("Invalid recapture interval; arm pass-wide Recapture in Learn mode");return false;}
    const auto accepted=workflowState.arm(true);
    if(!accepted)swells->diagnostic("Recapture requires idle Learn mode; applies to the encountered pass");
    return accepted;
}

workflow::Readiness PreverbAudioProcessor::getWorkflowReadiness() const
{
    const auto capture=captureWorker.snapshot();const auto time=getSongTime();
    const auto session=swells->savedSession();
    const auto solo=std::any_of(session.swells.begin(),session.swells.end(),[](const auto& w){return w.solo;});
    const auto ready=[&](const auto& w){return w.enabled && !w.muted && (!solo || w.solo) && m3::hasCompatibleReadyRender(w,time);};
    const auto frozen=std::any_of(session.swells.begin(),session.swells.end(),[&](const auto& w){return w.frozen && ready(w);});
    return {captureWorker.hasUsefulSource(),frozen || (!capture.unavailable && time.valid && (capture.manifest.ranges.empty() || capture.manifest.sampleRate==time.rate)
        && (!session.timing.valid || sameMusicalMapping(session.timing,time))),
        swells->hasExportableWet(time),frozen};
}
bool PreverbAudioProcessor::requestWorkflowMode(workflow::Mode mode)
{
    const auto previous=workflowState.snapshot().mode;
    const auto readiness=getWorkflowReadiness();
    if(const auto block=workflow::entryBlock(workflowState.snapshot(),mode,readiness);block!=workflow::EntryBlock::none)
    {swells->diagnostic(m3::ui::entryExplanation(block));return false;}
    if(!workflowState.requestMode(mode,readiness))return false;
    swells->stop();if(mode==workflow::Mode::finalise)swells->initializeExport();if(previous!=mode)persistentStateChanged();return true;
}
void PreverbAudioProcessor::loadPreviewIR(const juce::File& file)
{
    if(file==juce::File{})return;
    if(swells->savedSession().primary.isEmpty())setSourceBars(1,2);
    swells->loadIR(file);
}
void PreverbAudioProcessor::setSourceBars(int64_t start,int64_t end)
{
    int64_t a,b;std::string error;const auto time=getSongTime();
    if(!resolveSourceBars(time,captureWorker.snapshot().manifest.sampleRate,start,end,a,b,error))
    {swells->diagnostic(error);return;}
    auto state=swells->savedSession();
    if(state.primary.isEmpty()) {if(!swells->addTarget(a))return;state=swells->savedSession();}
    swells->edit(state.primary,[&](auto& w,auto& text)
    {if(!m3::moveArrival(w,a,text))return false;return m3::setSource(w,a,b,text);});
}
void PreverbAudioProcessor::setAuditionLevelDb(float db)
{const auto state=swells->savedSession();swells->setPreferences(db,state.sensitivity,state.separationSeconds,state.autoRegenerate);}
PreviewSnapshot PreverbAudioProcessor::getPreviewSnapshot() const
{
    const auto snapshot=swells->snapshot();PreviewSnapshot result;
    result.status=snapshot.message;result.playing=snapshot.playing;result.auditionLevelDb=snapshot.session.auditionDb;result.auditionAttenuationDb=snapshot.attenuationDb;
    const auto w=m3::find(snapshot.session,snapshot.session.primary);if(!w)return result;
    result.irName=w->irName;result.irRate=w->ir.rate;result.irChannels=w->ir.channels;result.progress=w->progress;result.status=w->status;
    switch(w->phase)
    {
        case m3::Phase::empty:result.phase=PreviewPhase::empty;break;
        case m3::Phase::ready:result.phase=snapshot.auditionReady?PreviewPhase::ready:PreviewPhase::rendering;break;
        case m3::Phase::checking:result.phase=w->status=="Importing IR"?PreviewPhase::loadingIR:PreviewPhase::rendering;break;
        case m3::Phase::queued:case m3::Phase::rendering:case m3::Phase::restoring:result.phase=PreviewPhase::rendering;break;
        case m3::Phase::error:result.phase=PreviewPhase::error;break;
        case m3::Phase::stale:result.phase=w->status.startsWith("IR:")?PreviewPhase::canGenerate:PreviewPhase::stale;break;
    }
    const auto& t=snapshot.session.timing;
    if(t.valid && t.rate>0 && t.bpm>0 && t.numerator>0 && t.denominator>0)
    {
        const auto bar=t.rate*60/t.bpm*t.numerator*4/t.denominator,origin=t.sample-t.ppq*t.rate*60/t.bpm;
        result.startBar=static_cast<int64_t>(std::llround((w->sourceStart-origin)/bar))+1;
        result.endBar=static_cast<int64_t>(std::llround((w->sourceEnd-origin)/bar))+1;
    }
    if(snapshot.raw)
    {
        auto wave=std::make_shared<PreviewWaveform>();wave->start=snapshot.raw->start;wave->end=snapshot.raw->end;
        wave->sourceStart=w->sourceStart;wave->sourceEnd=w->sourceEnd;wave->frames=static_cast<size_t>(w->render.frames);wave->rate=w->render.rate;
        wave->peak=snapshot.raw->peak;wave->peaks=snapshot.raw->peaks;result.waveform=std::move(wave);
    }
    return result;
}

size_t PreverbAudioProcessor::getCapturedRegionCount() const noexcept
{
    return captureWorker.snapshot().manifest.ranges.size();
}

uint64_t PreverbAudioProcessor::getDroppedCaptureBlockCount() const noexcept
{
    return droppedCaptureBlockCount.load(std::memory_order_relaxed);
}

uint64_t PreverbAudioProcessor::getTimelineDiscontinuityCount() const noexcept
{
    return captureWorker.discontinuities();
}

void PreverbAudioProcessor::setTestToneEnabled(bool enabled) noexcept
{
    if(testToneEnabled.exchange(enabled,std::memory_order_relaxed)!=enabled)persistentStateChanged();
}

bool PreverbAudioProcessor::isTestToneEnabled() const noexcept
{
    return testToneEnabled.load(std::memory_order_relaxed);
}

void PreverbAudioProcessor::setToneStartBar(int bar) noexcept
{
    const auto boundedBar = std::clamp(bar, 1, 511);
    const auto value=std::min(boundedBar,toneEndBar.load(std::memory_order_relaxed)-1);
    if(toneStartBar.exchange(value,std::memory_order_relaxed)!=value)persistentStateChanged();
}

int PreverbAudioProcessor::getToneStartBar() const noexcept
{
    return toneStartBar.load(std::memory_order_relaxed);
}

void PreverbAudioProcessor::setToneEndBar(int bar) noexcept
{
    const auto boundedBar = std::clamp(bar, 2, 512);
    const auto value=std::max(boundedBar,toneStartBar.load(std::memory_order_relaxed)+1);
    if(toneEndBar.exchange(value,std::memory_order_relaxed)!=value)persistentStateChanged();
}

int PreverbAudioProcessor::getToneEndBar() const noexcept
{
    return toneEndBar.load(std::memory_order_relaxed);
}

void PreverbAudioProcessor::updateHostPositionDiagnostics(int currentBlockSize) noexcept
{
    struct UpdateGuard
    {
        std::atomic<uint64_t>& sequence;
        explicit UpdateGuard(std::atomic<uint64_t>& value) : sequence(value) { sequence.fetch_add(1, std::memory_order_acq_rel); }
        ~UpdateGuard() { sequence.fetch_add(1, std::memory_order_release); }
    };

    auto* hostPlayHead = getPlayHead();
    const auto position = hostPlayHead != nullptr ? hostPlayHead->getPosition() : juce::Optional<juce::AudioPlayHead::PositionInfo>{};
    // Host queries may be relatively slow. Keep the prior complete snapshot
    // readable until the tiny atomic publication below, rather than making
    // worker/UI readers report unavailable timing throughout getPosition().
    UpdateGuard update(diagnosticsSequence);
    blockSize.store(currentBlockSize, std::memory_order_relaxed);
    positionAvailable.store(position.hasValue(), std::memory_order_relaxed);

    if (! position.hasValue())
    {
        samplePositionAvailable.store(false, std::memory_order_relaxed);
        ppqPositionAvailable.store(false, std::memory_order_relaxed);
        tempoAvailable.store(false, std::memory_order_relaxed);
        meterAvailable.store(false, std::memory_order_relaxed);
        isPlaying.store(false, std::memory_order_relaxed);
        isRecording.store(false, std::memory_order_relaxed);
        isLooping.store(false,std::memory_order_relaxed);
        return;
    }

    const auto& positionInfo = *position;
    const auto timeInSamples = positionInfo.getTimeInSamples();
    samplePositionAvailable.store(timeInSamples.hasValue(), std::memory_order_relaxed);
    if (timeInSamples.hasValue())
        samplePosition.store(*timeInSamples, std::memory_order_relaxed);

    const auto ppq = positionInfo.getPpqPosition();
    ppqPositionAvailable.store(ppq.hasValue(), std::memory_order_relaxed);
    if (ppq.hasValue())
        ppqPosition.store(*ppq, std::memory_order_relaxed);

    const auto tempo = positionInfo.getBpm();
    tempoAvailable.store(tempo.hasValue(), std::memory_order_relaxed);
    if (tempo.hasValue())
        bpm.store(*tempo, std::memory_order_relaxed);

    const auto timeSignature = positionInfo.getTimeSignature();
    meterAvailable.store(timeSignature.hasValue(), std::memory_order_relaxed);
    if (timeSignature.hasValue())
    {
        numerator.store(timeSignature->numerator, std::memory_order_relaxed);
        denominator.store(timeSignature->denominator, std::memory_order_relaxed);
    }

    isPlaying.store(positionInfo.getIsPlaying(), std::memory_order_relaxed);
    isRecording.store(positionInfo.getIsRecording(), std::memory_order_relaxed);
    int64_t first=0,last=0;bool validLoop=false;
    if(const auto points=positionInfo.getLoopPoints();points && positionInfo.getIsLooping() && timeInSamples && ppq && tempo && timeSignature){
        const SongTime t{true,*timeInSamples,*ppq,sampleRate.load(),*tempo,timeSignature->numerator,timeSignature->denominator};
        validLoop=resolvePpqSample(t,points->ppqStart,first) && resolvePpqSample(t,points->ppqEnd,last) && last>first;}
    loopStart.store(first,std::memory_order_relaxed);loopEnd.store(last,std::memory_order_relaxed);isLooping.store(validLoop,std::memory_order_relaxed);
}

juce::AudioProcessorEditor* PreverbAudioProcessor::createEditor()
{
    auto editor=std::make_unique<PreverbAudioProcessorEditor>(*this);
    editor->enableResponsivePresentation(std::make_shared<m3::ui::FilePresentationStorage>(m3::ui::FilePresentationStorage::userFile()));
    return editor.release();
}

bool PreverbAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String PreverbAudioProcessor::getName() const
{
    return "ReverbArise";
}

bool PreverbAudioProcessor::acceptsMidi() const
{
    return false;
}

bool PreverbAudioProcessor::producesMidi() const
{
    return false;
}

bool PreverbAudioProcessor::isMidiEffect() const
{
    return false;
}

double PreverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PreverbAudioProcessor::getNumPrograms()
{
    return 1;
}

int PreverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PreverbAudioProcessor::setCurrentProgram(int)
{
}

const juce::String PreverbAudioProcessor::getProgramName(int)
{
    return {};
}

void PreverbAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void PreverbAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData)
{
    juce::XmlElement state("PreverbState");
    state.setAttribute("stateVersion", 8);
    state.setAttribute("workflowMode",workflow::modeName(workflowState.snapshot().mode));
    state.setAttribute("toneEnabled", isTestToneEnabled());
    state.setAttribute("toneStartBar", getToneStartBar());
    state.setAttribute("toneEndBar", getToneEndBar());
    const auto capture = captureWorker.snapshot();
    state.setAttribute("projectId", juce::String(capture.manifest.projectId));
    state.setAttribute("instanceId", juce::String(capture.manifest.instanceId));
    state.setAttribute("manifestReference", capture.reference);
    state.setAttribute("manifestRevision", juce::String(static_cast<juce::int64>(capture.manifest.revision)));
    juce::String json,error;
    auto session=swells->savedSession();session.mappingEnabled=isMappingEnabled();
    if(!m3::encode(session,json,error)) {swells->diagnostic("Save failed: "+error);return;}
    state.createNewChildElement("SwellSession")->addTextElement(json);
    copyXmlToBinary(state, destinationData);
}

void PreverbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    const juce::ScopedLock stateGuard(storageStateLock);stateDecodeRejected=true;
    if(sizeInBytes<=0 || sizeInBytes>static_cast<int>(m3::maximumStateBytes*6+1024*1024))
    {swells->diagnostic("Invalid or oversized plugin state");return;}
    const auto state = getXmlFromBinary(data, sizeInBytes);
    if (state == nullptr || ! state->hasTagName("PreverbState")
        || state->getIntAttribute("stateVersion",0)<1 || state->getIntAttribute("stateVersion",0)>8)
    {swells->diagnostic("Unsupported or malformed plugin state");return;}
    const auto version=state->getIntAttribute("stateVersion");m3::Session session;juce::String error;
    // Envelopes predating creative metadata also retain the legacy Stop journey.
    if(version<3)session.autoGenerateOnStop=true;
    auto savedMode=workflow::Mode::capture;
    if(version>=8)
    {
        const auto name=state->getStringAttribute("workflowMode");
        if(name=="Live")savedMode=workflow::Mode::live;
        else if(name=="Finalise")savedMode=workflow::Mode::finalise;
        else if(name!="Capture"){swells->diagnostic("State rejected: invalid workflow mode");return;}
    }
    if(version>=3)
    {
        const auto child=state->getChildByName("SwellSession");
        if(!child || !m3::decode(child->getAllSubText(),session,error)) {swells->diagnostic("State rejected: "+error);return;}
        if(version==3)
        {
            session.mappingEnabled=false;session.automaticProcessing=false;
            for(auto& swell:session.swells)swell.provenance=m3::Provenance::manualEdited;
        }
    }
    if(version>=2)
    {
        const auto validID=[](const juce::String& id){return id.length()==36 && juce::Uuid(id).toDashedString()==id && !juce::Uuid(id).isNull();};
        const auto reference=state->getStringAttribute("manifestReference"),revision=state->getStringAttribute("manifestRevision");
        if(!validID(state->getStringAttribute("projectId")) || !validID(state->getStringAttribute("instanceId"))
            || (!reference.isEmpty() && (!reference.startsWith("manifests/") || !reference.endsWith(".json") || !isSafeCaptureAssetPath(reference.toStdString())))
            || revision.getLargeIntValue()<0 || juce::String(revision.getLargeIntValue())!=revision)
        {swells->diagnostic("State rejected: invalid capture reference");return;}
    }

    recoveryIntent.reset();
    awaitingRestoredPosition.store(true,std::memory_order_release);
    auto startBar = std::clamp(state->getIntAttribute("toneStartBar", 8), 1, 511);
    auto endBar = std::clamp(state->getIntAttribute("toneEndBar", 10), 2, 512);
    if (endBar <= startBar)
        endBar = std::min(startBar + 1, 512);

    toneStartBar.store(startBar, std::memory_order_relaxed);
    toneEndBar.store(endBar, std::memory_order_relaxed);
    testToneEnabled.store(state->getBoolAttribute("toneEnabled", false), std::memory_order_relaxed);
    if (version >= 2)
    {
        const auto revision = state->getStringAttribute("manifestRevision").getLargeIntValue();
        captureWorker.requestRestore(state->getStringAttribute("projectId").toStdString(),
                                     state->getStringAttribute("instanceId").toStdString(),
                                     state->getStringAttribute("manifestReference"),
                                     static_cast<uint64_t>(std::max(juce::int64{0}, revision)),savedMode);
    }
    else
    {
        captureWorker.requestRestore(juce::Uuid().toDashedString().toStdString(),
                                     juce::Uuid().toDashedString().toStdString(), {}, 0,savedMode);
    }
    swells->restore(std::move(session),version>=2?state->getStringAttribute("manifestReference"):juce::String{});
    stateDecodeRejected=false;
    cancelPendingUpdate(); // Loading acknowledged state is not a new user edit.
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PreverbAudioProcessor();
}

bool PreverbAudioProcessor::pinPortableAssets(m3::storage::portable::Input& output,juce::String& error,bool allowUnavailable) const
{
    const juce::ScopedLock stateGuard(storageStateLock);
    if(stateDecodeRejected.load()){error="Saved plugin metadata could not be decoded. The arrangement cannot be confirmed intact.";return false;}
    const auto generation=captureWorker.captureGeneration();const auto flow=workflowState.snapshot();const auto mapping=isMappingEnabled();
    if(captureWorker.isLoading()){error="Capture state is loading. Retry after it settles.";return false;}
    CaptureSessionSnapshot capture;m3::Session session;juce::File root;uint64_t epoch=0;
    if(!swells->acceptedStorageSnapshot(capture,session,root,epoch,error,allowUnavailable))return false;
    if(captureWorker.isLoading() || captureWorker.captureGeneration()!=generation || swells->assignmentEpoch()!=epoch
        || workflowState.snapshot().mode!=flow.mode || isMappingEnabled()!=mapping)
    {error="Accepted session changed while its backup metadata was pinned. Retry.";return false;}
    session.mappingEnabled=mapping;juce::String creative,manifest;
    if(!m3::encode(session,creative,error) || (!capture.unavailable && !serializeCaptureManifest(capture.manifest,manifest,error)))return false;
    juce::XmlElement state("PreverbState");state.setAttribute("stateVersion",8);state.setAttribute("workflowMode",workflow::modeName(flow.mode));
    state.setAttribute("toneEnabled",isTestToneEnabled());state.setAttribute("toneStartBar",getToneStartBar());state.setAttribute("toneEndBar",getToneEndBar());
    state.setAttribute("projectId",juce::String(capture.manifest.projectId));state.setAttribute("instanceId",juce::String(capture.manifest.instanceId));
    state.setAttribute("manifestReference",capture.reference);state.setAttribute("manifestRevision",juce::String(static_cast<juce::int64>(capture.manifest.revision)));
    state.createNewChildElement("SwellSession")->addTextElement(creative);
    m3::storage::portable::Input accepted{root,state.toString(),manifest};accepted.producer=juce::String(m3::build::report());accepted.sourceUnavailable=capture.unavailable;accepted.controlEpoch=epoch;output=std::move(accepted);error.clear();return true;
}
bool PreverbAudioProcessor::reloadRecoveredAssets(const m3::storage::portable::Input& expected,juce::String& error)
{
    const juce::ScopedLock guard(storageStateLock);m3::storage::portable::Input current;
    if(workflowState.snapshot().locked() || isPlaying.load() || !pinPortableAssets(current,error,true))
    {if(error.isEmpty())error="Stop the DAW and wait for pending work before reloading recovered assets.";return false;}
    const auto a=juce::XmlDocument::parse(expected.processorXml),b=juce::XmlDocument::parse(current.processorXml);
    if(!a || !b || expected.instanceRoot!=current.instanceRoot || expected.controlEpoch!=current.controlEpoch
        || a->getStringAttribute("manifestReference")!=b->getStringAttribute("manifestReference")
        || a->getStringAttribute("manifestRevision")!=b->getStringAttribute("manifestRevision"))
    {error="The accepted session changed; recovered copies were retained without replacing current state.";return false;}
    if(!swells->reloadStorageAssets(current.controlEpoch,b->getStringAttribute("manifestReference")))
    {error="The session changed before reload. Copied assets remain available.";return false;}
    captureWorker.requestRestore(b->getStringAttribute("projectId").toStdString(),b->getStringAttribute("instanceId").toStdString(),
        b->getStringAttribute("manifestReference"),static_cast<uint64_t>(b->getStringAttribute("manifestRevision").getLargeIntValue()),workflowState.snapshot().mode,true);
    error.clear();return true;
}

bool PreverbAudioProcessor::beginRelearning(const m3::storage::portable::Input& expected,juce::String& error)
{
    const juce::ScopedLock guard(storageStateLock);m3::storage::portable::Input current;
    if(isPlaying.load() || workflowState.snapshot().locked() || !pinPortableAssets(current,error,true))
    {if(error.isEmpty())error="Stop DAW playback before arming re-learning.";return false;}
    if(expected.instanceRoot!=current.instanceRoot || expected.controlEpoch!=current.controlEpoch || expected.processorXml!=current.processorXml)
    {error="The session changed; review recovery again.";return false;}
    RecoveryIntent intent;intent.original=current;const auto capture=captureWorker.snapshot();const auto session=swells->savedSession();
    const auto timing=getSongTime();
    if(!timing.valid || (session.timing.valid && !sameMusicalMapping(session.timing,timing)) || (capture.manifest.sampleRate>0 && capture.manifest.sampleRate!=timing.rate))
    {error="Re-learning requires the saved sample rate, tempo and musical mapping. Restore the original DAW timing first.";return false;}
    for(const auto& range:capture.manifest.ranges)intent.required.emplace_back(range.startSample,range.startSample+static_cast<int64_t>(range.frameCount));
    for(const auto& w:session.swells)intent.required.emplace_back(w.sourceStart,m3::renderInputEnd(w));
    if(intent.required.empty()){error="Saved source bounds are unavailable. Locate or restore assets before re-learning this empty arrangement.";return false;}
    swells->holdRecoveryGeneration(true);
    if(!captureWorker.armRecovery(intent.required)){swells->holdRecoveryGeneration(false);error="Re-learning could not be armed; previous source retained.";return false;}
    recoveryIntent=std::move(intent);persistentStateChanged();error.clear();return true;
}
void PreverbAudioProcessor::cancelRelearning()
{const juce::ScopedLock guard(storageStateLock);if(!recoveryIntent)return;captureWorker.cancelWorkflowPass();swells->holdRecoveryGeneration(false);recoveryIntent.reset();}
bool PreverbAudioProcessor::regenerateRecovered(bool frozenConsent,juce::String& error,bool validateOnly)
{
    m3::storage::portable::Input current;CaptureSessionSnapshot capture;m3::Session session;std::vector<std::pair<int64_t,int64_t>> required;
    {const juce::ScopedLock guard(storageStateLock);
        if(!recoveryIntent || isPlaying.load() || workflowState.snapshot().locked() || captureWorker.recoveryArmed() || !pinPortableAssets(current,error))
        {if(error.isEmpty())error="Complete fresh source and stop the DAW before recovery generation.";return false;}
        const auto old=juce::XmlDocument::parse(recoveryIntent->original.processorXml),now=juce::XmlDocument::parse(current.processorXml);
        if(!old || !now || recoveryIntent->original.instanceRoot!=current.instanceRoot || recoveryIntent->original.controlEpoch!=current.controlEpoch
            || old->getStringAttribute("manifestReference")==now->getStringAttribute("manifestReference"))
        {error="A complete fresh source pass has not committed for this recovery.";return false;}
        required=recoveryIntent->required;capture=captureWorker.snapshot();session=swells->savedSession();}
    for(const auto [start,end]:required){auto covered=start;for(const auto& range:capture.manifest.ranges){const auto tail=range.startSample+static_cast<int64_t>(range.frameCount);if(tail<=covered)continue;if(range.startSample>covered)break;covered=tail;if(covered>=end)break;}
        if(covered<end){error="Fresh source coverage is incomplete; previous assets retained.";return false;}}
    m3::SwellAssetStore store(current.instanceRoot);std::vector<juce::String> targets;std::set<juce::String> verified;
    for(const auto& w:session.swells){if(!w.ir.present()){error="Restore or explicitly assign the missing IR for "+w.name;return false;}
        if(verified.insert(w.ir.path).second){ImpulseResponse ir;if(!store.readIR(w.ir,previewMemoryBudget,ir,error))return false;}
        juce::String why;if(w.frozen && store.verify(w.render,why))continue;
        if(w.frozen && !frozenConsent){error="Explicit consent is required to replace missing frozen results. Surviving frozen audio will be retained.";return false;}targets.push_back(w.id);}
    const juce::ScopedLock guard(storageStateLock);m3::storage::portable::Input latest;
    if(!pinPortableAssets(latest,error) || latest.instanceRoot!=current.instanceRoot || latest.controlEpoch!=current.controlEpoch || latest.processorXml!=current.processorXml)
    {error="The arrangement changed during recovery checks; retry.";return false;}
    if(validateOnly){error.clear();return true;}
    if(!swells->generateRecovered(current.controlEpoch,capture.reference,targets,frozenConsent,error))return false;
    error.clear();return true;
}
