#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>
#include "CaptureGenerationContext.h"

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

enum class CaptureSourceKind { song, loop };

struct LoopPhraseTiming
{
    double tempoBpm = 0;
    int meterNumerator = 0;
    int meterDenominator = 0;
    int beats = 0;
    int64_t originSample = 0; // Observed start in the capture run, not a live phase reference.
    int64_t endSample = 0;    // Exclusive bound in phrase-local coordinates.

    bool operator==(const LoopPhraseTiming&) const = default;
};

struct CaptureManifest
{
    static constexpr int legacySchemaVersion = 1, currentSchemaVersion = 2, loopSchemaVersion = 3;
    static constexpr int latestSupportedSchemaVersion = loopSchemaVersion;

    // Contextless low-level adapters retain legacy output. Workflow capture
    // explicitly publishes schema 2 together with complete pinned ownership.
    int schemaVersion = legacySchemaVersion;
    std::string projectId;
    std::string instanceId;
    double sampleRate = 48000.0;
    int channelCount = 2;
    uint64_t revision = 0;
    std::vector<CaptureManifestRange> ranges;
    std::vector<CaptureGenerationContext> generationContexts;
    std::vector<CaptureContextSpan> contextSpans;
    std::optional<LoopPhraseTiming> loop;

    CaptureSourceKind kind() const noexcept { return loop ? CaptureSourceKind::loop : CaptureSourceKind::song; }
};

bool isValidCaptureManifest(const CaptureManifest& manifest, juce::String& error);
bool isSafeCaptureAssetPath(const std::string& path) noexcept;
bool serializeCaptureManifest(const CaptureManifest& manifest, juce::String& json, juce::String& error);
bool parseCaptureManifest(const juce::String& json, CaptureManifest& manifest, juce::String& error);
