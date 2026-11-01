#include "SwellSession.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace m3
{
bool checkedAdd(int64_t a, int64_t b, int64_t& result) noexcept
{
    if ((b > 0 && a > INT64_MAX - b) || (b < 0 && a < INT64_MIN - b)) return false;
    result = a + b; return true;
}
bool checkedSubtract(int64_t a, int64_t b, int64_t& result) noexcept
{
    if ((b > 0 && a < INT64_MIN + b) || (b < 0 && a > INT64_MAX + b)) return false;
    result = a - b; return true;
}
bool safeAssetPath(const juce::String& path) noexcept
{
    if (path.isEmpty()) return true;
    if (path.containsChar('\\') || path.containsChar(':') || path.startsWithChar('/') || path.containsChar('\0')) return false;
    const auto parts = juce::StringArray::fromTokens(path, "/", "");
    if (parts.size() != 2 || (parts[0] != "irs" && parts[0] != "renders")) return false;
    return parts[1].endsWith(".wav") && parts[1].dropLastCharacters(4).containsOnly("0123456789abcdef-")
        && parts[1].length() > 4 && parts[1].length() <= 100;
}
Swell* find(Session& s, const juce::String& id) { for (auto& w : s.swells) if (w.id == id) return &w; return nullptr; }
const Swell* find(const Session& s, const juce::String& id) { for (const auto& w : s.swells) if (w.id == id) return &w; return nullptr; }
bool select(Session& s, const std::vector<juce::String>& ids, const juce::String& primary)
{
    std::set<juce::String> unique;
    for (const auto& id : ids) if (! find(s, id) || ! unique.insert(id).second) return false;
    if (ids.empty() ? primary.isNotEmpty() : unique.count(primary) == 0) return false;
    s.selected = ids; s.primary = primary; return true;
}
bool erase(Session& s, const juce::String& id)
{
    if (! find(s, id)) return false;
    std::erase_if(s.swells, [&](const auto& w) { return w.id == id; });
    std::erase(s.selected, id);
    if (s.primary == id) s.primary = s.selected.empty() ? juce::String{} : s.selected.front();
    return true;
}
bool canAudition(const Session& s, const Swell& w) noexcept
{
    const auto anySolo = std::any_of(s.swells.begin(), s.swells.end(), [](const auto& v) { return v.solo; });
    return w.enabled && ! w.muted && (! anySolo || w.solo) && w.phase == Phase::ready;
}
bool add(Session& s, int64_t arrival, const CaptureManifest& map, const SongTime& time,
         const std::vector<int64_t>& others, juce::String& error)
{
    if (s.swells.size() >= maximumSwells) { error = "Session capacity: 4096 swells"; return false; }
    if (! time.valid || ! std::isfinite(time.rate) || ! std::isfinite(time.bpm) || time.rate <= 0 || time.bpm <= 0)
    { error = "Valid fixed host timing is required"; return false; }
    std::optional<int64_t> coveredEnd;
    for (const auto& range : map.ranges)
    {
        int64_t end = 0;
        if (range.frameCount > INT64_MAX || ! checkedAdd(range.startSample, static_cast<int64_t>(range.frameCount), end)) continue;
        if (! coveredEnd && range.startSample <= arrival && arrival < end) coveredEnd = end;
        else if (coveredEnd && range.startSample <= *coveredEnd) coveredEnd = std::max(*coveredEnd, end);
        else if (coveredEnd) break;
    }
    const auto quarter = time.rate * 60.0 / time.bpm;
    if (! coveredEnd || ! std::isfinite(quarter) || quarter >= static_cast<double>(INT64_MAX))
    { error = "Arrival needs covered source audio and valid timing"; return false; }
    int64_t end = 0;
    if (! checkedAdd(arrival, std::max(int64_t{1}, static_cast<int64_t>(std::llround(quarter))), end))
    { error = "Source position overflow"; return false; }
    end = std::min(end, *coveredEnd);
    for (const auto candidate : others) if (candidate > arrival) end = std::min(end, candidate);
    for (const auto& other : s.swells) if (other.arrival > arrival) end = std::min(end, other.arrival);
    if (end <= arrival) { error = "Source interval is empty"; return false; }
    Swell w; w.name = "Swell " + juce::String(static_cast<int>(s.swells.size() + 1));
    w.arrival = w.sourceStart = arrival; w.sourceEnd = end; w.shape=initialShape(time.rate);
    const auto id = w.id; s.swells.push_back(std::move(w)); s.timing = time;
    select(s, {id}, id); error.clear(); return true;
}
Shape initialShape(double rate)
{
    Shape s;if(std::isfinite(rate) && rate>0 && rate<1e9){s.endOffset=s.fadeOut=std::max(int64_t{1},static_cast<int64_t>(std::llround(rate*.020)));s.outCurve=Curve::smooth;}return s;
}
bool moveWet(Swell& w,int64_t placement,juce::String& error)
{
    auto trial=w;trial.placement=placement;int64_t a,b;
    const auto start=w.render.present()?w.render.start:w.sourceStart;
    if(!requestedShapeBounds(trial,start,a,b)){error="Wet placement overflow";return false;}
    w.placement=placement;++w.shapeGeneration;error.clear();return true;
}
bool moveArrival(Swell& w, int64_t arrival, juce::String& error)
{
    if(arrival==w.arrival){error.clear();return true;}
    int64_t delta, start, end;
    if (! checkedSubtract(arrival, w.arrival, delta) || ! checkedAdd(w.sourceStart, delta, start)
        || ! checkedAdd(w.sourceEnd, delta, end)) { error = "Arrival position overflow"; return false; }
    w.arrival = arrival; w.sourceStart = start; w.sourceEnd = end; w.supportOffset=0;
    ++w.inputGeneration; w.phase = Phase::stale; w.status = "Arrival changed; regenerate"; error.clear(); return true;
}
bool setSource(Swell& w, int64_t start, int64_t end, juce::String& error)
{
    int64_t length;
    if (start > w.arrival || end <= w.arrival || ! checkedSubtract(end, start, length) || length <= 0)
    { error = "Source must contain arrival and have a finite positive length"; return false; }
    if (start != w.sourceStart || end != w.sourceEnd)
    { w.sourceStart = start; w.sourceEnd = end; w.supportOffset=0; ++w.inputGeneration; w.phase = Phase::stale; w.status = "Source changed; regenerate"; }
    error.clear(); return true;
}
static bool validShape(const Shape& v)
{
    return std::isfinite(v.inTension) && std::abs(v.inTension)<=1 && std::isfinite(v.outTension) && std::abs(v.outTension)<=1
        && std::isfinite(v.gainDb) && v.gainDb >= -60 && v.gainDb <= 12 && v.fadeIn >= 0 && v.fadeOut >= 0
        && static_cast<int>(v.inCurve) >= 0 && static_cast<int>(v.inCurve) <= 2
        && static_cast<int>(v.outCurve) >= 0 && static_cast<int>(v.outCurve) <= 2
        && (v.automaticStart || v.startOffset < v.endOffset);
}
bool requestedShapeBounds(const Swell& w,int64_t rawStart,int64_t& a,int64_t& b) noexcept
{
    a=rawStart; int64_t length;
    return (w.shape.automaticStart || checkedAdd(w.arrival,w.shape.startOffset,a))
        && checkedAdd(w.arrival,w.shape.endOffset,b) && checkedAdd(a,w.placement,a) && checkedAdd(b,w.placement,b) && checkedSubtract(b,a,length) && length>0;
}
bool effectiveSourceEnd(const Swell& w,int64_t& end) noexcept
{
    int64_t wetEnd, retained;
    if(w.supportOffset<0 || !checkedAdd(w.arrival,w.shape.endOffset,wetEnd) || !checkedAdd(w.sourceEnd,w.supportOffset,retained))return false;
    end=std::max(retained,wetEnd);return true;
}
bool auditionContextEnd(const Swell& w,double rate,double seconds,int64_t& end) noexcept
{
    int64_t wetEnd,tailEnd;
    if(!std::isfinite(rate) || rate<=0 || !std::isfinite(seconds) || seconds<0 || seconds>5
        || !checkedAdd(w.arrival,w.shape.endOffset,wetEnd) || !checkedAdd(wetEnd,w.placement,wetEnd))return false;
    const auto frames=rate*seconds;if(!std::isfinite(frames) || frames>=static_cast<double>(INT64_MAX))return false;
    if(!checkedAdd(std::max(w.arrival,wetEnd),static_cast<int64_t>(std::llround(frames)),tailEnd))return false;
    end=std::max(w.sourceEnd,tailEnd);return true;
}
bool shapeBounds(const Swell& w, int64_t rawStart, int64_t rawEnd, int64_t& a, int64_t& b, juce::String& error)
{
    int64_t positionedStart,positionedEnd;
    if(!validShape(w.shape) || !requestedShapeBounds(w,rawStart,a,b)
        || !checkedAdd(rawStart,w.placement,positionedStart) || !checkedAdd(rawEnd,w.placement,positionedEnd)
        || rawStart>=rawEnd || a<positionedStart || b>positionedEnd)
    {error="Audible bounds exceed full render; adjust shape or Reset";return false;}
    error.clear(); return true;
}
bool setShape(Swell& w, Shape shape, int64_t rawStart, int64_t rawEnd, juce::String& error)
{
    auto candidate = w; candidate.shape = shape;
    int64_t a, b,positionedStart;
    // Endpoints beyond current support retain their intent and request real input extension.
    if (!validShape(shape) || !requestedShapeBounds(candidate,rawStart,a,b) || !checkedAdd(rawStart,w.placement,positionedStart) || a<positionedStart || rawStart>=rawEnd)
    {error="Invalid requested wet bounds";return false;}
    int64_t extension=0;
    int64_t logicalEnd;
    if(!checkedSubtract(b,w.placement,logicalEnd) || (logicalEnd>w.sourceEnd && !checkedSubtract(logicalEnd,w.sourceEnd,extension))){error="Wet support overflow";return false;}
    // Retain only successfully prepared support. The requested endpoint itself
    // carries pending intent, so dragging back can cancel an unavailable extension.
    int64_t effective;if(!effectiveSourceEnd(candidate,effective)){error="Wet support overflow";return false;}
    w.supportOffset=candidate.supportOffset;
    shape.fadeIn = std::min(shape.fadeIn, b - a - 1); shape.fadeOut = std::min(shape.fadeOut, b - a - 1);
    w.shape = shape; ++w.shapeGeneration; error.clear(); return true;
}
bool fullShape(Swell& w, int64_t rawStart, int64_t rawEnd, juce::String& error)
{
    Shape shape;
    shape.automaticStart = false;
    if (! checkedSubtract(rawStart, w.arrival, shape.startOffset) || ! checkedSubtract(rawEnd, w.arrival, shape.endOffset))
    { error = "Shape position overflow"; return false; }
    return setShape(w, shape, rawStart, rawEnd, error);
}
void resetShape(Swell& w) { w.shape = {}; ++w.shapeGeneration; }

static bool validId(const juce::String& id)
{
    return id.length() == 36 && juce::Uuid(id).toDashedString() == id && ! juce::Uuid(id).isNull();
}
static bool validAsset(const Asset& a)
{
    if (! a.present()) return a.checksum.isEmpty() && a.content.isEmpty();
    return safeAssetPath(a.path) && a.checksum.length() == 64 && a.checksum.containsOnly("0123456789abcdef")
        && a.content.length() == 64 && a.content.containsOnly("0123456789abcdef")
        && std::isfinite(a.rate) && a.rate > 0 && a.frames > 0 && a.frames <= INT64_MAX / 8
        && a.channels >= 1 && a.channels <= 2 && a.engine >= 1 && std::isfinite(a.bpm) && a.bpm >= 0
        && a.numerator > 0 && a.denominator > 0;
}
bool validate(const Session& s, juce::String& error)
{
    if (s.swells.size() > maximumSwells || ! std::isfinite(s.auditionDb) || s.auditionDb < -60 || s.auditionDb > 0
        || !std::isfinite(s.liveWetDb) || s.liveWetDb < -60 || s.liveWetDb > 0
        || !std::isfinite(s.regenerationSensitivity) || s.regenerationSensitivity<0 || s.regenerationSensitivity>1
        || static_cast<int>(s.liveRouting)<0 || static_cast<int>(s.liveRouting)>1
        || ! std::isfinite(s.sensitivity) || s.sensitivity < 0 || s.sensitivity > 1
        || ! std::isfinite(s.separationSeconds) || s.separationSeconds < 0.001 || s.separationSeconds > 2
        || !std::isfinite(s.postRollSeconds) || s.postRollSeconds<0 || s.postRollSeconds>5
        || ! std::isfinite(s.wetBlend) || s.wetBlend < 0 || s.wetBlend > 1
        || static_cast<int>(s.auditionMode)<0 || static_cast<int>(s.auditionMode)>1
        || s.suppressed.size()>maximumSwells || !validAsset(s.defaultIR)
        || (s.defaultIR.present() && !s.defaultIR.path.startsWith("irs/")) || s.defaultIRName.length()>512)
    { error = "Invalid session preferences/capacity"; return false; }
    if (s.timing.valid && (! std::isfinite(s.timing.rate) || s.timing.rate <= 0 || ! std::isfinite(s.timing.bpm)
        || s.timing.bpm <= 0 || ! std::isfinite(s.timing.ppq) || s.timing.numerator <= 0 || s.timing.denominator <= 0))
    { error = "Invalid saved timing"; return false; }
    std::set<juce::String> ids;
    for (const auto& w : s.swells)
    {
        int64_t length,effective,wetEnd,effectiveLength,explicitStart;
        if (!effectiveSourceEnd(w,effective) || !checkedSubtract(effective,w.sourceStart,effectiveLength) || effectiveLength<=0
            || (!w.shape.automaticStart && !checkedAdd(w.arrival,w.shape.startOffset,explicitStart))
            || !checkedAdd(w.arrival,w.shape.endOffset,wetEnd) || !checkedAdd(wetEnd,w.placement,wetEnd)
            || (w.render.present() && (!checkedAdd(w.render.start,w.placement,explicitStart) || !checkedAdd(w.render.sourceEnd,w.placement,explicitStart))) || ! validId(w.id) || ! ids.insert(w.id).second || w.name.length() > 256 || w.irName.length()>512 || w.name.containsChar('\0')
            || w.sourceStart > w.arrival || w.sourceEnd <= w.arrival
            || ! checkedSubtract(w.sourceEnd, w.sourceStart, length) || length <= 0 || ! validShape(w.shape)
            || static_cast<int>(w.provenance)<0 || static_cast<int>(w.provenance)>2
            || ! validAsset(w.ir) || ! validAsset(w.render) || (w.frozen && !w.render.present())
            || (w.ir.present() && !w.ir.path.startsWith("irs/")) || (w.render.present() && !w.render.path.startsWith("renders/")))
        { error = "Invalid swell identity, bounds, shape or asset reference"; return false; }
    }
    auto copy = s;
    if (! select(copy, s.selected, s.primary)) { error = "Invalid selection"; return false; }
    error.clear(); return true;
}
static juce::var object() { return juce::var(new juce::DynamicObject); }
static void put(juce::var& v, const char* name, const juce::var& value) { v.getDynamicObject()->setProperty(name, value); }
static juce::var integer(int64_t v) { return juce::var(juce::String(v)); }
juce::var assetMetadata(const Asset& a)
{
    auto v = object();
    put(v,"path",a.path); put(v,"checksum",a.checksum); put(v,"content",a.content);
    put(v,"dependency",a.dependency); put(v,"irContent",a.irContent);
    put(v,"rate",a.rate); put(v,"bpm",a.bpm); put(v,"channels",a.channels); put(v,"engine",a.engine);
    put(v,"numerator",a.numerator); put(v,"denominator",a.denominator);
    put(v,"frames",integer(a.frames)); put(v,"start",integer(a.start));
    put(v,"sourceStart",integer(a.sourceStart)); put(v,"sourceEnd",integer(a.sourceEnd)); return v;
}
bool encode(const Session& s, juce::String& json, juce::String& error)
{
    if (! validate(s, error)) return false;
    auto v = object(); put(v,"version",7); put(v,"isolateDry",s.isolateDry); put(v,"regenerationSensitivity",s.regenerationSensitivity); put(v,"autoMarkersEnabled",s.autoMarkersEnabled); put(v,"isolateSelected",s.isolateSelected); put(v,"postRollSeconds",s.postRollSeconds); put(v,"auditionDb",s.auditionDb); put(v,"sensitivity",s.sensitivity);
    put(v,"liveEnabled",s.liveEnabled); put(v,"liveRouting",static_cast<int>(s.liveRouting)); put(v,"liveWetDb",s.liveWetDb);
    put(v,"mappingEnabled",s.mappingEnabled); put(v,"automaticProcessing",s.automaticProcessing);
    put(v,"auditionMode",static_cast<int>(s.auditionMode)); put(v,"wetBlend",s.wetBlend);
    put(v,"defaultIR",assetMetadata(s.defaultIR)); put(v,"defaultIRName",s.defaultIRName);
    juce::Array<juce::var> suppressed; for(auto anchor:s.suppressed) suppressed.add(integer(anchor));
    put(v,"suppressed",suppressed);
    put(v,"separation",s.separationSeconds); put(v,"autoRegenerate",s.autoRegenerate);
    put(v,"primary",s.primary);
    juce::Array<juce::var> selected, swells;
    for (const auto& id : s.selected) selected.add(id);
    put(v,"selected",selected);
    auto t = object(); put(t,"valid",s.timing.valid); put(t,"sample",integer(s.timing.sample)); put(t,"ppq",s.timing.ppq);
    put(t,"rate",s.timing.rate); put(t,"bpm",s.timing.bpm); put(t,"numerator",s.timing.numerator); put(t,"denominator",s.timing.denominator); put(v,"timing",t);
    for (const auto& w : s.swells)
    {
        auto a = object(); put(a,"id",w.id); put(a,"name",w.name); put(a,"irName",w.irName); put(a,"arrival",integer(w.arrival));
        put(a,"placement",integer(w.placement)); put(a,"supportOffset",integer(w.supportOffset)); put(a,"sourceStart",integer(w.sourceStart)); put(a,"sourceEnd",integer(w.sourceEnd));
        put(a,"enabled",w.enabled); put(a,"muted",w.muted); put(a,"solo",w.solo);
        put(a,"frozen",w.frozen);
        put(a,"provenance",static_cast<int>(w.provenance)); put(a,"detectorAnchor",integer(w.detectorAnchor));
        put(a,"ir",assetMetadata(w.ir)); put(a,"render",assetMetadata(w.render));
        auto h = object(); put(h,"automaticStart",w.shape.automaticStart); put(h,"start",integer(w.shape.startOffset));
        put(h,"end",integer(w.shape.endOffset)); put(h,"fadeIn",integer(w.shape.fadeIn)); put(h,"fadeOut",integer(w.shape.fadeOut));
        put(h,"inTension",w.shape.inTension); put(h,"outTension",w.shape.outTension); put(h,"inCurve",static_cast<int>(w.shape.inCurve)); put(h,"outCurve",static_cast<int>(w.shape.outCurve)); put(h,"gain",w.shape.gainDb);
        put(a,"shape",h); swells.add(a);
    }
    put(v,"swells",swells);
    auto result = juce::JSON::toString(v, true);
    if (result.getNumBytesAsUTF8() > maximumStateBytes) { error = "Session state exceeds 8 MiB"; return false; }
    json = result; return true;
}
// Strict typed parsing prevents permissive var conversions from accepting partial state.
struct Parser
{
    bool ok = true;
    juce::var field(const juce::var& v, const char* key)
    { if (! v.isObject() || ! v.hasProperty(key)) { ok = false; return {}; } return v[key]; }
    juce::String string(const juce::var& v, const char* key)
    { auto x = field(v,key); if (! x.isString()) ok = false; return x.toString(); }
    bool boolean(const juce::var& v, const char* key)
    { auto x = field(v,key); if (! x.isBool()) ok = false; return static_cast<bool>(x); }
    double number(const juce::var& v, const char* key)
    { auto x = field(v,key); if (! (x.isDouble() || x.isInt() || x.isInt64())) ok = false; auto n = static_cast<double>(x); if (! std::isfinite(n)) ok = false; return n; }
    int small(const juce::var& v, const char* key)
    { const auto n = number(v,key); if (n < -1000000 || n > 1000000 || std::floor(n) != n) { ok = false; return 0; } return static_cast<int>(n); }
    int64_t big(const juce::var& v, const char* key)
    { auto x = string(v,key); auto n = x.getLargeIntValue(); if (juce::String(n) != x) ok = false; return n; }
    Asset asset(const juce::var& v)
    {
        Asset a; a.path=string(v,"path"); a.checksum=string(v,"checksum"); a.content=string(v,"content");
        a.dependency=string(v,"dependency"); a.irContent=string(v,"irContent"); a.rate=number(v,"rate"); a.bpm=number(v,"bpm");
        a.channels=small(v,"channels"); a.engine=small(v,"engine"); a.numerator=small(v,"numerator"); a.denominator=small(v,"denominator");
        a.frames=big(v,"frames"); a.start=big(v,"start"); a.sourceStart=big(v,"sourceStart"); a.sourceEnd=big(v,"sourceEnd"); return a;
    }
};
bool parseAssetMetadata(const juce::var& v, Asset& output, juce::String& error)
{
    Parser p; auto a=p.asset(v);
    if (!p.ok || !validAsset(a)) { error="Invalid managed asset metadata"; return false; }
    output=std::move(a); error.clear(); return true;
}
bool decode(const juce::String& json, Session& output, juce::String& error)
{
    if (json.getNumBytesAsUTF8() > maximumStateBytes) { error = "Session state exceeds 8 MiB"; return false; }
    juce::var v;
    if (auto result = juce::JSON::parse(json, v); result.failed()) { error = result.getErrorMessage(); return false; }
    Parser p; Session s;
    const auto version=p.small(v,"version");
    if (version < 1 || version > 7) { error = "Unsupported swell state version"; return false; }
    if(version>=7)s.isolateDry=p.boolean(v,"isolateDry");
    if(version>=6)s.regenerationSensitivity=static_cast<float>(p.number(v,"regenerationSensitivity"));
    if(version>=5){s.liveEnabled=p.boolean(v,"liveEnabled");s.liveRouting=static_cast<LiveRouting>(p.small(v,"liveRouting"));s.liveWetDb=static_cast<float>(p.number(v,"liveWetDb"));}
    if(version>=4){s.autoMarkersEnabled=p.boolean(v,"autoMarkersEnabled");s.isolateSelected=p.boolean(v,"isolateSelected");}
    if(version>=2)
    {
        s.mappingEnabled=p.boolean(v,"mappingEnabled"); s.automaticProcessing=p.boolean(v,"automaticProcessing");
        s.auditionMode=static_cast<AuditionMode>(p.small(v,"auditionMode")); s.wetBlend=static_cast<float>(p.number(v,"wetBlend"));
        s.defaultIR=p.asset(p.field(v,"defaultIR")); s.defaultIRName=p.string(v,"defaultIRName");
        const auto suppressed=p.field(v,"suppressed");
        if(!suppressed.isArray() || suppressed.size()>static_cast<int>(maximumSwells)) {error="Invalid suppression array";return false;}
        for(const auto& value:*suppressed.getArray())
        { const auto text=value.toString();const auto anchor=text.getLargeIntValue();if(!value.isString() || juce::String(anchor)!=text)p.ok=false;s.suppressed.push_back(anchor); }
    }
    else {s.mappingEnabled=false;s.automaticProcessing=false;}
    if(version>=3)s.postRollSeconds=p.number(v,"postRollSeconds");
    s.auditionDb=static_cast<float>(p.number(v,"auditionDb")); s.sensitivity=static_cast<float>(p.number(v,"sensitivity"));
    s.separationSeconds=p.number(v,"separation"); s.autoRegenerate=p.boolean(v,"autoRegenerate"); s.primary=p.string(v,"primary");
    const auto selection=p.field(v,"selected"), entries=p.field(v,"swells"), t=p.field(v,"timing");
    if (! selection.isArray() || ! entries.isArray() || entries.size() > static_cast<int>(maximumSwells) || selection.size() > static_cast<int>(maximumSwells))
    { error = "Invalid swell arrays/capacity"; return false; }
    s.timing.valid=p.boolean(t,"valid"); s.timing.sample=p.big(t,"sample"); s.timing.ppq=p.number(t,"ppq");
    s.timing.rate=p.number(t,"rate"); s.timing.bpm=p.number(t,"bpm"); s.timing.numerator=p.small(t,"numerator"); s.timing.denominator=p.small(t,"denominator");
    for (const auto& id : *selection.getArray()) { if (! id.isString()) p.ok=false; s.selected.push_back(id.toString()); }
    for (const auto& a : *entries.getArray())
    {
        Swell w; w.id=p.string(a,"id"); w.name=p.string(a,"name"); w.irName=p.string(a,"irName"); w.arrival=p.big(a,"arrival");
        if(version>=4)w.placement=p.big(a,"placement");
        if(version>=3)w.supportOffset=p.big(a,"supportOffset");
        w.sourceStart=p.big(a,"sourceStart"); w.sourceEnd=p.big(a,"sourceEnd");
        w.enabled=p.boolean(a,"enabled"); w.muted=p.boolean(a,"muted"); w.solo=p.boolean(a,"solo");
        if(version>=6)w.frozen=p.boolean(a,"frozen");
        if(version>=2) {w.provenance=static_cast<Provenance>(p.small(a,"provenance"));w.detectorAnchor=p.big(a,"detectorAnchor");}
        else {w.provenance=Provenance::manualEdited;w.detectorAnchor=w.arrival;}
        w.ir=p.asset(p.field(a,"ir")); w.render=p.asset(p.field(a,"render"));
        const auto h=p.field(a,"shape"); w.shape.automaticStart=p.boolean(h,"automaticStart");
        w.shape.startOffset=p.big(h,"start"); w.shape.endOffset=p.big(h,"end"); w.shape.fadeIn=p.big(h,"fadeIn"); w.shape.fadeOut=p.big(h,"fadeOut");
        if(version>=3){w.shape.inTension=p.number(h,"inTension");w.shape.outTension=p.number(h,"outTension");}
        w.shape.inCurve=static_cast<Curve>(p.small(h,"inCurve")); w.shape.outCurve=static_cast<Curve>(p.small(h,"outCurve")); w.shape.gainDb=static_cast<float>(p.number(h,"gain"));
        w.phase=Phase::restoring; w.status="Restoring dependencies"; s.swells.push_back(std::move(w));
    }
    if (! p.ok) { error="Malformed swell metadata"; return false; }
    if (! validate(s,error)) return false;
    output=std::move(s); error.clear(); return true;
}
}
