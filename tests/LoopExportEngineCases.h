#pragma once

static void loopExportEngineCases()
{
    TestDirectory root;CaptureSession capture(root.file);
    CaptureGenerationContext generation;generation.origin=CaptureContextOrigin::legacy;generation.id=captureGenerationContextId(generation);
    REQUIRE(capture.beginPass(1,1,false,&generation,CaptureSourceKind::loop));
    std::vector<float> source(4000,.125f);
    REQUIRE(capture.stagePass(1,0,1000,source.data(),2000));
    REQUIRE(capture.preparePass(1,1,LoopPhraseTiming{120,4,4,4,37,2000}).success);
    REQUIRE(capture.acceptPreparedPass(1,1));
    PreviewContext context{capture.snapshot(),{true,0,0,1000,120,4,4},1};
    SwellEngine engine(root.file,[&]{return context;});engine.setAutomaticProcessing(false);
    waitFor([&]{return engine.savedSession().defaultIR.present();});
    REQUIRE(engine.addTarget(0));const auto id=engine.savedSession().primary;
    const auto ir=root.file.getChildFile("loop-ir.wav");writeTestWav(ir,1,1000,std::vector<float>(101,.002f));
    engine.loadIR(ir);waitFor([&]{return find(engine.savedSession(),id)->irName==ir.getFileName();});
    REQUIRE(engine.generate(true).accepted);waitFor([&]{return find(engine.savedSession(),id)->phase==Phase::ready;});
    engine.setLivePlayback(true,LiveRouting::wetOnly,-12);
    workflow::State flow;REQUIRE(flow.requestMode(workflow::Mode::finalise,{true,true,true}));engine.useWorkflow(flow);
    REQUIRE(engine.initializeExport());
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    const auto first=engine.snapshot().songExport.prepared;
    REQUIRE(first && first->plan->loop && first->file.loop && first->file.dry==0 && first->file.wet==1);
    const auto performGain=juce::Decibels::decibelsToGain(-12.f);
    REQUIRE(std::abs(first->plan->loopWetGain-performGain)<1e-6f);
    juce::String error;REQUIRE(verifyExportFile(engine.assetDirectory(),first->file,error));
    auto unscaled=std::make_shared<SongExportPlan>(*first->plan);unscaled->loopWetGain=1;
    SongMix reference(engine.assetDirectory());
    REQUIRE(reference.prepare(unscaled,previewMemoryBudget,error) && reference.measure(error));
    REQUIRE(reference.plan()->identity!=first->plan->identity);
    REQUIRE(std::abs(first->metrics.peak-reference.metrics().peak*performGain)<1e-6f);
    REQUIRE(!engine.playExport(engine.assignmentEpoch(),44100));
    REQUIRE(engine.setLoopExportOptions({2,16},engine.assignmentEpoch(),flow.snapshot().pass));
    REQUIRE(engine.setExportLevels(.35f,.65f,engine.assignmentEpoch(),flow.snapshot().pass));
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    auto ready=engine.snapshot().songExport.prepared;
    REQUIRE(ready->plan->loop->repeats==2 && ready->file.loop->options.gridBeats==16);
    REQUIRE(ready->file.dry==.35f && ready->file.wet==.65f);
    REQUIRE(ready->plan->loopWetGain==first->plan->loopWetGain);
    REQUIRE(engine.savedSession().exportState.dry==0 && engine.savedSession().exportState.wet==1);
    REQUIRE(!engine.setLoopExportOptions({3,4},engine.assignmentEpoch(),flow.snapshot().pass+1));
    REQUIRE(!engine.setExportLevels(1,1,engine.assignmentEpoch()+1,flow.snapshot().pass));
    // No host grid is supplied to this callback. Preview only needs the audio
    // device rate and must equal the decoded finite file with disclosed ramps.
    constexpr int quantum=127;std::array<float,quantum> left{},right{};
    REQUIRE(engine.playExport(engine.assignmentEpoch(),1000));
    const auto process=[&](int count){left.fill(0);right.fill(0);{AudioAllocationGuard guard;engine.processLoopExport(left.data(),right.data(),count,1000);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);};
    waitFor([&]{process(quantum);return engine.snapshot().songExport.cursor.has_value();});
    std::vector<float> heard;const auto frames=*ready->metrics.audibleEnd-*ready->metrics.audibleStart;
    for(int n=0;n<quantum;++n){heard.push_back(left[n]);heard.push_back(right[n]);}
    for(int64_t at=quantum;at<frames;){const auto count=static_cast<int>(std::min(int64_t{quantum},frames-at));process(count);
        for(int n=0;n<count;++n){heard.push_back(left[n]);heard.push_back(right[n]);}at+=count;juce::Thread::sleep(1);}
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(engine.assetDirectory().getChildFile(ready->file.path).createInputStream().release(),true));
    juce::AudioBuffer<float> decoded(2,static_cast<int>(frames));
    REQUIRE(reader && reader->read(&decoded,0,decoded.getNumSamples(),*ready->metrics.audibleStart-ready->file.origin,true,true));
    for(int n=0;n<frames;++n)for(int c=0;c<2;++c)REQUIRE(std::abs(heard[static_cast<size_t>(n)*2+c]
        -decoded.getSample(c,n)*monitorEdgeGain(static_cast<size_t>(n),static_cast<size_t>(frames),1000,.005))<1e-6f);
    reader.reset();
    TestDirectory delivery;const auto target=delivery.file.getChildFile("loop-saved.wav");
    REQUIRE(engine.deliverExport(engine.assignmentEpoch(),ready->file,target,false,false,flow.snapshot().pass));
    waitFor([&]{return !engine.snapshot().songExport.delivering;});
    REQUIRE(fileHash(target)==ready->file.checksum);
    REQUIRE(engine.deliverExport(engine.assignmentEpoch(),ready->file,{},false,true,flow.snapshot().pass));
    waitFor([&]{return !engine.snapshot().songExport.delivering;});
    REQUIRE(engine.snapshot().songExport.dragReady && fileHash(juce::File(engine.snapshot().songExport.deliveryPath))==ready->file.checksum);
    REQUIRE(engine.playExport(engine.assignmentEpoch(),1000));
    waitFor([&]{process(quantum);return engine.snapshot().songExport.cursor.has_value();});
    engine.holdExportForTesting(true);const auto cursor=*engine.snapshot().songExport.cursor;
    REQUIRE(engine.setExportLevels(.5f,.5f,engine.assignmentEpoch(),flow.snapshot().pass));
    waitFor([&]{return engine.exportWaitingForTesting();});
    for(int block=0;block<4;++block)process(quantum);
    REQUIRE(engine.snapshot().songExport.cursor && *engine.snapshot().songExport.cursor>cursor);
    engine.stop();engine.holdExportForTesting(false);
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    ready=engine.snapshot().songExport.prepared;
    REQUIRE(engine.setExportLevels(1,0,engine.assignmentEpoch(),flow.snapshot().pass));
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    REQUIRE(engine.playExport(engine.assignmentEpoch(),1000));
    waitFor([&]{process(quantum);return engine.snapshot().songExport.cursor.has_value();});
    engine.holdExportForTesting(true);const auto dryCursor=*engine.snapshot().songExport.cursor;
    REQUIRE(engine.setExportLevels(1,1,engine.assignmentEpoch(),flow.snapshot().pass));
    waitFor([&]{return engine.exportWaitingForTesting();});
    for(int block=0;block<4;++block)process(quantum);
    REQUIRE(engine.snapshot().songExport.cursor && *engine.snapshot().songExport.cursor>dryCursor);
    engine.stop();engine.holdExportForTesting(false);
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    REQUIRE(engine.setExportLevels(0,0,engine.assignmentEpoch(),flow.snapshot().pass));
    REQUIRE(engine.snapshot().songExport.phase==ExportPhase::idle && !engine.playExport(engine.assignmentEpoch(),1000));
    REQUIRE(engine.setExportLevels(.5f,.5f,engine.assignmentEpoch(),flow.snapshot().pass));
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    ready=engine.snapshot().songExport.prepared;
    const auto saved=engine.savedSession();
    const auto staleEpoch=engine.assignmentEpoch();
    engine.holdExportForTesting(true);
    REQUIRE(engine.setLoopExportOptions({3,4},engine.assignmentEpoch(),flow.snapshot().pass));
    waitFor([&]{return engine.exportWaitingForTesting();});
    engine.restore(saved,context.capture.reference);engine.holdExportForTesting(false);
    waitFor([&]{return engine.assignmentEpoch()!=staleEpoch && engine.snapshot().songExport.phase==ExportPhase::ready;});
    REQUIRE(engine.savedSession().exportState.loop==saved.exportState.loop && !engine.snapshot().songExport.previewRequested);
    REQUIRE(!engine.deliverExport(staleEpoch,ready->file,target,true,false,flow.snapshot().pass));
    REQUIRE(engine.savedSession().exportState.history==saved.exportState.history);
    engine.holdExportForTesting(true);
    REQUIRE(engine.setLoopExportOptions({3,4},engine.assignmentEpoch(),flow.snapshot().pass));
    waitFor([&]{return engine.exportWaitingForTesting();});
    REQUIRE(flow.requestMode(workflow::Mode::live,{true,true,true}));engine.holdExportForTesting(false);
    waitFor([&]{return !engine.exportWaitingForTesting();});
    REQUIRE(engine.savedSession().exportState.history==saved.exportState.history && verifyExportFile(engine.assetDirectory(),ready->file,error));
}
