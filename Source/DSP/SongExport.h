#pragma once
#include <Capture/CaptureSession.h>
#include <Core/SwellSession.h>
#include <memory>

namespace m3
{
inline constexpr const char* songExportRevision="song-export-v1";
enum class ExportCoverage { gap, unavailable };
struct ExportWarning
{
    int64_t start=0,end=0;
    ExportCoverage kind=ExportCoverage::gap;
    bool operator==(const ExportWarning&) const=default;
};
struct ExportRequest
{
    uint64_t session=0,generation=0;
    juce::String project,instance;
    bool operator==(const ExportRequest&) const=default;
};
struct ExportWetMember
{
    Swell accepted;
    int64_t start=0,end=0;
};
// Metadata only. Publish through shared_ptr<const ...>; PCM belongs to bounded
// worker pages, never this plan or persistent plugin state.
struct SongExportPlan
{
    CaptureManifest source;
    juce::String sourceReference,identity;
    SongTime timing;
    GainPolicy gainPolicy=GainPolicy::legacy;
    juce::String gainRevision;
    float wet=1;
    int64_t origin=0,end=0;
    std::vector<ExportWetMember> members;
    std::vector<ExportWarning> warnings;
};
// Input session must contain accepted versions (pending IR intent is resolved
// by the engine before calling). This pure builder does not verify asset bytes.
bool prepareSongExportPlan(const Session&,const CaptureManifest&,const juce::String& sourceReference,
                           const SongTime&,float wet,std::shared_ptr<const SongExportPlan>&,
                           juce::String& error);
juce::String songExportIdentity(const SongExportPlan&);
}
