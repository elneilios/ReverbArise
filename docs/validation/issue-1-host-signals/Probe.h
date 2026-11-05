#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <atomic>

namespace hostprobe {
double now() noexcept;
enum Kind { audio, bypass, prepare, release, reset, channels, layout, track, host, handler, save, restore, realtime, marker, kindCount };
inline constexpr const char* names[]={"audio","bypass","prepare","release","reset","channels","layout","track","host","handler","save","restore","realtime","marker"};
struct Event {
    double time=0,rate=0;uint64_t block=0;int64_t sample=-1;
    int kind=0,mark=0,frames=0,inChannels=0,outChannels=0;
    bool position=false,playing=false,looping=false,offline=false,inputEnabled=false,outputEnabled=false,tone=false;
    float input=0,output=0;
};
// Multiple callback/message producers; one consumer. Single try, never spin/wait.
// Contention/full queue drops a row and increments an explicit loss counter.
struct Queue {
    std::array<Event,8192> rows{};size_t read=0,write=0;
    std::atomic_flag busy=ATOMIC_FLAG_INIT;std::atomic<uint32_t> dropped{0};
    void push(const Event& e) noexcept {
        if(busy.test_and_set(std::memory_order_acquire)){dropped.fetch_add(1,std::memory_order_relaxed);return;}
        const auto next=(write+1)%rows.size();
        if(next==read)dropped.fetch_add(1,std::memory_order_relaxed);
        else {rows[write]=e;write=next;}
        busy.clear(std::memory_order_release);
    }
    bool pop(Event& e) noexcept {
        if(busy.test_and_set(std::memory_order_acquire))return false;
        const bool ok=read!=write;if(ok){e=rows[read];read=(read+1)%rows.size();}
        busy.clear(std::memory_order_release);return ok;
    }
};
class Probe final:public juce::AudioProcessor,public juce::VST3ClientExtensions,private juce::Timer {
public:
    Probe();~Probe() override;
    std::array<std::atomic<uint64_t>,kindCount> counts{};
    std::atomic<uint64_t> blocks{0};std::atomic<int> mark{0};std::atomic<bool> tone{false};
    Queue queue;Event latest{}; // latest is only read/written on the message thread.
    juce::File evidence;juce::String logStatus;const juce::String instance=juce::Uuid().toString();
    void record(Kind) noexcept;
    void setMarker(int n) noexcept {mark.store(n);record(marker);}
    void drain();
    void prepareToPlay(double r,int) override {rate.store(r);phase=0;envelope=0;record(prepare);}
    void releaseResources() override {record(release);}
    void reset() override {record(hostprobe::reset);} // Observe only; no inferred disarm.
    void numChannelsChanged() override {record(channels);}
    void processorLayoutsChanged() override {record(layout);}
    void updateTrackProperties(const TrackProperties&) override {record(track);}
    void setNonRealtime(bool value) noexcept override {const auto changed=value!=isNonRealtime();AudioProcessor::setNonRealtime(value);if(changed)record(realtime);}
    juce::VST3ClientExtensions* getVST3ClientExtensions() override {return this;}
    void setIHostApplication(Steinberg::FUnknown*) override {record(host);}
    void setIComponentHandler(Steinberg::FUnknown*) override {record(handler);}
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout& b) const override {return b.getMainInputChannelSet()==juce::AudioChannelSet::stereo() && b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
    juce::AudioProcessorEditor* createEditor() override;bool hasEditor() const override {return true;}
    const juce::String getName() const override {return "ReverbArise Host Signal Probe";}
    bool acceptsMidi() const override {return false;}bool producesMidi() const override {return false;}
    double getTailLengthSeconds() const override {return 2;}
    int getNumPrograms() override {return 1;}int getCurrentProgram() override {return 0;}
    void setCurrentProgram(int) override {}const juce::String getProgramName(int) override {return {};}
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& b) override {record(save);const uint8_t version=1;b.replaceAll(&version,1);}
    void setStateInformation(const void*,int) override {tone=false;record(restore);}
private:
    std::atomic<double> rate{48000};double phase=0;float envelope=0;
    Event event(Kind) noexcept;
    void render(juce::AudioBuffer<float>&,bool bypassed);
    void timerCallback() override {drain();}
};
static_assert(std::atomic<double>::is_always_lock_free && std::atomic<uint64_t>::is_always_lock_free);
}
