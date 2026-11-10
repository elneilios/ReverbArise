#pragma once
#include "SongTime.h"

// One coherent host observation per audio block. Declared loop bounds use
// absolute host samples.
struct TransportTime
{
    SongTime song;
    bool playing=false, looping=false;
    int64_t loopStart=0,loopEnd=0;
};

inline bool sameDeclaredRepeat(const TransportTime& previous, const TransportTime& current) noexcept
{
    return previous.looping && current.looping && current.loopStart < current.loopEnd
        && previous.loopStart == current.loopStart && previous.loopEnd == current.loopEnd;
}

inline bool isDeclaredRepeatWrap(const TransportTime& previous, int64_t previousEnd, const TransportTime& current) noexcept
{
    return sameDeclaredRepeat(previous, current) && previousEnd == current.loopEnd
        && current.song.sample == current.loopStart;
}

inline bool resolvePpqSample(const SongTime& t,double ppq,int64_t& sample) noexcept
{
    if(!t.valid || !std::isfinite(ppq) || !std::isfinite(t.ppq) || !std::isfinite(t.rate) || t.rate<=0 || !std::isfinite(t.bpm) || t.bpm<=0)return false;
    const auto value=std::round(static_cast<long double>(t.sample)+(static_cast<long double>(ppq)-t.ppq)*t.rate*60/t.bpm);
    if(!std::isfinite(value) || value< -9223372036854775808.0L || value>=9223372036854775808.0L)return false;
    sample=static_cast<int64_t>(value);return true;
}

inline bool songOrigin(const SongTime& t, long double& origin) noexcept
{
    if(!t.valid || !std::isfinite(t.rate) || t.rate<=0 || !std::isfinite(t.bpm) || t.bpm<=0
        || !std::isfinite(t.ppq) || t.numerator<=0 || t.denominator<=0) return false;
    origin=static_cast<long double>(t.sample)-static_cast<long double>(t.ppq)*t.rate*60/t.bpm;
    return std::isfinite(origin) && origin>=-9223372036854775808.0L && origin<9223372036854775808.0L;
}

// Host sample positions are rounded integers; two independent observations can
// differ by one sample in their inferred origin. Larger drift is unsupported.
inline bool transportMappingMatches(const SongTime& prepared,const SongTime& now) noexcept
{
    long double a,b;
    return sameMusicalMapping(prepared,now) && songOrigin(prepared,a) && songOrigin(now,b) && std::abs(a-b)<=1.0L;
}
