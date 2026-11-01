#include "CaptureSession.h"
#include "SourceVisitor.h"
#include "PerformanceComparison.h"

#include <algorithm>
#include <cmath>
#include <limits>

CaptureSession::CaptureSession(juce::File projectsRoot) : root(std::move(projectsRoot))
{
    state.manifest.projectId = juce::Uuid().toDashedString().toStdString();
    state.manifest.instanceId = juce::Uuid().toDashedString().toStdString();
}

CaptureAssetStore& CaptureSession::store() const
{
    const auto scope = state.manifest.projectId + "/" + state.manifest.instanceId;
    if (! assets || assetScope != scope)
    {
        assets = std::make_unique<CaptureAssetStore>(root.getChildFile(scope));
        assetScope = scope;
    }
    return *assets;
}

bool CaptureSession::restore(const std::string& project, const std::string& instance,
                             const juce::String& reference, uint64_t revision)
{
    CaptureManifest identity;
    identity.projectId = project;
    identity.instanceId = instance;
    juce::String error;
    if (! isValidCaptureManifest(identity, error))
    {
        state.status = "Unavailable: invalid cache identifiers";
        state.unavailable = true;
        return false;
    }
    state = {};
    assets.reset(); // Reopen must check assets again even when their metadata is unchanged.
    state.manifest = identity;
    state.manifest.revision = revision;
    state.reference = reference;
    if (reference.isEmpty() && revision == 0)
        return true;
    CaptureManifest loaded;
    if (! store().loadSnapshot(reference, loaded, error)
        || loaded.projectId != project || loaded.instanceId != instance || loaded.revision != revision)
    {
        state.status = "Unavailable: saved manifest missing, invalid, or mismatched";
        state.unavailable = true;
        return false;
    }
    state.manifest = std::move(loaded);
    rebuildOverview();
    return ! state.unavailable;
}

bool CaptureSession::makeRange(CaptureManifestRange& range, const float* samples, double rate)
{
    range.kind = std::all_of(samples, samples + range.frameCount * 2,
                            [](float value) { return value == 0.0f; })
        ? CaptureCoverageKind::silence : CaptureCoverageKind::audio;
    if (range.kind == CaptureCoverageKind::silence)
        return true;
    range.assetPath = "segments/" + juce::Uuid().toDashedString().toStdString() + ".wav";
    juce::String error;
    if (store().writeAudioChunk(range, rate, samples, error))
        return true;
    state.status = "Uncommitted: " + error;
    return false;
}

bool CaptureSession::commit(int64_t start, double rate, const float* samples, uint32_t frames,double tolerance)
{
    if (state.unavailable) return false;
    if (frames == 0 || samples == nullptr || ! std::isfinite(rate) || rate <= 0.0
        || frames > 1920000 || static_cast<double>(frames) > rate * 5.0
        || start > std::numeric_limits<int64_t>::max() - frames
        || state.manifest.revision >= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
    {
        state.status = "Uncommitted: invalid capture block";
        return false;
    }
    if (! state.manifest.ranges.empty() && std::abs(state.manifest.sampleRate - rate) > 0.001)
    {
        state.status = "Stale: sample rate changed; clear and recapture";
        return false;
    }
    // Loop capture often replays the same samples with different block/chunk
    // boundaries. Preserve the committed identity when the covered PCM agrees.
    // Comparison reads only bounded existing chunks and never runs in the callback.
    if(!state.manifest.ranges.empty())
    {
        juce::String compareError;
        PerformanceComparison comparison(rate,std::clamp(tolerance,0.0,.3));
        if(m3::visitSource(store(),state.manifest,start,start+frames,false,
            [&](int64_t at,const float* existing,int count,bool)
            {const auto* incoming=samples+static_cast<size_t>(at-start)*2;
                return tolerance>0?comparison.push(existing,incoming,count):std::equal(existing,existing+count*2,incoming);},compareError)
            && (tolerance<=0 || comparison.finish()))
            return true;
    }
    auto next = state.manifest;
    next.sampleRate = rate;
    ++next.revision;
    next.ranges.clear();
    const auto end = start + frames;
    for (const auto& old : state.manifest.ranges)
    {
        const auto oldEnd = old.startSample + static_cast<int64_t>(old.frameCount);
        if (oldEnd <= start || old.startSample >= end)
        {
            next.ranges.push_back(old);
            continue;
        }
        CaptureAudioChunk chunk;
        juce::String error;
        if (old.kind == CaptureCoverageKind::audio
            && ! store().readAudioChunk(old, rate, chunk, error))
        {
            state.status = "Uncommitted: " + error;
            return false;
        }
        for (const auto interval : {std::pair{old.startSample, std::min(start, oldEnd)},
                                    std::pair{std::max(end, old.startSample), oldEnd}})
        {
            if (interval.first >= interval.second) continue;
            auto remainder = old;
            remainder.startSample = interval.first;
            remainder.frameCount = static_cast<uint64_t>(interval.second - interval.first);
            if (old.kind == CaptureCoverageKind::audio)
            {
                remainder.assetPath.clear();
                remainder.checksum.clear();
                const auto offset = static_cast<size_t>(interval.first - old.startSample) * 2;
                if (! makeRange(remainder, chunk.interleavedSamples.data() + offset, rate)) return false;
            }
            next.ranges.push_back(std::move(remainder));
        }
    }
    CaptureManifestRange incoming{start, frames, CaptureCoverageKind::audio, next.revision, {}, {}};
    if (! makeRange(incoming, samples, rate)) return false;
    next.ranges.push_back(std::move(incoming));
    std::sort(next.ranges.begin(), next.ranges.end(),
              [](const auto& a, const auto& b) { return a.startSample < b.startSample; });
    return publish(std::move(next));
}

bool CaptureSession::publish(CaptureManifest next)
{
    juce::String reference, error;
    if (! store().publishSnapshot(next, reference, error))
    {
        state.status = "Uncommitted: " + error + "; previous committed map retained";
        return false;
    }
    state.manifest = std::move(next);
    state.reference = reference;
    rebuildOverview();
    return true;
}

bool CaptureSession::clear(double rate)
{
    auto next = state.manifest;
    next.ranges.clear();
    next.sampleRate = std::isfinite(rate) && rate > 0 ? rate : next.sampleRate;
    if (next.revision >= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) return false;
    ++next.revision;
    return publish(std::move(next));
}

void CaptureSession::rebuildOverview()
{
    auto previous = std::move(state.overview);
    state.overview.clear();
    state.unavailable = false;
    for (const auto& range : state.manifest.ranges)
    {
        CaptureOverviewRange view;
        view.range = range;
        if (range.kind == CaptureCoverageKind::audio)
        {
            const auto cached = std::find_if(previous.begin(), previous.end(), [&](const auto& old)
            {
                return old.available && old.range.assetPath == range.assetPath
                    && old.range.checksum == range.checksum && old.range.startSample == range.startSample
                    && old.range.frameCount == range.frameCount;
            });
            if (cached != previous.end())
            {
                view.peaks = std::move(cached->peaks);
                state.overview.push_back(std::move(view));
                continue;
            }
            CaptureAudioChunk chunk;
            juce::String error;
            // Reject unbounded assets before allocating PCM.
            view.available = static_cast<double>(range.frameCount) <= state.manifest.sampleRate * 5.0
                && store().readAudioChunk(range, state.manifest.sampleRate, chunk, error);
            if (view.available)
            {
                for (uint32_t offset = 0; offset < chunk.frameCount; offset += 512)
                {
                    const auto count = std::min(uint32_t{512}, chunk.frameCount - offset);
                    const auto begin = chunk.interleavedSamples.begin() + static_cast<size_t>(offset) * 2;
                    const auto bounds = std::minmax_element(begin, begin + count * 2);
                    view.peaks.push_back({range.startSample + offset, count, *bounds.first, *bounds.second});
                }
            }
            else state.unavailable = true;
        }
        state.overview.push_back(std::move(view));
    }
    state.status = state.unavailable ? "Unavailable/stale assets: valid ranges retained; mapping paused"
                                    : "Ready (committed)";
}

bool CaptureSession::readSample(int64_t position, float& left, float& right) const
{
    for (const auto& view : state.overview)
    {
        const auto& range = view.range;
        if (! view.available || position < range.startSample
            || position >= range.startSample + static_cast<int64_t>(range.frameCount)) continue;
        if (range.kind == CaptureCoverageKind::silence) { left = right = 0; return true; }
        CaptureAudioChunk chunk;
        juce::String error;
        if (! store().readAudioChunk(range, state.manifest.sampleRate, chunk, error)) return false;
        const auto offset = static_cast<size_t>(position - range.startSample) * 2;
        left = chunk.interleavedSamples[offset]; right = chunk.interleavedSamples[offset + 1];
        return true;
    }
    return false;
}
