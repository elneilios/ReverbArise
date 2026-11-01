#include "PreviewEngine.h"
#include <algorithm>

PreviewEngine::PreviewEngine(juce::File projectsRoot, std::function<PreviewContext()> provider)
    : Thread("Preverb Render"), root(std::move(projectsRoot)), contextProvider(std::move(provider))
{ voice.setLevel(juce::Decibels::decibelsToGain(-12.0f)); startThread(); }
PreviewEngine::~PreviewEngine() { signalThreadShouldExit(); requestId.fetch_add(1); stopThread(-1); }

bool PreviewEngine::cancelled(uint64_t request) const noexcept
{ return threadShouldExit() || requestId.load() != request; }

uint64_t PreviewEngine::occupiedBytesLocked() const
{
    return (current ? current->samples.capacity() * sizeof(float) : 0)
        + (ir ? ir->samples.capacity() * sizeof(float) : 0) + voice.pinnedBytes();
}

void PreviewEngine::invalidateLocked(const juce::String& reason)
{
    requestId.fetch_add(1);
    pendingGenerate = false;
    dependencyContext.reset();
    audioAllowed.store(false);
    voice.stop(true);
    state.phase = PreviewPhase::stale; state.status = reason;
}

void PreviewEngine::loadIR(const juce::File& file)
{
    if (file == juce::File{}) return;
    const juce::ScopedLock guard(lock);
    invalidateLocked("Loading IR"); pendingIR = file;
    state.phase = PreviewPhase::loadingIR;
}
void PreviewEngine::setSourceBars(int64_t start, int64_t end)
{
    const juce::ScopedLock guard(lock);
    if (start == state.startBar && end == state.endBar) return;
    state.startBar = start; state.endBar = end;
    if (state.phase == PreviewPhase::loadingIR) return;
    invalidateLocked("Source selection changed; Generate to refresh preview");
    // A pending file selection still loads; its job receives the new request ID.
}
void PreviewEngine::generate()
{
    const juce::ScopedLock guard(lock);
    if (state.phase == PreviewPhase::loadingIR) return;
    invalidateLocked("Queued for generation");
    if (! ir || pendingIR) { state.phase = PreviewPhase::error; state.status = "Load a valid WAV/WavPack IR first"; return; }
    pendingGenerate = true; state.phase = PreviewPhase::rendering; state.progress = 0;
}
bool PreviewEngine::audition()
{
    const juce::ScopedLock guard(lock);
    if (state.phase != PreviewPhase::ready || ! current || ! audioAllowed.load()) return false;
    const auto attenuation = current->peak > 0 ? std::min(1.0f, juce::Decibels::decibelsToGain(-6.0f) / current->peak) : 1.0f;
    return voice.start(current, attenuation);
}
void PreviewEngine::setAuditionLevelDb(float db)
{
    const juce::ScopedLock guard(lock);
    state.auditionLevelDb = std::isfinite(db) ? std::clamp(db, -60.0f, 0.0f) : -12.0f;
    voice.setLevel(juce::Decibels::decibelsToGain(state.auditionLevelDb));
}
void PreviewEngine::stop() { const juce::ScopedLock guard(lock); voice.stop(); }
void PreviewEngine::invalidate(const juce::String& reason)
{
    const juce::ScopedLock guard(lock);
    invalidateLocked(reason); dependencyContext.reset();
}
void PreviewEngine::reset()
{
    const juce::ScopedLock guard(lock);
    invalidateLocked("Preview cleared on project restore");
    pendingIR.reset(); ir.reset(); current.reset(); dependencyContext.reset();
    state = {};
    voice.setLevel(juce::Decibels::decibelsToGain(state.auditionLevelDb));
}
PreviewSnapshot PreviewEngine::snapshot() const
{
    const juce::ScopedLock guard(lock);
    auto result = state; result.playing = voice.isPlaying();
    if (current && current->peak > 0)
        result.auditionAttenuationDb = std::min(0.0f, -6.0f - juce::Decibels::gainToDecibels(current->peak));
    return result;
}

void PreviewEngine::processAudio(float* left, float* right, int frames, const SongTime& time) noexcept
{
    auto allowed = audioAllowed.load(std::memory_order_acquire);
    const auto rateChanged = expectedRate.load() > 0 && time.rate != expectedRate.load();
    const auto mappingChanged = time.valid && (time.bpm != expectedBpm.load()
        || time.numerator != expectedNumerator.load() || time.denominator != expectedDenominator.load());
    if (allowed && (rateChanged || mappingChanged)) { audioMappingChanged.store(true); allowed = false; }
    voice.process(left, right, frames, time.rate, allowed);
}

void PreviewEngine::report(uint64_t request, PreviewPhase phase, const juce::String& message, float progress)
{
    const juce::ScopedLock guard(lock);
    if (cancelled(request)) return;
    state.phase = phase; state.status = message; state.progress = progress;
}

void PreviewEngine::inspectContext(const PreviewContext& context)
{
    const juce::ScopedLock guard(lock);
    voice.collect();
    if (! dependencyContext) return;
    const auto timingChanged = context.time.rate != dependencyContext->time.rate
        || (context.time.valid && ! sameMusicalMapping(context.time, dependencyContext->time));
    if (audioMappingChanged.exchange(false) || timingChanged
        || context.captureGeneration != dependencyContext->captureGeneration
        || sourceDependency(context.capture.manifest, sourceStart, sourceEnd) != dependency)
    {
        invalidateLocked("Stale: source or host timing changed; Generate again");
        dependencyContext.reset();
    }
}

void PreviewEngine::loadJob(const juce::File& file, uint64_t request)
{
    uint64_t occupied;
    { const juce::ScopedLock guard(lock); occupied = occupiedBytesLocked(); }
    ImpulseResponse decoded;
    juce::String error;
    if (! loadImpulseResponse(file, occupied < previewMemoryBudget ? previewMemoryBudget - occupied : 0, decoded, error))
    { report(request, PreviewPhase::error, error); return; }
    const juce::ScopedLock guard(lock);
    if (cancelled(request)) return;
    ir = std::make_shared<const ImpulseResponse>(std::move(decoded));
    state.irName = ir->name; state.irRate = ir->rate; state.irChannels = ir->channels;
    state.phase = PreviewPhase::canGenerate; state.status = "IR loaded; choose a committed source range and Generate";
    dependencyContext.reset();
}

void PreviewEngine::renderJob(uint64_t request)
{
    const auto context = contextProvider();
    std::shared_ptr<const ImpulseResponse> inputIR;
    int64_t firstBar, lastBar;
    uint64_t occupied;
    { const juce::ScopedLock guard(lock);
      inputIR = ir; firstBar = state.startBar; lastBar = state.endBar; occupied = occupiedBytesLocked(); }
    int64_t first = 0, end = 0;
    std::string failure;
    if (! inputIR || ! resolveSourceBars(context.time, context.capture.manifest.sampleRate,
                                         firstBar, lastBar, first, end, failure))
    { report(request, PreviewPhase::error, failure.empty() ? "No loaded IR" : juce::String(failure)); return; }
    const auto frameCount = static_cast<uint64_t>(end) - static_cast<uint64_t>(first);
    const auto preparedFrames = std::max(1.0, std::round(inputIR->frames() * context.time.rate / inputIR->rate));
    uint64_t required;
    juce::String error;
    if (! std::isfinite(preparedFrames) || preparedFrames > previewMemoryBudget / 8
        || ! estimateRenderMemory(frameCount, static_cast<uint64_t>(preparedFrames), occupied, required, error))
    { report(request, PreviewPhase::error, error.isEmpty() ? "IR exceeds memory budget" : error); return; }
    {
        const juce::ScopedLock guard(lock);
        if (cancelled(request)) return;
        dependencyContext = context; sourceStart = first; sourceEnd = end;
        dependency = sourceDependency(context.capture.manifest, first, end);
    }
    const auto cancel = [&] { return cancelled(request); };
    CaptureAssetStore store(root.getChildFile(context.capture.manifest.projectId).getChildFile(context.capture.manifest.instanceId));
    SourceInterval source;
    report(request, PreviewPhase::rendering, "Reading committed source", 0.05f);
    if (! readSourceInterval(store, context.capture.manifest, first, end, previewMemoryBudget / 8, source, error, cancel))
    { report(request, PreviewPhase::error, error); return; }
    ImpulseResponse prepared;
    report(request, PreviewPhase::rendering, "Preparing IR", 0.25f);
    if (! prepareImpulseResponse(*inputIR, source.rate, previewMemoryBudget - occupied - source.samples.size() * sizeof(float), prepared, error, cancel))
    { report(request, PreviewPhase::error, error); return; }
    auto rendered = std::make_shared<WetRender>();
    report(request, PreviewPhase::rendering, "Convolving", 0.4f);
    if (! renderReverseConvolution(source.samples, first, source.rate, prepared, occupied, *rendered, error,
        cancel, [&](float v) { report(request, PreviewPhase::rendering, "Convolving", 0.4f + v * 0.45f); }))
    { report(request, PreviewPhase::error, error); return; }
    report(request, PreviewPhase::rendering, "Building wet preview", 0.9f);
    auto waveform = std::make_shared<PreviewWaveform>();
    waveform->start = rendered->start; waveform->end = rendered->sourceEnd;
    waveform->sourceStart = first; waveform->sourceEnd = end; waveform->frames = rendered->frames();
    waveform->rate = rendered->rate; waveform->peak = rendered->peak;
    for (size_t frame = 0; frame < rendered->frames(); frame += 512)
    {
        if (cancel()) return;
        const auto count = std::min(size_t{512}, rendered->frames() - frame);
        const auto begin = rendered->samples.begin() + frame * 2;
        const auto bounds = std::minmax_element(begin, begin + count * 2);
        waveform->peaks.push_back({rendered->start + static_cast<int64_t>(frame), static_cast<uint32_t>(count), *bounds.first, *bounds.second});
    }
    inspectContext(contextProvider());
    const juce::ScopedLock guard(lock);
    if (cancel()) return;
    current = std::move(rendered); state.waveform = std::move(waveform);
    state.phase = PreviewPhase::ready; state.status = "Ready: wet-only preview (256 MiB budget)"; state.progress = 1;
    expectedRate.store(context.time.rate); expectedBpm.store(context.time.bpm);
    expectedNumerator.store(context.time.numerator); expectedDenominator.store(context.time.denominator);
    audioMappingChanged.store(false); audioAllowed.store(true, std::memory_order_release);
}

void PreviewEngine::run()
{
    while (! threadShouldExit())
    {
        std::optional<juce::File> file;
        bool generateNow;
        uint64_t request;
        {
            const juce::ScopedLock guard(lock);
            voice.collect(); file = std::move(pendingIR); pendingIR.reset();
            generateNow = std::exchange(pendingGenerate, false); request = requestId.load();
        }
        try
        {
            inspectContext(contextProvider());
            if (file) loadJob(*file, request);
            else if (generateNow) renderJob(request);
        }
        catch (const std::exception& error) { report(request, PreviewPhase::error, "Render failed: " + juce::String(error.what())); }
        wait(20);
    }
}
