#include "TestSupport.h"
#include <Analysis/AutomaticGeneration.h>
#include <Core/RenderDependency.h>
using namespace m3;

static Asset asset(char identity,bool render=false)
{
    Asset result;result.content=result.checksum=juce::String::repeatedString(juce::String::charToString(identity),64);
    result.path=juce::String(render?"renders/":"irs/")+result.content+".wav";
    result.rate=1000;result.channels=1;result.frames=32;return result;
}
int main(){return runTest([]
{
    SongTime timing{true,0,0,1000,120,4,4};CaptureManifest source;source.sampleRate=1000;
    source.ranges={{0,1000,CaptureCoverageKind::audio}};CompletedPasses complete{{0,1000}};
    Session accepted;accepted.defaultIR=asset('a');accepted.defaultIRName="A";accepted.defaultLevelDb=-4.f;
    const auto fingerprint=[](int64_t,int64_t,juce::String& result,juce::String&)
        {result=juce::String::repeatedString("a",64);return true;};
    const auto plan=[&](const Session& session,const std::vector<Candidate>& candidates,
                        GenerationPolicy policy=GenerationPolicy::reconcile)
        {return prepareAutomaticGeneration(session,candidates,source,timing,complete,fingerprint,{},policy);};
    const std::vector<Candidate> candidates{{600,1},{100,1},{100,2}};
    auto first=plan(accepted,candidates);
    REQUIRE(first.outcome==GenerationOutcome::generated && first.changed.size()==2 && accepted.swells.empty());
    auto saved=first.replacement;REQUIRE(saved.swells.size()==2);
    for(const auto& swell:saved.swells)
        REQUIRE(swell.shape.gainDb==-4.f && swell.soundPreset.id==customImpulseId);
    REQUIRE(saved.swells[0].sourceStart==100 && saved.swells[0].sourceEnd==600);
    REQUIRE(saved.swells[1].sourceStart==600 && saved.swells[1].sourceEnd==1000);
    const auto a=saved.swells[0].id,b=saved.swells[1].id;
    {
        auto shaped=accepted;
        shaped.defaultSoundPreset.shaping={50,230,1000};
        const auto added=plan(shaped,candidates);
        REQUIRE(added.outcome==GenerationOutcome::generated);
        for(const auto& swell:added.replacement.swells)
            REQUIRE(swell.soundPreset.shaping==shaped.defaultSoundPreset.shaping);
        auto existing=added.replacement;
        existing.defaultSoundPreset.shaping={100,20,20000};
        existing.defaultIR=asset('b');existing.defaultIRName="B";
        const auto regenerated=plan(existing,candidates);
        REQUIRE(regenerated.outcome==GenerationOutcome::generated && regenerated.changed.size()==2);
        for(const auto& swell:regenerated.replacement.swells)
            REQUIRE(swell.soundPreset.shaping==shaped.defaultSoundPreset.shaping && swell.ir==existing.defaultIR);
    }
    REQUIRE(saved.swells[0].generationSource==juce::String::repeatedString("a",64));
    REQUIRE(plan(saved,candidates).outcome==GenerationOutcome::noChange);
    // Older session metadata can lack generationSource after a wet-render upgrade.
    auto upgraded=saved;
    for(auto& w:upgraded.swells)
    {
        w.render=asset('a',true);w.render.dependency=juce::String(sourceEdgeRevision)+w.generationSource;
        w.generationSource.clear();
    }
    REQUIRE(plan(upgraded,candidates).outcome==GenerationOutcome::noChange);
    upgraded.swells[0].render.dependency="unknown-processing:"+saved.swells[0].generationSource;
    REQUIRE(plan(upgraded,candidates).changed==std::vector<juce::String>{a});
    // A nonempty subset is an arrangement change even when survivors need no render.
    REQUIRE(select(saved,{a,b},a));
    const auto reduced=plan(saved,{{100,1}});
    REQUIRE(reduced.outcome==GenerationOutcome::generated && reduced.changed.empty());
    REQUIRE(reduced.retired==std::vector<juce::String>{b} && reduced.replacement.swells.size()==1);
    REQUIRE(sameSavedSwell(*find(reduced.replacement,a),*find(saved,a)));
    REQUIRE(reduced.replacement.primary==a && reduced.replacement.selected==std::vector<juce::String>{a});
    REQUIRE(plan(reduced.replacement,{{100,1}}).outcome==GenerationOutcome::noChange);
    REQUIRE(saved.swells.size()==2);
    REQUIRE(plan(saved,{}).outcome==GenerationOutcome::empty && saved.swells.size()==2);
    REQUIRE(select(saved,{a,b},a));saved.swells[0].enabled=false;saved.swells[1].muted=true;
    saved.defaultIR=asset('b');saved.defaultIRName="B";saved.defaultLevelDb=5.f;
    auto replaced=plan(saved,candidates);
    REQUIRE(replaced.outcome==GenerationOutcome::generated && replaced.changed.size()==2);
    REQUIRE(find(replaced.replacement,a) && find(replaced.replacement,b));
    REQUIRE(!find(replaced.replacement,a)->enabled && find(replaced.replacement,b)->muted);
    REQUIRE(find(replaced.replacement,a)->ir==saved.defaultIR && replaced.replacement.selected==saved.selected);
    REQUIRE(find(replaced.replacement,a)->shape.gainDb==-4.f && find(replaced.replacement,b)->shape.gainDb==-4.f);
    REQUIRE(replaced.replacement.primary==a && saved.swells[0].ir==accepted.defaultIR);
    auto protectedSession=saved;protectedSession.swells[0].provenance=Provenance::manualEdited;
    protectedSession.swells[0].name="Manual edit";protectedSession.swells[0].placement=-30;
    protectedSession.swells[1].frozen=true;protectedSession.swells[1].render=asset('a',true);
    const auto protectedA=protectedSession.swells[0],protectedB=protectedSession.swells[1];
    auto protectedOnly=plan(protectedSession,candidates);
    REQUIRE(protectedOnly.outcome==GenerationOutcome::protectedOnly);
    auto mixed=plan(protectedSession,{{100,1},{600,1},{850,1}});
    REQUIRE(mixed.outcome==GenerationOutcome::generated && mixed.changed.size()==1);
    REQUIRE(sameSavedSwell(*find(mixed.replacement,a),protectedA));
    REQUIRE(sameSavedSwell(*find(mixed.replacement,b),protectedB));
    REQUIRE(find(mixed.replacement,b)->render==protectedB.render);
    auto suppressed=accepted;suppressed.suppressed={100,600};
    REQUIRE(plan(suppressed,candidates).outcome==GenerationOutcome::protectedOnly);
    const auto rediscovered=plan(suppressed,candidates,GenerationPolicy::rebuildUnfrozen);
    REQUIRE(rediscovered.outcome==GenerationOutcome::generated && rediscovered.changed.size()==2);
    REQUIRE(rediscovered.replacement.suppressed==suppressed.suppressed);
    auto rebuildInput=protectedSession;
    Swell outsideEdit=protectedA;outsideEdit.id=juce::Uuid().toString();
    outsideEdit.sourceStart=outsideEdit.arrival=outsideEdit.detectorAnchor=1200;outsideEdit.sourceEnd=1300;
    rebuildInput.swells.push_back(outsideEdit);
    const auto rebuilt=plan(rebuildInput,{{100,1},{600,1}},GenerationPolicy::rebuildUnfrozen);
    REQUIRE(rebuilt.outcome==GenerationOutcome::generated && rebuilt.changed.size()==1 && rebuilt.retired.size()==2);
    REQUIRE(!find(rebuilt.replacement,a) && !find(rebuilt.replacement,outsideEdit.id));
    REQUIRE(find(rebuilt.replacement,b) && sameSavedSwell(*find(rebuilt.replacement,b),protectedB));
    REQUIRE(plan(rebuildInput,{{600,1}},GenerationPolicy::rebuildUnfrozen).outcome==GenerationOutcome::protectedOnly);
    REQUIRE(plan(rebuildInput,{},GenerationPolicy::rebuildUnfrozen).outcome==GenerationOutcome::empty);

    source.ranges={{-200,600,CaptureCoverageKind::audio},{1000,400,CaptureCoverageKind::silence}};
    complete={{-200,400},{1000,1400}};
    Session sparse=accepted;Swell outside;outside.arrival=outside.sourceStart=outside.detectorAnchor=600;
    outside.sourceEnd=650;outside.provenance=Provenance::automatic;outside.ir=asset('b');
    outside.name="Outside mapped scope";sparse.swells.push_back(outside);
    auto sparsePlan=plan(sparse,{{-100,1},{1100,1},{600,1}});
    REQUIRE(sparsePlan.outcome==GenerationOutcome::generated && sparsePlan.changed.size()==2);
    REQUIRE(sameSavedSwell(*find(sparsePlan.replacement,outside.id),outside));
    REQUIRE(sparsePlan.replacement.swells.size()==3);
    for(const auto& w:sparsePlan.replacement.swells)
        REQUIRE(w.id==outside.id || (w.sourceStart>=-200 && w.sourceEnd<=400) || (w.sourceStart>=1000 && w.sourceEnd<=1400));
    complete.clear();REQUIRE(plan(accepted,{{1100,1}}).outcome!=GenerationOutcome::generated);
    complete={{-200,400},{1000,1400}};
    unsigned calls=0;
    const auto fail=[&](int64_t,int64_t,juce::String&,juce::String& error)
        {++calls;error="Injected source validation failure";return false;};
    const auto failed=prepareAutomaticGeneration(sparse,{{-100,1},{1100,1}},source,timing,complete,fail);
    REQUIRE(failed.outcome==GenerationOutcome::failed && calls==1 && sparse.swells.size()==1);
    const auto cancelled=prepareAutomaticGeneration(sparse,{{-100,1}},source,timing,complete,fingerprint,[]{return true;});
    REQUIRE(cancelled.outcome==GenerationOutcome::cancelled && sparse.swells.size()==1);
    REQUIRE(plan(accepted,std::vector<Candidate>(maximumSwells+1)).outcome==GenerationOutcome::failed);
    Session dense=accepted;
    for(size_t i=0;i<maximumSwells;++i)
    {
        Swell w;w.arrival=w.sourceStart=w.detectorAnchor=1100;w.sourceEnd=1300;
        w.provenance=Provenance::automatic;dense.swells.push_back(std::move(w));
    }
    std::vector<Candidate> close;
    for(int64_t at=1060;at<=1140;++at)close.push_back({at,1});
    REQUIRE(plan(dense,close).outcome==GenerationOutcome::failed && dense.swells.size()==maximumSwells);
    for(auto& w:dense.swells)w.provenance=Provenance::manualCreated;
    REQUIRE(plan(dense,{{-100,1}}).outcome==GenerationOutcome::failed && dense.swells.size()==maximumSwells);
    // A real source change is generation even at identical positions/settings.
    const auto changedSource=[](int64_t,int64_t,juce::String& result,juce::String&)
        {result=juce::String::repeatedString("b",64);return true;};
    source.ranges={{0,1000,CaptureCoverageKind::audio}};complete={{0,1000}};
    auto revised=prepareAutomaticGeneration(first.replacement,candidates,source,timing,complete,changedSource);
    REQUIRE(revised.outcome==GenerationOutcome::generated && revised.changed.size()==2);
    REQUIRE(find(revised.replacement,a)->id==a && find(revised.replacement,b)->id==b);
    Session ties=accepted;Swell left,right;left.arrival=left.sourceStart=left.detectorAnchor=100;left.sourceEnd=600;
    right.arrival=right.sourceStart=right.detectorAnchor=120;right.sourceEnd=620;
    left.provenance=right.provenance=Provenance::automatic;ties.swells={right,left};
    const auto tie=plan(ties,{{110,1}});REQUIRE(tie.outcome==GenerationOutcome::generated && tie.replacement.swells.size()==1);
    REQUIRE(tie.replacement.swells.front().id==std::min(left.id,right.id));
});}
