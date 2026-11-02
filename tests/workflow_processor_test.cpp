#include "TestSupport.h"
#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <Plugin/PluginProcessor.h>

struct WorkflowHost:juce::AudioPlayHead
{PositionInfo position;juce::Optional<PositionInfo> getPosition()const override{return position;}};
struct SavedStateListener final:juce::AudioProcessorListener
{
    unsigned changes=0;bool wrongThread=false;
    void audioProcessorParameterChanged(juce::AudioProcessor*,int,float)override{}
    void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails& details)override
    {if(details.nonParameterStateChanged){++changes;wrongThread|=!juce::MessageManager::getInstance()->isThisTheMessageThread();}}
};

int main(){return runTest([]{
    juce::ScopedJuceInitialiser_GUI gui;TestDirectory root;SavedStateListener listener;
    auto p=std::make_unique<PreverbAudioProcessor>(root.file);
    WorkflowHost host;p->prepareToPlay(1000,64);p->setPlayHead(&host);juce::AudioBuffer<float> block(2,64);juce::MidiBuffer midi;
    const auto process=[&](int64_t at,bool playing,float value){host.position.setIsPlaying(playing);host.position.setTimeInSamples(at);
        host.position.setPpqPosition(at/500.0);host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
        for(int c=0;c<2;++c)std::fill_n(block.getWritePointer(c),block.getNumSamples(),value);
        if(p->isNonRealtime())p->processBlock(block,midi);
        else {{AudioAllocationGuard guard;p->processBlock(block,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);}};
    auto& e=p->swellEngine();e.setAutomaticProcessing(false);
    const auto defaultDeadline=juce::Time::getMillisecondCounter()+10000;
    while(!e.savedSession().defaultIR.present() && juce::Time::getMillisecondCounter()<defaultDeadline)juce::Thread::sleep(2);
    REQUIRE(e.savedSession().defaultIR.present());p->flushHostStateChangesForTesting();p->addListener(&listener);
    REQUIRE(p->requestWorkflowMode(workflow::Mode::capture));
    REQUIRE(p->setRecaptureExisting(true) && p->setRecaptureExisting(false));p->flushHostStateChangesForTesting();
    REQUIRE(listener.changes==0); // Same mode and temporary arm are not saved edits.
    REQUIRE(p->getName()=="ReverbArise" && p->getParameters().isEmpty());
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::capture);
    REQUIRE(!e.addTarget(0) && !e.audition());process(0,true,.125f);
    REQUIRE(!p->requestWorkflowMode(workflow::Mode::live));REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getCaptureSnapshot().manifest.ranges.empty());
    juce::MemoryBlock midPass;p->getStateInformation(midPass);
    process(64,false,0);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::live);
    p->flushHostStateChangesForTesting();REQUIRE(listener.changes==1 && !listener.wrongThread);
    auto accepted=p->getCaptureSnapshot();REQUIRE(!accepted.manifest.ranges.empty());
    juce::MemoryBlock liveBytes;p->getStateInformation(liveBytes);
    auto liveRestore=std::make_unique<PreverbAudioProcessor>(root.file);liveRestore->setStateInformation(liveBytes.getData(),static_cast<int>(liveBytes.getSize()));
    REQUIRE(liveRestore->flushCaptureForTesting() && liveRestore->getWorkflowSnapshot().mode==workflow::Mode::live);liveRestore.reset();
    REQUIRE(block.getMagnitude(0,64)==0);process(0,true,.5f);process(64,false,0);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getCaptureSnapshot().reference==accepted.reference); // Live never captures.
    const auto beforeMode=listener.changes;
    REQUIRE(p->requestWorkflowMode(workflow::Mode::capture));p->flushHostStateChangesForTesting();REQUIRE(listener.changes==beforeMode+1);
    REQUIRE(p->setRecaptureExisting(true) && p->setRecaptureExisting(false));p->flushHostStateChangesForTesting();REQUIRE(listener.changes==beforeMode+1);
    process(0,true,.5f);process(64,false,0);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::capture && p->getCaptureSnapshot().reference==accepted.reference);
    REQUIRE(p->setRecaptureExisting(true));process(0,true,.5f);process(64,false,0);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::live && !p->getWorkflowSnapshot().recapture);
    REQUIRE(p->getCaptureSnapshot().reference!=accepted.reference);
    const auto replaced=p->getCaptureSnapshot();
    REQUIRE(e.addTarget(0));const auto id=e.savedSession().primary;
    REQUIRE(e.edit(id,[](auto& w,auto&){w.sourceStart=0;w.sourceEnd=64;w.shape.endOffset=64;w.shape.fadeOut=0;return true;}));
    const auto file=root.file.getChildFile("unit.wav");writeTestWav(file,1,1000,{1,0,0,0});e.loadIR(file);
    const auto until=juce::Time::getMillisecondCounter()+10000;
    while(juce::Time::getMillisecondCounter()<until && e.savedSession().swells[0].status=="Importing IR")juce::Thread::sleep(5);
    e.generate();while(juce::Time::getMillisecondCounter()<until && e.savedSession().swells[0].phase!=m3::Phase::ready)juce::Thread::sleep(5);
    REQUIRE(e.savedSession().swells[0].phase==m3::Phase::ready);
    p->flushHostStateChangesForTesting();const auto beforeSettings=listener.changes;
    REQUIRE(e.edit(id,[](auto& w,auto&){w.shape.gainDb=3;return true;}));
    e.setPostRoll(1);e.setIsolateDry(true);e.setIsolateSelected(true);e.setMonitoring(m3::AuditionMode::dryWet,.7f);
    e.setPreferences(-3,.25f,.1,false);e.setAutoMarkers(false);e.setRegenerationSensitivity(.2f);
    p->flushHostStateChangesForTesting();REQUIRE(listener.changes==beforeSettings+1); // Coalesces continuous/grouped edits.
    const auto beforeNoop=listener.changes;
    REQUIRE(e.edit(id,[](auto&,auto&){return true;}));e.setPostRoll(1);e.setIsolateDry(true);e.setIsolateSelected(true);
    e.setMonitoring(m3::AuditionMode::dryWet,.7f);e.setPreferences(-3,.25f,.1,false);e.setAutoMarkers(false);e.setRegenerationSensitivity(.2f);
    REQUIRE(e.selectTargets({},{}));REQUIRE(e.selectTargets({id},id));e.stop();
    e.setPostRoll(6);e.setLivePlayback(true,m3::LiveRouting::dryWet,100);
    REQUIRE(!e.edit(id,[](auto&,auto&){return false;}));
    p->flushHostStateChangesForTesting();REQUIRE(listener.changes==beforeNoop && !listener.wrongThread);
    e.setLivePlayback(true,m3::LiveRouting::wetOnly,0);
    REQUIRE(p->requestWorkflowMode(workflow::Mode::finalise));
    REQUIRE(!e.edit(id,[](auto& w,auto&){w.shape.gainDb=6;return true;}));
    process(0,true,.25f);process(64,true,.25f); // Let the <=5 ms transition settle.
    for(int c=0;c<2;++c)for(int n=0;n<64;++n)REQUIRE(block.getSample(c,n)==.25f);
    p->setNonRealtime(true);process(0,true,.375f);
    for(int c=0;c<2;++c)for(int n=0;n<64;++n)REQUIRE(block.getSample(c,n)==.375f);
    REQUIRE(p->getCaptureSnapshot().reference==replaced.reference);
    juce::MemoryBlock state;p->getStateInformation(state);
    {
        // Two inserts in one bounce retain independent mode snapshots.
        auto finalised=std::make_unique<PreverbAudioProcessor>(root.file);
        finalised->prepareToPlay(1000,64);finalised->setPlayHead(&host);
        finalised->setStateInformation(state.getData(),static_cast<int>(state.getSize()));
        REQUIRE(finalised->flushCaptureForTesting());
        p->setNonRealtime(false);process(0,false,0);
        REQUIRE(p->requestWorkflowMode(workflow::Mode::live));
        p->setNonRealtime(true);process(0,true,.375f);
        bool containsWet=false;for(int n=0;n<64;++n)containsWet|=std::abs(block.getSample(0,n))>1e-8f && std::abs(block.getSample(0,n)-.375f)>1e-6f;
        REQUIRE(containsWet);
        finalised->setNonRealtime(true);juce::AudioBuffer<float> dry(2,64);
        for(int c=0;c<2;++c)std::fill_n(dry.getWritePointer(c),64,.375f);
        const auto finaliseSource=finalised->getCaptureSnapshot().reference;
        finalised->processBlock(dry,midi);
        for(int c=0;c<2;++c)for(int n=0;n<64;++n)REQUIRE(dry.getSample(c,n)==.375f);
        REQUIRE(finalised->getCaptureSnapshot().reference==finaliseSource);
    }
    auto restored=std::make_unique<PreverbAudioProcessor>(root.file);restored->prepareToPlay(1000,64);
    restored->setStateInformation(state.getData(),static_cast<int>(state.getSize()));REQUIRE(restored->flushCaptureForTesting());
    REQUIRE(restored->getWorkflowSnapshot().mode==workflow::Mode::finalise && !restored->getWorkflowSnapshot().recapture);
    auto legacy=juce::AudioProcessor::getXmlFromBinary(state.getData(),static_cast<int>(state.getSize()));
    REQUIRE(legacy);legacy->setAttribute("stateVersion",7);legacy->removeAttribute("workflowMode");juce::MemoryBlock old;
    juce::AudioProcessor::copyXmlToBinary(*legacy,old);restored->setStateInformation(old.getData(),static_cast<int>(old.getSize()));
    REQUIRE(restored->flushCaptureForTesting() && restored->getWorkflowSnapshot().mode==workflow::Mode::capture);
    REQUIRE(restored->getCaptureSnapshot().reference==replaced.reference);
    for(int version=1;version<=7;++version)
    {legacy->setAttribute("stateVersion",version);juce::MemoryBlock fixture;juce::AudioProcessor::copyXmlToBinary(*legacy,fixture);
        restored->setStateInformation(fixture.getData(),static_cast<int>(fixture.getSize()));REQUIRE(restored->flushCaptureForTesting());
        REQUIRE(restored->getWorkflowSnapshot().mode==workflow::Mode::capture && !restored->getWorkflowSnapshot().recapture);}
    restored->setStateInformation(midPass.getData(),static_cast<int>(midPass.getSize()));REQUIRE(restored->flushCaptureForTesting());
    REQUIRE(restored->getWorkflowSnapshot().mode==workflow::Mode::capture && restored->getCaptureSnapshot().manifest.ranges.empty());
    // Bad mode metadata cannot replace a valid session.
    auto invalid=juce::AudioProcessor::getXmlFromBinary(state.getData(),static_cast<int>(state.getSize()));
    invalid->setAttribute("workflowMode","Unknown");juce::MemoryBlock invalidBytes;juce::AudioProcessor::copyXmlToBinary(*invalid,invalidBytes);
    restored->setStateInformation(invalidBytes.getData(),static_cast<int>(invalidBytes.getSize()));
    REQUIRE(restored->getWorkflowSnapshot().mode==workflow::Mode::capture && restored->getCaptureSnapshot().manifest.ranges.empty());
    p->setNonRealtime(false);process(64,false,0);REQUIRE(p->requestWorkflowMode(workflow::Mode::capture));
    p->holdCaptureValidationForTesting(true);process(200,true,.25f);process(264,false,0);
    const auto deadline=juce::Time::getMillisecondCounter()+2000;
    while(p->getWorkflowSnapshot().phase!=workflow::Phase::validating && juce::Time::getMillisecondCounter()<deadline)juce::Thread::sleep(1);
    REQUIRE(p->getWorkflowSnapshot().phase==workflow::Phase::validating && !p->requestWorkflowMode(workflow::Mode::live));
    // Restore supersedes the validating pass; late completion cannot change it.
    p->setStateInformation(old.getData(),static_cast<int>(old.getSize()));p->holdCaptureValidationForTesting(false);
    REQUIRE(p->flushCaptureForTesting() && p->getWorkflowSnapshot().mode==workflow::Mode::capture && p->getCaptureSnapshot().reference==replaced.reference);
    // Saturated capture queue aborts the whole pass, retaining accepted source.
    p->holdCaptureWorkerForTesting(true);juce::Thread::sleep(5);process(0,false,0);
    for(int i=0;i<40;++i)process(1000+i*64,true,.2f);process(3560,false,0);
    p->holdCaptureWorkerForTesting(false);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getDroppedCaptureBlockCount()>0 && p->getWorkflowSnapshot().phase==workflow::Phase::failed && p->getCaptureSnapshot().reference==replaced.reference);
    p->holdCaptureValidationForTesting(true);process(4000,false,0);process(4000,true,.1f);process(4064,false,0);
    const auto validationDeadline=juce::Time::getMillisecondCounter()+2000;
    while(p->getWorkflowSnapshot().phase!=workflow::Phase::validating && juce::Time::getMillisecondCounter()<validationDeadline)juce::Thread::sleep(1);
    REQUIRE(p->getWorkflowSnapshot().phase==workflow::Phase::validating);
    p->setNonRealtime(true);process(0,false,.375f);p->holdCaptureValidationForTesting(false);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::live);process(32,false,.375f);
    REQUIRE(block.getSample(0,63)==.375f); // A Capture-entered bounce cannot acquire wet mid-export.
    {
        auto quiet=std::make_unique<PreverbAudioProcessor>(root.file.getChildFile("quiet"));WorkflowHost quietHost;
        quiet->prepareToPlay(1000,64);quiet->setPlayHead(&quietHost);juce::AudioBuffer<float> silent(2,64);silent.clear();
        quietHost.position.setTimeInSamples(0);quietHost.position.setIsPlaying(true);quiet->processBlock(silent,midi);
        REQUIRE(quiet->getWorkflowSnapshot().phase==workflow::Phase::idle); // Missing musical timing is not fabricated.
        quietHost.position.setIsPlaying(false);quiet->processBlock(silent,midi);
        quietHost.position.setBpm(120);quietHost.position.setPpqPosition(0);quietHost.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
        quietHost.position.setIsPlaying(true);quiet->processBlock(silent,midi);quietHost.position.setIsPlaying(false);quiet->processBlock(silent,midi);
        REQUIRE(quiet->flushCaptureForTesting() && quiet->getWorkflowSnapshot().mode==workflow::Mode::capture);
        REQUIRE(!quiet->requestWorkflowMode(workflow::Mode::live) && quiet->getCaptureSnapshot().manifest.ranges.front().kind==CaptureCoverageKind::silence);
    }
});}
