#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <Core/MonitoringSafety.h>
#include <Core/ContentHash.h>
#include <DSP/SwellEngine.h>
#include <source_location>
using namespace m3;
template<class F>static void waitFor(F predicate,const std::source_location line=std::source_location::current())
{const auto end=juce::Time::getMillisecondCounter()+30000;while(!predicate()){
    if(juce::Time::getMillisecondCounter()>end)throw std::runtime_error("Export engine timeout "+std::to_string(line.line()));juce::Thread::sleep(2);}}
int main(){return runTest([]{
    TestDirectory root;CaptureSession capture(root.file);std::vector<float> source(10000,.125f);
    for(int pass=0;pass<16;++pass){for(int n=0;n<5000;++n){source[n*2]=.04f+pass*.003f+(n%31)*.0001f;source[n*2+1]=.03f+pass*.002f+(n%17)*.0002f;}
        REQUIRE(capture.commit(pass*5000,1000,source.data(),5000));}const PreviewContext context{capture.snapshot(),{true,950,3.8,1000,240,4,4},0};
    workflow::State flow;std::atomic<unsigned> changes{0},missingTimeReads{0};std::atomic<bool> missingTime{false};
    std::atomic<bool> publicationMissingTime{false};std::atomic<unsigned> publicationMisses{0};
    juce::File publicationDirectory;int publicationFileCount=0;
    SwellEngine engine(root.file,[&]{auto next=context;if(missingTime.load()){next.time.valid=false;++missingTimeReads;}
        // A new final-name file exists only after publishSongExport completes.
        // The first provider read after it appears is the final publication check.
        if(publicationMissingTime.load() && publicationDirectory.findChildFiles(juce::File::findFiles,false,"Swells-*.wav").size()>publicationFileCount){
            next.time.valid=false;++publicationMisses;}
        return next;},[&]{++changes;});engine.setAutomaticProcessing(false);
    waitFor([&]{return engine.savedSession().defaultIR.present();});
    REQUIRE(engine.addTarget(1000));const auto id=engine.savedSession().primary;
    const auto ir=root.file.getChildFile("accepted.wav");writeTestWav(ir,1,1000,std::vector<float>(201,.001f));
    engine.loadIR(ir);waitFor([&]{return find(engine.savedSession(),id)->irName=="accepted.wav";});
    engine.generate();waitFor([&]{return find(engine.savedSession(),id)->phase==Phase::ready;});
    REQUIRE(flow.requestMode(workflow::Mode::live,{true,true,true}));engine.useWorkflow(flow);
    engine.holdExportForTesting(true);REQUIRE(flow.requestMode(workflow::Mode::finalise,{true,true,true}));
    REQUIRE(engine.initializeExport());waitFor([&]{return engine.exportWaitingForTesting();});
    REQUIRE(engine.setExportMix(.5f,engine.assignmentEpoch()));REQUIRE(engine.setExportMix(.65f,engine.assignmentEpoch()));
    waitFor([&]{return engine.savedSession().exportState.desiredIdentity.isNotEmpty() && engine.exportWaitingForTesting();});
    const auto pending=engine.savedSession();REQUIRE(pending.exportState.wet==.65f && pending.exportState.completedPath.isEmpty());
    engine.holdExportForTesting(false);waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    auto ready=engine.snapshot().songExport.prepared;juce::String error;REQUIRE(ready && ready->file.wet==.65f && verifyExportFile(engine.assetDirectory(),ready->file,error));
    const auto auditionOracle=[&](const std::shared_ptr<const PreparedExport>& ready){
    constexpr int quantum=127; // Unaligned pages; the separate seek oracle below also covers 37-frame blocks.
    const auto requestedStart=engine.snapshot().songExport.previewPosition.value_or(*ready->metrics.audibleStart);
    REQUIRE(engine.playExport(engine.assignmentEpoch()));
    std::array<float,quantum> left{},right{};std::vector<float> heard;
    const auto process=[&](int frames){left.fill(0);right.fill(0);{AudioAllocationGuard guard;engine.processAudio(left.data(),right.data(),frames,context.time);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);};
    waitFor([&]{process(quantum);return engine.snapshot().songExport.cursor.has_value();});
    engine.requestExportDetail(*ready->metrics.audibleStart,std::min(*ready->metrics.audibleEnd,*ready->metrics.audibleStart+4096),1024,engine.assignmentEpoch(),ready->plan->identity);
    for(int n=0;n<quantum;++n){heard.push_back(left[n]);heard.push_back(right[n]);}
    const auto countFrames=*ready->metrics.audibleEnd-requestedStart;
    for(int64_t at=quantum;at<countFrames;){const auto count=static_cast<int>(std::min(int64_t{quantum},countFrames-at));process(count);
        for(int n=0;n<count;++n){heard.push_back(left[n]);heard.push_back(right[n]);}at+=count;juce::Thread::sleep(1);}
    juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(engine.assetDirectory().getChildFile(ready->file.path).createInputStream().release(),true));
    juce::AudioBuffer<float> decoded(2,static_cast<int>(countFrames));REQUIRE(reader && reader->read(&decoded,0,static_cast<int>(countFrames),requestedStart-ready->file.origin,true,true));
    for(int64_t at=0;at<countFrames;++at)for(int c=0;c<2;++c)
        REQUIRE(std::abs(heard[static_cast<size_t>(at)*2+c]-decoded.getSample(c,static_cast<int>(at))*monitorEdgeGain(static_cast<size_t>(at),static_cast<size_t>(countFrames),1000,.005))<1e-6);
    REQUIRE(!engine.snapshot().songExport.cursor);reader.reset();
    };auditionOracle(ready);
    // Gain-only edits retain the exact cursor while publication is deliberately held.
    REQUIRE(engine.playExport(engine.assignmentEpoch()));std::array<float,127> editLeft{},editRight{};
    const auto editBlock=[&]{editLeft.fill(0);editRight.fill(0);{AudioAllocationGuard guard;engine.processAudio(editLeft.data(),editRight.data(),127,context.time);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);};
    waitFor([&]{editBlock();return engine.snapshot().songExport.cursor.has_value();});
    engine.holdExportForTesting(true);auto editAt=*engine.snapshot().songExport.cursor;
    REQUIRE(engine.setExportLevels(1,.25f,engine.assignmentEpoch()));
    waitFor([&]{return engine.exportWaitingForTesting();});
    for(int block=0;block<100;++block){if(block==25)REQUIRE(engine.setExportLevels(.65f,1,engine.assignmentEpoch()));
        if(block==50)REQUIRE(engine.setExportLevels(1,.5f,engine.assignmentEpoch()));
        editBlock();editAt+=127;const auto snap=engine.snapshot().songExport;
        REQUIRE(snap.cursor==editAt && snap.previewRequested && !snap.prepared);
        REQUIRE(!engine.deliverExport(engine.assignmentEpoch(),ready->file,{},false,true));juce::Thread::sleep(1);}
    REQUIRE(engine.snapshot().songExport.previewLevels && engine.snapshot().songExport.previewLevels->dry==1 && engine.snapshot().songExport.previewLevels->wet==.5f);
    REQUIRE(verifyExportFile(engine.assetDirectory(),ready->file,error));
    waitFor([&]{return engine.exportWaitingForTesting();});
    const auto exports=engine.assetDirectory().getChildFile("exports"),retained=engine.assetDirectory().getChildFile("exports-retained-test");
    REQUIRE(exports.moveFileTo(retained) && exports.replaceWithText("injected publication obstruction"));
    engine.holdExportForTesting(false);waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::error;});
    for(int block=0;block<20;++block){editBlock();editAt+=127;REQUIRE(engine.snapshot().songExport.cursor==editAt);juce::Thread::sleep(1);}
    REQUIRE(!engine.snapshot().songExport.prepared && !engine.deliverExport(engine.assignmentEpoch(),ready->file,{},false,true));
    REQUIRE(exports.deleteFile() && retained.moveFileTo(exports));REQUIRE(verifyExportFile(engine.assetDirectory(),ready->file,error));
    REQUIRE(engine.setExportLevels(0,0,engine.assignmentEpoch()));editBlock();editBlock();
    REQUIRE(!engine.snapshot().songExport.previewRequested && !engine.snapshot().songExport.cursor);
    REQUIRE(engine.setExportMix(.65f,engine.assignmentEpoch()));engine.holdExportForTesting(false);
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    REQUIRE(!engine.snapshot().songExport.previewRequested);ready=engine.snapshot().songExport.prepared;
    REQUIRE(engine.playExport(engine.assignmentEpoch()));waitFor([&]{editBlock();return engine.snapshot().songExport.cursor.has_value();});
    auto incompatible=context.time;incompatible.bpm+=1;engine.processAudio(editLeft.data(),editRight.data(),127,incompatible);
    REQUIRE(!engine.snapshot().songExport.cursor && !engine.snapshot().songExport.previewRequested);editBlock();REQUIRE(!engine.snapshot().songExport.previewRequested);


    const auto baselineExport=engine.savedSession().exportState;const auto baselineRenders=engine.snapshot().renderCount;
    const auto seek=[&](int64_t sample){return engine.seekExport(sample,engine.assignmentEpoch(),flow.snapshot().pass,ready->plan->identity);};
    REQUIRE(seek(INT64_MIN) && engine.snapshot().songExport.previewPosition==ready->metrics.audibleStart);
    REQUIRE(!engine.snapshot().songExport.previewRequested && !engine.snapshot().songExport.cursor);
    REQUIRE(seek(INT64_MAX) && engine.snapshot().songExport.previewPosition==*ready->metrics.audibleEnd-1);
    REQUIRE(seek(40000));
    REQUIRE(!engine.seekExport(10,engine.assignmentEpoch()+1,flow.snapshot().pass,ready->plan->identity));
    REQUIRE(!engine.seekExport(10,engine.assignmentEpoch(),flow.snapshot().pass+1,ready->plan->identity));
    REQUIRE(!engine.seekExport(10,engine.assignmentEpoch(),flow.snapshot().pass,"superseded"));
    REQUIRE(engine.snapshot().songExport.previewPosition==40000);
    auditionOracle(ready); // Independent decoded-WAV oracle from the parked point, with a new start ramp.
    REQUIRE(engine.playExport(engine.assignmentEpoch()));
    std::array<float,37> seekLeft{},seekRight{};
    const auto advanceSeek=[&]{seekLeft.fill(0);seekRight.fill(0);{AudioAllocationGuard guard;engine.processAudio(seekLeft.data(),seekRight.data(),37,context.time);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);};
    waitFor([&]{advanceSeek();return engine.snapshot().songExport.cursor.has_value();});
    missingTime=true;waitFor([&]{return missingTimeReads.load()>=3;});
    REQUIRE(engine.snapshot().songExport.prepared==ready && engine.snapshot().songExport.previewRequested);
    missingTime=false;
    REQUIRE(seek(50000) && seek(60000));
    REQUIRE(!engine.snapshot().songExport.cursor && engine.snapshot().songExport.previewPosition==60000);
    waitFor([&]{advanceSeek();return engine.snapshot().songExport.cursor.has_value();});
    REQUIRE(engine.snapshot().songExport.cursor==60037);
    juce::WavAudioFormat seekFormat;std::unique_ptr<juce::AudioFormatReader> seekReader(seekFormat.createReaderFor(engine.assetDirectory().getChildFile(ready->file.path).createInputStream().release(),true));
    juce::AudioBuffer<float> seekDecoded(2,37);REQUIRE(seekReader && seekReader->read(&seekDecoded,0,37,60000-ready->file.origin,true,true));
    for(int n=0;n<37;++n){const auto gain=monitorEdgeGain(n,static_cast<size_t>(*ready->metrics.audibleEnd-60000),1000,.005);
        REQUIRE(std::abs(seekLeft[n]-seekDecoded.getSample(0,n)*gain)<1e-6 && std::abs(seekRight[n]-seekDecoded.getSample(1,n)*gain)<1e-6);}
    seekReader.reset();REQUIRE(seek(62000));engine.stop();juce::Thread::sleep(50);advanceSeek();advanceSeek();
    REQUIRE(!engine.snapshot().songExport.previewRequested && !engine.snapshot().songExport.cursor);
    REQUIRE(std::all_of(seekLeft.begin(),seekLeft.end(),[](float x){return x==0;}));
    REQUIRE(engine.savedSession().exportState==baselineExport && engine.snapshot().renderCount==baselineRenders);
    engine.requestExportDetail(0,64,64,engine.assignmentEpoch(),ready->plan->identity);
    engine.requestExportDetail(128,200,72,engine.assignmentEpoch(),ready->plan->identity);
    waitFor([&]{const auto d=engine.snapshot().songExport.detail;return d && d->first==128 && d->end==200;});
    REQUIRE(engine.savedSession().exportState==baselineExport && engine.snapshot().renderCount==baselineRenders);
    const auto saved=engine.savedSession();REQUIRE(saved.exportState.history.size()==1 && saved.exportState.completedPath==ready->file.path);
    TestDirectory external;REQUIRE(external.file.createDirectory().wasOk());const auto destination=external.file.getChildFile("delivered.wav");REQUIRE(destination.replaceWithText("keep existing"));
    auto delivery=engine.deliverExport(engine.assignmentEpoch(),ready->file,destination,false,false,flow.snapshot().pass);REQUIRE(delivery);
    waitFor([&]{return !engine.snapshot().songExport.delivering;});REQUIRE(destination.loadFileAsString()=="keep existing");
    REQUIRE(engine.deliverExport(engine.assignmentEpoch(),ready->file,destination,true,false,flow.snapshot().pass));
    waitFor([&]{return !engine.snapshot().songExport.delivering;});REQUIRE(m3::fileHash(destination)==ready->file.checksum);
    REQUIRE(engine.deliverExport(engine.assignmentEpoch(),ready->file,{},false,true));
    waitFor([&]{return !engine.snapshot().songExport.delivering;});REQUIRE(engine.snapshot().songExport.dragReady && juce::File(engine.snapshot().songExport.deliveryPath).existsAsFile());
    REQUIRE(!engine.deliverExport(engine.assignmentEpoch(),ready->file,engine.assetDirectory().getChildFile("renders/protected.wav"),true));
    const auto chooserPass=flow.snapshot().pass;REQUIRE(flow.requestMode(workflow::Mode::live,{true,true,true}));
    REQUIRE(flow.requestMode(workflow::Mode::finalise,{true,true,true}));
    REQUIRE(!engine.deliverExport(engine.assignmentEpoch(),ready->file,destination,true,false,chooserPass));
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});ready=engine.snapshot().songExport.prepared;
    const auto count=changes.load();REQUIRE(engine.setExportMix(.65f,engine.assignmentEpoch()));juce::Thread::sleep(100);
    REQUIRE(changes.load()==count && engine.snapshot().songExport.prepared==ready);
    engine.holdExportForTesting(true);REQUIRE(engine.setExportMix(.35f,engine.assignmentEpoch()));
    waitFor([&]{return engine.exportWaitingForTesting();});REQUIRE(!engine.snapshot().songExport.prepared && engine.snapshot().songExport.previous && engine.snapshot().songExport.previous->file==ready->file);const auto oldEpoch=engine.assignmentEpoch();
    REQUIRE(!seek(1000) && !engine.snapshot().songExport.previewPosition);
    engine.restore(saved);REQUIRE(!engine.setExportMix(.1f,oldEpoch));
    engine.holdExportForTesting(false);waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    REQUIRE(engine.snapshot().songExport.prepared->file==ready->file && engine.savedSession().exportState.history.size()==1);
    REQUIRE(!engine.snapshot().playing); // Publication is independent of editor lifetime and never starts preview.
    REQUIRE(!engine.snapshot().songExport.previewPosition && !engine.snapshot().songExport.previewRequested);
    REQUIRE(flow.requestMode(workflow::Mode::live,{true,true,true}));REQUIRE(!engine.snapshot().songExport.prepared);
    // A failed requested IR retains the independently verified accepted wet.
    const auto invalid=root.file.getChildFile("invalid.wav");REQUIRE(invalid.replaceWithText("not audio"));
    REQUIRE(engine.assignIR(id,find(engine.savedSession(),id)->inputGeneration,invalid));
    waitFor([&]{return find(engine.savedSession(),id)->phase==Phase::error;});
    REQUIRE(engine.hasExportableWet(context.time));auto wrong=context.time;wrong.bpm+=1;REQUIRE(!engine.hasExportableWet(wrong));
    REQUIRE(flow.requestMode(workflow::Mode::finalise,{true,true,true}));
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    REQUIRE(engine.snapshot().songExport.prepared->acceptedNotice.contains("replacement pending"));
    REQUIRE(engine.snapshot().songExport.prepared->file==ready->file);
    // Missing completed bytes rebuild to a different immutable path, retaining history.
    REQUIRE(engine.assetDirectory().getChildFile(ready->file.path).deleteFile());engine.restore(saved);
    waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    REQUIRE(engine.snapshot().songExport.prepared->file.path!=ready->file.path && engine.savedSession().exportState.history.size()==2);
    REQUIRE(verifyExportFile(engine.assetDirectory(),engine.snapshot().songExport.prepared->file,error));
    {
        workflow::State otherFlow;REQUIRE(otherFlow.requestMode(workflow::Mode::finalise,{true,true,true}));
        SwellEngine other(root.file,[&]{return context;});other.useWorkflow(otherFlow);other.restore(saved);
        waitFor([&]{return other.snapshot().songExport.phase==ExportPhase::ready;});
        REQUIRE(other.setExportMix(.2f,other.assignmentEpoch()));waitFor([&]{return other.snapshot().songExport.phase==ExportPhase::ready;});
        REQUIRE(other.snapshot().songExport.prepared->file.wet==.2f && engine.snapshot().songExport.prepared->file.wet==.65f);
        REQUIRE(verifyExportFile(engine.assetDirectory(),engine.snapshot().songExport.prepared->file,error));
    }
    for(float wet:{0.f,.5f,1.f,.65f}){REQUIRE(engine.setExportMix(wet,engine.assignmentEpoch()));
        waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});auditionOracle(engine.snapshot().songExport.prepared);}
    REQUIRE(engine.playExport(engine.assignmentEpoch()));waitFor([&]{std::array<float,16> l{},r{};engine.processAudio(l.data(),r.data(),16,context.time);return engine.snapshot().songExport.cursor.has_value();});
    engine.holdExportForTesting(true);REQUIRE(engine.setExportMix(.67f,engine.assignmentEpoch()));waitFor([&]{return engine.exportWaitingForTesting();});
    REQUIRE(engine.residentBytes()<=previewMemoryBudget);const auto stoppedAt=engine.snapshot().songExport.cursor;engine.stop();REQUIRE(engine.snapshot().songExport.previewPosition==stoppedAt);engine.holdExportForTesting(false);waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
    std::array<float,32> stoppedL{},stoppedR{};engine.processAudio(stoppedL.data(),stoppedR.data(),32,context.time);
    stoppedL.fill(0);stoppedR.fill(0);engine.processAudio(stoppedL.data(),stoppedR.data(),32,context.time);
    REQUIRE(!engine.snapshot().songExport.cursor && std::all_of(stoppedL.begin(),stoppedL.end(),[](float x){return x==0;}));
    {
        workflow::State replacementFlow;REQUIRE(replacementFlow.requestMode(workflow::Mode::live,{true,true,true}));
        SwellEngine replacement(root.file,[&]{return context;});replacement.useWorkflow(replacementFlow);replacement.restore(saved);
        waitFor([&]{return find(replacement.savedSession(),id)->phase==Phase::ready;});
        const auto next=root.file.getChildFile("replacement.wav");writeTestWav(next,1,1000,std::vector<float>(301,.002f));
        REQUIRE(replacement.assignIR(id,find(replacement.savedSession(),id)->inputGeneration,next));
        waitFor([&]{return find(replacement.savedSession(),id)->irName=="replacement.wav";});replacement.generate();
        waitFor([&]{return find(replacement.savedSession(),id)->phase==Phase::ready;});
        const auto accepted=find(replacement.savedSession(),id)->render;
        REQUIRE(replacementFlow.requestMode(workflow::Mode::finalise,{true,true,true}));
        waitFor([&]{return replacement.snapshot().songExport.phase==ExportPhase::ready;});
        REQUIRE(replacement.snapshot().songExport.prepared->plan->members[0].accepted.render==accepted);
        REQUIRE(replacement.snapshot().songExport.prepared->acceptedNotice.isEmpty());
    }
    {
        const auto previous=engine.snapshot().songExport.prepared->file;REQUIRE(engine.assetDirectory().getChildFile(previous.path).deleteFile());
        const auto id=engine.deliverExport(engine.assignmentEpoch(),previous,{},false,true);REQUIRE(id);
        waitFor([&]{const auto snapshot=engine.snapshot().songExport;return snapshot.phase==ExportPhase::ready && snapshot.prepared->file.path!=previous.path;});
        const auto recovered=engine.snapshot().songExport;
        REQUIRE(recovered.delivery==id && !recovered.dragReady && recovered.deliveryMessage.contains("missing"));
        REQUIRE(verifyExportFile(engine.assetDirectory(),recovered.prepared->file,error));
    }
    {
        publicationDirectory=engine.assetDirectory().getChildFile("exports");
        const auto countFiles=[&]{return publicationDirectory.findChildFiles(juce::File::findFiles,false,"Swells-*.wav").size();};
        const auto before=engine.savedSession().exportState;const auto previous=engine.snapshot().songExport.prepared->file;
        REQUIRE(engine.playExport(engine.assignmentEpoch()));
        waitFor([&]{editBlock();return engine.snapshot().songExport.cursor.has_value();});
        publicationFileCount=countFiles();publicationMissingTime=true;
        REQUIRE(engine.setExportMix(.43f,engine.assignmentEpoch()));
        waitFor([&]{return publicationMisses.load()>=3;});
        const auto waiting=engine.snapshot().songExport;
        REQUIRE(waiting.phase==ExportPhase::preparing && !waiting.prepared && waiting.previous->file==previous);
        REQUIRE(engine.savedSession().exportState.history==before.history && engine.savedSession().exportState.completedPath==before.completedPath);
        REQUIRE(countFiles()==publicationFileCount+1 && !engine.playExport(engine.assignmentEpoch()));
        auto at=*engine.snapshot().songExport.cursor;
        for(int block=0;block<20;++block){editBlock();at+=127;REQUIRE(engine.snapshot().songExport.cursor==at);juce::Thread::sleep(1);}
        engine.stop();editBlock();editBlock();REQUIRE(!engine.snapshot().songExport.previewRequested);

        publicationMissingTime=false;
        waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
        const auto recovered=engine.snapshot().songExport.prepared->file;
        REQUIRE(recovered.wet==.43f && countFiles()==publicationFileCount+1);
        REQUIRE(engine.savedSession().exportState.history.size()==before.history.size()+1);
        REQUIRE(verifyExportFile(engine.assetDirectory(),recovered,error) && verifyExportFile(engine.assetDirectory(),previous,error));
        REQUIRE(!engine.snapshot().songExport.previewRequested);
        // A pending retry cannot publish after a newer mix request supersedes it.
        publicationFileCount=countFiles();publicationMisses=0;publicationMissingTime=true;
        REQUIRE(engine.setExportMix(.44f,engine.assignmentEpoch()));
        waitFor([&]{return publicationMisses.load()>=3;});
        REQUIRE(engine.setExportMix(.45f,engine.assignmentEpoch()));publicationMissingTime=false;
        waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
        REQUIRE(engine.snapshot().songExport.prepared->file.wet==.45f && countFiles()==publicationFileCount+2);
        const auto history=engine.savedSession().exportState.history;
        REQUIRE(std::none_of(history.begin(),history.end(),[](const auto& file){return file.wet==.44f;}));
    }
    {
        const auto baseline=engine.savedSession();
        engine.holdExportForTesting(true);
        REQUIRE(engine.setExportLevels(1,1,engine.assignmentEpoch()));
        waitFor([&]{return engine.exportWaitingForTesting();});
        const auto pending=engine.savedSession();REQUIRE(pending.exportState.dry==1 && pending.exportState.wet==1);
        REQUIRE(pending.exportState.history==baseline.exportState.history);
        REQUIRE(engine.setExportLevels(1,1,engine.assignmentEpoch()) && engine.savedSession().exportState==pending.exportState);
        REQUIRE(!engine.setExportLevels(.25f,.75f,engine.assignmentEpoch()+1));
        engine.holdExportForTesting(false);waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
        const auto additive=engine.snapshot().songExport.prepared;
        REQUIRE(additive->plan->dry==1 && additive->plan->wet==1 && additive->file.dry==1 && additive->file.wet==1 && additive->file.gainLaw==2);
        REQUIRE(additive->file.path.contains("v2-100.000pct-dry-100pct-wet"));
        auditionOracle(additive);
        const auto restoredPair=engine.savedSession();engine.restore(restoredPair);
        waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
        REQUIRE(engine.snapshot().songExport.prepared->file==additive->file);
        REQUIRE(engine.playExport(engine.assignmentEpoch()));waitFor([&]{editBlock();return engine.snapshot().songExport.cursor.has_value();});
        engine.holdExportForTesting(true);REQUIRE(engine.setExportLevels(.7f,.3f,engine.assignmentEpoch()));waitFor([&]{return engine.exportWaitingForTesting();});
        engine.restore(restoredPair);engine.holdExportForTesting(false);waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
        editBlock();REQUIRE(!engine.snapshot().songExport.previewRequested && !engine.snapshot().songExport.cursor);
        REQUIRE(engine.playExport(engine.assignmentEpoch()));waitFor([&]{editBlock();return engine.snapshot().songExport.cursor.has_value();});
        REQUIRE(flow.requestMode(workflow::Mode::live,{true,true,true}));editBlock();REQUIRE(!engine.snapshot().songExport.previewRequested && !engine.snapshot().songExport.cursor);
        REQUIRE(flow.requestMode(workflow::Mode::finalise,{true,true,true}));waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});
        REQUIRE(!engine.snapshot().songExport.previewRequested);
        const auto beforeSilence=engine.savedSession().exportState;
        REQUIRE(engine.setExportLevels(0,0,engine.assignmentEpoch()));
        waitFor([&]{return engine.snapshot().songExport.phase==ExportPhase::idle;});
        REQUIRE(engine.snapshot().songExport.message.startsWith("Silent:") && !engine.snapshot().songExport.prepared);
        REQUIRE(!engine.playExport(engine.assignmentEpoch()));
        REQUIRE(engine.deliverExport(engine.assignmentEpoch(),additive->file,{},false,true)==0);
        REQUIRE(engine.savedSession().exportState.history==beforeSilence.history && verifyExportFile(engine.assetDirectory(),additive->file,error));
    }
    engine.holdExportForTesting(true);REQUIRE(engine.setExportMix(.15f,engine.assignmentEpoch()));
    waitFor([&]{return engine.exportWaitingForTesting();});const auto beforeExit=engine.savedSession().exportState;
    REQUIRE(flow.requestMode(workflow::Mode::live,{true,true,true}));engine.holdExportForTesting(false);
    waitFor([&]{return !engine.exportWaitingForTesting();});juce::Thread::sleep(30);
    REQUIRE(engine.savedSession().exportState==beforeExit && !engine.snapshot().songExport.prepared);
    const auto history=engine.savedSession().exportState;const auto completed=std::find_if(history.history.begin(),history.history.end(),[&](const auto& f){return f.path==history.completedPath;});REQUIRE(completed!=history.history.end());
    REQUIRE(flow.requestMode(workflow::Mode::capture,{true,true,true}));engine.clear();
    REQUIRE(engine.savedSession().exportState==history && verifyExportFile(engine.assetDirectory(),*completed,error));
    {
        TestDirectory missing;workflow::State unavailableFlow;REQUIRE(unavailableFlow.requestMode(workflow::Mode::finalise,{true,true,true}));
        SwellEngine unavailable(missing.file,[&]{return context;});unavailable.useWorkflow(unavailableFlow);unavailable.restore(saved);
        waitFor([&]{return unavailable.snapshot().songExport.phase==ExportPhase::error;});
        REQUIRE(!unavailable.snapshot().songExport.prepared && unavailable.savedSession().exportState.history==saved.exportState.history);
    }
    REQUIRE(engine.residentBytes()<previewMemoryBudget);
});}
