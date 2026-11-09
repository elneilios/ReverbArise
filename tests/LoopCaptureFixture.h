#pragma once

#include "TestSupport.h"
#include <Plugin/PluginProcessor.h>

struct LoopCaptureFixture
{
    struct TimingHost : juce::AudioPlayHead
    {
        PositionInfo position;
        juce::Optional<PositionInfo> getPosition() const override {return position;}
    };
    explicit LoopCaptureFixture(int callbackFrames=128,double captureRate=48000,double declaredTempo=0)
        : processor(std::make_unique<PreverbAudioProcessor>(root.file)), frames(callbackFrames),
          rate(captureRate),tempo(declaredTempo>0?declaredTempo:frames==128?137:280),audio(2,frames)
    {
        if(declaredTempo>0)
        {
            host.position.setBpm(tempo);
            host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
            processor->setPlayHead(&host);
        }
        processor->prepareToPlay(rate,frames);
        auto preference=processor->getLoopCapturePreference();
        preference.enabled=true;
        REQUIRE(processor->setLoopCapturePreference(preference));
    }

    int64_t marker(int index) const noexcept {return 37+std::llround(rate*60*index/tempo);}
    static float signal(int64_t sample) noexcept {return sample%3==0?.25f:sample%3==1?.5f:.75f;}

    void tick(bool silent=false, bool reset=false)
    {
        midi.clear();
        const auto note=[&](int number,int offset)
        {midi.addEvent(juce::MidiMessage::noteOn(1,number,static_cast<juce::uint8>(100)),offset);};
        if (marker(nextBeat) < cursor+frames)
        {
            const auto offset=static_cast<int>(marker(nextBeat)-cursor);
            note(21,offset);
            if (nextBeat%4==0) note(19,offset);
            if (nextBeat%16==0) note(12,offset);
            ++nextBeat;
        }
        if (reset) note(22,0);
        for (int sample=0;sample<frames;++sample)
        {
            const auto at=cursor+sample;
            const auto value=silent || (percussive && at%18000>=1800)?0.f:signal(at);
            audio.setSample(0,sample,value);audio.setSample(1,sample,-value);
        }
        processor->processBlock(audio,midi);
        cursor+=frames;
    }

    void through(int64_t sample, bool paced=false, bool silent=false)
    {
        unsigned blocks=0;
        while (cursor<=sample)
        {
            tick(silent);
            if (paced && ++blocks%16==0) REQUIRE(processor->flushCaptureForTesting());
        }
        if (paced) REQUIRE(processor->flushCaptureForTesting());
    }

    TestDirectory root;
    TimingHost host;
    std::unique_ptr<PreverbAudioProcessor> processor;
    int frames;
    double rate,tempo;
    juce::AudioBuffer<float> audio;
    juce::MidiBuffer midi;
    int nextBeat=0;
    int64_t cursor=0;
    bool percussive=false;
};
