#pragma once
#include "M3ProcessorCases.h"
inline void automaticProcessorCases()
{
    TestDirectory root;auto processor=std::make_unique<PreverbAudioProcessor>(root.file);
    processor->swellEngine().setLivePlayback(false,m3::LiveRouting::dryWet,0);
    auto& engine=processor->swellEngine();engine.setAutoMarkers(false);REQUIRE(processor->isMappingEnabled());
    processor->prepareToPlay(48000,4096);TestPlayHead head;juce::MidiBuffer midi;
    head.position.setBpm(120);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});head.position.setIsPlaying(true);
    processor->setPlayHead(&head);juce::AudioBuffer<float> buffer(2,4096);
    const auto position=[&](int64_t sample){head.position.setTimeInSamples(sample);head.position.setPpqPosition(sample/24000.0);};
    const auto pass=[&](int64_t start,float scale=1)
    {
        if(processor->getWorkflowSnapshot().mode!=workflow::Mode::capture)REQUIRE(processor->requestWorkflowMode(workflow::Mode::capture));
        head.position.setIsPlaying(true);
        for(int block=0;block<24;++block)
        {
            const auto at=block*4096;position(start+at);buffer.clear();
            for(int i=0;i<4096;++i){const auto local=(at+i)%48000;if(local>=6000 && local<18000){const auto value=static_cast<float>(scale*.15*std::exp(-(local-6000)/2400.0)*std::sin(local*.17));buffer.setSample(0,i,value);buffer.setSample(1,i,value*.6f);}}
            const auto original=buffer;
            {AudioAllocationGuard guard;processor->processBlock(buffer,midi);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
            for(int c=0;c<2;++c)for(int i=0;i<4096;++i)REQUIRE(buffer.getSample(c,i)==original.getSample(c,i));
            juce::Thread::sleep(2);
        }
        head.position.setIsPlaying(false);buffer.clear();processor->processBlock(buffer,midi);
        REQUIRE(processor->flushCaptureForTesting());
    };
    // Song bar 86 at 120 BPM/4:4, then bar 1, without any plugin capture/generate action.
    pass(85*96000);pass(0);
    REQUIRE(engine.savedSession().swells.empty() && engine.savedSession().automaticProcessing && processor->getWorkflowSnapshot().mode==workflow::Mode::live);engine.setAutoMarkers(true);
    waitM3([&]{const auto s=engine.savedSession();return s.swells.size()>=4 && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
    auto s=engine.savedSession();REQUIRE(s.automaticProcessing && s.auditionDb==0);
    const auto map=processor->getCaptureSnapshot().manifest;
    REQUIRE(std::none_of(map.ranges.begin(),map.ranges.end(),[](const auto& r){return r.startSample<=1000000 && r.startSample+static_cast<int64_t>(r.frameCount)>1000000;}));
    const auto id=s.swells.back().id;REQUIRE(engine.selectTargets({id},id));waitM3([&]{return engine.snapshot().auditionReady;});
    const auto count=engine.snapshot().renderCount;const auto asset=find(s,id)->render.path;
    engine.setPostRoll(0); // This routing fixture captures short disjoint ranges; extended context is tested separately.
    engine.setMonitoring(m3::AuditionMode::dryWet,.35f);waitM3([&]{return engine.snapshot().auditionReady && engine.snapshot().dryAvailable;});
    REQUIRE(engine.audition());buffer.clear();
    armCaptureFixture(*processor);head.position.setIsPlaying(true);position(300000);
    {AudioAllocationGuard guard;processor->processBlock(buffer,midi);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    finishCaptureFixture(*processor,head.position);
    const auto feedbackMap=processor->getCaptureSnapshot().manifest;
    CaptureAssetStore feedbackStore(engine.assetDirectory());SourceInterval capturedZero;juce::String feedbackError;
    REQUIRE(readSourceInterval(feedbackStore,feedbackMap,300000,304096,4096,capturedZero,feedbackError));
    REQUIRE(std::all_of(capturedZero.samples.begin(),capturedZero.samples.end(),[](float v){return v==0;}));
    head.position.setIsPlaying(false);
    engine.setMonitoring(m3::AuditionMode::wetOnly,.35f);waitM3([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.savedSession().wetBlend==.35f);
    engine.setMonitoring(m3::AuditionMode::dryWet,.35f);waitM3([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.snapshot().renderCount==count && find(engine.savedSession(),id)->render.path==asset);
    engine.stop();engine.setPreferences(-3,.5f,.08,true);
    processor->setMappingEnabled(true);
    juce::MemoryBlock saved;processor->getStateInformation(saved);
    auto restored=std::make_unique<PreverbAudioProcessor>(root.file);restored->prepareToPlay(48000,4096);restored->setPlayHead(&head);
    restored->setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));REQUIRE(restored->flushCaptureForTesting());
    REQUIRE(!restored->swellEngine().snapshot().playing && restored->isMappingEnabled());
    restored->processBlock(buffer,midi);waitM3([&]{return restored->swellEngine().snapshot().auditionReady;});
    const auto preferences=restored->swellEngine().savedSession();
    REQUIRE(preferences.primary==id && preferences.auditionMode==m3::AuditionMode::dryWet && preferences.wetBlend==.35f && preferences.auditionDb==-3);
    // Missing dry is explicit, but a separately verified committed wet remains recoverable.
    restored.reset();engine.stop();
    const auto directory=engine.assetDirectory();
    for(const auto& r:map.ranges)if(!r.assetPath.empty())REQUIRE(directory.getChildFile(r.assetPath).deleteFile());
    auto recovery=std::make_unique<PreverbAudioProcessor>(root.file);recovery->prepareToPlay(48000,4096);recovery->setPlayHead(&head);
    auto recoveryXml=juce::AudioProcessor::getXmlFromBinary(saved.getData(),static_cast<int>(saved.getSize()));
    recoveryXml->setAttribute("workflowMode","Live");juce::MemoryBlock recoveryBytes;juce::AudioProcessor::copyXmlToBinary(*recoveryXml,recoveryBytes);
    recovery->setStateInformation(recoveryBytes.getData(),static_cast<int>(recoveryBytes.getSize()));REQUIRE(recovery->flushCaptureForTesting());recovery->processBlock(buffer,midi);
    waitM3([&]{return recovery->swellEngine().snapshot().cachedWetOnly;});
    REQUIRE(!recovery->swellEngine().snapshot().auditionReady && !recovery->swellEngine().snapshot().dryAvailable);
    recovery->swellEngine().setMonitoring(m3::AuditionMode::wetOnly,.35f);
    waitM3([&]{return recovery->swellEngine().snapshot().auditionReady;});
    REQUIRE(find(recovery->swellEngine().savedSession(),id)->phase==m3::Phase::error);
    REQUIRE(recovery->swellEngine().audition());
}
