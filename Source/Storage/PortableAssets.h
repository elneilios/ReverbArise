#pragma once
#include <Storage/AssetInventory.h>

namespace m3::storage::portable
{
// All APIs are worker/control-thread only. No operation deletes originals,
// changes an association, edits the DAW project or generates/resamples audio.
inline constexpr int storeVersion=1,snapshotVersion=1,descriptorVersion=1,receiptVersion=1,journalVersion=1,readerVersion=1;
inline constexpr const char* scopeExclusions="Pending work, other instances, older FLPs and external Playlist copies are excluded.";
inline constexpr size_t maxDocumentBytes=64*1024*1024,maxFiles=20000;
struct Input
{
    juce::File instanceRoot;
    juce::String processorXml,manifestJson;
    juce::String producer="Synthetic/control input";
    juce::String exclusions=scopeExclusions;
    bool sourceUnavailable=false,folderUnavailable=false; // Runtime admission only; never a complete-pack certificate.
    uint64_t controlEpoch=0; // Runtime binding token only; never serialized into a backup.
};
struct Entry {juce::String path,object,sha256;uint64_t bytes=0;};
struct Snapshot
{
    juce::String id,processorXml,manifestJson,exclusions,producer;
    ObservedSnapshot observed;
    std::vector<Entry> files;
};
struct Locator {juce::String store,snapshot;};
struct Receipt
{
    Locator locator;
    juce::String project,instance,exclusions;
    uint64_t files=0,bytes=0;
    bool unknownCaptureHistory=false;
    juce::String producer;
};
bool safeRelativePath(const juce::String&);
bool decodeSnapshot(const juce::String&,Snapshot&,juce::String& error);
juce::String encodeSnapshot(const Snapshot&);
bool decodeReceipt(const juce::String&,Receipt&,juce::String& error);
juce::String encodeReceipt(const Receipt&);

// File handles pin the accepted immutable source dependencies, denying new
// writers/deletion. This object never claims to discover other FLPs/instances.
struct Hooks
{
    std::function<bool()> cancelled;
    // Native fault seam: zero performs normal I/O; nonzero models a Win32 write failure.
    std::function<uint32_t(uint64_t,uint32_t)> injectWriteError;
    // Fault/race checkpoints for native tests; false aborts without publication.
    std::function<bool(const juce::String&,const juce::String&)> checkpoint;
};
class Prepared;
bool prepare(const Input&,std::shared_ptr<Prepared>&,juce::String& error,const Hooks& = {});
const Snapshot& snapshot(const Prepared&);
struct Result
{
    bool success=false,cancelled=false;
    juce::String operation,error;
    Receipt receipt;
};
// Explicit original-instance-folder recovery; no store/association is adopted.
bool prepareLegacyRecovery(const juce::File& selectedInstance,const Input& target,std::shared_ptr<Prepared>&,juce::String& error,const Hooks& = {});
Result restoreLegacyCopy(const std::shared_ptr<Prepared>&,const juce::File& selectedProjectsRoot,const Hooks& = {});
Result backup(const std::shared_ptr<Prepared>&,const juce::File& selectedFolder,const Hooks& = {});
// Resume accepts only an explicitly supplied source root; journal hints never
// authorize discovery. Already published objects are verified, never overwritten.
Result resume(const juce::File& selectedFolder,const juce::String& operation,
              const juce::File& explicitSourceRoot,const Hooks& = {});
bool inspectInterrupted(const juce::File& selectedFolder,const juce::String& operation,
                        Snapshot&,juce::String& error);
// Advisory evidence only. The reader opens existing files and directories and
// has no action token, mutation API or automatic invocation from inventory.
struct ScratchPreviewEntry
{
    juce::String operation,path,reason;
    uint64_t bytes=0;
    juce::String evidence; // Binds reviewed bytes and handle identities. Not action approval.
};
struct ScratchPreview
{
    juce::String store,error;
    std::vector<ScratchPreviewEntry> eligible,excluded;
    uint64_t bytes=0;
    bool complete=false,cancelled=false;
};
// Both roots are explicit. The counterpart must be the original instance
// directory recorded by the Backup producer, never a path from its journal.
ScratchPreview previewScratch(const juce::File& selectedStore,const juce::File& originalInstance,const Hooks& = {});
class ScratchMaintenance;
// Worker only. Reopens one explicitly reviewed path and calls ready synchronously
// while the store, journal, staged file and surviving copy are pinned. Never
// retain the callback's view or reopen the staged path for an action.
bool withRevalidatedScratch(const juce::File& selectedStore,const juce::File& originalInstance,
                           const ScratchPreview& reviewed,const juce::String& selectedPath,
                           const std::function<void(const ScratchMaintenance&)>& ready,
                           juce::String& error,const Hooks& = {});
enum class ScratchQuarantineState { unavailable,staged,quarantined,restored,ambiguous };
struct ScratchQuarantineInfo
{
    ScratchQuarantineState state=ScratchQuarantineState::unavailable;
    juce::String operation,originalPath,quarantinePath,error;
    uint64_t bytes=0;
    bool undoPending=false;
};
// Worker only. A caller must obtain explicit approval for the exact reviewed
// entry before quarantine, and for the exact operation before undo. Inspection
// reconciles on-disk evidence without attempting a repair. After any failure,
// inspect again before reporting which location currently holds the bytes.
bool quarantineScratch(const juce::File& selectedStore,const juce::File& originalInstance,
                       const ScratchPreview& reviewed,const juce::String& selectedPath,
                       juce::String& error,const Hooks& = {});
ScratchQuarantineInfo inspectScratchQuarantine(const juce::File& selectedStore,const juce::String& operation,
                                               const Hooks& = {});
bool undoScratchQuarantine(const juce::File& selectedStore,const juce::String& operation,
                           juce::String& error,const Hooks& = {});
class Verified;
bool locate(const juce::File& selectedFolder,const Locator& expected,
            std::shared_ptr<Verified>&,juce::String& error,const Hooks& = {});
// Explicit completion-receipt selection supports reopening without a local registry.
// The selected receipt is read through the same pinned-root policy, then the
// exact store/snapshot and complete closure are verified; no head inference.
bool locateReceipt(const juce::File& selectedReceipt,std::shared_ptr<Verified>&,juce::String& error,const Hooks& = {});
// Validate current accepted references against a verified backup without adopting its edits.
bool matchesRecoveryTarget(const Verified&,const Input&,juce::String& error);
const Snapshot& snapshot(const Verified&);
const Receipt& receipt(const Verified&);
// An explicit recovery copy into a selected existing root, recreating legacy
// aliases. No live plugin state/root switch occurs here. Existing conflicting
// bytes fail; existing identical bytes are verified and preserved.
Result restoreCopy(const std::shared_ptr<Verified>&,const juce::File& selectedProjectsRoot,const Hooks& = {});
}
