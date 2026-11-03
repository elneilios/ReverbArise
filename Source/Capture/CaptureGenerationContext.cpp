#include "CaptureManifest.h"
#include <Core/ContentHash.h>
#include <algorithm>
#include <cmath>
#include <set>

namespace
{
juce::var contextValue(const CaptureGenerationContext& c,bool includeId)
{
    juce::var value(new juce::DynamicObject());auto& p=*value.getDynamicObject();
    if(includeId)p.setProperty("id",c.id);
    p.setProperty("origin",c.origin==CaptureContextOrigin::pass?"pass":"legacy");
    p.setProperty("automatic",c.automatic);p.setProperty("sensitivity",static_cast<double>(c.sensitivity));
    p.setProperty("separationSeconds",c.separationSeconds);p.setProperty("ir",m3::assetMetadata(c.ir));p.setProperty("irName",c.irName);
    return value;
}
bool fail(juce::String& error,const char* text){error=text;return false;}
bool number(const juce::var& value,double& output)
{
    if(!value.isInt() && !value.isInt64() && !value.isDouble())return false;
    output=static_cast<double>(value);return std::isfinite(output);
}
bool integer(const juce::var& value,int64_t& output)
{
    if(!value.isInt() && !value.isInt64())return false;
    output=static_cast<int64_t>(value);return true;
}
bool validContext(const CaptureGenerationContext& c,juce::String& error)
{
    if((c.origin!=CaptureContextOrigin::pass && c.origin!=CaptureContextOrigin::legacy)
        || !std::isfinite(c.sensitivity) || c.sensitivity<0 || c.sensitivity>1
        || !std::isfinite(c.separationSeconds) || c.separationSeconds<.001 || c.separationSeconds>2
        || c.irName.length()>1024 || c.irName.containsChar('\0'))
        return fail(error,"Invalid generation context controls, origin or label");
    m3::Asset checked;
    if(!m3::parseAssetMetadata(m3::assetMetadata(c.ir),checked,error))return false;
    if(c.ir.present()?!c.ir.path.startsWith("irs/"):c.ir!=m3::Asset{})
        return fail(error,"Generation context requires a managed IR or an explicit empty reference");
    if(c.id!=captureGenerationContextId(c))return fail(error,"Generation context identity does not match its content");
    return true;
}
}

juce::String captureGenerationContextId(const CaptureGenerationContext& context)
{
    const auto canonical=juce::JSON::toString(contextValue(context,false),true);
    m3::ContentHash hash;hash.update(canonical.toRawUTF8(),canonical.getNumBytesAsUTF8());return hash.finish();
}

juce::var captureContextMetadata(const CaptureManifest& manifest)
{
    juce::var value(new juce::DynamicObject());juce::Array<juce::var> contexts,spans;
    for(const auto& c:manifest.generationContexts)contexts.add(contextValue(c,true));
    for(const auto& span:manifest.contextSpans)
    {
        juce::var item(new juce::DynamicObject());auto& p=*item.getDynamicObject();
        p.setProperty("start",span.start);p.setProperty("end",span.end);p.setProperty("context",span.context);spans.add(item);
    }
    value.getDynamicObject()->setProperty("generationContexts",contexts);value.getDynamicObject()->setProperty("contextSpans",spans);return value;
}

bool validateCaptureContexts(const CaptureManifest& manifest,juce::String& error)
{
    const auto& contexts=manifest.generationContexts;const auto& spans=manifest.contextSpans;
    if(manifest.schemaVersion==1)
        return contexts.empty() && spans.empty()?true:fail(error,"Schema 1 cannot carry generation context metadata");
    if(contexts.size()>maximumCaptureContexts || spans.size()>maximumCaptureContextSpans)
        return fail(error,"Capture generation context capacity exceeded");
    std::set<juce::String> identities;
    for(const auto& c:contexts)
    {
        if(!validContext(c,error))return false;
        if(!identities.insert(c.id).second)return fail(error,"Duplicate generation context identity");
    }
    for(size_t i=0;i<spans.size();++i)
    {
        const auto& s=spans[i];
        if(s.start>=s.end || !identities.contains(s.context)
            || (i && (spans[i-1].end>s.start || (spans[i-1].end==s.start && spans[i-1].context==s.context))))
            return fail(error,"Context spans must have unique ordered ownership and coalesced neighbours");
    }
    // Compare coverage unions, not audio-chunk boundaries. A context may span
    // adjacent audio/silence chunks, but cannot cover a seek gap or omit source.
    for(const auto& r:manifest.ranges)
        if(!r.frameCount || r.frameCount>INT64_MAX || r.startSample>INT64_MAX-static_cast<int64_t>(r.frameCount))
            return fail(error,"Invalid source bounds for generation context ownership");
    size_t r=0,s=0;int64_t source=manifest.ranges.empty()?0:manifest.ranges[0].startSample;
    int64_t context=spans.empty()?0:spans[0].start;
    while(r<manifest.ranges.size() && s<spans.size())
    {
        if(source!=context)return fail(error,"Generation context ownership must exactly cover mapped source");
        const auto sourceEnd=manifest.ranges[r].startSample+static_cast<int64_t>(manifest.ranges[r].frameCount);
        const auto cut=std::min(sourceEnd,spans[s].end);source=context=cut;
        if(cut==sourceEnd && ++r<manifest.ranges.size())source=manifest.ranges[r].startSample;
        if(cut==spans[s].end && ++s<spans.size())context=spans[s].start;
    }
    if(r!=manifest.ranges.size() || s!=spans.size())return fail(error,"Generation context ownership is missing or extends outside source");
    if(juce::JSON::toString(captureContextMetadata(manifest),true).getNumBytesAsUTF8()>maximumCaptureContextBytes)
        return fail(error,"Capture generation context metadata exceeds 8 MiB");
    error.clear();return true;
}

bool parseCaptureContexts(const juce::var& root,CaptureManifest& manifest,juce::String& error)
{
    const auto* definitions=root["generationContexts"].getArray();const auto* spans=root["contextSpans"].getArray();
    if(!definitions || !spans || definitions->size()>static_cast<int>(maximumCaptureContexts)
        || spans->size()>static_cast<int>(maximumCaptureContextSpans))return fail(error,"Invalid or excessive capture context tables");
    juce::var metadata(new juce::DynamicObject());metadata.getDynamicObject()->setProperty("generationContexts",*definitions);metadata.getDynamicObject()->setProperty("contextSpans",*spans);
    if(juce::JSON::toString(metadata,true).getNumBytesAsUTF8()>maximumCaptureContextBytes)return fail(error,"Capture generation context metadata exceeds 8 MiB");
    auto next=manifest;next.generationContexts.clear();next.contextSpans.clear();
    for(const auto& item:*definitions)
    {
        CaptureGenerationContext c;double sensitivity=0;
        if(!item.isObject() || item.getDynamicObject()->getProperties().size()!=7 || !item["id"].isString() || !item["origin"].isString() || !item["automatic"].isBool()
            || !item["irName"].isString() || !number(item["sensitivity"],sensitivity) || sensitivity<0 || sensitivity>1 || !number(item["separationSeconds"],c.separationSeconds)
            || !m3::parseAssetMetadata(item["ir"],c.ir,error))return fail(error,"Invalid generation context fields");
        const auto origin=item["origin"].toString();if(origin!="pass" && origin!="legacy")return fail(error,"Unknown generation context origin");
        c.id=item["id"].toString();c.origin=origin=="pass"?CaptureContextOrigin::pass:CaptureContextOrigin::legacy;
        c.automatic=static_cast<bool>(item["automatic"]);c.sensitivity=static_cast<float>(sensitivity);c.irName=item["irName"].toString();
        next.generationContexts.push_back(std::move(c));
    }
    for(const auto& item:*spans)
    {
        CaptureContextSpan span;
        if(!item.isObject() || item.getDynamicObject()->getProperties().size()!=3 || !integer(item["start"],span.start) || !integer(item["end"],span.end) || !item["context"].isString())return fail(error,"Invalid generation context span fields");
        span.context=item["context"].toString();next.contextSpans.push_back(std::move(span));
    }
    if(!isValidCaptureManifest(next,error))return false;
    manifest=std::move(next);return true;
}

bool captureWithLegacyContext(const CaptureManifest& source,CaptureGenerationContext baseline,CaptureManifest& output,juce::String& error)
{
    if(!isValidCaptureManifest(source,error))return false;
    auto next=source;
    if(source.schemaVersion==1)
    {
        next.schemaVersion=2;baseline.origin=CaptureContextOrigin::legacy;baseline.id=captureGenerationContextId(baseline);
        if(!source.ranges.empty())next.generationContexts.push_back(baseline);
        for(const auto& r:source.ranges)
        {
            const auto end=r.startSample+static_cast<int64_t>(r.frameCount);
            if(!next.contextSpans.empty() && next.contextSpans.back().end==r.startSample)next.contextSpans.back().end=end;
            else next.contextSpans.push_back({r.startSample,end,baseline.id});
        }
    }
    if(!isValidCaptureManifest(next,error))return false;
    output=std::move(next);return true;
}

const CaptureGenerationContext* captureContextAt(const CaptureManifest& manifest,int64_t arrival)
{
    const auto at=std::upper_bound(manifest.contextSpans.begin(),manifest.contextSpans.end(),arrival,[](int64_t p,const auto& s){return p<s.start;});
    if(at==manifest.contextSpans.begin())return nullptr;const auto& span=*std::prev(at);if(arrival>=span.end)return nullptr;
    const auto context=std::find_if(manifest.generationContexts.begin(),manifest.generationContexts.end(),[&](const auto& c){return c.id==span.context;});
    return context==manifest.generationContexts.end()?nullptr:&*context;
}

bool captureWithContext(const CaptureManifest& source,const CaptureGenerationContext& context,int64_t start,int64_t end,CaptureManifest& output,juce::String& error)
{
    if(start>=end || source.schemaVersion!=2 || !validContext(context,error))return fail(error,"Invalid pass generation context assignment");
    auto next=source;next.contextSpans.clear();
    for(const auto& span:source.contextSpans)
    {
        if(span.end<=start || span.start>=end)next.contextSpans.push_back(span);
        else
        {
            if(span.start<start)next.contextSpans.push_back({span.start,start,span.context});
            if(span.end>end)next.contextSpans.push_back({end,span.end,span.context});
        }
    }
    next.contextSpans.push_back({start,end,context.id});
    std::sort(next.contextSpans.begin(),next.contextSpans.end(),[](const auto& a,const auto& b){return a.start<b.start;});
    std::vector<CaptureContextSpan> merged;std::set<juce::String> used;
    for(const auto& span:next.contextSpans)
    {
        used.insert(span.context);
        if(!merged.empty() && merged.back().end==span.start && merged.back().context==span.context)merged.back().end=span.end;
        else merged.push_back(span);
    }
    next.contextSpans=std::move(merged);
    std::erase_if(next.generationContexts,[&](const auto& c){return !used.contains(c.id);});
    if(std::none_of(next.generationContexts.begin(),next.generationContexts.end(),[&](const auto& c){return c.id==context.id;}))next.generationContexts.push_back(context);
    if(!isValidCaptureManifest(next,error))return false;
    output=std::move(next);return true;
}

bool captureAllowsAutomatic(const CaptureManifest& map,int64_t start,int64_t end)
{
    if(map.schemaVersion<2)return true;
    for(const auto& span:map.contextSpans){if(span.start>=end)break;if(span.end<=start)continue;
        const auto* owner=captureContextAt(map,span.start);
        if(!owner || (owner->origin==CaptureContextOrigin::pass && !owner->automatic))return false;}
    return true;
}
