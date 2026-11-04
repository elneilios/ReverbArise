#include "TestSupport.h"
#include <Analysis/LearnCandidates.h>
#include <Capture/CaptureSession.h>
using namespace m3;

int main(){return runTest([]
{
    TestDirectory directory;CaptureSession capture(directory.file);juce::String error;
    constexpr int rate=48000,frames=96000;std::vector<float> pcm(frames*2);
    for(int at:{4800,33600,67200})for(int n=0;n<2880;++n)
    {
        const auto value=static_cast<float>(std::exp(-n/480.0)*std::cos(n*.17));
        pcm[(at+n)*2]=value;pcm[(at+n)*2+1]=-value;
    }
    REQUIRE(capture.commit(-24000,rate,pcm.data(),frames));
    REQUIRE(capture.commit(192000,rate,pcm.data(),frames));
    std::vector<float> silence(24000*2);REQUIRE(capture.commit(120000,rate,silence.data(),24000));
    const auto saved=capture.snapshot();
    CaptureAssetStore store(directory.file.getChildFile(saved.manifest.projectId).getChildFile(saved.manifest.instanceId));
    CandidateIdentity identity;identity.session=1;identity.captureGeneration=1;
    identity.sourceRevision=saved.manifest.revision;identity.sourceReference=saved.reference;
    identity.timing={true,0,0,rate,120,4,4};identity.settings=1;
    LearnCandidateSnapshot scan;
    REQUIRE(scanLearnCandidates(store,saved.manifest,identity,{},scan,error));
    REQUIRE(scan.positions.size()==6 && scan.mappedRuns.size()==3 && !scan.identity.provisional);
    std::vector<Candidate> reference;
    REQUIRE(suggestOnsets(store,saved.manifest,-24000,288000,.5f,.08,reference,error));
    REQUIRE(reference.size()==scan.positions.size());
    for(size_t i=0;i<reference.size();++i)REQUIRE(reference[i].sample==scan.positions[i].sample);
    for(auto c:scan.positions)REQUIRE(c.sample<72000 || c.sample>=192000);
    auto later=identity;later.timing.sample=48000;later.timing.ppq=2;
    REQUIRE(sameCandidateIdentity(identity,later));
    for(int mismatch=0;mismatch<14;++mismatch)
    {
        later=identity;
        if(mismatch==0)++later.session;if(mismatch==1)++later.captureGeneration;
        if(mismatch==2)++later.sourceRevision;if(mismatch==3)++later.settings;
        if(mismatch==4)++later.pass;if(mismatch==5)later.sourceReference="different";
        if(mismatch==6)later.provisional=true;if(mismatch==7)later.sensitivity=.7f;
        if(mismatch==8)later.separationSeconds=.2;if(mismatch==9)later.timing.bpm=121;
        if(mismatch==10)++later.policy;if(mismatch==11)later.scope="different";
        if(mismatch==12)later.timing.ppq=1;
        if(mismatch==13)++later.boundaries;
        REQUIRE(!sameCandidateIdentity(identity,later));
    }
    auto provisional=identity;provisional.provisional=true;provisional.pass=5;
    LearnCandidateSnapshot interim;
    REQUIRE(scanLearnCandidates(store,saved.manifest,provisional,{},interim,error) && interim.identity.provisional);
    REQUIRE(interim.positions.size()==scan.positions.size());
    const auto before=scan.positions;
    REQUIRE(!scanLearnCandidates(store,saved.manifest,identity,{},scan,error,[]{return true;}));
    REQUIRE(scan.positions.size()==before.size());
    for(size_t i=0;i<before.size();++i)REQUIRE(scan.positions[i].sample==before[i].sample);
    // Splitting identical immutable input must converge to the same final snapshot.
    REQUIRE(capture.commit(-24000,rate,pcm.data(),frames/2));
    REQUIRE(capture.commit(24000,rate,pcm.data()+frames,frames/2));
    const auto split=capture.snapshot();identity.sourceReference=split.reference;identity.sourceRevision=split.manifest.revision;
    LearnCandidateSnapshot repeated;REQUIRE(scanLearnCandidates(store,split.manifest,identity,{},repeated,error));
    REQUIRE(repeated.positions.size()==before.size());
    for(size_t i=0;i<before.size();++i)REQUIRE(repeated.positions[i].sample==before[i].sample);
    REQUIRE(capture.snapshot().reference==split.reference);
    auto unavailable=split.manifest;
    unavailable.ranges.front().assetPath="segments/"+juce::Uuid().toDashedString().toStdString()+".wav";
    REQUIRE(!scanLearnCandidates(store,unavailable,identity,{},repeated,error));
    REQUIRE(repeated.positions.size()==before.size() && error.isNotEmpty());
});}
