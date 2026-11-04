#include "SongExport.h"
#include <Core/ContentHash.h>
#include <Core/TransportTime.h>
#include <algorithm>

namespace m3
{
namespace
{
juce::var object(){return new juce::DynamicObject;}
void put(juce::var& v,const char* key,const juce::var& value){v.getDynamicObject()->setProperty(key,value);}
}
juce::String songExportIdentity(const SongExportPlan& plan)
{
    // Reuse the canonical, validated swell serializer, but retain only the
    // accepted members' musical definitions. Selection and monitor/live fields
    // cannot influence the resulting audio identity.
    Session definition;definition.timing=plan.timing;definition.gainPolicy=plan.gainPolicy;definition.gainRevision=plan.gainRevision;
    for(const auto& member:plan.members)definition.swells.push_back(member.accepted);
    juce::String encoded,error,source;
    if(!encode(definition,encoded,error) || !serializeCaptureManifest(plan.source,source,error))return {};
    auto values=juce::JSON::parse(encoded)["swells"];
    for(auto& v:*values.getArray())for(const char* key:{"name","irName","provenance","detectorAnchor","generationSource"})v.getDynamicObject()->removeProperty(key);
    auto metadata=object();put(metadata,"revision",songExportRevision);put(metadata,"members",values);
    put(metadata,"source",source);put(metadata,"sourceReference",plan.sourceReference);
    put(metadata,"origin",juce::String(plan.origin));put(metadata,"end",juce::String(plan.end));
    put(metadata,"rate",plan.timing.rate);put(metadata,"bpm",plan.timing.bpm);put(metadata,"numerator",plan.timing.numerator);put(metadata,"denominator",plan.timing.denominator);
    put(metadata,"wet",plan.wet);put(metadata,"gainPolicy",static_cast<int>(plan.gainPolicy));put(metadata,"gainRevision",plan.gainRevision);
    juce::Array<juce::var> warnings;
    for(const auto& warning:plan.warnings){auto v=object();put(v,"start",juce::String(warning.start));put(v,"end",juce::String(warning.end));put(v,"kind",static_cast<int>(warning.kind));warnings.add(v);}
    put(metadata,"warnings",warnings);
    const auto text=juce::JSON::toString(metadata,true);ContentHash hash;hash.update(text.toRawUTF8(),text.getNumBytesAsUTF8());return hash.finish();
}
bool prepareSongExportPlan(const Session& session,const CaptureManifest& source,const juce::String& reference,
                           const SongTime& time,float wet,std::shared_ptr<const SongExportPlan>& output,juce::String& error)
{
    long double origin;
    if(!std::isfinite(wet) || wet<0 || wet>1 || !songOrigin(time,origin)
        || !transportMappingMatches(session.timing,time)) {error="Export needs a valid mix and compatible song timing";return false;}
    if(!validate(session,error) || !isValidCaptureManifest(source,error))return false;
    if(!source.ranges.empty() && source.sampleRate!=time.rate){error="Export source sample rate is incompatible";return false;}
    auto plan=std::make_shared<SongExportPlan>();plan->source=source;plan->sourceReference=reference;plan->timing=time;
    plan->wet=wet;plan->gainPolicy=session.gainPolicy;plan->gainRevision=session.gainRevision;
    if(!resolvePpqSample(time,0,plan->origin)){error="Export origin overflows";return false;}plan->end=plan->origin;
    const bool solo=std::any_of(session.swells.begin(),session.swells.end(),[](const auto& w){return w.solo;});
    for(const auto& w:session.swells)
    {
        if(!w.enabled || w.muted || (solo && !w.solo))continue;
        if(!hasCompatibleReadyRender(w,time)){error="Export unavailable: "+w.name+" ("+w.id+") has no compatible accepted render";return false;}
        int64_t a,b;
        if(!shapeBounds(w,w.render.start,w.render.sourceEnd,a,b,error)) {error="Export bounds invalid: "+w.id+" | "+error;return false;}
        plan->members.push_back({w,a,b});plan->end=std::max(plan->end,b);
    }
    if(plan->members.empty()){error="Export needs an enabled accepted wet render";return false;}
    std::sort(plan->members.begin(),plan->members.end(),[](const auto& a,const auto& b){return a.accepted.id<b.accepted.id;});
    for(const auto& r:source.ranges){int64_t end;
        if(r.frameCount>INT64_MAX || !checkedAdd(r.startSample,static_cast<int64_t>(r.frameCount),end)){error="Export source extent overflows";return false;}
        plan->end=std::max(plan->end,end);}
    int64_t frames;
    if(!checkedSubtract(plan->end,plan->origin,frames) || frames<=0){error="Export has no nonnegative song interval";return false;}
    // The writer verifies the exact RIFF header allowance; reject unsupported
    // ranges conservatively before any whole-song work is admitted.
    if(static_cast<uint64_t>(frames)>(UINT32_MAX-1024ull)/8){error="Export exceeds supported WAV size";return false;}
    plan->identity=songExportIdentity(*plan);
    if(plan->identity.isEmpty()){error="Invalid export dependency metadata";return false;}
    output=std::move(plan);error.clear();return true;
}
}
