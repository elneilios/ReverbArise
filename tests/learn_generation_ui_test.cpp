#include "LearnGenerationSupport.h"
#include "ControlLibraryCases.h"
#include <Plugin/PluginEditor.h>
#include <UI/InitialPerformView.h>

static juce::Component* named(juce::Component& root,const juce::String& name)
{
    if(root.getName()==name)return &root;
    for(auto* child:root.getChildren())if(auto* result=named(*child,name))return result;
    return nullptr;
}
static bool visible(juce::Component* value)
{if(!value)return false;for(;value;value=value->getParentComponent())if(!value->isVisible())return false;return true;}
static bool sameView(const SwellViewport& a,const SwellViewport& b)
{return a.first==b.first && a.last==b.last;}
static void controlBindings()
{
    GenerationFixture fixture;auto& engine=fixture.engine();PreverbAudioProcessorEditor editor(*fixture.processor);editor.addToDesktop(0);editor.setVisible(true);editor.refresh();
    auto* generate=dynamic_cast<m3::ui::controls::ActionButton*>(named(editor,"learnGenerateSwells"));
    auto* automatic=dynamic_cast<m3::ui::controls::Switch*>(named(editor,"learnAutomaticSwells"));
    auto* stop=dynamic_cast<m3::ui::controls::Switch*>(named(editor,"learnAutoGenerateOnStop"));
    auto* helper=dynamic_cast<juce::Label*>(named(editor,"learnManualSwellsHelp"));
    REQUIRE(generate && automatic && stop && helper && !generate->isEnabled() && visible(helper));
    REQUIRE(generate->getTooltip().contains(engine.snapshot().learnGeneration.reason) && generate->getAccessibilityHandler());
    stop->keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(engine.savedSession().autoGenerateOnStop);
    controlReview::pointerClick(*automatic);REQUIRE(!visible(stop) && !visible(generate) && engine.savedSession().autoGenerateOnStop);
    controlReview::pointerClick(*automatic);REQUIRE(visible(stop) && stop->getToggleState());
    stop->beforeInvocation();engine.restore(engine.savedSession(),fixture.processor->getCaptureSnapshot().reference);
    stop->setToggleState(false,juce::dontSendNotification);stop->onClick();REQUIRE(engine.savedSession().autoGenerateOnStop);
    REQUIRE(engine.setAutoGenerateOnStop(false,engine.assignmentEpoch(),fixture.processor->getWorkflowSnapshot().pass));
    fixture.record();fixture.ready();editor.refresh();REQUIRE(generate->isEnabled());
    generate->beforeInvocation();engine.restore(engine.savedSession(),fixture.processor->getCaptureSnapshot().reference);generate->onClick();
    REQUIRE(engine.savedSession().swells.empty() && !engine.snapshot().initialPerformHandoff);
    fixture.ready();editor.refresh();engine.holdLearnRendersForTesting(true);
    generate->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    await([&]{return fixture.processor->getWorkflowSnapshot().mode==workflow::Mode::live;});editor.refresh();
    REQUIRE(engine.savedSession().primary==m3::ui::firstChronologicalSwell(engine.savedSession()));
}
static void captureImage(juce::Component& editor,const juce::String& tag,float scale=1)
{
    const auto folder=juce::SystemStats::getEnvironmentVariable("PREVERB_UI_M3_OUTPUT",{});if(folder.isEmpty())return;
    const juce::File directory(folder);REQUIRE(directory.createDirectory().wasOk());
    auto output=directory.getChildFile(tag+".png").createOutputStream();REQUIRE(output);
    REQUIRE(juce::PNGImageFormat().writeImageToStream(editor.createComponentSnapshot(editor.getLocalBounds(),true,scale),*output));
}
static void candidateOverlay(PreverbAudioProcessorEditor& editor,GenerationFixture& fixture)
{
    auto* workspace=dynamic_cast<m3::ui::LearnWorkspace*>(named(editor,"learnWorkspace"));REQUIRE(workspace);
    const auto snapshot=fixture.engine().snapshot();REQUIRE(snapshot.learnGeneration.displayed && !snapshot.learnGeneration.displayed->positions.empty());
    const auto before=fixture.engine().savedSession();const auto renderCount=snapshot.renderCount;
    const auto bounds=workspace->timelineBounds();const auto view=workspace->dryViewport();
    const auto sample=snapshot.learnGeneration.displayed->positions.front().sample;
    const auto x=juce::roundToInt(bounds.getX()+(sample-view.first)/(view.second-view.first)*bounds.getWidth());
    const auto y=bounds.getY()+m3::ui::spacing::tight+2;
    const auto settled=workspace->createComponentSnapshot(workspace->getLocalBounds());
    REQUIRE(settled.getPixelAt(x,y)==m3::ui::timing);
    auto updating=snapshot;updating.learnGeneration.updating=true;
    workspace->display(fixture.processor->getCaptureSnapshot(),updating,fixture.processor->getSongTime(),fixture.processor->getWorkflowSnapshot(),false);
    const auto stale=workspace->createComponentSnapshot(workspace->getLocalBounds());
    REQUIRE(stale.getPixelAt(x,y)!=m3::ui::timing && workspace->timelineStatusText().contains("updating"));
    REQUIRE(fixture.engine().snapshot().renderCount==renderCount);
    juce::String a,b,error;REQUIRE(encode(before,a,error) && encode(fixture.engine().savedSession(),b,error) && a==b);
    editor.refresh();
}
static void selected(PreverbAudioProcessorEditor& editor,SwellEngine& engine,const juce::String& id)
{
    const auto snapshot=engine.snapshot();REQUIRE(snapshot.session.primary==id && snapshot.session.selected==std::vector<juce::String>{id});
    auto* list=dynamic_cast<m3::ui::controls::SwellList*>(named(editor,"swellList"));
    REQUIRE(list && list->primaryId()==id && list->isRowSelected(list->rowFor(id)));
    REQUIRE(visible(named(editor,"inspectorViewport")));
    const auto* swell=find(snapshot.session,id);REQUIRE(swell);
    const auto bounds=m3::ui::initialSwellBounds(*swell,SongTime{true,0,0,48000,120,4,4});
    const auto& view=editor.timelineComponent().viewport();REQUIRE(view.first<=bounds.first && view.last>=bounds.second);
    auto* overview=dynamic_cast<SwellOverview*>(named(editor,"songOverview"));REQUIRE(overview && !overview->detailBounds().isEmpty());
}
int main(int argc,char** argv){return runTest([&]
{
    juce::ScopedJuceInitialiser_GUI gui;
    const bool review=argc>1 && juce::String(argv[1])=="--review";
    if(!review)controlBindings();
    for(bool automatic:{false,true})
    {
        GenerationFixture fixture;auto& engine=fixture.engine();auto& processor=*fixture.processor;
        auto editor=std::make_unique<PreverbAudioProcessorEditor>(processor);editor->setVisible(true);editor->setSize(1100,850);editor->refresh();
        const SwellViewport original{1200000,1584000};editor->timelineComponent().setViewport(original);
        fixture.tick(4096,true);fixture.tick(8192,false);REQUIRE(processor.flushCaptureForTesting());
        await([&]{return processor.getWorkflowReadiness().usefulSource && !processor.getWorkflowSnapshot().locked();});
        // A useful-source, empty Perform visit does not establish an editing position.
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));editor->refresh();
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));editor->refresh();
        REQUIRE(sameView(original,editor->timelineComponent().viewport()));
        auto seed=engine.savedSession();Swell late;late.arrival=late.sourceStart=200000;late.sourceEnd=204000;
        late.provenance=Provenance::manualCreated;late.name="Protected later swell";late.enabled=false;
        seed.swells.push_back(late);seed.primary.clear();seed.selected.clear();engine.restore(seed,processor.getCaptureSnapshot().reference);
        editor->refresh(); // Empty selection in a restored Learn session is still eligible.
        engine.holdLearnRendersForTesting(true);engine.holdLearnGenerationForTesting(!review);
        if(automatic)REQUIRE(engine.setAutoGenerateOnStop(true,engine.assignmentEpoch(),processor.getWorkflowSnapshot().pass));
        fixture.record();auto ready=automatic?engine.snapshot():fixture.ready();editor->refresh();
        if(!automatic)
        {
            candidateOverlay(*editor,fixture);
            auto* button=dynamic_cast<m3::ui::controls::ActionButton*>(named(*editor,"learnGenerateSwells"));REQUIRE(button && button->isEnabled());
            auto* helper=dynamic_cast<juce::Label*>(named(*editor,"learnManualSwellsHelp"));REQUIRE(helper && visible(helper));
            REQUIRE(helper->getText().replace("\n"," ")=="Want to place swells yourself? Add manual swells in Perform.");
            REQUIRE(helper->getText().containsChar('\n') && helper->getFont().getHeight()==m3::ui::learnTypography::secondarySize);
            for(auto size:{juce::Point{1100,850},juce::Point{1400,1000},juce::Point{1600,1100}})
            {
                editor->setSize(size.x,size.y);editor->refresh();
                REQUIRE(helper->getWidth()>0 && helper->getHeight()>=40);
                for(float scale:{1.f,1.25f,1.5f,2.f})captureImage(*editor,"issue38-learn-"+juce::String(size.x)+"-"+juce::String(scale),scale);
            }
            editor->setSize(1100,850);editor->refresh();
            if(review){engine.holdLearnRendersForTesting(false);controlReview::run(std::move(editor));return;}
            controlReview::pointerClick(*button);
        }
        // Pin preparation so neither fast explicit nor automatic completion can
        // be consumed by a refresh before the editor-closed assertion begins.
        await([&]{return engine.learnGenerationWaitingForTesting();});editor.reset();engine.holdLearnGenerationForTesting(false);
        try{await([&]{return processor.getWorkflowSnapshot().mode==workflow::Mode::live;});}
        catch(...){const auto view=engine.snapshot().learnGeneration;std::cerr<<"Initial handoff automatic="<<automatic<<" notice="<<view.notice<<" reason="<<view.reason
            <<" generating="<<view.generating<<" pass="<<processor.getWorkflowSnapshot().pass<<'\n';throw;}
        const auto committed=engine.snapshot();REQUIRE(committed.workflowState.mode==workflow::Mode::live && committed.initialPerformHandoff && committed.session.swells.size()>2);
        const auto first=m3::ui::firstChronologicalSwell(committed.session);
        REQUIRE(first!=committed.session.swells.front().id && committed.session.swells.front().id==late.id);
        // Commit with no editor refresh/reopen yet retains the pending handoff.
        editor=std::make_unique<PreverbAudioProcessorEditor>(processor);editor->setVisible(true);editor->refresh();
        selected(*editor,engine,first);REQUIRE(!engine.snapshot().initialPerformHandoff);
        REQUIRE(!find(engine.savedSession(),late.id)->enabled && processor.getWorkflowSnapshot().mode==workflow::Mode::live);
        REQUIRE(!fixture.host.position.getIsPlaying());
        captureImage(*editor,automatic?"issue38-auto-first-pending":"issue38-explicit-first-pending");
        // Rendering cannot steal a newer selection or viewport, even across mode return.
        const auto second=committed.session.swells.back().id;REQUIRE(engine.selectTargets({second},second,engine.assignmentEpoch()));editor->refresh();
        const SwellViewport editing{90000,220000};editor->timelineComponent().setViewport(editing);
        engine.holdLearnRendersForTesting(false);
        await([&]{const auto s=engine.snapshot();const auto* firstSwell=find(s.session,first);return firstSwell && firstSwell->phase==Phase::ready;});
        editor->refresh();REQUIRE(engine.savedSession().primary==second && sameView(editing,editor->timelineComponent().viewport()));
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));editor->refresh();fixture.ready();
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));editor->refresh();
        REQUIRE(engine.savedSession().primary==second && sameView(editing,editor->timelineComponent().viewport()));
        // Restoring a valid selection keeps the existing viewport and never offers a handoff.
        engine.restore(engine.savedSession(),processor.getCaptureSnapshot().reference);editor->refresh();
        REQUIRE(engine.savedSession().primary==second && !engine.snapshot().initialPerformHandoff && sameView(editing,editor->timelineComponent().viewport()));
        await([&]{const auto session=engine.savedSession();return std::all_of(session.swells.begin(),session.swells.end(),[](const auto& swell)
            {return swell.provenance!=Provenance::automatic || swell.phase==Phase::ready;});});
        editor->refresh();REQUIRE(sameView(editing,editor->timelineComponent().viewport()));
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));editor->refresh();fixture.ready();
        REQUIRE(fixture.generate());await([&]{return !engine.snapshot().learnGeneration.generating;});editor->refresh();
        REQUIRE(processor.getWorkflowSnapshot().mode==workflow::Mode::capture && sameView(editing,editor->timelineComponent().viewport()));
        REQUIRE(engine.assetDirectory().getChildFile(engine.savedSession().defaultIR.path).replaceWithText("Injected UI generation failure"));
        REQUIRE(fixture.generate());
        try{await([&]{const auto view=engine.snapshot().learnGeneration;return !view.generating && view.failed;});}
        catch(...){const auto view=engine.snapshot().learnGeneration;std::cerr<<"UI failure outcome: "<<view.notice<<" reason="<<view.reason<<" failed="<<view.failed<<" generating="<<view.generating<<'\n';throw;}
        editor->refresh();
        REQUIRE(processor.getWorkflowSnapshot().mode==workflow::Mode::capture && !engine.snapshot().initialPerformHandoff);
        REQUIRE(engine.savedSession().primary==second && sameView(editing,editor->timelineComponent().viewport()));
    }
});}
