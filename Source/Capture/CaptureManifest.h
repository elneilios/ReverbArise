#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

enum class CaptureCoverageKind
{
    audio,
    silence
};

struct CaptureManifestRange
{
    int64_t startSample = 0;
    uint64_t frameCount = 0;
    CaptureCoverageKind kind = CaptureCoverageKind::audio;
    uint64_t revision = 0;
    std::string assetPath;
    std::string checksum;
};

struct CaptureManifest
{
    static constexpr int currentSchemaVersion = 1;

    int schemaVersion = currentSchemaVersion;
    std::string projectId;
    std::string instanceId;
    double sampleRate = 48000.0;
    int channelCount = 2;
    uint64_t revision = 0;
    std::vector<CaptureManifestRange> ranges;
};

bool isValidCaptureManifest(const CaptureManifest& manifest, juce::String& error);
bool isSafeCaptureAssetPath(const std::string& path) noexcept;
bool serializeCaptureManifest(const CaptureManifest& manifest, juce::String& json, juce::String& error);
bool parseCaptureManifest(const juce::String& json, CaptureManifest& manifest, juce::String& error);