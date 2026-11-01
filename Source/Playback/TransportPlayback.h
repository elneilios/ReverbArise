#pragma once
#include <Core/SwellSession.h>
#include <Core/TransportTime.h>
#include <DSP/ReverseConvolution.h>
#include <array>
#include <atomic>

namespace m3
{
constexpr size_t liveMaximumOverlap=64;
struct LiveEntry
{
    size_t slot=0;
    uint64_t generation=0;
    int64_t start=0,end=0;
    float peak=0;
    std::shared_ptr<const WetRender> audio;
    std::shared_ptr<const WetRender> loopAudio;
};
struct LiveSegment
{
    int64_t start=0,end=0;
    std::array<uint16_t,liveMaximumOverlap> members{};
    size_t count=0;
};
struct LivePublication
{
    SongTime mapping;
    uint64_t revision=0;
    int64_t start=0,end=0;
    int64_t loopStart=0,loopEnd=0;
    float attenuation=1;
    std::vector<LiveEntry> entries;
    std::vector<LiveSegment> segments;
    uint64_t bytes() const noexcept;
};
bool prepareLiveSegments(LivePublication&,juce::String& error);

// Serialized control producer, single audio consumer. Slots are reclaimed only
// after the consumer explicitly retires them; callback never owns shared_ptrs.
class TransportPlayback
{
public:
    TransportPlayback(){channels.fill(-1);}
    bool publish(std::unique_ptr<LivePublication>);
    void collect();
    uint64_t pinnedBytes() const noexcept;
    void invalidate(size_t slot,uint64_t generation) noexcept {if(slot<maximumSwells)generations[slot].store(generation,std::memory_order_release);}
    void configure(bool enabled,LiveRouting mode,float db) noexcept;
    // Offline start has no previous route to crossfade from.
    void primeForOffline() noexcept {gain=gainTarget=level.load();dry=dryTarget=enabled.load() && wetOnly.load()?0.0f:1.0f;gainRemaining=dryRemaining=0;}
    void reset() noexcept {epoch.fetch_add(1);}
    void process(float*,float*,int,const TransportTime&) noexcept;
    bool missing() const noexcept {return missed.load();}
    bool incompatible() const noexcept {return badMapping.load();}
    bool active() const noexcept {return liveActive.load();}
private:
    enum State{free,ready,activeSlot,retired};
    struct Slot{std::atomic<int> state{free};std::unique_ptr<LivePublication> data;};
    struct Voice
    {
        size_t slot=maximumSwells;uint64_t generation=0;
        const LiveEntry* entry=nullptr;
        float lastL=0,lastR=0,fromL=0,fromR=0;
        int remaining=0;
        bool audible=false;
        float attenuation=1;
    };
    std::array<Slot,4> slots;
    std::array<std::atomic<uint64_t>,maximumSwells> generations{};
    std::array<Voice,liveMaximumOverlap> voices{};
    std::array<int,maximumSwells> channels{};
    std::atomic<int> pending{-1};
    std::atomic<uint64_t> epoch{0};uint64_t seenEpoch=0;
    std::atomic<bool> enabled{true},wetOnly{false},missed{false},badMapping{false},liveActive{false};
    std::atomic<float> level{1};
    int current=-1;
    int64_t expectedPosition=0;
    bool wasRunning=false;
    float gain=1,gainTarget=1,gainStep=0,dry=1,dryTarget=1,dryStep=0;
    int gainRemaining=0,dryRemaining=0;
};
}
