#include "LoopPlayback.h"

namespace m3
{
bool LoopPlaybackPublication::prepareLeadInPeaks(const std::function<bool()>& cancelled)
{
    const auto start = std::max(plan.audio.start,-timing.endSample);
    if (start >= 0) return true;
    constexpr int64_t maximumBuckets = 2048;
    const auto stride = std::max(int64_t{1},(-start + maximumBuckets - 1) / maximumBuckets);
    auto peaks = std::make_shared<std::vector<CapturePeak>>();
    peaks->reserve(static_cast<size_t>((-start + stride - 1) / stride));
    for (auto at = start; at < 0; at += stride)
        peaks->push_back({at,static_cast<uint32_t>(std::min(stride,-at)),0,0});
    for (const auto& segment : plan.audio.segments)
    {
        const auto first = std::max(start,segment.start);
        const auto end = std::min(int64_t{0},segment.end);
        for (auto at = first; at < end; ++at)
        {
            if ((at - first) % 4096 == 0 && cancelled()) return false;
            LoopWetSample mix;
            for (size_t member = 0; member < segment.count; ++member)
            {
                const auto& render = *plan.audio.entries[segment.members[member]].audio;
                const auto offset = static_cast<size_t>(at - render.start) * 2;
                mix.left += render.samples[offset];
                mix.right += render.samples[offset + 1];
            }
            auto& peak = (*peaks)[static_cast<size_t>((at - start) / stride)];
            peak.minimum = std::min({peak.minimum,mix.left * gain,mix.right * gain});
            peak.maximum = std::max({peak.maximum,mix.left * gain,mix.right * gain});
        }
    }
    leadInPeaks = std::move(peaks);
    return true;
}

void LoopPlayback::collect()
{
    for (size_t index = 0; index < slots.size(); ++index)
    {
        auto& slot = slots[index];
        // After retirement the callback can no longer publish this slot.
        // Its last displayed word may remain until the end of that block.
        if (slot.state.load(std::memory_order_acquire) == retired
            && (statusWord.load(std::memory_order_acquire) >> displaySlotShift) != index + 1)
        {
            slot.data.reset();
            slot.state.store(free,std::memory_order_release);
        }
    }
}

void LoopPlayback::discardPending()
{
    const auto discarded = pending.exchange(-1,std::memory_order_acq_rel);
    if (discarded >= 0) slots[discarded].state.store(retired,std::memory_order_release);
    collect();
}

uint64_t LoopPlayback::pinnedBytes() const noexcept
{
    uint64_t result = 0;
    for (const auto& slot : slots) if (slot.data) result += slot.data->bytes();
    return result;
}

bool LoopPlayback::publish(std::unique_ptr<LoopPlaybackPublication> publication)
{
    collect();
    for (size_t index = 0; index < slots.size(); ++index)
    {
        auto& slot = slots[index];
        if (slot.state.load(std::memory_order_acquire) != free) continue;
        slot.data = std::move(publication);
        slot.state.store(published,std::memory_order_release);
        const auto old = pending.exchange(static_cast<int>(index),std::memory_order_acq_rel);
        if (old >= 0) slots[old].state.store(retired,std::memory_order_release);
        return true;
    }
    return false;
}

void LoopPlayback::takePublication() noexcept
{
    const auto next = pending.exchange(-1,std::memory_order_acq_rel);
    if (next < 0) return;
    if (latest >= 0 && latest != held) slots[latest].state.store(retired,std::memory_order_release);
    latest = next;
    slots[latest].state.store(retained,std::memory_order_release);
}

void LoopPlayback::releaseHeld() noexcept
{
    if (held >= 0 && held != latest) slots[held].state.store(retired,std::memory_order_release);
    held = -1;
}

void LoopPlayback::retireLatestWithoutSource(uint64_t revision) noexcept
{
    if (revision || latest < 0 || scheduler.state() != LoopCueScheduler::State::standby) return;
    slots[latest].state.store(retired,std::memory_order_release);
    latest = -1;
}

void LoopPlayback::setReference(const LoopReferenceSetup& setup, bool externalCommands) noexcept
{
    configSequence.fetch_add(1,std::memory_order_acq_rel);
    referenceRate.store(setup.clock.rate,std::memory_order_relaxed);
    referenceTempo.store(setup.clock.tempo,std::memory_order_relaxed);
    referenceBeats.store(setup.beats,std::memory_order_relaxed);
    referenceNumerator.store(setup.numerator,std::memory_order_relaxed);
    referenceDenominator.store(setup.denominator,std::memory_order_relaxed);
    phraseInput.store(setup.phraseInput,std::memory_order_relaxed);
    external.store(externalCommands,std::memory_order_relaxed);
    configSequence.fetch_add(1,std::memory_order_release);
}

std::optional<int64_t> LoopPlayback::decodeLeadIn(uint64_t word) noexcept
{
    const auto leadIn = static_cast<int64_t>((word >> leadInShift) & leadInMask);
    return leadIn == 0 ? std::nullopt : std::optional{leadIn - 1};
}

LoopPlayback::LeadInDisplay LoopPlayback::leadInDisplay() const noexcept
{
    const auto word = statusWord.load(std::memory_order_acquire);
    const auto displaySlot = word >> displaySlotShift;
    if (displaySlot == 0) return {};
    const auto& publication = slots[displaySlot - 1].data;
    return {decodeLeadIn(word),publication ? publication->leadInPeaks : nullptr};
}

LoopPlayback::Status LoopPlayback::status() const noexcept
{
    const auto word = statusWord.load(std::memory_order_acquire);
    const auto position = static_cast<int64_t>((word >> playheadShift) & playheadMask);
    return {static_cast<LoopLiveReference::State>(word & 7),
            static_cast<LoopCueScheduler::State>((word >> 3) & 7),
            static_cast<LoopPlaybackPublication::Preparation>((word >> 6) & 3),
            position == 0 ? std::nullopt : std::optional{position - playheadBias},
            decodeLeadIn(word)};
}

void LoopPlayback::process(float* left, float* right, int frames, const loop_capture::MidiObservations& observations,
                           double rate, uint64_t generation, uint64_t revision, Access access, bool discontinuity,
                           const LoopReferenceSetup& observedReference) noexcept
{
    using namespace loop_capture;
    using Preparation = LoopPlaybackPublication::Preparation;
    if (frames <= 0) return;
    // Without an accepted Loop, the worker owns any unadopted publication.
    if (revision) takePublication();
    const auto current = latest >= 0 ? slots[latest].data.get() : nullptr;
    const auto sameSource = [&](const LoopPlaybackPublication* p){
        return p && p->captureGeneration == generation && p->sourceRevision == revision;
    };
    const auto currentMatches = sameSource(current);
    const auto epoch = dependencyEpoch.load(std::memory_order_acquire);
    const auto prepared = [&](const LoopPlaybackPublication* p){
        return sameSource(p) && p->rate == rate && p->dependencies == epoch && p->preparation == Preparation::ready;
    };

    const auto before = configSequence.load(std::memory_order_acquire);
    LoopReferenceSetup setup{{referenceRate.load(std::memory_order_relaxed),referenceTempo.load(std::memory_order_relaxed)},
        referenceBeats.load(std::memory_order_relaxed),referenceNumerator.load(std::memory_order_relaxed),
        referenceDenominator.load(std::memory_order_relaxed),phraseInput.load(std::memory_order_relaxed)};
    auto externalAllowed = external.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if ((before & 1) || configSequence.load(std::memory_order_acquire) != before) setup = {};
    else if (before == 0)
    {
        setup = observedReference;
        externalAllowed = setup.clock.valid();
        if (setup.phraseInput && currentMatches)
        {
            constexpr int beatsPerBar = loop_capture::PhraseClock::beatsPerBar;
            const auto beats = current->timing.beats;
            if (beats == beatsPerBar || beats == 2 * beatsPerBar || beats == 4 * beatsPerBar)
            {
                setup.beats = beats;
                setup.observeCadence = true;
            }
        }
    }
    if (currentMatches && current->rate != rate) setup.clock.rate = rate;
    scheduler.setProcessingRate(rate);
    scheduler.configure(setup,currentMatches ? current->rate : 0,currentMatches ? std::optional{current->timing} : std::nullopt);
    const auto reset = resetEpoch.load(std::memory_order_acquire);
    const bool validFrames = frames > 0 && cursor <= INT64_MAX - frames;
    const bool canObserve = access != Access::disabled && validFrames && observations.error == MidiObservations::Error::none;
    const bool canPlay = canObserve && access == Access::perform;
    const bool sourceChanged = seenRevision != 0 && (seenGeneration != generation || seenRevision != revision);
    const bool interrupted = discontinuity || sourceChanged || (wasObserving && !canObserve) || reset != seenReset;
    seenReset = reset;
    seenGeneration = generation;
    seenRevision = revision;
    wasObserving = canObserve;
    if (interrupted || !validFrames) scheduler.invalidateReference();
    if (!validFrames) { cursor = 0; return; }
    const auto local = commands.exchange(0,std::memory_order_acq_rel);
    const auto preparation = currentMatches && current->dependencies == epoch ? current->preparation : Preparation::waiting;
    const auto publishStatus = [&]{
        auto position = scheduler.playheadAt(cursor - 1);
        uint64_t leadIn = 0;
        if (held >= 0)
        {
            const auto& pinned = *slots[held].data;
            if (position && *position >= 0)
                *position = std::min(*position,pinned.timing.endSample - 1);
            const auto start = scheduler.nextLeadInStart(pinned.plan.audio.start);
            if (start && *start < pinned.timing.endSample && static_cast<uint64_t>(*start) < leadInMask)
                leadIn = static_cast<uint64_t>(*start + 1);
        }
        const auto encoded = position && *position > -playheadBias && *position < playheadBias
            ? static_cast<uint64_t>(*position + playheadBias) : 0;
        statusWord.store(static_cast<unsigned>(scheduler.phase()) | (static_cast<unsigned>(scheduler.state()) << 3)
            | (static_cast<unsigned>(preparation) << 6) | (encoded << playheadShift)
            | (leadIn << leadInShift)
            | ((leadIn ? static_cast<uint64_t>(held + 1) : 0) << displaySlotShift),std::memory_order_release);
    };
    if (!canPlay && scheduler.state() == LoopCueScheduler::State::standby && dry == 1)
    {
        // In Learn, verify the bounded event batch without processing dry samples.
        if (canObserve)
        {
            for (unsigned event = 0; event < observations.size; ++event)
            {
                const auto& edge = observations.samples[event];
                const auto roles = edge.roles & (phrase | reset);
                if (!roles || (interrupted && edge.sampleOffset == 0)) continue;
                const auto sample = cursor + edge.sampleOffset;
                if (edge.sampleOffset != 0)
                    scheduler.processSample(sample - 1,0,false,0);
                scheduler.processSample(sample,roles,false,0);
            }
            scheduler.processSample(cursor + frames - 1,0,false,0);
        }
        releaseHeld();
        retireLatestWithoutSource(revision);
        cursor += frames;
        publishStatus();
        return;
    }
    const auto dryStep = static_cast<float>(1.0 / (std::isfinite(rate) ? std::clamp(rate * .005,1.0,960.0) : 1.0));
    unsigned event = 0;
    for (int offset = 0; offset < frames; ++offset)
    {
        const auto sample = cursor + offset;
        uint16_t roles = 0;
        if (canObserve && event < observations.size && observations.samples[event].sampleOffset == offset)
            roles = observations.samples[event++].roles;
        if (!canPlay || !externalAllowed) roles &= ~(cue | stop);
        if (offset == 0 && !interrupted && canPlay) roles |= local;
        if (offset == 0 && interrupted) roles = 0;
        const auto plan = held >= 0 ? slots[held].data.get() : current;
        const auto ready = canPlay && prepared(plan);
        scheduler.processSample(sample,roles,ready,ready ? -plan->plan.audio.start : 0);
        if (scheduler.active() && held < 0) held = latest;
        const auto state = scheduler.state();
        const auto playing = state == LoopCueScheduler::State::leadIn || state == LoopCueScheduler::State::repeating;
        LoopWetSample wet;
        if (playing && ready)
        {
            wet = plan->plan.sample(scheduler.nearCycle(sample),scheduler.firstCycle(),INT64_MAX,
                [&](int64_t cycle){ return scheduler.positionFor(cycle,sample); });
            wet.left *= plan->gain;
            wet.right *= plan->gain;
            lastWet = wet;
        }
        else if (state == LoopCueScheduler::State::stopping)
        {
            const auto fade = scheduler.fadeWeight();
            wet = {lastWet.left * fade,lastWet.right * fade};
        }
        else lastWet = {};
        if (!scheduler.active()) releaseHeld();
        const auto dryTarget = playing && ready && plan->wetOnly ? 0.0f : 1.0f;
        dry += std::clamp(dryTarget-dry,-dryStep,dryStep);
        if (dry != 1 || wet.left != 0) left[offset] = dry * left[offset] + wet.left;
        if (dry != 1 || wet.right != 0) right[offset] = dry * right[offset] + wet.right;
    }
    retireLatestWithoutSource(revision);
    cursor += frames;
    publishStatus();
}
}
