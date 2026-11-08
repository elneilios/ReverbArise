#include "Processor.h"
#include <array>
#include <atomic>

namespace
{
constexpr int period=49153, lead=2048, fadeLength=240;
constexpr int recordLimit=19*period;
enum Flags : uint32_t
{
    marker=1, cue=2, stop=4, locked=8, scheduled=16, preRoll=32, playing=64, fading=128
};
struct Sample
{
    float dryLeft, dryRight, outLeft, outRight;
    uint32_t flags;
};
static_assert(sizeof(Sample)==20);

class PolicyProbe final : public ProbeProcessor, private juce::Timer
{
public:
    PolicyProbe() : ProbeProcessor(BusesProperties()
        .withInput("Dry",juce::AudioChannelSet::stereo(),true)
        .withOutput("Dry and prepared wet",juce::AudioChannelSet::stereo(),true)),
        recording(static_cast<size_t>(recordLimit))
    {
        const juce::File folder(TRACE_DIRECTORY);
        juce::MemoryBlock data;
        if (folder.getParentDirectory().getChildFile("prepared-wet.f32").loadFileAsData(data)
            && data.getSize()==wet.size()*sizeof(float))
        {
            std::memcpy(wet.data(),data.getData(),data.getSize());
            ready=true;
        }
        folder.createDirectory();
        file=folder.getChildFile("policy-"+juce::Uuid().toString()+".f32");
        startTimerHz(5);
    }
    ~PolicyProbe() override { stopTimer(); timerCallback(); }
    void prepareToPlay(double,int) override { invalidated=true; }
    void reset() override { invalidated=true; }
    void releaseResources() override { invalidated=true; }

    void processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi) override
    {
        if (audio.getNumChannels()<2 || getSampleRate()!=48000 || !ready)
            return;
        if (invalidated.exchange(false))
        {
            anchor=-1;
            launch=-1;
            fadeStart=-1;
            matching=0;
        }
        callbackSize.store(audio.getNumSamples(),std::memory_order_relaxed);
        auto next=midi.begin();
        const auto end=midi.end();
        for (int offset=0;offset<audio.getNumSamples();++offset,++position)
        {
            uint32_t flags=0;
            while (next!=end)
            {
                const auto event=*next;
                if (event.samplePosition>offset)
                    break;
                if (event.samplePosition==offset && event.numBytes==3
                    && (event.data[0]&0xf0)==0x90 && event.data[2]>0)
                {
                    if (event.data[1]==12) flags|=marker;
                    if (event.data[1]==16) flags|=cue;
                    if (event.data[1]==17) flags|=stop;
                }
                ++next;
            }
            if (anchor>=0 && position==anchor+period && !(flags&marker))
            {
                stopWet();
                anchor=-1;
                matching=0;
            }
            if (flags&marker)
            {
                if (anchor>=0 && position-anchor==period)
                    matching=std::min(matching+1,2);
                else
                {
                    stopWet();
                    matching=0;
                }
                anchor=position;
            }
            if (flags&stop)
                stopWet();
            else if ((flags&cue) && matching>=2 && launch<0)
            {
                const auto offsetToDeadline=position+lead-anchor;
                launch=anchor+std::max<int64_t>(1,(offsetToDeadline+period-1)/period)*period;
            }
            if (fadeStart>=0 && position>=fadeStart+fadeLength)
            {
                launch=-1;
                fadeStart=-1;
            }
            if (matching>=2) flags|=locked;
            if (launch>=0)
                flags|=position<launch-lead ? scheduled : position<launch ? preRoll : playing;
            if (fadeStart>=0) flags|=fading;

            const float left=audio.getSample(0,offset),right=audio.getSample(1,offset);
            const float mixedLeft=left+wetAt(0),mixedRight=right+wetAt(1);
            audio.setSample(0,offset,mixedLeft);
            audio.setSample(1,offset,mixedRight);
            if (position<recordLimit)
            {
                recording[static_cast<size_t>(position)]={left,right,mixedLeft,mixedRight,flags};
                if (position==recordLimit-1)
                    complete.store(true,std::memory_order_release);
            }
        }
        midi.clear();
    }

private:
    void stopWet() noexcept
    {
        if (launch<0 || fadeStart>=0)
            return;
        if (position<launch-lead)
            launch=-1;
        else
            fadeStart=position;
    }

    float wetAt(int channel) const noexcept
    {
        if (launch<0 || position<launch-lead)
            return 0;
        const auto sample=[&](int64_t index)
        { return wet[static_cast<size_t>(index*2+channel)]; };
        const auto local=position-launch;
        float value=0;
        if (local<0)
            value=sample(local+lead);
        else
        {
            const auto phase=local%period;
            if (phase<lead) value+=sample(phase+lead);
            if (phase>=period-lead) value+=sample(phase-(period-lead));
        }
        if (fadeStart>=0)
            value*=1-static_cast<float>(position-fadeStart)/fadeLength;
        return value;
    }

    void timerCallback() override
    {
        if (complete.load(std::memory_order_acquire) && !written)
        {
            written=true;
            if (file.replaceWithData(recording.data(),recording.size()*sizeof(Sample)))
                file.withFileExtension(".txt").replaceWithText(juce::String(callbackSize.load()));
        }
    }
    std::array<float,4*lead> wet{};
    std::vector<Sample> recording;
    juce::File file;
    std::atomic<bool> invalidated{true},complete{false};
    std::atomic<int> callbackSize{0};
    bool ready=false,written=false;
    int matching=0;
    int64_t position=0,anchor=-1,launch=-1,fadeStart=-1;
};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PolicyProbe; }
