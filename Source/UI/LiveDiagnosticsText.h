#pragma once
#include <DSP/SwellEngine.h>

namespace m3::ui
{
inline const char* liveGateText(LiveGate gate)
{
    switch(gate){
        case LiveGate::admitted:return "admitted";
        case LiveGate::workflow:return "workflow excludes live";
        case LiveGate::disabled:return "live disabled";
        case LiveGate::stopped:return "stopped (not playing admission)";
        case LiveGate::noPublication:return "no publication";
        case LiveGate::invalidRange:return "invalid callback range";
        case LiveGate::invalidTiming:return "invalid timing/origin";
        case LiveGate::rate:return "sample-rate mismatch";
        case LiveGate::tempo:return "tempo mismatch";
        case LiveGate::meter:return "meter mismatch";
        case LiveGate::origin:return "PPQ-zero origin mismatch";
        case LiveGate::reset:return "reset transition";
        case LiveGate::missingCoverage:return "missing prepared coverage";
    }
    return "unknown";
}
inline juce::String liveTimingText(const SongTime& t)
{
    return "sample "+juce::String(t.sample)+" | PPQ "+juce::String(t.ppq,15)
        +" | Hz "+juce::String(t.rate,6)+" | BPM "+juce::String(t.bpm,12)
        +" | "+juce::String(t.numerator)+"/"+juce::String(t.denominator)+" | valid "+juce::String(t.valid?1:0);
}
inline juce::String liveCallbackText(const std::optional<LiveCallbackObservation>& observation,const juce::String& title)
{
    if(!observation)return title+": unavailable\n";
    const auto& d=*observation;
    juce::String evidence=title+" | block "+juce::String(static_cast<juce::int64>(d.block))+" | "+liveGateText(d.gate)
        +" | publication "+juce::String(d.hasPublication?1:0)+" revision "+juce::String(static_cast<juce::int64>(d.revision));
    if(d.usesLiveClock)evidence+="\nLive run "+juce::String(static_cast<juce::int64>(d.liveRun))
        +" | session "+juce::String(static_cast<juce::int64>(d.liveSession))+" reset "+juce::String(static_cast<juce::int64>(d.liveReset))
        +"\nLive reference: "+liveTimingText(d.liveReference);
    evidence+="\nObserved: "+liveTimingText(d.observed);
    if(d.hasPublication)evidence+="\nPublished: "+liveTimingText(d.published);
    evidence+="\nOrigins published / observed / delta samples: ";
    evidence+=d.originsValid?juce::String(d.publishedOrigin,12)+" / "+juce::String(d.observedOrigin,12)+" / "+juce::String(d.originDelta,12):"unavailable";
    evidence+="\nWindow ["+juce::String(d.start)+", "+juce::String(d.end)+") loop ["+juce::String(d.loopStart)+", "+juce::String(d.loopEnd)
        +") | count "+juce::String(d.count)+" valid "+juce::String(d.validCount?1:0);
    evidence+="\nPlaying / workflow / enabled / running / missing: "+juce::String(d.playing?1:0)+" / "+juce::String(d.workflowAllows?1:0)
        +" / "+juce::String(d.enabled?1:0)+" / "+juce::String(d.running?1:0)+" / "+juce::String(d.missing?1:0)
        +" | route "+(d.wetOnly?juce::String("Wet Only"):juce::String("Dry + Wet"))+" | linear wet gain "+juce::String(d.wetGain,9)+"\n";
    return evidence;
}
inline juce::String liveDiagnosticsText(const EditorSnapshot& state)
{
    auto evidence=liveCallbackText(state.liveObservation,"Current callback");
    evidence+="\n"+liveCallbackText(state.liveLastRejection,"Historical last rejection (not current)");
    const auto& w=state.livePreparation;
    evidence+="\nWorker (separate observation) | service "+juce::String(static_cast<juce::int64>(w.service))+" | "+w.result;
    if(w.clock)evidence+="\nWorker live run "+juce::String(static_cast<juce::int64>(w.clock->run))+" | "+liveGateText(w.clock->gate)
        +" | session "+juce::String(static_cast<juce::int64>(w.clock->session))+"\nWorker run reference: "+liveTimingText(w.clock->reference);
    evidence+="\nService ms "+juce::String(w.servicedAtMs,3)+" | attempt "+juce::String(static_cast<juce::int64>(w.attempt))
        +" at ms "+juce::String(w.attemptedAtMs,3)+" | last attempt: "+w.lastAttemptResult;
    evidence+="\nRevision current / prepared: "+juce::String(static_cast<juce::int64>(w.revision))+" / "+juce::String(static_cast<juce::int64>(w.preparedRevision))
        +" | window ["+juce::String(w.start)+", "+juce::String(w.end)+") loop hint ["+juce::String(w.loopStart)+", "+juce::String(w.loopEnd)+")";
    evidence+="\nWorker source: "+liveTimingText(w.source)+"\nWorker observed: "+liveTimingText(w.observed)+"\nWorker latest: "+liveTimingText(w.latest);
    long double a,b;
    evidence+="\nWorker source / observed origins / delta samples: ";
    evidence+=songOrigin(w.source,a) && songOrigin(w.observed,b)?juce::String(static_cast<double>(a),12)+" / "+juce::String(static_cast<double>(b),12)+" / "+juce::String(static_cast<double>(b-a),12):"unavailable";
    return evidence;
}
}
