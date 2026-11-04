#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Playback/ExportVoice.h>
#include <Core/MonitoringSafety.h>
#include <vector>
#include <thread>
int main(){return runTest([]{
    auto voice=std::make_unique<m3::ExportVoice>();constexpr int frames=10000;std::vector<float> pcm(frames*2);
    for(int n=0;n<frames;++n){pcm[n*2]=static_cast<float>((n%23)*.01);pcm[n*2+1]=-.1f;}
    const auto ticket=voice->requestStart();
    for(int at=0;at<frames;at+=4096){const auto count=std::min(4096,frames-at);
        REQUIRE(voice->push(ticket,500+at,500,500+frames,1000,{pcm.data()+at*2,static_cast<size_t>(count)*2}));}
    voice->arm(ticket);std::vector<float> left(frames),right(frames);
    for(int at=0;at<frames;){const auto count=std::min(37,frames-at);
        {AudioAllocationGuard guard;voice->process(left.data()+at,right.data()+at,count,1000,true);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        at+=count;if(at<frames)REQUIRE(voice->position()==500+at);
    }
    REQUIRE(!voice->position() && !voice->starved());
    for(int n=0;n<frames;++n){const auto gain=m3::monitorEdgeGain(n,frames,1000,.005);
        REQUIRE(std::abs(left[n]-pcm[n*2]*gain)<1e-7 && std::abs(right[n]-pcm[n*2+1]*gain)<1e-7);}
    std::array<float,64> l{},r{};
    // Stop revokes an unarmed completion; stale pages cannot start it later.
    const auto delayed=voice->requestStart();REQUIRE(voice->push(delayed,0,0,10,1000,{pcm.data(),20}));
    voice->stop();voice->arm(delayed);voice->process(l.data(),r.data(),64,1000,true);
    REQUIRE(!voice->position() && std::all_of(l.begin(),l.end(),[](float x){return x==0;}));
    // A missing next page stops at the boundary instead of skipping it.
    const auto starved=voice->requestStart();REQUIRE(voice->push(starved,0,0,100,1000,{pcm.data(),20}));voice->arm(starved);
    voice->process(l.data(),r.data(),64,1000,true);REQUIRE(voice->starved() && !voice->position());
    const auto stop=voice->requestStart();REQUIRE(voice->push(stop,0,0,100,1000,{pcm.data(),200}));voice->arm(stop);
    voice->process(l.data(),r.data(),10,1000,true);REQUIRE(voice->position()==10);
    voice->stop();l.fill(0);r.fill(0);voice->process(l.data(),r.data(),64,1000,true);
    REQUIRE(!voice->position() && l[4]==0 && l[5]==0);
    const auto exited=voice->requestStart();REQUIRE(voice->push(exited,0,0,100,1000,{pcm.data(),200}));voice->arm(exited);
    voice->process(l.data(),r.data(),10,1000,true);voice->process(l.data(),r.data(),10,1000,false);
    REQUIRE(!voice->position());voice->process(l.data(),r.data(),10,1000,true);REQUIRE(!voice->position());
    // A new seek ticket hides the outgoing cursor and cannot be armed by stale prefetch.
    const auto outgoing=voice->requestStart();REQUIRE(voice->push(outgoing,0,0,100,1000,{pcm.data(),200}));voice->arm(outgoing);
    voice->process(l.data(),r.data(),10,1000,true);REQUIRE(voice->position()==10);
    const auto seek=voice->requestStart();REQUIRE(!voice->position());voice->arm(outgoing);
    REQUIRE(voice->push(seek,40,40,100,1000,{pcm.data()+80,120}));voice->arm(seek);
    voice->process(l.data(),r.data(),10,1000,true); // Finish only the old five-sample fade.
    l.fill(0);r.fill(0);{AudioAllocationGuard guard;voice->process(l.data(),r.data(),10,1000,true);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && voice->position()==50);
    for(int n=0;n<10;++n)REQUIRE(std::abs(l[n]-pcm[(40+n)*2]*m3::monitorEdgeGain(n,60,1000,.005))<1e-7);
    voice->stop(true);voice->process(l.data(),r.data(),10,1000,true);
    {
        auto stream=std::make_unique<m3::ExportVoice>();constexpr int total=150000;
        std::vector<float> input(total*2,.1f);std::atomic<bool> primed{false};const auto request=stream->requestStart();
        std::jthread producer([&](std::stop_token stop){int at=0,pages=0;while(at<total && !stop.stop_requested() && stream->requested(request)){
            const auto count=std::min(4096,total-at);
            if(stream->push(request,100000+at,100000,100000+total,48000,{input.data()+at*2,static_cast<size_t>(count)*2})){
                at+=count;if(++pages==8 || at==total){stream->arm(request);primed=true;}}
            else std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }});
        while(!primed.load())std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::array<float,127> a{},b{};
        for(int at=0;at<total;){a.fill(0);b.fill(0);const auto count=std::min(127,total-at);
            {AudioAllocationGuard guard;stream->process(a.data(),b.data(),count,48000,true);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && !stream->starved());
            for(int n=0;n<count;++n)REQUIRE(std::abs(a[n]-.1f*m3::monitorEdgeGain(static_cast<size_t>(at+n),total,48000,.005))<1e-7);
            at+=count;std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        REQUIRE(!stream->position() && stream->workingBytes()<600000);
    }
    const auto full=voice->requestStart();for(size_t i=0;i<m3::ExportVoice::pageCount;++i)
        REQUIRE(voice->push(full,static_cast<int64_t>(i),0,100,1000,{pcm.data(),2}));
    REQUIRE(!voice->push(full,20,0,100,1000,{pcm.data(),2}) && voice->workingBytes()<600000);
});}
