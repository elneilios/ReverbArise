#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Plugin/PluginEditor.h>

struct Host final:juce::AudioPlayHead
{
    juce::Optional<PositionInfo> position;
    juce::Optional<PositionInfo> getPosition()const override{return position;}
};
int main(int argc,char** argv){return runTest([&]{
    juce::ScopedJuceInitialiser_GUI gui;TestDirectory root;
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
    report=processor->compatibilityReport();REQUIRE(report.contains("Capture blocked: host time signature unavailable"));
    REQUIRE(report.contains("279.965 BPM - compare with your intended tempo"));
    REQUIRE(report.contains("signal received"));REQUIRE(report.contains("does not establish inconsistent sample/PPQ mapping"));
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
