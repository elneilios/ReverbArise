#include "Processor.h"
#include <array>
#include <atomic>

namespace
{
struct Event
{
    int64_t sample=0, blockStart=0;
    int frames=0, rate=0;
    const char* stream="";
    int value=0, note=-1, channel=0;
};
constexpr int capacity=4096, limit=120000, maxMidiEvents=128;
constexpr std::array<int,10> notes{14,12,19,21,22,16,17,23,24,25};
constexpr std::array<const char*,10> labels{"capture","phrase","bar","beat","reset-signal","cue","stop",
                                          "capture-clock","capture-arm","capture-stop"};

class Trace final : public ProbeProcessor, private juce::Thread
{
public:
    Trace() : ProbeProcessor(BusesProperties()
        .withInput("Reference",juce::AudioChannelSet::stereo(),true)
        .withOutput("Dry",juce::AudioChannelSet::stereo(),true)),
        juce::Thread("Rack timing trace writer")
    {
        const juce::File folder(TRACE_DIRECTORY);
        if (folder.createDirectory().wasOk())
            trace=folder.getChildFile("timing-"+juce::Uuid().toString()+".csv").createOutputStream();
        if (trace && trace->openedOk())
            *trace << "sample,blockStart,frames,rate,stream,value,wallMs,note,channel\n";
        startThread();
    }
    ~Trace() override { stopThread(-1); drain(); }
    void prepareToPlay(double,int) override { prepares.fetch_add(1); }
    void reset() override { resets.fetch_add(1); }
    void releaseResources() override { releases.fetch_add(1); }
    void processBlockBypassed(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi) override
    {
        const int frames=buffer.getNumSamples();
        if (!wasBypassed)
            record({sample,sample,frames,static_cast<int>(getSampleRate()),"bypass",1});
        wasBypassed=true;
        sample+=frames;
        midi.clear();
    }

    void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi) override
    {
        const int frames=buffer.getNumSamples(), rate=static_cast<int>(getSampleRate());
        if (frames==0)
            return;
        const auto preparesNow=prepares.load(), resetsNow=resets.load(), releasesNow=releases.load();
        if (preparesNow!=lastPrepares || resetsNow!=lastResets || releasesNow!=lastReleases || wasBypassed)
        {
            record({sample,sample,frames,rate,"prepare",preparesNow});
            record({sample,sample,frames,rate,"reset",resetsNow});
            record({sample,sample,frames,rate,"release",releasesNow});
            known.fill(false);
            lastPrepares=preparesNow;lastResets=resetsNow;lastReleases=releasesNow;wasBypassed=false;
        }
        if (frames!=lastFrames || rate!=lastRate || sample>=nextHeartbeat)
        {
            record({sample,sample,frames,rate,"callback",0});
            lastFrames=frames;lastRate=rate;nextHeartbeat=sample+rate/4;
        }
        int eventCount=0;
        for (const auto event:midi)
        {
            if (++eventCount > maxMidiEvents)
            {
                record({sample,sample,frames,rate,"midi-overflow",1});
                break;
            }
            if (event.samplePosition<0 || event.samplePosition>=frames)
            {
                record({sample,sample,frames,rate,"invalid-offset",event.samplePosition});
                continue;
            }
            if (event.numBytes!=3)
                continue;
            const auto type=event.data[0]&0xf0;
            if (type!=0x80 && type!=0x90)
                continue;
            for (size_t i=0;i<notes.size();++i)
                if (event.data[1]==notes[i])
                    record({sample+event.samplePosition,sample,frames,rate,labels[i],
                            type==0x90 && event.data[2]>0,event.data[1],(event.data[0]&0x0f)+1});
        }
        constexpr std::array<const char*,2> audioLabels{"left","right"};
        for (int channel=0;channel<std::min(2,buffer.getNumChannels());++channel)
        {
            const auto* lane=buffer.getReadPointer(channel);
            for (int offset=0;offset<frames;++offset)
            {
                const bool high=lane[offset]>0.2f;
                if (!known[channel] || high!=level[channel])
                {
                    record({sample+offset,sample,frames,rate,audioLabels[channel],high});
                    level[channel]=high;
                    known[channel]=true;
                }
            }
        }
        sample+=frames;
        midi.clear();
    }

private:
    void record(const Event& event) noexcept
    {
        if (logged>=limit)
        {
            exhausted.store(true);
            return;
        }
        const auto slot=queue.write(1);
        if (slot.blockSize1!=1)
        {
            dropped.fetch_add(1);
            return;
        }
        data[static_cast<size_t>(slot.startIndex1)]=event;
        ++logged;
    }

    void run() override
    {
        while (!threadShouldExit())
        {
            drain();
            wait(100);
        }
    }

    void drain()
    {
        if (!trace || !trace->openedOk())
            return;
        const auto wall=juce::Time::currentTimeMillis();
        const auto slots=queue.read(capacity);
        const auto write=[&](int start,int size)
        {
            for (int i=start;i<start+size;++i)
            {
                const auto& event=data[static_cast<size_t>(i)];
                *trace << event.sample << "," << event.blockStart << "," << event.frames << ","
                    << event.rate << "," << event.stream << "," << event.value << "," << wall << ","
                    << event.note << "," << event.channel << "\n";
            }
        };
        write(slots.startIndex1,slots.blockSize1);
        write(slots.startIndex2,slots.blockSize2);
        if (const auto losses=dropped.exchange(0);losses>0)
            *trace << "# dropped " << losses << "\n";
        if (exhausted.load() && !reportedLimit)
        {
            *trace << "# trace limit reached\n";
            reportedLimit=true;
        }
        trace->flush();
    }

    juce::AbstractFifo queue{capacity};
    std::array<Event,capacity> data{};
    std::unique_ptr<juce::FileOutputStream> trace;
    std::atomic<int> dropped{0},prepares{0},resets{0},releases{0};
    std::atomic<bool> exhausted{false};
    std::array<bool,2> known{},level{};
    int64_t sample=0,nextHeartbeat=0;
    int lastFrames=0,lastRate=0,logged=0,lastPrepares=0,lastResets=0,lastReleases=0;
    bool wasBypassed=false,reportedLimit=false;
};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Trace; }
