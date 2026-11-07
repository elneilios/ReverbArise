#pragma once
#include <Storage/PortableAssets.h>
#include <Core/AssetAssociation.h>

namespace m3::storage::portable
{
// Worker-only verified folder/lease. Keeping this object alive pins the selected
// root and managed directory ancestry; no saved branch is reused as a writer.
class WorkingFolder;
struct WorkingFolderInfo
{
    juce::File folder,projectsRoot;
    Association association;
    Input expected;
    uint64_t files=0,bytes=0;
    bool incompleteRecovery=false,captureMetadataKnown=true,originalFolderUnavailable=false,recreatedCaptureMetadata=false;
    Association originalAssociation;
    juce::String recoveryRecord,restoreManifest;
    std::vector<juce::String> missing;
};
const WorkingFolderInfo& workingFolderInfo(const WorkingFolder&);
bool prepareWorkingFolder(const Input&,const juce::File& selectedFolder,const juce::File& registryRoot,
                          std::shared_ptr<WorkingFolder>&,juce::String& error,const Hooks& = {});
bool revalidateWorkingFolder(const WorkingFolder&,const Input& current,juce::String& error,const Hooks& = {});
// Explicit Locate verifies the saved store and exact host-state dependency
// metadata. Registry paths are only hints; mutable heads never select history.
bool locateWorkingFolder(const juce::File& selectedFolder,const Association&,const juce::String& legacyXml,
                         const juce::String& manifest,const juce::File& registryRoot,
                         std::shared_ptr<WorkingFolder>&,juce::String& error,const Hooks& = {});
bool resolveWorkingFolder(const Association&,const juce::String& legacyXml,const juce::String& manifest,
                          const juce::File& registryRoot,std::shared_ptr<WorkingFolder>&,juce::String& error,const Hooks& = {});
// Explicit incomplete recovery only: preserves original metadata and verified
// surviving aliases. Never emits a complete snapshot or backup receipt.
bool prepareRecoveryWorkspace(const Input&,const juce::File& selectedFolder,const juce::File& registryRoot,
                              std::shared_ptr<WorkingFolder>&,juce::String& error,const Hooks& = {});
bool revalidateRecoveryWorkspace(const WorkingFolder&,const Input&,juce::String& error,const Hooks& = {});
}
