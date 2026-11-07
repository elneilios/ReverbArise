static bool storageLocationTests()
{
    TemporaryRoot original,target,absent;
    auto firstLease=std::make_shared<int>(1);std::weak_ptr<int> firstWeak=firstLease;
    auto first=std::make_shared<const CaptureStorageLocation>(CaptureStorageLocation{original.file,original.file,firstLease});
    CaptureSession source(original.file,first);const float audio[]{.1f,-.2f,.3f,-.4f};CHECK(source.commit(0,1000,audio,2));
    const auto saved=source.snapshot();juce::String manifest,error;CHECK(serializeCaptureManifest(saved.manifest,manifest,error));
    CHECK(saved.location==first && saved.manifestKnown && saved.location->writable);
    CHECK(original.file.copyDirectoryTo(target.file));
    auto secondLease=std::make_shared<int>(2);std::weak_ptr<int> secondWeak=secondLease;
    auto second=std::make_shared<const CaptureStorageLocation>(CaptureStorageLocation{target.file,target.file,secondLease});
    auto third=std::make_shared<const CaptureStorageLocation>(CaptureStorageLocation{absent.file,absent.file,std::make_shared<int>(3)});
    auto queue=std::make_unique<PluginCaptureQueue>();auto worker=std::make_unique<CaptureWorker>(*queue,original.file);
    workflow::State flow;worker->useWorkflow(flow);worker->setHostRate(1000);worker->startThread();
    const auto restore=[&](CaptureWorker::StorageRestore storage,bool preserve){worker->requestRestore(saved.manifest.projectId,saved.manifest.instanceId,saved.reference,saved.manifest.revision,workflow::Mode::capture,preserve,std::move(storage));};
    CaptureWorker::StorageRestore selected;selected.location=second;selected.savedManifest=manifest;
    restore(selected,true);CHECK(worker->flush());CHECK(!worker->isLoading());
    auto retained=worker->snapshot();CHECK(retained.location==second && retained.reference==saved.reference && !retained.unavailable && retained.manifestKnown);
    CaptureSession copied(target.file,second);CHECK(copied.restore(saved.manifest.projectId,saved.manifest.instanceId,saved.reference,saved.manifest.revision));
    float left=0,right=0;CHECK(copied.readSample(0,left,right) && left==.1f && right==-.2f);
    selected.location=third;restore(selected,true);CHECK(worker->flush());
    CHECK(worker->snapshot().location==second && !worker->snapshot().unavailable); // Failed cutover preserves the accepted writer.
    selected.allowMissingSource=true;restore(selected,true);CHECK(worker->flush());
    CHECK(worker->snapshot().location==third && worker->snapshot().unavailable && worker->snapshot().manifestKnown);
    CHECK(worker->snapshot().manifest.ranges.size()==saved.manifest.ranges.size());
    secondLease.reset();second.reset();CHECK(!secondWeak.expired()); // Prior source readers still pin their directory/lease.
    std::atomic<bool> entered=false,released=false;
    CaptureWorker::StorageRestore slow;slow.savedManifest=manifest;slow.allowMissingSource=true;
    slow.resolve=[&]{entered=true;while(!released.load())juce::Thread::sleep(1);return third;};
    restore(slow,true);const auto until=juce::Time::getMillisecondCounter()+10000;
    while(!entered.load() && juce::Time::getMillisecondCounter()<until)juce::Thread::sleep(1);
    CHECK(entered.load());selected.location=retained.location;selected.allowMissingSource=false;
    restore(selected,true);released=true;CHECK(worker->flush());CHECK(worker->snapshot().location==retained.location && !worker->snapshot().unavailable);
    CaptureWorker::StorageRestore unavailable;unavailable.savedManifest=manifest;unavailable.allowMissingSource=true;unavailable.resolve=[]{return std::shared_ptr<const CaptureStorageLocation>{};};
    restore(unavailable,false);CHECK(worker->flush());auto unresolved=worker->snapshot();
    CHECK(unresolved.unavailable && unresolved.manifestKnown && unresolved.location && !unresolved.location->writable);
    CHECK(unresolved.manifest.projectId==saved.manifest.projectId && unresolved.reference==saved.reference);
    CHECK(!worker->armRecovery({{0,2}}));
    CaptureSession blocked({},unresolved.location);CHECK(!blocked.restore(saved.manifest.projectId,saved.manifest.instanceId,saved.reference,saved.manifest.revision,manifest));
    CHECK(!blocked.clear(1000) && !blocked.beginRecoveryPass(8,9,{{0,2}}) && !blocked.commit(0,1000,audio,2));
    CHECK(source.snapshot().reference==saved.reference && source.readSample(0,left,right) && left==.1f && right==-.2f);
    // An associated host manifest remains authoritative even if a local file
    // with the same IDs/revision has been replaced with a different valid map.
    TemporaryRoot tampered;CHECK(original.file.copyDirectoryTo(tampered.file));
    auto other=saved.manifest;other.ranges.clear();juce::String changedJson;CHECK(serializeCaptureManifest(other,changedJson,error));
    const auto local=tampered.file.getChildFile(saved.manifest.projectId).getChildFile(saved.manifest.instanceId).getChildFile(saved.reference);
    CHECK(local.replaceWithText(changedJson));auto owned=std::make_shared<const CaptureStorageLocation>(CaptureStorageLocation{tampered.file,tampered.file,{}});
    CaptureSession authoritative(tampered.file,owned);CHECK(authoritative.restore(saved.manifest.projectId,saved.manifest.instanceId,saved.reference,saved.manifest.revision,manifest));
    CHECK(authoritative.snapshot().manifest.ranges.size()==saved.manifest.ranges.size() && authoritative.readSample(0,left,right) && left==.1f && right==-.2f);
    CHECK(local.loadFileAsString()==changedJson); // No silent filesystem repair.
    worker->stopThread(-1);
    std::cout<<"Capture storage: pinned reader locations, verified root restore, failure rollback, generation fencing and blocked unavailable-folder writes passed.\n";
    return true;
}
