#include "SourceRangeReader.h"
#include <algorithm>
#include <cmath>
#include <sstream>

std::string sourceDependency(const CaptureManifest& manifest, int64_t start, int64_t end)
{
    std::ostringstream result;
    result.precision(17);
    result << manifest.projectId << '/' << manifest.instanceId << '@' << manifest.sampleRate;
    if (manifest.loop)
        result << "/loop@" << manifest.revision;
    int64_t runStart = start, runEnd = start;
    uint64_t revision = 0;
    bool haveRun = false;
    const auto emit = [&] { if (haveRun) result << ':' << runStart << ',' << runEnd << ',' << revision; };
    for (const auto& range : manifest.ranges)
    {
        const auto begin = std::max(start, range.startSample);
        const auto finish = std::min(end, range.startSample + static_cast<int64_t>(range.frameCount));
        if (finish <= begin) continue;
        // Splitting an old audio chunk may reveal an all-zero remainder and store it
        // as silence. Its unchanged capture revision is authoritative for validity.
        if (haveRun && begin == runEnd && revision == range.revision)
            runEnd = finish;
        else
        {
            emit(); runStart = begin; runEnd = finish; revision = range.revision; haveRun = true;
        }
    }
    emit(); return result.str();
}

bool readSourceInterval(CaptureAssetStore& store, const CaptureManifest& manifest, int64_t start, int64_t end,
                        uint64_t maximumFrames, SourceInterval& destination, juce::String& error,
                        const std::function<bool()>& cancelled)
{
    const auto unavailable = [&](int64_t a, int64_t b, const juce::String& reason)
    { error = "Source [" + juce::String(a) + ", " + juce::String(b) + "): " + reason; return false; };
    if (! isValidCaptureManifest(manifest, error)) return false;
    const auto frames = static_cast<uint64_t>(end) - static_cast<uint64_t>(start);
    if (end <= start || frames > maximumFrames || frames > std::numeric_limits<size_t>::max() / (2 * sizeof(float)))
        return unavailable(start, end, "invalid interval or memory budget exceeded");
    auto cursor = start;
    for (const auto& range : manifest.ranges)
    {
        const auto finish = range.startSample + static_cast<int64_t>(range.frameCount);
        if (finish <= cursor || range.startSample >= end) continue;
        if (range.startSample > cursor) return unavailable(cursor, std::min(end, range.startSample), "uncaptured gap");
        cursor = std::min(end, finish);
    }
    if (cursor != end) return unavailable(cursor, end, "uncaptured gap");
    SourceInterval result;
    result.start = start; result.end = end; result.rate = manifest.sampleRate;
    result.dependency = sourceDependency(manifest, start, end);
    result.samples.resize(static_cast<size_t>(frames) * 2, 0);
    for (const auto& range : manifest.ranges)
    {
        if (cancelled && cancelled()) return unavailable(start, end, "cancelled");
        const auto begin = std::max(start, range.startSample);
        const auto finish = std::min(end, range.startSample + static_cast<int64_t>(range.frameCount));
        if (finish <= begin || range.kind == CaptureCoverageKind::silence) continue;
        CaptureAudioChunk chunk;
        juce::String failure;
        if (! store.readAudioChunk(range, manifest.sampleRate, chunk, failure)) return unavailable(begin, finish, failure);
        const auto offset = static_cast<size_t>(begin - range.startSample) * 2;
        const auto count = static_cast<size_t>(finish - begin) * 2;
        const auto source = chunk.interleavedSamples.begin() + offset;
        if (! std::all_of(source, source + count, [](float v) { return std::isfinite(v); }))
            return unavailable(begin, finish, "non-finite samples");
        std::copy_n(source, count, result.samples.begin() + static_cast<size_t>(begin - start) * 2);
    }
    destination = std::move(result); error.clear(); return true;
}
