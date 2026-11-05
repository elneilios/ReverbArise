#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <atomic>

namespace probe {
constexpr uint32_t maxWord=0xffffff;
double qpcSeconds() noexcept;
inline float normalized(uint32_t n) { return static_cast<float>(n / double(maxWord)); }
inline uint32_t word(float v) noexcept {return std::isfinite(v) && v>=0 && v<=1 ? static_cast<uint32_t>(std::llround(v*double(maxWord))) : 0;}
inline uint32_t clockWord(double seconds) noexcept {return static_cast<uint32_t>(static_cast<uint64_t>(seconds*8000))&maxWord;}
inline void setRaw(juce::AudioParameterFloat* p,float v) noexcept {static_cast<juce::AudioProcessorParameter*>(p)->setValue(v);}
inline double ageMs(double seconds,uint32_t stamp) noexcept {return ((clockWord(seconds)-stamp)&maxWord)/8.0;}
struct Event {
    uint64_t block=0; double seconds=0,queryMs=-1,writeMs=-1,rate=0;
    int64_t hostSample=-1; uint32_t packet=0; int frames=0,mode=0;
    bool fresh=false,playing=false,offline=false,tone=false;
    float inputPeak=0,outputPeak=0,signalPeak=0;
};
// One callback producer, one message-thread consumer. No callback-owned heap data.
struct Ring {
    std::array<Event,8192> data{};
    std::atomic<uint32_t> read{0},write{0},dropped{0};
    void push(const Event& e) noexcept {
        const auto w=write.load(std::memory_order_relaxed);const auto next=(w+1)%data.size();
        if(next==read.load(std::memory_order_acquire)){++dropped;return;}
        data[w]=e;write.store(static_cast<uint32_t>(next),std::memory_order_release);
    }
    bool pop(Event& e) noexcept {
        const auto r=read.load(std::memory_order_relaxed);
        if(r==write.load(std::memory_order_acquire))return false;
        e=data[r];read.store(static_cast<uint32_t>((r+1)%data.size()),std::memory_order_release);return true;
    }
};
class Receiver final : public juce::AudioProcessor, private juce::Timer {
public:
    Receiver(); ~Receiver() override {stopTimer();timerCallback();}
    juce::AudioParameterFloat *context=nullptr,*ack=nullptr,*query=nullptr,*written=nullptr;
    std::atomic<bool> toneEnabled{false},resetPending{false};
    Ring ring;
    Event latest{}; // Message-thread only, after draining ring.
    const juce::String instance=juce::Uuid().toString();
    juce::File evidence;
    juce::String logStatus;
    void prepareToPlay(double r,int) override {rate=r;phase=0;envelope=0;}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    void processAt(juce::AudioBuffer<float>&,double now,bool playing,bool offline,int64_t hostSample=-1) noexcept;
    bool isBusesLayoutSupported(const BusesLayout& l) const override {return l.getMainInputChannelSet()==juce::AudioChannelSet::stereo() && l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    const juce::String getName() const override {return "ReverbArise FL Context Probe";}
    bool acceptsMidi() const override {return false;} bool producesMidi() const override {return false;}
    double getTailLengthSeconds() const override {return 2;}
    int getNumPrograms() override {return 1;} int getCurrentProgram() override {return 0;}
    void setCurrentProgram(int) override {} const juce::String getProgramName(int) override {return {};}
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& b) override {const uint8_t enabled=toneEnabled.load()?1:0;b.replaceAll(&enabled,1);}
    void setStateInformation(const void* p,int n) override {
        if(n==1)toneEnabled=(*static_cast<const uint8_t*>(p)!=0);
        setRaw(context,0);setRaw(query,0);setRaw(written,0);setRaw(ack,0);resetPending=true;
    }
private:
    double rate=48000,phase=0,lastReceipt=-1;float envelope=0;
    uint32_t lastPacket=0;uint64_t blocks=0;
    void timerCallback() override;
};
}
