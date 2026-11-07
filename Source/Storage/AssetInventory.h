#pragma once
#include <Core/SwellSession.h>
#include <atomic>
#include <map>
#include <set>

namespace m3::storage
{
inline constexpr size_t maximumMetadataBytes=8*1024*1024;
struct Dependency
{
    juce::String path,checksum,decodedIdentity,descriptor;
};
struct ObservedSnapshot
{
    juce::String label,project,instance,reference;
    CaptureManifest capture;
    Session session;
    std::vector<Dependency> dependencies;
    bool unknownCaptureHistory=false;
    size_t emptyIRReferences=0;
};
// Pure adapters: never adopt state, generate audio or touch the filesystem.
bool readLegacySnapshot(const juce::String& envelopeXml,const juce::String& manifestJson,
                        ObservedSnapshot&,juce::String& error);
bool visitDependencies(ObservedSnapshot&,juce::String& error);
struct Pin { juce::String label,path; };
struct Catalog
{
    std::map<juce::String,std::set<juce::String>> references;
    std::map<juce::String,std::set<juce::String>> aliases;
    std::vector<juce::String> uncertainty;
    void rebuild(const std::vector<ObservedSnapshot>&,const std::vector<Pin>&,bool priorIndexAvailable);
    // Legacy/offline FLPs cannot be enumerated. No catalog state grants deletion.
    bool protectedByDefault(const juce::String&)const noexcept{return true;}
};
struct Limits
{
    size_t entries=20000,depth=12;
    uint64_t hashBytes=16ull*1024*1024*1024,decodeBytes=32ull*1024*1024;
    double milliseconds=30000;
};
struct InventoryFile
{
    juce::String root,path,category,physicalId,sha256,decodedIdentity,metadata;
    uint64_t bytes=0,allocated=0;
    std::set<juce::String> observedBy;
};
struct Category {uint64_t files=0,bytes=0;};
struct Report
{
    std::vector<InventoryFile> files;
    std::map<juce::String,Category> categories;
    std::vector<juce::String> issues;
    Catalog catalog;
    uint64_t logicalBytes=0,physicalBytes=0,allocatedBytes=0,physicalFiles=0;
    uint64_t exactDuplicateBytes=0,decodedEquivalentFiles=0,decodedSkipped=0;
    uint64_t referencedFiles=0,missingReferences=0,observedCaptureSnapshots=0;
    uint64_t observedReferenceBytes=0,protectedHistoryOrUnknownBytes=0;
    bool inventoryComplete=true,dependencyVerificationComplete=true,cancelled=false;
    juce::Time scannedAt;
    // This phase has no eligible scratch class, collector or deletion operation.
    uint64_t reclaimableBytes=0;
};
struct Request
{
    std::vector<juce::File> roots;
    std::vector<ObservedSnapshot> snapshots;
    std::vector<Pin> activePins;
    bool priorIndexAvailable=false;
    juce::String observationError;
    Limits limits;
};
Report inspect(const Request&,const std::function<bool()>& cancelled={});
struct CaptureRecoveryCandidate
{
    juce::String reference,sha256;
    uint64_t revision=0;
    juce::StringArray unavailable;
};
struct CaptureRecoveryReport
{
    std::vector<CaptureRecoveryCandidate> snapshots;
    juce::StringArray unindexedAudio;
    bool complete=false;
};
// Observational only. Unindexed audio may be owned by another pass or FLP.
// Only an exact, independently verified snapshot can be recovered as a map.
CaptureRecoveryReport captureRecoveryCandidates(const Report&,const juce::File& instanceRoot,
                                                 const ObservedSnapshot& accepted);
class InventoryService final:private juce::Thread
{
public:
    InventoryService();
    ~InventoryService()override;
    uint64_t request(Request);
    void cancel();
    std::shared_ptr<const Report> snapshot()const{return result.load();}
    bool busy()const noexcept{return working.load();}
private:
    void run()override;
    juce::CriticalSection lock;
    std::optional<Request> pending;
    std::atomic<uint64_t> epoch{0};
    std::atomic<bool> working{false};
    std::atomic<std::shared_ptr<const Report>> result;
};
}
