#include "LoopPlayback.h"

namespace m3
{
bool LoopPlayback::requestCue() noexcept
{
    const auto fence = tryFenceCue();
    if (!fence.isLocked()) return false;
    commands.fetch_or(loop_capture::cue, std::memory_order_release);
    return true;
}

namespace
{
std::optional<int64_t> playlistDownbeat(const SongTime& time, int64_t blockStart,
                                       int offset, int64_t lead, double originPpq) noexcept
{
    const auto delay = offset + std::max(int64_t{1},lead);
    if (delay > INT64_MAX - time.sample) return {};
    const auto earliest = time.sample + delay;
    const auto ppq = time.ppq + (static_cast<double>(delay) - .5) * time.bpm / (time.rate * 60);
    const auto bar = originPpq + std::ceil((ppq - originPpq) / 4) * 4;
    int64_t boundary = 0;
    if (!resolvePpqSample(time,bar,boundary)) return {};
    if (boundary < earliest && !resolvePpqSample(time,bar + 4,boundary)) return {};
    if (boundary < earliest || boundary - time.sample > INT64_MAX - blockStart) return {};
    return blockStart + (boundary - time.sample);
}
}

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
            peak.minimum = std::min({peak.minimum,mix.left,mix.right});
            peak.maximum = std::max({peak.maximum,mix.left,mix.right});
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
        if (slot.state.load(std::memory_order_acquire) != retired) continue;
        const auto status = statusWord.load(std::memory_order_acquire);
        if (!decodeLeadIn(status) || ((status >> displaySlotShift) & displaySlotMask) != index)
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
    if (latest >= 0 && latest != held && latest != fadingFrom)
        slots[latest].state.store(retired,std::memory_order_release);
    latest = next;
    slots[latest].state.store(retained,std::memory_order_release);
}

void LoopPlayback::releaseEditFade() noexcept
{
    if (fadingFrom >= 0 && fadingFrom != latest)
        slots[fadingFrom].state.store(retired,std::memory_order_release);
    fadingFrom = -1;
    editFadeRemaining = 0;
}

void LoopPlayback::releaseHeld() noexcept
{
    releaseEditFade();
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
    const auto word = currentStatusWord();
    const auto leadIn = decodeLeadIn(word);
    if (!leadIn) return {};
    const auto& publication = slots[(word >> displaySlotShift) & displaySlotMask].data;
    if (!publication) return {};
    return {leadIn,publication->leadInPeaks,publication->gain};
}

uint64_t LoopPlayback::currentStatusWord() const noexcept
{
    const auto publishedReset = statusResetEpoch.load(std::memory_order_acquire);
    const auto word = statusWord.load(std::memory_order_acquire);
    return publishedReset == resetEpoch.load(std::memory_order_acquire) ? word : 0;
}

LoopPlayback::Status LoopPlayback::status() const noexcept
{
    const auto word = currentStatusWord();
    const auto position = static_cast<int64_t>((word >> playheadShift) & playheadMask);
    return {static_cast<LoopLiveReference::State>(word & 7),
            static_cast<LoopCueScheduler::State>((word >> 3) & 7),
            static_cast<LoopPlaybackPublication::Preparation>((word >> 6) & 3),
            position == 0 ? std::nullopt : std::optional{position - playheadBias},
            decodeLeadIn(word),statusRoute.load(std::memory_order_acquire),
            statusPlacementAvailable.load(std::memory_order_acquire)};
}

void LoopPlayback::process(float* left, float* right, int frames, const loop_capture::MidiObservations& observations,
                           double rate, uint64_t generation, uint64_t revision, Access access, bool discontinuity,
                           const LoopReferenceSetup& observedReference, std::optional<SongTime> playlist,
                           HostSchedule schedule, bool hostPlaying) noexcept
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
    const auto compatible = [&](const LoopPlaybackPublication* p){
        return sameSource(p) && p->rate == rate;
    };
    const auto compatibleGrid = currentMatches && playlist && playlist->valid && compatible(current)
        && playlist->bpm == current->timing.tempoBpm && playlist->numerator == current->timing.meterNumerator
        && playlist->denominator == current->timing.meterDenominator;
    if (schedule.route == Route::hostRepeat && (!compatibleGrid
        || std::abs(schedule.period-current->timing.endSample) > 1))
        schedule = {};
    if (schedule.route == Route::placement && compatibleGrid && schedule.period == 0)
        schedule.period = current->timing.endSample;
    if (!discontinuity && schedule.route == Route::placement && previousSchedule.route == Route::placement
        && schedule.period == previousSchedule.period
        && std::abs(static_cast<long double>(schedule.start)-previousSchedule.start) <= 1)
        schedule.start = previousSchedule.start;

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
    if (playlist)
    {
        setup = {};
        externalAllowed = playlist->valid;
    }
    const bool autoClock = before == 0 && !playlist;
    if (autoClock && scheduler.usesClock())
    {
        setup = {};
        externalAllowed = true;
    }
    scheduler.setProcessingRate(rate);
    scheduler.configure(setup,currentMatches ? current->rate : 0,currentMatches ? std::optional{current->timing} : std::nullopt,
                        playlist && playlist->valid,autoClock);
    const bool validFrames = frames > 0 && cursor <= INT64_MAX - frames;
    const bool canObserve = access != Access::disabled && validFrames && observations.error == MidiObservations::Error::none;
    const bool canPlay = canObserve && access == Access::perform;
    const auto commandEpoch = resetEpoch.load(std::memory_order_acquire);
    const auto local = commands.exchange(0, std::memory_order_acq_rel);
    const bool midiCue = canPlay && externalAllowed
        && std::any_of(observations.samples.begin(), observations.samples.begin() + observations.size,
                       [](const auto& event){return (event.roles & loop_capture::cue) != 0;});
    const auto reset = resetEpoch.load(std::memory_order_acquire);
    const bool sourceChanged = seenRevision != 0 && (seenGeneration != generation || seenRevision != revision);
    const bool timingInterrupted = discontinuity || (wasObserving && !canObserve) || reset != seenReset;
    const bool routeChanged = schedule != previousSchedule;
    const bool interrupted = timingInterrupted || sourceChanged || routeChanged;
    seenReset = reset;
    seenGeneration = generation;
    seenRevision = revision;
    wasObserving = canObserve;
    if (interrupted || !validFrames) scheduler.invalidateReference(timingInterrupted || !validFrames);
    // Workflow changes fade wet without suppressing automatic playback on return.
    if (local & leavePerformCommand) scheduler.stop();
    if (!hostPlaying) timelineSuppressed = false;
    if (interrupted || !canPlay) timeline.reset();
    constexpr auto stopRoles = finish | stop | LoopCueScheduler::finishCommand;
    if (hostPlaying && (schedule.route != Route::manual || previousSchedule.route != Route::manual)
        && (local & stopRoles))
        timelineSuppressed = true;
    previousSchedule = schedule;
    statusRoute.store(schedule.route,std::memory_order_release);
    statusPlacementAvailable.store(schedule.route == Route::placement && compatibleGrid && schedule.period > 0,
                                   std::memory_order_release);
    if (canPlay && !timeline && playlist && schedule.route != Route::manual && schedule.period > 0)
    {
        // Give a newly joined repeating range enough preceding passes for every
        // tail the bounded wet plan can mix. A placed passage starts at pass zero.
        const auto history = schedule.route == Route::hostRepeat ? LoopCueScheduler::anchorCapacity : 0;
        const auto origin = static_cast<long double>(cursor) + schedule.start
            - playlist->sample - static_cast<long double>(history) * schedule.period;
        if (std::abs(origin) < INT64_MAX / 4 && cursor < INT64_MAX / 4)
            timeline = LoopCueScheduler::Timeline{static_cast<int64_t>(origin),schedule.period};
    }
    std::optional<juce::SpinLock::ScopedTryLockType> fence;
    if (canPlay && (midiCue || (local & cue) || (timeline && !timelineSuppressed && !scheduler.active())))
        fence.emplace(cueFence);
    // Hold through status publication so Storage sees any Cue admitted by this block.
    const bool canCue = fence && fence->isLocked() && commandEpoch == reset
        && resetEpoch.load(std::memory_order_acquire) == reset;
    if (!canObserve || timingInterrupted || !playlist || !playlist->valid) playlistPhrasePpq.reset();
    if (!validFrames) { cursor = 0; signalPeaks.publish(0,0,frames,rate); return; }
    const auto observePlaylistPhrase = [&](uint16_t roles, int offset)
    {
        if ((roles & phrase) && playlist && playlist->valid && scheduler.state() == LoopCueScheduler::State::standby)
            playlistPhrasePpq = std::fmod(playlist->ppq + offset * (playlist->bpm / (playlist->rate * 60)),4.0);
    };
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
            | (leadIn ? (static_cast<uint64_t>(held) << displaySlotShift) : 0),std::memory_order_release);
        statusResetEpoch.store(reset,std::memory_order_release);
    };
    if (!canPlay && scheduler.state() == LoopCueScheduler::State::standby && dry == 1)
    {
        signalPeaks.publish(0,0,frames,rate);
        // In Learn, verify the bounded event batch without processing dry samples.
        if (canObserve)
        {
            for (unsigned event = 0; event < observations.size; ++event)
            {
                const auto& edge = observations.samples[event];
                const auto roles = edge.roles & (captureClock | rackBeatMask | bar | phrase | reset);
                const bool newClock = autoClock && (roles & captureClock);
                if (!roles || (timingInterrupted && edge.sampleOffset == 0 && !newClock)) continue;
                observePlaylistPhrase(roles,edge.sampleOffset);
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
    using State = LoopCueScheduler::State;
    const auto fadeSamples = std::isfinite(rate) ? std::clamp(rate * .005,1.0,960.0) : 1.0;
    const auto dryStep = static_cast<float>(1.0 / fadeSamples);
    if (canPlay && !interrupted && scheduler.state() == State::repeating && held >= 0
        && fadingFrom < 0 && latest != held && compatible(current) && current->dependencies == epoch
        && (current->preparation == Preparation::ready || current->intentionallySilent))
    {
        fadingFrom = held;
        held = latest;
        editFadeRemaining = editFadeLength = static_cast<int>(fadeSamples);
    }
    unsigned event = 0;
    float wetPeak = 0, outputPeak = 0;
    for (int offset = 0; offset < frames; ++offset)
    {
        const auto sample = cursor + offset;
        uint16_t roles = 0;
        if (canObserve && event < observations.size && observations.samples[event].sampleOffset == offset)
            roles = observations.samples[event++].roles;
        if (!canPlay || !externalAllowed) roles &= ~(cue | finish | stop);
        if (hostPlaying && schedule.route != Route::manual && (roles & stopRoles))
            timelineSuppressed = true;
        if (offset == 0 && !interrupted && canPlay) roles |= static_cast<uint16_t>(local);
        if (!canCue) roles &= ~cue;
        if (offset == 0 && interrupted && (!playlist || discontinuity))
            roles = autoClock && (roles & captureClock) ? roles & (captureClock | rackBeatMask | bar | phrase) : 0;
        if (schedule.route != Route::manual)
            roles &= ~cue;
        if (roles & phrase) observePlaylistPhrase(roles,offset);
        const auto plan = held >= 0 ? slots[held].data.get() : current;
        const auto playingBefore = scheduler.active() && scheduler.state() != State::queued;
        const auto ready = canPlay && compatible(plan) && (held >= 0 && playingBefore
            ? plan->preparation == Preparation::ready || plan->intentionallySilent
            : plan->dependencies == epoch && plan->preparation == Preparation::ready);
        const auto lead = ready ? -plan->plan.audio.start : 0;
        const auto downbeat = ready && playlist && playlist->valid && (roles & cue)
            ? playlistDownbeat(*playlist,cursor,offset,lead,playlistPhrasePpq.value_or(0)) : std::nullopt;
        scheduler.processSample(sample,roles,ready,lead,downbeat,timeline ? &*timeline : nullptr,
                                ready ? plan->plan.audio.end : 0,canCue && !timelineSuppressed);
        if (scheduler.active() && held < 0) held = latest;
        const auto state = scheduler.state();
        const auto playing = state == LoopCueScheduler::State::leadIn || state == LoopCueScheduler::State::repeating
            || state == LoopCueScheduler::State::finishing;
        LoopWetSample wet;
        if (playing && ready)
        {
            const auto cycle = scheduler.nearCycle(sample);
            const auto position = [&](int64_t index){ return scheduler.positionFor(index,sample); };
            const auto samplePlan = [&](const LoopPlaybackPublication& publication){
                const auto mix = publication.plan.sample(cycle,scheduler.firstCycle(),scheduler.lastCycle(),position);
                const auto gain = publication.gain;
                return LoopWetSample{mix.left * gain,mix.right * gain,mix.present};
            };
            wet = samplePlan(*plan);
            if (fadingFrom >= 0)
            {
                const auto old = samplePlan(*slots[fadingFrom].data);
                const auto weight = 1.0f - static_cast<float>(--editFadeRemaining) / static_cast<float>(editFadeLength);
                wet = {std::lerp(old.left,wet.left,weight),std::lerp(old.right,wet.right,weight),
                    wet.present || (old.present && weight < 1.0f)};
                if (editFadeRemaining == 0) releaseEditFade();
            }
            lastWet = wet;
        }
        else if (state == LoopCueScheduler::State::stopping)
        {
            const auto fade = scheduler.fadeWeight();
            wet = {lastWet.left * fade,lastWet.right * fade,lastWet.present && fade > 0.0f};
        }
        else lastWet = {};
        if (!scheduler.active()) releaseHeld();
        const auto dryTarget = playing && ready && plan->wetOnly ? 0.0f : 1.0f;
        dry += std::clamp(dryTarget-dry,-dryStep,dryStep);
        if (dry != 1 || wet.left != 0) left[offset] = dry * left[offset] + wet.left;
        if (dry != 1 || wet.right != 0) right[offset] = dry * right[offset] + wet.right;
        if (wet.present)
        {
            wetPeak = std::max({wetPeak,std::abs(wet.left),std::abs(wet.right)});
            outputPeak = std::max({outputPeak,std::abs(left[offset]),std::abs(right[offset])});
        }
    }
    signalPeaks.publish(wetPeak,outputPeak,frames,rate);
    retireLatestWithoutSource(revision);
    cursor += frames;
    publishStatus();
}
}
