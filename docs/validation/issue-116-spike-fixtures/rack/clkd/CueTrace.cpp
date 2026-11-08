#include "../Processor.h"
#include <array>
#include <atomic>
#include <cmath>

namespace
{
constexpr int lead=2048, fadeLength=240, rate=48000, limit=96*rate, maxPeriod=400000;
enum Flags : uint32_t
{
    phrase=1, bar=2, cue=4, stop=8, resetSignal=16, locked=32, scheduled=64,
    preRoll=128, downbeat=256, lost=512, fading=1024
};
struct Sample { float dryLeft,dryRight,outLeft,outRight; uint32_t flags; };
static_assert(sizeof(Sample)==20);

class CueTrace final : public ProbeProcessor, private juce::Timer
{
public:
    CueTrace() : ProbeProcessor(BusesProperties()
        .withInput("Dry",juce::AudioChannelSet::stereo(),true)
        .withOutput("Dry and wet",juce::AudioChannelSet::stereo(),true)),
        captured(maxPeriod), trace(limit)
    {
        const juce::File folder(TRACE_DIRECTORY);
        folder.createDirectory();
        file=folder.getChildFile("clkd-cue-"+juce::Uuid().toString()+".f32");
        startTimerHz(10);
    }

    ~CueTrace() override { stopTimer(); timerCallback(); }

    void prepareToPlay(double,int) override { resetRequested=true; }
    void releaseResources() override { resetRequested=true; }
    void reset() override { resetRequested=true; }

    void processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi) override
    {
        if (audio.getNumChannels()<2 || getSampleRate()!=rate) return;
        callback.store(audio.getNumSamples(),std::memory_order_relaxed);
        if (resetRequested.exchange(false))
            clearSync();

        auto next=midi.cbegin();
        const auto end=midi.cend();
        for (int offset=0;offset<audio.getNumSamples();++offset,++position)
        {
            uint32_t flags=0;
            while (next!=end && (*next).samplePosition<=offset)
            {
                const auto event=*next++;
                if (event.samplePosition!=offset || event.numBytes!=3
                    || (event.data[0]&0xf0)!=0x90 || event.data[2]==0) continue;
                switch (event.data[1])
                {
                    case 12: flags|=phrase; break;
                    case 19: flags|=bar; break;
                    case 16: flags|=cue; break;
                    case 17: flags|=stop; break;
                    case 22: flags|=resetSignal; break;
                }
            }

            if ((flags&resetSignal) || (anchor>=0 && interval>0 && position-anchor>interval+1))
            {
                if (matches>=2) flags|=lost;
                clearSync();
            }
            const bool boundary=(flags&(phrase|bar))==(phrase|bar);
            if (boundary)
            {
                if (capturedFrames.load(std::memory_order_relaxed)==0 && captureStart>=0)
                {
                    const int size=position-captureStart;
                    if (size>lead && size<=maxPeriod)
                        capturedFrames.store(size,std::memory_order_release);
                }
                if (capturedFrames.load(std::memory_order_relaxed)==0)
                    captureStart=position;

                if (anchor>=0)
                {
                    const int previous=interval;
                    interval=position-anchor;
                    const int recorded=capturedFrames.load(std::memory_order_relaxed);
                    if (std::abs(interval-recorded)<=1 && (!previous || std::abs(interval-previous)<=1))
                        matches=std::min(2,matches+1);
                    else
                    {
                        if (matches>=2) flags|=lost;
                        stopWet();
                        matches=0;
                    }
                }
                anchor=position;
                if (launching && leadStart>=0 && position-leadStart>=lead)
                {
                    launching=false;
                    repeating=true;
                }
                if (repeating) flags|=downbeat;
                if (launching || repeating)
                    leadStart=position+interval-1-lead;
            }

            if (flags&stop)
                stopWet();
            else if ((flags&cue) && matches>=2 && wetReady.load(std::memory_order_acquire)
                     && !launching && !repeating && fade==0)
            {
                launching=true;
                const int earliest=anchor+interval-1-lead;
                leadStart=position<=earliest?earliest:-1;
            }
            if (matches>=2) flags|=locked;
            if (launching) flags|=scheduled;
            const float dryLeft=audio.getSample(0,offset);
            const float dryRight=audio.getSample(1,offset);
            if (captureStart>=0 && capturedFrames.load(std::memory_order_relaxed)==0 && position-captureStart<maxPeriod)
                captured[static_cast<size_t>(position-captureStart)]=dryLeft;

            float wet=0;
            if (fade)
            {
                flags|=fading;
                wet=fadeFrom*static_cast<float>(fade--)/fadeLength;
            }
            else if (leadStart>=0 && position>=leadStart && (launching || repeating))
            {
                flags|=preRoll;
                wet=negative[static_cast<size_t>(std::min(position-leadStart,lead-1))];
            }
            else if (repeating && position-anchor<static_cast<int>(positive.size()))
                wet=positive[static_cast<size_t>(position-anchor)];

            lastWet=wet;
            audio.setSample(0,offset,dryLeft+wet);
            if (position<limit)
            {
                trace[static_cast<size_t>(position)]={dryLeft,dryRight,dryLeft+wet,dryRight,flags};
                if (position==limit-1) complete.store(true,std::memory_order_release);
            }
        }
        progress.store(position,std::memory_order_relaxed);
        midi.clear();
    }

private:
    void stopWet() noexcept
    {
        if ((launching || repeating) && lastWet!=0)
        {
            fadeFrom=lastWet;
            fade=fadeLength;
        }
        launching=repeating=false;
        leadStart=-1;
    }

    void clearSync() noexcept
    {
        stopWet();
        anchor=-1;
        interval=matches=0;
        if (!capturedFrames.load(std::memory_order_relaxed)) captureStart=-1;
    }

    void timerCallback() override
    {
        const auto observed=progress.load(std::memory_order_relaxed);
        if (observed-lastProgress>=rate)
        {
            lastProgress=observed;
            file.withFileExtension(".progress").replaceWithText(juce::String(observed)+" "+juce::String(callback.load()));
        }
        const auto size=capturedFrames.load(std::memory_order_acquire);
        if (size && !wetReady.load(std::memory_order_relaxed))
        {
            for (int i=0;i<lead;++i)
                negative[static_cast<size_t>(i)]=captured[static_cast<size_t>(lead-1-i)]*.25f;
            for (size_t i=0;i<positive.size();++i)
                positive[i]=captured[i]*.25f;
            wetReady.store(true,std::memory_order_release);
        }
        if (complete.load(std::memory_order_acquire) && !written)
        {
            written=true;
            if (file.replaceWithData(trace.data(),trace.size()*sizeof(Sample)))
                file.withFileExtension(".txt").replaceWithText(juce::String(callback.load())+" "+juce::String(size));
        }
    }

    std::vector<float> captured;
    std::vector<Sample> trace;
    std::array<float,lead> negative{};
    std::array<float,256> positive{};
    juce::File file;
    std::atomic<int> capturedFrames{0},callback{0},progress{0};
    int lastProgress = -rate;
    std::atomic<bool> wetReady{false},complete{false},resetRequested{true};
    bool launching=false,repeating=false,written=false;
    int position=0,anchor=-1,interval=0,captureStart=-1,leadStart=-1,matches=0,fade=0;
    float lastWet=0,fadeFrom=0;
};
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CueTrace; }
