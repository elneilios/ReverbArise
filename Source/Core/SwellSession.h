#pragma once
#include <Capture/CaptureManifest.h>
#include <Core/SongTime.h>
#include <Core/ManagedAsset.h>
#include <Core/PerformNavigationState.h>
#include <Core/ExportState.h>
#include <juce_core/juce_core.h>
#include <optional>
#include <vector>

namespace m3
{
constexpr size_t maximumSwells = 4096, maximumStateBytes = 8 * 1024 * 1024;
constexpr size_t maximumImportedIRs = maximumSwells + 1;
enum class Curve { linear, smooth, exponential };
enum class Provenance { automatic, manualCreated, manualEdited };
enum class AuditionMode { wetOnly, dryWet };
enum class LiveRouting { dryWet, wetOnly };
enum class GainPolicy { legacy, musical };
enum class CalibrationState { unavailable, fixed, matched, capped };
enum class Phase { empty, checking, queued, rendering, ready, stale, error, restoring };
inline constexpr const char* legacyGainRevision="legacy-v1";
inline constexpr const char* candidateGainRevision="candidate-source-peak-v1";
inline constexpr const char* energyGainRevision="candidate-source-energy-v1";
inline constexpr const char* approvedGainRevision="musical-source-energy-plus6-v1";
inline constexpr float approvedEnergyTargetDb=6.0f;
inline bool isEnergyRevision(const juce::String& revision) noexcept
{return revision==energyGainRevision || revision==approvedGainRevision;}
inline constexpr double candidateRmsSeconds=.020,candidateRmsHopSeconds=.001;
inline constexpr float candidateWetCeilingDb=-1.0f,candidateMatchFloorDb=-60.0f,candidateMaximumBoostDb=24.0f;
inline constexpr double candidateAttackSeconds=.100;
// Durable Level domain. The editor's focused dial sweep is a separate policy.
inline constexpr float minimumSwellLevelDb=-60.0f,maximumSwellLevelDb=12.0f,defaultSwellLevelDb=0.0f;
inline constexpr double swellLevelStepDb=1.0,swellLevelFineStepDb=.1;
struct ImportedIR
{
    Asset asset;
    juce::String name;
};
struct Shape
{
    bool operator==(const Shape&) const = default;
    bool automaticStart = true;
    int64_t startOffset = 0, endOffset = 1, fadeIn = 0, fadeOut = 0;
    Curve inCurve = Curve::linear, outCurve = Curve::linear;
    float gainDb = defaultSwellLevelDb;
    double inTension = 0, outTension = 0;
};
struct GainCalibration
{
    bool operator==(const GainCalibration&) const = default;
    CalibrationState state=CalibrationState::unavailable;
    float baselineDb=0,dryPeak=0,wetPeak=0;
    float dryRms=0,wetRms=0;
    int64_t windowFrames=0,hopFrames=0;
    int64_t referenceStart=0,referenceEnd=0;
    juce::String sourceIdentity,irIdentity,renderIdentity,revision;
};
struct Swell
{
    juce::String id = juce::Uuid().toDashedString(), name, irName;
    int64_t arrival = 0, sourceStart = 0, sourceEnd = 1;
    Shape shape;
    GainCalibration calibration;
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
    juce::String generationSource; // Last explicit generation's PCM fingerprint, not a render claim.
};
struct Session
{
    std::vector<Swell> swells;
    std::vector<juce::String> selected;
    juce::String primary;
    SongTime timing;
    PerformNavigationState performUi;
    float auditionDb = 0, sensitivity = 0.5f;
    double separationSeconds = 0.08;
    bool autoRegenerate = false;
    bool autoGenerateOnStop = false;
    bool mappingEnabled = true, automaticProcessing = true, autoMarkersEnabled = true;
    bool isolateSelected = false, isolateDry = false;
    bool liveEnabled = true;
    LiveRouting liveRouting = LiveRouting::dryWet;
    float liveWetDb = 0;
    // Bare Session is also the migration container for old metadata. Fresh
    // plugin instances explicitly select the approved policy at construction.
    GainPolicy gainPolicy=GainPolicy::legacy;
    juce::String gainRevision=legacyGainRevision;
    float regenerationSensitivity=.5f;
    AuditionMode auditionMode = AuditionMode::wetOnly;
    float wetBlend = 0.5f;
    bool auditionFollowsLive = false; // Absent metadata preserves legacy monitoring.
    double postRollSeconds = 2;
    ExportState exportState;
    Asset defaultIR;
    juce::String defaultIRName = "Preverb Hall";
    std::vector<ImportedIR> importedIRs;
    std::vector<int64_t> suppressed;
};
inline bool initializeExport(Session& session,juce::String& error)
{
    if(session.exportState.initialized){error.clear();return true;}
    session.exportState.initialized=true;session.exportState.dry=0;session.exportState.wet=1;error.clear();return true;
}
inline bool setExportLevels(Session& session,float dry,float wet,juce::String& error)
{
    if(!std::isfinite(dry) || dry<0 || dry>1 || !std::isfinite(wet) || wet<0 || wet>1){error="Invalid Export levels";return false;}
    if(session.exportState.dry!=dry || session.exportState.wet!=wet)session.exportState.desiredIdentity.clear();
    session.exportState.initialized=true;session.exportState.dry=dry;session.exportState.wet=wet;error.clear();return true;
}
// Transitional binding for the existing crossfade UI until the reviewed pair
// control is integrated. This never adds a second processing stage.
inline bool setExportMix(Session& session,float wet,juce::String& error)
{return setExportLevels(session,1-wet,wet,error);}
// Foundation eligibility uses already-verified readiness and timing metadata.
// Full export preparation/asset verification remains the export workspace's job.
inline bool hasCompatibleReadyRender(const Swell& w,const SongTime& time) noexcept
{
    const auto& a=w.render;
    return w.phase==Phase::ready && a.present() && a.frames>0 && time.valid
        && a.rate==time.rate && a.bpm==time.bpm && a.numerator==time.numerator && a.denominator==time.denominator;
}
// Excludes selection, worker progress and generation counters. These fields
// describe the durable musical definition, not the current playback/UI state.
inline bool sameSavedSwell(const Swell& a,const Swell& b)
{
    return a.id==b.id && a.name==b.name && a.irName==b.irName && a.arrival==b.arrival
        && a.sourceStart==b.sourceStart && a.sourceEnd==b.sourceEnd && a.shape==b.shape
        && a.calibration==b.calibration && a.placement==b.placement && a.supportOffset==b.supportOffset
        && a.ir==b.ir && a.render==b.render && a.enabled==b.enabled && a.muted==b.muted
        && a.solo==b.solo && a.frozen==b.frozen && a.provenance==b.provenance && a.detectorAnchor==b.detectorAnchor
        && a.generationSource==b.generationSource;
}
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
float calibrationDb(const Session&,const Swell&) noexcept;
float gainCeilingDb(const Session&) noexcept;
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
