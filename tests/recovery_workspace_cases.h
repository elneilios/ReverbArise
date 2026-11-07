static void recoveryWorkspaceCases()
{
    Fixture original;TestDirectory oldStore,registry,recovery;
    REQUIRE(oldStore.file.createDirectory().wasOk() && registry.file.createDirectory().wasOk() && recovery.file.createDirectory().wasOk());
    Input input{original.root,original.xml,original.manifest};juce::String error;
    std::shared_ptr<WorkingFolder> normal;checked(prepareWorkingFolder(input,oldStore.file,registry.file,normal,error),error);
    auto origin=workingFolderInfo(*normal).association;juce::String saved;
    checked(encodeAssociatedState(input.processorXml,input.manifestJson,origin,saved,error),error);
    Input missing{oldStore.file.getChildFile("working").getChildFile(original.observed.project).getChildFile(original.observed.instance),saved,input.manifestJson};
    missing.sourceUnavailable=true;missing.controlEpoch=73;normal.reset();
    const auto offline=oldStore.file.getSiblingFile("offline-"+oldStore.file.getFileName());
    // Windows scanners may briefly retain a just-written disposable directory.
    // The original lease is already released; require the actual rename before
    // exercising recovery, rather than weakening the unavailable-store premise.
    const auto renameDeadline=juce::Time::getMillisecondCounterHiRes()+5000;
    while(!oldStore.file.moveFileTo(offline) && juce::Time::getMillisecondCounterHiRes()<renameDeadline)juce::Thread::sleep(10);
    REQUIRE(offline.isDirectory() && !oldStore.file.exists());oldStore.file=offline;
    std::shared_ptr<WorkingFolder> folder;checked(prepareRecoveryWorkspace(missing,recovery.file,registry.file,folder,error),error);
    const auto& info=workingFolderInfo(*folder);
    REQUIRE(info.incompleteRecovery && info.originalFolderUnavailable && info.captureMetadataKnown && info.recreatedCaptureMetadata);
    REQUIRE(info.originalAssociation==origin && info.association.store!=origin.store && !info.missing.empty());
    REQUIRE(!recovery.file.getChildFile("receipts").exists() && !recovery.file.getChildFile("snapshots").exists());
    const auto recordFile=info.projectsRoot.getChildFile(original.observed.project).getChildFile(original.observed.instance).getChildFile("recovery/"+info.recoveryRecord+".json");
    const auto record=parse(recordFile.loadFileAsString());REQUIRE(record["complete"].isBool() && !static_cast<bool>(record["complete"]));
    REQUIRE(record["originalProcessorXml"].toString()==saved && record["originalCaptureManifest"].toString()==original.manifest);
    REQUIRE(fileHash(recordFile)==info.recoveryRecord && !recordFile.replaceWithText("must not change pinned recovery metadata"));
    auto recoveryAssociation=info.association;juce::String recoveryState,base,recoveredManifest;Association decoded;
    checked(encodeAssociatedState(original.xml,original.manifest,recoveryAssociation,recoveryState,error),error);
    checked(decodeAssociatedState(recoveryState,decoded,base,recoveredManifest,error),error);
    REQUIRE(decoded.recoveryRecord==info.recoveryRecord && decoded.recoveryOrigins.size()==1 && decoded.recoveryOrigins[0].store==origin.store
        && decoded.recoveryOrigins[0].snapshot==origin.snapshot && decoded.recoveryOrigins[0].reference==original.observed.reference);
    auto malformed=juce::XmlDocument::parse(recoveryState);malformed->getChildByName("WorkingFolder")->getChildByName("RecoveryOrigins")->setAttribute("version",2);
    REQUIRE(!decodeAssociatedState(malformed->toString(),decoded,base,recoveredManifest,error) && decoded==recoveryAssociation);
    checked(revalidateRecoveryWorkspace(*folder,missing,error),error);REQUIRE(!revalidateWorkingFolder(*folder,missing,error));
    auto changed=missing;++changed.controlEpoch;REQUIRE(!revalidateRecoveryWorkspace(*folder,changed,error));
    changed=missing;changed.sourceUnavailable=false;REQUIRE(!revalidateRecoveryWorkspace(*folder,changed,error));
    const auto root=info.projectsRoot.getChildFile(original.observed.project).getChildFile(original.observed.instance);
    // The directory's pinned marker cannot be removed to make it a junction.
    const auto category=root.getChildFile("renders"),marker=category.getChildFile(".directory-pin");
    REQUIRE(marker.existsAsFile() && !marker.deleteFile());
    TestDirectory outside;REQUIRE(outside.file.createDirectory().wasOk());bool redirected=false;
    try{junction(category,outside.file);redirected=true;}catch(const std::exception&){}
    REQUIRE(!redirected && marker.existsAsFile());
    Input incomplete{root,original.xml,original.manifest};std::shared_ptr<Prepared> certificate;REQUIRE(!prepare(incomplete,certificate,error));
    CaptureSession captured(info.projectsRoot);REQUIRE(!captured.restore(original.observed.project.toStdString(),original.observed.instance.toStdString(),original.observed.reference,original.observed.capture.revision));
    REQUIRE(captured.snapshot().unavailable && captured.snapshot().manifest.ranges.size()==original.observed.capture.ranges.size());
    // The ordinary capture transaction can accept complete fresh source here.
    // IR/frozen/export recovery and render readiness remain separate authorities.
    const auto captureContext=original.observed.capture.generationContexts.front();
    REQUIRE(captured.beginRecoveryPass(1,99,{{0,6}},&captureContext));
    const float fresh[]{.1f,-.2f,.3f,-.4f,.2f,-.1f,.4f,-.3f,0,0,0,0};
    if(!captured.stagePass(1,0,48000,fresh,6))throw std::runtime_error(captured.passFailure().toStdString());const auto accepted=captured.finishPass(1,99);
    REQUIRE(accepted.success && !captured.snapshot().unavailable && captured.snapshot().reference!=original.observed.reference);
    float left=0,right=0;REQUIRE(captured.readSample(0,left,right) && left==.1f && right==-.2f);
    REQUIRE(recordFile.loadFileAsString()==juce::JSON::toString(record,true));
    // The missing original locator and its complete offline bytes remain intact.
    for(const auto& dependency:original.observed.dependencies){const auto file=original.root.getChildFile(dependency.path);
        REQUIRE(file.existsAsFile());REQUIRE(fileHash(file)==fileHash(offline.getChildFile("working").getChildFile(original.observed.project).getChildFile(original.observed.instance).getChildFile(dependency.path)));}

    Fixture partial;const auto audio=std::find_if(partial.observed.dependencies.begin(),partial.observed.dependencies.end(),[](const auto& dependency){return dependency.path.startsWith("segments/");});
    REQUIRE(audio!=partial.observed.dependencies.end() && partial.root.getChildFile(audio->path).deleteFile());
    TestDirectory partialTarget;REQUIRE(partialTarget.file.createDirectory().wasOk());Input partialInput{partial.root,partial.xml,{}};partialInput.sourceUnavailable=true;
    std::shared_ptr<WorkingFolder> partialFolder;checked(prepareRecoveryWorkspace(partialInput,partialTarget.file,registry.file,partialFolder,error),error);
    const auto& partialInfo=workingFolderInfo(*partialFolder);REQUIRE(partialInfo.captureMetadataKnown && !partialInfo.originalFolderUnavailable && !partialInfo.originalAssociation.present());
    REQUIRE(partialInfo.missing.size()==1 && partialInfo.missing[0]==audio->path && partialInfo.files+1==partial.observed.dependencies.size());
    const auto partialRoot=partialInfo.projectsRoot.getChildFile(partial.observed.project).getChildFile(partial.observed.instance);
    for(const auto& dependency:partial.observed.dependencies)if(dependency.path!=audio->path)REQUIRE(fileHash(partialRoot.getChildFile(dependency.path))==fileHash(partial.root.getChildFile(dependency.path)));

    // Legacy metadata without its manifest retains uncertainty, never invents chunks.
    TestDirectory unknownTarget,unknownSource;REQUIRE(unknownTarget.file.createDirectory().wasOk());
    Input unknown{unknownSource.file.getChildFile(original.observed.project).getChildFile(original.observed.instance),original.xml,{}};unknown.sourceUnavailable=true;
    std::shared_ptr<WorkingFolder> unknownFolder;checked(prepareRecoveryWorkspace(unknown,unknownTarget.file,registry.file,unknownFolder,error),error);
    const auto& unknownInfo=workingFolderInfo(*unknownFolder);REQUIRE(!unknownInfo.captureMetadataKnown && unknownInfo.files==0 && !unknownInfo.recreatedCaptureMetadata);
    REQUIRE(!unknownInfo.projectsRoot.getChildFile(original.observed.project).getChildFile(original.observed.instance).getChildFile(original.observed.reference).exists());

    const auto unchanged=folder;
    for(const auto& event:juce::StringArray{"recovery-workspace-start","recovery-workspace-ready"}){
        TestDirectory target;REQUIRE(target.file.createDirectory().wasOk());Hooks fault;fault.checkpoint=[&](const auto& name,const auto&){return name!=event;};
        REQUIRE(!prepareRecoveryWorkspace(missing,target.file,registry.file,folder,error,fault) && folder==unchanged && !target.file.getChildFile("receipts").exists());
    }
    {TestDirectory target;REQUIRE(target.file.createDirectory().wasOk());ReadOnlyFixture deny(target.file);REQUIRE(!prepareRecoveryWorkspace(missing,target.file,registry.file,folder,error) && folder==unchanged);}
    {TestDirectory target;REQUIRE(target.file.createDirectory().wasOk());Hooks full;full.injectWriteError=[](auto,auto){return static_cast<uint32_t>(ERROR_DISK_FULL);};REQUIRE(!prepareRecoveryWorkspace(missing,target.file,registry.file,folder,error,full) && folder==unchanged);}
    {TestDirectory target;REQUIRE(target.file.createDirectory().wasOk());Hooks cancelled;cancelled.cancelled=[]{return true;};REQUIRE(!prepareRecoveryWorkspace(missing,target.file,registry.file,folder,error,cancelled) && folder==unchanged);}
    {TestDirectory target;REQUIRE(target.file.createDirectory().wasOk());Input bad=missing;bad.processorXml="<!DOCTYPE bad>"+bad.processorXml;REQUIRE(!prepareRecoveryWorkspace(bad,target.file,registry.file,folder,error) && folder==unchanged && !target.file.getChildFile("store.json").exists());}
    std::cout<<"Recovery workspace: original locators/metadata, verified survivors, unknown source history, normal complete Learn capture, stale/cancel/read-only/disk-full/fault guards and no complete-pack claim passed.\n";
}
