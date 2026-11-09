#include "LearnCandidates.h"
#include <algorithm>

namespace m3
{
bool scanLearnCandidates(CaptureAssetStore& store,const CaptureManifest& source,const CandidateIdentity& identity,
    const CompletedPasses& completed,LearnCandidateSnapshot& output,juce::String& error,
    const std::function<bool()>& cancelled)
{
    const bool compatible = source.loop
        ? identity.loop==source.loop && !identity.provisional
        : !identity.loop && identity.timing.valid && source.sampleRate==identity.timing.rate;
    if(!compatible
        || !std::isfinite(identity.sensitivity) || identity.sensitivity<0 || identity.sensitivity>1
        || !std::isfinite(identity.separationSeconds) || identity.separationSeconds<.001 || identity.separationSeconds>2)
    {error="Candidate analysis needs compatible captured source and host timing";return false;}
    LearnCandidateSnapshot prepared;prepared.identity=identity;
    if(!source.loop)prepared.completedPasses=usableCompletedPasses(source,completed);
    for(const auto& range:source.ranges)
    {
        int64_t end;
        if(!range.frameCount || range.frameCount>INT64_MAX
            || !checkedAdd(range.startSample,static_cast<int64_t>(range.frameCount),end))
        {error="Invalid captured range for candidate analysis";return false;}
        if(!prepared.mappedRuns.empty() && range.startSample<=prepared.mappedRuns.back().second)
            prepared.mappedRuns.back().second=std::max(prepared.mappedRuns.back().second,end);
        else prepared.mappedRuns.push_back({range.startSample,end});
    }
    for(const auto& [first,last]:prepared.mappedRuns)
    {
        if(cancelled && cancelled()){error="Candidate analysis superseded";return false;}
        int64_t end=last;
        if(identity.provisional)
        {
            int64_t safe;
            if(!checkedSubtract(last,analysisWindow(identity.timing.rate),safe))continue;
            end=std::max(first,safe);
        }
        if(first>=end)continue;
        std::vector<Candidate> detected;
        if(!suggestOnsets(store,source,first,end,identity.sensitivity,identity.separationSeconds,detected,error,cancelled))return false;
        if(error.isNotEmpty())return false; // A missing mapped asset is not a smaller settled candidate set.
        if(detected.size()>maximumSwells-prepared.positions.size())
        {error="Candidate capacity: 4096 markers per arrangement";return false;}
        prepared.positions.insert(prepared.positions.end(),detected.begin(),detected.end());
    }
    if(cancelled && cancelled()){error="Candidate analysis superseded";return false;}
    std::sort(prepared.positions.begin(),prepared.positions.end(),[](const auto& a,const auto& b){return a.sample<b.sample;});
    prepared.positions.erase(std::unique(prepared.positions.begin(),prepared.positions.end(),[](const auto& a,const auto& b)
        {return a.sample==b.sample;}),prepared.positions.end());
    output=std::move(prepared);error.clear();return true;
}
}
