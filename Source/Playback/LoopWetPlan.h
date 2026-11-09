#pragma once

#include "LoopCueScheduler.h"
#include "TransportPlayback.h"

namespace m3
{
struct LoopWetSample { float left = 0, right = 0; };

struct LoopWetPlan
{
    LivePublication audio;
    float peakBound = 0;
    int pastCycles = 0, futureCycles = 0;

    bool prepare(int64_t phraseFrames, uint64_t budget, juce::String& error);

    template <typename PositionAt>
    LoopWetSample sample(int64_t near, int64_t first, int64_t last, PositionAt&& position) const noexcept
    {
        LoopWetSample wet;
        const auto low = std::max(first,near - pastCycles);
        const auto high = std::min(last,near + futureCycles);
        for (auto cycle = low; cycle <= high; ++cycle)
        {
            const auto local = position(cycle);
            if (!local || *local < audio.start || *local >= audio.end) continue;
            const auto it = std::upper_bound(audio.segments.begin(),audio.segments.end(),*local,
                [](int64_t at, const LiveSegment& segment){ return at < segment.start; });
            if (it == audio.segments.begin()) continue;
            const auto& segment = *std::prev(it);
            if (*local >= segment.end) continue;
            for (size_t index = 0; index < segment.count; ++index)
            {
                const auto& render = *audio.entries[segment.members[index]].audio;
                const auto offset = static_cast<size_t>(*local - render.start) * 2;
                wet.left += render.samples[offset];
                wet.right += render.samples[offset + 1];
            }
        }
        return wet;
    }
};
}
