#pragma once
#include "TestSupport.h"
#include <Plugin/PluginProcessor.h>

// Legacy DSP fixtures need committed source, not progressive chunk publication.
// Report a real host Stop using a separate buffer so their output oracle stays
// intact. The workflow-specific tests separately assert automatic transitions.
inline void finishCaptureFixture(PreverbAudioProcessor& processor,juce::AudioPlayHead::PositionInfo& position,bool generateSwells=false)
{
    position.setIsPlaying(false);juce::AudioBuffer<float> stopped(2,1);stopped.clear();juce::MidiBuffer midi;
    processor.processBlock(stopped,midi);REQUIRE(processor.flushCaptureForTesting());
    if(generateSwells && processor.getWorkflowSnapshot().mode==workflow::Mode::capture)
    {
        auto& engine=processor.swellEngine();const auto deadline=juce::Time::getMillisecondCounter()+30000;
        while(!engine.snapshot().learnGeneration.canGenerate)
        {REQUIRE(juce::Time::getMillisecondCounter()<deadline);juce::Thread::sleep(2);}
        const auto snapshot=engine.snapshot();
        REQUIRE(engine.generateLearn(snapshot.sessionEpoch,processor.getWorkflowSnapshot().pass,snapshot.learnGeneration.displayed->serial));
        while(engine.snapshot().learnGeneration.generating)
        {REQUIRE(juce::Time::getMillisecondCounter()<deadline);juce::Thread::sleep(2);}
    }
    if(processor.getWorkflowSnapshot().mode==workflow::Mode::capture && processor.getWorkflowReadiness().usefulSource)
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));
}
inline void armCaptureFixture(PreverbAudioProcessor& processor)
{REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));REQUIRE(processor.setRecaptureExisting(true));}
