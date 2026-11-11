#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>

namespace m3
{
// Exact fixed-period anchors shared by playback and finite export. Never add
// rounded beat intervals incrementally or allocate expanded repeat buffers.
inline std::optional<int64_t> loopCycleAnchor(int64_t origin,int64_t cycle,int64_t period) noexcept
{
    if(cycle<0 || period<=0 || cycle>INT64_MAX/period)return {};
    const auto offset=cycle*period;
    if(origin>INT64_MAX-offset)return {};
    return origin+offset;
}
inline int64_t loopFloorCycle(int64_t sample,int64_t period) noexcept
{return sample/period-(sample%period<0);}

template<class Visitor>
void visitLoopCycles(int64_t near,int64_t first,int64_t last,int64_t past,int64_t future,Visitor&& visit)
{
    if(past<0 || future<0 || first>last)return;
    const auto low=std::max(first,near<INT64_MIN+past?INT64_MIN:near-past);
    const auto high=std::min(last,near>INT64_MAX-future?INT64_MAX:near+future);
    if(low>high)return;
    for(auto cycle=low;;++cycle){visit(cycle);if(cycle==high)break;}
}
}
