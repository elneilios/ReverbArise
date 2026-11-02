#pragma once
#include <Capture/CaptureManifest.h>
#include <Core/SongTime.h>
#include <juce_core/juce_core.h>
#include <optional>
#include <vector>

namespace m3
{
constexpr size_t maximumSwells = 4096, maximumStateBytes = 8 * 1024 * 1024;
enum class Curve { linear, smooth, exponential };
enum class Provenance { automatic, manualCreated, manualEdited };
enum class AuditionMode { wetOnly, dryWet };
enum class LiveRouting { dryWet, wetOnly };
enum class Phase { empty, checking, queued, rendering, ready, stale, error, restoring };
struct Asset
{
    bool operator==(const Asset&) const = default;
    juce::String path, checksum, content, dependency, irContent;
    double rate = 0, bpm = 0;
    int channels = 0, engine = 1, numerator = 4, denominator = 4;
    int64_t frames = 0, start = 0, sourceStart = 0, sourceEnd = 0;
    bool present() const noexcept { return path.isNotEmpty(); }
};
struct Shape
{
    bool operator==(const Shape&) const = default;
    bool automaticStart = true;
    int64_t startOffset = 0, endOffset = 1, fadeIn = 0, fadeOut = 0;
    Curve inCurve = Curve::linear, outCurve = Curve::linear;
    float gainDb = 0;
    double inTension = 0, outTension = 0;
};
struct Swell
{
    juce::String id = juce::Uuid().toDashedString(), name, irName;
    int64_t arrival = 0, sourceStart = 0, sourceEnd = 1;
    Shape shape;
    int64_t placement = 0;
    int64_t supportOffset = 0; // Retain genuine extended input when shortening the wet shape.
    Asset ir, render;
    bool enabled = true, muted = false, solo = false;
    bool frozen=false;
    Phase phase = Phase::empty;
    juce::String status = "Assign an IR and Generate";
    float progress = 0;
    uint64_t inputGeneration = 0, shapeGeneration = 0;
    Provenance provenance = Provenance::manualCreated;
    int64_t detectorAnchor = 0;
};
struct Session
{
    std::vector<Swell> swells;
    std::vector<juce::String> selected;
    juce::String primary;
    SongTime timing;
    float auditionDb = 0, sensitivity = 0.5f;
    double separationSeconds = 0.08;
    bool autoRegenerate = false;
    bool mappingEnabled = true, automaticProcessing = true, autoMarkersEnabled = true;
    bool isolateSelected = false, isolateDry = false;
    bool liveEnabled = true;
    LiveRouting liveRouting = LiveRouting::dryWet;
    float liveWetDb = 0;
    float regenerationSensitivity=.5f;
    AuditionMode auditionMode = AuditionMode::wetOnly;
    float wetBlend = 0.5f;
    double postRollSeconds = 2;
    Asset defaultIR;
    juce::String defaultIRName = "Preverb Hall";
    std::vector<int64_t> suppressed;
};
bool checkedAdd(int64_t a, int64_t b, int64_t& result) noexcept;
bool checkedSubtract(int64_t a, int64_t b, int64_t& result) noexcept;
bool safeAssetPath(const juce::String&) noexcept;
Swell* find(Session&, const juce::String&);
const Swell* find(const Session&, const juce::String&);
bool select(Session&, const std::vector<juce::String>& ids, const juce::String& primary);
bool erase(Session&, const juce::String& id);
bool canAudition(const Session&, const Swell&) noexcept;
bool add(Session&, int64_t arrival, const CaptureManifest&, const SongTime&,
         const std::vector<int64_t>& otherArrivals, juce::String& error);
Shape initialShape(double rate);
juce::String defaultSwellName(int64_t arrival,const SongTime&,const Session* existing=nullptr);
bool resetWetDefaults(Swell&,double rate,juce::String& error);
bool resetSwellDefaults(const Session&,const juce::String& id,const CaptureManifest&,const SongTime&,Swell&,juce::String& error);
bool moveWet(Swell&,int64_t placement,juce::String& error);
bool moveArrival(Swell&, int64_t arrival, juce::String& error);
bool setSource(Swell&, int64_t start, int64_t end, juce::String& error);
bool effectiveSourceEnd(const Swell&, int64_t& end) noexcept;
inline int64_t renderInputEnd(const Swell& w) noexcept {int64_t end=w.sourceEnd;effectiveSourceEnd(w,end);return end;}
bool auditionContextEnd(const Swell&, double rate, double seconds, int64_t& end) noexcept;
inline bool auditionDryEnd(const Swell& w,double rate,double seconds,bool isolated,int64_t& end) noexcept
{if(isolated){end=w.sourceEnd;return end>w.sourceStart;}return auditionContextEnd(w,rate,seconds,end);}

bool requestedShapeBounds(const Swell&, int64_t rawStart, int64_t& a, int64_t& b) noexcept;
bool shapeBounds(const Swell&, int64_t rawStart, int64_t rawEnd, int64_t& a, int64_t& b, juce::String& error);
bool setShape(Swell&, Shape, int64_t rawStart, int64_t rawEnd, juce::String& error);
bool fullShape(Swell&, int64_t rawStart, int64_t rawEnd, juce::String& error);
void resetShape(Swell&);
bool validate(const Session&, juce::String& error);
juce::var assetMetadata(const Asset&);
bool parseAssetMetadata(const juce::var&, Asset&, juce::String& error);
bool encode(const Session&, juce::String& json, juce::String& error);
bool decode(const juce::String& json, Session&, juce::String& error);
}
