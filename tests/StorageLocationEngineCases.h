static void runStorageLocationEngineCases()
{
    TestDirectory original,target,external;CaptureSession captured(original.file);
    std::vector<float> audio(2000,.125f);REQUIRE(captured.commit(0,1000,audio.data(),1000));
    const auto source=captured.snapshot();juce::CriticalSection contextLock;
    PreviewContext context{source,{true,0,0,1000,120,4,4},0};workflow::State flow;REQUIRE(flow.requestMode(workflow::Mode::live,{true,true,true}));
    SwellEngine engine(original.file,[&]{const juce::ScopedLock guard(contextLock);return context;});engine.useWorkflow(flow);
    engine.setAutomaticProcessing(false);engine.setPostRoll(.1);
    const auto firstIR=external.file.getChildFile("first.wav"),secondIR=external.file.getChildFile("second.wav");
    writeTestWav(firstIR,1,1000,{1,.5f,.25f});writeTestWav(secondIR,1,1000,{.5f,.25f,.125f});
    engine.loadDefaultIR(firstIR);waitFor([&]{return engine.savedSession().defaultIR.present() && !engine.snapshot().defaultIRPending;});
    REQUIRE(engine.addTarget(0));waitFor([&]{return engine.snapshot().auditionReady;});
    const auto accepted=engine.savedSession();const auto render=accepted.swells.front().render;const auto count=engine.snapshot().renderCount;
    const auto projects=target.file.getChildFile("working");REQUIRE(original.file.copyDirectoryTo(projects));
    auto lease=std::make_shared<int>(17);
    auto location=std::make_shared<const CaptureStorageLocation>(CaptureStorageLocation{projects,target.file,lease});
    CaptureSession relocated(projects,location);REQUIRE(relocated.restore(source.manifest.projectId,source.manifest.instanceId,source.reference,source.manifest.revision));
    const auto oldDirectory=engine.assetDirectory();
    const auto oldIRs=oldDirectory.getChildFile("irs").getNumberOfChildFiles(juce::File::findFiles);
    {const juce::ScopedLock guard(contextLock);context.capture.status="Loading folder handoff";++context.captureGeneration;}
    REQUIRE(engine.reloadStorageAssets(engine.assignmentEpoch(),source.reference));engine.loadDefaultIR(secondIR);
    juce::Thread::sleep(20);REQUIRE(engine.savedSession().defaultIR==accepted.defaultIR);
    REQUIRE(oldDirectory.getChildFile("irs").getNumberOfChildFiles(juce::File::findFiles)==oldIRs);
    {const juce::ScopedLock guard(contextLock);context.capture=relocated.snapshot();}
    const auto newDirectory=projects.getChildFile(source.manifest.projectId).getChildFile(source.manifest.instanceId);
    waitFor([&]{return engine.assetDirectory()==newDirectory && engine.snapshot().auditionReady
        && !engine.snapshot().defaultIRPending && engine.savedSession().defaultIR!=accepted.defaultIR;});
    REQUIRE(engine.snapshot().renderCount==count && engine.savedSession().swells.front().render==render);
    REQUIRE(fileHash(oldDirectory.getChildFile(render.path))==render.checksum && fileHash(newDirectory.getChildFile(render.path))==render.checksum);
    const auto changedDefault=engine.savedSession().defaultIR;
    REQUIRE(newDirectory.getChildFile(changedDefault.path).existsAsFile() && !oldDirectory.getChildFile(changedDefault.path).exists());
    REQUIRE(flow.requestMode(workflow::Mode::finalise,{true,true,true}));REQUIRE(engine.initializeExport());
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});const auto prepared=engine.snapshot().songExport.prepared;REQUIRE(prepared && prepared->storage==location);
    REQUIRE(newDirectory.getChildFile(prepared->file.path).existsAsFile() && !oldDirectory.getChildFile(prepared->file.path).exists());
    REQUIRE(engine.deliverExport(engine.assignmentEpoch(),prepared->file,target.file.getChildFile("forbidden.wav"),false,false)==0);
    REQUIRE(engine.snapshot().songExport.deliveryMessage.contains("outside managed") && !target.file.getChildFile("forbidden.wav").exists());
    const auto history=engine.savedSession().exportState.history;
    auto unavailable=std::make_shared<const CaptureStorageLocation>(CaptureStorageLocation{projects,target.file,lease,false});
    REQUIRE(engine.reloadStorageAssets(engine.assignmentEpoch(),source.reference));
    {const juce::ScopedLock guard(contextLock);context.capture.location=unavailable;context.capture.unavailable=true;++context.captureGeneration;}
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::error && engine.snapshot().message.contains("working folder unavailable");});
    REQUIRE(!engine.snapshot().auditionReady && engine.savedSession().exportState.history==history);
    REQUIRE(flow.requestMode(workflow::Mode::live,{true,true,true}));
    REQUIRE(!engine.assignDefaultIR(firstIR,engine.assignmentEpoch()));
    float left[16]{},right[16]{};{AudioAllocationGuard guard;engine.processAudio(left,right,16,context.time);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && std::all_of(left,left+16,[](auto sample){return sample==0;}));
    REQUIRE(engine.savedSession().swells.front().render==render && fileHash(oldDirectory.getChildFile(render.path))==render.checksum);
    std::cout<<"Storage engine: coherent root handoff, deferred imports, no redundant render, export ownership/boundary, blocked unavailable writes and callback allocation safety passed.\n";
}
