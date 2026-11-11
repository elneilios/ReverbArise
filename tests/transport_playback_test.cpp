#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Playback/TransportPlayback.h>
#include <thread>
#include <string_view>
using namespace m3;
static std::unique_ptr<LivePublication> fixture(int64_t first=-20,int64_t last=180)
{
    auto p=std::make_unique<LivePublication>();p->mapping={true,0,0,1000,120,4,4};p->start=first;p->end=last;
    for(size_t n=0;n<2;++n){auto a=std::make_shared<WetRender>();a->start=-10+static_cast<int64_t>(n)*40;a->rate=1000;a->samples.resize(240);
        for(size_t i=0;i<120;++i){a->samples[i*2]=static_cast<float>(i+1)*.0005f;a->samples[i*2+1]=-.02f;}
        p->entries.push_back({n,1,a->start,a->start+120,.08f,a});}
    juce::String error;REQUIRE(prepareLiveSegments(*p,error));return p;
}

static void verifySongOutputSum()
{
    TransportPlayback live, offline;
    for (auto* player : {&live, &offline})
    {
        player->invalidate(0, 1);
        player->invalidate(1, 1);
        REQUIRE(player->publish(fixture()));
    }
    offline.primeForOffline();
    using Frame = std::array<float, 2>;
    const auto render = [&](int64_t position, Frame input, bool playing = true)
    {
        TransportTime time{{true,position,position / 500.0,1000,120,4,4},playing};
        auto guarded = input, unguarded = input;
        {
            AudioAllocationGuard allocations;
            live.process(&guarded[0], &guarded[1], 1, time);
            offline.process(&unguarded[0], &unguarded[1], 1, time);
        }
        REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
        return std::pair{guarded, unguarded};
    };
    for (int position = 0; position < 6; ++position)
    {
        const auto [liveFrame, offlineFrame] = render(position, {0.4f, -0.2f});
        REQUIRE(liveFrame == offlineFrame && liveFrame[0] > 0.4f);
    }
    const Frame hotDry{1.1f, -0.55f};
    const auto [peak, offlinePeak] = render(6, hotDry);
    REQUIRE(peak == offlinePeak && peak[0] > hotDry[0]);
    REQUIRE(live.peaks().output == peak[0] && live.peaks().wet > 0);
    const auto [afterSeek, offlineAfterSeek] = render(50, {0.4f, -0.2f});
    REQUIRE(afterSeek == offlineAfterSeek);
    const auto [fading, offlineFade] = render(51, hotDry, false);
    REQUIRE(fading == offlineFade && fading[0] > hotDry[0]);
    for (int position = 52; position <= 56; ++position)
        render(position, hotDry, false);
    REQUIRE(render(57, hotDry, false).first == hotDry);
    live.configure(true, LiveRouting::wetOnly, 0);
    offline.configure(true, LiveRouting::wetOnly, 0);
    for (int position = 58; position < 69; ++position)
    {
        const auto [liveFrame, offlineFrame] = render(position, hotDry);
        REQUIRE(liveFrame == offlineFrame);
    }
    live.configure(true, LiveRouting::dryWet, 0);
    offline.configure(true, LiveRouting::dryWet, 0);
    for (int position = 69; position < 75; ++position)
    {
        const auto [liveFrame, offlineFrame] = render(position, {1.2f, -0.6f});
        REQUIRE(liveFrame == offlineFrame);
    }
}

static void verifyDisjointSwellGain()
{
    const auto render = [](float firstGain)
    {
        auto publication = std::make_unique<LivePublication>();
        publication->mapping = {true,0,0,1000,120,4,4};
        publication->end = 128;
        for (size_t index = 0; index < 2; ++index)
        {
            auto audio = std::make_shared<WetRender>();
            audio->rate = 1000;
            audio->start = static_cast<int64_t>(index) * 64;
            audio->peak = index == 0 ? .5f * firstGain : .25f;
            audio->samples.assign(64,audio->peak);
            publication->entries.push_back({index,1,audio->start,audio->start+32,audio->peak,audio});
        }
        juce::String error;
        REQUIRE(prepareLiveSegments(*publication,error));
        TransportPlayback playback;
        playback.invalidate(0,1);
        playback.invalidate(1,1);
        REQUIRE(playback.publish(std::move(publication)));
        std::array<float,128> left{}, right{};
        playback.process(left.data(),right.data(),128,{{true,0,0,1000,120,4,4},true});
        return left;
    };
    const auto boost = juce::Decibels::decibelsToGain(12.0f);
    const auto initial = render(1), boosted = render(boost);
    REQUIRE(boosted[16] > 1 && boosted[16] == initial[16] * boost);
    REQUIRE(std::equal(initial.begin()+64,initial.end(),boosted.begin()+64));
}

int main(int argc,char* argv[]){return runTest([&]{
    verifySongOutputSum();
    verifyDisjointSwellGain();
    {LiveTransportClock first,second;TransportTime at{{true,16,0,48000,122,4,4},true};
        REQUIRE(!first.snapshot(3));REQUIRE(!gridClockContext(first.snapshot(3),true).available);
        first.observe(at,73,3);const auto original=gridClockContext(first.snapshot(3),true);
        REQUIRE(original.available && original.admitted && original.session==3 && original.run==1);
        REQUIRE(!gridClockContext(first.snapshot(4),true).available && !second.snapshot(3));
        at.song.sample+=24000;at.song.ppq+=61./60;first.observe(at,97,3);
        REQUIRE(original.sameAuthority(gridClockContext(first.snapshot(3),true)));
        REQUIRE(!gridClockContext(first.snapshot(3),false).available);
        at.playing=false;first.observe(at,21,3);REQUIRE(!original.sameAuthority(gridClockContext(first.snapshot(3),false)));
        at.playing=true;first.observe(at,55,3);REQUIRE(first.snapshot(3)->run==2);
        REQUIRE(!original.sameAuthority(gridClockContext(first.snapshot(3),true)));
        first.reset();REQUIRE(!first.snapshot(3));first.observe(at,55,4);REQUIRE(!first.snapshot(3) && first.snapshot(4)->run==1);
    }
    // #73: these are injected sequences, not a reconstruction of unrecorded FL callbacks.
    for(bool stablePrefix:{false,true}){
        LiveTransportClock clock;auto player=std::make_unique<TransportPlayback>();
        const SongTime source{true,3399344,144,48000,122,4,4};
        auto pub=std::make_unique<LivePublication>();pub->mapping=source;pub->revision=73;
        pub->start=3399344;pub->end=3796300;
        auto audio=std::make_shared<WetRender>();audio->start=pub->start;audio->rate=48000;
        audio->samples.resize(static_cast<size_t>(pub->end-pub->start)*2,.03125f);
        pub->entries.push_back({0,1,pub->start,pub->end,.03125f,audio});
        juce::String error;REQUIRE(prepareLiveSegments(*pub,error));player->invalidate(0,1);REQUIRE(player->publish(std::move(pub)));
        float l[245]{},r[245]{};TransportTime at{source,true};at.song.ppq=143.999322222222219;
        const auto process=[&]{const auto current=clock.observe(at,245,73);
            {AudioAllocationGuard guard;player->process(l,r,245,at,true,&current);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);};
        process();REQUIRE(player->observation()->gate==LiveGate::admitted && l[244]>.03f);
        const auto firstBlock=player->observation()->block;
        if(stablePrefix){at.song.sample+=245;at.song.ppq+=245*122.0/(48000*60);process();}
        const auto previous=player->observation()->block;
        at.song={true,3795967,160.801390972220531,48000,122,4,4};process();
        auto causal=player->causalObservation();REQUIRE(causal && causal->hasStart && causal->hasRejection && causal->hasPreceding);
        REQUIRE(causal->runStart.block==firstBlock && causal->precedingAdmitted.block==previous);
        REQUIRE(causal->firstRejection.gate==LiveGate::origin && causal->firstRejection.clockGate==LiveGate::origin);
        REQUIRE(std::abs(causal->firstRejection.originDelta)<.00001);
        long double a,b;REQUIRE(songOrigin(causal->firstRejection.liveReference,a) && songOrigin(causal->firstRejection.observed,b));
        REQUIRE(std::abs(static_cast<double>(b-a)+16)<.00001);
        const auto rejectedBlock=causal->firstRejection.block;
        at.looping=true;at.loopStart=3399344;at.loopEnd=3796300;process();
        REQUIRE(player->observation()->hostLooping && player->observation()->hostLoopStart==3399344 && !player->causalObservation()->firstRejection.hostLooping);
        REQUIRE(player->lastRejection()->block>rejectedBlock && player->causalObservation()->firstRejection.block==rejectedBlock);
        at.song.valid=false;process();REQUIRE(player->causalObservation()->firstRejection.block==rejectedBlock);
        at.song.valid=true;at.playing=false;process();REQUIRE(player->causalObservation()->firstRejection.block==rejectedBlock);
        at.playing=true;process();causal=player->causalObservation();REQUIRE(causal && causal->hasStart && !causal->hasRejection && causal->runStart.liveRun==2);
        clock.reset();process();REQUIRE(!player->causalObservation()->hasRejection);
        const auto fresh=clock.observe(at,245,74);player->process(l,r,245,at,true,&fresh);
        REQUIRE(player->causalObservation()->runStart.liveSession==74);
    }
    { // No admitted publication before a clock failure: never fabricate a predecessor.
        LiveTransportClock clock;auto player=std::make_unique<TransportPlayback>();
        TransportTime at{{true,0,0,1000,120,4,4},true};float l[16]{},r[16]{};
        auto current=clock.observe(at,16,1);player->process(l,r,16,at,true,&current);
        at.song.sample=18;current=clock.observe(at,16,1);player->process(l,r,16,at,true,&current);
        const auto causal=player->causalObservation();REQUIRE(causal && causal->hasRejection && !causal->hasPreceding);
        REQUIRE(causal->firstRejection.gate==LiveGate::noPublication && causal->firstRejection.clockGate==LiveGate::origin);
        LiveCausalHistory other;LiveCallbackObservation d;REQUIRE(!other.observe(d).hasStart);
        std::cout<<"#73 causal payload bytes="<<sizeof(LiveCausalObservation)<<" atomic words="<<(sizeof(LiveCausalObservation)+7)/8<<"\n";
    }
    { // Independent two-channel sample oracle, including the five-sample joins/tails.
        auto realtime=std::make_unique<TransportPlayback>(),offline=std::make_unique<TransportPlayback>();
        for(auto* player:{realtime.get(),offline.get()}){player->invalidate(0,1);player->invalidate(1,1);REQUIRE(player->publish(fixture()));}
        offline->primeForOffline();LiveTransportClock clock;
        std::array<float,200> liveL{},liveR{},offlineL{},offlineR{};
        TransportTime at{{true,-20,-.04,1000,120,4,4},true};
        offline->process(offlineL.data(),offlineR.data(),200,at);
        int offset=0;for(int stride:{1,7,13,31,64,84}){at.song.sample=-20+offset;at.song.ppq=at.song.sample/500.0;
            const auto current=clock.observe(at,stride,73);
            {AudioAllocationGuard guard;realtime->process(liveL.data()+offset,liveR.data()+offset,stride,at,true,&current);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);offset+=stride;}
        REQUIRE(offset==200);
        for(int n=0;n<200;++n){double expectedL=0,expectedR=0;
            for(int start:{-10,30}){const auto age=n-20-start;if(age<0 || age>=125)continue;
                const auto blend=age<120?std::min(1.0,(age+1)/5.0):(124-age)/5.0;
                expectedL+=(age<120?(age+1)*.0005:.06)*blend;expectedR+=-.02*blend;}
            REQUIRE(std::abs(liveL[n]-expectedL)<1e-7 && std::abs(liveR[n]-expectedR)<1e-7);
            REQUIRE(liveL[n]==offlineL[n] && liveR[n]==offlineR[n]);}
        REQUIRE(liveL[50]>0 && liveR[50]<0);
    }
    // Exact failing offline tuple from the owner's 2026-11-05 diagnostic.
    {const SongTime prepared{true,3399344,144,48000,122,4,4};
        const SongTime host{true,3399344,143.999322222222219,48000,122,4,4};
        REQUIRE(!transportMappingMatches(prepared,host)); // Global guard remains strict.
        LiveClockSnapshot clock;clock.reference=clock.observed=host;clock.playing=true;clock.gate=LiveGate::admitted;
        REQUIRE(clock.accepts(prepared,host)==LiveGate::admitted);
        auto pub=std::make_unique<LivePublication>();pub->mapping=prepared;pub->start=host.sample;pub->end=host.sample+600;
        auto audio=std::make_shared<WetRender>();audio->start=host.sample;audio->rate=48000;audio->samples.resize(1200,.07f);
        pub->entries.push_back({0,1,host.sample,host.sample+600,.07f,audio});juce::String error;REQUIRE(prepareLiveSegments(*pub,error));
        TransportPlayback player;player.invalidate(0,1);REQUIRE(player.publish(std::move(pub)));
        float l[300]{},r[300]{};TransportTime time{host,true};player.process(l,r,300,time,true,&clock);
        REQUIRE(!player.incompatible() && std::abs(l[299]-.07f)<1e-6f);
        time.song.sample+=300;time.song.ppq+=300*122./(60*48000);time.song.ppq-=2*122./(60*48000);
        player.process(l,r,300,time,true,&clock);REQUIRE(player.incompatible());
        REQUIRE(clock.accepts(prepared,time.song)==LiveGate::origin);
    }
    // Owner recording 2026-11-04: the ready stopped mapping differs from
    // every playing observation by 16 samples. PCM here is synthetic; timing
    // and publication identity are replayed from the host diagnostic evidence.
    {auto replay=std::make_unique<TransportPlayback>();LiveTransportClock clock;
        const SongTime stopped{true,3399344,144,48000,122,4,4};
        const SongTime playing{true,3436475,145.572232638889488,48000,122,4,4};
        auto publication=std::make_unique<LivePublication>();publication->mapping=stopped;
        publication->revision=30;publication->start=3387344;publication->end=3495344;
        auto audio=std::make_shared<WetRender>();audio->start=playing.sample;audio->rate=48000;
        audio->samples.resize(245*2,.07f);
        publication->entries.push_back({0,1,playing.sample,playing.sample+245,.07f,audio});
        juce::String error;REQUIRE(prepareLiveSegments(*publication,error));
        replay->invalidate(0,1);REQUIRE(replay->publish(std::move(publication)));
        float left[245],right[245];std::fill_n(left,245,.25f);std::fill_n(right,245,.25f);
        TransportTime time{playing,true};
        {AudioAllocationGuard guard;const auto current=clock.observe(time,245,1);replay->process(left,right,245,time,true,&current);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        if(argc>1 && std::string_view(argv[1])=="--issue-53-expected-wet"){
            // Exact owner timing must now admit resident synthetic wet without
            // changing any stored audio/sample coordinate.
            REQUIRE(left[244]>.25f);return;
        }
        const auto admitted=replay->observation();REQUIRE(admitted);
        REQUIRE(admitted->gate==LiveGate::admitted && admitted->revision==30 && admitted->validCount);
        REQUIRE(admitted->usesLiveClock && admitted->liveRun==1 && admitted->liveReference.sample==playing.sample);
        REQUIRE(std::abs(admitted->originDelta-16)<.00001); // Informational source delta.
        REQUIRE(!replay->incompatible() && replay->active() && left[0]>.25f && std::abs(left[244]-.32f)<1e-6f);
        time={stopped,false};auto current=clock.observe(time,100,1);replay->process(left,right,100,time,true,&current);
        REQUIRE(!replay->incompatible() && replay->observation()->gate==LiveGate::stopped);
        REQUIRE(!replay->lastRejection());
        time={playing,true};current=clock.observe(time,245,1);replay->process(left,right,245,time,true,&current);
        REQUIRE(!replay->incompatible() && replay->observation()->revision==30 && replay->observation()->liveRun==2);
    }

    // A continuous run cannot be silently rebased by a seek, invalid timing,
    // output gating or revision changes. Confirmed Stop and session reset can.
    {LiveTransportClock clock;TransportTime at{{true,16,0,1000,120,4,4},true};
        auto current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::admitted && current.run==1);
        at.song.sample=516;at.song.ppq=1;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::admitted);
        at.song.sample=18;at.song.ppq=0;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::origin && current.reference.sample==16);
        at.song.valid=false;at.playing=false;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::invalidTiming);
        at.song.valid=true;at.playing=true;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::origin && current.run==1);
        at.playing=false;current=clock.observe(at,0,1);REQUIRE(current.gate==LiveGate::invalidRange);
        at.playing=true;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::origin);
        at.song.sample=16;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::admitted);
        at.playing=false;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::stopped);
        at.playing=true;at.song.sample=64;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::admitted && current.run==2);
        at.song.bpm=121;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::tempo);
        at.song.bpm=120;at.song.rate=44100;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::rate);
        at.song.rate=1000;at.song.numerator=3;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::meter);
        at.song.numerator=4;at.song.sample=INT64_MAX-2;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::invalidRange);
        clock.reset();REQUIRE(!clock.snapshot(1));at.song.sample=32;current=clock.observe(at,20,1);REQUIRE(current.gate==LiveGate::admitted);
        REQUIRE(!clock.snapshot(2));current=clock.observe(at,20,2);REQUIRE(current.session==2 && current.gate==LiveGate::admitted);
        auto player=std::make_unique<TransportPlayback>();player->invalidate(0,1);player->invalidate(1,1);REQUIRE(player->publish(fixture()));
        float l[20]{},r[20]{};at.song.sample=60;at.song.ppq=.12;clock.reset();current=clock.observe(at,20,2);
        player->process(l,r,20,at,true,&current);REQUIRE(!player->incompatible() && std::abs(l[19]-.07f)<1e-6f);
        at.song.sample+=8;current=clock.observe(at,20,2);
        player->configure(false,LiveRouting::dryWet,0);player->process(l,r,20,at,true,&current);
        player->configure(true,LiveRouting::dryWet,0);player->process(l,r,20,at,false,&current);
        player->process(l,r,20,at,true,&current);REQUIRE(player->incompatible() && player->observation()->gate==LiveGate::origin);
        for(int dimension=0;dimension<3;++dimension){
            LiveTransportClock fresh;TransportTime incompatible{{true,60,.12,1000,120,4,4},true};
            if(dimension==0)incompatible.song.rate=44100;if(dimension==1)incompatible.song.bpm=121;if(dimension==2)incompatible.song.numerator=3;
            const auto validated=fresh.observe(incompatible,20,1);REQUIRE(validated.gate==LiveGate::admitted);
            player->process(l,r,20,incompatible,true,&validated);
            const auto expected=dimension==0?LiveGate::rate:dimension==1?LiveGate::tempo:LiveGate::meter;
            REQUIRE(player->incompatible() && player->observation()->gate==expected);
        }
    }
    {LiveTransportClock clock;TransportTime at{{true,0,0,1000,120,4,4},true};clock.observe(at,16,1);
        std::atomic<bool> done{false},coherent{true};std::thread reader([&]{while(!done.load())if(const auto d=clock.snapshot(1))
            if(d->gate!=LiveGate::admitted || d->observed.ppq!=d->observed.sample/500.0 || d->reference.sample!=0)coherent=false;});
        for(int n=0;n<2000;++n){at.song.sample=n;at.song.ppq=n/500.0;
            {AudioAllocationGuard guard;clock.observe(at,16,1);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);}
        done=true;reader.join();REQUIRE(coherent.load());}

    // Telemetry owns values, not publication pointers; Stop preserves rejection.
    {auto diagnostic=std::make_unique<TransportPlayback>();
        REQUIRE(!diagnostic->observation() && !diagnostic->lastRejection());
        auto publication=fixture();publication->revision=42;REQUIRE(diagnostic->publish(std::move(publication)));
        float l[20]{},r[20]{};TransportTime at{{true,60,.12,1000,120,4,4},true};
        const auto check=[&](LiveGate gate,bool allowed=true){
            {AudioAllocationGuard guard;diagnostic->process(l,r,20,at,allowed);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
            const auto d=diagnostic->observation();REQUIRE(d && d->gate==gate && d->revision==42);
            REQUIRE(d->observed.sample==at.song.sample && d->observed.ppq==at.song.ppq);
        };
        check(LiveGate::missingCoverage); // Fixture members deliberately not admitted.
        at.song.ppq=.1;check(LiveGate::origin);
        const auto rejected=diagnostic->lastRejection();REQUIRE(rejected && rejected->originsValid);
        REQUIRE(std::abs(rejected->originDelta-10)<1e-12 && rejected->validCount);
        at.playing=false;check(LiveGate::stopped);
        REQUIRE(diagnostic->lastRejection()->block==rejected->block && !diagnostic->incompatible());
        at.playing=true;at.song= {true,60,.12,1000,121,4,4};check(LiveGate::tempo);
        at.song.bpm=120;at.song.rate=999;check(LiveGate::rate);
        at.song.rate=1000;at.song.numerator=3;check(LiveGate::meter);
        at.song.numerator=4;at.song.valid=false;check(LiveGate::invalidTiming);
        at.song.valid=true;check(LiveGate::workflow,false);
        diagnostic->configure(false,LiveRouting::wetOnly,-6);check(LiveGate::disabled);
        diagnostic->configure(true,LiveRouting::dryWet,0);
        at.song.sample=INT64_MAX-3;check(LiveGate::invalidRange);
        // Retirement cannot invalidate already copied diagnostics.
        REQUIRE(diagnostic->publish(fixture()));at.song={true,0,0,1000,120,4,4};
        diagnostic->process(l,r,20,at);diagnostic->collect();
        REQUIRE(rejected->revision==42 && rejected->published.bpm==120);
        std::atomic<bool> finished{false},coherent{true};
        std::thread reader([&]{while(!finished.load())if(const auto d=diagnostic->observation()){
            if(d->observed.valid && d->observed.bpm==120 && d->observed.sample<10000
                && d->observed.ppq!=d->observed.sample/500.0)coherent=false;}});
        for(int i=0;i<2000;++i){at.song.sample=i;at.song.ppq=i/500.0;
            {AudioAllocationGuard guard;diagnostic->process(l,r,20,at);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);}
        finished=true;reader.join();REQUIRE(coherent.load());
    }

    {juce::String error;TransportPlayback handover;handover.invalidate(0,1);handover.invalidate(1,1);REQUIRE(handover.publish(fixture()));
        float left[10]{},right[10]{};TransportTime time{{true,40,.08,1000,120,4,4},true};handover.process(left,right,10,time);
        auto replacement=fixture();auto changed=std::make_shared<WetRender>(*replacement->entries[0].audio);
        for(auto& v:changed->samples)v*=2;replacement->entries[0].audio=changed;replacement->entries[0].version=2;replacement->entries[0].peak=.16f;
        REQUIRE(prepareLiveSegments(*replacement,error));REQUIRE(handover.publish(std::move(replacement)));
        time.song.sample=50;time.song.ppq=.1;std::fill_n(left,10,0.f);std::fill_n(right,10,0.f);
        {AudioAllocationGuard guard;handover.process(left,right,10,time);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        for(int i=0;i<10;++i){const auto current=(61+i)*.001f,neighbour=(21+i)*.0005f;
            const auto blend=i<5?static_cast<float>(4-i)/5:0.f;
            REQUIRE(std::abs(left[i]-(current*(1-blend)+.03f*blend+neighbour))<1e-7f);}}
    TransportPlayback player;player.invalidate(0,1);player.invalidate(1,1);auto p=fixture();REQUIRE(player.publish(std::move(p)));
    float l[200]{},r[200]{};TransportTime t{{true,-20,-.04,1000,120,4,4},true};
    {AudioAllocationGuard guard;player.process(l,r,200,t);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    for(int i=15;i<125;++i){const auto at=-20+i;float expected=0;
        for(int n=0;n<2;++n){const auto j=at-(-10+n*40);if(j>=0 && j<120){
            const auto admission=std::min(1.0f,static_cast<float>(j+1)/5);expected+=(j+1)*.0005f*admission;}}
        REQUIRE(std::abs(l[i]-expected)<1e-6f);}
    // A seek reads the new absolute position, not a sequential preview cursor.
    t.song.sample=60;t.song.ppq=.12;std::fill_n(l,20,0);std::fill_n(r,20,0);player.process(l,r,20,t);
    REQUIRE(std::abs(l[10]-(81+41)*.0005f)<1e-6f);
    player.invalidate(0,2);t.song.sample=80;t.song.ppq=.16;std::fill_n(l,20,0);std::fill_n(r,20,0);player.process(l,r,20,t);
    REQUIRE(player.missing() && std::abs(l[10]-61*.0005f)<1e-6f); // Valid neighbour continues.
    t.playing=false;std::fill_n(l,20,.125f);std::fill_n(r,20,.125f);player.process(l,r,20,t);
    REQUIRE(l[19]==.125f && r[19]==.125f);
    // Publication replacement and retirement stay outside the callback.
    for(int i=0;i<50;++i){REQUIRE(player.publish(fixture()));t.playing=true;t.song.sample=60;t.song.ppq=.12;
        {AudioAllocationGuard guard;player.process(l,r,20,t);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);player.collect();}
    REQUIRE(player.pinnedBytes()<100000);
    auto loud=fixture();loud->entries[0].peak=2;loud->entries[1].peak=3;juce::String error;
    REQUIRE(prepareLiveSegments(*loud,error));
    loud->start=100;loud->end=110;REQUIRE(prepareLiveSegments(*loud,error));
    auto full=fixture();full->entries.resize(65,full->entries[0]);for(size_t i=0;i<full->entries.size();++i)full->entries[i].slot=i;REQUIRE(!prepareLiveSegments(*full,error));
    // Publication segmentation keeps accepted sample gains even when
    // member peak metadata is high.
    auto musical=fixture();
    musical->entries[0].peak=20;musical->entries[1].peak=30;
    REQUIRE(prepareLiveSegments(*musical,error));
    TransportPlayback musicalPlayer;musicalPlayer.invalidate(0,1);musicalPlayer.invalidate(1,1);
    musicalPlayer.configure(true,LiveRouting::wetOnly,6);REQUIRE(musicalPlayer.publish(std::move(musical)));
    float musicalL[32]{},musicalR[32]{};TransportTime musicalTime{{true,60,.12,1000,120,4,4},true};
    musicalPlayer.process(musicalL,musicalR,32,musicalTime);
    REQUIRE(std::abs(musicalL[31]-((102+62)*.0005f*juce::Decibels::decibelsToGain(6.0f)))<1e-6f);
    musicalTime.song.sample=92;musicalTime.song.ppq=.184;std::fill_n(musicalL,32,.125f);std::fill_n(musicalR,32,.125f);
    {AudioAllocationGuard guard;musicalPlayer.process(musicalL,musicalR,32,musicalTime,false);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    for(int n=5;n<32;++n)REQUIRE(musicalL[n]==.125f && musicalR[n]==.125f);
    REQUIRE(!musicalPlayer.active() && !musicalPlayer.missing());
    musicalTime.song.sample=70;musicalTime.song.ppq=.14;std::fill_n(musicalL,32,.125f);std::fill_n(musicalR,32,.125f);
    musicalPlayer.process(musicalL,musicalR,32,musicalTime,true);
    REQUIRE(std::abs(musicalL[15]-((96+56)*.0005f*juce::Decibels::decibelsToGain(6.0f)))<1e-6f);
    player.configure(true,LiveRouting::wetOnly,0);t.playing=true;t.song.sample=10000;t.song.ppq=20;
    std::fill_n(l,20,.125f);std::fill_n(r,20,.125f);player.process(l,r,20,t);REQUIRE(l[19]==0 && player.missing());
    player.configure(false,LiveRouting::wetOnly,0);player.process(l,r,20,t);REQUIRE(!player.active());
    player.collect();
    // Arbitrary block partitioning changes neither interior timing nor gain.
    TransportPlayback a,b;a.invalidate(0,1);a.invalidate(1,1);b.invalidate(0,1);b.invalidate(1,1);
    REQUIRE(a.publish(fixture()) && b.publish(fixture()));float wholeL[200]{},wholeR[200]{},partL[200]{},partR[200]{};
    TransportTime at{{true,-20,-.04,1000,120,4,4},true};a.process(wholeL,wholeR,200,at);
    int offset=0;unsigned random=17;while(offset<200){random=random*1664525u+1013904223u;const auto count=std::min(200-offset,1+static_cast<int>(random%31));
        at.song.sample=-20+offset;at.song.ppq=at.song.sample/500.0;b.process(partL+offset,partR+offset,count,at);offset+=count;}
    for(int i=0;i<200;++i)REQUIRE(std::abs(wholeL[i]-partL[i])<1e-7f && std::abs(wholeR[i]-partR[i])<1e-7f);
    std::vector<float> largeL(16384),largeR(16384);at.song.sample=-20;at.song.ppq=-.04;
    {AudioAllocationGuard guard;a.process(largeL.data(),largeR.data(),16384,at);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    REQUIRE(largeL.back()==0 && a.missing());
    at.song.sample=INT64_MAX-3;at.song.ppq=0;std::fill_n(l,20,0.0f);std::fill_n(r,20,0.0f);a.process(l,r,20,at);REQUIRE(a.incompatible() && l[19]==0);
    {TransportPlayback oversized;REQUIRE(oversized.publish(fixture()));std::vector<float> x(1048577,.125f),y(1048577,.125f);TransportTime t{{true,0,0,1000,120,4,4},true};
        {AudioAllocationGuard guard;oversized.process(x.data(),y.data(),static_cast<int>(x.size()),t);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && oversized.incompatible() && x.back()==.125f);}
    at.song.sample=60;at.song.ppq=.12;at.playing=true;
    b.process(l,r,20,at);at.playing=false;for(int i=0;i<5;++i){l[0]=r[0]=0;b.process(l,r,1,at);}REQUIRE(l[0]==0 && r[0]==0);
    at.playing=true;
    auto noOverlap=fixture();noOverlap->entries[0].peak=2;noOverlap->entries[1].start=1000;noOverlap->entries[1].end=1100;noOverlap->entries[1].peak=3;
    REQUIRE(prepareLiveSegments(*noOverlap,error));
    // A far loop destination can be resident without allocating intervening PCM.
    auto loop=fixture();loop->loopStart=10000;loop->loopEnd=10200;auto late=std::make_shared<WetRender>();late->start=10000;late->rate=1000;late->samples.resize(400,.01f);
    loop->entries.push_back({2,1,10000,10200,.01f,{},late});REQUIRE(prepareLiveSegments(*loop,error));
    TransportPlayback looping;looping.invalidate(0,1);looping.invalidate(1,1);looping.invalidate(2,1);REQUIRE(looping.publish(std::move(loop)));
    at.song.sample=10010;at.song.ppq=20.02;std::fill_n(l,20,0.0f);std::fill_n(r,20,0.0f);looping.process(l,r,20,at);REQUIRE(!looping.missing() && std::abs(l[19]-.01f)<1e-7f);
    // Producer/consumer retirement pressure exercises actual concurrent reads.
    TransportPlayback concurrent;concurrent.invalidate(0,1);concurrent.invalidate(1,1);std::atomic<bool> done{false};std::atomic<unsigned> allocations{0};
    std::thread audio([&]{float x[16]{},y[16]{};TransportTime block{{true,60,.12,1000,120,4,4},true};
        LiveTransportClock clock;
        while(!done.load()){AudioAllocationGuard guard;const auto current=clock.observe(block,16,73);concurrent.process(x,y,16,block,true,&current);allocations.fetch_add(watchedAllocations+watchedDeallocations);}});
    for(int i=0;i<500;++i){concurrent.publish(fixture());concurrent.collect();if(i%7==0)concurrent.invalidate(0,static_cast<uint64_t>(i+1));
        if(const auto causal=concurrent.causalObservation();causal && causal->hasStart){
            REQUIRE(causal->runStart.liveSession==73 && causal->runStart.liveRun==1 && causal->runStart.observed.sample==60);
            if(causal->hasAdmitted)REQUIRE(causal->lastAdmitted.liveSession==73 && causal->lastAdmitted.observed.ppq==.12 && causal->lastAdmitted.gate==LiveGate::admitted);
            REQUIRE(!causal->hasRejection);}}
    done=true;audio.join();concurrent.collect();REQUIRE(allocations.load()==0 && concurrent.pinnedBytes()<100000);
});}
