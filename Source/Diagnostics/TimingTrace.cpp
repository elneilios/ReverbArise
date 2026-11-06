#include "TimingTrace.h"
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

namespace diagnostic
{
std::unique_ptr<TimingTrace> TimingTrace::fromEnvironment()
{
    const auto path=juce::SystemStats::getEnvironmentVariable("REVERBARISE_TIMING_TRACE_DIR",{});
    if(path.isEmpty() || !juce::File::isAbsolutePath(path))return {};
    const juce::File directory(path);
    if(!directory.isDirectory())return {};
    // Diagnostic resource failure must not prevent loading the effect.
    try {return std::make_unique<TimingTrace>(directory);}
    catch(...) {return {};}
}
TimingTrace::TimingTrace(const juce::File& directory):Thread("ReverbArise timing trace")
{
    if(!directory.isDirectory()){ioFailed=true;return;}
    output=directory.getChildFile("timing-"+juce::Uuid().toDashedString()+".csv");
    stream=output.createOutputStream(16384);
    if(!stream || !stream->openedOk()){ioFailed=true;return;}
    const juce::String header="# ReverbArise diagnostic timing trace schema 1; raw callback observations; no timing authority\n"
        "# flags: position=1 sample=2 ppq=4 tempo=8 meter=16 playing=32 recording=64 looping=128 offline=256 bar=512 cycle=1024 discontinuity=2048 invalidRange=4096\n"
        "# maximum_records=65536; subsequent eligible rows omitted; repeated stopped callbacks omitted by design\n"
        "callback,run,reset,source_epoch,pass,sample,ppq,bpm,rate,bar_ppq,cycle_start_ppq,cycle_end_ppq,frames,numerator,denominator,flags\n";
    if(!stream->writeText(header,false,false,nullptr)){ioFailed=true;return;}
    if(!startThread())ioFailed=true;
}
TimingTrace::~TimingTrace()
{
    signalThreadShouldExit();stopThread(-1); // Outside the audio callback.
}
void TimingTrace::observe(const juce::Optional<juce::AudioPlayHead::PositionInfo>& info,
                         double rate,int frames,bool nonRealtime,uint64_t reset,uint64_t epoch,uint64_t pass) noexcept
{
    ++callback;
    const bool isPlaying=info && info->getIsPlaying();
    const bool newAuthority=!havePrevious || reset!=previousReset || epoch!=previousEpoch || nonRealtime!=previousOffline;
    // Keep one parked observation per authority and the confirmed Stop edge.
    // Invalid observations during a run remain visible, not synthesized Stops.
    const bool eligible=newAuthority || nonRealtime || isPlaying || previousPlaying;
    if(newAuthority || (isPlaying && !previousPlaying)){
        ++runId;expectedValid=false;
    }
    TimingTraceRecord record;
    record.callback=callback;record.run=runId;record.reset=reset;record.sourceEpoch=epoch;record.pass=pass;
    record.rate=rate;record.frames=frames;record.flags=nonRealtime?TraceFlag::offline:0u;
    if(info){
        record.flags|=TraceFlag::position;
        if(const auto value=info->getTimeInSamples()){record.flags|=TraceFlag::sample;record.sample=*value;}
        if(const auto value=info->getPpqPosition()){record.flags|=TraceFlag::ppq;record.ppq=*value;}
        if(const auto value=info->getBpm()){record.flags|=TraceFlag::tempo;record.bpm=*value;}
        if(const auto value=info->getTimeSignature()){
            record.flags|=TraceFlag::meter;record.numerator=value->numerator;record.denominator=value->denominator;
        }
        if(const auto value=info->getPpqPositionOfLastBarStart()){record.flags|=TraceFlag::bar;record.barPpq=*value;}
        if(const auto value=info->getLoopPoints()){
            record.flags|=TraceFlag::cycle;record.cycleStartPpq=value->ppqStart;record.cycleEndPpq=value->ppqEnd;
        }
        if(isPlaying)record.flags|=TraceFlag::playing;
        if(info->getIsRecording())record.flags|=TraceFlag::recording;
        if(info->getIsLooping())record.flags|=TraceFlag::looping;
    }
    const bool hasSample=(record.flags&TraceFlag::sample)!=0;
    if(expectedValid && hasSample && record.sample!=expected)record.flags|=TraceFlag::discontinuity;
    const bool validRange=hasSample && frames>0 && record.sample<=std::numeric_limits<int64_t>::max()-frames;
    if(!validRange)record.flags|=TraceFlag::invalidRange;
    expectedValid=validRange && (isPlaying || nonRealtime);
    if(expectedValid)expected=record.sample+frames;
    // Missing PositionInfo never establishes a confirmed stopped run.
    if(info)previousPlaying=isPlaying;
    previousReset=reset;previousEpoch=epoch;previousOffline=nonRealtime;havePrevious=true;
    if(!eligible)return;
    if(retained>=maximumRecords || ioFailed.load(std::memory_order_relaxed) || !queue.push(record)){
        omitted.fetch_add(1,std::memory_order_relaxed);return;
    }
    ++retained;
}
void TimingTrace::run()
{
    uint64_t count=0;
    for(;;){
        TimingTraceRecord record;
        bool drained=false;
        while(queue.pop(record)){
            drained=true;
            std::ostringstream row;row.imbue(std::locale::classic());row<<std::setprecision(17)
                <<record.callback<<','<<record.run<<','<<record.reset<<','<<record.sourceEpoch<<','<<record.pass<<','
                <<record.sample<<','<<record.ppq<<','<<record.bpm<<','<<record.rate<<','<<record.barPpq<<','
                <<record.cycleStartPpq<<','<<record.cycleEndPpq<<','<<record.frames<<','<<record.numerator<<','
                <<record.denominator<<','<<record.flags<<'\n';
            const auto text=row.str();
            if(!stream->write(text.data(),text.size()))ioFailed=true;
            ++count;
        }
        if(drained)stream->flush();
        if(threadShouldExit())break;
        wait(5);
    }
    const auto footer="# written="+juce::String(count)+" omitted="+juce::String(omitted.load())
        +" io_failed="+juce::String(ioFailed.load()?1:0)+" queue_bytes="+juce::String(sizeof(queue))+"\n";
    if(!stream->writeText(footer,false,false,nullptr))ioFailed=true;
    stream->flush();
    if(stream->getStatus().failed())ioFailed=true;
}
}
