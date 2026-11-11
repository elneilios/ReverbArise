#pragma once

#include "LoopMidiObservations.h"
#include <cmath>

namespace loop_capture
{
struct ClockProfile
{
    double rate=0, tempo=0;
    bool operator==(const ClockProfile&) const = default;
    bool valid() const noexcept {return rate>=8000 && rate<=192000 && tempo>=30 && tempo<=300;}
};

// Measured fallback for a timing source that does not declare a usable tempo.
inline ClockProfile measuredProfile(double sampleRate, int frames) noexcept
{
    if (sampleRate == 48000 && frames == 128) return {sampleRate,137};
    if (sampleRate == 48000 && frames == 256) return {sampleRate,280};
    return {};
}

class PhraseClock
{
public:
    static constexpr int beatsPerBar = 4;
    static constexpr int beatsPerPhrase = 16;
    enum class State { notConfigured, notVerified, ready, syncLost };
    struct Boundary
    {
        bool verifiedBeat = false;
        bool verifiedBar = false;
        bool verifiedPhrase = false;
        bool revoked = false;
    };

    bool configure(ClockProfile next) noexcept
    {
        if (!next.valid()) next={};
        if (next == profile) return false;
        profile = next;
        clearClock();
        authority = profile.valid() ? State::notVerified : State::notConfigured;
        return true;
    }
    State state() const noexcept { return authority; }
    bool phaseReady() const noexcept { return authority == State::ready && intervals >= 2 * beatsPerPhrase; }
    double tempo() const noexcept {return profile.tempo;}
    void revoke() noexcept
    {
        clearClock();
        if (authority == State::ready) authority = State::syncLost;
    }

    // Call once per sample group. A verified marker at the deadline is
    // processed before advance() is called for the rest of the block.
    Boundary observe(int64_t sample, uint16_t roles) noexcept
    {
        if (!profile.valid()) return {};
        if (roles & reset)
        {
            revoke();
            return {false, false, false, true};
        }
        if (sample < 0 || (last >= 0 && sample < last))
        {
            revoke();
            return {false, false, false, true};
        }
        const bool hasBeat = (roles & beat) != 0, hasBar = (roles & bar) != 0, hasPhrase = (roles & phrase) != 0;
        if (origin < 0)
        {
            if (hasBeat && hasBar && hasPhrase)
                origin = last = sample;
            return {};
        }
        if (!hasBeat && !hasBar && !hasPhrase)
            return {false, false, false, advance(sample)};

        const auto next = intervals + 1;
        if (!hasBeat || hasBar != (next % beatsPerBar == 0) || hasPhrase != (next % beatsPerPhrase == 0)
            || !matches(sample - last, 1) || !matches(sample - origin, next))
        {
            revoke();
            return {false, false, false, true};
        }
        last = sample;
        intervals = next;
        if (intervals >= 2) authority = State::ready;
        if (authority != State::ready) return {};
        return {true, hasBar, hasPhrase, false};
    }

    bool advance(int64_t sample) noexcept
    {
        if (last < 0 || sample < last)
            return false;
        if (sample-last<=std::ceil(period(1)) && sample-origin<=std::ceil(period(intervals+1)))
            return false;
        revoke();
        return true;
    }

private:
    long double period(int64_t beats) const noexcept
    {return static_cast<long double>(profile.rate)*60*beats/profile.tempo;}
    bool matches(int64_t samples, int64_t beats) const noexcept
    {
        const auto extent=period(beats);
        return samples>=std::floor(extent) && samples<=std::ceil(extent);
    }
    void clearClock() noexcept { origin = last = -1; intervals = 0; }
    ClockProfile profile;
    State authority = State::notConfigured;
    int64_t origin = -1, last = -1, intervals = 0;
};

// Both references consume the same sample-offset observations. Once a reference
// wins, a change or loss revokes its authority before another one can take over.
class ClockAdmission
{
public:
    using State=PhraseClock::State;
    bool configure(ClockProfile declared, ClockProfile measured) noexcept
    {
        const auto declaredChanged=host.configure(declared);
        const auto measuredChanged=fallback.configure(measured);
        if ((selected==&host && declaredChanged) || (selected==&fallback && measuredChanged))
        {
            selected=nullptr;
            return true;
        }
        return false;
    }
    PhraseClock::Boundary observe(int64_t sample,uint16_t roles) noexcept
    {
        const auto hostEdge=host.observe(sample,roles);
        const auto fallbackEdge=fallback.observe(sample,roles);
        if (selected)
        {
            const auto edge=selected==&host?hostEdge:fallbackEdge;
            if(edge.revoked)selected=nullptr;
            return edge;
        }
        if(host.state()==State::ready){selected=&host;return hostEdge;}
        if(fallback.state()==State::ready){selected=&fallback;return fallbackEdge;}
        return {};
    }
    bool advance(int64_t sample) noexcept
    {
        const auto lostHost=host.advance(sample);
        const auto lostFallback=fallback.advance(sample);
        if ((selected==&host && lostHost) || (selected==&fallback && lostFallback))
        {
            selected=nullptr;
            return true;
        }
        return false;
    }
    void revoke() noexcept {selected=nullptr;host.revoke();fallback.revoke();}
    double tempo() const noexcept {return selected?selected->tempo():0;}
    State state() const noexcept
    {
        if(selected)return State::ready;
        if(host.state()==State::syncLost || fallback.state()==State::syncLost)return State::syncLost;
        if(host.state()!=State::notConfigured || fallback.state()!=State::notConfigured)return State::notVerified;
        return State::notConfigured;
    }
private:
    PhraseClock host,fallback;
    PhraseClock* selected=nullptr;
};
}
