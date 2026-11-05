#pragma once
#include <array>
#include <atomic>
#include <optional>
#include <span>
#include <cstdint>

namespace m3
{
// One worker producer and one audio consumer. Storage is allocated once, never
// retired on the callback. Control intent revokes even a delayed worker arm.
class ExportVoice
{
    static_assert(std::atomic<uint64_t>::is_always_lock_free);
public:
    static constexpr size_t pageFrames=4096,pageCount=16;
    uint64_t requestStart() noexcept;
    void stop(bool immediately=false) noexcept;
    bool requested(uint64_t ticket)const noexcept;
    uint64_t request()const noexcept{return intent.load(std::memory_order_acquire);}
    bool hasRoom(uint64_t ticket)const noexcept; // Worker-only capacity query.
    bool push(uint64_t ticket,int64_t at,int64_t start,int64_t end,double rate,std::span<const float>);
    bool pushLayers(uint64_t ticket,int64_t at,int64_t start,int64_t end,double rate,std::span<const float> dry,std::span<const float> wet);
    void setLevels(uint64_t ticket,float dry,float wet,int64_t end) noexcept;
    struct AcceptedLevels {float dry=0,wet=1;uint64_t revision=0;};
    AcceptedLevels acceptedLevels()const noexcept;
    void arm(uint64_t ticket) noexcept {if(requested(ticket))armed.store(ticket,std::memory_order_release);}
    void process(float*,float*,int,double rate,bool allowed)noexcept;
    std::optional<int64_t> position()const noexcept;
    bool starved()const noexcept{return underrun.load();}
    bool invalidAudio()const noexcept{return invalidSamples.load();}
    uint64_t workingBytes()const noexcept{return sizeof(*this);}
private:
    enum : unsigned {free,writing,ready,reading};
    struct Page{
        std::atomic<unsigned> state{free};
        uint64_t ticket=0;int64_t at=0,start=0,end=0;double rate=0;size_t frames=0;
        std::array<float,pageFrames*2> samples;
        std::array<float,pageFrames*2> wetSamples;
        bool layered=false;
    };
    int take(uint64_t,std::optional<int64_t> at)noexcept;
    bool pushPage(uint64_t,int64_t,int64_t,int64_t,double,std::span<const float>,std::span<const float>);
    void release()noexcept;
    std::array<Page,pageCount> pages;
    std::atomic<uint64_t> intent{0},armed{0},finished{0},immediate{0};
    std::atomic<bool> underrun{false},positionActive{false};
    std::atomic<bool> invalidSamples{false};
    std::atomic<int64_t> positionSample{0};std::atomic<uint64_t> positionSequence{0};
    std::atomic<uint64_t> positionIntent{0};
    uint64_t producerTicket=0;
    uint64_t playingTicket=0;int active=-1;size_t cursor=0;
    int stopRemaining=0,stopLength=1;
    std::atomic<uint64_t> targetSequence{0},targetTicket{0},targetPair{uint64_t{0x3f800000}<<32};
    std::atomic<int64_t> targetEnd{0};
    std::atomic<uint64_t> acceptedPair{uint64_t{0x3f800000}<<32},acceptedRevision{0};
    uint64_t appliedRevision=0;int64_t playingEnd=0;
    float dryLevel=0,wetLevel=1,dryTarget=0,wetTarget=1;int gainRemaining=0;
};
}
