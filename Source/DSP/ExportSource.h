#pragma once
#include "SongExport.h"
#include <span>
#include <functional>
namespace m3
{
inline constexpr int exportPageFrames=4096;
// Worker-only adapter. The normal Perform source reader keeps its strict policy.
class ExportSourceReader
{
public:
    explicit ExportSourceReader(juce::File instanceDirectory):store(std::move(instanceDirectory)){}
    bool prepare(std::shared_ptr<const SongExportPlan>,uint64_t availableBytes,juce::String& error,
                 const std::function<bool()>& cancelled={});
    bool read(int64_t start,std::span<float> stereo,juce::String& error);
    const std::vector<ExportWarning>& warnings()const noexcept{return missing;}
    uint64_t audioReads()const noexcept{return store.audioReadCount();}
    uint64_t workingBytes()const noexcept{return cache.interleavedSamples.capacity()*sizeof(float)+missing.capacity()*sizeof(ExportWarning)+(readable.capacity()+7)/8;}
private:
    bool load(size_t index,juce::String& error);
    bool readCoverage(int64_t start,std::span<float> stereo,juce::String& error);
    void warn(int64_t,int64_t,ExportCoverage);
    CaptureAssetStore store;
    std::shared_ptr<const SongExportPlan> plan;
    std::vector<bool> readable;
    std::vector<ExportWarning> missing;
    CaptureAudioChunk cache;
    size_t cached=SIZE_MAX;
    uint64_t allowance=0;
};
}
