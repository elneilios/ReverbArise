#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <juce_audio_basics/juce_audio_basics.h>

namespace diagnostic
{
// Raw observations only. This domain is never an input to timing admission.
enum TraceFlag : uint32_t
{
    position=1, sample=2, ppq=4, tempo=8, meter=16, playing=32,
    recording=64, looping=128, offline=256, bar=512, cycle=1024,
    discontinuity=2048, invalidRange=4096
};
struct TimingTraceRecord
{
    uint64_t callback=0,run=0,reset=0,sourceEpoch=0,pass=0;
    int64_t sample=0;
    double ppq=0,bpm=0,rate=0,barPpq=0,cycleStartPpq=0,cycleEndPpq=0;
    int32_t frames=0,numerator=0,denominator=0;
    uint32_t flags=0;
};
static_assert(std::is_trivially_copyable_v<TimingTraceRecord>);
static_assert(std::atomic<uint64_t>::is_always_lock_free);

// Single callback producer / worker consumer; release/acquire transfers slots.
template<size_t Capacity> class TraceQueue
{
    static_assert(Capacity>0);
    std::array<TimingTraceRecord,Capacity> records{};
    std::atomic<uint64_t> written{0},read{0};
public:
    bool push(const TimingTraceRecord& record) noexcept
    {
        const auto w=written.load(std::memory_order_relaxed);
        if(w-read.load(std::memory_order_acquire)>=Capacity)return false;
        records[w%Capacity]=record;written.store(w+1,std::memory_order_release);return true;
    }
    bool pop(TimingTraceRecord& record) noexcept
    {
        const auto r=read.load(std::memory_order_relaxed);
        if(r==written.load(std::memory_order_acquire))return false;
        record=records[r%Capacity];read.store(r+1,std::memory_order_release);return true;
    }
};

class TimingTrace final : private juce::Thread
{
public:
    static constexpr uint64_t maximumRecords=65536;
    static std::unique_ptr<TimingTrace> fromEnvironment();
    // Explicit directory is also used by native fixtures. No implicit creation.
    explicit TimingTrace(const juce::File& existingDirectory);
    ~TimingTrace() override;
    void observe(const juce::Optional<juce::AudioPlayHead::PositionInfo>&,
                 double rate,int frames,bool offline,uint64_t reset,uint64_t sourceEpoch,uint64_t pass) noexcept;
    const juce::File& file() const noexcept {return output;}
    uint64_t omittedRecords() const noexcept {return omitted.load();}
    bool failed() const noexcept {return ioFailed.load();}
private:
    void run() override;
    juce::File output;
    std::unique_ptr<juce::FileOutputStream> stream;
    TraceQueue<maximumRecords> queue;
    std::atomic<uint64_t> omitted{0};
    std::atomic<bool> ioFailed{false};
    uint64_t callback=0,runId=0,retained=0,previousReset=0,previousEpoch=0;
    bool previousPlaying=false,previousOffline=false,expectedValid=false,havePrevious=false;
    int64_t expected=0;
};
}
