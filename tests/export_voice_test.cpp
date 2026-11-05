#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Playback/ExportVoice.h>
#include <Core/MonitoringSafety.h>
#include <vector>
#include <thread>
#include <limits>
int main(){return runTest([]{
    {
        auto layers=std::make_unique<m3::ExportVoice>();std::array<float,200> dry{},wet{};
        dry.fill(.2f);wet.fill(.4f);const auto request=layers->requestStart();
        layers->setLevels(request,1,0,100);REQUIRE(layers->pushLayers(request,0,0,100,1000,dry,wet));layers->arm(request);
        std::array<float,64> a{},b{};layers->process(a.data(),b.data(),37,1000,true);REQUIRE(layers->position()==37);
        layers->setLevels(request,0,1,100);a.fill(0);b.fill(0);
        {AudioAllocationGuard guard;layers->process(a.data(),b.data(),13,1000,true);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && layers->position()==50 && layers->request()==request);
        for(int n=0;n<13;++n)REQUIRE(std::abs(a[n]-(.2f+.2f*std::min(1.f,(n+1)/5.f)))<1e-6f);
        REQUIRE(layers->acceptedLevels().dry==0 && layers->acceptedLevels().wet==1);
        layers->setLevels(request,1,0,100);layers->setLevels(request,.5f,.25f,100);
        a.fill(0);b.fill(0);layers->process(a.data(),b.data(),13,1000,true);
        REQUIRE(layers->position()==63 && layers->acceptedLevels().dry==.5f && layers->acceptedLevels().wet==.25f);
        layers->stop(true);layers->setLevels(request,1,1,100);layers->process(a.data(),b.data(),13,1000,true);REQUIRE(!layers->position());
        REQUIRE(layers->workingBytes()<1200000);
    }
    {
        auto layers=std::make_unique<m3::ExportVoice>();constexpr int end=10000;
        std::vector<float> dry(end*2,.2f),wet(end*2,.4f);const auto ticket=layers->requestStart();layers->setLevels(ticket,1,0,end);
        for(int at=0;at<end;at+=4096){const auto count=static_cast<size_t>(std::min(4096,end-at))*2;
            REQUIRE(layers->pushLayers(ticket,at,0,end,1000,{dry.data()+at*2,count},{wet.data()+at*2,count}));}layers->arm(ticket);
        std::vector<float> a(end),b(end);int at=0;
        while(at<end){const int count=std::min(at==4070?37:std::min(37,4070-at),end-at);
            if(at==4070)break;{AudioAllocationGuard guard;layers->process(a.data()+at,b.data()+at,count,1000,true);}at+=count;}
        REQUIRE(at==4070 && layers->position()==4070);
        layers->setLevels(ticket,0,1,end);
        while(at<end){const int count=std::min(127,end-at);{AudioAllocationGuard guard;layers->process(a.data()+at,b.data()+at,count,1000,true);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && !layers->starved());at+=count;if(at<end)REQUIRE(layers->position()==at);}
        for(int n=0;n<end;++n){const float mix=n<4070?.2f:.2f+.2f*std::min(1.f,(n-4069)/5.f);
            REQUIRE(std::abs(a[n]-mix*m3::monitorEdgeGain(n,end,1000,.005))<1e-6f);}
        REQUIRE(!layers->position());
        const auto next=layers->requestStart();layers->setLevels(next,1,1,100);
        REQUIRE(layers->pushLayers(next,0,0,100,1000,{dry.data(),200},{wet.data(),200}));layers->arm(next);
        layers->process(a.data(),b.data(),37,1000,true);REQUIRE(layers->position()==37);
        layers->setLevels(next,1,0,20);layers->process(a.data(),b.data(),37,1000,true);REQUIRE(!layers->position());
    }
    {
        auto layers=std::make_unique<m3::ExportVoice>();constexpr int end=60000;
        std::vector<float> d(end*2,.2f),w(end*2,.4f);const auto ticket=layers->requestStart();layers->setLevels(ticket,1,0,end);
        for(int at=0;at<end;at+=4096){const auto size=static_cast<size_t>(std::min(4096,end-at))*2;
            REQUIRE(layers->pushLayers(ticket,at,0,end,1000,{d.data()+at*2,size},{w.data()+at*2,size}));}layers->arm(ticket);
        std::array<float,127> a{},b{};layers->process(a.data(),b.data(),13,1000,true);REQUIRE(layers->position()==13);
        std::jthread writer([&]{for(int n=0;n<20000;++n)layers->setLevels(ticket,n%2?1.f:0.f,n%2?0.f:1.f,end);});
        for(int at=13;at<end;){const auto count=std::min(127,end-at);a.fill(0);b.fill(0);
            {AudioAllocationGuard guard;layers->process(a.data(),b.data(),count,1000,true);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && !layers->starved());
            const auto pair=layers->acceptedLevels();REQUIRE((pair.dry==1 && pair.wet==0) || (pair.dry==0 && pair.wet==1));
            for(int n=0;n<count;++n)if(at+n>=5 && at+n<end-5)REQUIRE(a[n]>=.19999f && a[n]<=.40001f);
            at+=count;if(at<end)REQUIRE(layers->position()==at);}
        writer.join();
    }
    {
        auto layers=std::make_unique<m3::ExportVoice>();std::array<float,200> hot;hot.fill(std::numeric_limits<float>::max());
        const auto ticket=layers->requestStart();layers->setLevels(ticket,1,1,100);
        REQUIRE(layers->pushLayers(ticket,0,0,100,1000,hot,hot));layers->arm(ticket);std::array<float,37> a{},b{};
        {AudioAllocationGuard guard;layers->process(a.data(),b.data(),37,1000,true);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && layers->invalidAudio() && !layers->position());
        REQUIRE(std::all_of(a.begin(),a.end(),[](float v){return std::isfinite(v);}) && std::all_of(b.begin(),b.end(),[](float v){return std::isfinite(v);}));
    }
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
        std::vector<float> input(total*2,.1f);std::atomic<bool> primed{false};const auto request=stream->requestStart();stream->setLevels(request,.5f,.25f,100000+total);
        std::jthread producer([&](std::stop_token stop){int at=0,pages=0;while(at<total && !stop.stop_requested() && stream->requested(request)){
            const auto count=std::min(4096,total-at);
            if(stream->pushLayers(request,100000+at,100000,100000+total,48000,{input.data()+at*2,static_cast<size_t>(count)*2},{input.data()+at*2,static_cast<size_t>(count)*2})){
                at+=count;if(++pages==8 || at==total){stream->arm(request);primed=true;}}
            else std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }});
        while(!primed.load())std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::array<float,127> a{},b{};
        for(int at=0;at<total;){a.fill(0);b.fill(0);const auto count=std::min(127,total-at);
            {AudioAllocationGuard guard;stream->process(a.data(),b.data(),count,48000,true);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && !stream->starved());
            for(int n=0;n<count;++n)REQUIRE(std::abs(a[n]-.075f*m3::monitorEdgeGain(static_cast<size_t>(at+n),total,48000,.005))<1e-7);
            at+=count;std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        REQUIRE(!stream->position() && stream->workingBytes()<1200000);
    }
    const auto full=voice->requestStart();for(size_t i=0;i<m3::ExportVoice::pageCount;++i)
        REQUIRE(voice->push(full,static_cast<int64_t>(i),0,100,1000,{pcm.data(),2}));
    REQUIRE(!voice->push(full,20,0,100,1000,{pcm.data(),2}) && voice->workingBytes()<1200000);
});}
