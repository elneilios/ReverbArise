#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Playback/LoopLiveReference.h>
#include <Playback/LoopCueScheduler.h>
#include <Playback/LoopWetPlan.h>
#include <Playback/LoopPlayback.h>
#include <fstream>

namespace
{
using namespace m3;
using namespace loop_capture;

uint16_t referenceRoles(int index, int phraseBeats)
{
    return beat | (index % 4 == 0 ? bar : 0) | (index % phraseBeats == 0 ? phrase : 0);
}

void liveReferenceCases()
{
    using State = LoopLiveReference::State;
    LoopLiveReference reference;
    const LoopPhraseTiming threeBars{137,4,4,12,8341,252263};
    const LoopReferenceSetup fourBarReference{{48000,137},16,4,4,true};
    reference.configure({},48000,threeBars);
    for (int index = 0; index < 64; ++index)
        REQUIRE(!reference.observe(8341 + index * 21022, captureClock).boundary);
    REQUIRE(reference.state() == State::unconfigured);
    reference.configure(fourBarReference,48000,threeBars);
    for (int index = 0; index <= 32; ++index)
        REQUIRE(!reference.observe(index * 21022, referenceRoles(index,16)).boundary);
    REQUIRE(reference.state() == State::mismatch);

    constexpr int64_t liveOrigin = 320;
    for (auto length : {4,12,16})
    for (auto bpm : {137.,280.})
    {
        const auto sample = [&](int index) { return liveOrigin + std::llround(48000.L * 60 * index / bpm); };
        const LoopPhraseTiming source{length == 4 && bpm == 137 ? 136.999334031015138 : bpm,
                                     4,4,length,8341,sample(length) - sample(0)};
        REQUIRE(reference.configure({{48000,bpm},length,4,4,true},48000,source));
        for (int index = 0; index < 2 * length; ++index)
        {
            REQUIRE(!reference.observe(sample(index), referenceRoles(index,length)).boundary);
            REQUIRE(reference.state() == State::waiting);
        }
        REQUIRE(reference.observe(sample(2 * length), referenceRoles(2 * length,length)).boundary);
        REQUIRE(reference.state() == State::ready && reference.ordinal() == 2);
        const auto projected = reference.window(3);
        REQUIRE(projected && projected->earliest <= sample(3 * length) && projected->latest >= sample(3 * length));
        for (int index = 2 * length + 1; index <= 3 * length; ++index)
            REQUIRE(reference.observe(sample(index), referenceRoles(index,length)).boundary == (index == 3 * length));
        const auto late = sample(3 * length) + static_cast<int64_t>(std::ceil(48000.L * 60 * length / bpm)) + 1;
        REQUIRE(reference.observe(late,cue).lost && reference.state() == State::lost);
        REQUIRE(!reference.window(4));
    }

    REQUIRE(reference.configure(fourBarReference,48000,LoopPhraseTiming{137,4,4,16,8341,336350}));
    for (int index = 0; index <= 32; ++index)
        reference.observe(std::llround(48000.L * 60 * index / 137),referenceRoles(index,16));
    REQUIRE(reference.state() == State::ready);
    REQUIRE(reference.observe(std::llround(48000.L * 60 * 48 / 137) + 4,phrase).lost);
    REQUIRE(reference.state() == State::lost);
}

LiveEntry testEntry(size_t slot, int64_t start, int64_t frames, float scale, double rate);

void automaticShortPhraseCases()
{
    using Phase = LoopLiveReference::State;
    using Playing = LoopCueScheduler::State;
    for (const int beats : {4,8})
    {
        LoopPlayback playback;
        const auto marker = [](int beatIndex){return 37 + std::llround(48000.L * 60 * beatIndex / 137);};
        const auto frames = marker(beats) - marker(0);
        auto publication = std::make_unique<LoopPlaybackPublication>();
        publication->captureGeneration = publication->sourceRevision = 1;
        publication->rate = 48000;
        publication->timing = {137,4,4,beats,0,frames};
        publication->preparation = LoopPlaybackPublication::Preparation::ready;
        publication->plan.audio.entries = {testEntry(0,0,128,.01f,48000)};
        juce::String error;
        REQUIRE(publication->plan.prepare(frames,1024*1024,error));
        REQUIRE(playback.publish(std::move(publication)));
        const LoopReferenceSetup rack{{48000,137},16,4,4,true};
        int64_t cursor = 0;
        const auto through = [&](int beatIndex, bool sendPhrase, LoopPlayback::Access access=LoopPlayback::Access::perform){
            const auto boundary = marker(beatIndex);
            bool heardWet = false;
            while (cursor <= boundary)
            {
                std::array<float,128> left{},right{};
                MidiObservations events;
                if (sendPhrase && boundary < cursor + static_cast<int64_t>(left.size()))
                    events.samples[events.size++] = {static_cast<int>(boundary-cursor),phrase};
                { AudioAllocationGuard guard;
                    playback.process(left.data(),right.data(),static_cast<int>(left.size()),events,48000,1,1,access,false,rack); }
                REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
                heardWet |= std::any_of(left.begin(),left.end(),[](float sample){return sample != 0;});
                cursor += left.size();
            }
            return heardWet;
        };
        const auto learn=LoopPlayback::Access::observe;
        REQUIRE(!through(0,true,learn));
        REQUIRE(!through(16,true,learn));
        REQUIRE(playback.status().phase == Phase::mismatch);
        REQUIRE(!through(16 + beats,true,learn));
        REQUIRE(playback.status().phase != Phase::ready);
        playback.requestCue();
        REQUIRE(!through(16 + 2 * beats,true,learn));
        REQUIRE(playback.status().phase == Phase::ready);
        REQUIRE(!through(16 + 2 * beats + 1,false));
        REQUIRE(playback.status().playback == Playing::standby);
        REQUIRE(playback.requestCue());
        {
            const auto handoff = playback.tryFenceCue();
            REQUIRE(handoff.isLocked());
            REQUIRE(!playback.requestCue());
            std::array<float,128> left{},right{};
            MidiObservations events;
            events.samples[events.size++] = {37,cue};
            { AudioAllocationGuard guard;
                playback.process(left.data(),right.data(),static_cast<int>(left.size()),events,
                                 48000,1,1,LoopPlayback::Access::perform,false,rack); }
            REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
            cursor += left.size();
            REQUIRE(playback.status().playback == Playing::standby);
            REQUIRE(std::all_of(left.begin(),left.end(),[](float sample){return sample == 0;}));
        }
        REQUIRE(playback.requestCue());
        REQUIRE(through(16 + 3 * beats,true));
        REQUIRE(playback.status().playback == Playing::repeating);
        through(16 + 4 * beats + 1,false);
        REQUIRE(playback.status().phase == Phase::lost && playback.status().playback == Playing::standby);
    }
}

void schedulerCases()
{
    using State = LoopCueScheduler::State;
    using Phase = LoopLiveReference::State;
    LoopCueScheduler scheduler;
    scheduler.setProcessingRate(48000);
    constexpr int64_t beatFrames = 24000, period = 4 * beatFrames;
    const auto configure = [&]{ scheduler.configure({{48000,120},4,4,4,true},48000,
                                                  LoopPhraseTiming{120,4,4,4,8337,period}); };
    const auto sample = [&](int64_t at, uint16_t roles = 0, int64_t lead = 1000){
        scheduler.processSample(at,roles,true,lead);
    };
    const auto markers = [&](int first, int last){
        for (int n = first; n <= last; ++n) sample(n * beatFrames, referenceRoles(n,4));
    };
    configure();
    markers(0,7);
    sample(2 * period,referenceRoles(8,4) | cue | stop);
    REQUIRE(scheduler.phase() == Phase::ready && scheduler.state() == State::standby);
    sample(2 * period + 1,cue);
    REQUIRE(scheduler.firstCycle() == 3 && scheduler.state() == State::queued);
    markers(9,11);
    sample(3 * period - 1000);
    REQUIRE(scheduler.state() == State::leadIn && scheduler.positionFor(3,3 * period - 1000) == -1000);
    sample(3 * period,referenceRoles(12,4));
    REQUIRE(scheduler.state() == State::repeating && scheduler.positionFor(3,3 * period) == 0);
    sample(3 * period + 1,captureStop | captureArm);
    REQUIRE(scheduler.state() == State::repeating);
    sample(3 * period + 2,stop | cue);
    REQUIRE(scheduler.state() == State::stopping && scheduler.firstCycle() == -1);
    REQUIRE(scheduler.fadeWeight() < 1);
    for (int n = 1; n < 240; ++n) scheduler.fadeWeight();
    REQUIRE(scheduler.state() == State::standby);

    markers(13,15);
    sample(4 * period - 999,cue);
    REQUIRE(scheduler.firstCycle() == 5 && scheduler.state() == State::queued);
    sample(4 * period,referenceRoles(16,4));
    REQUIRE(scheduler.state() == State::queued);
    sample(4 * period + 1,stop);
    REQUIRE(scheduler.state() == State::standby);
    sample(4 * period + 2,cue,period + 1000);
    REQUIRE(scheduler.firstCycle() == 6);
    scheduler.invalidateReference();
    REQUIRE(scheduler.phase() == Phase::lost && scheduler.state() == State::standby);
    markers(20,28);
    REQUIRE(scheduler.phase() == Phase::ready && scheduler.state() == State::standby);
    sample(8 * period + 1,cue);
    REQUIRE(scheduler.phase() == Phase::lost && scheduler.state() == State::standby);
}

void earliestCueCase()
{
    using State = LoopCueScheduler::State;
    LoopCueScheduler scheduler;
    const auto tempo = 8000. * 60 / 2000.5;
    scheduler.configure({{8000,tempo},1,4,4,true},8000,LoopPhraseTiming{tempo,4,4,1,0,2000});
    for (const auto marker : {0,2001,4002}) scheduler.processSample(marker,phrase,true,1000);
    REQUIRE(scheduler.phase() == LoopLiveReference::State::ready);
    scheduler.processSample(5002,cue,true,1000);
    REQUIRE(scheduler.firstCycle() == 3 && scheduler.state() == State::leadIn);
    REQUIRE(scheduler.positionFor(3,5002) == -1000);
    REQUIRE(scheduler.positionFor(3,6001) == -1);
    scheduler.processSample(6002,phrase,true,1000);
    REQUIRE(scheduler.state() == State::repeating && scheduler.positionFor(3,6002) == 0);
}

LiveEntry testEntry(size_t slot, int64_t start, int64_t frames, float scale, double rate=48000)
{
    auto render = std::make_shared<WetRender>();
    render->rate = rate;
    render->start = start;
    for (int64_t n = 0; n < frames; ++n)
    {
        const auto value = scale * static_cast<float>(1 + n % 7);
        render->samples.push_back(value);
        render->samples.push_back(-value);
        render->peak = std::max(render->peak,value);
    }
    return {slot,1,start,start+frames,render->peak,std::move(render),{},1};
}

void compositorCases()
{
    LoopWetPlan plan;
    plan.audio.entries = {testEntry(0,-121,360,.01f),testEntry(1,-30,96,.02f),testEntry(2,62,213,.03f)};
    juce::String error;
    REQUIRE(plan.prepare(101,1024*1024,error));
    for (const int block : {1,17,64})
    for (int from = 180; from < 1250; from += block)
    for (int at = from; at < std::min(from+block,1250); ++at)
    {
        const auto position = [&](int64_t cycle) -> std::optional<int64_t>{
            const auto nominal = 100.75L * cycle;
            const auto earliest = static_cast<int64_t>(std::floor(nominal));
            const auto actual = static_cast<int64_t>(std::llround(nominal));
            return at < actual ? std::min(int64_t{-1},at-earliest) : at-actual;
        };
        const auto rendered = plan.sample(static_cast<int64_t>(at/100.75L),3,9,position);
        float expected = 0;
        for (int cycle = 3; cycle <= 9; ++cycle)
        for (const auto& entry : plan.audio.entries)
        {
            const auto local = *position(cycle);
            if (local >= entry.start && local < entry.end)
                expected += entry.audio->samples[static_cast<size_t>(local-entry.start)*2];
        }
        REQUIRE(std::abs(rendered.left-expected)<1e-6f && std::abs(rendered.right+expected)<1e-6f);
    }
    REQUIRE(!plan.prepare(101,100,error) && error.contains("PCM budget"));
    plan.audio.entries.clear();
    for (size_t index=0; index<40; ++index) plan.audio.entries.push_back(testEntry(index,-50,210,.01f));
    REQUIRE(!plan.prepare(101,1024*1024,error) && error.contains("simultaneous"));
}

void fixedGridPeakCases()
{
    LoopWetPlan plan;
    auto left = std::make_shared<WetRender>();
    auto right = std::make_shared<WetRender>();
    left->start = -12;
    right->start = 8;
    left->samples.resize(8);
    right->samples.resize(8);
    left->samples[0] = .7f;
    right->samples[2] = -.7f;
    left->peak = right->peak = .7f;
    plan.audio.entries = {{0,0,-12,-8,.7f,left},{1,0,8,12,.7f,right}};
    juce::String error;
    REQUIRE(plan.prepare(20,1024*1024,error));
    REQUIRE(std::abs(plan.peakBound - 1.4f) < 1e-6f);
    REQUIRE(std::abs(plan.fixedGridPeak - .7f) < 1e-6f);

    // These two passes now hit the same phase. Opposite polarities cancel in
    // the repeating mix, but a single pass at Cue or Stop still peaks at .7.
    right->samples[2] = 0;
    right->samples[0] = -.7f;
    REQUIRE(plan.prepare(20,1024*1024,error));
    REQUIRE(std::abs(plan.fixedGridPeak - .7f) < 1e-6f);
    right->samples[0] = .7f;
    REQUIRE(plan.prepare(20,1024*1024,error));
    REQUIRE(std::abs(plan.fixedGridPeak - 1.4f) < 1e-6f);
    const std::array candidateGains{1.0f,.2f};
    REQUIRE(plan.prepare(20,1024*1024,error,{},candidateGains));
    REQUIRE(std::abs(plan.fixedGridPeak - .84f) < 1e-6f);
    REQUIRE(left->samples[0] == .7f && right->samples[0] == .7f);

    plan.audio.entries = {testEntry(0,-121,360,.01f),testEntry(1,-30,96,.02f),testEntry(2,62,213,.03f)};
    constexpr int64_t period = 101, near = 5;
    REQUIRE(plan.prepare(period,1024*1024,error));
    float measured = 0;
    for (int64_t phase = 0; phase < period; ++phase)
    for (int64_t first = 0; first < 10; ++first)
    for (auto last = first; last < 10; ++last)
    {
        const auto mix = plan.sample(near, first, last,
            [&](int64_t cycle){ return std::optional{(near - cycle) * period + phase}; });
        measured = std::max({measured, std::abs(mix.left), std::abs(mix.right)});
    }
    REQUIRE(measured <= plan.fixedGridPeak && plan.fixedGridPeak - measured < 1e-5f);
    REQUIRE(!plan.prepare(period,1024*1024,error,[]{return true;}) && error.contains("superseded"));
}

void publicationCases()
{
    using Prep = LoopPlaybackPublication::Preparation;
    using PlaybackState = LoopCueScheduler::State;
    LoopPlayback playback;
    const auto publish = [&](float level, uint64_t revision = 1, bool silent = false){
        auto next = std::make_unique<LoopPlaybackPublication>();
        next->captureGeneration = 1;
        next->sourceRevision = revision;
        next->rate = 8000;
        next->timing = {300,4,4,4,123,6400};
        next->dependencies = playback.dependencies();
        next->preparation = silent ? Prep::noWet : Prep::ready;
        next->intentionallySilent = silent;
        if (!silent)
        {
            next->plan.audio.entries.push_back(testEntry(0,-16,80,level,8000));
            juce::String error;
            REQUIRE(next->plan.prepare(6400,1024*1024,error));
        }
        REQUIRE(playback.publish(std::move(next)));
    };
    publish(.01f);
    playback.setReference({{8000,300},4,4,4,true},false);
    int64_t position = 0;
    const auto advance = [&](int64_t until, int64_t commandSample=-1, uint16_t roles=0, uint64_t source=1){
        float sample = 0;
        while (position <= until)
        {
            std::array<float,64> left,right;
            const auto count = static_cast<int>(std::min<int64_t>(64,until-position+1));
            left.fill(.1f);right.fill(.2f);
            MidiObservations events;
            for (int offset = 0; offset < count; ++offset)
            {
                const auto at = position + offset;
                uint16_t bits = at % 1600 == 0 ? referenceRoles(static_cast<int>(at/1600),4) : 0;
                if (at == commandSample) bits |= roles;
                if (bits) events.samples[events.size++] = {offset,bits};
            }
            { AudioAllocationGuard guard; playback.process(left.data(),right.data(),count,events,8000,1,source,
                source!=0?LoopPlayback::Access::perform:LoopPlayback::Access::disabled,false); }
            REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
            for (int n = 0; n < count; ++n) REQUIRE(std::abs(left[n]+right[n]-.3f)<1e-6f);
            sample = left[count-1];
            position += count;
        }
        return sample;
    };
    REQUIRE(advance(12801,12801,cue)==.1f); // External commands are disabled.
    REQUIRE(playback.status().phase == LoopLiveReference::State::ready);
    REQUIRE(!playback.status().playhead);
    playback.requestCue();
    REQUIRE(advance(12802)==.1f && !playback.status().playhead);
    REQUIRE(std::abs(advance(19184)-.11f)<1e-6f);
    REQUIRE(playback.status().playhead == -16);
    REQUIRE(!playback.status().nextLeadInStart);
    REQUIRE(std::abs(advance(19200)-.13f)<1e-6f);
    REQUIRE(playback.status().playback == PlaybackState::repeating);
    REQUIRE(playback.status().playhead == 0);
    REQUIRE(playback.status().nextLeadInStart == 6384);
    playback.invalidatePrepared();
    REQUIRE(std::abs(advance(19207)-.13f)<1e-6f);
    REQUIRE(playback.status().preparation == Prep::waiting);
    REQUIRE(playback.status().playhead == 7);
    publish(.05f);
    REQUIRE(std::abs(advance(19227)-.16f)<1e-6f); // Halfway through the 40-sample fade.
    REQUIRE(playback.status().playhead == 27);
    REQUIRE(std::abs(advance(19247)-.15f)<1e-6f); // Only the successor remains.
    const auto both = playback.pinnedBytes();
    playback.collect();
    REQUIRE(playback.pinnedBytes() < both);
    advance(25599);
    REQUIRE(playback.status().playhead == 6399);
    playback.invalidatePrepared();
    REQUIRE(std::abs(advance(25600)-.25f)<1e-6f && playback.status().playhead == 0);
    publish(0,1,true);
    REQUIRE(advance(25650)==.1f && playback.status().playback == PlaybackState::repeating);
    REQUIRE(playback.status().playhead == 50);
    REQUIRE(!playback.status().nextLeadInStart);
    REQUIRE(playback.status().preparation == Prep::noWet);
    playback.invalidatePrepared();
    publish(.05f);
    REQUIRE(std::abs(advance(32000)-.25f)<1e-6f && playback.status().playhead == 0);
    publish(.05f,2);
    REQUIRE(advance(32050,-1,0,2)==.1f);
    REQUIRE(playback.status().phase != LoopLiveReference::State::ready); // Same length, different accepted source.
    REQUIRE(advance(51200,-1,0,2)==.1f);
    REQUIRE(playback.status().playback == PlaybackState::standby && playback.status().phase == LoopLiveReference::State::ready);
    playback.requestCue();
    REQUIRE(advance(57600,-1,0,2)>.1f);
    playback.collect();
    const auto acceptedBytes = playback.pinnedBytes();
    REQUIRE(acceptedBytes > 0);
    publish(.02f,2);
    REQUIRE(playback.pinnedBytes() == 2 * acceptedBytes);
    REQUIRE(advance(57664,-1,0,0)==.1f && playback.status().playback == PlaybackState::standby);
    playback.collect();
    REQUIRE(playback.pinnedBytes() == acceptedBytes); // The final Loop callback released the adopted plan.
    playback.discardPending();
    REQUIRE(playback.pinnedBytes() == 0);
}

void gridCueCases()
{
    using State = LoopCueScheduler::State;
    constexpr int64_t beatLength = 24000;
    constexpr int64_t loopLength = 4 * beatLength;
    for (const bool playlist : {true,false})
    {
        LoopPlayback playback;
        auto publication = std::make_unique<LoopPlaybackPublication>();
        publication->captureGeneration = publication->sourceRevision = 1;
        publication->rate = 48000;
        publication->timing = {120,4,4,4,0,loopLength};
        publication->preparation = LoopPlaybackPublication::Preparation::ready;
        publication->plan.audio.entries = {testEntry(0,-16,80,.01f)};
        publication->gain = .25f;
        publication->gain = .75f;
        juce::String error;
        REQUIRE(publication->plan.prepare(loopLength,1024*1024,error));
        REQUIRE(publication->prepareLeadInPeaks([]{return false;}));
        REQUIRE(playback.publish(std::move(publication)));

        int64_t cursor = 0;
        float dryLevel = 0, lastRight = 0;
        const auto through = [&](int64_t until, uint16_t midi = 0, bool clockRunning = true,
                                 LoopPlayback::Access access = LoopPlayback::Access::perform, bool seek = false)
        {
            float sample = 0;
            while (cursor <= until)
            {
                std::array<float,128> left{},right{};
                left.fill(dryLevel);
                right.fill(-dryLevel * 0.5f);
                const auto frames = static_cast<int>(std::min<int64_t>(left.size(),until-cursor+1));
                MidiObservations events;
                for (int offset = 0; offset < frames; ++offset)
                {
                    const auto at = cursor + offset;
                    uint16_t roles = !playlist && clockRunning && at % beatLength == 0 ? captureClock : 0;
                    if (at == until) roles |= midi;
                    if (roles) events.samples[events.size++] = {offset,roles};
                }
                const SongTime grid{true,cursor,static_cast<double>(cursor)/beatLength,48000,120,4,4};
                { AudioAllocationGuard guard;
                    playback.process(left.data(),right.data(),frames,events,48000,1,1,
                        access,seek,{},playlist ? std::optional{grid} : std::nullopt); }
                seek = false;
                REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
                sample = left[frames-1];
                lastRight = right[frames-1];
                cursor += frames;
            }
            return sample;
        };

        REQUIRE(through(2*beatLength) == 0);
        REQUIRE(playback.status().phase == LoopLiveReference::State::ready);
        playback.requestCue();
        REQUIRE(through(loopLength-17) == 0 && playback.status().playback == State::queued);
        REQUIRE(std::abs(through(loopLength-16) - .0075f) < 1e-6f && playback.status().playhead == -16);
        REQUIRE(through(loopLength) > 0 && playback.status().playhead == 0);
        const auto display = playback.leadInDisplay();
        REQUIRE(display.start == loopLength - 16 && display.peaks && display.gain == .75f);
        REQUIRE(through(loopLength+1,finish) > 0 && playback.status().playback == State::finishing);
        REQUIRE(through(loopLength+2,finish) > 0 && playback.status().playback == State::finishing);
        REQUIRE(!playback.status().nextLeadInStart);
        REQUIRE(through(2*loopLength-16) == 0); // The next pass would have sounded here.
        REQUIRE(through(2*loopLength+240) == 0 && playback.status().playback == State::standby);

        REQUIRE(through(3*loopLength-10,cue) == 0); // Too late for the next downbeat.
        REQUIRE(through(3*loopLength) == 0 && playback.status().playback == State::queued);
        dryLevel = .9f;
        REQUIRE(through(4*loopLength-17) == dryLevel);
        REQUIRE(std::abs(through(4*loopLength-16) - (.9f + .0075f)) < 1e-6f && playback.status().playhead == -16);
        REQUIRE(std::abs(lastRight - (-.45f - .0075f)) < 1e-6f);
        playback.requestFinish();
        REQUIRE(through(4*loopLength-15) > 0 && playback.status().playback == State::finishing);
        playback.requestFinish();
        REQUIRE(through(4*loopLength-14) > dryLevel && playback.status().playback == State::stopping);
        REQUIRE(through(4*loopLength+240) == dryLevel && playback.status().playback == State::standby);
        dryLevel = 0;
        REQUIRE(through(4*loopLength+241,cue | finish) == 0 && playback.status().playback == State::standby);
        REQUIRE(through(4*loopLength+242,cue) == 0 && playback.status().playback == State::queued);
        REQUIRE(through(4*loopLength+243,finish) == 0 && playback.status().playback == State::standby);
        if (playlist)
        {
            const auto shifted = 5*loopLength + beatLength + 57;
            REQUIRE(through(shifted,phrase,true,LoopPlayback::Access::observe) == 0);
            REQUIRE(through(shifted+1,cue) == 0 && playback.status().playback == State::queued);
            REQUIRE(through(shifted+beatLength,phrase) == 0); // A queued launch keeps its origin.
            REQUIRE(through(shifted+loopLength-17) == 0);
            REQUIRE(through(shifted+loopLength-16) > 0 && playback.status().playhead == -16);
            REQUIRE(through(shifted+loopLength) > 0 && playback.status().playhead == 0);
            REQUIRE(through(shifted+loopLength+1,stop | finish | cue) > 0
                    && playback.status().playback == State::stopping);
            REQUIRE(through(shifted+loopLength+240) == 0 && playback.status().playback == State::standby);
            REQUIRE(through(shifted+loopLength+241,0,true,LoopPlayback::Access::perform,true) == 0);
            REQUIRE(through(shifted+loopLength+242,cue) == 0);
            REQUIRE(through(7*loopLength) > 0 && playback.status().playhead == 0);
        }
        else
        {
            playback.requestCue();
            through(5*loopLength,false,false);
            REQUIRE(playback.status().playback == State::standby);
            REQUIRE(playback.status().phase == LoopLiveReference::State::waitingForClock);
        }
    }
}

void offlineFadeCases()
{
    using State = LoopCueScheduler::State;
    constexpr int64_t beatFrames = 24000, loopFrames = 4 * beatFrames;
    constexpr float dryLeft = 1.1f, dryRight = -.55f, wetLevel = .0075f;
    for (const bool alreadyStopping : {false,true})
    {
        LoopPlayback playback;
        auto publication = std::make_unique<LoopPlaybackPublication>();
        publication->captureGeneration = publication->sourceRevision = 1;
        publication->rate = 48000;
        publication->timing = {120,4,4,4,0,loopFrames};
        publication->preparation = LoopPlaybackPublication::Preparation::ready;
        publication->plan.audio.entries = {testEntry(0,-16,80,.01f)};
        publication->gain = .75f;
        juce::String error;
        REQUIRE(publication->plan.prepare(loopFrames,1024*1024,error));
        REQUIRE(playback.publish(std::move(publication)));
        int64_t cursor = 0;
        const auto process = [&](int frames, LoopPlayback::Access access)
        {
            std::vector<float> left(frames,dryLeft),right(frames,dryRight);
            const SongTime grid{true,cursor,static_cast<double>(cursor)/beatFrames,48000,120,4,4};
            {
                AudioAllocationGuard guard;
                playback.process(left.data(),right.data(),frames,{},48000,1,1,access,false,{},grid);
            }
            REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
            cursor += frames;
            return std::pair{std::move(left),std::move(right)};
        };
        process(1,LoopPlayback::Access::perform);
        REQUIRE(playback.status().phase == LoopLiveReference::State::ready);
        playback.requestCue();
        const auto lead = process(static_cast<int>(loopFrames-16),LoopPlayback::Access::perform);
        REQUIRE(std::abs(lead.first.back() - (dryLeft + wetLevel)) < 1e-7f);
        REQUIRE(playback.status().playback == State::leadIn);
        REQUIRE(playback.peaks().output > 1 && playback.peaks().wet == wetLevel);
        const auto precedingFade = alreadyStopping ? 17 : 0;
        if (alreadyStopping)
        {
            playback.requestStop();
            process(precedingFade,LoopPlayback::Access::perform);
            REQUIRE(playback.status().playback == State::stopping);
        }
        const auto fade = process(256,LoopPlayback::Access::disabled);
        for (int n = 0; n < 256; ++n)
        {
            const auto weight = static_cast<float>(std::max(0,239-precedingFade-n))/240;
            const auto wet = wetLevel * weight;
            const auto rawLeft = dryLeft + wet,rawRight = dryRight - wet;
            REQUIRE(std::abs(fade.first[n]-rawLeft) < 1e-7f);
            REQUIRE(std::abs(fade.second[n]-rawRight) < 1e-7f);
        }
        REQUIRE(fade.first.back() == dryLeft && fade.second.back() == dryRight);
        REQUIRE(playback.status().playback == State::standby);
    }
}

void rateChangeFadeCases()
{
    using PlaybackState = LoopCueScheduler::State;
    for (const auto alreadyStopping : {false,true})
    {
        constexpr int64_t period = 19200;
        LoopPlayback playback;
        auto publication = std::make_unique<LoopPlaybackPublication>();
        publication->captureGeneration = publication->sourceRevision = 1;
        publication->rate = 96000;
        publication->timing = {300,4,4,1,0,period};
        publication->preparation = LoopPlaybackPublication::Preparation::ready;
        publication->plan.audio.entries = {testEntry(0,0,1024,.01f,96000)};
        juce::String error;
        REQUIRE(publication->plan.prepare(period,1024*1024,error));
        REQUIRE(playback.publish(std::move(publication)));
        playback.setReference({{96000,300},1,4,4,true},true);
        std::vector<float> left(3 * period + 1),right(left.size());
        MidiObservations events;
        events.samples[events.size++] = {0,phrase};
        events.samples[events.size++] = {period,phrase};
        events.samples[events.size++] = {2 * period,phrase | cue};
        events.samples[events.size++] = {3 * period,phrase};
        { AudioAllocationGuard guard; playback.process(left.data(),right.data(),static_cast<int>(left.size()),events,96000,1,1,LoopPlayback::Access::perform,false); }
        REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
        REQUIRE(playback.status().playback == PlaybackState::repeating && left.back() > 0);
        auto last = left.back();
        std::array<float,241> fadeLeft{},fadeRight{};
        if (alreadyStopping)
        {
            playback.requestStop();
            { AudioAllocationGuard guard; playback.process(fadeLeft.data(),fadeRight.data(),64,{},96000,1,1,LoopPlayback::Access::perform,false); }
            REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
            REQUIRE(playback.status().playback == PlaybackState::stopping && fadeLeft[63] > 0);
            last = fadeLeft[63];
            fadeLeft.fill(0);
            fadeRight.fill(0);
        }
        { AudioAllocationGuard guard; playback.process(fadeLeft.data(),fadeRight.data(),241,{},48000,1,1,LoopPlayback::Access::perform,false); }
        REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
        const auto end = alreadyStopping ? 208 : 240;
        REQUIRE(fadeLeft.front() <= last && fadeLeft[end-2] > 0 && fadeLeft[end-1] == 0 && fadeLeft.back() == 0);
        REQUIRE(playback.status().playback == PlaybackState::standby);
    }
}

void measuredReplay(const char* path, double bpm)
{
    struct Event { int64_t sample; unsigned flags; };
    constexpr unsigned beatBit=1, barBit=2, phraseBit=4, cueBit=8, stopBit=16, resetBit=32, dryBit=64;
    const int block = bpm == 137 ? 128 : 256;
    constexpr int64_t replayEnd=4512512, lead=2048;
    const auto period = 48000.L*60*16/bpm;
    const auto capturedFrames = static_cast<int64_t>(std::floor(period));
    std::ifstream input(path);
    REQUIRE(input.good());
    std::vector<Event> events;
    std::vector<int64_t> phrases, cues, resets, stops;
    Event event{};
    while (input >> event.sample >> event.flags)
    {
        REQUIRE(events.empty() || event.sample > events.back().sample);
        if (event.flags & phraseBit) phrases.push_back(event.sample);
        if (event.flags & cueBit) cues.push_back(event.sample);
        if (event.flags & resetBit) resets.push_back(event.sample);
        if (event.flags & stopBit) stops.push_back(event.sample);
        REQUIRE((event.flags & phraseBit) == 0 || (event.flags & dryBit) != 0);
        events.push_back(event);
    }
    REQUIRE(input.eof() && phrases.size()>12 && resets.size()==1 && stops.size()==1 && cues.size()==4);
    const auto dropped = *std::lower_bound(phrases.begin(),phrases.end(),3000000);
    const auto dropPosition = std::lower_bound(phrases.begin(),phrases.end(),dropped);
    const auto resetOrigin = std::upper_bound(phrases.begin(),phrases.end(),resets[0]);
    const auto distance = [&](int64_t count){ return 48000.L*60*16*count/bpm; };
    const auto deadline = std::min(*resetOrigin + static_cast<int64_t>(std::floor(distance(dropPosition-resetOrigin)+1)),
        *(dropPosition-1)+static_cast<int64_t>(std::ceil(period))) + 1;

    LoopPlayback playback;
    auto publication = std::make_unique<LoopPlaybackPublication>();
    publication->captureGeneration = publication->sourceRevision = 1;
    publication->rate = 48000;
    publication->timing = {bpm,4,4,16,1234,capturedFrames};
    publication->preparation = LoopPlaybackPublication::Preparation::ready;
    publication->plan.audio.entries = {testEntry(0,-lead,lead+capturedFrames+512,.002f),
                                       testEntry(1,-333,capturedFrames+256,.001f)};
    const auto entries = publication->plan.audio.entries;
    juce::String error;
    REQUIRE(publication->plan.prepare(capturedFrames,8*1024*1024,error));
    REQUIRE(playback.publish(std::move(publication)));
    playback.setReference({{48000,bpm},16,4,4,true},true);

    // Draw each source interval independently into a reference buffer. The
    // oracle uses the raw Rack markers and never calls the scheduler/compositor.
    std::vector<float> expected(replayEnd,0);
    size_t launches=0, downbeats=0;
    const std::array<int64_t,4> breaks{-1,resets[0],deadline,stops[0]};
    for (size_t epoch = 0; epoch+1 < breaks.size(); ++epoch)
    {
        const auto origin = std::upper_bound(phrases.begin(),phrases.end(),breaks[epoch]);
        const auto stopAt = breaks[epoch+1];
        if (std::distance(origin,phrases.end())<3 || origin[2]>=stopAt) continue;
        const auto cueAt = std::lower_bound(cues.begin(),cues.end(),origin[2]);
        if (cueAt==cues.end() || *cueAt>=stopAt) continue;
        const auto epochEarliest = [&](int64_t ordinal){ return *origin + static_cast<int64_t>(std::ceil(distance(ordinal)-1)); };
        const auto lastBeforeCue = std::upper_bound(origin,phrases.end(),*cueAt) - 1;
        const auto launchEarliest = [&](auto anchor){
            return std::max(epochEarliest(anchor-origin),*lastBeforeCue + (anchor-lastBeforeCue)*capturedFrames);
        };
        auto first = std::upper_bound(origin,phrases.end(),*cueAt);
        while (first!=phrases.end() && launchEarliest(first)-lead<*cueAt) ++first;
        if (first==phrases.end() || launchEarliest(first)-lead>=stopAt) continue;
        ++launches;
        for (auto anchor=first; anchor!=phrases.end(); ++anchor)
        {
            const auto early = anchor==first ? launchEarliest(anchor) : epochEarliest(anchor-origin);
            if (early-lead>=stopAt) break;
            const auto observed = *anchor != dropped;
            if (*anchor<stopAt && observed) ++downbeats;
            for (const auto& entry : entries)
            {
                for (auto local=entry.start; local<entry.end; ++local)
                {
                    const auto destination = (local<0 ? early : *anchor)+local;
                    if (destination>=0 && destination<stopAt && (local<0 || observed))
                        expected[destination] += entry.audio->samples[static_cast<size_t>(local-entry.start)*2];
                }
                if (entry.start<=-1 && entry.end> -1)
                    for (auto hold=early; (!observed || hold<*anchor) && hold<stopAt; ++hold)
                        expected[hold] += entry.audio->samples[static_cast<size_t>(-1-entry.start)*2];
            }
        }
        for (int n=0; n<240 && stopAt+n<replayEnd; ++n)
            expected[stopAt+n]=expected[stopAt-1]*static_cast<float>(239-n)/240.f;
    }
    REQUIRE(launches>=2 && downbeats>=2);
    size_t index=0;
    float maximumError=0;
    for (int64_t start=0; start<replayEnd; start+=block)
    {
        const auto frames=static_cast<int>(std::min<int64_t>(block,replayEnd-start));
        std::array<float,256> left,right,dry;
        left.fill(.125f);right.fill(.125f);dry.fill(.125f);
        MidiObservations blockEvents;
        while (index<events.size() && events[index].sample<start+frames)
        {
            const auto [sample,flags]=events[index++];
            const auto offset=static_cast<int>(sample-start);
            uint16_t roles=0;
            if (flags&beatBit) roles|=beat;
            if (flags&barBit) roles|=bar;
            if ((flags&phraseBit) && sample!=dropped) roles|=phrase;
            if (flags&cueBit) roles|=cue;
            if (flags&stopBit) roles|=stop;
            if (flags&resetBit) roles|=reset;
            if (flags&dryBit) dry[offset]=left[offset]=.375f;
            if (roles) blockEvents.samples[blockEvents.size++]={offset,roles};
        }
        {AudioAllocationGuard guard; playback.process(left.data(),right.data(),frames,blockEvents,48000,1,1,LoopPlayback::Access::perform,false);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        for (int n=0;n<frames;++n)
        {
            const auto wet=expected[start+n];
            maximumError=std::max({maximumError,std::abs(left[n]-(dry[n]+wet)),std::abs(right[n]-(.125f-wet))});
            if (maximumError>=1e-6f)
                throw std::runtime_error("Rack replay mismatch at sample "+std::to_string(start+n)
                    +" expected "+std::to_string(wet)+" actual "+std::to_string(left[n]-dry[n]));
        }
    }
    std::cout << bpm << " BPM, " << launches << " launches, " << downbeats << " aligned downbeats, peak error " << maximumError << '\n';
}

void flStudioScheduleCases()
{
    using Route=LoopPlayback::Route;
    constexpr int64_t period=96000;
    constexpr int64_t placement=10;
    const auto entry=testEntry(0,-4,period+8,.002f);
    const auto create=[&]{
        auto playback=std::make_unique<LoopPlayback>();
        auto publication=std::make_unique<LoopPlaybackPublication>();
        publication->captureGeneration=publication->sourceRevision=1;
        publication->rate=48000;
        publication->timing={120,4,4,4,0,period};
        publication->preparation=LoopPlaybackPublication::Preparation::ready;
        publication->plan.audio.entries={entry};
        juce::String error;
        REQUIRE(publication->plan.prepare(period,8*1024*1024,error));
        REQUIRE(playback->publish(std::move(publication)));
        return playback;
    };
    const auto expected=[&](int64_t position){
        return position >= entry.start && position < entry.end
            ? entry.audio->samples[static_cast<size_t>(position-entry.start)*2] : 0.f;
    };
    const auto block=[&](LoopPlayback& playback,int64_t hostSample,int frames,LoopPlayback::HostSchedule schedule,
                          const auto& oracle){
        std::array<float,256> left{},right{};
        const SongTime time{true,hostSample,static_cast<double>(hostSample)/24000.0,48000,120,4,4};
        {AudioAllocationGuard guard;
            playback.process(left.data(),right.data(),frames,{},48000,1,1,LoopPlayback::Access::perform,
                             false,{},time,schedule,true);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        for(int i=0;i<frames;++i) REQUIRE(std::abs(left[i]-oracle(hostSample+i))<1e-6f);
    };

    auto placed=create();
    for(int64_t pos=0;pos<3*period+32;)
    {
        const auto frames=static_cast<int>(std::min<int64_t>(256,3*period+32-pos));
        block(*placed,pos,frames,{Route::placement,placement,0},[&](int64_t sample){
            const auto local=sample-placement;
            const auto cycle=std::max<int64_t>(0,local/period);
            const auto phase=local-cycle*period;
            return (cycle>0?expected(phase+period):0.f)+expected(phase)+expected(phase-period);
        });
        pos+=frames;
    }
    REQUIRE(placed->status().playback==LoopCueScheduler::State::repeating);

    auto repeating=create();
    const LoopPlayback::HostSchedule repeat{Route::hostRepeat,0,period};
    const auto cyclic=[&](int64_t sample){
        return expected(sample+period)+expected(sample)+expected(sample-period);
    };
    block(*repeating,period-12,12,repeat,cyclic);
    block(*repeating,0,32,repeat,cyclic);
    REQUIRE(repeating->status().route==Route::hostRepeat && repeating->status().playhead==31);
}
}

int main(int argc,char** argv)
{
    return runTest([&]{
        if (argc == 3) measuredReplay(argv[1],std::stod(argv[2]));
        else {liveReferenceCases(); automaticShortPhraseCases(); schedulerCases(); earliestCueCase(); compositorCases(); fixedGridPeakCases(); publicationCases(); gridCueCases(); offlineFadeCases(); rateChangeFadeCases(); flStudioScheduleCases();}
    });
}
