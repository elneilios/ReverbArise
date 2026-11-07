#pragma once
static void associationChild(const juce::File& stateFile,const juce::File& registry,const juce::File& output,const juce::File& release,int variant)
{
    auto p=std::make_unique<PreverbAudioProcessor>(registry);load(*p,stateFile.loadFileAsString());
    ProfileHost host;host.position.setTimeInSamples(0);host.position.setPpqPosition(0.0);host.position.setBpm(120);
    host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});host.position.setIsPlaying(false);
    p->setPlayHead(&host);p->prepareToPlay(48000,14);juce::AudioBuffer<float> audio(2,14);audio.clear();juce::MidiBuffer midi;p->processBlock(audio,midi);
    auto& engine=p->swellEngine();Input initial;juce::String error;waitFor([&]{return p->pinPortableAssets(initial,error);});
    const auto old=p->getCaptureSnapshot();const auto oldRender=engine.savedSession().swells.front().render;
    engine.setAutomaticProcessing(false);engine.setAutoMarkers(false);engine.setAutoGenerateOnStop(false);
    TestDirectory external;REQUIRE(external.file.createDirectory().wasOk());const auto irFile=external.file.getChildFile("writer-ir.wav");
    writeTestWav(irFile,1,48000,{.12f*variant,.04f,-.02f});REQUIRE(engine.assignDefaultIR(irFile,engine.assignmentEpoch()));
    waitFor([&]{return !engine.snapshot().defaultIRPending && engine.savedSession().defaultIRName==irFile.getFileName();});
    const auto imported=engine.savedSession().defaultIR;REQUIRE(imported.present());REQUIRE(p->setRecaptureExisting(true));
    for(int c=0;c<2;++c)for(int i=0;i<14;++i)audio.setSample(c,i,static_cast<float>((variant+1)*(i+1))*.001f*(c?-1.0f:1.0f));
    host.position.setIsPlaying(true);p->processBlock(audio,midi);host.position.setIsPlaying(false);p->processBlock(audio,midi);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(!p->getCaptureSnapshot().unavailable && p->getCaptureSnapshot().reference!=old.reference);
    REQUIRE(p->requestWorkflowMode(workflow::Mode::live));const auto id=engine.savedSession().swells.front().id;
    REQUIRE(engine.setFrozen(id,false));REQUIRE(engine.edit(id,[](auto& w,auto&){w.enabled=true;w.shape.endOffset=2;w.shape.fadeIn=w.shape.fadeOut=0;return true;}));
    const auto selected=engine.savedSession().swells.front();REQUIRE(engine.assignIR(id,selected.inputGeneration,irFile,false,engine.assignmentEpoch()));
    waitFor([&]{const auto s=engine.savedSession();return s.swells.front().ir.content==imported.content && s.swells.front().render!=oldRender && s.swells.front().phase==Phase::ready;});
    REQUIRE(p->requestWorkflowMode(workflow::Mode::finalise));REQUIRE(engine.initializeExport());
    waitFor([&]{const auto snapshot=engine.snapshot();if(snapshot.songExport.phase==ExportPhase::error)throw std::runtime_error(snapshot.songExport.message.toStdString());return snapshot.songExport.phase==ExportPhase::ready;});
    const auto frame=p->getCaptureSnapshot();const auto session=engine.savedSession();const auto prepared=engine.snapshot().songExport.prepared;REQUIRE(prepared);
    juce::MemoryBlock bytes;p->getStateInformation(bytes);const auto xml=juce::AudioProcessor::getXmlFromBinary(bytes.getData(),static_cast<int>(bytes.getSize()));REQUIRE(xml);
    const auto root=engine.assetDirectory();auto record=juce::var(new juce::DynamicObject);auto* values=record.getDynamicObject();
    values->setProperty("writer",frame.location->association.writer);values->setProperty("capture",frame.reference);values->setProperty("xml",xml->toString());
    values->setProperty("ir",session.swells.front().ir.path);values->setProperty("render",session.swells.front().render.path);values->setProperty("export",prepared->file.path);
    values->setProperty("renderHash",fileHash(root.getChildFile(session.swells.front().render.path)));values->setProperty("exportHash",fileHash(root.getChildFile(prepared->file.path)));
    REQUIRE(output.replaceWithText(juce::JSON::toString(record)));const auto deadline=juce::Time::getMillisecondCounterHiRes()+30000;
    while(!release.existsAsFile()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<deadline);juce::Thread::sleep(10);}
}
static void associationProcessCases(const juce::File& stateFile,const juce::File& registry,const juce::File& instanceRoot)
{
    TestDirectory bridge;REQUIRE(bridge.file.createDirectory().wasOk());const auto release=bridge.file.getChildFile("release");
    juce::ChildProcess children[2];juce::File results[2]{bridge.file.getChildFile("a.json"),bridge.file.getChildFile("b.json")};
    for(int i=0;i<2;++i){juce::StringArray command{juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName(),"--association-child",stateFile.getFullPathName(),registry.getFullPathName(),results[i].getFullPathName(),release.getFullPathName(),juce::String(i+1)};REQUIRE(children[i].start(command));}
    const auto deadline=juce::Time::getMillisecondCounterHiRes()+45000;
    while(!results[0].existsAsFile() || !results[1].existsAsFile()){
        for(auto& child:children)if(!child.isRunning()){std::cerr<<child.readAllProcessOutput();REQUIRE(child.getExitCode()==0);}
        REQUIRE(juce::Time::getMillisecondCounterHiRes()<deadline);juce::Thread::sleep(10);
    }
    const auto a=juce::JSON::parse(results[0].loadFileAsString()),b=juce::JSON::parse(results[1].loadFileAsString());
    for(const auto* field:{"writer","capture","ir","render","export","renderHash","exportHash"})REQUIRE(a[field].toString()!=b[field].toString());
    for(const auto& value:{a,b}){
        const auto writer=value["writer"].toString();const auto iid=instanceRoot.getFileName();
        const auto lease=instanceRoot.getParentDirectory().getParentDirectory().getParentDirectory().getChildFile("instances/"+iid+"/branches/"+writer+"/lease");
        const auto denied=CreateFileW(lease.getFullPathName().toWideCharPointer(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);REQUIRE(denied==INVALID_HANDLE_VALUE);
        REQUIRE(fileHash(instanceRoot.getChildFile(value["render"].toString()))==value["renderHash"].toString());
        REQUIRE(fileHash(instanceRoot.getChildFile(value["export"].toString()))==value["exportHash"].toString());
        auto reopen=std::make_unique<PreverbAudioProcessor>(registry);load(*reopen,value["xml"].toString());REQUIRE(!reopen->getCaptureSnapshot().unavailable);
        REQUIRE(reopen->getCaptureSnapshot().reference==value["capture"].toString() && reopen->getCaptureSnapshot().location->association.writer!=writer);
    }
    REQUIRE(release.replaceWithText("release"));for(auto& child:children){REQUIRE(child.waitForProcessToFinish(10000));const auto log=child.readAllProcessOutput();if(child.getExitCode()!=0)std::cerr<<log;REQUIRE(child.getExitCode()==0);}
    std::cout<<"Independent native processor processes: simultaneous branch leases, actual capture/IR/render/export publication, exact saved reopen and preserved original history passed.\n";
}
