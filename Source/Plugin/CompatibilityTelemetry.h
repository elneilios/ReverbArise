#pragma once
#include <Capture/CaptureFailure.h>
#include <Core/WorkflowState.h>
#include <juce_core/juce_core.h>
#include <array>
#include <atomic>
#include <deque>

namespace compatibility
{
// Serialized host prepare writer, bounded message-thread reads. Every payload
// field is atomic: no racy plain-struct seqlock and no callback allocation.
class RateHistory
{
public:
    static_assert(std::atomic<double>::is_always_lock_free);
    static constexpr size_t capacity=16;
    void prepared(double rate,int frames)noexcept
    {
        const auto n=count.load(std::memory_order_relaxed);auto& slot=slots[n%capacity];
        slot.sequence.fetch_add(1,std::memory_order_acq_rel);
        slot.rate.store(rate,std::memory_order_relaxed);slot.frames.store(frames,std::memory_order_relaxed);
        slot.at.store(juce::Time::getMillisecondCounterHiRes(),std::memory_order_relaxed);
        slot.ordinal.store(n+1,std::memory_order_relaxed);slot.sequence.fetch_add(1,std::memory_order_release);
        count.store(n+1,std::memory_order_release);
    }
    juce::String report()const
    {
        const auto n=count.load(std::memory_order_acquire);juce::String s;
        s<<"Host preparation history (instance lifetime): "<<juce::String(static_cast<juce::int64>(n))
            <<" calls; older entries omitted="<<juce::String(static_cast<juce::int64>(n>capacity?n-capacity:0))<<"\n";
        for(auto i=n>capacity?n-capacity:0;i<n;++i){const auto& slot=slots[i%capacity];bool copied=false;
            for(int attempt=0;attempt<8;++attempt){const auto before=slot.sequence.load(std::memory_order_acquire);if(before&1)continue;
                const auto rate=slot.rate.load();const auto frames=slot.frames.load();const auto at=slot.at.load();const auto ordinal=slot.ordinal.load();
                if(slot.sequence.load(std::memory_order_acquire)!=before || ordinal!=i+1)continue;
                s<<"prepare #"<<juce::String(static_cast<juce::int64>(ordinal))<<": "<<juce::String(rate,3)<<" Hz, max block "<<frames<<", "<<juce::String(static_cast<juce::int64>(at))<<" ms\n";copied=true;break;}
            if(!copied)s<<"prepare observation changed during read; unavailable\n";
        }return s;
    }
private:
    struct Slot {std::atomic<uint64_t> sequence{0},ordinal{0};std::atomic<double> rate{0},at{0};std::atomic<int> frames{0};};
    std::array<Slot,capacity> slots{};std::atomic<uint64_t> count{0};
};
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
    bool rawMeterAvailable=false;int rawNumerator=0,rawDenominator=0;
    double seconds=0,loopBegin=0,loopEnd=0,at=0;
    float left=0,right=0;
    int channels=0,frames=0;
    uint64_t callbacks=0,queued=0,dropped=0;
};
inline bool sameState(const Observation& a,const Observation& b)
{
    const auto& x=a.timing;const auto& y=b.timing;
    return a.gate==b.gate && a.rejection==b.rejection && a.nonzero==b.nonzero && a.nonfinite==b.nonfinite
        && a.rawMeterAvailable==b.rawMeterAvailable && a.rawNumerator==b.rawNumerator && a.rawDenominator==b.rawDenominator
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
