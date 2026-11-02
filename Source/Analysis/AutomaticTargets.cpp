#include "AutomaticTargets.h"
#include <set>
#include <tuple>
#include <cmath>

namespace m3
{
int analysisWindow(double rate) noexcept
{int order=8;while(order<13 && (1<<order)<rate*.021)++order;return 1<<order;}
int64_t targetTolerance(double rate,double separation) noexcept
{return std::max(int64_t{1},std::min(static_cast<int64_t>(analysisWindow(rate)),static_cast<int64_t>(rate*separation*.5)));}
static long double distance(int64_t a,int64_t b) {return std::abs(static_cast<long double>(a)-b);}
bool suppressAndErase(Session& s,const juce::String& id,juce::String& error)
{
    const auto w=find(s,id);if(!w)return false;
    if(w->provenance!=Provenance::manualCreated)
    {
        if(s.suppressed.size()>=maximumSwells){error="Suppression capacity reached; reset deleted targets in Diagnostics";return false;}
        s.suppressed.push_back(w->detectorAnchor);
    }
    error.clear();return erase(s,id);
}
CompletedPasses usableCompletedPasses(const CaptureManifest& map,const CompletedPasses& passes)
{
    CompletedPasses result;
    for(const auto& pass:passes){std::optional<int64_t> covered;
        for(const auto& range:map.ranges){int64_t end;
            if(range.frameCount>INT64_MAX || !checkedAdd(range.startSample,static_cast<int64_t>(range.frameCount),end))continue;
            if(!covered && range.startSample<pass.second && end>=pass.second)covered=end;
            else if(covered && range.startSample<=*covered)covered=std::max(*covered,end);
            else if(covered)break;}
        // Integer sample and PPQ endpoints may round independently by one frame.
        // A genuine later extension still removes an inferred extraction cap.
        if(!covered || *covered<=pass.second || (pass.second<INT64_MAX && *covered==pass.second+1))result.push_back(pass);
    }
    return result;
}
bool updateAutomaticWindows(Session& s,const SongTime& time,juce::String& error,const CompletedPasses& completed)
{
    const auto quarter=time.rate*60/time.bpm;
    if(!time.valid || !std::isfinite(quarter) || quarter<1 || quarter>=static_cast<double>(INT64_MAX))
    {error="Valid fixed timing required for automatic source windows";return false;}
    for(auto& w:s.swells)if(w.provenance==Provenance::automatic)
    {
        int64_t end;if(!checkedAdd(w.arrival,static_cast<int64_t>(std::llround(quarter)),end))
        {error="Automatic source position overflow";return false;}
        for(const auto& next:s.swells)if(next.arrival>w.arrival)end=std::min(end,next.arrival);
        int64_t boundary=INT64_MAX;
        for(const auto& pass:completed)if(pass.first<=w.arrival && w.arrival<pass.second){end=std::min(end,pass.second);boundary=std::min(boundary,pass.second);}
        auto shape=w.shape;const auto defaults=initialShape(time.rate);
        // Recognise only the untouched modern default (or a previous boundary
        // fit). Older saved transitions and all manual ownership stay intact.
        auto check=shape;check.endOffset=defaults.endOffset;check.fadeOut=defaults.fadeOut;
        if(check==defaults && shape.endOffset>0 && shape.endOffset<=defaults.endOffset && shape.fadeOut==shape.endOffset){
            shape=defaults;int64_t wetEnd;
            if(!checkedAdd(w.arrival,shape.endOffset,wetEnd)){error="Automatic wet endpoint overflow";return false;}
            if(wetEnd>boundary){shape.endOffset=boundary-w.arrival;shape.fadeOut=shape.endOffset;}}
        if(shape!=w.shape){w.shape=shape;++w.shapeGeneration;}
        if(w.sourceStart!=w.arrival || w.sourceEnd!=end)
        {w.sourceStart=w.arrival;w.sourceEnd=end;w.supportOffset=0;++w.inputGeneration;w.phase=Phase::stale;w.status="Waiting for source";}
    }
    error.clear();return true;
}
bool reconcileTargets(Session& session,const std::vector<Candidate>& candidates,int64_t start,int64_t end,
                      const SongTime& time,juce::String& error,const CompletedPasses& completed)
{
    if(start>=end){error="Invalid reconciliation interval";return false;}
    auto s=session;const auto tolerance=targetTolerance(time.rate,s.separationSeconds);
    std::vector<Candidate> found;
    for(const auto& c:candidates)if(c.sample>=start && c.sample<end)found.push_back(c);
    std::sort(found.begin(),found.end(),[](const auto& a,const auto& b){return a.sample<b.sample;});
    found.erase(std::unique(found.begin(),found.end(),[](const auto& a,const auto& b){return a.sample==b.sample;}),found.end());
    if(found.size()>maximumSwells){error="Detector capacity: 4096 targets per interval";return false;}
    struct Edge {long double distance;int64_t sample;juce::String id;size_t candidate;};
    std::vector<Edge> edges;
    for(size_t i=0;i<found.size();++i)for(const auto& w:s.swells)
    {
        const auto anchor=w.provenance==Provenance::manualCreated?w.arrival:w.detectorAnchor;
        const auto d=distance(anchor,found[i].sample);
        if(d<=tolerance)
        {if(edges.size()>=65536){error="Target reconciliation budget exceeded";return false;}edges.push_back({d,found[i].sample,w.id,i});}
    }
    std::sort(edges.begin(),edges.end(),[](const auto& a,const auto& b)
    {return std::tie(a.distance,a.sample,a.id)<std::tie(b.distance,b.sample,b.id);});
    std::set<juce::String> matched;std::set<size_t> used;
    for(const auto& e:edges)if(!matched.count(e.id) && !used.count(e.candidate))
    {
        matched.insert(e.id);used.insert(e.candidate);auto& w=*find(s,e.id);
        if(w.provenance==Provenance::automatic && w.arrival!=e.sample)
        {w.arrival=e.sample;w.detectorAnchor=e.sample;++w.inputGeneration;w.phase=Phase::stale;w.status="Detected arrival updated";}
    }
    // Protected anchors also suppress unmatched neighboring detector candidates.
    for(size_t i=0;i<found.size();++i)if(!used.count(i))
    {
        bool suppressed=false;
        for(auto anchor:s.suppressed)if(distance(anchor,found[i].sample)<=tolerance)suppressed=true;
        for(const auto& w:s.swells)if(w.provenance!=Provenance::automatic
            && distance(w.provenance==Provenance::manualCreated?w.arrival:w.detectorAnchor,found[i].sample)<=tolerance)suppressed=true;
        if(suppressed)continue;
        if(s.swells.size()>=maximumSwells){error="Session capacity: 4096 swells";return false;}
        Swell w;w.arrival=w.sourceStart=w.detectorAnchor=found[i].sample;
        if(!checkedAdd(w.arrival,1,w.sourceEnd)){error="Target overflow";return false;}
        w.shape=initialShape(time.rate);w.provenance=Provenance::automatic;w.ir=s.defaultIR;w.irName=s.defaultIRName;
        w.name=defaultSwellName(w.arrival,time);w.phase=Phase::stale;w.status="Waiting for source";
        matched.insert(w.id);s.swells.push_back(std::move(w));
    }
    std::vector<juce::String> retired;
    for(const auto& w:s.swells)if(w.provenance==Provenance::automatic && w.detectorAnchor>=start && w.detectorAnchor<end && !matched.count(w.id))retired.push_back(w.id);
    for(const auto& id:retired)erase(s,id);
    if(!updateAutomaticWindows(s,time,error,completed) || !validate(s,error))return false;
    s.timing=time;session=std::move(s);error.clear();return true;
}
}
