#include "ExportState.h"
#include <cmath>
#include <set>
namespace m3
{
namespace
{
bool hash(const juce::String& s){return s.length()==64 && s.containsOnly("0123456789abcdef");}
bool percentage(float v){return std::isfinite(v) && v>=0 && v<=1;}
bool path(const juce::String& s)
{
    if(!s.startsWith("exports/") || s.length()>192)return false;
    const auto name=s.substring(8);
    return (name.startsWith("Swells-") || name.startsWith("Swells -")) && name.length()>4 && name.endsWith(".wav") && !name.contains("..")
        && name.dropLastCharacters(4).containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_ .");
}
juce::var object(){return new juce::DynamicObject;}
void put(juce::var& v,const char* key,const juce::var& value){v.getDynamicObject()->setProperty(key,value);}
}
bool validExportState(const ExportState& state,juce::String& error)
{
    if(!percentage(state.wet) || state.history.size()>maximumExportRevisions
        || (!state.desiredIdentity.isEmpty() && !hash(state.desiredIdentity))
        || (!state.initialized && (!state.history.empty() || state.desiredIdentity.isNotEmpty() || state.completedPath.isNotEmpty())))
    {error="Invalid Export preferences or revision capacity";return false;}
    std::set<juce::String> paths;bool completed=state.completedPath.isEmpty();
    for(const auto& f:state.history)
    {
        if(!path(f.path) || !paths.insert(f.path.toLowerCase()).second || !hash(f.composition) || !hash(f.checksum)
            || !percentage(f.wet) || !std::isfinite(f.rate) || f.rate<=0 || f.rate>768000
            || f.frames<=0 || static_cast<uint64_t>(f.frames)>(UINT32_MAX-1024ull)/8
            || f.origin>INT64_MAX-f.frames)
        {error="Invalid immutable Export revision";return false;}
        if(f.path==state.completedPath)completed=true;
    }
    if(!completed){error="Completed Export reference is not in immutable history";return false;}
    error.clear();return true;
}
juce::var exportStateMetadata(const ExportState& state)
{
    auto v=object();put(v,"initialized",state.initialized);put(v,"wet",state.wet);
    put(v,"desiredIdentity",state.desiredIdentity);put(v,"completedPath",state.completedPath);
    juce::Array<juce::var> files;
    for(const auto& f:state.history){auto a=object();put(a,"composition",f.composition);put(a,"checksum",f.checksum);put(a,"path",f.path);
        put(a,"rate",f.rate);put(a,"origin",juce::String(f.origin));put(a,"frames",juce::String(f.frames));put(a,"wet",f.wet);files.add(a);}
    put(v,"history",files);return v;
}
bool parseExportState(const juce::var& v,ExportState& output,juce::String& error)
{
    ExportState state;bool ok=v.isObject();
    const auto field=[&](const juce::var& a,const char* name){if(!a.isObject() || !a.hasProperty(name))ok=false;return a[name];};
    const auto text=[&](const juce::var& a,const char* name){const auto x=field(a,name);if(!x.isString())ok=false;return x.toString();};
    const auto number=[&](const juce::var& a,const char* name){const auto x=field(a,name);if(!x.isDouble() && !x.isInt() && !x.isInt64())ok=false;return static_cast<double>(x);};
    const auto big=[&](const juce::var& a,const char* name){const auto x=text(a,name);const auto n=x.getLargeIntValue();if(juce::String(n)!=x)ok=false;return n;};
    const auto initialized=field(v,"initialized");if(!initialized.isBool())ok=false;state.initialized=static_cast<bool>(initialized);
    const auto wet=number(v,"wet");if(!std::isfinite(wet) || wet<0 || wet>1)ok=false;state.wet=static_cast<float>(wet);state.desiredIdentity=text(v,"desiredIdentity");state.completedPath=text(v,"completedPath");
    const auto history=field(v,"history");if(!history.isArray() || history.size()>static_cast<int>(maximumExportRevisions)){error="Invalid Export history capacity";return false;}
    for(const auto& a:*history.getArray()){ExportFile f;f.composition=text(a,"composition");f.checksum=text(a,"checksum");f.path=text(a,"path");
        f.rate=number(a,"rate");f.origin=big(a,"origin");f.frames=big(a,"frames");const auto weight=number(a,"wet");if(!std::isfinite(weight) || weight<0 || weight>1)ok=false;f.wet=static_cast<float>(weight);state.history.push_back(std::move(f));}
    if(!ok){error="Malformed Export metadata";return false;}
    if(!validExportState(state,error))return false;
    output=std::move(state);return true;
}
}
