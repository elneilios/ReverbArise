#include <Diagnostics/TimingTrace.h>
#include <Plugin/PluginProcessor.h>
#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Windows.h>
#include <thread>

namespace
{
using Info=juce::AudioPlayHead::PositionInfo;
Info at(int64_t sample,double ppq,double bpm,bool playing=true)
{
    Info info;info.setTimeInSamples(sample);info.setPpqPosition(ppq);info.setBpm(bpm);
    info.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});info.setIsPlaying(playing);return info;
}
juce::StringArray rows(const juce::File& file)
{
    juce::StringArray result;
    for(const auto& line:juce::StringArray::fromLines(file.loadFileAsString()))
        if(line.isNotEmpty() && !line.startsWithChar('#'))result.add(line);
    return result;
}
struct Head:juce::AudioPlayHead
{
    Info info;
    juce::Optional<Info> getPosition()const override{return info;}
};
}

int main(){return runTest([]{
    using namespace diagnostic;
    TestDirectory root;REQUIRE(root.file.createDirectory());
    // Ownership/pressure and concurrent release/acquire transfer, independent of I/O.
    {TraceQueue<2> queue;TimingTraceRecord record,read;
        record.callback=1;REQUIRE(queue.push(record));record.callback=2;REQUIRE(queue.push(record));
        record.callback=3;REQUIRE(!queue.push(record));REQUIRE(queue.pop(read) && read.callback==1);
        REQUIRE(queue.push(record));REQUIRE(queue.pop(read) && read.callback==2);
        REQUIRE(queue.pop(read) && read.callback==3);REQUIRE(!queue.pop(read));}
    {auto queue=std::make_unique<TraceQueue<16>>();std::atomic<bool> bad{false};
        std::thread producer([&]{for(uint64_t i=1;i<=10000;++i){TimingTraceRecord record;record.callback=i;record.sample=static_cast<int64_t>(i*3);
            while(!queue->push(record))std::this_thread::yield();}});
        for(uint64_t i=1;i<=10000;++i){TimingTraceRecord record;while(!queue->pop(record))std::this_thread::yield();
            if(record.callback!=i || record.sample!=static_cast<int64_t>(i*3))bad=true;}
        producer.join();REQUIRE(!bad.load());}
    juce::File traceFile;
    {auto trace=std::make_unique<TimingTrace>(root.file);traceFile=trace->file();
        const auto observe=[&](const juce::Optional<Info>& info,int frames,bool offline=false,uint64_t reset=1,uint64_t epoch=2){
            {AudioAllocationGuard guard;trace->observe(info,48000,frames,offline,reset,epoch,3);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        };
        observe(at(0,0,120,false),64);observe(at(0,0,120,false),64); // Repeated parked row suppressed.
        observe(at(0,0,120),64);observe(at(64,64./24000,120),127);
        observe(at(191,.008,90),31); // Raw step tuple is retained, not corrected.
        auto ramp=at(222,.009,90.125);ramp.setPpqPositionOfLastBarStart(0);
        ramp.setLoopPoints(juce::AudioPlayHead::LoopPoints{0,4});ramp.setIsLooping(true);observe(ramp,209);
        observe(juce::nullopt,209); // Missing data is retained, never a confirmed Stop.
        auto missing=at(0,0,91);missing.setPpqPosition(juce::nullopt);observe(missing,245);
        observe(at(245,.01,91,false),32);
        observe(at(8192,.3,92,false),512,true,2); // Offline with Playing false.
        observe(at(64,.002,92,false),512,true,2); // A backward seek.
        observe(at(INT64_MAX,.1,92),1,false,3,4); // Overflow is flagged, not computed.
        REQUIRE(!trace->failed() && trace->omittedRecords()==0);
    }
    const auto data=rows(traceFile);REQUIRE(data.size()==12); // Header + eleven records.
    const auto step=juce::StringArray::fromTokens(data[4],",","");REQUIRE(step[5]=="191" && step[7]=="90");
    const auto ramp=juce::StringArray::fromTokens(data[5],",","");
    REQUIRE(ramp[7].getDoubleValue()==90.125 && (ramp[15].getIntValue()&TraceFlag::cycle));
    const auto unavailable=juce::StringArray::fromTokens(data[6],",","");REQUIRE(!(unavailable[15].getIntValue()&TraceFlag::position));
    const auto missing=juce::StringArray::fromTokens(data[7],",","");REQUIRE(!(missing[15].getIntValue()&TraceFlag::ppq));
    const auto offline=juce::StringArray::fromTokens(data[9],",","");
    REQUIRE((offline[15].getIntValue()&TraceFlag::offline) && !(offline[15].getIntValue()&TraceFlag::playing));
    REQUIRE(offline[2]=="2" && offline[3]=="2" && offline[4]=="3");
    const auto seek=juce::StringArray::fromTokens(data[10],",","");
    REQUIRE(seek[15].getIntValue()&TraceFlag::discontinuity);
    const auto overflow=juce::StringArray::fromTokens(data[data.size()-1],",","");REQUIRE(overflow[15].getIntValue()&TraceFlag::invalidRange);
    REQUIRE(overflow[2]=="3" && overflow[3]=="4" && overflow[4]=="3");
    REQUIRE(traceFile.loadFileAsString().contains("omitted=0 io_failed=0"));
    {auto trace=std::make_unique<TimingTrace>(root.file.getChildFile("absent"));
        const auto info=at(0,0,120);{AudioAllocationGuard guard;trace->observe(info,48000,32,false,1,1,0);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        REQUIRE(trace->failed() && trace->omittedRecords()==1);}

    // A saturated diagnostic prefix discloses loss; it cannot mutate capture policy.
    {auto trace=std::make_unique<TimingTrace>(root.file);traceFile=trace->file();const auto info=at(0,0,120);
        {AudioAllocationGuard guard;for(uint64_t i=0;i<TimingTrace::maximumRecords+3;++i)trace->observe(info,48000,32,false,1,1,0);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);REQUIRE(trace->omittedRecords()==3);}
    REQUIRE(traceFile.loadFileAsString().contains("written=65536 omitted=3 io_failed=0"));

    // Exercise the actual processor hook and prove the original tempo rejection
    // and dry samples agree with an otherwise identical uninstrumented instance.
    const auto oldEnv=juce::SystemStats::getEnvironmentVariable("REVERBARISE_TIMING_TRACE_DIR",{});
    struct RestoreEnvironment {juce::String value;~RestoreEnvironment(){SetEnvironmentVariableW(L"REVERBARISE_TIMING_TRACE_DIR",value.isEmpty()?nullptr:value.toWideCharPointer());}} restore{oldEnv};
    SetEnvironmentVariableW(L"REVERBARISE_TIMING_TRACE_DIR",nullptr);
    auto baseline=std::make_unique<PreverbAudioProcessor>(root.file.getChildFile("baseline"));
    SetEnvironmentVariableW(L"REVERBARISE_TIMING_TRACE_DIR",root.file.getFullPathName().toWideCharPointer());
    auto observed=std::make_unique<PreverbAudioProcessor>(root.file.getChildFile("observed"));
    Head head;head.info=at(0,0,120);baseline->setPlayHead(&head);observed->setPlayHead(&head);
    baseline->swellEngine().setLearningAutomatic(false);observed->swellEngine().setLearningAutomatic(false);
    baseline->prepareToPlay(48000,512);observed->prepareToPlay(48000,512);
    juce::MidiBuffer midi;juce::AudioBuffer<float> a(2,512),b(2,512);
    for(int block=0;block<12;++block){
        head.info=at(block*512,block*512./24000,block<4?120:130+block*.125);
        for(int c=0;c<2;++c)for(int n=0;n<512;++n)a.setSample(c,n,(c?-.1f:.2f));b.makeCopyOf(a);
        baseline->processBlock(a,midi);observed->processBlock(b,midi);
        for(int c=0;c<2;++c)for(int n=0;n<512;++n)REQUIRE(a.getSample(c,n)==b.getSample(c,n));
    }
    REQUIRE(baseline->getDroppedCaptureBlockCount()==observed->getDroppedCaptureBlockCount());
    REQUIRE(observed->getDroppedCaptureBlockCount()>0); // No tempo-policy repair slipped in.
    observed.reset();baseline.reset();
    std::cout<<"trace record bytes="<<sizeof(TimingTraceRecord)<<" queue bytes="<<sizeof(TraceQueue<TimingTrace::maximumRecords>)<<"\n";
});}
