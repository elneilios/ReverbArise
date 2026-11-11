#include "ExportState.h"
#include <cmath>
#include <set>
namespace m3
{
namespace
{
bool hash(const juce::String& s){return s.length()==64 && s.containsOnly("0123456789abcdef");}
bool percentage(float v){return std::isfinite(v) && v>=0 && v<=1;}
bool loopOptions(LoopExportOptions v){return v.repeats>=1 && v.repeats<=LoopExportOptions::maximumRepeats && v.gridBeats>=1;}
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
    if(!loopOptions(state.loop.options) || !percentage(state.loop.levels.dry) || !percentage(state.loop.levels.wet)
        || (!state.loop.initialized && state.loop!=LoopExportPreferences{}))
    {error="Invalid Loop Export preferences";return false;}
    if(!percentage(state.wet) || !percentage(state.dry) || state.history.size()>maximumExportRevisions
        || (!state.desiredIdentity.isEmpty() && !hash(state.desiredIdentity))
        || (!state.initialized && (!state.history.empty() || state.desiredIdentity.isNotEmpty() || state.completedPath.isNotEmpty())))
    {error="Invalid Export preferences or revision capacity";return false;}
    std::set<juce::String> paths;bool completed=state.completedPath.isEmpty();
    for(const auto& f:state.history)
    {
        if(f.loop && (!loopOptions(f.loop->options) || f.loop->periodFrames<=0 || f.gainLaw!=2 || f.origin>0
            || f.loop->periodFrames>INT64_MAX/f.loop->options.repeats))
        {error="Invalid immutable Loop Export descriptor";return false;}
        if(!path(f.path) || !paths.insert(f.path.toLowerCase()).second || !hash(f.composition) || !hash(f.checksum)
            || !percentage(f.wet) || !percentage(f.dry) || (f.gainLaw!=1 && f.gainLaw!=2) || !std::isfinite(f.rate) || f.rate<=0 || f.rate>768000
            || f.frames<=0 || static_cast<uint64_t>(f.frames)>(UINT32_MAX-1024ull)/8
            || f.origin>INT64_MAX-f.frames)
        {error="Invalid immutable Export revision";return false;}
        if(f.loop && f.origin+f.frames<f.loop->periodFrames*f.loop->options.repeats)
        {error="Loop Export file omits a scheduled pass";return false;}
        if(f.path==state.completedPath)completed=true;
    }
    if(!completed){error="Completed Export reference is not in immutable history";return false;}
    error.clear();return true;
}
juce::var exportStateMetadata(const ExportState& state)
{
    auto v=object();put(v,"version",2);put(v,"initialized",state.initialized);put(v,"wet",state.wet);put(v,"dry",state.dry);
    put(v,"desiredIdentity",state.desiredIdentity);put(v,"completedPath",state.completedPath);
    if(state.loop.initialized){auto loop=object();put(loop,"version",1);put(loop,"repeats",state.loop.options.repeats);put(loop,"gridBeats",state.loop.options.gridBeats);
        put(loop,"dry",state.loop.levels.dry);put(loop,"wet",state.loop.levels.wet);put(v,"loopSettings",loop);}
    juce::Array<juce::var> files;
    for(const auto& f:state.history){auto a=object();put(a,"composition",f.composition);put(a,"checksum",f.checksum);put(a,"path",f.path);
        put(a,"rate",f.rate);put(a,"origin",juce::String(f.origin));put(a,"frames",juce::String(f.frames));put(a,"wet",f.wet);put(a,"dry",f.dry);put(a,"gainLaw",f.gainLaw);
        if(f.loop){auto loop=object();put(loop,"version",1);put(loop,"repeats",f.loop->options.repeats);put(loop,"gridBeats",f.loop->options.gridBeats);put(loop,"periodFrames",juce::String(f.loop->periodFrames));put(a,"loop",loop);}files.add(a);}
    put(v,"history",files);return v;
}
bool parseExportState(const juce::var& v,ExportState& output,juce::String& error)
{
    ExportState state;bool ok=v.isObject();
    const bool legacy=v.isObject() && !v.hasProperty("version");
    const auto field=[&](const juce::var& a,const char* name){if(!a.isObject() || !a.hasProperty(name))ok=false;return a[name];};
    const auto text=[&](const juce::var& a,const char* name){const auto x=field(a,name);if(!x.isString())ok=false;return x.toString();};
    const auto number=[&](const juce::var& a,const char* name){const auto x=field(a,name);if(!x.isDouble() && !x.isInt() && !x.isInt64())ok=false;return static_cast<double>(x);};
    const auto big=[&](const juce::var& a,const char* name){const auto x=text(a,name);const auto n=x.getLargeIntValue();if(juce::String(n)!=x)ok=false;return n;};
    const auto whole=[&](const juce::var& a,const char* name){const auto n=number(a,name);if(!std::isfinite(n) || n<1 || n>INT32_MAX || std::floor(n)!=n){ok=false;return 0;}return static_cast<int>(n);};
    if(!legacy && number(v,"version")!=2)ok=false;
    const auto initialized=field(v,"initialized");if(!initialized.isBool())ok=false;state.initialized=static_cast<bool>(initialized);
    const auto wet=number(v,"wet");if(!std::isfinite(wet) || wet<0 || wet>1)ok=false;state.wet=static_cast<float>(wet);state.desiredIdentity=text(v,"desiredIdentity");state.completedPath=text(v,"completedPath");
    if(!legacy){const auto dry=number(v,"dry");if(!std::isfinite(dry) || dry<0 || dry>1)ok=false;state.dry=static_cast<float>(dry);}
    if(v.hasProperty("loopSettings")){
        const auto loop=field(v,"loopSettings");if(legacy || number(loop,"version")!=1)ok=false;
        state.loop.initialized=true;state.loop.options={whole(loop,"repeats"),whole(loop,"gridBeats")};
        const auto dry=number(loop,"dry"),wet=number(loop,"wet");
        if(!std::isfinite(dry) || dry<0 || dry>1 || !std::isfinite(wet) || wet<0 || wet>1)ok=false;
        state.loop.levels={static_cast<float>(dry),static_cast<float>(wet)};
    }
    const auto history=field(v,"history");if(!history.isArray() || history.size()>static_cast<int>(maximumExportRevisions)){error="Invalid Export history capacity";return false;}
    for(const auto& a:*history.getArray()){ExportFile f;f.composition=text(a,"composition");f.checksum=text(a,"checksum");f.path=text(a,"path");
        f.rate=number(a,"rate");f.origin=big(a,"origin");f.frames=big(a,"frames");const auto weight=number(a,"wet");if(!std::isfinite(weight) || weight<0 || weight>1)ok=false;f.wet=static_cast<float>(weight);
        if(legacy){f.dry=1-f.wet;f.gainLaw=1;}
        else {const auto dry=number(a,"dry"),law=number(a,"gainLaw");if(!std::isfinite(dry) || dry<0 || dry>1 || (law!=1 && law!=2))ok=false;f.dry=static_cast<float>(dry);if(law==1 || law==2)f.gainLaw=static_cast<int>(law);}
        if(a.hasProperty("loop")){
            const auto loop=field(a,"loop");if(legacy || number(loop,"version")!=1)ok=false;
            f.loop=LoopExportDescriptor{{whole(loop,"repeats"),whole(loop,"gridBeats")},big(loop,"periodFrames")};
        }
        state.history.push_back(std::move(f));}
    if(!ok){error="Malformed Export metadata";return false;}
    if(!validExportState(state,error))return false;
    if(legacy){state.dry=0;state.wet=1;state.desiredIdentity.clear();state.completedPath.clear();}
    output=std::move(state);return true;
}
}
