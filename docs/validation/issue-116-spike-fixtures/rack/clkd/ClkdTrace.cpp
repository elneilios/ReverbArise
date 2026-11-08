#include "../Processor.h"
#include <cmath>
#include <array>
#include <atomic>

namespace
{
struct Event
{
    int64_t sample=0, blockStart=0;
    int frames=0, rate=0;
    const char* stream="";
    int value=0;
};
constexpr int capacity=4096, limit=18000;
constexpr std::array<int,4> notes{14,12,16,17};
constexpr std::array<const char*,4> labels{"reset-signal","phrase","bar","capture-stop"};

class Trace final : public ProbeProcessor, private juce::Timer
{
public:
    Trace() : ProbeProcessor(BusesProperties()
        .withInput("Reference",juce::AudioChannelSet::stereo(),true)
        .withOutput("Dry",juce::AudioChannelSet::stereo(),true))
    {
        const juce::File folder(TRACE_DIRECTORY);
        if (folder.createDirectory().wasOk())
            trace=folder.getChildFile("timing-"+juce::Uuid().toString()+".csv").createOutputStream();
        if (trace && trace->openedOk())
            *trace << "sample,blockStart,frames,rate,stream,value,wallMs\n";
        startTimerHz(10);
    }
    ~Trace() override { stopTimer(); timerCallback(); }
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
        for (const auto event:midi)
        {
            if (event.numBytes!=3)
                continue;
            const auto type=event.data[0]&0xf0;
            if (type!=0x80 && type!=0x90)
                continue;
            for (size_t i=0;i<notes.size();++i)
                if (event.data[1]==notes[i])
                    record({sample+event.samplePosition,sample,frames,rate,labels[i],type==0x90 && event.data[2]>0});
        }
        constexpr std::array<const char*,2> audioLabels{"left","right"};
        for (int channel=0;channel<std::min(2,buffer.getNumChannels());++channel)
        {
            const auto* lane=buffer.getReadPointer(channel);
            for (int offset=0;offset<frames;++offset)
            {
                const bool high=std::abs(lane[offset])>0.01f;
                if (!known[channel] || high!=level[channel])
                {
                    record({sample+offset,sample,frames,rate,audioLabels[channel],high});
                    level[channel]=high;
                    known[channel]=true;
                }
            }
        }
        // Test-only capture window, referenced to incoming MIDI sample offsets.
        auto next = midi.begin();
        const auto end = midi.end();
        for (int offset = 0; offset < frames; ++offset)
        {
            bool phrase = false, bar = false, stop = false, reset = false;
            while (next != end && (*next).samplePosition <= offset)
            {
                const auto event = *next;
                if (event.numBytes == 3 && (event.data[0] & 0xf0) == 0x90 && event.data[2] > 0)
                {
                    reset |= event.data[1] == notes[0];
                    phrase |= event.data[1] == notes[1];
                    bar |= event.data[1] == notes[2];
                    stop |= event.data[1] == notes[3];
                }
                ++next;
            }

            const auto now = sample + offset;
            if (reset)
            {
                if (captureStart >= 0)
                    record({now, sample, frames, rate, "capture-cancelled", capturedFrames});
                captureStart = -1;
                lastBar = -1;
                barPeriod = 0;
            }
            if (captureStart >= 0 && lastBar >= 0 && barPeriod > 0
                && now - lastBar > barPeriod + rate / 200)
            {
                record({now, sample, frames, rate, "capture-cancelled", capturedFrames});
                captureStart = -1;
            }
            if (bar)
            {
                if (lastBar >= 0)
                    barPeriod = now - lastBar;
                lastBar = now;
            }

            const bool wasCapturing = captureStart >= 0;
            if (wasCapturing && stop && !waitingForBar)
            {
                waitingForBar = true;
                record({now, sample, frames, rate, "stop-pending", capturedFrames});
            }
            if (wasCapturing && waitingForBar && bar)
            {
                record({now, sample, frames, rate, "capture-end", capturedFrames});
                record({now, sample, frames, rate, "non-silent-frames", nonSilentFrames});
                captureStart = -1;
            }
            if (!wasCapturing && phrase && bar && barPeriod > 0)
            {
                captureStart = now;
                capturedFrames = nonSilentFrames = 0;
                waitingForBar = false;
                record({now, sample, frames, rate, "capture-start", 0});
            }
            if (captureStart >= 0)
            {
                ++capturedFrames;
                if (std::abs(buffer.getSample(0, offset)) > 0.01f
                    || std::abs(buffer.getSample(1, offset)) > 0.01f)
                    ++nonSilentFrames;
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

    void timerCallback() override
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
                    << event.rate << "," << event.stream << "," << event.value << "," << wall << "\n";
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
    int64_t captureStart = -1, lastBar = -1, barPeriod = 0;
    int capturedFrames = 0, nonSilentFrames = 0;
    bool waitingForBar = false;
    bool wasBypassed=false,reportedLimit=false;
};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Trace; }
