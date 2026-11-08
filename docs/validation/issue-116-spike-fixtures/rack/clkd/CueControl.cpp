#include "../Processor.h"

class CueControl final : public ProbeProcessor
{
public:
    CueControl() : ProbeProcessor(BusesProperties()
        .withInput("Unused",juce::AudioChannelSet::mono(),true)
        .withOutput("Controls",juce::AudioChannelSet::octagonal(),true)) {}

    void releaseResources() override {}
    void prepareToPlay(double,int) override { frame=0; }
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        buffer.clear();
        if (buffer.getNumChannels()<8 || getSampleRate()!=48000)
            return;
        for (int i=0;i<buffer.getNumSamples();++i,++frame)
        {
            const auto pulse=[&](int sec) { return frame>=48000*sec && frame<48000*sec+48; };
            buffer.setSample(0,i,pulse(36)?1.0f:0.0f);
            buffer.setSample(1,i,frame<61*48000 || frame>=68*48000?1.0f:0.0f);
            buffer.setSample(2,i,pulse(15) || pulse(23) || pulse(52) || pulse(87)?1.0f:0.0f);
            buffer.setSample(3,i,pulse(94)?1.0f:0.0f);
        }
    }
private:
    int frame=0;
};
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CueControl; }
