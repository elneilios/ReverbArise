#include "Capture/AudioCaptureQueue.h"
#include "Capture/CaptureAssetStore.h"
#include "Capture/CaptureChunkStore.h"
#include "Capture/CaptureManifest.h"
#include "Capture/CaptureContinuity.h"
#include "Capture/SparseSongMap.h"

#include <array>

namespace
{
struct TemporaryDirectory
{
    juce::File file = juce::File::getSpecialLocation(juce::File::tempDirectory)
                          .getChildFile("PreverbChunkTest-" + juce::Uuid().toString());

    ~TemporaryDirectory()
    {
        file.deleteRecursively();
    }
};

bool testFloatWavChunkRoundTrip()
{
    TemporaryDirectory temporaryDirectory;
    if (temporaryDirectory.file.createDirectory().failed())
        return false;

    const auto wavFile = temporaryDirectory.file.getChildFile("capture.wav");
    const std::array<float, 10> samples{0.25f, -0.75f, 0.5f, -0.5f, 0.0f,
                                        1.25f, -1.25f, -0.125f, 0.875f, 0.375f};
    CaptureAudioChunk writtenChunk;
    juce::String error;
    if (! writeFloat32StereoWavChunk(wavFile, 123456, 48000.0, samples.data(), 5, writtenChunk, error))
        return false;

    if (writtenChunk.startSample != 123456 || writtenChunk.frameCount != 5
        || writtenChunk.sampleRate != 48000.0 || writtenChunk.channelCount != 2
        || writtenChunk.interleavedSamples.size() != samples.size()
        || writtenChunk.checksum.size() != 64)
        return false;

    CaptureManifestRange range;
    range.startSample = writtenChunk.startSample;
    range.frameCount = writtenChunk.frameCount;
    range.kind = CaptureCoverageKind::audio;
    range.revision = 1;
    range.assetPath = "captures/capture.wav";
    range.checksum = writtenChunk.checksum;

    CaptureAudioChunk restoredChunk;
    if (! readFloat32StereoWavChunk(wavFile, range, 48000.0, restoredChunk, error))
        return false;

    if (restoredChunk.startSample != writtenChunk.startSample
        || restoredChunk.frameCount != writtenChunk.frameCount
        || restoredChunk.sampleRate != writtenChunk.sampleRate
        || restoredChunk.channelCount != 2
        || restoredChunk.checksum != writtenChunk.checksum
        || restoredChunk.interleavedSamples.size() != samples.size())
        return false;

    for (size_t sample = 0; sample < samples.size(); ++sample)
        if (restoredChunk.interleavedSamples[sample] != samples[sample])
            return false;

    range.checksum = std::string(64, '0');
    if (readFloat32StereoWavChunk(wavFile, range, 48000.0, restoredChunk, error))
        return false;

    return true;
}

bool testAtomicCaptureAssetStore()
{
    TemporaryDirectory temporaryDirectory;
    if (temporaryDirectory.file.createDirectory().failed())
        return false;

    CaptureAssetStore store(temporaryDirectory.file.getChildFile("managed-cache"));
    const std::array<float, 8> samples{0.25f, -0.25f, 0.5f, -0.5f, 0.75f, -0.75f, 1.0f, -1.0f};
    CaptureManifest manifest;
    manifest.projectId = "123e4567-e89b-12d3-a456-426614174000";
    manifest.instanceId = "123e4567-e89b-12d3-a456-426614174001";
    manifest.sampleRate = 48000.0;
    manifest.channelCount = 2;
    manifest.revision = 1;

    CaptureManifestRange firstRange;
    firstRange.startSample = 4000;
    firstRange.frameCount = 4;
    firstRange.kind = CaptureCoverageKind::audio;
    firstRange.revision = 1;
    firstRange.assetPath = "segments/revision-1/first.wav";
    juce::String error;
    if (! store.writeAudioChunk(firstRange, manifest.sampleRate, samples.data(), error))
        return false;
    manifest.ranges.push_back(firstRange);
    if (! store.publishManifest(manifest, error))
        return false;

    CaptureManifest loaded;
    if (! store.loadManifest(loaded, error) || loaded.revision != 1 || loaded.ranges.size() != 1)
        return false;

    CaptureAudioChunk restoredChunk;
    if (! store.readAudioChunk(loaded.ranges.front(), loaded.sampleRate, restoredChunk, error)
        || restoredChunk.startSample != 4000 || restoredChunk.interleavedSamples.size() != samples.size())
        return false;
    for (size_t sample = 0; sample < samples.size(); ++sample)
        if (restoredChunk.interleavedSamples[sample] != samples[sample])
            return false;

    auto interruptedUpdate = loaded;
    interruptedUpdate.revision = 2;
    interruptedUpdate.ranges.push_back({9000, 2, CaptureCoverageKind::audio, 2,
                                        "segments/revision-2/missing.wav", std::string(64, 'b')});
    if (store.publishManifest(interruptedUpdate, error))
        return false;

    CaptureManifest afterFailure;
    if (! store.loadManifest(afterFailure, error)
        || afterFailure.revision != 1
        || afterFailure.ranges.size() != 1
        || ! store.readAudioChunk(afterFailure.ranges.front(), afterFailure.sampleRate, restoredChunk, error))
        return false;

    return true;
}
}

int main()
{
    SparseSongMap map;
    const std::array<float, 6> firstBlock{0.1f, -0.1f, 0.2f, -0.2f, 0.3f, -0.3f};
    const std::array<float, 4> secondBlock{0.4f, -0.4f, 0.5f, -0.5f};
    map.write(100, firstBlock.data(), 3);
    map.write(200, secondBlock.data(), 2);

    if (map.segmentCount() != 2 || ! map.isCovered(100) || ! map.isCovered(102)
        || map.isCovered(103) || map.isCovered(150) || ! map.isCovered(201))
        return 1;

    const std::array<float, 8> capturedSilence{};
    map.write(300, capturedSilence.data(), 4);
    float left = 1.0f;
    float right = 1.0f;
    if (! map.getSample(303, left, right) || left != 0.0f || right != 0.0f || map.isCovered(304))
        return 2;

    const std::array<float, 16> original{
        0.0f, 0.0f, 1.0f, -1.0f, 2.0f, -2.0f, 3.0f, -3.0f,
        4.0f, -4.0f, 5.0f, -5.0f, 6.0f, -6.0f, 7.0f, -7.0f
    };
    const std::array<float, 4> replacement{30.0f, -30.0f, 40.0f, -40.0f};
    map.clear();
    map.write(10, original.data(), 8);
    map.write(13, replacement.data(), 2);

    if (map.segmentCount() != 1)
        return 3;
    if (! map.getSample(12, left, right) || left != 2.0f || right != -2.0f)
        return 4;
    if (! map.getSample(13, left, right) || left != 30.0f || right != -30.0f)
        return 5;
    if (! map.getSample(14, left, right) || left != 40.0f || right != -40.0f)
        return 6;
    if (! map.getSample(15, left, right) || left != 5.0f || right != -5.0f)
        return 7;

    SpscAudioCaptureQueue<2> queue;
    const std::array<float, 2> leftSamples{0.25f, 0.5f};
    const std::array<float, 2> rightSamples{-0.25f, -0.5f};
    if (! queue.tryPush(50, 48000, leftSamples.data(), rightSamples.data(), 2)
        || ! queue.tryPush(52, 48000, leftSamples.data(), rightSamples.data(), 2)
        || queue.tryPush(54, 48000, leftSamples.data(), rightSamples.data(), 2))
        return 8;

    AudioCaptureBlock block;
    if (! queue.tryPop(block) || block.startSample != 50 || block.frameCount != 2
        || block.interleavedSamples[0] != 0.25f || block.interleavedSamples[1] != -0.25f)
        return 9;

    CaptureContinuity continuity;
    if (continuity.beginBlock(100, 4) || continuity.beginBlock(104, 4)
        || ! continuity.beginBlock(1000, 4) || ! continuity.beginBlock(0, 4))
        return 10;

    CaptureManifest manifest;
    manifest.projectId = "123e4567-e89b-12d3-a456-426614174000";
    manifest.instanceId = "123e4567-e89b-12d3-a456-426614174001";
    manifest.sampleRate = 48000.0;
    manifest.channelCount = 2;
    manifest.revision = 3;
    manifest.ranges = {
        {100, 4, CaptureCoverageKind::audio, 2, "captures/revision-2/chunk.wav", std::string(64, 'a')},
        {110, 8, CaptureCoverageKind::silence, 3, {}, {}}
    };

    juce::String manifestJson;
    juce::String manifestError;
    if (! serializeCaptureManifest(manifest, manifestJson, manifestError))
        return 11;

    CaptureManifest decodedManifest;
    if (! parseCaptureManifest(manifestJson, decodedManifest, manifestError)
        || decodedManifest.projectId != manifest.projectId
        || decodedManifest.instanceId != manifest.instanceId
        || decodedManifest.revision != manifest.revision
        || decodedManifest.ranges.size() != 2
        || decodedManifest.ranges[0].startSample != 100
        || decodedManifest.ranges[1].kind != CaptureCoverageKind::silence)
        return 12;

    if (! isSafeCaptureAssetPath("captures/revision-2/chunk.wav")
        || isSafeCaptureAssetPath("../escape.wav")
        || isSafeCaptureAssetPath("C:/outside.wav")
        || isSafeCaptureAssetPath("captures\\chunk.wav"))
        return 13;

    const auto invalidJson = juce::String("{not valid json");
    if (parseCaptureManifest(invalidJson, decodedManifest, manifestError))
        return 14;

    auto overlappingManifest = manifest;
    overlappingManifest.ranges[1].startSample = 103;
    if (isValidCaptureManifest(overlappingManifest, manifestError))
        return 15;

    if (! testFloatWavChunkRoundTrip())
        return 16;
    if (! testAtomicCaptureAssetStore())
        return 17;

    return 0;
}
