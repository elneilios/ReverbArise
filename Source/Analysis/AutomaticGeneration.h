#pragma once
#include "AutomaticTargets.h"

namespace m3
{
enum class GenerationOutcome { generated, empty, protectedOnly, noChange, failed, cancelled };
struct AutomaticGenerationPlan
{
    GenerationOutcome outcome=GenerationOutcome::failed;
    Session replacement;
    std::vector<juce::String> changed,retired;
    juce::String message;
};
// Non-realtime preparation only. Fingerprinting reads immutable source; the
// planner never writes assets or changes the supplied accepted session.
using GenerationFingerprint=std::function<bool(int64_t,int64_t,juce::String&,juce::String&)>;
AutomaticGenerationPlan prepareAutomaticGeneration(const Session&,const std::vector<Candidate>&,
    const CaptureManifest&,const SongTime&,const CompletedPasses&,const GenerationFingerprint&,
    const std::function<bool()>& cancelled={});
}
