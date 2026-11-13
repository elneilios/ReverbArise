#pragma once
static void recoveryDependencyCases()
{
    Fixture fixture;auto first=fixture.session.swells[0];first.shape.endOffset=2;first.shape.fadeIn=first.shape.fadeOut=0;
    first.generatedGain=1.0f; // The disabled frozen renders already have accepted starting gains.
    fixture.session.swells[0]=first;
    fixture.session.automaticProcessing=false;fixture.session.autoMarkersEnabled=false;fixture.session.autoGenerateOnStop=false;
    auto survivor=first;survivor.id=juce::Uuid().toDashedString();survivor.name="Surviving frozen sound";survivor.ir=fixture.irs[2];
    SwellAssetStore original(fixture.root);WetRender wet;wet.rate=48000;wet.start=-1;wet.sourceStart=0;wet.sourceEnd=2;wet.samples={0,0,.2f,-.1f,.4f,-.3f};juce::String error;
    REQUIRE(original.writeRender(wet,survivor.ir,"source",fixture.session.timing,survivor.render,error));fixture.session.swells.push_back(survivor);fixture.refresh();
    TestDirectory destination;const auto root=destination.file.getChildFile(fixture.observed.project).getChildFile(fixture.observed.instance);REQUIRE(root.createDirectory().wasOk());
    REQUIRE(fixture.root.getChildFile("irs").copyDirectoryTo(root.getChildFile("irs")));REQUIRE(root.getChildFile(survivor.render.path).getParentDirectory().createDirectory().wasOk());
    REQUIRE(fixture.root.getChildFile(survivor.render.path).copyFileTo(root.getChildFile(survivor.render.path)));
    REQUIRE(fixture.root.getChildFile(survivor.render.path).withFileExtension(".json").copyFileTo(root.getChildFile(survivor.render.path).withFileExtension(".json")));
    const auto kept=root.getChildFile(survivor.render.path);const auto keptHash=fileHash(kept);const auto firstId=first.id;
    auto owner=std::make_unique<PreverbAudioProcessor>(destination.file);auto& p=*owner;load(p,fixture.xml);
    ProfileHost host;host.position.setTimeInSamples(0);host.position.setPpqPosition(0.0);host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});host.position.setIsPlaying(false);
    p.setPlayHead(&host);p.prepareToPlay(48000,6);juce::AudioBuffer<float> audio(2,6);audio.clear();juce::MidiBuffer midi;p.processBlock(audio,midi);
    const auto prior=p.getCaptureSnapshot();const auto work=renderWorkCounts();
    REQUIRE(p.setRecaptureExisting(true));juce::AudioBuffer<float> partial(2,1);partial.clear();partial.setSample(0,0,.5f);
    REQUIRE(setFixtureCaptureArmed(p));host.position.setIsPlaying(true);p.processBlock(partial,midi);host.position.setIsPlaying(false);p.processBlock(partial,midi);REQUIRE(p.flushCaptureForTesting());
    REQUIRE(p.getCaptureSnapshot().unavailable && p.getCaptureSnapshot().reference==prior.reference);p.cancelCapturePass();
    if(!p.setRecaptureExisting(true))
        throw std::runtime_error("Re-arm after rejected partial recovery: "+p.swellEngine().snapshot().message.toStdString());
    REQUIRE(setFixtureCaptureArmed(p));audio.setSample(0,0,.5f);audio.setSample(1,0,-.5f);host.position.setIsPlaying(true);p.processBlock(audio,midi);host.position.setIsPlaying(false);p.processBlock(audio,midi);REQUIRE(p.flushCaptureForTesting());
    REQUIRE(!p.getCaptureSnapshot().unavailable && p.getCaptureSnapshot().reference!=prior.reference);
    try{waitFor([&]{return find(p.swellEngine().savedSession(),survivor.id)->phase==Phase::ready;});}
    catch(...){const auto s=p.swellEngine().savedSession();std::cerr<<"Frozen survivor: "<<find(s,survivor.id)->status<<" | root="<<p.swellEngine().assetDirectory().getFullPathName()<<" | source="<<p.getCaptureSnapshot().status<<"\n";throw;}
    auto state=p.swellEngine().savedSession();REQUIRE(find(state,firstId)->frozen && find(state,firstId)->render==first.render);
    REQUIRE(p.swellEngine().snapshot().renderCount==0 && renderWorkCounts().convolutions==work.convolutions && fileHash(kept)==keptHash);
    REQUIRE(p.requestWorkflowMode(workflow::Mode::live));const auto ir=root.getChildFile(first.ir.path),offline=ir.getSiblingFile("missing-custom-ir.wav");waitFor([&]{return ir.moveFileTo(offline);});
    REQUIRE(p.swellEngine().setFrozen(firstId,false));
    waitFor([&]{return find(p.swellEngine().savedSession(),firstId)->phase==Phase::error;});
    state=p.swellEngine().savedSession();REQUIRE(find(state,firstId)->render==first.render && state.swells.size()==2 && fileHash(kept)==keptHash && p.swellEngine().snapshot().renderCount==0);
    REQUIRE(offline.moveFileTo(ir));Input scope;REQUIRE(p.pinPortableAssets(scope,error,true));REQUIRE(p.reloadRecoveredAssets(scope,error));REQUIRE(p.flushCaptureForTesting());
    waitFor([&]{return find(p.swellEngine().savedSession(),firstId)->phase==Phase::ready;});
    state=p.swellEngine().savedSession();REQUIRE(!find(state,firstId)->frozen && find(state,firstId)->render!=first.render && find(state,firstId)->shape==first.shape);
    REQUIRE(find(state,survivor.id)->frozen && find(state,survivor.id)->render==survivor.render && fileHash(kept)==keptHash && p.swellEngine().snapshot().renderCount==1);
    REQUIRE(!state.automaticProcessing && !state.autoMarkersEnabled && !state.autoGenerateOnStop);p.setPlayHead(nullptr);
    std::cout<<"Ordinary dependency recovery: partial-source rejection, missing frozen hold, explicit unfreeze, missing custom IR failure/retention, asset reload and automatic rebuild passed.\n";
}
