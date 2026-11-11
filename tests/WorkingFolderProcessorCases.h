#pragma once
#include "LoopCaptureFixture.h"
#include <Storage/WorkingFolder.h>
#include <future>

static void workingFolderLoopCases()
{
    TestDirectory destination;
    LoopCaptureFixture fixture;
    REQUIRE(destination.file.createDirectory().wasOk());
    auto& processor=*fixture.processor;
    const auto preference=processor.getLoopCapturePreference();
    REQUIRE(processor.setLoopCapturePreference({}));
    auto& position=fixture.host.position;
    position.setTimeInSamples(0);position.setPpqPosition(0.0);position.setBpm(120);
    position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    position.setIsPlaying(true);
    processor.setPlayHead(&fixture.host);
    juce::AudioBuffer<float> song(2,128);
    juce::MidiBuffer midi;
    for(int channel=0;channel<2;++channel)std::fill_n(song.getWritePointer(channel),128,.25f);
    processor.processBlock(song,midi);
    position.setIsPlaying(false);
    processor.processBlock(song,midi);
    REQUIRE(processor.flushCaptureForTesting() && processor.getCaptureSnapshot().reference.isNotEmpty());
    processor.setPlayHead(nullptr);
    REQUIRE(processor.setLoopCapturePreference(preference));
    waitFor([&]{return processor.swellEngine().savedSession().defaultIR.present();});
    juce::String error;
    Input input;
    waitFor([&]{return processor.pinPortableAssets(input,error);});
    std::shared_ptr<WorkingFolder> folder;
    if(!prepareWorkingFolder(input,destination.file,processor.assetRegistryRoot(),folder,error))
        throw std::runtime_error(error.toStdString());
    if(!processor.useWorkingFolder(folder,input,error))
        throw std::runtime_error(error.toStdString());
    const auto roundTrip=[&](CaptureSourceKind kind)
    {
        const auto accepted=processor.getCaptureSnapshot();
        juce::MemoryBlock bytes;
        processor.getStateInformation(bytes);
        const auto envelope=juce::AudioProcessor::getXmlFromBinary(bytes.getData(),static_cast<int>(bytes.getSize()));
        REQUIRE(envelope && envelope->getStringAttribute("stateVersion")=="9");
        m3::storage::Association association;
        juce::String base,manifest;
        REQUIRE(m3::storage::decodeAssociatedState(envelope->toString(),association,base,manifest,error));
        const auto decoded=juce::XmlDocument::parse(base);
        REQUIRE(decoded && decoded->getChildByName("NextCapture"));
        REQUIRE((decoded->getChildByName("AcceptedLoopSource")!=nullptr)==(kind==CaptureSourceKind::loop));
        auto reopened=std::make_unique<PreverbAudioProcessor>(fixture.root.file);
        reopened->setStateInformation(bytes.getData(),static_cast<int>(bytes.getSize()));
        REQUIRE(reopened->flushCaptureForTesting());
        const auto restored=reopened->getCaptureSnapshot();
        REQUIRE(restored.location->association.store==association.store && !restored.unavailable);
        REQUIRE(restored.sourceKind()==kind && restored.reference==accepted.reference);
        REQUIRE(restored.manifest.loop==accepted.manifest.loop && restored.manifest.revision==accepted.manifest.revision);
        REQUIRE(reopened->getLoopCapturePreference()==processor.getLoopCapturePreference() && !reopened->loopCaptureArmed());

        // Recompute the envelope digest to check shape rejection independently
        // of the integrity check.
        const auto invalid=[&](const auto& mutate)
        {
            auto altered=juce::XmlDocument::parse(base);
            mutate(*altered);
            auto bad=association;
            juce::String output;
            REQUIRE(!m3::storage::encodeAssociatedState(altered->toString(),manifest,bad,output,error));
            const auto digest=m3::storage::associationSnapshot(altered->toString(),manifest);
            auto* node=new juce::XmlElement(*envelope->getChildByName("WorkingFolder"));
            node->setAttribute("snapshot",digest);
            altered->addChildElement(node);
            altered->setAttribute("stateVersion",9);
            juce::String ignoredBase,ignoredManifest;
            REQUIRE(!m3::storage::decodeAssociatedState(altered->toString(),bad,ignoredBase,ignoredManifest,error));
        };
        invalid([](auto& state){state.createNewChildElement("Unknown");});
        invalid([](auto& state){state.addChildElement(new juce::XmlElement(*state.getChildByName("NextCapture")));});
        invalid([](auto& state){state.getChildByName("NextCapture")->setAttribute("count",0);});
        if(kind==CaptureSourceKind::loop)
            invalid([](auto& state){state.getChildByName("AcceptedLoopSource")->setAttribute("version",2);});
    };
    roundTrip(CaptureSourceKind::song);
    fixture.through(fixture.marker(2));
    REQUIRE(processor.armLoopCapture());
    fixture.through(fixture.marker(20),true);
    REQUIRE(processor.enterLoopStandby());
    auto& engine = processor.swellEngine();
    REQUIRE(engine.addTarget(512));
    const auto swellId = engine.savedSession().primary;
    REQUIRE(engine.edit(swellId, [](auto& swell, auto&){
        swell.name = "Loop recovery edit";
        swell.shape.gainDb = -3;
        return true;
    }));
    engine.generate(true);
    waitFor([&]{return engine.savedSession().swells.front().render.present();});
    auto nextCapture = preference;
    nextCapture.enabled = false;
    nextCapture.unit = LoopCapturePreference::Unit::beats;
    nextCapture.count = 3;
    REQUIRE(processor.setLoopCapturePreference(nextCapture));
    roundTrip(CaptureSourceKind::loop);

    waitFor([&]{return processor.pinPortableAssets(input,error);});
    std::shared_ptr<Prepared> prepared;
    REQUIRE(prepare(input, prepared, error));
    const auto original = snapshot(*prepared);
    TestDirectory backupRoot, moved, restoreRoot;
    for (const auto& path : {backupRoot.file, restoreRoot.file})
        REQUIRE(path.createDirectory().wasOk());
    const auto packed = backup(prepared, backupRoot.file);
    REQUIRE(packed.success);
    prepared.reset();
    const auto receipt = backupRoot.file.getChildFile("receipts/" + packed.receipt.locator.snapshot + ".json");
    juce::MemoryBlock saved;
    processor.getStateInformation(saved);
    fixture.processor.reset();
    folder.reset();
    const auto moveDeadline = juce::Time::getMillisecondCounterHiRes() + 5000;
    while (!destination.file.moveFileTo(moved.file) && juce::Time::getMillisecondCounterHiRes() < moveDeadline)
        juce::Thread::sleep(10);
    REQUIRE(moved.file.isDirectory() && !destination.file.exists());

    const auto checkRecovered = [&](PreverbAudioProcessor& owner){
        const auto capture = owner.getCaptureSnapshot();
        REQUIRE(!capture.unavailable && capture.sourceKind() == CaptureSourceKind::loop);
        REQUIRE(capture.reference == original.observed.reference && capture.manifest.revision == original.observed.capture.revision
            && capture.manifest.projectId == original.observed.capture.projectId
            && capture.manifest.instanceId == original.observed.capture.instanceId
            && capture.manifest.sampleRate == original.observed.capture.sampleRate
            && capture.manifest.loop == original.observed.capture.loop);
        REQUIRE(owner.getSourceEditTime(capture).valid && owner.getLoopCapturePreference() == nextCapture);
        REQUIRE(!owner.loopRecaptureEnabled() && !owner.loopCaptureArmed() && !owner.requestLoopCue());
        const auto swell = owner.swellEngine().savedSession().swells.front();
        const auto& savedSwell = original.observed.session.swells.front();
        REQUIRE(swell.id == savedSwell.id && swell.name == savedSwell.name && swell.shape.gainDb == savedSwell.shape.gainDb
            && swell.ir == savedSwell.ir && swell.render == savedSwell.render);
        for (const auto& dependency : original.files)
            REQUIRE(fileHash(owner.swellEngine().assetDirectory().getChildFile(dependency.path)) == dependency.sha256);
    };
    for (const bool useBackup : {false, true})
    {
        TestDirectory missingRegistry;
        REQUIRE(missingRegistry.file.createDirectory().wasOk());
        auto owner = std::make_unique<PreverbAudioProcessor>(missingRegistry.file);
        owner->setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
        REQUIRE(owner->flushCaptureForTesting());
        LoopCaptureFixture::TimingHost rackHost;
        rackHost.position.setIsPlaying(true);
        rackHost.position.setTimeInSamples(0);
        rackHost.position.setPpqPosition(0.0);
        rackHost.position.setBpm(137);
        owner->setPlayHead(&rackHost);
        owner->prepareToPlay(48000, 128);
        juce::AudioBuffer<float> audio(2, 128);
        audio.clear();
        juce::MidiBuffer timing;
        owner->processBlock(audio, timing);
        REQUIRE(owner->getHostPositionDiagnostics().isPlaying && !owner->getSongTime().valid);
        Input missing;
        waitFor([&]{return owner->pinPortableAssets(missing,error,true);});
        REQUIRE(missing.folderUnavailable && owner->getCaptureSnapshot().sourceKind() == CaptureSourceKind::loop);
        std::shared_ptr<WorkingFolder> recovery;
        if (useBackup)
            REQUIRE(prepareRestoredWorkingFolder(missing, receipt, restoreRoot.file, owner->assetRegistryRoot(), recovery, error));
        else
        {
            std::shared_ptr<WorkingFolder> rejected;
            Hooks cancelled;
            cancelled.cancelled = []{return true;};
            REQUIRE(!prepareAssociatedLocation(missing, moved.file, owner->assetRegistryRoot(), rejected, error, cancelled));
            REQUIRE(!prepareAssociatedLocation(missing, restoreRoot.file, owner->assetRegistryRoot(), rejected, error));
            const auto& captured = original.observed.capture.ranges.front();
            const auto& wet = original.observed.session.swells.front().render;
            for (const auto& path : {juce::String(captured.assetPath), wet.path})
            {
                TestDirectory damaged;
                REQUIRE(moved.file.copyDirectoryTo(damaged.file));
                const auto asset = damaged.file.getChildFile("working").getChildFile(original.observed.project)
                    .getChildFile(original.observed.instance).getChildFile(path);
                REQUIRE(asset.existsAsFile() && asset.replaceWithText("corrupt"));
                REQUIRE(!prepareAssociatedLocation(missing, damaged.file, owner->assetRegistryRoot(), rejected, error));
                REQUIRE(owner->getCaptureSnapshot().unavailable && !owner->requestLoopCue());
                REQUIRE(owner->swellEngine().savedSession().swells.front().id == swellId);
            }
            REQUIRE(prepareAssociatedLocation(missing, moved.file, owner->assetRegistryRoot(), recovery, error));
        }
        auto stale = missing;
        ++stale.controlEpoch;
        REQUIRE(!owner->useWorkingFolder(recovery, stale, error));
        REQUIRE(owner->getCaptureSnapshot().unavailable);
        REQUIRE(owner->useWorkingFolder(recovery, missing, error));
        checkRecovered(*owner);
        juce::MemoryBlock recoveredState;
        owner->getStateInformation(recoveredState);
        owner->setPlayHead(nullptr);
        owner.reset();
        recovery.reset();
        auto reopened = std::make_unique<PreverbAudioProcessor>(missingRegistry.file);
        reopened->setStateInformation(recoveredState.getData(), static_cast<int>(recoveredState.getSize()));
        REQUIRE(reopened->flushCaptureForTesting());
        checkRecovered(*reopened);
    }

    TestDirectory incompleteRegistry, incompleteRoot;
    REQUIRE(incompleteRegistry.file.createDirectory().wasOk() && incompleteRoot.file.createDirectory().wasOk());
    auto incomplete = std::make_unique<PreverbAudioProcessor>(incompleteRegistry.file);
    incomplete->setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    REQUIRE(incomplete->flushCaptureForTesting());
    Input unavailable;
    waitFor([&]{return incomplete->pinPortableAssets(unavailable,error,true);});
    std::shared_ptr<WorkingFolder> workspace;
    REQUIRE(prepareRecoveryWorkspace(unavailable, incompleteRoot.file, incomplete->assetRegistryRoot(), workspace, error));
    REQUIRE(incomplete->useWorkingFolder(workspace, unavailable, error));
    const auto unresolved = incomplete->getCaptureSnapshot();
    REQUIRE(unresolved.unavailable && unresolved.sourceKind() == CaptureSourceKind::loop
        && unresolved.reference == original.observed.reference && unresolved.manifest.loop == original.observed.capture.loop);
    REQUIRE(incomplete->swellEngine().savedSession().swells.front().id == swellId && !incomplete->requestLoopCue());
    REQUIRE(incomplete->pinPortableAssets(unavailable,error,true));
    REQUIRE(!prepare(unavailable, prepared, error) && !incompleteRoot.file.getChildFile("receipts").exists());
}

static void workingFolderProcessorCases()
{
    Fixture fixture;TestDirectory destination,forkDestination;juce::String error;
    REQUIRE(destination.file.createDirectory().wasOk() && forkDestination.file.createDirectory().wasOk());
    auto processor=std::make_unique<PreverbAudioProcessor>(fixture.directory.file);load(*processor,fixture.xml);
    ProfileHost host;host.position.setTimeInSamples(0);host.position.setPpqPosition(0.0);host.position.setBpm(120);
    host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});host.position.setIsPlaying(false);
    processor->setPlayHead(&host);processor->prepareToPlay(48000,6);juce::AudioBuffer<float> audio(2,6);audio.clear();juce::MidiBuffer midi;processor->processBlock(audio,midi);
    Input input;waitFor([&]{return processor->pinPortableAssets(input,error);});const auto before=processor->getCaptureSnapshot();const auto flow=processor->getWorkflowSnapshot();
    std::shared_ptr<WorkingFolder> folder;REQUIRE(prepareWorkingFolder(input,destination.file,processor->assetRegistryRoot(),folder,error));
    const auto newRoot=workingFolderInfo(*folder).projectsRoot.getChildFile(fixture.observed.project).getChildFile(fixture.observed.instance);
    TestDirectory backupFolder;REQUIRE(backupFolder.file.createDirectory().wasOk());std::shared_ptr<Prepared> backupInput;
    REQUIRE(prepare(input,backupInput,error));const auto backupResult=backup(backupInput,backupFolder.file);REQUIRE(backupResult.success);backupInput.reset();
    const auto receiptFile=backupFolder.file.getChildFile("receipts/"+backupResult.receipt.locator.snapshot+".json");
    const auto apply=[&](auto& owner,const auto& prepared,const auto& expected){
        auto task=std::async(std::launch::async,[&]{return owner.useWorkingFolder(prepared,expected,error);});
        REQUIRE(task.wait_for(std::chrono::seconds(15))==std::future_status::ready);
        if(!task.get())throw std::runtime_error(error.toStdString());
    };
    Hooks cancel;cancel.cancelled=[]{return true;};REQUIRE(!processor->useWorkingFolder(folder,input,error,cancel));
    REQUIRE(processor->swellEngine().assetDirectory()==input.instanceRoot && processor->getCaptureSnapshot().reference==before.reference);
    auto stale=input;++stale.controlEpoch;REQUIRE(!processor->useWorkingFolder(folder,stale,error));
    REQUIRE(processor->swellEngine().assetDirectory()==input.instanceRoot);
    apply(*processor,folder,input);
    REQUIRE(processor->swellEngine().assetDirectory()==newRoot && processor->getCaptureSnapshot().reference==before.reference);
    REQUIRE(processor->getWorkflowSnapshot().mode==flow.mode && processor->getWorkflowSnapshot().pass==flow.pass
        && processor->getWorkflowSnapshot().phase==flow.phase && processor->getWorkflowSnapshot().recapture==flow.recapture);
    juce::MemoryBlock saved;processor->getStateInformation(saved);const auto xml=juce::AudioProcessor::getXmlFromBinary(saved.getData(),static_cast<int>(saved.getSize()));
    REQUIRE(xml && xml->getStringAttribute("stateVersion")=="9");
    m3::storage::Association association;juce::String base,manifest;
    REQUIRE(m3::storage::decodeAssociatedState(xml->toString(),association,base,manifest,error));
    REQUIRE(association.store==workingFolderInfo(*folder).association.store && association.writer==workingFolderInfo(*folder).association.writer);
    {
        // A same-folder Restore after v8 -> v9 adoption must save the active
        // association even while the worker is held in the loading state.
        const auto expectedManifest=manifest;
        Input active;waitFor([&]{return processor->pinPortableAssets(active,error);});
        std::shared_ptr<Verified> verified;REQUIRE(locateReceipt(receiptFile,verified,error));
        REQUIRE(matchesRecoveryTarget(*verified,active,error));
        REQUIRE(restoreCopy(verified,workingFolderInfo(*folder).projectsRoot).success);
        processor->holdCaptureWorkerForTesting(true);
        waitFor([&]{return processor->captureWorkerHeldForTesting();});
        REQUIRE(processor->reloadRecoveredAssets(active,error));
        juce::MemoryBlock duringReload;processor->getStateInformation(duringReload);
        const auto pendingXml=juce::AudioProcessor::getXmlFromBinary(duringReload.getData(),static_cast<int>(duringReload.getSize()));
        REQUIRE(pendingXml && pendingXml->getStringAttribute("stateVersion")=="9");
        REQUIRE(pendingXml->getStringAttribute("projectId")==fixture.observed.project
            && pendingXml->getStringAttribute("instanceId")==fixture.observed.instance
            && pendingXml->getStringAttribute("manifestReference")==before.reference
            && pendingXml->getStringAttribute("manifestRevision")==xml->getStringAttribute("manifestRevision"));
        m3::storage::Association pendingAssociation;juce::String pendingBase,pendingManifest;
        REQUIRE(m3::storage::decodeAssociatedState(pendingXml->toString(),pendingAssociation,pendingBase,pendingManifest,error));
        REQUIRE(pendingAssociation.store==association.store && pendingAssociation.writer==association.writer
            && pendingAssociation.registration==association.registration && pendingManifest==expectedManifest);
        processor->holdCaptureWorkerForTesting(false);
        REQUIRE(processor->flushCaptureForTesting());
    }
    {CaptureSession verified(workingFolderInfo(*folder).projectsRoot);REQUIRE(verified.restore(before.manifest.projectId,before.manifest.instanceId,before.reference,before.manifest.revision));
    float left=0,right=0;REQUIRE(verified.readSample(0,left,right));}
    {TestDirectory unsafeRegistry;REQUIRE(unsafeRegistry.file.getChildFile("locations").createDirectory().wasOk());
        auto hint=juce::var(new juce::DynamicObject);hint.getDynamicObject()->setProperty("version",1);hint.getDynamicObject()->setProperty("store",association.store);
        hint.getDynamicObject()->setProperty("folder",destination.file.getFileName());
        REQUIRE(unsafeRegistry.file.getChildFile("locations/"+association.registration+".json").replaceWithText(juce::JSON::toString(hint)));
        struct CwdGuard{juce::File prior=juce::File::getCurrentWorkingDirectory();~CwdGuard(){prior.setAsCurrentWorkingDirectory();}} cwd;
        REQUIRE(destination.file.getParentDirectory().setAsCurrentWorkingDirectory());std::shared_ptr<WorkingFolder> inferred;
        REQUIRE(!resolveWorkingFolder(association,base,manifest,unsafeRegistry.file,inferred,error) && !inferred);
    }
    auto reopened=std::make_unique<PreverbAudioProcessor>(fixture.directory.file);reopened->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
    REQUIRE(reopened->flushCaptureForTesting());waitFor([&]{return reopened->swellEngine().assetDirectory()==newRoot;});
    REQUIRE(reopened->getCaptureSnapshot().location->association.writer!=association.writer && !reopened->getCaptureSnapshot().unavailable);
    Input forkInput;waitFor([&]{return reopened->pinPortableAssets(forkInput,error);});std::shared_ptr<WorkingFolder> fork;
    REQUIRE(prepareWorkingFolder(forkInput,forkDestination.file,reopened->assetRegistryRoot(),fork,error));apply(*reopened,fork,forkInput);
    REQUIRE(reopened->getCaptureSnapshot().location->association.store!=association.store && processor->swellEngine().assetDirectory()==newRoot);
    // A legacy save still selects its original tree after an associated load.
    load(*reopened,fixture.xml);waitFor([&]{return reopened->swellEngine().assetDirectory()==fixture.root;});
    REQUIRE(!reopened->getCaptureSnapshot().location->association.present() && !reopened->getCaptureSnapshot().unavailable);
    for(const auto& dependency:fixture.observed.dependencies)REQUIRE(fileHash(fixture.root.getChildFile(dependency.path))==fileHash(newRoot.getChildFile(dependency.path)));
    // A host save while associated resolution is pending retains exact metadata.
    reopened->holdCaptureWorkerForTesting(true);waitFor([&]{return reopened->captureWorkerHeldForTesting();});
    reopened->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));juce::MemoryBlock pending;reopened->getStateInformation(pending);
    const auto pendingXml=juce::AudioProcessor::getXmlFromBinary(pending.getData(),static_cast<int>(pending.getSize()));
    REQUIRE(pendingXml && pendingXml->getStringAttribute("manifestReference")==before.reference && pendingXml->getStringAttribute("stateVersion")=="9");
    reopened->holdCaptureWorkerForTesting(false);REQUIRE(reopened->flushCaptureForTesting());
    const auto processState=fixture.directory.file.getChildFile("association-process-state.xml");REQUIRE(processState.replaceWithText(xml->toString()));
    associationProcessCases(processState,fixture.directory.file,newRoot);
    REQUIRE(processor->getCaptureSnapshot().reference==before.reference && processor->swellEngine().savedSession().swells.front().render==fixture.session.swells.front().render);
    for(const auto& dependency:fixture.observed.dependencies)REQUIRE(fileHash(fixture.root.getChildFile(dependency.path))==fileHash(newRoot.getChildFile(dependency.path)));
    reopened.reset();processor.reset();folder.reset();fork.reset();
    const auto offline=destination.file.getSiblingFile("Lost-"+juce::Uuid().toString());
    const auto renameDeadline=juce::Time::getMillisecondCounterHiRes()+5000;
    while(!destination.file.moveFileTo(offline) && juce::Time::getMillisecondCounterHiRes()<renameDeadline)juce::Thread::sleep(10);
    REQUIRE(offline.isDirectory() && !destination.file.exists());destination.file=offline;
    auto missing=std::make_unique<PreverbAudioProcessor>(fixture.directory.file);missing->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
    REQUIRE(missing->flushCaptureForTesting());missing->setPlayHead(&host);missing->prepareToPlay(48000,6);missing->processBlock(audio,midi);
    Input unavailable;waitFor([&]{return missing->pinPortableAssets(unavailable,error,true);});
    REQUIRE(unavailable.sourceUnavailable && missing->getCaptureSnapshot().manifestKnown && !missing->getCaptureSnapshot().location->writable);
    REQUIRE(!missing->setRecaptureExisting(true));
    TestDirectory recovery;REQUIRE(recovery.file.createDirectory().wasOk());std::shared_ptr<WorkingFolder> workspace;
    REQUIRE(prepareRecoveryWorkspace(unavailable,recovery.file,missing->assetRegistryRoot(),workspace,error));apply(*missing,workspace,unavailable);
    REQUIRE(missing->getCaptureSnapshot().unavailable && missing->getCaptureSnapshot().location->writable && missing->getCaptureSnapshot().reference==before.reference);
    juce::MemoryBlock recoveryState;missing->getStateInformation(recoveryState);const auto recoveryXml=juce::AudioProcessor::getXmlFromBinary(recoveryState.getData(),static_cast<int>(recoveryState.getSize()));
    m3::storage::Association recoveryAssociation;REQUIRE(recoveryXml && m3::storage::decodeAssociatedState(recoveryXml->toString(),recoveryAssociation,base,manifest,error));
    REQUIRE(recoveryAssociation.recoveryOrigins.size()==1 && recoveryAssociation.recoveryOrigins[0].store==association.store
        && !recovery.file.getChildFile("receipts").exists());
    REQUIRE(missing->setRecaptureExisting(true));missing->cancelCapturePass();
    auto recoveryReopen=std::make_unique<PreverbAudioProcessor>(fixture.directory.file);recoveryReopen->setStateInformation(recoveryState.getData(),static_cast<int>(recoveryState.getSize()));
    REQUIRE(recoveryReopen->flushCaptureForTesting());
    REQUIRE(recoveryReopen->getCaptureSnapshot().location->writable && recoveryReopen->getCaptureSnapshot().location->association.store==recoveryAssociation.store);
    for(const auto& dependency:fixture.observed.dependencies)REQUIRE(fileHash(fixture.root.getChildFile(dependency.path))==fileHash(offline.getChildFile("working").getChildFile(fixture.observed.project).getChildFile(fixture.observed.instance).getChildFile(dependency.path)));
    auto relocated=std::make_unique<PreverbAudioProcessor>(fixture.directory.file);relocated->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
    REQUIRE(relocated->flushCaptureForTesting());Input locateInput;waitFor([&]{return relocated->pinPortableAssets(locateInput,error,true);});
    REQUIRE(locateInput.folderUnavailable);std::shared_ptr<WorkingFolder> located;
    if(!prepareAssociatedLocation(locateInput,offline,relocated->assetRegistryRoot(),located,error))throw std::runtime_error("Exact Locate: "+error.toStdString());apply(*relocated,located,locateInput);
    REQUIRE(!relocated->getCaptureSnapshot().unavailable && relocated->getCaptureSnapshot().location->association.store==association.store);
    REQUIRE(relocated->swellEngine().snapshot().renderCount==0);
    TestDirectory cleanRegistry,restoredStore;REQUIRE(cleanRegistry.file.createDirectory().wasOk() && restoredStore.file.createDirectory().wasOk());
    auto restored=std::make_unique<PreverbAudioProcessor>(cleanRegistry.file);restored->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
    REQUIRE(restored->flushCaptureForTesting());Input restoreInput;waitFor([&]{return restored->pinPortableAssets(restoreInput,error,true);});
    REQUIRE(restoreInput.folderUnavailable);std::shared_ptr<WorkingFolder> restoredFolder;
    if(!prepareRestoredWorkingFolder(restoreInput,receiptFile,restoredStore.file,restored->assetRegistryRoot(),restoredFolder,error))throw std::runtime_error("Associated Restore: "+error.toStdString());apply(*restored,restoredFolder,restoreInput);
    REQUIRE(!restored->getCaptureSnapshot().unavailable && restored->getCaptureSnapshot().location->association.store!=association.store);
    REQUIRE(restored->swellEngine().snapshot().renderCount==0 && restored->getCaptureSnapshot().reference==before.reference);
    for(const auto& dependency:fixture.observed.dependencies)REQUIRE(fileHash(fixture.root.getChildFile(dependency.path))==fileHash(restored->swellEngine().assetDirectory().getChildFile(dependency.path)));
    juce::MemoryBlock accepted;restored->getStateInformation(accepted);
    const auto acceptedXml=juce::AudioProcessor::getXmlFromBinary(accepted.getData(),static_cast<int>(accepted.getSize()));REQUIRE(acceptedXml);
    const auto declared=juce::String("<!DOCTYPE PreverbState [<!ENTITY ignored 'must not expand'>]>")+acceptedXml->toString();
    const auto declaredBytes=static_cast<uint32_t>(declared.getNumBytesAsUTF8());const auto littleLength=juce::ByteOrder::swapIfBigEndian(declaredBytes);
    juce::MemoryBlock rejected(static_cast<size_t>(declaredBytes)+9,true);rejected.copyFrom(accepted.getData(),0,4);rejected.copyFrom(&littleLength,4,4);rejected.copyFrom(declared.toRawUTF8(),8,declaredBytes);
    restored->setStateInformation(rejected.getData(),static_cast<int>(rejected.getSize()));juce::MemoryBlock retained;restored->getStateInformation(retained);
    REQUIRE(retained==accepted && restored->swellEngine().snapshot().message.contains("declarations"));
    auto deep=std::make_unique<juce::XmlElement>(*acceptedXml);deep->getChildByName("SwellSession")->deleteAllTextElements();
    deep->getChildByName("SwellSession")->addTextElement(juce::String::repeatedString("[",32)+"0"+juce::String::repeatedString("]",32));
    // Use a legacy envelope here so the creative-depth guard itself is tested,
    // independently of association snapshot-integrity rejection.
    deep->removeChildElement(deep->getChildByName("WorkingFolder"),true);deep->setAttribute("stateVersion",8);
    juce::MemoryBlock nested;juce::AudioProcessor::copyXmlToBinary(*deep,nested);restored->setStateInformation(nested.getData(),static_cast<int>(nested.getSize()));
    restored->getStateInformation(retained);REQUIRE(retained==accepted && restored->swellEngine().snapshot().message.contains("nested"));
    std::cout<<"Production folder association: verified handoff, unchanged Learn state/source, v9 memory-only save, independent reopen lease, physical fork, legacy reopen and pending-restore save passed.\n";
}
