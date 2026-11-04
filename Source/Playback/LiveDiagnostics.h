#pragma once
#include <Core/TransportTime.h>
#include <array>
#include <atomic>
#include <cstring>
#include <optional>
#include <type_traits>

namespace m3
{
// Single writer, any number of readers. Atomic words avoid data races even when
// a reader overlaps publication. Sequential consistency makes the sequence and
// every payload word one ordered observation; readers never spin indefinitely.
template<class T> class LiveObservationSlot
{
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(std::atomic<uint64_t>::is_always_lock_free);
    static constexpr size_t words=(sizeof(T)+sizeof(uint64_t)-1)/sizeof(uint64_t);
    std::atomic<uint64_t> sequence{0};
    std::array<std::atomic<uint64_t>,words> payload{};
public:
    void publish(const T& value) noexcept
    {
        std::array<uint64_t,words> copy{};std::memcpy(copy.data(),&value,sizeof(T));
        sequence.fetch_add(1);
        for(size_t i=0;i<words;++i)payload[i].store(copy[i]);
        sequence.fetch_add(1);
    }
    std::optional<T> read() const noexcept
    {
        for(int attempt=0;attempt<8;++attempt){
            const auto before=sequence.load();if(!before || (before&1))continue;
            std::array<uint64_t,words> copy{};
            for(size_t i=0;i<words;++i)copy[i]=payload[i].load();
            if(sequence.load()==before){T value;std::memcpy(&value,copy.data(),sizeof(T));return value;}
        }
        return std::nullopt;
    }
};
enum class LiveGate { admitted, workflow, disabled, stopped, noPublication, invalidRange,
    invalidTiming, rate, tempo, meter, origin, reset, missingCoverage };
inline LiveGate liveMappingGate(const SongTime& prepared,const SongTime& now) noexcept
{
    if(!prepared.valid || !now.valid)return LiveGate::invalidTiming;
    if(prepared.rate!=now.rate)return LiveGate::rate;
    if(prepared.bpm!=now.bpm)return LiveGate::tempo;
    if(prepared.numerator!=now.numerator || prepared.denominator!=now.denominator)return LiveGate::meter;
    long double a,b;if(!songOrigin(prepared,a) || !songOrigin(now,b))return LiveGate::invalidTiming;
    return std::abs(a-b)<=1.0L?LiveGate::admitted:LiveGate::origin;
}
struct LiveCallbackObservation
{
    uint64_t block=0,revision=0;
    SongTime observed{},published{};
    int64_t start=0,end=0,loopStart=0,loopEnd=0;
    double publishedOrigin=0,observedOrigin=0,originDelta=0;
    int count=0;
    LiveGate gate=LiveGate::noPublication;
    bool hasPublication=false,originsValid=false,validCount=false,playing=false;
    bool workflowAllows=false,enabled=false,wetOnly=false,running=false,missing=false;
    float wetGain=1;
};
}
