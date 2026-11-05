#pragma once
#include <DSP/SwellEngine.h>
#include <Core/HostGrid.h>

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
        +"\nRun clock gate: "+liveGateText(d.clockGate)+"\nLive reference: "+liveTimingText(d.liveReference);
    if(d.usesLiveClock){long double a,b;
        evidence+="\nRun reference / observed / delta samples: ";
        evidence+=songOrigin(d.liveReference,a) && songOrigin(d.observed,b)
            ?juce::String(static_cast<double>(a),12)+" / "+juce::String(static_cast<double>(b),12)+" / "+juce::String(static_cast<double>(b-a),12):"unavailable";
    }
    evidence+="\nObserved: "+liveTimingText(d.observed);
    if(d.hasPublication)evidence+="\nPublished: "+liveTimingText(d.published);
    evidence+="\nOrigins published / observed / delta samples: ";
    evidence+=d.originsValid?juce::String(d.publishedOrigin,12)+" / "+juce::String(d.observedOrigin,12)+" / "+juce::String(d.originDelta,12):"unavailable";
    evidence+="\nWindow ["+juce::String(d.start)+", "+juce::String(d.end)+") loop ["+juce::String(d.loopStart)+", "+juce::String(d.loopEnd)
        +") | count "+juce::String(d.count)+" valid "+juce::String(d.validCount?1:0);
    evidence+="\nHost looping "+juce::String(d.hostLooping?1:0)+" | bounds ["+juce::String(d.hostLoopStart)+", "+juce::String(d.hostLoopEnd)+")";
    evidence+="\nPlaying / workflow / enabled / running / missing: "+juce::String(d.playing?1:0)+" / "+juce::String(d.workflowAllows?1:0)
        +" / "+juce::String(d.enabled?1:0)+" / "+juce::String(d.running?1:0)+" / "+juce::String(d.missing?1:0)
        +" | route "+(d.wetOnly?juce::String("Wet Only"):juce::String("Dry + Wet"))+" | linear wet gain "+juce::String(d.wetGain,9)+"\n";
    return evidence;
}
inline juce::String liveDiagnosticsText(const EditorSnapshot& state)
{
    auto evidence=liveCallbackText(state.liveObservation,"Current callback");
    evidence+="\n"+liveCallbackText(state.liveLastRejection,"Historical last rejection (not current)");
    const auto causal=[&](const char* title,auto member,auto available){
        return liveCallbackText(state.liveCausal && ((*state.liveCausal).*available)?std::optional<LiveCallbackObservation>{(*state.liveCausal).*member}:std::nullopt,title);};
    evidence+="\n"+causal("Run start callback",&LiveCausalObservation::runStart,&LiveCausalObservation::hasStart);
    evidence+="\n"+causal("Last admitted callback in retained run",&LiveCausalObservation::lastAdmitted,&LiveCausalObservation::hasAdmitted);
    evidence+="\n"+causal("Preceding admitted callback at first rejection",&LiveCausalObservation::precedingAdmitted,&LiveCausalObservation::hasPreceding);
    evidence+="\n"+causal("First timing rejection in retained run",&LiveCausalObservation::firstRejection,&LiveCausalObservation::hasRejection);
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
inline juce::String gridDiagnosticsText(const GridEvidence& d)
{
    const auto reason=[](GridGate gate){switch(gate){
        case GridGate::available:return "available";case GridGate::sampleUnavailable:return "sample position unavailable";
        case GridGate::captureRate:return "capture-rate mismatch/unavailable";case GridGate::invalidTiming:return "invalid timing/origin";
        case GridGate::rate:return "host sample-rate mismatch";case GridGate::tempo:return "tempo mismatch";
        case GridGate::meter:return "meter mismatch";case GridGate::origin:return "accepted-grid origin mismatch";
        case GridGate::division:return "invalid grid division";case GridGate::conversion:return "grid conversion unavailable";
    }return "unavailable";};
    auto gridText=juce::String("Fixed grid (UI snapshot; independent of callback admission): ")+reason(d.gate);
    gridText+="\nGrid sample position available: "+juce::String(d.sampleAvailable?1:0)+" | capture Hz "+juce::String(d.captureRate,6)
        +" | division "+juce::String(static_cast<int>(d.division));
    gridText+="\nGrid accepted: "+(d.accepted.valid?liveTimingText(d.accepted):juce::String("unavailable"));
    gridText+="\nGrid observed: "+(d.sampleAvailable && d.observed.valid?liveTimingText(d.observed):juce::String("unavailable"));
    long double a,b;gridText+="\nGrid accepted / observed / delta samples: ";
    gridText+=d.sampleAvailable && songOrigin(d.accepted,a) && songOrigin(d.observed,b)
        ?juce::String(static_cast<double>(a),12)+" / "+juce::String(static_cast<double>(b),12)+" / "+juce::String(static_cast<double>(b-a),12):"unavailable";
    return gridText+"\n";
}
}
