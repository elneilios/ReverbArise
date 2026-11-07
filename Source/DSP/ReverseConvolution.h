#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
#include <functional>
#include <vector>

inline constexpr uint64_t previewMemoryBudget = 256ULL * 1024 * 1024;
using RenderCancellation = std::function<bool()>;
using RenderProgress = std::function<void(float)>;

struct ImpulseResponse
{
    double rate = 0;
    int channels = 0;
    std::vector<float> samples; // interleaved, original channel count
    juce::String name, identity;
    size_t frames() const noexcept { return channels > 0 ? samples.size() / static_cast<size_t>(channels) : 0; }
};

struct WetRender
{
    int64_t start = 0, sourceStart = 0, sourceEnd = 0;
    double rate = 0;
    std::vector<float> samples; // interleaved stereo
    float peak = 0;
    size_t frames() const noexcept { return samples.size() / 2; }
};

// Worker-side counters distinguish verified reload from regeneration/resampling.
struct RenderWorkCounts{uint64_t convolutions=0,resamplings=0;};
RenderWorkCounts renderWorkCounts() noexcept;
bool loadImpulseResponse(const juce::File&, uint64_t availableBytes, ImpulseResponse&, juce::String& error);
bool prepareImpulseResponse(const ImpulseResponse&, double targetRate, uint64_t availableBytes,
                            ImpulseResponse&, juce::String& error, const RenderCancellation& cancelled = {});
bool estimateRenderMemory(uint64_t sourceFrames, uint64_t irFrames, uint64_t occupiedBytes,
                          uint64_t& requiredBytes, juce::String& error);
bool renderReverseConvolution(const std::vector<float>& source, int64_t sourceStart, double rate,
                              const ImpulseResponse&, uint64_t occupiedBytes, WetRender&,
                              juce::String& error, const RenderCancellation& cancelled = {},
                              const RenderProgress& progress = {});
