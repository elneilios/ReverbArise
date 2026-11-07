#include "CaptureSession.h"
#include "SourceVisitor.h"
#include "PerformanceComparison.h"

#include <algorithm>
#include <cmath>
#include <limits>

CaptureSession::CaptureSession(juce::File projectsRoot,std::shared_ptr<const CaptureStorageLocation> selected)
    :root(std::move(projectsRoot)),location(selected?std::move(selected):std::make_shared<const CaptureStorageLocation>(CaptureStorageLocation{root,root,{}}))
{
    jassert(location->projectsRoot==root);state.location=location;
    if(!location->writable){state.unavailable=true;state.manifestKnown=false;state.status="Asset working folder unavailable; locate, restore or explicitly create a recovery workspace";}
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
                             const juce::String& reference, uint64_t revision,const juce::String& savedManifest)
{
    abortPass();
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
    state = {};state.location=location;state.manifestKnown=false;
    assets.reset(); // Reopen must check assets again even when their metadata is unchanged.
    state.manifest = identity;
    state.manifest.revision = revision;
    state.reference = reference;
    if (reference.isEmpty() && revision == 0)
    {state.manifestKnown=true;state.unavailable=!location->writable;return !state.unavailable;}
    CaptureManifest loaded;
    const bool fromDisk=savedManifest.isEmpty() && location->writable && store().loadSnapshot(reference,loaded,error);
    // Associated host metadata is authoritative. A changed local manifest must
    // not silently substitute another map with the same IDs/revision.
    const bool fromState=reference.startsWith("manifests/") && reference.endsWith(".json") && isSafeCaptureAssetPath(reference.toStdString())
        && savedManifest.getNumBytesAsUTF8()<=8*1024*1024 && savedManifest.isNotEmpty() && parseCaptureManifest(savedManifest,loaded,error);
    if ((!fromDisk && !fromState)
        || loaded.projectId != project || loaded.instanceId != instance || loaded.revision != revision)
    {
        state.status = "Unavailable: saved manifest missing, invalid, or mismatched";
        state.unavailable = true;
        return false;
    }
    state.manifest = std::move(loaded);state.manifestKnown=true;
    if(!location->writable){state.unavailable=true;state.status="Asset working folder unavailable; source metadata retained";return false;}
    rebuildOverview();
    return ! state.unavailable;
}

bool CaptureSession::relocateVerified(std::shared_ptr<const CaptureStorageLocation> next)
{
    if(staged || state.unavailable || !next || !next->writable)return false;
    root=next->projectsRoot;location=std::move(next);state.location=location;assets.reset();assetScope.clear();return true;
}
bool CaptureSession::reload(const std::string& project,const std::string& instance,const juce::String& reference,uint64_t revision)
{
    CaptureSession candidate(root,location);
    if(!candidate.restore(project,instance,reference,revision))
    {state.status="Recovery reload failed; previous accepted source retained: "+candidate.snapshot().status;return false;}
    state=std::move(candidate.state);assets=std::move(candidate.assets);assetScope=std::move(candidate.assetScope);return true;
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
    const auto began=juce::Time::getMillisecondCounterHiRes();
    const auto written=store().writeAudioChunk(range, rate, samples, error);
    maxWriteMilliseconds=std::max(maxWriteMilliseconds,juce::Time::getMillisecondCounterHiRes()-began);
    if (written)
        return true;
    state.status = "Uncommitted: " + error;
    return false;
}

bool CaptureSession::commit(int64_t start, double rate, const float* samples, uint32_t frames,double tolerance,const CaptureGenerationContext* context)
{
    lastCommitAudioChanged=false;
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
    bool identical=false;
    if(!state.manifest.ranges.empty())
    {
        juce::String compareError;
        PerformanceComparison comparison(rate,std::clamp(tolerance,0.0,.3));
        identical=m3::visitSource(store(),state.manifest,start,start+frames,false,
            [&](int64_t at,const float* existing,int count,bool)
            {const auto* incoming=samples+static_cast<size_t>(at-start)*2;
                return tolerance>0?comparison.push(existing,incoming,count):std::equal(existing,existing+count*2,incoming);},compareError)
            && (tolerance<=0 || comparison.finish());
    }
    auto next = state.manifest;
    juce::String contextError;
    if(context && next.schemaVersion==1)
    {
        CaptureManifest migrated;
        if(!captureWithLegacyContext(next,*context,migrated,contextError)){state.status="Uncommitted: "+contextError;return false;}
        next=std::move(migrated);
    }
    if(!context && next.schemaVersion==2 && !identical)
    {state.status="Uncommitted: source changes require generation context";return false;}
    next.sampleRate = rate;
    ++next.revision;
    if(!identical)
    {
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
    }
    if(context)
    {
        CaptureManifest annotated;
        if(!captureWithContext(next,*context,start,start+frames,annotated,contextError))
        {state.status="Uncommitted: "+contextError;return false;}
        next=std::move(annotated);
    }
    const auto metadataChanged=next.schemaVersion!=state.manifest.schemaVersion || next.generationContexts!=state.manifest.generationContexts || next.contextSpans!=state.manifest.contextSpans;
    if(identical && !metadataChanged)return true;
    const auto accepted=publish(std::move(next));if(accepted)lastCommitAudioChanged=!identical;return accepted;
}

bool CaptureSession::publish(CaptureManifest next)
{
    juce::String reference,error;
    if(!isValidCaptureManifest(next,error)){state.status="Uncommitted: "+error;return false;}
    // Private candidate maps reuse exact chunk/slicing logic, but never expose
    // a source revision or write intermediate manifests during a capture pass.
    if(provisional){state.manifest=std::move(next);state.manifestKnown=true;rebuildOverview();return !state.unavailable;}
    if (! store().publishSnapshot(next, reference, error))
    {
        state.status = "Uncommitted: " + error + "; previous committed map retained";
        return false;
    }
    state.manifest = std::move(next);state.manifestKnown=true;
    state.reference = reference;
    rebuildOverview();
    return true;
}

bool CaptureSession::clear(double rate)
{
    if(!location->writable)return false;
    abortPass();
    auto next = state.manifest;
    next.ranges.clear();
    next.contextSpans.clear();next.generationContexts.clear();
    next.sampleRate = std::isfinite(rate) && rate > 0 ? rate : next.sampleRate;
    if (next.revision >= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) return false;
    ++next.revision;
    return publish(std::move(next));
}

bool CaptureSession::beginPass(uint64_t token,uint64_t epoch,bool replace,const CaptureGenerationContext* context)
{
    if(!token || staged || state.unavailable)return false;
    staged=std::make_unique<CaptureSession>(root,location);staged->state=state;staged->provisional=true;
    passToken=token;passEpoch=epoch;baseRevision=state.manifest.revision;
    replaceMapped=replace;passUseful=false;passReplaced=false;passAnyChanged=false;passError.clear();passChanges.clear();passContextChanges.clear();
    passReason=CaptureFailureReason::none;
    passContext=context?std::optional<CaptureGenerationContext>(*context):std::nullopt;return true;
}

bool CaptureSession::beginRecoveryPass(uint64_t token,uint64_t epoch,const std::vector<std::pair<int64_t,int64_t>>& required,const CaptureGenerationContext* context)
{
    if(!location->writable || !token || staged || required.empty() || required.size()>65536 || state.manifest.revision>=static_cast<uint64_t>(INT64_MAX))return false;
    auto coverage=required;std::sort(coverage.begin(),coverage.end());
    for(const auto [a,b]:coverage)if(b<=a)return false;
    staged=std::make_unique<CaptureSession>(root,location);staged->state.manifest=state.manifest;
    staged->state.manifest.ranges.clear();staged->state.manifest.generationContexts.clear();staged->state.manifest.contextSpans.clear();
    staged->provisional=true;passToken=token;passEpoch=epoch;baseRevision=state.manifest.revision;
    replaceMapped=false;passUseful=false;passReplaced=false;passAnyChanged=false;passError.clear();passChanges.clear();passContextChanges.clear();
    passReason=CaptureFailureReason::none;recoveryRequired=std::move(coverage);
    passContext=context?std::optional<CaptureGenerationContext>(*context):std::nullopt;return true;
}
bool CaptureSession::stagePass(uint64_t token,int64_t start,double rate,const float* samples,uint32_t frames)
{
    if(!staged || token!=passToken || passError.isNotEmpty())return false;
    if(!samples || !frames || frames>1920000 || !std::isfinite(rate) || rate<=0
        || frames>rate*5 || start>INT64_MAX-frames
        || !std::all_of(samples,samples+static_cast<size_t>(frames)*2,[](float x){return std::isfinite(x);}))
    {passReason=CaptureFailureReason::invalidData;passError="Invalid or non-finite capture data";return false;}
    if(!state.manifest.ranges.empty() && std::abs(state.manifest.sampleRate-rate)>.001)
    {passReason=CaptureFailureReason::sampleRate;passError="Capture sample rate is incompatible";return false;}
    const auto end=start+frames;
    std::vector<std::pair<int64_t,int64_t>> intervals;
    if(replaceMapped)intervals.emplace_back(start,end);
    else
    {
        auto cursor=start;
        for(const auto& old:staged->state.manifest.ranges)
        {
            const auto oldEnd=old.startSample+static_cast<int64_t>(old.frameCount);
            if(oldEnd<=cursor)continue;if(old.startSample>=end)break;
            if(old.startSample>cursor)intervals.emplace_back(cursor,std::min(end,old.startSample));
            cursor=std::max(cursor,std::min(end,oldEnd));if(cursor>=end)break;
        }
        if(cursor<end)intervals.emplace_back(cursor,end);
    }
    for(const auto [a,b]:intervals)
    {
        const auto* input=samples+static_cast<size_t>(a-start)*2;
        const auto count=static_cast<uint32_t>(b-a);
        passUseful=passUseful || std::any_of(input,input+static_cast<size_t>(count)*2,[](float x){return x!=0;});
        if(replaceMapped)for(const auto& old:state.manifest.ranges)
            if(a<old.startSample+static_cast<int64_t>(old.frameCount) && b>old.startSample)passReplaced=true;
        const auto revision=staged->state.manifest.revision;
        const auto previousContexts=staged->state.manifest.contextSpans;
        if(!staged->commit(a,rate,input,count,0,passContext?&*passContext:nullptr))
        {passReason=CaptureFailureReason::assetWrite;passError=staged->state.status;return false;}
        if(staged->state.manifest.revision!=revision)passAnyChanged=true;
        if(staged->lastCommitAudioChanged)passChanges.emplace_back(a,b);
        if(staged->state.manifest.contextSpans!=previousContexts)passContextChanges.emplace_back(a,b);
    }
    return true;
}

CaptureSession::PassResult CaptureSession::preparePass(uint64_t token,uint64_t epoch)
{
    PassResult result;
    if(!staged || token!=passToken || epoch!=passEpoch || state.manifest.revision!=baseRevision)
    {result.error="Obsolete capture pass";return result;}
    result.error=passError;
    if(result.error.isEmpty() && !recoveryRequired.empty())
        for(const auto [start,end]:recoveryRequired){auto covered=start;
            for(const auto& range:staged->state.manifest.ranges){const auto tail=range.startSample+static_cast<int64_t>(range.frameCount);
                if(tail<=covered)continue;if(range.startSample>covered)break;covered=tail;if(covered>=end)break;}
            if(covered<end){result.error="Re-learning is incomplete; previous source and arrangement retained";passReason=CaptureFailureReason::rejected;break;}}
    if(result.error.isEmpty())
    {
        result.changed=passAnyChanged;
        if(result.changed)
        {
            auto candidate=staged->state.manifest;candidate.revision=baseRevision+1;
            for(auto& range:candidate.ranges)if(range.revision>baseRevision)range.revision=candidate.revision;
            juce::String reference;
            if(!store().publishSnapshot(candidate,reference,result.error,&passReason))result.changed=false;
            else {prepared=std::make_unique<CaptureSessionSnapshot>(staged->state);prepared->manifest=std::move(candidate);prepared->reference=reference;prepared->status="Ready (committed)";}
        }
        if(result.error.isEmpty())
        {result.success=true;result.useful=passUseful;result.replaced=passReplaced;result.changedIntervals=passChanges;result.contextChangedIntervals=passContextChanges;}
    }
    return result;
}

bool CaptureSession::acceptPreparedPass(uint64_t token,uint64_t epoch)
{
    if(!staged || token!=passToken || epoch!=passEpoch || state.manifest.revision!=baseRevision || passError.isNotEmpty())return false;
    if(prepared)state=std::move(*prepared);
    abortPass();return true;
}

CaptureSession::PassResult CaptureSession::finishPass(uint64_t token,uint64_t epoch)
{
    auto result=preparePass(token,epoch);
    if(result.success){result.success=acceptPreparedPass(token,epoch);return result;}
    if(staged && token==passToken && epoch==passEpoch)
    {state.status="Capture pass rejected: "+result.error+"; previous committed source retained";abortPass();}
    return result;
}

void CaptureSession::abortPass()
{staged.reset();prepared.reset();passContext.reset();recoveryRequired.clear();passToken=0;passEpoch=0;passError.clear();passChanges.clear();passContextChanges.clear();passAnyChanged=false;passUseful=false;passReplaced=false;}

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
