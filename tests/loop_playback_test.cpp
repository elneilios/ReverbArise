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

void schedulerCases()
{
    using State = LoopCueScheduler::State;
    using Phase = LoopLiveReference::State;
    LoopCueScheduler scheduler;
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

void publicationCases()
{
    using Prep = LoopPlaybackPublication::Preparation;
    using PlaybackState = LoopCueScheduler::State;
    LoopPlayback playback;
    const auto publish = [&](float level, uint64_t revision = 1){
        auto next = std::make_unique<LoopPlaybackPublication>();
        next->captureGeneration = 1;
        next->sourceRevision = revision;
        next->rate = 8000;
        next->timing = {300,4,4,4,123,6400};
        next->dependencies = playback.dependencies();
        next->preparation = Prep::ready;
        next->plan.audio.entries.push_back(testEntry(0,-16,80,level,8000));
        juce::String error;
        REQUIRE(next->plan.prepare(6400,1024*1024,error));
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
            { AudioAllocationGuard guard; playback.process(left.data(),right.data(),count,events,8000,1,source,true,false); }
            REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
            for (int n = 0; n < count; ++n) REQUIRE(std::abs(left[n]+right[n]-.3f)<1e-6f);
            sample = left[count-1];
            position += count;
        }
        return sample;
    };
    REQUIRE(advance(12801,12801,cue)==.1f); // External commands are disabled.
    REQUIRE(playback.status().phase == LoopLiveReference::State::ready);
    playback.requestCue();
    REQUIRE(std::abs(advance(19184)-.11f)<1e-6f);
    publish(.05f); // A compatible successor waits while the old revision is pinned.
    REQUIRE(std::abs(advance(19200)-.13f)<1e-6f);
    REQUIRE(playback.status().playback == PlaybackState::repeating);
    const auto both = playback.pinnedBytes();
    playback.invalidatePrepared();
    REQUIRE(advance(19250)==.1f && playback.status().playback == PlaybackState::standby);
    playback.collect();
    REQUIRE(playback.pinnedBytes() < both && playback.status().preparation == Prep::waiting);
    publish(.05f,2);
    REQUIRE(advance(25600,-1,0,2)==.1f);
    REQUIRE(playback.status().playback == PlaybackState::standby);
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
        const auto firstPossible = [&](int64_t ordinal){ return *origin + static_cast<int64_t>(std::ceil(distance(ordinal)-1)); };
        auto first = std::upper_bound(origin,phrases.end(),*cueAt);
        while (first!=phrases.end() && firstPossible(first-origin)-lead<*cueAt) ++first;
        if (first==phrases.end() || firstPossible(first-origin)-lead>=stopAt) continue;
        ++launches;
        for (auto anchor=first; anchor!=phrases.end(); ++anchor)
        {
            const auto early = firstPossible(anchor-origin);
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
        {AudioAllocationGuard guard; playback.process(left.data(),right.data(),frames,blockEvents,48000,1,1,true,false);}
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
}

int main(int argc,char** argv)
{
    return runTest([&]{
        if (argc == 3) measuredReplay(argv[1],std::stod(argv[2]));
        else {liveReferenceCases(); schedulerCases(); compositorCases(); publicationCases();}
    });
}
