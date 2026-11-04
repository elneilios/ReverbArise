#include "AutomaticGeneration.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <tuple>

namespace m3
{
namespace
{
bool covered(const CompletedPasses& runs,int64_t first,int64_t last)
{
    if(first>=last)return false;
    const auto run=std::lower_bound(runs.begin(),runs.end(),first,[](const auto& interval,int64_t sample){return interval.second<=sample;});
    return run!=runs.end() && run->first<=first && run->second>=last;
}
long double separation(int64_t a,int64_t b)
{return std::abs(static_cast<long double>(a)-b);}
bool sameDefinition(const Swell& a,const Swell& b)
{
    return a.id==b.id && a.name==b.name && a.arrival==b.arrival && a.detectorAnchor==b.detectorAnchor
        && a.sourceStart==b.sourceStart && a.sourceEnd==b.sourceEnd && a.supportOffset==b.supportOffset
        && a.ir==b.ir && a.irName==b.irName && a.shape==b.shape && a.placement==b.placement
        && a.enabled==b.enabled && a.muted==b.muted && a.solo==b.solo && a.provenance==b.provenance;
}
}
AutomaticGenerationPlan prepareAutomaticGeneration(const Session& accepted,const std::vector<Candidate>& candidates,
    const CaptureManifest& source,const SongTime& time,const CompletedPasses& completed,
    const GenerationFingerprint& fingerprint,const std::function<bool()>& cancelled)
{
    AutomaticGenerationPlan plan;
    const auto stop=[&]{return cancelled && cancelled();};
    const auto fail=[&](const juce::String& message){plan.message=message;return plan;};
    if(stop()){plan.outcome=GenerationOutcome::cancelled;return plan;}
    juce::String error;
    if(!validate(accepted,error))return fail(error);
    const auto quarter=time.rate*60/time.bpm;
    if(!time.valid || source.sampleRate!=time.rate || !std::isfinite(quarter) || quarter<1
        || quarter>=static_cast<double>(INT64_MAX) || !accepted.defaultIR.present() || !fingerprint)
        return fail("Generate needs compatible source, fixed timing and a valid Default IR");
    if(!std::isfinite(accepted.separationSeconds) || accepted.separationSeconds<.001 || accepted.separationSeconds>2
        || candidates.size()>maximumSwells)return fail("Automatic generation capacity or settings are invalid");
    const auto tolerance=targetTolerance(time.rate,accepted.separationSeconds);
    CompletedPasses runs;
    for(const auto& range:source.ranges)
    {
        int64_t end;
        if(range.frameCount>INT64_MAX || !checkedAdd(range.startSample,static_cast<int64_t>(range.frameCount),end))
            return fail("Invalid captured coverage for generation");
        if(!runs.empty() && range.startSample<=runs.back().second)runs.back().second=std::max(runs.back().second,end);
        else runs.push_back({range.startSample,end});
    }
    std::set<juce::String> eligibleIds;
    for(const auto& w:accepted.swells)
    {
        int64_t anchorEnd;
        if(w.provenance==Provenance::automatic && !w.frozen
            && checkedAdd(w.detectorAnchor,1,anchorEnd) && covered(runs,w.detectorAnchor,anchorEnd)
            && covered(runs,w.sourceStart,renderInputEnd(w)))eligibleIds.insert(w.id);
    }
    const auto eligible=[&](const Swell& w){return eligibleIds.contains(w.id);};
    std::vector<int64_t> found;found.reserve(candidates.size());
    for(const auto& candidate:candidates)
    {
        int64_t end;
        if(!std::isfinite(candidate.strength))return fail("Invalid automatic candidate");
        if(checkedAdd(candidate.sample,1,end) && covered(runs,candidate.sample,end))found.push_back(candidate.sample);
    }
    std::sort(found.begin(),found.end());found.erase(std::unique(found.begin(),found.end()),found.end());
    if(found.empty()){plan.outcome=GenerationOutcome::empty;plan.message="No automatic markers found. Adjust sensitivity and try again.";return plan;}
    std::vector<int64_t> protectedAnchors=accepted.suppressed;
    for(const auto& w:accepted.swells)if(!eligible(w))
    {
        protectedAnchors.push_back(w.arrival);
        if(w.provenance!=Provenance::manualCreated)protectedAnchors.push_back(w.detectorAnchor);
    }
    std::sort(protectedAnchors.begin(),protectedAnchors.end());
    std::erase_if(found,[&](int64_t sample)
    {
        const auto at=std::lower_bound(protectedAnchors.begin(),protectedAnchors.end(),sample);
        return (at!=protectedAnchors.end() && separation(*at,sample)<=tolerance)
            || (at!=protectedAnchors.begin() && separation(*std::prev(at),sample)<=tolerance);
    });
    if(found.empty()){plan.outcome=GenerationOutcome::protectedOnly;plan.message="No eligible swells to generate; manual and frozen swells were preserved.";return plan;}
    struct Edge{long double distance;int64_t sample;juce::String id;size_t index;};
    std::vector<Edge> edges;
    for(const auto& w:accepted.swells)if(eligible(w))
    {
        const auto first=std::lower_bound(found.begin(),found.end(),static_cast<long double>(w.detectorAnchor)-tolerance,
            [](int64_t sample,long double lower){return sample<lower;});
        for(auto at=first;at!=found.end() && separation(w.detectorAnchor,*at)<=tolerance;++at)
        {
            if(edges.size()>=65536)return fail("Automatic target matching budget exceeded");
            edges.push_back({separation(w.detectorAnchor,*at),*at,w.id,static_cast<size_t>(at-found.begin())});
        }
    }
    std::sort(edges.begin(),edges.end(),[](const auto& a,const auto& b)
        {return std::tie(a.distance,a.sample,a.id)<std::tie(b.distance,b.sample,b.id);});
    std::vector<juce::String> matches(found.size());std::set<juce::String> matched;
    for(const auto& edge:edges)if(matches[edge.index].isEmpty() && !matched.contains(edge.id))
    {matches[edge.index]=edge.id;matched.insert(edge.id);}

    // The same automatic source-window/default-shape policy used by reconciliation
    // runs on a private arrangement. Restore every untouched definition afterward.
    Session working=accepted;
    std::erase_if(working.swells,[&](const auto& w){return eligible(w);});
    std::vector<juce::String> planned;
    for(size_t i=0;i<found.size();++i)
    {
        if(stop()){plan.outcome=GenerationOutcome::cancelled;return plan;}
        if(working.swells.size()>=maximumSwells)return fail("Session capacity: 4096 swells");
        Swell w;
        if(matches[i].isNotEmpty())w=*find(accepted,matches[i]);
        else {w.shape=initialShape(time.rate);w.name=defaultSwellName(found[i],time,&working);}
        w.arrival=w.detectorAnchor=w.sourceStart=found[i];w.sourceEnd=found[i]+1;w.supportOffset=0;
        w.provenance=Provenance::automatic;w.ir=accepted.defaultIR;w.irName=accepted.defaultIRName;
        planned.push_back(w.id);working.swells.push_back(std::move(w));
    }
    // Prune incomplete windows from right to left: removing a late target can
    // extend an earlier interval, so each preceding decision sees the final tail.
    const auto passes=usableCompletedPasses(source,completed);
    std::set<juce::String> viable(planned.begin(),planned.end());
    for(auto it=planned.rbegin();it!=planned.rend();++it)
    {
        auto* w=find(working,*it);int64_t end;
        if(!checkedAdd(w->arrival,static_cast<int64_t>(std::llround(quarter)),end))return fail("Automatic source position overflow");
        for(const auto& next:working.swells)if(next.arrival>w->arrival)end=std::min(end,next.arrival);
        for(const auto& pass:passes)if(pass.first<=w->arrival && w->arrival<pass.second)end=std::min(end,pass.second);
        if(!covered(runs,w->arrival,end))
        {
            viable.erase(*it);
            const auto old=find(accepted,*it);erase(working,*it);
            if(old)working.swells.push_back(*old);
        }
    }
    for(auto& w:working.swells)if(!viable.contains(w.id) && w.provenance==Provenance::automatic)
        w.provenance=Provenance::manualEdited; // Geometry sees its arrival, never edits its window.
    if(viable.empty())
    {plan.outcome=GenerationOutcome::empty;plan.message="No complete automatic source windows. Capture the missing audio and try again.";return plan;}
    if(!updateAutomaticWindows(working,time,error,passes))return fail(error);
    for(const auto& old:accepted.swells)
        if(!viable.contains(old.id))
            if(auto* w=find(working,old.id))*w=old;
    for(const auto& id:planned)
    {
        auto* w=find(working,id);if(!w || !viable.contains(id))continue;
        const auto* old=find(accepted,id);
        if(!covered(runs,w->sourceStart,renderInputEnd(*w))){if(old)*w=*old;else erase(working,id);continue;}
        juce::String dependency;
        if(!fingerprint(w->sourceStart,renderInputEnd(*w),dependency,error))return fail(error);
        if(stop()){plan.outcome=GenerationOutcome::cancelled;return plan;}
        const auto previous=old?(old->generationSource.isEmpty()?(old->render.present()?old->render.dependency:juce::String{}):old->generationSource):juce::String{};
        const auto changed=!old || !sameDefinition(*old,*w) || dependency!=previous;
        w->generationSource=dependency;
        if(changed)
        {
            w->inputGeneration=old?old->inputGeneration+1:1;
            w->calibration={};w->phase=Phase::checking;w->status="Queued after Generate Swells";
            plan.changed.push_back(id);
        }
        else if(old)*w=*old;
    }
    if(plan.changed.empty())
    {plan.outcome=GenerationOutcome::noChange;plan.message="No changes to generate. Existing swells were kept.";return plan;}
    for(const auto& old:accepted.swells)if(!find(working,old.id))plan.retired.push_back(old.id);
    std::erase_if(working.selected,[&](const auto& id){return !find(working,id);});
    if(!find(working,working.primary))working.primary=working.selected.empty()?juce::String{}:working.selected.front();
    working.timing=time;
    juce::String saved;
    if(!validate(working,error) || !encode(working,saved,error))return fail(error);
    plan.replacement=std::move(working);plan.outcome=GenerationOutcome::generated;
    plan.message=juce::String(static_cast<int>(plan.changed.size()))+" automatic swells generated. Opening Perform...";
    return plan;
}
}
