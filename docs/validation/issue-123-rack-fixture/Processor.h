#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class ProbeProcessor : public juce::AudioProcessor
{
public:
    explicit ProbeProcessor(const BusesProperties& properties) : AudioProcessor(properties) {}
    const juce::String getName() const override { return JucePlugin_Name; }
    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    double getTailLengthSeconds() const override { return 0; }
    bool acceptsMidi() const override { return JucePlugin_WantsMidiInput; }
    bool producesMidi() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*,int) override {}
};
