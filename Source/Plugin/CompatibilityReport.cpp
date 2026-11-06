#include "PluginProcessor.h"
#if PREVERB_COMPATIBILITY_TEST
#include <BuildInformationData.h>

namespace
{
juce::String number(uint64_t n){return juce::String(static_cast<juce::int64>(n));}
juce::String field(bool available,double value){return available?juce::String(value,6):juce::String("unavailable");}
juce::String phase(workflow::Snapshot s)
{
    if(s.result==workflow::Result::failed)return "Capture rejected; previous committed source retained";
    switch(s.phase){case workflow::Phase::recording:return "Recording provisional source (not yet committed)";
    case workflow::Phase::draining:return "Finishing capture";case workflow::Phase::validating:return "Validating captured source";
    case workflow::Phase::failed:return "Capture rejected";case workflow::Phase::idle:break;}
    if(s.result==workflow::Result::accepted)return "Capture completed successfully";
    if(s.result==workflow::Result::empty)return "Pass completed without new useful source";
    if(s.result==workflow::Result::cancelled)return "Capture cancelled";
    return "No active capture pass";
}
juce::String observationText(const compatibility::Observation& o)
{
    const auto& t=o.timing;juce::String s;
    s<<juce::String(o.at,0)<<" ms | "<<compatibility::text(o.gate)
        <<" | "<<(o.nonzero?"input present":"silent input")
        <<" L="<<juce::String(o.left,6)<<" R="<<juce::String(o.right,6)
        <<" | playback "<<(t.position?(t.playing?"running":"stopped"):"unknown")
        <<" | "<<phase(o.flow)<<" | callbacks="<<number(o.callbacks)
        <<" attempt="<<int(o.attempt)<<" started="<<int(o.started)<<" discontinuity="<<int(o.discontinuity)
        <<" | "<<captureFailureReasonText(o.rejection)
        <<" | sample="<<(t.sample?juce::String(t.samplePosition):"unavailable")
        <<" seconds="<<field(o.secondsAvailable,o.seconds)<<" PPQ="<<field(t.ppq,t.ppqPosition)
        <<" BPM="<<field(t.tempo,t.bpm)<<" meter="<<(t.meter?juce::String(t.numerator)+"/"+juce::String(t.denominator):"unavailable")
        <<" raw host meter="<<(o.rawMeterAvailable?juce::String(o.rawNumerator)+"/"+juce::String(o.rawDenominator):"unavailable")
        <<" looping="<<int(t.looping)<<" loop PPQ="<<field(o.loopAvailable,o.loopBegin)<<".."<<field(o.loopAvailable,o.loopEnd)
        <<" rate="<<juce::String(t.rate)<<" frames="<<o.frames<<" channels="<<o.channels
        <<" pass="<<number(o.flow.pass)<<" queued="<<number(o.queued)<<" dropped="<<number(o.dropped)
        <<" nonfinite="<<int(o.nonfinite);
    return s;
}
}
juce::String PreverbAudioProcessor::compatibilityReport(bool detailed)
{
    auto& model=compatibilityTelemetry;model.drain();const auto& o=model.latest;
    const auto flow=getWorkflowSnapshot();const auto capture=getCaptureSnapshot();
    const auto age=model.hasLatest?juce::Time::getMillisecondCounterHiRes()-o.at:0;
    juce::String s="ReverbArise Compatibility Test - experimental private diagnostic\n";
    s<<"Audio: "<<(!model.hasLatest?"No audio callbacks observed":age>1500?"No recent audio callbacks (host may be idle)":"Audio callbacks arriving")
        <<"\nInput: "<<(!model.hasLatest?"unknown":o.nonzero?"signal received": "silent in latest observation")
        <<" | Left "<<juce::String(o.left,6)<<" / Right "<<juce::String(o.right,6)
        <<" | non-zero ever observed this instance: "<<(model.everNonzero.load()?"yes":"no")
        <<"\nHost playback: "<<(!model.hasLatest || !o.timing.position?"unknown (host information missing)":o.timing.playing?"running":"stopped")
        <<"\nReported tempo: "<<(model.hasLatest && o.timing.tempo?juce::String(o.timing.bpm,3)+" BPM - compare with your intended tempo; no correction applied":"unavailable")
        <<"\nLast callback start gate: "<<(model.hasLatest?compatibility::text(o.gate):"unknown until audio callbacks arrive")
        <<"\n"<<phase(flow)<<" | committed regions: "<<number(capture.manifest.ranges.size())
        <<"\nCapture worker: "<<capture.status
        <<"\n"<<(capture.failure?juce::String("Last rejected pass (historical): ")+captureFailureReasonText(capture.failure.event.reason):juce::String("No recorded capture failure"))
        <<"\nOpen plugin, start patch/playback, attempt capture, stop, then Copy report.\n";
#if PREVERB_COMPATIBILITY_METER_4_4
    s<<"EXPERIMENT: fixed 4/4 supplied ONLY when host time signature is unavailable. Host meter takes precedence.\n";
    if(model.hasLatest)s<<"Raw host time signature: "<<(!o.timing.position?"unknown (host position unavailable; no fallback)":o.rawMeterAvailable?juce::String(o.rawNumerator)+"/"+juce::String(o.rawDenominator):"unavailable; using experimental 4/4")<<"\n";
#else
    if(model.hasLatest && !o.timing.meter)s<<"Missing meter alone prevents capture. This does not establish inconsistent sample/PPQ mapping.\n";
#endif
    if(!detailed){
        s<<"\nRecent activity (older entries retained in Copy report):\n";
        const auto first=model.history.size()>8?model.history.size()-8:0;
        for(size_t i=first;i<model.history.size();++i){const auto& e=model.history[i];
            s<<juce::String(e.at/1000,1)<<" s: "<<compatibility::text(e.gate)<<" | "<<phase(e.flow)
                <<" | "<<(e.nonzero?"input received":"silent input")<<"\n";}
        s<<"Older entries truncated: "<<number(model.truncated)<<"; observations omitted: "<<number(model.omitted.load()-model.omittedBaseline)<<"\n";
        if(!model.settlementHistory.empty()){const auto& last=model.settlementHistory.back();s<<"Worker confirmed pass "<<number(last.pass)<<": "<<(last.useful?"capture completed successfully":"completed without new useful source")<<"\n";}
        return s;
    }
    s<<"\nVersion: "<<juce::String(m3::build::version.data())<<" | source SHA: "<<juce::String(m3::build::gitSha.data())
        <<" | build: "<<juce::String(m3::build::identifier.data())<<" | dirty="<<int(m3::build::worktreeDirty)
        <<"\nVariant: PREVERB_COMPATIBILITY_TEST=ON | Windows Release required for delivery"
        <<" | PREVERB_COMPATIBILITY_METER_4_4="<<int(PREVERB_COMPATIBILITY_METER_4_4)
        <<"\nHost detected: "<<juce::PluginHostType().getHostDescription()<<" | wrapper: "<<juce::AudioProcessor::getWrapperTypeDescription(wrapperType)
        <<"\nOS: "<<juce::SystemStats::getOperatingSystemName()<<" | sample rate "<<juce::String(getSampleRate())
        <<" | configured block "<<getBlockSize()<<" | buses "<<getTotalNumInputChannels()<<" in / "<<getTotalNumOutputChannels()<<" out"
        <<"\nCallbacks lifetime="<<number(model.callbackCount.load())<<" since Clear="<<number(model.callbackCount.load()-model.callbackBaseline)
        <<" | capture dropped lifetime="<<number(getDroppedCaptureBlockCount())<<" | capture failure events omitted="<<number(getOmittedCaptureFailureCount())
        <<"\nHistory: "<<number(model.history.size())<<"/128; older observations truncated="<<number(model.truncated)
        <<"; audio observations omitted since Clear="<<number(model.omitted.load()-model.omittedBaseline)
        <<"\nTimestamps: monotonic milliseconds, not song time. Unchanged callbacks coalesced; periodic observation at most once/second."
        <<"\nJUCE exposes playback/looping as booleans when PositionInfo exists; individual boolean-field availability is not exposed. Seconds are observed only, never used for capture admission."
        <<"\nClear resets diagnostic history only; lifetime counters and accepted source are retained.\n";
    if(model.hasLatest)s<<"\nLatest observation:\n"<<observationText(o)<<"\n";
    if(capture.failure){const auto& f=capture.failure;s<<"\nHistorical worker rejection: "<<captureFailureReasonText(f.event.reason)<<" pass="<<number(f.event.pass)<<" queued="<<number(f.event.queued)<<" dropped="<<number(f.finalDropped)<<"\n";}
    s<<"\nBounded activity history:\n";for(const auto& event:model.history)s<<observationText(event)<<"\n";
    s<<"\nWorker confirmations (retained even when host callbacks stop):\n";
    for(const auto& e:model.settlementHistory)s<<juce::String(e.at,0)<<" ms | pass "<<number(e.pass)<<" | "<<(e.useful?"capture completed successfully":"completed without new useful source")<<"\n";
    s<<"Older confirmations truncated="<<number(model.settlementTruncated)<<"; omitted="<<number(model.settlementOmitted.load()-model.settlementOmittedBaseline)<<"\n";
    return s;
}
#endif
