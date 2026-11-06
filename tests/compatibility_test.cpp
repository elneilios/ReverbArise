#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Plugin/PluginEditor.h>

struct Host final:juce::AudioPlayHead
{
    juce::Optional<PositionInfo> position;
    juce::Optional<PositionInfo> getPosition()const override{return position;}
};
static void rateRecheckCases()
{
    TestDirectory root;auto p=std::make_unique<PreverbAudioProcessor>(root.file);Host host;
    p->setPlayHead(&host);p->prepareToPlay(48000,128);p->swellEngine().setLearningAutomatic(false);
    REQUIRE(!p->recheckCompatibilityRate()); // No transport evidence yet.
    juce::AudioPlayHead::PositionInfo info;info.setBpm(120);info.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    juce::AudioBuffer<float> audio(2,128);juce::MidiBuffer midi;
    const auto process=[&](int64_t at,bool playing){info.setIsPlaying(playing);info.setTimeInSamples(at);info.setPpqPosition(double(at)/24000);host.position=info;
        for(int c=0;c<2;++c)std::fill_n(audio.getWritePointer(c),128,.25f);
        {AudioAllocationGuard guard;p->processBlock(audio,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);};
    process(0,true);REQUIRE(!p->recheckCompatibilityRate());process(128,false);REQUIRE(p->flushCaptureForTesting());
    const auto source=p->getCaptureSnapshot();const auto pass=p->getWorkflowSnapshot().pass;
    juce::MemoryBlock stateBefore;p->getStateInformation(stateBefore);
    REQUIRE(source.manifest.sampleRate==48000 && !source.manifest.ranges.empty());
    p->prepareToPlay(44100,8192);process(128,false);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->compatibilityReport().contains("48000.000 Hz saved | host 44100.000 Hz"));
    REQUIRE(p->recheckCompatibilityRate());REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->compatibilityReport().contains("Saved and host rates differ"));
    REQUIRE(p->compatibilityReport().contains("mapping currently blocked"));
    p->prepareToPlay(48000,128);process(128,false);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->compatibilityReport().contains("stale-rate latch: set"));
    REQUIRE(p->recheckCompatibilityRate());REQUIRE(p->flushCaptureForTesting());
    auto report=p->compatibilityReport();REQUIRE(report.contains("Rates matched at recheck; stale-rate latch released"));
    REQUIRE(report.contains("mapping currently permitted"));REQUIRE(report.contains("44100.000 Hz, max block 8192"));
    REQUIRE(report.contains("saved 48000.000 Hz / host 44100.000 Hz"));
    REQUIRE(p->getCaptureSnapshot().reference==source.reference && p->getCaptureSnapshot().manifest.revision==source.manifest.revision);
    REQUIRE(p->getWorkflowSnapshot().pass==pass && p->getWorkflowSnapshot().phase==workflow::Phase::idle);
    juce::MemoryBlock stateAfter;p->getStateInformation(stateAfter);REQUIRE(stateBefore==stateAfter);
    CaptureSession original(root.file);REQUIRE(original.restore(source.manifest.projectId,source.manifest.instanceId,source.reference,source.manifest.revision));
    float left=0,right=0;REQUIRE(original.readSample(0,left,right) && left==.25f && right==.25f);
    process(256,true);REQUIRE(p->getWorkflowSnapshot().phase==workflow::Phase::recording);
    process(384,false);REQUIRE(p->flushCaptureForTesting());REQUIRE(p->getWorkflowSnapshot().result==workflow::Result::accepted);
    p->clearCompatibilityReport();REQUIRE(p->compatibilityReport().contains("prepare #2: 44100.000")); // Preserve lifetime setup evidence.
    compatibility::RateHistory bounded;for(int i=0;i<40;++i)bounded.prepared(48000+i,128);
    REQUIRE(bounded.report().contains("40 calls; older entries omitted=24"));REQUIRE(!bounded.report().contains("prepare #1:"));
    REQUIRE(bounded.report().contains("prepare #40:"));
    compatibility::RateHistory concurrent;std::atomic<bool> done=false;bool reportsBounded=true;
    std::thread writer([&]{for(int i=0;i<4000;++i)concurrent.prepared(48000+i,128);done.store(true);});
    while(!done.load()){if(concurrent.report().length()>4096)reportsBounded=false;}
    writer.join();REQUIRE(reportsBounded);REQUIRE(concurrent.report().contains("4000 calls; older entries omitted=3984"));
    std::cout<<"Rate recheck: real mismatch stays blocked; returned matching rate releases latch; source/identity preserved; subsequent capture accepted; setup history bounded\n";
}
#if PREVERB_COMPATIBILITY_METER_4_4
static void fixedMeterExperiment()
{
    TestDirectory root;auto p=std::make_unique<PreverbAudioProcessor>(root.file);Host host;
    p->prepareToPlay(48000,64);p->setPlayHead(&host);p->swellEngine().setLearningAutomatic(false);
    juce::AudioPlayHead::PositionInfo info;info.setIsPlaying(true);info.setBpm(279.965);
    juce::AudioBuffer<float> audio(2,64);juce::MidiBuffer midi;
    const auto process=[&](int64_t at,double ppq,bool playing){info.setTimeInSamples(at);info.setPpqPosition(ppq);info.setIsPlaying(playing);host.position=info;
        for(int c=0;c<2;++c)std::fill_n(audio.getWritePointer(c),64,.25f);
        {AudioAllocationGuard guard;p->processBlock(audio,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);};
    process(0,0,true);REQUIRE(p->getWorkflowSnapshot().phase==workflow::Phase::recording);
    REQUIRE(p->getSongTime().numerator==4 && p->getSongTime().denominator==4);
    process(64,64.0*279.965/(48000*60),true);process(128,128.0*279.965/(48000*60),false);
    REQUIRE(p->flushCaptureForTesting());REQUIRE(p->getWorkflowSnapshot().result==workflow::Result::accepted);
    const auto committed=p->getCaptureSnapshot();REQUIRE(!committed.manifest.ranges.empty());
    REQUIRE(p->compatibilityReport().contains("raw host meter=unavailable"));
    // Exact consecutive observations from Stuart's report: PPQ advances about
    // 128 samples while the host sample position repeats. Keep this guard.
    info.setBpm(280.100547);process(329095,31.994458,true);
    process(329095,32.006907,true);
    process(329223,32.019356,false);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getWorkflowSnapshot().result==workflow::Result::failed);
    REQUIRE(p->getCaptureSnapshot().failure.event.reason==CaptureFailureReason::timingChanged);
    REQUIRE(p->getCaptureSnapshot().reference==committed.reference);
    info.setTimeSignature(juce::AudioPlayHead::TimeSignature{3,4});process(2000,2,false);
    REQUIRE(p->getSongTime().numerator==3 && p->getSongTime().denominator==4);
    info.setTimeSignature(juce::AudioPlayHead::TimeSignature{0,0});process(2000,2,false);
    REQUIRE(p->getSongTime().numerator==0); // Supplied invalid meter is not silently replaced.
}
#endif
int main(int argc,char** argv){return runTest([&]{
    juce::ScopedJuceInitialiser_GUI gui;TestDirectory root;
    rateRecheckCases();
#if PREVERB_COMPATIBILITY_METER_4_4
    fixedMeterExperiment();
#endif
    auto processor=std::make_unique<PreverbAudioProcessor>(root.file);
    Host host;processor->setPlayHead(&host);processor->prepareToPlay(48000,64);
    juce::AudioBuffer<float> audio(2,64);juce::MidiBuffer midi;
    const auto process=[&](float value){for(int c=0;c<2;++c)std::fill_n(audio.getWritePointer(c),64,value);
        {AudioAllocationGuard guard;processor->processBlock(audio,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);};
    REQUIRE(processor->compatibilityReport().contains("No audio callbacks"));
    process(0);REQUIRE(processor->compatibilityReport().contains("silent in latest"));
    process(.25f);auto report=processor->compatibilityReport();REQUIRE(report.contains("signal received"));REQUIRE(report.contains("host position information is missing"));
    REQUIRE(processor->getWorkflowSnapshot().phase==workflow::Phase::idle);
    juce::AudioPlayHead::PositionInfo info;info.setIsPlaying(true);info.setTimeInSeconds(.25);host.position=info;
    process(.5f);REQUIRE(processor->compatibilityReport().contains("host sample position is missing"));
    REQUIRE(processor->compatibilityReport().contains("seconds=0.250000")); // Observation only; never a fallback.
    info.setTimeInSamples(0);host.position=info;process(.5f);REQUIRE(processor->compatibilityReport().contains("host musical position is missing"));
    info.setPpqPosition(0);host.position=info;process(.5f);REQUIRE(processor->compatibilityReport().contains("host tempo is missing"));
    info.setBpm(279.965);host.position=info;process(.5f);
    report=processor->compatibilityReport();
#if PREVERB_COMPATIBILITY_METER_4_4
    REQUIRE(report.contains("EXPERIMENT: fixed 4/4 supplied ONLY"));REQUIRE(processor->getSongTime().valid);
#else
    REQUIRE(report.contains("Capture blocked: host time signature unavailable"));
    REQUIRE(report.contains("does not establish inconsistent sample/PPQ mapping"));
#endif
    REQUIRE(report.contains("279.965 BPM - compare with your intended tempo"));
    REQUIRE(report.contains("signal received"));
    REQUIRE(processor->getWorkflowSnapshot().phase==workflow::Phase::idle);
    REQUIRE(processor->getDroppedCaptureBlockCount()==0);REQUIRE(processor->getCaptureSnapshot().manifest.revision==0);
    if(argc>1 && juce::String(argv[1])=="--snapshot"){
        auto blockedEditor=std::make_unique<PreverbAudioProcessorEditor>(*processor);
        juce::MessageManager::getInstance()->runDispatchLoopUntil(80);
        auto* w=blockedEditor->compatibilityWindowForTesting();REQUIRE(w && w->isVisible());
        juce::FileOutputStream output(juce::File::getCurrentWorkingDirectory().getChildFile("out/compatibility-window.png"));
        REQUIRE(output.openedOk() && output.setPosition(0));REQUIRE(output.truncate().wasOk());
        juce::PNGImageFormat png;REQUIRE(png.writeImageToStream(w->createComponentSnapshot(w->getLocalBounds()),output));
    }
    info.setBpm(120);
    info.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});info.setTimeInSeconds(0);info.setIsPlaying(false);host.position=info;process(.5f);
    info.setIsPlaying(true);host.position=info;process(.5f);REQUIRE(processor->getWorkflowSnapshot().phase==workflow::Phase::recording);
    info.setTimeInSamples(64);info.setPpqPosition(64.0/24000);info.setTimeInSeconds(64.0/48000);host.position=info;process(.5f);
    info.setIsPlaying(false);host.position=info;process(0);REQUIRE(processor->flushCaptureForTesting());
    REQUIRE(processor->compatibilityReport().contains("Capture completed successfully"));
    REQUIRE(!processor->compatibilityTelemetry.settlementHistory.empty());
    const auto regions=processor->getCapturedRegionCount();REQUIRE(regions>0);
    info.setIsPlaying(true);host.position=info;process(.5f);
    info.setBpm(130);host.position=info;process(.5f);
    info.setIsPlaying(false);host.position=info;process(0);REQUIRE(processor->flushCaptureForTesting());
    REQUIRE(processor->compatibilityReport().contains("Host timing changed during capture"));
    REQUIRE(processor->getCapturedRegionCount()==regions);
    {
        auto second=std::make_unique<PreverbAudioProcessor>(root.file);
        REQUIRE(second->compatibilityReport().contains("No audio callbacks"));
    }
    auto editor=std::make_unique<PreverbAudioProcessorEditor>(*processor);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(80);
    auto* window=editor->compatibilityWindowForTesting();REQUIRE(window && window->isVisible() && !window->isCurrentlyModal());
    window->recheckRate();REQUIRE(window->feedbackText().contains("Recheck requested"));REQUIRE(processor->flushCaptureForTesting());
    editor->showCompatibility();REQUIRE(editor->compatibilityWindowForTesting()==window);
    window->copyReport();REQUIRE(window->feedbackText().contains("Report copied"));
    REQUIRE(juce::SystemClipboard::getTextFromClipboard()==processor->compatibilityReport());
    window->closeButtonPressed();REQUIRE(!window->isVisible());editor->showCompatibility();REQUIRE(window->isVisible());
    const auto history=processor->compatibilityTelemetry.history.size();juce::Component::SafePointer<CompatibilityWindow> lifetime(window);editor.reset();REQUIRE(!lifetime);
    editor=std::make_unique<PreverbAudioProcessorEditor>(*processor);juce::MessageManager::getInstance()->runDispatchLoopUntil(80);
    REQUIRE(editor->compatibilityWindowForTesting()->isVisible());REQUIRE(processor->compatibilityTelemetry.history.size()==history);
    editor->compatibilityWindowForTesting()->clearReport();REQUIRE(editor->compatibilityWindowForTesting()->feedbackText().contains("Report cleared"));
    editor.reset(); // Pending automatic-show callback must not resurrect a destroyed editor.
    {auto pending=std::make_unique<PreverbAudioProcessorEditor>(*processor);}
    juce::MessageManager::getInstance()->runDispatchLoopUntil(50);
    processor->clearCompatibilityReport();REQUIRE(processor->compatibilityTelemetry.history.empty());REQUIRE(processor->getCapturedRegionCount()==regions);
    REQUIRE(processor->compatibilityReport().contains("since Clear=0"));
    compatibility::Telemetry bounded;compatibility::Observation o;o.timing.position=true;
    for(unsigned i=0;i<2000;++i){o.nonzero=(i%2)!=0;bounded.publish(o);}
    REQUIRE(bounded.omitted.load()>0);bounded.drain();REQUIRE(bounded.history.size()==bounded.historyCapacity);REQUIRE(bounded.truncated==128);
    bounded.clear();REQUIRE(bounded.history.empty());REQUIRE(bounded.omittedBaseline==bounded.omitted.load());
    for(unsigned i=0;i<2000;++i)bounded.settled(i,true);bounded.drain();
    REQUIRE(bounded.settlementHistory.size()==32);REQUIRE(bounded.settlementOmitted.load()==1936);REQUIRE(bounded.settlementTruncated==32);
    bounded.clear();REQUIRE(bounded.settlementHistory.empty());
    compatibility::Telemetry coalesced;for(unsigned i=0;i<10000;++i)coalesced.publish(o);coalesced.drain();
    REQUIRE(coalesced.callbackCount.load()==10000);REQUIRE(coalesced.history.size()<10);
    std::cout<<"Native compatibility: blocked starts, raw input, success/rejection, realtime allocation, clipboard, clear, instances, reopen, pending destruction, bounded/coalesced storage passed\n";
});}
