#pragma once
#include "LiveDiagnostics.h"
#include <Core/SwellSession.h>
#include <Core/HostGrid.h>

namespace m3
{
struct LiveClockSnapshot
{
    uint64_t session=0,run=0,reset=0;
    SongTime reference{},observed{};
    LiveGate gate=LiveGate::invalidTiming;
    bool playing=false;
    LiveGate accepts(const SongTime& asset,const SongTime& now) const noexcept
    {
        if(gate!=LiveGate::admitted && gate!=LiveGate::stopped)return gate;
        const auto fixed=liveAssetGate(asset,now);if(fixed!=LiveGate::admitted)return fixed;
        return liveMappingGate(playing?reference:observed,now);
    }
    bool sameRun(const LiveClockSnapshot& other) const noexcept
    {return session==other.session && reset==other.reset && run==other.run && playing==other.playing;}
};
inline GridClockContext gridClockContext(const std::optional<LiveClockSnapshot>& clock,bool playing)noexcept
{
    GridClockContext result;result.playing=playing;
    if(clock && clock->playing==playing){
        result.available=true;result.admitted=clock->gate==LiveGate::admitted;
        result.reference=clock->reference;result.session=clock->session;result.run=clock->run;result.reset=clock->reset;
    }
    return result;
}
// Only the callback writes state. Worker reads a separate bounded atomic value
// publication, never a telemetry snapshot or a pointer into callback ownership.
class LiveTransportClock
{
    LiveClockSnapshot state;
    bool hasRun=false;
    std::atomic<uint64_t> resetRequest{0};
    LiveObservationSlot<LiveClockSnapshot> publication;
public:
    void reset() noexcept {resetRequest.fetch_add(1);}
    LiveClockSnapshot observe(const TransportTime& time,int frames,uint64_t session) noexcept
    {
        const auto reset=resetRequest.load();
        if(state.session!=session || state.reset!=reset){state={};state.session=session;state.reset=reset;hasRun=false;}
        state.observed=time.song;state.playing=time.playing;
        int64_t end;long double origin;
        if(frames<=0 || frames>1048576 || !checkedAdd(time.song.sample,frames,end))state.gate=LiveGate::invalidRange;
        else if(!songOrigin(time.song,origin))state.gate=LiveGate::invalidTiming;
        else if(!time.playing){hasRun=false;state.reference={};state.gate=LiveGate::stopped;}
        else {
            if(!hasRun){++state.run;state.reference=time.song;hasRun=true;}
            state.gate=liveMappingGate(state.reference,time.song);
        }
        publication.publish(state);return state;
    }
    std::optional<LiveClockSnapshot> snapshot(uint64_t session) const noexcept
    {
        const auto value=publication.read();
        if(!value || value->session!=session || value->reset!=resetRequest.load())return std::nullopt;
        return value;
    }
};
}
