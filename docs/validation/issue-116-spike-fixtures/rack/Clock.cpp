#include "Processor.h"
#include <atomic>
#include <cmath>

// The odd period walks the reference across callback offsets. All outputs
// derive from this one sample counter and reset on the same input edge.
class FixtureClock final : public ProbeProcessor
{
public:
    FixtureClock() : ProbeProcessor(BusesProperties()
        .withInput("Reset",juce::AudioChannelSet::mono(),true)
        .withOutput("Fixture",juce::AudioChannelSet::octagonal(),true)) {}

    void prepareToPlay(double,int) override { resetRequested=true; }
    void reset() override { resetRequested=true; }
    void releaseResources() override { resetRequested=true; }

    void processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer&) override
    {
        if (audio.getNumChannels()<8)
        {
            audio.clear();
            return;
        }
        if (resetRequested.exchange(false))
        {
            frame=0;
            resetHigh=false;
        }
        constexpr int period=49153;
        for (int i=0;i<audio.getNumSamples();++i)
        {
            const auto resetInput=audio.getSample(0,i)>0.2f;
            if (resetInput && !resetHigh)
                frame=0;
            resetHigh=resetInput;
            const auto cycle=frame/period;
            const auto phase=static_cast<int>(frame%period);
            const auto cue=phase==97 || (phase>=193 && phase<241)
                || (phase>=4099 && phase<4147) || (phase>=period-1024 && phase<period-976);
            const auto stop=(phase>=4099 && phase<4147) || phase==8197;
            audio.setSample(0,i,cycle%2==0 ? 0.8f : 0.0f);
#if POLICY_CLOCK
            const bool phrase = cycle != 8 && (cycle == 14 ? phase>=1 && phase<49 : phase<48);
            audio.setSample(1,i,phrase ? 0.8f : 0.0f);
#else
            audio.setSample(1,i,phase<48 ? 0.8f : 0.0f);
#endif
            audio.setSample(2,i,cue ? 0.8f : 0.0f);
            audio.setSample(3,i,stop ? 0.8f : 0.0f);
            const auto tone=static_cast<float>(std::sin(phase*0.0575958653));
            audio.setSample(4,i,phase<48 ? 0.6f : 0.1f*tone);
            audio.setSample(5,i,phase<48 ? 0.5f : -0.08f*tone);
#if POLICY_CLOCK
            const bool testCue = ((cycle==2 || cycle==9 || cycle==11 || cycle==13) && phase==97)
                || (cycle==5 && phase==period-1024) || (cycle==12 && phase==512);
            const bool testStop = (cycle==5 || cycle==12) && phase==512;
            audio.setSample(6,i,testCue ? 0.8f : 0.0f);
            audio.setSample(7,i,testStop ? 0.8f : 0.0f);
#else
            audio.setSample(6,i,0);
            audio.setSample(7,i,0);
#endif
            ++frame;
        }
    }

private:
    std::atomic<bool> resetRequested{true};
    int64_t frame=0;
    bool resetHigh=false;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new FixtureClock; }
