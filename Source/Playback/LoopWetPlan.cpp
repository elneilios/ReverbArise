#include "LoopWetPlan.h"
#include <limits>

namespace m3
{
namespace
{
using PhaseRange = std::pair<int64_t, int64_t>;
using Stereo = std::array<double, 2>;

struct PeakPrefix
{
    Stereo sum{}, low{}, high{};
};

std::optional<double> measureFixedGridPeak(const LivePublication& audio, int64_t period,
    const std::vector<PhaseRange>& occupied, const RenderCancellation& cancelled)
{
    constexpr int64_t chunkFrames = 1024;
    std::array<PeakPrefix, chunkFrames> prefixes;
    std::array<Stereo, chunkFrames> pass;
    const auto firstCycle = audio.start / period - (audio.start % period < 0);
    const auto lastCycle = (audio.end - 1) / period;
    double peak = 0;
    for (const auto& [first, end] : occupied)
    {
        for (auto phase = first; phase < end;)
        {
            if (cancelled && cancelled()) return {};
            const auto frames = std::min(chunkFrames, end - phase);
            std::fill_n(prefixes.begin(), frames, PeakPrefix{});
            // The callback visits passes from oldest to newest. A Cue or a
            // finish can omit passes at either end, so keep prefix extrema.
            for (auto cycle = lastCycle; cycle >= firstCycle; --cycle)
            {
                const auto start = cycle * period + phase;
                const auto stop = start + frames;
                auto segment = std::upper_bound(audio.segments.begin(), audio.segments.end(), start,
                    [](int64_t at, const LiveSegment& item){ return at < item.end; });
                if (segment == audio.segments.end() || segment->start >= stop) continue;
                std::fill_n(pass.begin(), frames, Stereo{});
                for (; segment != audio.segments.end() && segment->start < stop; ++segment)
                {
                    const auto from = std::max(start, segment->start);
                    const auto to = std::min(stop, segment->end);
                    for (size_t index = 0; index < segment->count; ++index)
                    {
                        const auto& render = *audio.entries[segment->members[index]].audio;
                        for (auto at = from; at < to; ++at)
                        {
                            const auto offset = static_cast<size_t>(at - render.start) * 2;
                            auto& value = pass[static_cast<size_t>(at - start)];
                            value[0] += render.samples[offset];
                            value[1] += render.samples[offset + 1];
                        }
                    }
                }
                for (int64_t frame = 0; frame < frames; ++frame)
                {
                    auto& prefix = prefixes[static_cast<size_t>(frame)];
                    for (size_t channel = 0; channel < 2; ++channel)
                    {
                        const auto sum = prefix.sum[channel] += pass[static_cast<size_t>(frame)][channel];
                        peak = std::max({peak, sum - prefix.low[channel], prefix.high[channel] - sum});
                        prefix.low[channel] = std::min(prefix.low[channel], sum);
                        prefix.high[channel] = std::max(prefix.high[channel], sum);
                    }
                }
            }
            phase += frames;
        }
    }
    return peak;
}
}

bool LoopWetPlan::prepare(int64_t phraseFrames, uint64_t budget, juce::String& error,
                         const RenderCancellation& cancelled)
{
    if (phraseFrames <= 1 || phraseFrames > INT32_MAX || audio.entries.empty() || audio.entries.size() > maximumSwells)
    { error = "Loop wet has no valid prepared entries"; return false; }
    audio.start = 0;
    audio.end = 0;
    for (const auto& entry : audio.entries)
    {
        if (!entry.audio || entry.audio->start != entry.start || entry.end <= entry.start
            || static_cast<uint64_t>(entry.end) - static_cast<uint64_t>(entry.start) != entry.audio->frames())
        { error = "Loop wet contains incomplete prepared audio"; return false; }
        audio.start = std::min(audio.start,entry.start);
        audio.end = std::max(audio.end,entry.end);
    }

    const auto shortest = phraseFrames - 1;
    const auto range = shortest * LoopCueScheduler::anchorCapacity;
    if (audio.end > range || audio.start < -range)
    { error = "Loop wet extends beyond the supported cycle range"; return false; }
    const auto past = audio.end / shortest + 2;
    const auto future = -(audio.start / shortest) + 2;
    if (past >= LoopCueScheduler::anchorCapacity || future >= LoopCueScheduler::anchorCapacity
        || past + future + 1 > LoopCueScheduler::anchorCapacity)
    { error = "Loop wet extends beyond the supported cycle range"; return false; }
    pastCycles = static_cast<int>(past);
    futureCycles = static_cast<int>(future);

    // Fold conservatively padded support into one period. The padding accounts
    // for the distance from the current cycle, fractional timing and the hold.
    struct Edge { int64_t at; int count; double peak; };
    std::vector<Edge> edges;
    const auto margin = past + future + 2;
    int count = 0;
    double peak = 0;
    for (const auto& entry : audio.entries)
    {
        if (!std::isfinite(entry.peak) || entry.peak < 0)
        { error = "Loop wet contains invalid peak data"; return false; }
        const auto length = entry.end - entry.start + 2 * margin;
        const auto full = length / phraseFrames;
        if (full > static_cast<int64_t>(liveMaximumOverlap))
        { error = "Loop wet exceeds the simultaneous contribution limit"; return false; }
        count += static_cast<int>(full);
        peak += static_cast<double>(entry.peak) * full;
        const auto remaining = length % phraseFrames;
        if (!remaining) continue;
        auto first = (entry.start - margin) % phraseFrames;
        if (first < 0) first += phraseFrames;
        const auto end = first + remaining;
        if (end > phraseFrames)
        {
            ++count;
            peak += entry.peak;
            edges.push_back({end - phraseFrames,-1,-entry.peak});
        }
        else edges.push_back({end,-1,-entry.peak});
        edges.push_back({first,1,entry.peak});
    }
    std::sort(edges.begin(),edges.end(),[](const auto& a,const auto& b){
        return a.at == b.at ? a.count < b.count : a.at < b.at;
    });
    if (count > static_cast<int>(liveMaximumOverlap))
    { error = "Loop wet exceeds the simultaneous contribution limit"; return false; }
    double maximumPeak = peak;
    int maximumOverlap = count;
    int64_t occupiedStart = 0;
    std::vector<PhaseRange> occupied;
    for (const auto& edge : edges)
    {
        const auto before = count;
        count += edge.count;
        peak += edge.peak;
        if (count > static_cast<int>(liveMaximumOverlap))
        { error = "Loop wet exceeds the simultaneous contribution limit"; return false; }
        if (!before && count) occupiedStart = edge.at;
        if (before && !count && occupiedStart < edge.at) occupied.emplace_back(occupiedStart, edge.at);
        maximumOverlap = std::max(maximumOverlap, count);
        maximumPeak = std::max(maximumPeak,peak);
    }
    if (count && occupiedStart < phraseFrames) occupied.emplace_back(occupiedStart, phraseFrames);
    peakBound = static_cast<float>(maximumPeak);
    if (!prepareLiveSegments(audio,error)) return false;
    if (audio.bytes() > budget)
    { error = "Loop wet exceeds the shared PCM budget"; return false; }
    fixedGridPeak = peakBound;
    if (maximumOverlap > 1)
    {
        const auto measured = measureFixedGridPeak(audio, phraseFrames, occupied, cancelled);
        if (!measured) { error = "Loop wet preparation superseded"; return false; }
        // Keep headroom for float accumulation in the callback.
        const auto rounding = maximumPeak * std::numeric_limits<float>::epsilon() * maximumOverlap;
        fixedGridPeak = static_cast<float>(std::min(maximumPeak, *measured + rounding));
    }
    return true;
}
}
