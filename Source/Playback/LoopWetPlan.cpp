#include "LoopWetPlan.h"

namespace m3
{
bool LoopWetPlan::prepare(int64_t phraseFrames, uint64_t budget, juce::String& error)
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
    for (const auto& edge : edges)
    {
        count += edge.count;
        peak += edge.peak;
        if (count > static_cast<int>(liveMaximumOverlap))
        { error = "Loop wet exceeds the simultaneous contribution limit"; return false; }
        maximumPeak = std::max(maximumPeak,peak);
    }
    peakBound = static_cast<float>(maximumPeak);
    if (!prepareLiveSegments(audio,error)) return false;
    if (audio.bytes() > budget)
    { error = "Loop wet exceeds the shared PCM budget"; return false; }
    return true;
}
}
