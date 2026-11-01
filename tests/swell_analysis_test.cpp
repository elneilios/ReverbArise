#include "TestSupport.h"
#include <Analysis/OnsetAnalysis.h>
#include <Capture/CaptureSession.h>
#include <Core/ContentHash.h>
using namespace m3;
int main() { return runTest([]
{
    TestDirectory root; CaptureSession session(root.file); juce::String error;
    for(auto rate:{44100.0,48000.0,96000.0})
    {
        TestDirectory runRoot; CaptureSession run(runRoot.file);
        const auto count=static_cast<int>(rate*2); std::vector<float> samples(static_cast<size_t>(count)*2,0);
        for(auto seconds:{0.1,0.7,1.4})
        {
            const auto at=static_cast<int>(seconds*rate);
            for(int n=0;n<static_cast<int>(rate*0.06);++n)
            {
                const auto value=static_cast<float>(std::exp(-n/(rate*0.01))*std::cos(n*0.17));
                samples[static_cast<size_t>(at+n)*2]=value; samples[static_cast<size_t>(at+n)*2+1]=-value;
            }
        }
        REQUIRE(run.commit(0,rate,samples.data(),count)); auto map=run.snapshot().manifest;
        CaptureAssetStore store(runRoot.file.getChildFile(map.projectId).getChildFile(map.instanceId));
        std::vector<Candidate> found; REQUIRE(suggestOnsets(store,map,0,count,0.5f,0.08,found,error));
        int window=256; while(window<rate*0.021) window*=2;
        for(auto seconds:{0.1,0.7,1.4})
            REQUIRE(std::any_of(found.begin(),found.end(),[&](const auto& c){return std::abs(c.sample-seconds*rate)<=window;}));
        for(size_t i=1;i<found.size();++i) REQUIRE(found[i].sample-found[i-1].sample>=static_cast<int64_t>(rate*0.08));
        std::vector<Candidate> repeated; REQUIRE(suggestOnsets(store,map,0,count,0.5f,0.08,repeated,error));
        REQUIRE(repeated.size()==found.size()); for(size_t i=0;i<found.size();++i) REQUIRE(repeated[i].sample==found[i].sample);
        REQUIRE(run.commit(0,rate,samples.data(),count/2));
        REQUIRE(run.commit(count/2,rate,samples.data()+count,count-count/2));
        REQUIRE(suggestOnsets(store,run.snapshot().manifest,0,count,0.5f,0.08,repeated,error));
        REQUIRE(repeated.size()==found.size()); for(size_t i=0;i<found.size();++i) REQUIRE(repeated[i].sample==found[i].sample);
        const auto retained=found.size(); REQUIRE(!suggestOnsets(store,map,0,count,0.5f,0.08,found,error,[]{return true;})); REQUIRE(found.size()==retained);
        juce::String before,after;
        REQUIRE(sourceFingerprint(store,map,100,1000,before,error));
        REQUIRE(run.commit(100,rate,samples.data()+200,900)); auto next=run.snapshot().manifest;
        REQUIRE(sourceFingerprint(store,next,100,1000,after,error) && before==after);
        float changed[]{0.25f,-0.5f}; REQUIRE(run.commit(500,rate,changed,1));
        REQUIRE(sourceFingerprint(store,run.snapshot().manifest,100,1000,after,error) && before!=after);
        REQUIRE(sourceFingerprint(store,map,0,100,before,error));
        REQUIRE(sourceFingerprint(store,run.snapshot().manifest,0,100,after,error) && before==after);
    }
    // Attack acceptance: one decaying event must not generate release/tail targets.
    // The former flux-only detector produced multiple markers on these fixtures.
    for(int fixture=0;fixture<5;++fixture)
    {
        TestDirectory fixtureRoot; CaptureSession capture(fixtureRoot.file);
        constexpr int rate=48000,count=144000; std::vector<float> pcm(count*2);
        juce::Random noise(781); const std::vector<int> attacks=fixture==3 ? std::vector<int>{4800,14400,24000} : std::vector<int>{4800};
        for(auto at:attacks) for(int n=0;n<count-at;++n)
        {
            const auto decay=fixture==4 ? (n<96000?1.0:0.0) : std::exp(-n/18000.0);
            const auto modulation=1.0+0.25*std::sin(n*juce::MathConstants<double>::twoPi*5/rate);
            const auto attack=fixture==2 ? std::min(1.0,n/600.0) : 1.0;
            const auto tone=fixture==1 ? noise.nextFloat()*2-1 : std::sin(n*(fixture==3 ? 0.04+at*0.000003 : 0.058)+0.8*std::sin(n*0.001));
            const auto value=static_cast<float>(0.3*decay*modulation*attack*tone);
            pcm[(at+n)*2]+=value; pcm[(at+n)*2+1]-=value;
        }
        REQUIRE(capture.commit(0,rate,pcm.data(),count));
        auto manifest=capture.snapshot().manifest; CaptureAssetStore assets(fixtureRoot.file.getChildFile(manifest.projectId).getChildFile(manifest.instanceId));
        std::vector<Candidate> whole,split;
        REQUIRE(suggestOnsets(assets,manifest,0,count,0.5f,0.08,whole,error));
        std::cout << "attack fixture " << fixture << ": " << whole.size() << " targets\n";
        for(auto c:whole)std::cout << c.sample << " "; std::cout << "\n";
        REQUIRE(whole.size()==attacks.size());
        for(size_t i=0;i<attacks.size();++i) REQUIRE(std::abs(whole[i].sample-attacks[i])<=2048);
        REQUIRE(capture.commit(0,rate,pcm.data(),7777));
        REQUIRE(capture.commit(7777,rate,pcm.data()+7777*2,count-7777));
        REQUIRE(suggestOnsets(assets,capture.snapshot().manifest,0,count,0.5f,0.08,split,error));
        REQUIRE(split.size()==whole.size());
        for(size_t i=0;i<whole.size();++i) REQUIRE(split[i].sample==whole[i].sample);
    }
    {
        TestDirectory gapRoot;CaptureSession gaps(gapRoot.file);std::vector<float> event(24000*2);
        for(int i=4800;i<10000;++i)event[i*2]=static_cast<float>(.2*std::exp(-(i-4800)/1000.0)*std::sin(i*.13));
        REQUIRE(gaps.commit(0,48000,event.data(),24000));REQUIRE(gaps.commit(48000,48000,event.data(),24000));
        const auto manifest=gaps.snapshot().manifest;CaptureAssetStore assets(gapRoot.file.getChildFile(manifest.projectId).getChildFile(manifest.instanceId));
        std::vector<Candidate> notes;REQUIRE(suggestOnsets(assets,manifest,0,72000,.5f,.08,notes,error));
        REQUIRE(notes.size()==2 && std::abs(notes[0].sample-4800)<=2048 && std::abs(notes[1].sample-52800)<=2048);
    }
    {
        TestDirectory busyRoot;CaptureSession busy(busyRoot.file);std::vector<float> pcm(192000*2);
        const std::vector<int> major{4800,62400,120000};
        for(int at=4800;at<180000;at+=14400){const auto strong=std::find(major.begin(),major.end(),at)!=major.end();
            for(int n=0;n<4800;++n){const auto v=static_cast<float>((strong?.6:.18)*std::exp(-n/1000.0)*std::sin(n*.11));pcm[(at+n)*2]=v;pcm[(at+n)*2+1]=v;}}
        REQUIRE(busy.commit(0,48000,pcm.data(),192000));const auto map=busy.snapshot().manifest;
        CaptureAssetStore assets(busyRoot.file.getChildFile(map.projectId).getChildFile(map.instanceId));std::vector<Candidate> low,normal,high;
        REQUIRE(suggestOnsets(assets,map,0,192000,0,.08,low,error));REQUIRE(suggestOnsets(assets,map,0,192000,.5f,.08,normal,error));REQUIRE(suggestOnsets(assets,map,0,192000,1,.08,high,error));
        std::cout<<"Busy sensitivity low/default/high: "<<low.size()<<"/"<<normal.size()<<"/"<<high.size()<<"\n";
        REQUIRE(low.size()<high.size());for(auto at:major)REQUIRE(std::any_of(low.begin(),low.end(),[&](auto c){return std::abs(c.sample-at)<=2048;}));
    }
    std::vector<float> zeros(2000,0); REQUIRE(session.commit(-1000,48000,zeros.data(),1000));
    REQUIRE(session.commit(1000,48000,zeros.data(),1000)); const auto map=session.snapshot().manifest;
    CaptureAssetStore store(root.file.getChildFile(map.projectId).getChildFile(map.instanceId));
    std::vector<Candidate> found; REQUIRE(suggestOnsets(store,map,-1000,2000,1,0.001,found,error) && found.empty());
    juce::String fingerprint; REQUIRE(!sourceFingerprint(store,map,-1000,2000,fingerprint,error));
    // A zero-valued audio region and covered silence have identical sample identity.
    REQUIRE(sourceFingerprint(store,map,-900,-100,fingerprint,error));
    auto altered=map; altered.ranges[0].revision++; juce::String again;
    REQUIRE(sourceFingerprint(store,altered,-900,-100,again,error) && fingerprint==again);
    auto& zeroRange=altered.ranges[0]; zeroRange.kind=CaptureCoverageKind::audio;
    zeroRange.assetPath="segments/"+juce::Uuid().toString().toStdString()+".wav";
    REQUIRE(store.writeAudioChunk(zeroRange,48000,zeros.data(),error));
    REQUIRE(sourceFingerprint(store,altered,-900,-100,again,error) && fingerprint==again);
    // Soft attacks and dense/noisy material still give bounded deterministic suggestions.
    std::vector<float> soft(96000,0);
    for(int i=0;i<12000;++i)
    {
        const auto level=std::min(1.0,i/600.0)*std::exp(-i/3000.0);
        soft[static_cast<size_t>(4800+i)*2]=static_cast<float>(level*std::sin(i*0.1));
    }
    REQUIRE(session.commit(3000,48000,soft.data(),48000));
    REQUIRE(suggestOnsets(store,session.snapshot().manifest,3000,51000,0.7f,0.08,found,error));
    REQUIRE(std::any_of(found.begin(),found.end(),[](const auto& c){return std::abs(c.sample-7800)<=1024;}));
    juce::Random random(481);
    for(size_t i=0;i<soft.size();++i) soft[i]=random.nextFloat()*0.05f;
    for(int at=2000;at<46000;at+=2000) soft[static_cast<size_t>(at)*2]=1;
    REQUIRE(session.commit(3000,48000,soft.data(),48000));
    REQUIRE(suggestOnsets(store,session.snapshot().manifest,3000,51000,0.9f,0.08,found,error));
    for(size_t i=1;i<found.size();++i) REQUIRE(found[i].sample-found[i-1].sample>=3840);
}); }
