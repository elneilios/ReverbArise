#pragma once

#include "LoopCueScheduler.h"
#include "TransportPlayback.h"
#include "LoopCycles.h"

namespace m3
{
struct LoopWetSample { float left = 0, right = 0; bool present = false; };

struct LoopWetPlan
{
    LivePublication audio;
    float peakBound = 0, fixedGridPeak = 0;
    int pastCycles = 0, futureCycles = 0;

    bool prepare(int64_t phraseFrames, uint64_t budget, juce::String& error,
                 const RenderCancellation& cancelled = {});
    float wetGain(float requested, float ceiling, bool fixedGrid) const noexcept
    {
        const auto peak = fixedGrid ? fixedGridPeak : peakBound;
        return peak > 0 ? std::min(requested, ceiling / peak) : requested;
    }

    template <typename PositionAt>
    LoopWetSample sample(int64_t near, int64_t first, int64_t last, PositionAt&& position) const noexcept
    {
        LoopWetSample wet;
        visitLoopCycles(near,first,last,pastCycles,futureCycles,[&](int64_t cycle)
        {
            const auto local = position(cycle);
            if (!local || *local < audio.start || *local >= audio.end) return;
            const auto it = std::upper_bound(audio.segments.begin(),audio.segments.end(),*local,
                [](int64_t at, const LiveSegment& segment){ return at < segment.start; });
            if (it == audio.segments.begin()) return;
            const auto& segment = *std::prev(it);
            if (*local >= segment.end) return;
            wet.present |= segment.count != 0;
            for (size_t index = 0; index < segment.count; ++index)
            {
                const auto& render = *audio.entries[segment.members[index]].audio;
                const auto offset = static_cast<size_t>(*local - render.start) * 2;
                wet.left += render.samples[offset];
                wet.right += render.samples[offset + 1];
            }
        });
        return wet;
    }
};
}
