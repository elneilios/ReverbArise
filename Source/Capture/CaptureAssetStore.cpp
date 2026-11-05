#include "CaptureAssetStore.h"
#include <Core/ManagedDirectory.h>

#define NOMINMAX
#include <Windows.h>

#include <limits>
#include <memory>
#include <string>

CaptureAssetStore::CaptureAssetStore(juce::File root)
    : cacheRoot(std::move(root))
{
}

bool CaptureAssetStore::resolveAssetFile(const CaptureManifestRange& range,
                                         juce::File& file,
                                         juce::String& error) const
{
    if (range.kind != CaptureCoverageKind::audio || ! isSafeCaptureAssetPath(range.assetPath))
    {
        error = "Capture asset path is not a safe relative audio path";
        return false;
    }

    const auto relativePath = juce::String::fromUTF8(range.assetPath.data(),
                                                      static_cast<int>(range.assetPath.size()));
    file = cacheRoot.getChildFile(relativePath);
    error.clear();
    return true;
}

bool CaptureAssetStore::writeAudioChunk(CaptureManifestRange& range,
                                        double sampleRate,
                                        const float* interleavedSamples,
                                        juce::String& error)
{
    juce::File destination;
    if (! resolveAssetFile(range, destination, error))
        return false;

    if (range.frameCount == 0 || range.frameCount > std::numeric_limits<uint32_t>::max()
        || destination.existsAsFile())
    {
        error = "Capture chunk size is invalid or its immutable destination already exists";
        return false;
    }

    if (ensureManagedDirectory(cacheRoot).failed() || ensureManagedDirectory(destination.getParentDirectory()).failed())
    {
        error = "Could not create the capture cache directory";
        return false;
    }

    const auto temporary = destination.getSiblingFile(destination.getFileName() + ".tmp-" + juce::Uuid().toString());
    CaptureAudioChunk writtenChunk;
    if (! writeFloat32StereoWavChunk(temporary,
                                     range.startSample,
                                     sampleRate,
                                     interleavedSamples,
                                     static_cast<uint32_t>(range.frameCount),
                                     writtenChunk,
                                     error))
        return false;

    auto temporaryRange = range;
    temporaryRange.checksum = writtenChunk.checksum;
    CaptureAudioChunk verifiedChunk;
    if (! readFloat32StereoWavChunk(temporary, temporaryRange, sampleRate, verifiedChunk, error))
    {
        temporary.deleteFile();
        return false;
    }

    if (! moveAtomically(temporary, destination, false, error))
    {
        temporary.deleteFile();
        return false;
    }

    range.checksum = std::move(writtenChunk.checksum);
    verifiedAssets[range.assetPath] = {range.checksum, range.frameCount, sampleRate,
                                      destination.getSize(), destination.getLastModificationTime()};
    return true;
}

bool CaptureAssetStore::readAudioChunk(const CaptureManifestRange& range,
                                       double sampleRate,
                                       CaptureAudioChunk& chunk,
                                       juce::String& error) const
{
    if (range.frameCount > 1920000 || static_cast<double>(range.frameCount) > sampleRate * 5.0)
    {
        error = "Capture chunk exceeds the bounded chunk size";
        return false;
    }
    juce::File file;
    if (! resolveAssetFile(range, file, error))
        return false;

    ++audioReads;
    if (! readFloat32StereoWavChunk(file, range, sampleRate, chunk, error))
    {
        verifiedAssets.erase(range.assetPath);
        return false;
    }
    verifiedAssets[range.assetPath] = {range.checksum, range.frameCount, sampleRate,
                                      file.getSize(), file.getLastModificationTime()};
    return true;
}

bool CaptureAssetStore::validateAssets(const CaptureManifest& manifest, juce::String& error) const
{
    for (const auto& range : manifest.ranges)
    {
        if (range.kind == CaptureCoverageKind::silence)
            continue;

        juce::File file;
        if (! resolveAssetFile(range, file, error)) return false;
        const auto previous = verifiedAssets.find(range.assetPath);
        // Files are immutable. Revalidate if their identity or observed disk metadata changes.
        // A fresh store on restore always checks every referenced asset.
        if (previous != verifiedAssets.end() && file.existsAsFile()
            && previous->second.checksum == range.checksum && previous->second.frames == range.frameCount
            && previous->second.rate == manifest.sampleRate && previous->second.bytes == file.getSize()
            && previous->second.modified == file.getLastModificationTime())
            continue;

        CaptureAudioChunk chunk;
        if (! readAudioChunk(range, manifest.sampleRate, chunk, error))
            return false;
    }

    error.clear();
    return true;
}

bool CaptureAssetStore::publishManifest(const CaptureManifest& manifest, juce::String& error) const
{
    if (! isValidCaptureManifest(manifest, error) || ! validateAssets(manifest, error))
        return false;

    if (ensureManagedDirectory(cacheRoot).failed())
    {
        error = "Could not create the capture cache directory";
        return false;
    }

    juce::String json;
    if (! serializeCaptureManifest(manifest, json, error))
        return false;

    const auto manifestFile = cacheRoot.getChildFile("manifest.json");
    const auto temporary = cacheRoot.getChildFile("manifest.json.tmp-" + juce::Uuid().toString());
    if (! temporary.replaceWithText(json))
    {
        error = "Could not write a temporary capture manifest";
        temporary.deleteFile();
        return false;
    }

    CaptureManifest parsedTemporary;
    const auto temporaryJson = temporary.loadFileAsString();
    if (! parseCaptureManifest(temporaryJson, parsedTemporary, error)
        || ! validateAssets(parsedTemporary, error))
    {
        temporary.deleteFile();
        return false;
    }

    if (! moveAtomically(temporary, manifestFile, true, error))
    {
        temporary.deleteFile();
        return false;
    }

    error.clear();
    return true;
}

bool CaptureAssetStore::loadManifest(CaptureManifest& manifest, juce::String& error) const
{
    const auto manifestFile = cacheRoot.getChildFile("manifest.json");
    if (! manifestFile.existsAsFile())
    {
        error = "Capture manifest does not exist";
        return false;
    }

    CaptureManifest parsed;
    if (! parseCaptureManifest(manifestFile.loadFileAsString(), parsed, error)
        || ! validateAssets(parsed, error))
        return false;

    manifest = std::move(parsed);
    error.clear();
    return true;
}

bool CaptureAssetStore::publishSnapshot(const CaptureManifest& manifest, juce::String& reference,
                                        juce::String& error,CaptureFailureReason* reason) const
{
    if(reason)*reason=CaptureFailureReason::assetValidation;
    juce::String json;
    if (! serializeCaptureManifest(manifest, json, error) || ! validateAssets(manifest, error))
        return false;
    if(reason)*reason=CaptureFailureReason::publication;
    const auto name = "manifests/" + juce::Uuid().toDashedString() + ".json";
    const auto destination = cacheRoot.getChildFile(name);
    if (ensureManagedDirectory(destination.getParentDirectory()).failed())
    {
        error = "Could not create manifest directory";
        return false;
    }
    const auto temporary = destination.getSiblingFile(destination.getFileName() + ".tmp");
    if (! temporary.replaceWithText(json) || ! moveAtomically(temporary, destination, false, error))
    {
        if (error.isEmpty()) error = "Could not write manifest snapshot";
        return false;
    }
    reference = name;
    return true;
}

bool CaptureAssetStore::loadSnapshot(const juce::String& reference, CaptureManifest& manifest,
                                     juce::String& error) const
{
    if (! isSafeCaptureAssetPath(reference.toStdString()) || ! reference.startsWith("manifests/")
        || ! reference.endsWith(".json"))
    {
        error = "Unsafe manifest reference";
        return false;
    }
    const auto file = cacheRoot.getChildFile(reference);
    if (! file.existsAsFile())
    {
        error = "Saved manifest is missing";
        return false;
    }
    return parseCaptureManifest(file.loadFileAsString(), manifest, error);
}

bool CaptureAssetStore::moveAtomically(const juce::File& source,
                                       const juce::File& destination,
                                       bool replaceExisting,
                                       juce::String& error)
{
    auto flags = MOVEFILE_WRITE_THROUGH;
    if (replaceExisting)
        flags |= MOVEFILE_REPLACE_EXISTING;

    if (! MoveFileExW(source.getFullPathName().toWideCharPointer(),
                      destination.getFullPathName().toWideCharPointer(),
                      flags))
    {
        error = "Could not atomically publish capture data (Windows error "
            + juce::String(static_cast<int>(GetLastError())) + ")";
        return false;
    }

    error.clear();
    return true;
}
