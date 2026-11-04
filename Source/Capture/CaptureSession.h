#pragma once

#include "CaptureAssetStore.h"
#include <optional>

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
struct ProvisionalCaptureSnapshot
{
    CaptureManifest manifest;
    uint64_t pass=0,epoch=0,sequence=0;
};

// Worker-thread only. PCM is read one bounded chunk at a time, never retained for the full song.
class CaptureSession
{
public:
    explicit CaptureSession(juce::File projectsRoot);
    bool restore(const std::string& projectId, const std::string& instanceId,
                 const juce::String& reference, uint64_t revision);
    bool commit(int64_t start, double rate, const float* samples, uint32_t frames,double tolerance=0,const CaptureGenerationContext* context=nullptr);
    bool clear(double rate);
    struct PassResult
    {
        bool success=false,useful=false,changed=false,replaced=false;
        juce::String error;
        std::vector<std::pair<int64_t,int64_t>> changedIntervals;
        std::vector<std::pair<int64_t,int64_t>> contextChangedIntervals;
    };
    bool beginPass(uint64_t token,uint64_t epoch,bool replace,const CaptureGenerationContext* context=nullptr);
    bool stagePass(uint64_t token,int64_t start,double rate,const float* samples,uint32_t frames);
    PassResult finishPass(uint64_t token,uint64_t epoch);
    PassResult preparePass(uint64_t token,uint64_t epoch);
    bool acceptPreparedPass(uint64_t token,uint64_t epoch);
    void abortPass();
    bool passActive() const noexcept {return staged!=nullptr;}
    const juce::String& passFailure() const noexcept {return passError;}
    std::optional<CaptureManifest> stagedManifest() const
    {return staged && passError.isEmpty()?std::optional<CaptureManifest>{staged->state.manifest}:std::nullopt;}
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
    std::unique_ptr<CaptureSession> staged;
    uint64_t passToken=0,passEpoch=0,baseRevision=0;
    bool provisional=false,replaceMapped=false,passUseful=false,passReplaced=false;
    bool lastCommitAudioChanged=false,passAnyChanged=false;
    std::optional<CaptureGenerationContext> passContext;
    juce::String passError;
    std::vector<std::pair<int64_t,int64_t>> passChanges;
    std::vector<std::pair<int64_t,int64_t>> passContextChanges;
    std::unique_ptr<CaptureSessionSnapshot> prepared;
};
