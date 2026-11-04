#include "LearnGenerationSupport.h"
#include "DenseLearnGenerationCases.h"
int main(){return runTest([]
{
    juce::ScopedJuceInitialiser_GUI gui;
    GenerationFixture fixture;auto& engine=fixture.engine();auto& processor=*fixture.processor;
    REQUIRE(!engine.savedSession().autoGenerateOnStop);fixture.record(true);
    auto settled=fixture.ready();REQUIRE(processor.getWorkflowSnapshot().mode==workflow::Mode::capture);
    REQUIRE(settled.session.swells.empty() && settled.renderCount==0 && !settled.learnGeneration.displayed->identity.provisional);
    REQUIRE(settled.learnGeneration.displayed->positions.size()>=8);
    // Cancel an analysis with a mode switch, then return with exactly the same
    // source/settings identity: an abandoned attempt must remain retryable.
    engine.holdLearnAnalysisForTesting(true);
    engine.setPreferences(0,.65f,.08,false,processor.getWorkflowSnapshot().pass);
    await([&]{return engine.learnAnalysisWaitingForTesting();});
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));
    await([&]{return !engine.learnAnalysisWaitingForTesting();});
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));
    engine.holdLearnAnalysisForTesting(false);fixture.ready();
    engine.setPreferences(0,.5f,.08,false,processor.getWorkflowSnapshot().pass);settled=fixture.ready();
    const auto initialSerial=settled.learnGeneration.displayed->serial;
    const auto countFiles=[&]{return engine.assetDirectory().findChildFiles(juce::File::findFiles,true).size();};
    const auto files=countFiles();engine.holdLearnAnalysisForTesting(true);
    for(float sensitivity:{.2f,.8f,.6f})engine.setPreferences(0,sensitivity,.08,false,processor.getWorkflowSnapshot().pass);
    await([&]{return engine.learnAnalysisWaitingForTesting();});
    auto updating=engine.snapshot();REQUIRE(updating.learnGeneration.updating && updating.learnGeneration.displayed->serial==initialSerial);
    REQUIRE(updating.session.swells.empty() && updating.renderCount==0 && !updating.learnGeneration.canGenerate);
    juce::String before,after,error;REQUIRE(encode(engine.savedSession(),before,error));
    engine.holdLearnAnalysisForTesting(false);settled=fixture.ready();
    REQUIRE(settled.learnGeneration.displayed->identity.sensitivity==.6f && settled.learnGeneration.displayed->serial>initialSerial);
    REQUIRE(encode(engine.savedSession(),after,error) && before==after && countFiles()==files && settled.renderCount==0);
    const auto beforeDefault=settled.learnGeneration.displayed->serial;
    REQUIRE(engine.assignDefaultBundledIR("monumental"));await([&]{return !engine.snapshot().defaultIRPending;});
    settled=fixture.ready();REQUIRE(settled.learnGeneration.displayed->serial==beforeDefault && settled.renderCount==0);
    // Mode/capture authority wins over a held prepared generation.
    engine.holdLearnGenerationForTesting(true);REQUIRE(fixture.generate());await([&]{return engine.learnGenerationWaitingForTesting();});
    {
        juce::MemoryBlock preparation;processor.getStateInformation(preparation);
        auto reopened=std::make_unique<PreverbAudioProcessor>(fixture.root.file);reopened->prepareToPlay(48000,4096);reopened->setPlayHead(&fixture.host);
        reopened->setStateInformation(preparation.getData(),static_cast<int>(preparation.getSize()));REQUIRE(reopened->flushCaptureForTesting());
        juce::AudioBuffer<float> stopped(2,16);stopped.clear();juce::MidiBuffer midi;
        {AudioAllocationGuard guard;reopened->processBlock(stopped,midi);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        await([&]{return reopened->swellEngine().snapshot().learnGeneration.canGenerate;});
        REQUIRE(reopened->getWorkflowSnapshot().mode==workflow::Mode::capture && reopened->swellEngine().savedSession().swells.empty());
        REQUIRE(!reopened->swellEngine().snapshot().initialPerformHandoff);
    }
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));engine.holdLearnGenerationForTesting(false);
    await([&]{return !engine.snapshot().learnGeneration.generating;});
    REQUIRE(engine.savedSession().swells.empty() && !engine.snapshot().initialPerformHandoff);
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));fixture.ready();
    // Requesting a different IR cancels before its asynchronous import can finish.
    engine.holdLearnGenerationForTesting(true);REQUIRE(fixture.generate());await([&]{return engine.learnGenerationWaitingForTesting();});
    REQUIRE(engine.assignDefaultBundledIR("preverb-hall"));engine.holdLearnGenerationForTesting(false);
    await([&]{return !engine.snapshot().learnGeneration.generating && !engine.snapshot().defaultIRPending;});
    REQUIRE(engine.savedSession().swells.empty());fixture.ready();
    engine.holdLearnGenerationForTesting(true);REQUIRE(fixture.generate());await([&]{return engine.learnGenerationWaitingForTesting();});
    engine.setPreferences(0,.5f,.08,false,processor.getWorkflowSnapshot().pass);engine.holdLearnGenerationForTesting(false);
    await([&]{return !engine.snapshot().learnGeneration.generating;});fixture.ready();REQUIRE(engine.savedSession().swells.empty());
    engine.holdLearnRendersForTesting(true);REQUIRE(fixture.generate());
    await([&]{return processor.getWorkflowSnapshot().mode==workflow::Mode::live;});
    const auto generated=engine.snapshot();REQUIRE(generated.workflowState.mode==workflow::Mode::live && generated.session.swells.size()>=8 && generated.renderCount==0 && generated.initialPerformHandoff);
    REQUIRE(!processor.getWorkflowReadiness().readySwell && processor.getWorkflowReadiness().usefulSource);
    for(const auto& swell:generated.session.swells)REQUIRE(swell.provenance==Provenance::automatic && swell.ir==generated.session.defaultIR && swell.generationSource.isNotEmpty());
    // Identical repeated Generate cannot fabricate success or consume a new handoff.
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));fixture.ready();
    engine.holdLearnAnalysisForTesting(true);engine.setPreferences(0,.8f,.08,false,processor.getWorkflowSnapshot().pass);
    await([&]{return engine.learnAnalysisWaitingForTesting();});
    const auto beforeRefinement=engine.savedSession();
    for(const auto& w:generated.session.swells)REQUIRE(sameSavedSwell(*find(beforeRefinement,w.id),w));
    engine.setPreferences(0,.5f,.08,false,processor.getWorkflowSnapshot().pass);engine.holdLearnAnalysisForTesting(false);fixture.ready();
    const auto afterRefinement=engine.savedSession();
    for(const auto& w:generated.session.swells)REQUIRE(sameSavedSwell(*find(afterRefinement,w.id),w));
    REQUIRE(engine.snapshot().renderCount==0);REQUIRE(fixture.generate());
    await([&]{const auto view=engine.snapshot().learnGeneration;return !view.generating && view.notice.contains("No changes");});
    REQUIRE(processor.getWorkflowSnapshot().mode==workflow::Mode::capture && engine.savedSession().swells.size()==generated.session.swells.size());
    engine.holdLearnRendersForTesting(false);

    GenerationFixture automatic;auto& autoEngine=automatic.engine();autoEngine.holdLearnRendersForTesting(true);
    REQUIRE(autoEngine.setAutoGenerateOnStop(true,autoEngine.assignmentEpoch(),automatic.processor->getWorkflowSnapshot().pass));
    automatic.record();await([&]{return automatic.processor->getWorkflowSnapshot().mode==workflow::Mode::live;});
    const auto autoResult=autoEngine.snapshot();REQUIRE(autoResult.renderCount==0 && autoResult.initialPerformHandoff);
    REQUIRE(autoResult.session.swells.size()==generated.session.swells.size());
    for(size_t i=0;i<autoResult.session.swells.size();++i)REQUIRE(autoResult.session.swells[i].arrival==generated.session.swells[i].arrival);
    REQUIRE(automatic.processor->requestWorkflowMode(workflow::Mode::capture));automatic.ready();
    REQUIRE(automatic.processor->setRecaptureExisting(true));automatic.record();
    await([&]{const auto view=autoEngine.snapshot().learnGeneration;return !view.generating && view.notice.contains("No changes");});
    REQUIRE(automatic.processor->getWorkflowSnapshot().mode==workflow::Mode::capture);
    const auto autoBeforeFailure=autoEngine.savedSession();
    REQUIRE(autoEngine.assetDirectory().getChildFile(autoBeforeFailure.defaultIR.path).replaceWithText("Injected automatic-path IR failure"));
    REQUIRE(automatic.processor->setRecaptureExisting(true));automatic.record();
    await([&]{const auto view=autoEngine.snapshot().learnGeneration;return !view.generating && view.failed;});
    const auto autoAfterFailure=autoEngine.savedSession();
    REQUIRE(automatic.processor->getWorkflowSnapshot().mode==workflow::Mode::capture && autoAfterFailure.swells.size()==autoBeforeFailure.swells.size());
    for(const auto& w:autoBeforeFailure.swells)REQUIRE(sameSavedSwell(*find(autoAfterFailure,w.id),w));

    GenerationFixture cancelled;auto& cancelEngine=cancelled.engine();cancelEngine.holdLearnAnalysisForTesting(true);
    REQUIRE(cancelEngine.setAutoGenerateOnStop(true,cancelEngine.assignmentEpoch(),cancelled.processor->getWorkflowSnapshot().pass));
    cancelled.record();await([&]{return cancelEngine.learnAnalysisWaitingForTesting();});
    REQUIRE(cancelEngine.setAutoGenerateOnStop(false,cancelEngine.assignmentEpoch(),cancelled.processor->getWorkflowSnapshot().pass));
    cancelEngine.holdLearnAnalysisForTesting(false);cancelled.ready();
    REQUIRE(cancelled.processor->getWorkflowSnapshot().mode==workflow::Mode::capture && cancelEngine.savedSession().swells.empty());
    REQUIRE(cancelEngine.setAutoGenerateOnStop(true,cancelEngine.assignmentEpoch(),cancelled.processor->getWorkflowSnapshot().pass));
    juce::Thread::sleep(60);REQUIRE(cancelEngine.savedSession().swells.empty());
    GenerationFixture settingsCancelled;auto& settingsEngine=settingsCancelled.engine();settingsEngine.holdLearnAnalysisForTesting(true);
    REQUIRE(settingsEngine.setAutoGenerateOnStop(true,settingsEngine.assignmentEpoch(),settingsCancelled.processor->getWorkflowSnapshot().pass));
    settingsCancelled.record();await([&]{return settingsEngine.learnAnalysisWaitingForTesting();});
    settingsEngine.setPreferences(0,.3f,.08,false,settingsCancelled.processor->getWorkflowSnapshot().pass);
    settingsEngine.holdLearnAnalysisForTesting(false);settingsCancelled.ready();
    REQUIRE(settingsEngine.savedSession().swells.empty() && settingsCancelled.processor->getWorkflowSnapshot().mode==workflow::Mode::capture);

    GenerationFixture restarted;auto& restartEngine=restarted.engine();restarted.record();restarted.ready();
    const auto oldSource=restarted.processor->getCaptureSnapshot().reference;
    restartEngine.holdLearnGenerationForTesting(true);REQUIRE(restarted.generate());await([&]{return restartEngine.learnGenerationWaitingForTesting();});
    REQUIRE(restarted.processor->setRecaptureExisting(true));restarted.tick(4096,true,.5f);
    restartEngine.holdLearnGenerationForTesting(false);restarted.tick(8192,false);REQUIRE(restarted.processor->flushCaptureForTesting());
    await([&]{return !restartEngine.snapshot().learnGeneration.generating;});
    const auto revised=restarted.ready();REQUIRE(revised.session.swells.empty() && restarted.processor->getCaptureSnapshot().reference!=oldSource);
    REQUIRE(revised.learnGeneration.displayed->identity.sourceReference==restarted.processor->getCaptureSnapshot().reference);

    GenerationFixture protectedWork;auto& protectedEngine=protectedWork.engine();protectedWork.record();protectedWork.ready();
    REQUIRE(protectedWork.processor->requestWorkflowMode(workflow::Mode::live));
    REQUIRE(protectedEngine.addTarget(32000));const auto manualId=protectedEngine.savedSession().primary;
    await([&]{return find(protectedEngine.savedSession(),manualId)->phase==Phase::ready;});
    REQUIRE(protectedEngine.edit(manualId,[](auto& w,auto&){w.shape.gainDb=-3;return true;}));
    REQUIRE(protectedEngine.addTarget(96000));const auto frozenId=protectedEngine.savedSession().primary;
    await([&]{return find(protectedEngine.savedSession(),frozenId)->phase==Phase::ready;});
    REQUIRE(protectedEngine.setFrozen(frozenId,true));
    const auto protectedSession=protectedEngine.savedSession();
    const auto manual=*find(protectedSession,manualId),frozen=*find(protectedSession,frozenId);
    REQUIRE(protectedWork.processor->requestWorkflowMode(workflow::Mode::capture));protectedWork.ready();
    protectedEngine.holdLearnRendersForTesting(true);REQUIRE(protectedWork.generate());
    await([&]{return protectedWork.processor->getWorkflowSnapshot().mode==workflow::Mode::live;});
    juce::Thread::sleep(60);const auto preserved=protectedEngine.savedSession();
    REQUIRE(sameSavedSwell(*find(preserved,manualId),manual) && sameSavedSwell(*find(preserved,frozenId),frozen));
    REQUIRE(preserved.swells.size()>protectedSession.swells.size());
    REQUIRE(protectedWork.processor->requestWorkflowMode(workflow::Mode::capture));protectedWork.ready();
    REQUIRE(protectedEngine.assignDefaultBundledIR("monumental"));await([&]{return !protectedEngine.snapshot().defaultIRPending;});
    REQUIRE(protectedEngine.setAutoGenerateOnStop(true,protectedEngine.assignmentEpoch(),protectedWork.processor->getWorkflowSnapshot().pass));
    REQUIRE(protectedWork.processor->setRecaptureExisting(true));protectedWork.record();
    await([&]{return protectedWork.processor->getWorkflowSnapshot().mode==workflow::Mode::live;});
    juce::Thread::sleep(60);const auto automaticPreserved=protectedEngine.savedSession();
    REQUIRE(sameSavedSwell(*find(automaticPreserved,manualId),manual) && sameSavedSwell(*find(automaticPreserved,frozenId),frozen));
    // Save/reopen rebuilds read-only candidates and committed render recovery,
    // never a deferred source-context generation command or initial handoff.
    REQUIRE(protectedWork.processor->requestWorkflowMode(workflow::Mode::capture));protectedWork.ready();
    REQUIRE(protectedEngine.setAutoGenerateOnStop(false,protectedEngine.assignmentEpoch(),protectedWork.processor->getWorkflowSnapshot().pass));
    juce::MemoryBlock saved;protectedWork.processor->getStateInformation(saved);
    auto restored=std::make_unique<PreverbAudioProcessor>(protectedWork.root.file);restored->prepareToPlay(48000,4096);restored->setPlayHead(&protectedWork.host);
    restored->swellEngine().holdLearnRendersForTesting(true);
    restored->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));REQUIRE(restored->flushCaptureForTesting());
    protectedWork.host.position.setIsPlaying(false);juce::AudioBuffer<float> stopped(2,4096);stopped.clear();juce::MidiBuffer midi;
    restored->processBlock(stopped,midi);await([&]{return restored->swellEngine().snapshot().learnGeneration.canGenerate;});
    REQUIRE(!restored->swellEngine().savedSession().autoGenerateOnStop && !restored->swellEngine().snapshot().initialPerformHandoff);
    REQUIRE(restored->swellEngine().savedSession().swells.size()==preserved.swells.size());
    REQUIRE(restored->swellEngine().savedSession().primary==preserved.primary);
    restored.reset();

    // Failed asset validation preserves every committed definition/render ref.
    const auto beforeFailure=protectedEngine.savedSession();
    const auto irFile=protectedEngine.assetDirectory().getChildFile(beforeFailure.defaultIR.path);
    juce::MemoryBlock irBytes;REQUIRE(irFile.loadFileAsData(irBytes));
    REQUIRE(irFile.replaceWithText("Injected invalid managed IR"));REQUIRE(protectedWork.generate());
    await([&]{const auto view=protectedEngine.snapshot().learnGeneration;return !view.generating && view.failed;});
    const auto failed=protectedEngine.savedSession();REQUIRE(failed.swells.size()==beforeFailure.swells.size());
    for(const auto& w:beforeFailure.swells)REQUIRE(sameSavedSwell(*find(failed,w.id),w));
    REQUIRE(protectedWork.processor->getWorkflowSnapshot().mode==workflow::Mode::capture);
    REQUIRE(irFile.replaceWithData(irBytes.getData(),irBytes.getSize()));
    const auto captured=protectedWork.processor->getCaptureSnapshot();
    const auto range=std::find_if(captured.manifest.ranges.begin(),captured.manifest.ranges.end(),[](const auto& r){return r.kind==CaptureCoverageKind::audio;});
    REQUIRE(range!=captured.manifest.ranges.end());
    REQUIRE(protectedEngine.assetDirectory().getChildFile(juce::String(range->assetPath)).replaceWithText("Injected captured-source corruption"));
    REQUIRE(protectedWork.generate());await([&]{const auto view=protectedEngine.snapshot().learnGeneration;return !view.generating && view.failed;});
    const auto sourceFailed=protectedEngine.savedSession();
    for(const auto& w:beforeFailure.swells)REQUIRE(sameSavedSwell(*find(sourceFailed,w.id),w));
    REQUIRE(protectedWork.processor->getCaptureSnapshot().reference==captured.reference);

    GenerationFixture abandoned;auto& abandonEngine=abandoned.engine();abandoned.processor->holdCaptureValidationForTesting(true);
    abandoned.record(true,false);await([&]{return abandoned.processor->getWorkflowSnapshot().phase==workflow::Phase::validating;});
    abandoned.processor->cancelCapturePass();abandoned.processor->holdCaptureValidationForTesting(false);
    REQUIRE(abandoned.processor->flushCaptureForTesting());
    await([&]{const auto view=abandonEngine.snapshot().learnGeneration;return !view.canGenerate && (!view.displayed || !view.displayed->identity.provisional);});
    REQUIRE(abandoned.processor->getCaptureSnapshot().manifest.ranges.empty() && abandonEngine.savedSession().swells.empty());
    REQUIRE(abandoned.processor->getWorkflowSnapshot().mode==workflow::Mode::capture);
    GenerationFixture dryOnly;auto& dryEngine=dryOnly.engine();dryEngine.setLearningAutomatic(false);dryOnly.record();
    REQUIRE(dryOnly.processor->getWorkflowSnapshot().mode==workflow::Mode::capture && dryEngine.savedSession().swells.empty());
    REQUIRE(dryOnly.processor->getWorkflowReadiness().usefulSource && dryEngine.snapshot().renderCount==0);
    dryEngine.setLearningAutomatic(true);dryOnly.ready();REQUIRE(dryEngine.savedSession().swells.empty() && dryEngine.snapshot().renderCount==0);
    dryEngine.holdLearnRendersForTesting(true);REQUIRE(dryOnly.generate());
    await([&]{return dryOnly.processor->getWorkflowSnapshot().mode==workflow::Mode::live;});
    REQUIRE(!dryEngine.savedSession().swells.empty() && dryEngine.snapshot().renderCount==0);
    dryEngine.holdLearnRendersForTesting(false);
    await([&]{const auto s=dryEngine.savedSession();return std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==Phase::ready;});});
    denseLearnGenerationCases();
});}
