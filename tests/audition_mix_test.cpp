#include "TestSupport.h"
#include <DSP/AuditionMix.h>
#include <Playback/TransportPlayback.h>
using namespace m3;
int main(){return runTest([]
{
    WetRender wet;wet.start=-3;wet.rate=1000;wet.sourceStart=0;wet.sourceEnd=4;
    wet.samples={.1f,-.1f,.2f,-.2f,.3f,-.3f,.4f,-.4f};
    SourceInterval dry;dry.start=0;dry.end=4;dry.rate=1000;dry.samples={.2f,.1f,.1f,.2f,-.2f,-.1f,-.1f,-.2f};
    juce::String error;WetRender out;
    for(float mix:{0.0f,.5f,.65f,1.0f})
    {
        REQUIRE(prepareAuditionMix(wet,&dry,AuditionMode::dryWet,mix,previewMemoryBudget,out,error));
        REQUIRE(out.start==-3 && out.frames()==7);
        for(int i=0;i<7;++i)for(int c=0;c<2;++c)
        {
            const auto expected=(i<4?wet.samples[i*2+c]*mix*(i==0 || i==3?0:1):0)+(i>=3?dry.samples[(i-3)*2+c]*(1-mix):0);
            REQUIRE(std::abs(out.samples[i*2+c]-expected)<1e-7f);
        }
    }
    REQUIRE(prepareAuditionMix(wet,nullptr,AuditionMode::wetOnly,.35f,previewMemoryBudget,out,error));
    REQUIRE(out.frames()==4 && out.samples[0]==0 && out.samples[6]==0 && out.samples[2]==wet.samples[2]);
    REQUIRE(!prepareAuditionMix(wet,nullptr,AuditionMode::dryWet,.5f,previewMemoryBudget,out,error));
    REQUIRE(!prepareAuditionMix(wet,&dry,AuditionMode::dryWet,.5f,4,out,error));
    wet.samples.assign(20,juce::Decibels::decibelsToGain(7.2f));wet.start=0;
    REQUIRE(prepareAuditionMix(wet,nullptr,AuditionMode::wetOnly,1,previewMemoryBudget,out,error));
    REQUIRE(out.peak==wet.samples[0]);
    dry.samples[0]=2;
    REQUIRE(prepareAuditionMix(wet,&dry,AuditionMode::dryWet,.5f,previewMemoryBudget,out,error));
    // At 1 kHz the 2 ms edge ramp reaches unity at frame two.
    REQUIRE(out.peak>1 && out.samples[4]==(wet.samples[4]+dry.samples[4])*.5f);
    REQUIRE(wet.samples[0]>1 && dry.samples[0]==2); // No asset normalization.
    REQUIRE(prepareAuditionMix(wet,nullptr,AuditionMode::wetOnly,1,previewMemoryBudget,out,error,false));
    REQUIRE(out.peak==wet.samples[0]);
    // Equal musical membership and unity path controls produce the same wet
    // interior samples; live admission ramps remain a documented boundary.
    WetRender quiet;quiet.start=0;quiet.rate=1000;quiet.samples.assign(80,.25f);quiet.peak=.25f;
    for(const float db:{-12.f,0.f,6.f})for(const auto mode:{AuditionMode::wetOnly,AuditionMode::dryWet})
    {
        SourceInterval captured;captured.start=0;captured.end=40;captured.rate=1000;captured.samples.assign(80,.01f);
        WetRender result;REQUIRE(prepareAuditionMix(quiet,&captured,mode,.05f,previewMemoryBudget,result,error,false,db));
        const auto gain=juce::Decibels::decibelsToGain(db);const auto expected=.25f*gain+(mode==AuditionMode::dryWet?.01f:0.f);
        for(auto sample:result.samples)REQUIRE(std::abs(sample-expected)<1e-6);
        auto pub=std::make_unique<LivePublication>();pub->mapping={true,0,0,1000,120,4,4};pub->start=0;pub->end=40;
        pub->entries.push_back({0,1,0,40,.25f,std::make_shared<WetRender>(quiet)});REQUIRE(prepareLiveSegments(*pub,error));
        TransportPlayback transport;transport.invalidate(0,1);transport.configure(true,mode==AuditionMode::wetOnly?LiveRouting::wetOnly:LiveRouting::dryWet,db);REQUIRE(transport.publish(std::move(pub)));
        float a[40],b[40];std::fill_n(a,40,.01f);std::fill_n(b,40,.01f);transport.process(a,b,40,{{true,0,0,1000,120,4,4},true});
        for(int n=6;n<40;++n)REQUIRE(std::abs(a[n]-result.samples[n*2])<1e-6 && std::abs(b[n]-result.samples[n*2+1])<1e-6);
    }
    REQUIRE(!prepareAuditionMix(quiet,nullptr,AuditionMode::wetOnly,.5f,previewMemoryBudget,out,error,false,std::numeric_limits<float>::quiet_NaN()));
    WetRender monitored;REQUIRE(prepareAuditionMix(quiet,nullptr,AuditionMode::wetOnly,1,previewMemoryBudget,monitored,error,false));
    auto publication=std::make_unique<LivePublication>();publication->mapping={true,0,0,1000,120,4,4};publication->start=0;publication->end=40;
    publication->entries.push_back({0,1,0,40,.25f,std::make_shared<WetRender>(quiet)});
    REQUIRE(prepareLiveSegments(*publication,error));TransportPlayback live;live.invalidate(0,1);live.configure(true,LiveRouting::wetOnly,0);REQUIRE(live.publish(std::move(publication)));
    float liveL[40]{},liveR[40]{};TransportTime time{{true,0,0,1000,120,4,4},true};live.process(liveL,liveR,40,time);
    for(int n=5;n<40;++n)REQUIRE(std::abs(liveL[n]-monitored.samples[n*2])<1e-7f && std::abs(liveR[n]-monitored.samples[n*2+1])<1e-7f);
});}
