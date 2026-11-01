#include "CaptureChunkStore.h"

#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>

#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace
{
struct BCryptHandles
{
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;

    ~BCryptHandles()
    {
        if (hash != nullptr)
            BCryptDestroyHash(hash);
        if (algorithm != nullptr)
            BCryptCloseAlgorithmProvider(algorithm, 0);
    }
};

bool calculateSha256(const juce::File& file, std::string& checksum, juce::String& error)
{
    std::unique_ptr<juce::InputStream> input(file.createInputStream());
    if (input == nullptr)
    {
        error = "Could not open the completed WAV chunk for checksum validation";
        return false;
    }

    BCryptHandles handles;
    if (BCryptOpenAlgorithmProvider(&handles.algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
    {
        error = "Could not initialize the SHA-256 provider";
        return false;
    }

    ULONG objectLength = 0;
    ULONG digestLength = 0;
    ULONG bytesWritten = 0;
    if (BCryptGetProperty(handles.algorithm, BCRYPT_OBJECT_LENGTH,
                          reinterpret_cast<PUCHAR>(&objectLength), sizeof(objectLength), &bytesWritten, 0) < 0
        || BCryptGetProperty(handles.algorithm, BCRYPT_HASH_LENGTH,
                             reinterpret_cast<PUCHAR>(&digestLength), sizeof(digestLength), &bytesWritten, 0) < 0)
    {
        error = "Could not query SHA-256 provider properties";
        return false;
    }

    std::vector<UCHAR> hashObject(objectLength);
    std::vector<UCHAR> digest(digestLength);
    if (BCryptCreateHash(handles.algorithm, &handles.hash, hashObject.data(), objectLength,
                         nullptr, 0, 0) < 0)
    {
        error = "Could not create a SHA-256 hash";
        return false;
    }

    std::array<UCHAR, 65536> bytes{};
    for (;;)
    {
        const auto count = input->read(bytes.data(), static_cast<int>(bytes.size()));
        if (count < 0)
        {
            error = "Could not read the completed WAV chunk for checksum validation";
            return false;
        }
        if (count == 0)
            break;

        if (BCryptHashData(handles.hash, bytes.data(), static_cast<ULONG>(count), 0) < 0)
        {
            error = "Could not hash the completed WAV chunk";
            return false;
        }
    }

    if (BCryptFinishHash(handles.hash, digest.data(), digestLength, 0) < 0)
    {
        error = "Could not finalize the WAV chunk checksum";
        return false;
    }

    constexpr char hexDigits[] = "0123456789abcdef";
    checksum.clear();
    checksum.reserve(digest.size() * 2);
    for (const auto byte : digest)
    {
        checksum.push_back(hexDigits[(byte >> 4) & 0x0f]);
        checksum.push_back(hexDigits[byte & 0x0f]);
    }

    error.clear();
    return true;
}
}

bool writeFloat32StereoWavChunk(const juce::File& file,
                                int64_t startSample,
                                double sampleRate,
                                const float* interleavedSamples,
                                uint32_t frameCount,
                                CaptureAudioChunk& writtenChunk,
                                juce::String& error)
{
    if (file.existsAsFile() || ! file.getParentDirectory().isDirectory())
    {
        error = "WAV chunk destination must be a new file in an existing directory";
        return false;
    }

    if (interleavedSamples == nullptr || frameCount == 0
        || frameCount > static_cast<uint32_t>(std::numeric_limits<int>::max())
        || startSample > std::numeric_limits<int64_t>::max() - static_cast<int64_t>(frameCount)
        || ! std::isfinite(sampleRate) || sampleRate <= 0.0)
    {
        error = "WAV chunk input metadata or sample buffer is invalid";
        return false;
    }

    juce::AudioBuffer<float> buffer(2, static_cast<int>(frameCount));
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    for (uint32_t frame = 0; frame < frameCount; ++frame)
    {
        left[frame] = interleavedSamples[static_cast<size_t>(frame) * 2];
        right[frame] = interleavedSamples[static_cast<size_t>(frame) * 2 + 1];
    }

    juce::WavAudioFormat format;
    juce::AudioFormatWriterOptions options;
    options = options.withSampleRate(sampleRate)
                     .withNumChannels(2)
                     .withBitsPerSample(32)
                     .withSampleFormat(juce::AudioFormatWriterOptions::SampleFormat::floatingPoint);

    std::unique_ptr<juce::OutputStream> output(file.createOutputStream());
    if (output == nullptr)
    {
        error = "Could not create the WAV chunk output stream";
        return false;
    }

    auto writer = format.createWriterFor(output, options);
    if (writer == nullptr || writer->getNumChannels() != 2 || writer->getBitsPerSample() != 32 || ! writer->isFloatingPoint())
    {
        writer.reset();
        file.deleteFile();
        error = "JUCE could not create a stereo 32-bit-float WAV writer";
        return false;
    }

    const auto writeSucceeded = writer->writeFromAudioSampleBuffer(buffer, 0, static_cast<int>(frameCount));
    writer.reset();
    if (! writeSucceeded)
    {
        file.deleteFile();
        error = "Could not write the complete WAV chunk";
        return false;
    }

    std::string checksum;
    if (! calculateSha256(file, checksum, error))
    {
        file.deleteFile();
        return false;
    }

    CaptureAudioChunk result;
    result.startSample = startSample;
    result.frameCount = frameCount;
    result.sampleRate = sampleRate;
    result.channelCount = 2;
    result.interleavedSamples.assign(interleavedSamples,
                                     interleavedSamples + static_cast<size_t>(frameCount) * 2);
    result.checksum = std::move(checksum);
    writtenChunk = std::move(result);
    error.clear();
    return true;
}

bool readFloat32StereoWavChunk(const juce::File& file,
                               const CaptureManifestRange& range,
                               double expectedSampleRate,
                               CaptureAudioChunk& chunk,
                               juce::String& error)
{
    if (range.kind != CaptureCoverageKind::audio || ! isSafeCaptureAssetPath(range.assetPath)
        || ! std::isfinite(expectedSampleRate) || expectedSampleRate <= 0.0)
    {
        error = "WAV chunk metadata is invalid";
        return false;
    }

    if (range.frameCount == 0 || range.frameCount > static_cast<uint64_t>(std::numeric_limits<int>::max()))
    {
        error = "WAV chunk frame count is invalid";
        return false;
    }

    std::string actualChecksum;
    if (! calculateSha256(file, actualChecksum, error))
        return false;
    if (actualChecksum != range.checksum)
    {
        error = "WAV chunk checksum does not match the manifest";
        return false;
    }

    std::unique_ptr<juce::InputStream> input(file.createInputStream());
    if (input == nullptr)
    {
        error = "Could not open the WAV chunk";
        return false;
    }

    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(input.release(), true));
    if (reader == nullptr || reader->numChannels != 2 || reader->bitsPerSample != 32
        || ! reader->usesFloatingPointData
        || reader->lengthInSamples != static_cast<int64_t>(range.frameCount)
        || ! juce::approximatelyEqual(reader->sampleRate, expectedSampleRate))
    {
        error = "WAV chunk format or metadata does not match the manifest";
        return false;
    }

    juce::AudioBuffer<float> buffer(2, static_cast<int>(range.frameCount));
    if (! reader->read(&buffer, 0, static_cast<int>(range.frameCount), 0, true, true))
    {
        error = "Could not read the complete WAV chunk";
        return false;
    }

    CaptureAudioChunk result;
    result.startSample = range.startSample;
    result.frameCount = static_cast<uint32_t>(range.frameCount);
    result.sampleRate = reader->sampleRate;
    result.channelCount = static_cast<int>(reader->numChannels);
    result.interleavedSamples.resize(static_cast<size_t>(result.frameCount) * 2);
    const auto* left = buffer.getReadPointer(0);
    const auto* right = buffer.getReadPointer(1);
    for (uint32_t frame = 0; frame < result.frameCount; ++frame)
    {
        result.interleavedSamples[static_cast<size_t>(frame) * 2] = left[frame];
        result.interleavedSamples[static_cast<size_t>(frame) * 2 + 1] = right[frame];
    }
    result.checksum = std::move(actualChecksum);
    chunk = std::move(result);
    error.clear();
    return true;
}