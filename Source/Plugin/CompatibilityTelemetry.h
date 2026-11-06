#pragma once
#include <Capture/CaptureFailure.h>
#include <Core/WorkflowState.h>
#include <juce_core/juce_core.h>
#include <array>
#include <atomic>
#include <deque>

namespace compatibility
{
enum class Gate { ready, offline, mode, locked, position, samples, ppq, tempo, meter, mapping, edge, admission };
inline const char* text(Gate g)
{
    switch(g) {
    case Gate::ready:return "Capture start eligible";
    case Gate::offline:return "Capture blocked: host is rendering offline";
    case Gate::mode:return "Capture blocked: select Learn mode";
    case Gate::locked:return "Capture start blocked: pass or generation is busy";
    case Gate::position:return "Capture blocked: host position information is missing (playback unknown)";
    case Gate::samples:return "Capture blocked: host sample position is missing";
    case Gate::ppq:return "Capture blocked: host musical position is missing";
    case Gate::tempo:return "Capture blocked: host tempo is missing";
    case Gate::meter:return "Capture blocked: host time signature unavailable";
    case Gate::mapping:return "Capture blocked: source mapping is disabled or unavailable";
    case Gate::edge:return "Capture start waits for a new host play edge or entry into Learn; stop then play";
    case Gate::admission:return "Capture start rejected: workflow changed during bounded admission";
    }return "Unknown";
}
struct Observation
{
    CaptureHostTiming timing;
    workflow::Snapshot flow;
    Gate gate=Gate::position;
    CaptureFailureReason rejection=CaptureFailureReason::none;
    bool secondsAvailable=false,loopAvailable=false,nonzero=false,nonfinite=false,attempt=false,started=false,discontinuity=false;
    double seconds=0,loopBegin=0,loopEnd=0,at=0;
    float left=0,right=0;
    int channels=0,frames=0;
    uint64_t callbacks=0,queued=0,dropped=0;
};
inline bool sameState(const Observation& a,const Observation& b)
{
    const auto& x=a.timing;const auto& y=b.timing;
    return a.gate==b.gate && a.rejection==b.rejection && a.nonzero==b.nonzero && a.nonfinite==b.nonfinite
        && a.attempt==b.attempt && a.started==b.started && a.discontinuity==b.discontinuity
        && a.secondsAvailable==b.secondsAvailable && a.loopAvailable==b.loopAvailable
        && a.flow.phase==b.flow.phase && a.flow.result==b.flow.result && a.flow.pass==b.flow.pass
        && x.position==y.position && x.sample==y.sample && x.ppq==y.ppq && x.tempo==y.tempo && x.meter==y.meter
        && x.playing==y.playing && x.looping==y.looping && x.bpm==y.bpm && x.numerator==y.numerator && x.denominator==y.denominator;
}
// One audio producer, one message-thread consumer. No allocation/locks on publish.
// Full queue omits observations, never capture. Strings/history exist only on UI thread.
class Telemetry
{
public:
    struct Settlement {double at=0;uint64_t pass=0;bool useful=false;};
    // Capture worker is a separate producer; it never writes the audio queue.
    void settled(uint64_t pass,bool useful)noexcept
    {
        const auto w=settlementWrite.load(std::memory_order_relaxed),r=settlementRead.load(std::memory_order_acquire);
        if(w-r==settlements.size()){settlementOmitted.fetch_add(1);return;}
        settlements[w%settlements.size()]={juce::Time::getMillisecondCounterHiRes(),pass,useful};
        settlementWrite.store(w+1,std::memory_order_release);
    }
    static constexpr unsigned capacity=256,historyCapacity=128;
    void publish(Observation o) noexcept
    {
        o.callbacks=++callbackCount;o.at=juce::Time::getMillisecondCounterHiRes();
        if(o.nonzero)everNonzero.store(true,std::memory_order_relaxed);
        const bool changed=!hasPrevious || !sameState(o,previous);
        previous=o;hasPrevious=true;
        if(!changed && o.at-lastPublished<1000)return;
        const auto w=write.load(std::memory_order_relaxed),r=read.load(std::memory_order_acquire);
        if(w-r==capacity){omitted.fetch_add(1,std::memory_order_relaxed);return;}
        queue[w%capacity]=o;write.store(w+1,std::memory_order_release);lastPublished=o.at;
    }
    void drain()
    {
        auto r=read.load(std::memory_order_relaxed);const auto w=write.load(std::memory_order_acquire);
        while(r!=w){latest=queue[r%capacity];hasLatest=true;
            if(history.size()==historyCapacity){history.pop_front();++truncated;}
            history.push_back(latest);++r;}
        read.store(r,std::memory_order_release);
        auto sr=settlementRead.load(std::memory_order_relaxed);const auto sw=settlementWrite.load(std::memory_order_acquire);
        while(sr!=sw){if(settlementHistory.size()==32){settlementHistory.pop_front();++settlementTruncated;}
            settlementHistory.push_back(settlements[sr%settlements.size()]);++sr;}
        settlementRead.store(sr,std::memory_order_release);
    }
    void clear()
    {
        drain();history.clear();truncated=0;omittedBaseline=omitted.load();callbackBaseline=callbackCount.load();
        nonzeroBaseline=everNonzero.load(); // Lifetime evidence remains explicitly labelled.
        settlementHistory.clear();settlementTruncated=0;settlementOmittedBaseline=settlementOmitted.load();
    }
    std::array<Observation,capacity> queue{};
    Observation latest{};bool hasLatest=false;
    std::deque<Observation> history;
    uint64_t truncated=0,omittedBaseline=0,callbackBaseline=0;bool nonzeroBaseline=false;
    std::atomic<uint64_t> callbackCount{0},omitted{0};std::atomic<bool> everNonzero{false};
    std::deque<Settlement> settlementHistory;
    uint64_t settlementTruncated=0,settlementOmittedBaseline=0;
    std::atomic<uint64_t> settlementOmitted{0};
private:
    std::array<Settlement,64> settlements{};
    std::atomic<unsigned> settlementWrite{0},settlementRead{0};
    std::atomic<unsigned> write{0},read{0};
    Observation previous{};bool hasPrevious=false;double lastPublished=0;
};
}
