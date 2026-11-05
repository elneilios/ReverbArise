#include "LearnGenerationSupport.h"
#include "ControlLibraryCases.h"
#include <Plugin/PluginEditor.h>
#include <UI/InitialPerformView.h>
#include <Core/BuildInformation.h>

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
static void pendingProtectedFraming()
{
    GenerationFixture fixture;const auto accepted=fixture.engine().savedSession();
    const auto time=fixture.processor->getSongTime();Swell pending;pending.ir=accepted.defaultIR;
    pending.arrival=pending.sourceStart=-2*static_cast<int64_t>(pending.ir.frames);pending.sourceEnd=pending.arrival+32;
    pending.phase=Phase::checking;REQUIRE(pending.generationSource.isEmpty() && pending.provenance==Provenance::manualCreated);
    EditorSnapshot shown;shown.session=accepted;shown.session.swells={pending};shown.session.primary=pending.id;shown.session.selected={pending.id};
    SwellTimeline timeline(fixture.engine());timeline.setSize(600,400);timeline.setData({},shown,time,true);timeline.revealInitialSelection();
    const auto bounds=m3::ui::initialSwellBounds(pending,time);const auto view=timeline.viewport();
    REQUIRE(view.first<=bounds.first && view.last>=bounds.second);
    REQUIRE(view.first>=timeline.navigationBounds().first && view.last<=timeline.navigationBounds().last);
    REQUIRE(fixture.engine().savedSession().swells.empty() && fixture.engine().snapshot().renderCount==0);
}
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
    REQUIRE(visible(helper) && helper->getY()>named(editor,"learnAutomaticHelp")->getBottom());
    {
        PreverbAudioProcessorEditor reopened(*fixture.processor);reopened.setVisible(true);reopened.refresh();
        auto* retained=named(reopened,"learnManualSwellsHelp");
        REQUIRE(visible(retained) && !retained->getBounds().isEmpty() && retained->getParentComponent()->getLocalBounds().contains(retained->getBounds()));
        REQUIRE(!visible(named(reopened,"learnGenerateSwells")));
    }
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
    REQUIRE(settled.getPixelAt(x,y)==m3::ui::separator);
    const auto event=[&](juce::ModifierKeys mods){const auto point=juce::Point<float>{static_cast<float>(x),static_cast<float>(y)};const auto time=juce::Time::getCurrentTime();
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,mods,0,0,0,0,0,workspace,workspace,time,point,time,1,false);};
    workspace->mouseMove(event({}));
    const auto hovered=workspace->createComponentSnapshot(workspace->getLocalBounds());REQUIRE(hovered.getPixelAt(x,y)==m3::ui::timing);
    captureImage(editor,"issue38-event-hover");
    workspace->mouseDown(event(juce::ModifierKeys::leftButtonModifier));workspace->mouseExit(event({}));
    REQUIRE(workspace->hasPinnedInspection() && workspace->diagnosticDetails().contains("Sample"));
    REQUIRE(!workspace->visibleInspectionText().contains("Sample") && !workspace->getDescription().contains("samples"));
    const auto pinned=workspace->createComponentSnapshot(workspace->getLocalBounds()).getPixelAt(x,bounds.getY()+21);
    REQUIRE(pinned.getRed()>pinned.getBlue()+64);
    captureImage(editor,"issue38-event-pinned");
    workspace->setVisible(false);workspace->setVisible(true);
    auto updating=snapshot;updating.learnGeneration.updating=true;
    workspace->display(fixture.processor->getCaptureSnapshot(),updating,fixture.processor->getSongTime(),fixture.processor->getWorkflowSnapshot(),false);
    const auto stale=workspace->createComponentSnapshot(workspace->getLocalBounds());
    REQUIRE(stale.getPixelAt(x,y)!=m3::ui::timing && workspace->timelineStatusText().contains("updating"));
    const auto waveformPixels=[&](float amplitude)
    {
        auto shown=snapshot;shown.session.automaticProcessing=false;
        auto signal=fixture.processor->getCaptureSnapshot();signal.manifest.ranges={{0,48000,CaptureCoverageKind::audio}};
        signal.overview={{signal.manifest.ranges.front(),true,{{0,48000,-amplitude,amplitude}}}};
        workspace->display(signal,shown,fixture.processor->getSongTime(),fixture.processor->getWorkflowSnapshot(),false);workspace->fitDrySource();
        const auto lane=workspace->timelineBounds();const auto viewport=workspace->dryViewport();
        const auto at=juce::roundToInt(lane.getX()+(24000-viewport.first)/(viewport.second-viewport.first)*lane.getWidth());
        const auto raster=workspace->createComponentSnapshot(workspace->getLocalBounds());int pixels=0;
        for(int row=lane.getY()+1;row<lane.getBottom()-1;++row)if(raster.getPixelAt(at,row)==m3::ui::text)++pixels;
        return pixels;
    };
    const auto quiet=waveformPixels(.25f),loud=waveformPixels(.5f);
    REQUIRE(quiet>0 && std::abs(loud-2*quiet)<=3); // Display is not normalized or secretly amplified per event.
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
static void aboutBindings(bool review=false)
{
    GenerationFixture fixture;auto& engine=fixture.engine();
    juce::String copiedInformation;
    std::function<void(const juce::String&)> writer;
    if(!review)writer=[&](const juce::String& information){copiedInformation=information;};
    auto editor=std::make_unique<PreverbAudioProcessorEditor>(*fixture.processor,std::move(writer));editor->setVisible(true);editor->refresh();
    const auto showAbout=[&]{auto* action=dynamic_cast<juce::Button*>(named(*editor,"diagnostics"));REQUIRE(action);action->onClick();
        auto* tabs=dynamic_cast<juce::TabbedComponent*>(editor->settingsPanel());REQUIRE(tabs && tabs->getNumTabs()==6);tabs->setCurrentTabIndex(5);};
    if(review)
    {
        juce::Timer::callAfterDelay(200,[safe=juce::Component::SafePointer<PreverbAudioProcessorEditor>(editor.get())]{if(!safe)return;
            auto* action=dynamic_cast<juce::Button*>(named(*safe,"diagnostics"));REQUIRE(action);action->onClick();
            auto* tabs=dynamic_cast<juce::TabbedComponent*>(safe->settingsPanel());REQUIRE(tabs);tabs->setCurrentTabIndex(5);});
        controlReview::run(std::move(editor));return;
    }
    juce::String before,after,error;REQUIRE(encode(engine.savedSession(),before,error));
    const auto renders=engine.snapshot().renderCount;const auto capture=fixture.processor->getCaptureSnapshot().manifest.revision;
    showAbout();auto* panel=editor->settingsPanel();
    const auto metadata=juce::JSON::parse(juce::File(PREVERB_BUILD_INFO_FILE).loadFileAsString());REQUIRE(metadata.isObject());
    REQUIRE(metadata["version"].toString()==juce::String(m3::build::version.data()));
    REQUIRE(metadata["buildIdentifier"].toString()==juce::String(m3::build::identifier.data()));
    REQUIRE(metadata["gitSha"].toString()==juce::String(m3::build::gitSha.data()));
    REQUIRE(static_cast<bool>(metadata["worktreeDirty"])==m3::build::worktreeDirty);
    const juce::StringArray expectedDisplay{metadata["version"].toString(),metadata["buildIdentifier"].toString(),
        metadata["gitSha"].toString()+(static_cast<bool>(metadata["worktreeDirty"])?" (modified)":"")};
    for(int i=0;i<3;++i){auto* value=dynamic_cast<juce::Label*>(named(*panel,"aboutBuildValue"+juce::String(i)));REQUIRE(value && visible(value));
        REQUIRE(value->getText()==expectedDisplay[i] && value->getTooltip()==expectedDisplay[i]);
        REQUIRE(value->getParentComponent()->getLocalBounds().contains(value->getBounds()) && value->getAccessibilityHandler());}
    auto* copy=dynamic_cast<m3::ui::controls::ActionButton*>(named(*panel,"copyBuildInformation"));REQUIRE(copy && copy->isEnabled() && copy->getAccessibilityHandler());
    auto* status=dynamic_cast<juce::Label*>(named(*panel,"buildInformationCopyStatus"));REQUIRE(status && visible(status));
    REQUIRE(!status->getText().contains("copied to clipboard"));
    const auto fill=[&](bool over,bool down){juce::Image image(juce::Image::ARGB,copy->getWidth(),copy->getHeight(),true);
        {juce::Graphics graphics(image);copy->paintButton(graphics,over,down);}return image.getPixelAt(8,8);};
    const auto normal=fill(false,false),hover=fill(true,false),pressed=fill(true,true);
    REQUIRE(normal==m3::ui::aqua && hover==m3::ui::hover && pressed==m3::ui::hover && normal!=hover);
    copy->setEnabled(false);REQUIRE(fill(false,false)!=normal);copy->setEnabled(true);
    controlReview::pointerClick(*copy);REQUIRE(copiedInformation==juce::String(m3::build::report()));
    REQUIRE(status->getText().contains("Build information copied to clipboard."));
    REQUIRE(status->getDescription()==status->getText() && copy->isEnabled());
    REQUIRE(status->getParentComponent()->getLocalBounds().contains(status->getBounds()));
    copiedInformation.clear();copy->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    await([&]{juce::MessageManager::getInstance()->runDispatchLoopUntil(10);return copiedInformation==juce::String(m3::build::report());});
    REQUIRE(status->getText().contains("copied to clipboard") && copy->isEnabled());
    REQUIRE(copiedInformation.contains(metadata["gitSha"].toString()) && copiedInformation.contains(metadata["buildIdentifier"].toString()));
    REQUIRE(encode(engine.savedSession(),after,error) && before==after);
    REQUIRE(engine.snapshot().renderCount==renders && fixture.processor->getCaptureSnapshot().manifest.revision==capture);
    captureImage(*panel,"alpha-about-settings");
}
static void alphaGuide()
{
    GenerationFixture fixture;auto& engine=fixture.engine();auto& processor=*fixture.processor;
    PreverbAudioProcessorEditor editor(processor);editor.setSize(1400,1000);editor.setVisible(true);editor.refresh();
    REQUIRE(processor.getWorkflowSnapshot().mode==workflow::Mode::capture && engine.savedSession().swells.empty());
    REQUIRE(!named(editor,"learnGenerateSwells")->isEnabled());captureImage(editor,"alpha-empty-learn");
    fixture.record();const auto markers=fixture.ready();editor.refresh();
    REQUIRE(markers.learnGeneration.displayed && !markers.learnGeneration.displayed->positions.empty());
    REQUIRE(named(editor,"learnGenerateSwells")->isEnabled());captureImage(editor,"alpha-captured-markers");
    auto* generate=dynamic_cast<m3::ui::controls::ActionButton*>(named(editor,"learnGenerateSwells"));REQUIRE(generate);
    controlReview::pointerClick(*generate);
    await([&]{const auto s=engine.snapshot();return processor.getWorkflowSnapshot().mode==workflow::Mode::live && !s.session.swells.empty()
        && std::all_of(s.session.swells.begin(),s.session.swells.end(),[](const auto& swell){return swell.phase==Phase::ready;});});
    editor.refresh();selected(editor,engine,m3::ui::firstChronologicalSwell(engine.savedSession()));
    await([&]{editor.refresh();return named(editor,"audition")->isEnabled();});
    captureImage(editor,"alpha-ready-perform");
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::finalise));
    await([&]{return engine.snapshot().songExport.phase==ExportPhase::ready;});editor.refresh();
    REQUIRE(visible(named(editor,"exportWorkspace")) && named(editor,"exportSave")->isEnabled() && named(editor,"exportDrag")->isEnabled());
    REQUIRE(engine.snapshot().songExport.prepared);captureImage(editor,"alpha-ready-export");
}
int main(int argc,char** argv){return runTest([&]
{
    juce::ScopedJuceInitialiser_GUI gui;
    if(argc>1 && juce::String(argv[1])=="--guide-only"){alphaGuide();aboutBindings();return;}
    if(argc>1 && juce::String(argv[1])=="--review-about"){aboutBindings(true);return;}
    const bool review=argc>1 && juce::String(argv[1])=="--review";
    if(!review)aboutBindings();
    if(!review)controlBindings();
    if(!review)pendingProtectedFraming();
    for(bool automatic:{false,true})
    {
        GenerationFixture fixture;auto& engine=fixture.engine();auto& processor=*fixture.processor;
        auto editor=std::make_unique<PreverbAudioProcessorEditor>(processor);editor->setVisible(true);editor->setSize(1100,850);editor->refresh();
        const SwellViewport original{1200000,1584000};editor->timelineComponent().setViewport(original);
        fixture.tick(4096,true);fixture.tick(8192,false);REQUIRE(processor.flushCaptureForTesting());
        await([&]{return processor.getWorkflowReadiness().usefulSource && !processor.getWorkflowSnapshot().locked();});
        // A useful-source, empty Perform visit does not establish an editing position.
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));editor->refresh();
        const auto emptyView=editor->timelineComponent().viewport();
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));editor->refresh();
        REQUIRE(sameView(emptyView,editor->timelineComponent().viewport()));
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
            REQUIRE(!helper->getText().contains("?\n") && helper->getText().contains("? Add"));
            for(auto size:{juce::Point{1100,850},juce::Point{1400,1000},juce::Point{1600,1100}})
            {
                editor->setSize(size.x,size.y);editor->refresh();
                REQUIRE(helper->getWidth()>0 && helper->getHeight()>=40);
                const auto* reason=named(*editor,"learnGenerateReason");REQUIRE(reason);
                REQUIRE(reason->getY()-button->getBottom()==m3::ui::spacing::labelHelp);
                REQUIRE(helper->getY()-reason->getBottom()==m3::ui::spacing::labelHelp);
                REQUIRE(named(*editor,"learnAutoGenerateOnStop")->getY()-named(*editor,"learnAutomaticSwells")->getBottom()==m3::ui::spacing::related);
                REQUIRE(helper->getParentComponent()->getLocalBounds().contains(helper->getBounds()));
                auto* learn=dynamic_cast<m3::ui::LearnWorkspace*>(named(*editor,"learnWorkspace"));REQUIRE(learn);
                REQUIRE(learn->timelineBounds().getHeight()<=learn->timelineBounds().getWidth()/2);
                REQUIRE(learn->timelineStatusText().contains("events") && !learn->timelineStatusText().contains("Mapped"));
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
        const auto initialNavigation=engine.savedSession().performUi;
        REQUIRE(initialNavigation.hasView && sameView({initialNavigation.first,initialNavigation.last},editor->timelineComponent().viewport()));
        REQUIRE(!find(engine.savedSession(),late.id)->enabled && processor.getWorkflowSnapshot().mode==workflow::Mode::live);
        REQUIRE(!fixture.host.position.getIsPlaying());
        captureImage(*editor,automatic?"issue38-auto-first-pending":"issue38-explicit-first-pending");
        // Rendering cannot steal a newer selection or viewport, even across mode return.
        const auto second=committed.session.swells.back().id;REQUIRE(engine.selectTargets({second},second,engine.assignmentEpoch()));editor->refresh();
        const SwellViewport editing{90000,220000};editor->timelineComponent().setViewport(editing);
        editor->timelineComponent().onViewportChanged(); // Publish through the accepted #48 navigation binding.
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
