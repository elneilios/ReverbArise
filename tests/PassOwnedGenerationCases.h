#pragma once
// Worker-level recovery from accepted provenance, independent of future prefs.
static void passOwnedGenerationCases()
{
    TestDirectory root;CaptureSession capture(root.file);juce::CriticalSection mutex;
    PreviewContext context{capture.snapshot(),{false,0,0,48000,120,4,4},0};
    const auto irFile=root.file.getChildFile("A.wav");writeTestWav(irFile,1,48000,{1,.4f,.1f});
    const auto irFileB=root.file.getChildFile("B.wav");writeTestWav(irFileB,1,48000,{.5f,-.1f,.05f});
    const auto map=capture.snapshot().manifest;
    SwellAssetStore store(root.file.getChildFile(map.projectId).getChildFile(map.instanceId));
    CaptureGenerationContext a,b;a.automatic=b.automatic=true;a.irName="Pass A";b.irName="Pass B";
    juce::String error;REQUIRE(store.importIR(irFile,previewMemoryBudget,a.ir,error));
    REQUIRE(store.importIR(irFileB,previewMemoryBudget,b.ir,error));
    a.id=captureGenerationContextId(a);b.id=captureGenerationContextId(b);
    std::vector<float> pcm(48000*2);for(int i=0;i<10000;++i)
        pcm[(12000+i)*2]=pcm[(12000+i)*2+1]=static_cast<float>(.3*std::exp(-i/1800.)*std::sin(i*.17));
    // Chronological pass A is later on the timeline. B's work completes first.
    REQUIRE(capture.commit(96000,48000,pcm.data(),48000,0,&a));
    REQUIRE(capture.commit(0,48000,pcm.data(),48000,0,&b));
    REQUIRE(irFile.deleteFile() && irFileB.deleteFile());
    {const juce::ScopedLock guard(mutex);context.capture=capture.snapshot();}
    Session selected;selected.automaticProcessing=false;selected.autoRegenerate=false;
    selected.defaultIR=b.ir;selected.defaultIRName=b.irName;
    juce::String json;REQUIRE(encode(selected,json,error));Session reopened;REQUIRE(decode(json,reopened,error));
    auto engine=std::make_unique<SwellEngine>(root.file,[&]{const juce::ScopedLock guard(mutex);return context;});
    engine->restore(reopened,context.capture.reference);
    {const juce::ScopedLock guard(mutex);context.time.valid=true;}
    await([&]{const auto s=engine->savedSession();return s.swells.size()==2 && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==Phase::ready;});});
    REQUIRE(engine->setFrozen(engine->savedSession().swells[1].id,true));
    const auto accepted=engine->savedSession();
    REQUIRE(accepted.swells[0].arrival<48000 && accepted.swells[0].ir==b.ir);
    REQUIRE(accepted.swells[1].arrival>=96000 && accepted.swells[1].ir==a.ir);
    REQUIRE(engine->snapshot().renderCount==2 && !accepted.automaticProcessing);
    engine.reset(); // Reopen after completion: exact IR, gain and render identity.
    REQUIRE(encode(accepted,json,error) && decode(json,reopened,error));
    engine=std::make_unique<SwellEngine>(root.file,[&]{const juce::ScopedLock guard(mutex);return context;});
    engine->restore(reopened,context.capture.reference);
    await([&]{const auto s=engine->savedSession();return s.swells.size()==2 && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==Phase::ready;});});
    for(const auto& old:accepted.swells){const auto state=engine->savedSession();const auto* now=find(state,old.id);
        REQUIRE(now && now->ir==old.ir && now->render==old.render && now->shape==old.shape && now->calibration==old.calibration);}
    REQUIRE(engine->snapshot().renderCount==0);
    auto copy=std::make_unique<SwellEngine>(root.file,[&]{const juce::ScopedLock guard(mutex);return context;});
    copy->restore(reopened,context.capture.reference);
    await([&]{const auto s=copy->savedSession();return s.swells.size()==2 && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==Phase::ready;});});
    const auto changed=accepted.swells[0].id;
    REQUIRE(copy->edit(changed,[](auto& w,auto&){w.shape.gainDb=-4;return true;}));
    REQUIRE(find(copy->savedSession(),changed)->shape.gainDb==-4 && find(engine->savedSession(),changed)->shape.gainDb==0);
    // Manual work uses the accepted current default, never earlier pass A.
    REQUIRE(engine->addTarget(130000));REQUIRE(engine->savedSession().swells.back().ir==b.ir);
}
