#pragma once

#include "CaptureAssetStore.h"

struct CapturePeak
{
    int64_t startSample = 0;
    uint32_t frameCount = 0;
    float minimum = 0.0f, maximum = 0.0f;
};

struct CaptureOverviewRange
{
    CaptureManifestRange range;
    bool available = true;
    std::vector<CapturePeak> peaks;
};

struct CaptureSessionSnapshot
{
    CaptureManifest manifest;
    juce::String reference;
    juce::String status = "Ready (empty)";
    bool unavailable = false;
    std::vector<CaptureOverviewRange> overview;
};

// Worker-thread only. PCM is read one bounded chunk at a time, never retained for the full song.
class CaptureSession
{
public:
    explicit CaptureSession(juce::File projectsRoot);
    bool restore(const std::string& projectId, const std::string& instanceId,
                 const juce::String& reference, uint64_t revision);
    bool commit(int64_t start, double rate, const float* samples, uint32_t frames,double tolerance=0);
    bool clear(double rate);
    const CaptureSessionSnapshot& snapshot() const noexcept { return state; }
    bool readSample(int64_t position, float& left, float& right) const;
    uint64_t audioReadCount() const { return store().audioReadCount(); }

private:
    CaptureAssetStore& store() const;
    bool publish(CaptureManifest next);
    bool makeRange(CaptureManifestRange& range, const float* samples, double rate);
    void rebuildOverview();
    juce::File root;
    CaptureSessionSnapshot state;
    mutable std::unique_ptr<CaptureAssetStore> assets;
    mutable std::string assetScope;
};
