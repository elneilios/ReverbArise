#pragma once
#include "AutomaticTargets.h"
#include <Core/TransportTime.h>
#include <Capture/CaptureSession.h>
#include <memory>

namespace m3
{
struct CandidateIdentity
{
    uint64_t session=0,captureGeneration=0,sourceRevision=0,settings=0,pass=0,boundaries=0;
    int policy=attackPolicyVersion;
    juce::String sourceReference,scope;
    SongTime timing;
    std::optional<LoopPhraseTiming> loop;
    float sensitivity=.5f;
    double separationSeconds=.08;
    bool provisional=false;
};
inline bool sameCandidateIdentity(const CandidateIdentity& a,const CandidateIdentity& b)
{
    return a.session==b.session && a.captureGeneration==b.captureGeneration && a.sourceRevision==b.sourceRevision
        && a.settings==b.settings && a.pass==b.pass && a.boundaries==b.boundaries && a.policy==b.policy && a.sourceReference==b.sourceReference && a.scope==b.scope
        && a.sensitivity==b.sensitivity && a.separationSeconds==b.separationSeconds && a.provisional==b.provisional
        && a.loop==b.loop && (a.loop.has_value() || transportMappingMatches(a.timing,b.timing));
}
struct LearnCandidateSnapshot
{
    CandidateIdentity identity;
    std::vector<Candidate> positions;
    CompletedPasses mappedRuns,completedPasses;
    uint64_t serial=0;
};
struct LearnGenerationView
{
    std::shared_ptr<const LearnCandidateSnapshot> displayed;
    bool updating=false,generating=false,canGenerate=false;
    juce::String reason,notice;
    bool failed=false;
};
// Safe immutable staged chunks only; no pending callback or worker PCM buffers.
using ProvisionalCandidateSource=ProvisionalCaptureSnapshot;
bool scanLearnCandidates(CaptureAssetStore&,const CaptureManifest&,const CandidateIdentity&,
    const CompletedPasses&,LearnCandidateSnapshot&,juce::String&,
    const std::function<bool()>& cancelled={});
}
