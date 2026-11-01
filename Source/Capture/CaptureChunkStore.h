#pragma once

#include "CaptureManifest.h"

#include <cstdint>
#include <string>
#include <vector>

#include <juce_audio_formats/juce_audio_formats.h>

struct CaptureAudioChunk
{
    int64_t startSample = 0;
    uint32_t frameCount = 0;
    double sampleRate = 0.0;
    int channelCount = 0;
    std::vector<float> interleavedSamples;
    std::string checksum;
};

bool writeFloat32StereoWavChunk(const juce::File& file,
                                int64_t startSample,
                                double sampleRate,
                                const float* interleavedSamples,
                                uint32_t frameCount,
                                CaptureAudioChunk& writtenChunk,
                                juce::String& error);

bool readFloat32StereoWavChunk(const juce::File& file,
                               const CaptureManifestRange& range,
                               double expectedSampleRate,
                               CaptureAudioChunk& chunk,
                               juce::String& error);