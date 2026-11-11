#pragma once
#include "ReverseConvolution.h"
#include <Capture/CaptureSession.h>
#include <Capture/SourceRangeReader.h>
#include <Core/SongTime.h>
#include <Playback/AuditionVoice.h>
#include <optional>

enum class PreviewPhase { empty, loadingIR, canGenerate, rendering, ready, stale, error };
struct PreviewWaveform
{
    int64_t start = 0, end = 0, sourceStart = 0, sourceEnd = 0;
    double rate = 0;
    size_t frames = 0;
    float peak = 0;
    std::vector<CapturePeak> peaks;
};
struct PreviewSnapshot
{
    PreviewPhase phase = PreviewPhase::empty;
    juce::String status = "Load a WAV/WavPack impulse response (256 MiB preview budget)";
    juce::String irName;
    double irRate = 0;
    int irChannels = 0;
    int64_t startBar = 1, endBar = 2;
    float progress = 0;
    bool playing = false;
    float auditionLevelDb = -12, auditionAttenuationDb = 0;
    std::shared_ptr<const PreviewWaveform> waveform;
};
inline SongTime loopExportTime(const CaptureSessionSnapshot& capture) noexcept
{
    const auto& manifest=capture.manifest;
    if(!capture.manifestKnown || capture.reference.isEmpty() || !manifest.loop || !manifest.revision
       || capture.status.startsWith("Loading"))return {};
    const auto& loop=*manifest.loop;
    return {true,0,0,manifest.sampleRate,loop.tempoBpm,loop.meterNumerator,loop.meterDenominator};
}

struct PreviewContext
{
    CaptureSessionSnapshot capture;
    SongTime time;
    uint64_t captureGeneration = 0;
    SongTime exportTime() const noexcept
    {return capture.sourceKind()==CaptureSourceKind::loop?loopExportTime(capture):time;}
};

class PreviewEngine final : private juce::Thread
{
public:
    PreviewEngine(juce::File projectsRoot, std::function<PreviewContext()> context);
    ~PreviewEngine() override;
    void loadIR(const juce::File&); // Empty File means chooser cancelled: no change.
    void setSourceBars(int64_t start, int64_t end);
    void generate();
    bool audition();
    void setAuditionLevelDb(float db);
    void stop();
    void reset();
    void invalidate(const juce::String& reason);
    PreviewSnapshot snapshot() const;
    void processAudio(float* left, float* right, int frames, const SongTime& time) noexcept;
private:
    void run() override;
    void loadJob(const juce::File&, uint64_t request);
    void renderJob(uint64_t request);
    void inspectContext(const PreviewContext&);
    void invalidateLocked(const juce::String&);
    uint64_t occupiedBytesLocked() const;
    bool cancelled(uint64_t request) const noexcept;
    void report(uint64_t request, PreviewPhase, const juce::String&, float progress = 0);
    juce::File root;
    std::function<PreviewContext()> contextProvider;
    mutable juce::CriticalSection lock;
    PreviewSnapshot state;
    std::shared_ptr<const ImpulseResponse> ir;
    std::shared_ptr<const WetRender> current;
    std::optional<juce::File> pendingIR;
    bool pendingGenerate = false;
    std::atomic<uint64_t> requestId{0};
    AuditionVoice voice;
    std::optional<PreviewContext> dependencyContext;
    std::string dependency;
    int64_t sourceStart = 0, sourceEnd = 0;
    std::atomic<bool> audioAllowed{false}, audioMappingChanged{false};
    std::atomic<double> expectedBpm{0}, expectedRate{0};
    std::atomic<int> expectedNumerator{0}, expectedDenominator{0};
};
