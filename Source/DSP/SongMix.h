#pragma once
#include "ExportSource.h"
#include "SwellAssetStore.h"
#include <array>
#include <optional>
namespace m3
{
struct SongMixMetrics
{
    float wetPeak=0,dryPeak=0,peak=0,wetGain=1,sumGain=1,displayGain=1;
    std::optional<int64_t> audibleStart,audibleEnd;
    std::vector<CapturePeak> dryPeaks,wetPeaks;
};
struct SongMixDetail
{
    juce::String composition;int64_t first=0,end=0;size_t bins=0;
    std::vector<CapturePeak> dry,wet;
};
// Worker-only streaming composition. File handles pin verified wet versions;
// at most one decoded source chunk and fixed-size sample pages are retained.
class SongMix
{
public:
    explicit SongMix(juce::File directory):directory(std::move(directory)),dry(this->directory),scratch(2,exportPageFrames){}
    bool prepare(std::shared_ptr<const SongExportPlan>,uint64_t available,juce::String& error,const RenderCancellation& cancel={});
    bool measure(juce::String& error,const RenderCancellation& cancel={});
    bool adoptMeasurements(const SongMixMetrics&,const juce::String& identity,juce::String& error);
    bool summarize(int64_t first,int64_t end,size_t bins,SongMixDetail&,juce::String& error,const RenderCancellation& cancel={});
    bool read(int64_t at,std::span<float> output,juce::String& error);
    // Verified worker-only branches before output levels. A positive plan level
    // admits that branch; prepare at unit levels to admit both, without measuring.
    bool readLayers(int64_t at,std::span<float> source,std::span<float> wet,juce::String& error)
    {if(!layersReady){error="Export layers are not prepared";return false;}return branches(at,source,wet,error);}
    const SongMixMetrics& metrics()const noexcept{return measured;}
    const std::shared_ptr<const SongExportPlan>& plan()const noexcept{return current;}
    uint64_t workingBytes()const noexcept;
private:
    bool branches(int64_t,std::span<float> dry,std::span<float> wet,juce::String& error);
    bool scan(const std::function<bool(int64_t,int)>&,juce::String&,const RenderCancellation&);
    juce::File directory;
    ExportSourceReader dry;
    juce::AudioBuffer<float> scratch;
    std::array<float,exportPageFrames*2> dryPage{},wetPage{};
    std::vector<std::unique_ptr<juce::AudioFormatReader>> readers;
    std::vector<std::pair<int64_t,int64_t>> occupied;
    std::shared_ptr<const SongExportPlan> current;
    SongMixMetrics measured;
    bool ready=false,layersReady=false;
};
}
