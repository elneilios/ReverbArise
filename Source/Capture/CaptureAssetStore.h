#pragma once

#include "CaptureChunkStore.h"
#include "CaptureFailure.h"

#include <juce_core/juce_core.h>
#include <unordered_map>

class CaptureAssetStore
{
public:
    explicit CaptureAssetStore(juce::File cacheRoot);

    bool writeAudioChunk(CaptureManifestRange& range,
                         double sampleRate,
                         const float* interleavedSamples,
                         juce::String& error);
    bool readAudioChunk(const CaptureManifestRange& range,
                        double sampleRate,
                        CaptureAudioChunk& chunk,
                        juce::String& error) const;
    bool publishManifest(const CaptureManifest& manifest, juce::String& error) const;
    bool loadManifest(CaptureManifest& manifest, juce::String& error) const;
    // Immutable project-state references; loading metadata permits partial asset recovery.
    bool publishSnapshot(const CaptureManifest& manifest, juce::String& reference, juce::String& error,CaptureFailureReason* reason=nullptr) const;
    bool loadSnapshot(const juce::String& reference, CaptureManifest& manifest, juce::String& error) const;
    uint64_t audioReadCount() const noexcept { return audioReads; }

private:
    bool resolveAssetFile(const CaptureManifestRange& range,
                          juce::File& file,
                          juce::String& error) const;
    bool validateAssets(const CaptureManifest& manifest, juce::String& error) const;
    static bool moveAtomically(const juce::File& source,
                               const juce::File& destination,
                               bool replaceExisting,
                               juce::String& error);

    juce::File cacheRoot;
    struct VerifiedAsset
    {
        std::string checksum;
        uint64_t frames = 0;
        double rate = 0;
        juce::int64 bytes = 0;
        juce::Time modified;
    };
    mutable std::unordered_map<std::string, VerifiedAsset> verifiedAssets;
    mutable uint64_t audioReads = 0;
};
