#include "LearnGenerationSupport.h"
#include "AudioTestFiles.h"
#include <Plugin/PluginEditor.h>
#include <wavpack.h>
#include <bit>

static void writeManualIR(const juce::File& file)
{
    if(file.hasFileExtension("wav"))
    {writeTestWav(file,1,48000,{.5f,.25f,.125f});return;}
    std::vector<char> bytes;
    const auto output=[](void* destination,void* data,int32_t count)->int
    {auto& sink=*static_cast<std::vector<char>*>(destination);auto begin=static_cast<char*>(data);sink.insert(sink.end(),begin,begin+count);return 1;};
    std::unique_ptr<WavpackContext,decltype(&WavpackCloseFile)> writer(WavpackOpenFileOutput(output,&bytes,nullptr),&WavpackCloseFile);REQUIRE(writer);
    WavpackConfig config{};config.sample_rate=48000;config.num_channels=1;config.channel_mask=4;config.bits_per_sample=32;config.bytes_per_sample=4;config.float_norm_exp=127;
    REQUIRE(WavpackSetConfiguration64(writer.get(),&config,3,nullptr) && WavpackPackInit(writer.get()));
    std::vector<int32_t> samples{std::bit_cast<int32_t>(.5f),std::bit_cast<int32_t>(.25f),std::bit_cast<int32_t>(.125f)};
    REQUIRE(WavpackPackSamples(writer.get(),samples.data(),3) && WavpackFlushSamples(writer.get()));
    REQUIRE(file.replaceWithData(bytes.data(),bytes.size()));
}

static juce::Component* named(juce::Component& root,const juce::String& name)
{
    if(root.getName()==name)return &root;
    for(auto* child:root.getChildren())if(auto* result=named(*child,name))return result;
    return nullptr;
}

static void verifyTiming(PreverbAudioProcessorEditor& editor,bool available)
{
    editor.refresh();
    for(const auto* name:{"preRollCell","endOffsetCell"})
    {
        auto* cell=dynamic_cast<m3::ui::KnobValue*>(named(editor,name));REQUIRE(cell);
        REQUIRE(cell->isEnabled()==available);
        const auto text=cell->valueField().getText();
        if(!available)REQUIRE(text.isEmpty());
        else REQUIRE(text.isNotEmpty() && std::abs(text.getDoubleValue())<2);
    }
    auto* audition=dynamic_cast<juce::Button*>(named(editor,"audition"));REQUIRE(audition);
    REQUIRE(audition->isEnabled()==available);
}

static void manualCreation(const char* extension,bool creationOnly)
{
    std::cout<<"Manual preparation: "<<extension<<std::endl;
    GenerationFixture fixture;auto& engine=fixture.engine();auto& processor=*fixture.processor;
    engine.setLearningAutomatic(false);engine.setAutoMarkers(false);
    engine.setPreferences(0,.5f,.1,false);
    const auto ir=fixture.root.file.getChildFile(juce::String("manual.")+extension);writeManualIR(ir);
    REQUIRE(engine.assignDefaultIR(ir));await([&]{return engine.savedSession().defaultIRName==ir.getFileName();});
    const int64_t origin=juce::String(extension)=="wv"?3725040:0; // Target at 77.705 s.
    if(origin==0)fixture.record();
    else
    {
        fixture.tick(origin,false);
        // Include the saved two-second dry audition post-roll as well as the
        // wet extraction/support interval; missing dry is a separate gate.
        for(int n=0;n<48;++n){fixture.tick(origin+n*4096,true);if(n%8==7)REQUIRE(processor.flushCaptureForTesting());}
        fixture.tick(origin+48*4096,false);REQUIRE(processor.flushCaptureForTesting());
    }
    REQUIRE(engine.savedSession().swells.empty() && engine.snapshot().renderCount==0);
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));
    engine.setMonitoring(AuditionMode::wetOnly,1);
    REQUIRE(engine.addTarget(origin+4800));const auto id=engine.savedSession().primary;
    try{await([&]{const auto state=engine.snapshot();return find(state.session,id)->phase==Phase::ready && state.auditionReady;});}
    catch(...){const auto state=engine.snapshot();std::cerr<<extension<<": "<<find(state.session,id)->status<<" | "<<state.dryStatus<<" | "<<state.message<<'\n';throw;}
    const auto ready=engine.snapshot();
    REQUIRE(!ready.session.automaticProcessing && !ready.session.autoMarkersEnabled && !ready.session.autoRegenerate);
    REQUIRE(ready.session.swells.size()==1 && ready.renderCount==1 && !ready.playing);
    REQUIRE(find(ready.session,id)->ir.content==ready.session.defaultIR.content);
    REQUIRE(engine.audition());double energy=0;
    for(int block=0;block<100;++block)
    {fixture.audio.clear();{AudioAllocationGuard guard;processor.processBlock(fixture.audio,fixture.midi);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        for(int i=0;i<fixture.audio.getNumSamples();++i)energy+=std::abs(fixture.audio.getSample(0,i));}
    REQUIRE(energy>0);engine.stop();
    if(creationOnly)return;

    const auto source=processor.getCaptureSnapshot();const auto analysis=engine.snapshot().analysisCount;
    const auto original=*find(engine.savedSession(),id);
    // Accepted source/arrival edits are manual preparation intents too.
    REQUIRE(engine.edit(id,[](auto& w,auto&){w.arrival+=120;w.sourceStart+=120;w.sourceEnd+=120;return true;}));
    REQUIRE(engine.edit(id,[](auto& w,auto&){w.sourceEnd+=240;return true;}));
    await([&]{const auto s=engine.snapshot();return find(s.session,id)->phase==Phase::ready && s.auditionReady;});
    REQUIRE(find(engine.savedSession(),id)->id==original.id);
    REQUIRE(find(engine.savedSession(),id)->render.sourceStart==original.sourceStart+120);
    REQUIRE(find(engine.savedSession(),id)->render.sourceEnd>=original.sourceEnd+360);
    const auto changed=*find(engine.savedSession(),id);
    REQUIRE(changed.render!=original.render);
    // Future defaults must not reassign existing manual targets or start jobs.
    const auto other=fixture.root.file.getChildFile("other.wav");writeTestWav(other,1,48000,{.25f,.125f});
    const auto count=engine.snapshot().renderCount;REQUIRE(engine.assignDefaultIR(other));
    await([&]{return engine.savedSession().defaultIRName=="other.wav";});
    REQUIRE(find(engine.savedSession(),id)->ir==changed.ir && engine.snapshot().renderCount==count);
    REQUIRE(engine.assignIR(id,changed.inputGeneration,other,false,engine.assignmentEpoch()));
    await([&]{const auto s=engine.snapshot();return find(s.session,id)->irName=="other.wav" && s.auditionReady;});
    REQUIRE(find(engine.savedSession(),id)->ir.content!=changed.ir.content);
    REQUIRE(engine.setFrozen(id,true));
    REQUIRE(!engine.edit(id,[](auto& w,auto&){w.sourceEnd+=1;return true;}));
    REQUIRE(!engine.assignIR(id,find(engine.savedSession(),id)->inputGeneration,ir));
    REQUIRE(engine.setFrozen(id,false));await([&]{return engine.snapshot().auditionReady;});

    // A pending selection has no wet bounds, even when another target is ready.
    const auto neighbour=*find(engine.savedSession(),id);engine.holdRenderPublicationForTesting(true);
    REQUIRE(engine.addTarget(origin+40000));const auto pending=engine.savedSession().primary;
    await([&]{return engine.renderPublicationWaitingForTesting();});
    auto editor=std::make_unique<PreverbAudioProcessorEditor>(processor);editor->stopAutomaticRefreshForReview();
    verifyTiming(*editor,false);
    engine.holdRenderPublicationForTesting(false);
    await([&]{return engine.snapshot().auditionReady;});verifyTiming(*editor,true);
    REQUIRE(engine.selectTargets({id},id));await([&]{return engine.snapshot().auditionReady;});verifyTiming(*editor,true);
    REQUIRE(engine.selectTargets({pending},pending));await([&]{return engine.snapshot().auditionReady;});verifyTiming(*editor,true);
    REQUIRE(find(engine.savedSession(),id)->render==neighbour.render);
    // Hold an explicit All request at publication, then delete its target.
    engine.holdRenderPublicationForTesting(true);
    REQUIRE(engine.generate(true).queued==2);
    await([&]{return engine.renderPublicationWaitingForTesting();});
    REQUIRE(engine.removeTarget(pending,engine.assignmentEpoch()));engine.holdRenderPublicationForTesting(false);
    await([&]{return !engine.renderPublicationWaitingForTesting();});
    REQUIRE(!find(engine.savedSession(),pending));
    REQUIRE(engine.snapshot().analysisCount==analysis && processor.getCaptureSnapshot().reference==source.reference);
}

static void manualFailures()
{
    GenerationFixture fixture;auto& engine=fixture.engine();auto& processor=*fixture.processor;
    engine.setLearningAutomatic(false);engine.setAutoMarkers(false);engine.setPreferences(0,.5f,.1,false);
    fixture.record();REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));
    engine.setMonitoring(AuditionMode::wetOnly,1);
    REQUIRE(engine.addTarget(4800));const auto id=engine.savedSession().primary;
    await([&]{return engine.snapshot().auditionReady;});
    const auto capture=processor.getCaptureSnapshot().reference;
    const auto accepted=*find(engine.savedSession(),id);
    // Complete source coverage is still required after an explicit edit.
    REQUIRE(engine.edit(id,[](auto& w,auto&){w.sourceEnd=400000;return true;}));
    await([&]{return find(engine.savedSession(),id)->status=="Waiting for source";});
    REQUIRE(!engine.snapshot().auditionReady && find(engine.savedSession(),id)->phase!=Phase::ready);
    REQUIRE(engine.edit(id,[&](auto& w,auto&){w.sourceEnd=accepted.sourceEnd;return true;}));
    await([&]{return engine.snapshot().auditionReady;});
    REQUIRE(engine.edit(id,[](auto& w,auto&){w.ir={};w.irName.clear();return true;}));
    await([&]{return find(engine.savedSession(),id)->status.contains("Missing IR");});
    REQUIRE(!engine.snapshot().auditionReady && find(engine.savedSession(),id)->phase!=Phase::ready);
    REQUIRE(engine.assignManagedIR(id,find(engine.savedSession(),id)->inputGeneration,accepted.ir,accepted.irName));
    await([&]{return engine.snapshot().auditionReady;});
    const auto bad=fixture.root.file.getChildFile("unsupported.wv");REQUIRE(bad.replaceWithText("invalid WavPack content"));
    REQUIRE(engine.assignIR(id,find(engine.savedSession(),id)->inputGeneration,bad));
    await([&]{return find(engine.savedSession(),id)->phase==Phase::error;});
    REQUIRE(find(engine.savedSession(),id)->status.isNotEmpty());
    REQUIRE(find(engine.savedSession(),id)->ir==accepted.ir && find(engine.savedSession(),id)->render.present());
    REQUIRE(engine.assignManagedIR(id,find(engine.savedSession(),id)->inputGeneration,accepted.ir,accepted.irName));
    await([&]{return engine.snapshot().auditionReady;});
    // Session replacement rejects a render completion held at publication.
    engine.holdRenderPublicationForTesting(true);
    REQUIRE(engine.generate(true).queued==1);
    await([&]{return engine.renderPublicationWaitingForTesting();});const auto epoch=engine.assignmentEpoch();
    auto replacement=engine.savedSession();replacement.swells.clear();replacement.selected.clear();replacement.primary.clear();
    engine.restore(replacement);engine.holdRenderPublicationForTesting(false);
    await([&]{return !engine.renderPublicationWaitingForTesting();});
    REQUIRE(engine.assignmentEpoch()!=epoch && engine.savedSession().swells.empty() && !engine.snapshot().auditionReady);
    REQUIRE(processor.getCaptureSnapshot().reference==capture);
    // Invalid host timing rejects insertion without fabricating a target.
    fixture.host.position.setBpm(0);fixture.tick(0,false);
    REQUIRE(!engine.addTarget(4800));REQUIRE(engine.savedSession().swells.empty());
}

static void firstTimelineInsertion()
{
    GenerationFixture fixture;auto& engine=fixture.engine();auto& processor=*fixture.processor;
    engine.setLearningAutomatic(false);engine.setAutoMarkers(false);engine.setPreferences(0,.5f,.1,false);
    fixture.record();REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));
    engine.setMonitoring(AuditionMode::wetOnly,1);
    REQUIRE(engine.savedSession().swells.empty() && !engine.savedSession().timing.valid);
    PreverbAudioProcessorEditor editor(processor);editor.stopAutomaticRefreshForReview();editor.setSize(1100,850);editor.refresh();
    auto& timeline=editor.timelineComponent();timeline.setViewport({0,96000});timeline.setSnap(true);
    timeline.setGridDivision(GridDivision::quarterBeat);
    const auto lane=timeline.sourceLane();
    const auto doubleClick=[&](int64_t sample)
    {
        const auto view=timeline.viewport();const auto point=juce::Point<float>{static_cast<float>(view.pixel(sample,lane.getX(),lane.getWidth())),lane.getCentreY()};
        const auto now=juce::Time::getCurrentTime();
        juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(),point,juce::ModifierKeys::leftButtonModifier,0,0,0,0,0,&timeline,&timeline,now,point,now,2,false);
        timeline.mouseDoubleClick(event);editor.refresh();
    };
    doubleClick(12345);
    REQUIRE(engine.savedSession().swells.size()==1);
    const auto first=engine.savedSession().primary;REQUIRE(find(engine.savedSession(),first)->arrival==12000);
    await([&]{return engine.snapshot().auditionReady;});
    // The same gesture also works after the first target has pinned timing.
    editor.refresh();doubleClick(48345);REQUIRE(engine.savedSession().swells.size()==2);
    REQUIRE(find(engine.savedSession(),engine.savedSession().primary)->arrival==48000);
    // Existing timing mismatch and missing-position guards must still reject Snap.
    const auto capture=processor.getCaptureSnapshot();auto state=engine.snapshot();auto time=processor.getSongTime();
    time.bpm+=1;timeline.setData(capture,state,time);doubleClick(72345);REQUIRE(engine.savedSession().swells.size()==2);
    timeline.setData(capture,state,processor.getSongTime(),false);doubleClick(72345);REQUIRE(engine.savedSession().swells.size()==2);
}

int main(int argc,char** argv){juce::ScopedJuceInitialiser_GUI gui;return runTest([&]
{
    if(argc>1 && juce::String(argv[1])=="--first-insertion"){firstTimelineInsertion();return;}
    const auto creationOnly=argc>1 && juce::String(argv[1])=="--creation-only";
    manualCreation("wav",creationOnly);manualCreation("wv",creationOnly);
    if(!creationOnly){manualFailures();firstTimelineInsertion();}
});}
