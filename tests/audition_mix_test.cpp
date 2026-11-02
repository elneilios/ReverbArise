#include "TestSupport.h"
#include <DSP/AuditionMix.h>
using namespace m3;
int main(){return runTest([]
{
    WetRender wet;wet.start=-3;wet.rate=1000;wet.sourceStart=0;wet.sourceEnd=4;
    wet.samples={.1f,-.1f,.2f,-.2f,.3f,-.3f,.4f,-.4f};
    SourceInterval dry;dry.start=0;dry.end=4;dry.rate=1000;dry.samples={.2f,.1f,.1f,.2f,-.2f,-.1f,-.1f,-.2f};
    juce::String error;float branch,sum;WetRender out;
    for(float mix:{0.0f,.5f,.65f,1.0f})
    {
        REQUIRE(prepareAuditionMix(wet,&dry,AuditionMode::dryWet,mix,previewMemoryBudget,out,branch,sum,error));
        REQUIRE(out.start==-3 && out.frames()==7 && branch==0 && sum==0);
        for(int i=0;i<7;++i)for(int c=0;c<2;++c)
        {
            const auto expected=(i<4?wet.samples[i*2+c]*mix*(i==0 || i==3?0:1):0)+(i>=3?dry.samples[(i-3)*2+c]*(1-mix):0);
            REQUIRE(std::abs(out.samples[i*2+c]-expected)<1e-7f);
        }
    }
    REQUIRE(prepareAuditionMix(wet,nullptr,AuditionMode::wetOnly,.35f,previewMemoryBudget,out,branch,sum,error));
    REQUIRE(out.frames()==4 && out.samples[0]==0 && out.samples[6]==0 && out.samples[2]==wet.samples[2]);
    REQUIRE(!prepareAuditionMix(wet,nullptr,AuditionMode::dryWet,.5f,previewMemoryBudget,out,branch,sum,error));
    REQUIRE(!prepareAuditionMix(wet,&dry,AuditionMode::dryWet,.5f,4,out,branch,sum,error));
    wet.samples.assign(20,juce::Decibels::decibelsToGain(7.2f));wet.start=0;
    REQUIRE(prepareAuditionMix(wet,nullptr,AuditionMode::wetOnly,1,previewMemoryBudget,out,branch,sum,error));
    REQUIRE(std::abs(branch+13.2f)<1e-4f && std::abs(juce::Decibels::gainToDecibels(out.peak)+6)<1e-4f);
    dry.samples[0]=2;
    REQUIRE(prepareAuditionMix(wet,&dry,AuditionMode::dryWet,.5f,previewMemoryBudget,out,branch,sum,error));
    REQUIRE(sum<0 && out.peak<=juce::Decibels::decibelsToGain(-6.0f)+1e-7f);
    REQUIRE(wet.samples[0]>1 && dry.samples[0]==2); // No asset normalization.
});}
