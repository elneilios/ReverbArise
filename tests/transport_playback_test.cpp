#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Playback/TransportPlayback.h>
#include <thread>
using namespace m3;
static std::unique_ptr<LivePublication> fixture(int64_t first=-20,int64_t last=180)
{
    auto p=std::make_unique<LivePublication>();p->mapping={true,0,0,1000,120,4,4};p->start=first;p->end=last;
    for(size_t n=0;n<2;++n){auto a=std::make_shared<WetRender>();a->start=-10+static_cast<int64_t>(n)*40;a->rate=1000;a->samples.resize(240);
        for(size_t i=0;i<120;++i){a->samples[i*2]=static_cast<float>(i+1)*.0005f;a->samples[i*2+1]=-.02f;}
        p->entries.push_back({n,1,a->start,a->start+120,.08f,a});}
    juce::String error;REQUIRE(prepareLiveSegments(*p,error));return p;
}
int main(){return runTest([]{
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
    TransportPlayback player;player.invalidate(0,1);player.invalidate(1,1);auto p=fixture();REQUIRE(p->attenuation==1);REQUIRE(player.publish(std::move(p)));
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
    REQUIRE(prepareLiveSegments(*loud,error));REQUIRE(std::abs(loud->attenuation-std::pow(10.0f,-6.0f/20)/5)<1e-6f);
    loud->start=100;loud->end=110;REQUIRE(prepareLiveSegments(*loud,error));REQUIRE(std::abs(loud->attenuation-std::pow(10.0f,-6.0f/20)/5)<1e-6f);
    auto full=fixture();full->entries.resize(65,full->entries[0]);for(size_t i=0;i<full->entries.size();++i)full->entries[i].slot=i;REQUIRE(!prepareLiveSegments(*full,error));
    // A musical publication carries one fixed full-support measurement; page
    // segmentation must not replace it with the legacy member-peak bound.
    auto musical=fixture();musical->measuredAttenuation=true;musical->attenuation=.75f;
    musical->entries[0].peak=20;musical->entries[1].peak=30;
    REQUIRE(prepareLiveSegments(*musical,error) && musical->attenuation==.75f);
    TransportPlayback musicalPlayer;musicalPlayer.invalidate(0,1);musicalPlayer.invalidate(1,1);
    musicalPlayer.configure(true,LiveRouting::wetOnly,6);REQUIRE(musicalPlayer.publish(std::move(musical)));
    float musicalL[32]{},musicalR[32]{};TransportTime musicalTime{{true,60,.12,1000,120,4,4},true};
    musicalPlayer.process(musicalL,musicalR,32,musicalTime);
    REQUIRE(std::abs(musicalL[31]-((102+62)*.0005f*.75f*juce::Decibels::decibelsToGain(6.0f)))<1e-6f);
    musicalTime.song.sample=92;musicalTime.song.ppq=.184;std::fill_n(musicalL,32,.125f);std::fill_n(musicalR,32,.125f);
    {AudioAllocationGuard guard;musicalPlayer.process(musicalL,musicalR,32,musicalTime,false);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    for(int n=5;n<32;++n)REQUIRE(musicalL[n]==.125f && musicalR[n]==.125f);
    REQUIRE(!musicalPlayer.active() && !musicalPlayer.missing());
    musicalTime.song.sample=70;musicalTime.song.ppq=.14;std::fill_n(musicalL,32,.125f);std::fill_n(musicalR,32,.125f);
    musicalPlayer.process(musicalL,musicalR,32,musicalTime,true);
    REQUIRE(std::abs(musicalL[15]-((96+56)*.0005f*.75f*juce::Decibels::decibelsToGain(6.0f)))<1e-6f);
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
    REQUIRE(prepareLiveSegments(*noOverlap,error));REQUIRE(std::abs(noOverlap->attenuation-std::pow(10.0f,-6.0f/20)/3)<1e-6f);
    // A far loop destination can be resident without allocating intervening PCM.
    auto loop=fixture();loop->loopStart=10000;loop->loopEnd=10200;auto late=std::make_shared<WetRender>();late->start=10000;late->rate=1000;late->samples.resize(400,.01f);
    loop->entries.push_back({2,1,10000,10200,.01f,{},late});REQUIRE(prepareLiveSegments(*loop,error));
    TransportPlayback looping;looping.invalidate(0,1);looping.invalidate(1,1);looping.invalidate(2,1);REQUIRE(looping.publish(std::move(loop)));
    at.song.sample=10010;at.song.ppq=20.02;std::fill_n(l,20,0.0f);std::fill_n(r,20,0.0f);looping.process(l,r,20,at);REQUIRE(!looping.missing() && std::abs(l[19]-.01f)<1e-7f);
    // Producer/consumer retirement pressure exercises actual concurrent reads.
    TransportPlayback concurrent;concurrent.invalidate(0,1);concurrent.invalidate(1,1);std::atomic<bool> done{false};std::atomic<unsigned> allocations{0};
    std::thread audio([&]{float x[16]{},y[16]{};TransportTime block{{true,60,.12,1000,120,4,4},true};
        while(!done.load()){AudioAllocationGuard guard;concurrent.process(x,y,16,block);allocations.fetch_add(watchedAllocations+watchedDeallocations);}});
    for(int i=0;i<500;++i){concurrent.publish(fixture());concurrent.collect();if(i%7==0)concurrent.invalidate(0,static_cast<uint64_t>(i+1));}
    done=true;audio.join();concurrent.collect();REQUIRE(allocations.load()==0 && concurrent.pinnedBytes()<100000);
});}
