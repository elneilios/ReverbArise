#include "LoopPlayback.h"

namespace m3
{
void LoopPlayback::collect()
{
    for (auto& slot : slots)
        if (slot.state.load(std::memory_order_acquire) == retired)
        {
            slot.data.reset();
            slot.state.store(free,std::memory_order_release);
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
    for (const auto& slot : slots) if (slot.data) result += slot.data->plan.audio.bytes();
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
    originConfirmed.store(setup.sharedOrigin,std::memory_order_relaxed);
    external.store(externalCommands,std::memory_order_relaxed);
    configSequence.fetch_add(1,std::memory_order_release);
}

LoopPlayback::Status LoopPlayback::status() const noexcept
{
    const auto word = statusWord.load(std::memory_order_acquire);
    return {static_cast<LoopLiveReference::State>(word & 7),
            static_cast<LoopCueScheduler::State>((word >> 3) & 7),
            static_cast<LoopPlaybackPublication::Preparation>((word >> 6) & 3)};
}

void LoopPlayback::process(float* left, float* right, int frames, const loop_capture::MidiObservations& observations,
                           double rate, uint64_t generation, uint64_t revision, bool allowed, bool discontinuity) noexcept
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
        referenceDenominator.load(std::memory_order_relaxed),originConfirmed.load(std::memory_order_relaxed)};
    const auto externalAllowed = external.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if ((before & 1) || configSequence.load(std::memory_order_acquire) != before) setup = {};
    if (currentMatches && current->rate != rate) setup.clock.rate = rate;
    scheduler.setProcessingRate(rate);
    scheduler.configure(setup,currentMatches ? current->rate : 0,currentMatches ? std::optional{current->timing} : std::nullopt);
    const auto reset = resetEpoch.load(std::memory_order_acquire);
    const bool validFrames = frames > 0 && cursor <= INT64_MAX - frames;
    const bool canObserve = allowed && validFrames && observations.error == MidiObservations::Error::none;
    const bool interrupted = discontinuity || (wasAllowed && !canObserve) || reset != seenReset;
    seenReset = reset;
    wasAllowed = canObserve;
    if (interrupted || !validFrames) scheduler.invalidateReference();
    if (!validFrames) { cursor = 0; return; }
    const auto local = commands.exchange(0,std::memory_order_acq_rel);
    const auto preparation = currentMatches && current->dependencies == epoch ? current->preparation : Preparation::waiting;
    const auto publishStatus = [&]{
        statusWord.store(static_cast<unsigned>(scheduler.phase()) | (static_cast<unsigned>(scheduler.state()) << 3)
            | (static_cast<unsigned>(preparation) << 6),std::memory_order_release);
    };
    if (!canObserve && scheduler.state() == LoopCueScheduler::State::standby && dry == 1)
    {
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
        if (!externalAllowed) roles &= ~(cue | stop);
        if (offset == 0 && !interrupted && canObserve) roles |= local;
        if (offset == 0 && interrupted) roles = 0;
        const auto plan = held >= 0 ? slots[held].data.get() : current;
        const auto ready = canObserve && prepared(plan);
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
