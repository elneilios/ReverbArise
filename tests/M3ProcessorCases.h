#pragma once
#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <source_location>

template<class F> static void waitM3(F condition,const std::source_location location=std::source_location::current())
{
    const auto deadline=juce::Time::getMillisecondCounter()+15000;
    while(!condition()) {if(juce::Time::getMillisecondCounter()>deadline)throw std::runtime_error("M3 timeout at "+std::string(location.file_name())+":"+std::to_string(location.line()));juce::Thread::sleep(2);}
}
inline void m3ProcessorCases()
{
    TestDirectory root;auto processor=std::make_unique<PreverbAudioProcessor>(root.file);
    processor->swellEngine().setMonitoring(m3::AuditionMode::wetOnly,.5f); // This historical oracle explicitly exercises wet-only monitoring.
    REQUIRE(processor->swellEngine().setGainPolicy(m3::GainPolicy::legacy)); // Retain the pre-upgrade numerical oracle.
    processor->swellEngine().setLivePlayback(false,m3::LiveRouting::dryWet,0);
    processor->swellEngine().setAutomaticProcessing(false);
    processor->prepareToPlay(1000,1000);TestPlayHead head;
    head.position.setBpm(240);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});head.position.setIsPlaying(true);
    processor->setPlayHead(&head);juce::AudioBuffer<float> buffer(2,1000);juce::MidiBuffer midi;
    const auto position=[&](int64_t sample){head.position.setTimeInSamples(sample);head.position.setPpqPosition(sample/250.0);};
    processor->setMappingEnabled(true);
    for(int block=0;block<2;++block)
    {position(block*1000);buffer.clear();buffer.setSample(0,0,0.25f);buffer.setSample(1,0,-0.5f);processor->processBlock(buffer,midi);}
    finishCaptureFixture(*processor,head.position);REQUIRE(processor->flushCaptureForTesting());
    auto& engine=processor->swellEngine();REQUIRE(engine.addTarget(0));const auto first=engine.savedSession().primary;
    REQUIRE(engine.addTarget(1000));const auto second=engine.savedSession().primary;
    REQUIRE(engine.selectTargets({first,second},first));const auto original=root.file.getChildFile("m3-ir.wav");
    writeTestWav(original,1,1000,{1,0.5f,0.25f});engine.loadIR(original);
    waitM3([&]{auto s=engine.savedSession();return s.swells[0].irName=="m3-ir.wav" && s.swells[1].irName=="m3-ir.wav";});
    engine.generate(true);waitM3([&]{auto s=engine.snapshot();return s.auditionReady && s.session.swells[1].phase==m3::Phase::ready;});
    REQUIRE(engine.edit(first,[](auto& w,auto&){w.name="First arrival";w.shape.automaticStart=false;w.shape.startOffset=-2;w.shape.endOffset=1;
        w.shape.fadeIn=w.shape.fadeOut=1;w.shape.inCurve=m3::Curve::smooth;w.shape.gainDb=-3;return true;}));
    engine.setPreferences(-24,0.7f,0.12,true);waitM3([&]{return engine.snapshot().auditionReady;});
    const auto verifyInFlightSave=[&]
    {
        juce::MemoryBlock bytes;processor->getStateInformation(bytes);
        auto state=juce::AudioProcessor::getXmlFromBinary(bytes.getData(),static_cast<int>(bytes.getSize()));
        REQUIRE(state && state->getChildByName("SwellSession"));m3::Session records;juce::String parseError;
        REQUIRE(m3::decode(state->getChildByName("SwellSession")->getAllSubText(),records,parseError));
        const auto map=processor->getCaptureSnapshot().manifest;
        m3::SwellAssetStore store(root.file.getChildFile(map.projectId).getChildFile(map.instanceId));
        for(const auto& w:records.swells) {if(w.ir.present())REQUIRE(store.verify(w.ir,parseError));if(w.render.present())REQUIRE(store.verify(w.render,parseError));}
    };
    engine.generate();verifyInFlightSave();waitM3([&]{return engine.snapshot().auditionReady;});
    const auto beforeIdentical=engine.snapshot().renderCount;
    engine.loadIR(original);verifyInFlightSave();waitM3([&]{return engine.savedSession().swells[0].phase==m3::Phase::ready;});
    REQUIRE(engine.snapshot().renderCount==beforeIdentical); // Same verified PCM needs no convolution.
    engine.generate(true);waitM3([&]{return engine.snapshot().auditionReady;});
    armCaptureFixture(*processor);verifyInFlightSave();processor->cancelRecapture();REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));
    // Live preferences persist independently from the selected edit and audition trim.
    auto liveState=engine.savedSession();liveState.liveEnabled=false;liveState.liveRouting=m3::LiveRouting::wetOnly;liveState.liveWetDb=-7.5f;
    engine.restore(liveState);waitM3([&]{return engine.snapshot().auditionReady;});
    juce::MemoryBlock saved;processor->getStateInformation(saved);const auto savedModel=engine.savedSession();
    const auto capture=processor->getCaptureSnapshot();REQUIRE(saved.getSize()<16384);
    auto xml=juce::AudioProcessor::getXmlFromBinary(saved.getData(),static_cast<int>(saved.getSize()));REQUIRE(xml && xml->getIntAttribute("stateVersion")==8);
    const auto rejected=[&](juce::XmlElement& bad)
    {
        juce::MemoryBlock bytes;juce::AudioProcessor::copyXmlToBinary(bad,bytes);processor->setStateInformation(bytes.getData(),static_cast<int>(bytes.getSize()));
        REQUIRE(engine.savedSession().primary==first && engine.savedSession().swells.size()==2);
        REQUIRE(processor->getCaptureSnapshot().reference==capture.reference);
    };
    auto bad=std::make_unique<juce::XmlElement>(*xml);bad->setAttribute("stateVersion",9);rejected(*bad);
    bad=std::make_unique<juce::XmlElement>(*xml);bad->setAttribute("manifestReference","../outside.json");rejected(*bad);
    bad=std::make_unique<juce::XmlElement>(*xml);auto child=bad->getChildByName("SwellSession");auto json=child->getAllSubText();
    child->deleteAllChildElements();child->addTextElement(json.replace("\"version\": 11","\"version\": 12"));rejected(*bad);
    // Entering Capture stops preview; capture still reads only original dry.
    REQUIRE(engine.audition());armCaptureFixture(*processor);buffer.setSize(2,500);buffer.clear();head.position.setIsPlaying(true);position(2900);
    {AudioAllocationGuard guard;processor->processBlock(buffer,midi);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && buffer.getMagnitude(5,495)==0);
    finishCaptureFixture(*processor,head.position);REQUIRE(!processor->getWorkflowSnapshot().recapture);
    auto manifest=processor->getCaptureSnapshot().manifest;CaptureAssetStore assets(root.file.getChildFile(manifest.projectId).getChildFile(manifest.instanceId));
    SourceInterval silence;juce::String error;REQUIRE(readSourceInterval(assets,manifest,3000,3200,1000,silence,error));
    REQUIRE(std::all_of(silence.samples.begin(),silence.samples.end(),[](float v){return v==0;}));
    armCaptureFixture(*processor);head.position.setIsPlaying(true);position(4950);buffer.setSize(2,200);for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),200,0.125f);
    {AudioAllocationGuard guard;processor->processBlock(buffer,midi);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    for(int c=0;c<2;++c)for(int i=0;i<200;++i)REQUIRE(buffer.getSample(c,i)==0.125f);
    finishCaptureFixture(*processor,head.position);manifest=processor->getCaptureSnapshot().manifest;
    REQUIRE(manifest.ranges.back().startSample==4950 && manifest.ranges.back().frameCount==200);
    const auto count=engine.snapshot().renderCount;
    armCaptureFixture(*processor);head.position.setIsPlaying(true);position(0);buffer.setSize(2,250);buffer.clear();buffer.setSample(0,0,0.25f);buffer.setSample(1,0,-0.5f);
    processor->processBlock(buffer,midi);finishCaptureFixture(*processor,head.position);
    waitM3([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.snapshot().renderCount==count);
    armCaptureFixture(*processor);head.position.setIsPlaying(true);position(0);buffer.clear();buffer.setSample(0,0,0.75f);buffer.setSample(1,0,-0.5f);
    processor->processBlock(buffer,midi);finishCaptureFixture(*processor,head.position);
    // Learn Automatic Swells off now deliberately overrides legacy forced refresh.
    waitM3([&]{return m3::find(engine.savedSession(),first)->phase==m3::Phase::stale;});
    REQUIRE(engine.snapshot().renderCount==count);REQUIRE(engine.selectTargets({first},first));engine.generate(false);
    waitM3([&]{return engine.snapshot().auditionReady && engine.snapshot().renderCount>count;});
    REQUIRE(engine.savedSession().swells[1].render.path==savedModel.swells[1].render.path && processor->getDroppedCaptureBlockCount()==0);
    REQUIRE(original.moveFileTo(root.file.getChildFile("original-moved.wav")));
    auto restored=std::make_unique<PreverbAudioProcessor>(root.file);restored->prepareToPlay(1000,250);restored->setPlayHead(&head);
    restored->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));REQUIRE(restored->flushCaptureForTesting());
    REQUIRE(!restored->getSongTime().valid);
    REQUIRE(!restored->swellEngine().snapshot().auditionReady && !restored->getRecaptureSnapshot().active);
    head.position.setIsPlaying(false);position(0);buffer.clear();restored->processBlock(buffer,midi);
    waitM3([&]{return restored->swellEngine().snapshot().auditionReady;});auto state=restored->swellEngine().savedSession();
    REQUIRE(state.swells.size()==2 && state.primary==first && state.auditionDb==-24 && state.autoRegenerate);
    REQUIRE(!state.liveEnabled && state.liveRouting==m3::LiveRouting::wetOnly && state.liveWetDb==-7.5f);
    REQUIRE(state.swells[0].name=="First arrival" && state.swells[0].shape.gainDb==-3 && state.swells[0].irName=="m3-ir.wav");
    REQUIRE(state.swells[0].render.path==savedModel.swells[0].render.path && !restored->swellEngine().snapshot().playing && !restored->isMappingEnabled());
    const auto storeRoot=root.file.getChildFile(capture.manifest.projectId).getChildFile(capture.manifest.instanceId);
    REQUIRE(storeRoot.getChildFile(savedModel.swells[0].render.path).deleteFile());
    restored->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));REQUIRE(restored->flushCaptureForTesting());restored->processBlock(buffer,midi);
    waitM3([&]{return restored->swellEngine().snapshot().auditionReady;});
    REQUIRE(restored->swellEngine().savedSession().swells[0].render.path!=savedModel.swells[0].render.path);
    REQUIRE(restored->requestWorkflowMode(workflow::Mode::capture));restored->requestClearCapture();REQUIRE(restored->flushCaptureForTesting());REQUIRE(restored->getCaptureSnapshot().manifest.ranges.empty());
    restored->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));REQUIRE(restored->flushCaptureForTesting());restored->processBlock(buffer,midi);
    waitM3([&]{return restored->swellEngine().snapshot().auditionReady;});
    REQUIRE(restored->getCaptureSnapshot().reference==capture.reference);
    restored->prepareToPlay(2000,250);restored->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
    REQUIRE(restored->flushCaptureForTesting());restored->processBlock(buffer,midi);
    waitM3([&]{return restored->swellEngine().savedSession().swells[0].phase==m3::Phase::stale;});
    REQUIRE(!restored->swellEngine().snapshot().auditionReady && restored->swellEngine().savedSession().swells[0].sourceStart==0);
    restored->prepareToPlay(1000,250);restored->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
    REQUIRE(restored->flushCaptureForTesting());restored->processBlock(buffer,midi);waitM3([&]{return restored->swellEngine().snapshot().auditionReady;});
    // An actual old session payload migrates all edits as protected manual state.
    auto v3xml=std::make_unique<juce::XmlElement>(*xml);v3xml->setAttribute("stateVersion",3);
    auto v3child=v3xml->getChildByName("SwellSession");auto v3json=juce::JSON::parse(v3child->getAllSubText());
    v3json.getDynamicObject()->setProperty("version",1);v3child->deleteAllChildElements();v3child->addTextElement(juce::JSON::toString(v3json));
    juce::MemoryBlock v3;juce::AudioProcessor::copyXmlToBinary(*v3xml,v3);
    restored->setStateInformation(v3.getData(),static_cast<int>(v3.getSize()));
    REQUIRE(restored->isMappingEnabled() && !restored->swellEngine().savedSession().automaticProcessing);
    REQUIRE(restored->swellEngine().savedSession().swells[0].provenance==m3::Provenance::manualEdited);
    REQUIRE(restored->swellEngine().savedSession().auditionDb==-24 && !restored->swellEngine().snapshot().playing);
    REQUIRE(restored->swellEngine().savedSession().liveEnabled && restored->swellEngine().savedSession().liveWetDb==0);
    REQUIRE(restored->flushCaptureForTesting());restored->processBlock(buffer,midi);
    waitM3([&]{return restored->swellEngine().snapshot().auditionReady;});
    // Legacy v2 is still independently supported, without fabricating unsaved M2 swells.
    auto legacy=std::make_unique<juce::XmlElement>(*xml);legacy->setAttribute("stateVersion",2);legacy->deleteAllChildElements();
    juce::MemoryBlock v2;juce::AudioProcessor::copyXmlToBinary(*legacy,v2);
    REQUIRE(restored->requestWorkflowMode(workflow::Mode::live));REQUIRE(restored->swellEngine().audition());restored->setStateInformation(v2.getData(),static_cast<int>(v2.getSize()));
    REQUIRE(restored->flushCaptureForTesting());buffer.clear();
    {AudioAllocationGuard guard;restored->processBlock(buffer,midi);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && buffer.getMagnitude(0,250)==0);
    REQUIRE(restored->swellEngine().savedSession().swells.empty() && restored->swellEngine().savedSession().auditionDb==0);
    REQUIRE(restored->getCaptureSnapshot().reference==capture.reference);
}
