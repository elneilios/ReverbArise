#pragma once
#include <Capture/CaptureFailureRecord.h>

namespace m3::ui
{
inline juce::String captureFailureText(const CaptureFailureRecord& failure,bool current)
{
    if(!failure)return {};
    const auto& e=failure.event;
    const auto number=[](uint64_t value){return juce::String(static_cast<juce::int64>(value));};
    juce::String output=current?"Current failed pass: ":"Last failed pass (historical): ";
    output<<captureFailureReasonText(e.reason)<<" | pass "<<number(e.pass)<<" epoch "<<number(e.epoch)
        <<" | attempted start "<<juce::String(e.start)<<" frames "<<e.frames
        <<" | queued "<<number(e.queued)<<" dropped at failure "<<number(e.dropped)<<" final "<<number(failure.finalDropped)
        <<" | queue "<<number(e.depth)<<"/"<<number(e.capacity)
        <<"\nAccepted source: "<<failure.project<<"/"<<failure.instance<<" | "<<failure.reference<<" revision "<<number(failure.revision)
        <<"\nFailure timing: sample "<<(e.timing.sample?juce::String(e.timing.samplePosition):"unavailable")
        <<" PPQ "<<(e.timing.ppq?juce::String(e.timing.ppqPosition,6):"unavailable")
        <<" BPM "<<(e.timing.tempo?juce::String(e.timing.bpm,3):"unavailable")
        <<" meter "<<(e.timing.meter?juce::String(e.timing.numerator)+"/"+juce::String(e.timing.denominator):"unavailable")
        <<" rate "<<juce::String(e.timing.rate,0)<<" playing "<<int(e.timing.playing)<<" looping "<<int(e.timing.looping)
        <<" ["<<juce::String(e.timing.loopStart)<<","<<juce::String(e.timing.loopEnd)<<")"
        <<"\nPass baseline: sample "<<(e.baseline.sample?juce::String(e.baseline.samplePosition):"unavailable")
        <<" PPQ "<<(e.baseline.ppq?juce::String(e.baseline.ppqPosition,6):"unavailable")
        <<" BPM "<<(e.baseline.tempo?juce::String(e.baseline.bpm,3):"unavailable")
        <<" rate "<<juce::String(e.baseline.rate,0)<<" | "<<failure.detail;
    return output;
}
}
