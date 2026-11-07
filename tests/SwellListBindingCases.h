#pragma once

static void swellListBindingCases(PreverbAudioProcessor& processor,PreverbAudioProcessorEditor& editor)
{
    auto& engine=processor.swellEngine();const auto baseline=engine.savedSession();
    REQUIRE(baseline.swells.size()==2);
    const auto first=baseline.swells[0].id,second=baseline.swells[1].id;
    auto* list=dynamic_cast<m3::ui::controls::SwellList*>(named(editor,"swellList"));REQUIRE(list);
    const auto verify=[&]
    {
        const auto s=engine.savedSession();REQUIRE(list->getModel()->getNumRows()==static_cast<int>(s.swells.size()));
        for(int i=0;i<list->getModel()->getNumRows();++i)
        {
            const auto* row=list->rowAt(i);const auto* w=m3::find(s,row->id);REQUIRE(w && w->arrival==row->arrival && w->name==row->name);
            if(i){const auto* previous=list->rowAt(i-1);REQUIRE(previous->arrival<row->arrival || (previous->arrival==row->arrival && previous->id.compare(row->id)<0));}
            REQUIRE(list->isRowSelected(i)==(std::find(s.selected.begin(),s.selected.end(),row->id)!=s.selected.end()));
        }
        REQUIRE(list->primaryId()==s.primary);
    };
    editor.refresh();verify();const auto renders=engine.snapshot().renderCount;
    struct SavedChanges final:juce::AudioProcessorListener
    {
        PreverbAudioProcessor& owner;int changes=0;
        explicit SavedChanges(PreverbAudioProcessor& p):owner(p){owner.addListener(this);}
        ~SavedChanges()override{owner.removeListener(this);}
        void audioProcessorParameterChanged(juce::AudioProcessor*,int,float)override{}
        void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails& d)override{if(d.nonParameterStateChanged)++changes;}
    };
    processor.flushHostStateChangesForTesting();
    {SavedChanges changes(processor);for(int i=0;i<4;++i)editor.refresh();processor.flushHostStateChangesForTesting();REQUIRE(changes.changes==0);}
    REQUIRE(engine.snapshot().renderCount==renders);
    REQUIRE(engine.savedSession().selected==baseline.selected && engine.savedSession().primary==baseline.primary);
    for(size_t i=0;i<baseline.swells.size();++i)REQUIRE(m3::sameSavedSwell(baseline.swells[i],engine.savedSession().swells[i]));

    // Earlier insertion after a late target. Preserve model vector order.
    REQUIRE(engine.edit(first,[](auto& w,auto& e){w.name="AAA arbitrary later";return m3::moveArrival(w,60000,e);}));
    REQUIRE(engine.addTarget(12000));const auto earlier=engine.savedSession().primary;
    REQUIRE(engine.edit(earlier,[](auto& w,auto&){w.name="ZZZ arbitrary earlier";return true;}));
    REQUIRE(engine.selectTargets({first,earlier},first));editor.refresh();verify();
    REQUIRE(list->rowAt(0)->id==earlier && list->rowAt(1)->id==second && list->rowAt(2)->id==first);
    REQUIRE(engine.savedSession().swells[0].id==first && engine.savedSession().swells.back().id==earlier);
    const auto settings=*m3::find(engine.savedSession(),first);
    auto* units=dynamic_cast<juce::ToggleButton*>(named(editor,"sampleUnits"));REQUIRE(units);
    units->setToggleState(true,juce::sendNotificationSync);editor.refresh();
    auto* target=dynamic_cast<m3::ui::KnobValue*>(named(editor,"arrivalSampleCell"));REQUIRE(target);
    target->onBegin();REQUIRE(target->onCommit("6000"));target->onEnd();editor.refresh();verify();
    REQUIRE(list->rowAt(0)->id==first && engine.savedSession().primary==first && engine.savedSession().selected.size()==2);
    const auto moved=*m3::find(engine.savedSession(),first);
    REQUIRE(moved.ir==settings.ir && moved.enabled==settings.enabled && moved.shape==settings.shape && moved.placement==settings.placement);

    auto& timeline=editor.timelineComponent();timeline.setSnap(false);
    auto view=timeline.viewport();view.first=-10000;view.last=100000;timeline.setViewport(view);
    const auto lane=timeline.sourceLane();
    const auto x=[&](int64_t sample){return static_cast<float>(timeline.viewport().pixel(static_cast<double>(sample),lane.getX(),lane.getWidth()));};
    REQUIRE(timeline.beginDrag({x(6000),lane.getY()+15}));timeline.dragTo({x(70000),lane.getY()+15});
    const auto t=juce::Time::getCurrentTime();const auto point=juce::Point<float>{x(70000),lane.getY()+15};
    timeline.mouseUp(juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,{},0,0,0,0,0,&timeline,&timeline,t,point,t,1,false));
    editor.refresh();verify();REQUIRE(list->rowAt(2)->id==first && engine.savedSession().primary==first);
    REQUIRE(engine.savedSession().swells[0].id==first); // Presentation never sorts definitions.

    using namespace m3::ui::controls;
    nativeInputPeer(editor);
    const juce::ScopeGuard removePeer{[&editor]{editor.removeFromDesktop();}};
    const auto pump = [] { juce::MessageManager::getInstance()->runDispatchLoopUntil(40); };
    const auto openMenu = [&](SwellList& targetList, const juce::String& id, bool rightClick)
    {
        const auto index = targetList.rowFor(id);
        REQUIRE(index >= 0);
        targetList.scrollToEnsureRowIsOnscreen(index);
        const auto row = targetList.getRowPosition(index, true);
        const juce::Point<int> point{rightClick ? row.getX() + 70 : row.getRight() - 12, row.getCentreY()};
        auto* surface = targetList.getComponentAt(point);
        REQUIRE(surface != nullptr && surface != &targetList);
        const juce::Component::SafePointer<juce::Component> pressed(surface);
        const auto position = surface->getLocalPoint(&targetList, point).toFloat();
        const auto now = juce::Time::getCurrentTime();
        const juce::ModifierKeys mods(rightClick ? juce::ModifierKeys::rightButtonModifier : juce::ModifierKeys::leftButtonModifier);
        const juce::MouseEvent click(juce::Desktop::getInstance().getMainMouseSource(), position, mods,
                                     0, 0, 0, 0, 0, surface, surface, now, position, now, 1, false);
        const auto anchor = SwellList::menuAnchor(click);
        surface->mouseDown(click);
        if (pressed) pressed->mouseUp(click);

        auto* menu = dynamic_cast<ChoicePopup*>(juce::Component::getCurrentlyModalComponent());
        REQUIRE(menu && menu->getName() == "swellActionMenu");
        REQUIRE(menu->getTitle() == "Actions for " + targetList.rowAt(targetList.rowFor(id))->name);
        REQUIRE(menu->state().items()[0].role == PopupRowRole::action && menu->state().items()[1].role == PopupRowRole::destructiveAction);
        const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(anchor);
        REQUIRE(display && menu->getScreenBounds() == ChoicePopup::placedBounds(anchor, display->userArea, menu->getWidth(), menu->getHeight()));
        return menu;
    };
    const auto renameDialog=[&](const juce::String& id, bool rightClick)
    {
        auto* menu = openMenu(*list, id, rightClick);
        menu->keyPressed(juce::KeyPress(juce::KeyPress::downKey));menu->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
        pump();
        auto* dialog=dynamic_cast<juce::AlertWindow*>(juce::Component::getCurrentlyModalComponent());REQUIRE(dialog && dialog->getTextEditor("name"));return dialog;
    };
    const auto originalName = m3::find(engine.savedSession(), first)->name;
    auto* cancelledRename = renameDialog(first, false);
    cancelledRename->getTextEditor("name")->setText("Cancelled edit");
    cancelledRename->exitModalState(0);
    pump();
    REQUIRE(m3::find(engine.savedSession(), first)->name == originalName);

    auto* cancelledMenu = openMenu(*list, second, true);
    cancelledMenu->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
    pump();
    REQUIRE(juce::Component::getCurrentlyModalComponent() == nullptr);
    REQUIRE(m3::find(engine.savedSession(), second));

    // Rename remains pinned while its original row moves.
    auto* rename=renameDialog(first, true);rename->getTextEditor("name")->setText("Pending rename follows ID");
    REQUIRE(engine.edit(first,[](auto& w,auto& e){return m3::moveArrival(w,0,e);}));editor.refresh();
    rename->exitModalState(1);juce::MessageManager::getInstance()->runDispatchLoopUntil(30);editor.refresh();verify();
    REQUIRE(m3::find(engine.savedSession(),first)->name=="Pending rename follows ID" && m3::find(engine.savedSession(),earlier)->name=="ZZZ arbitrary earlier");

    // A menu opened before a reorder deletes its original ID, not its old row.
    auto* menu = openMenu(*list, earlier, false);
    REQUIRE(engine.edit(earlier,[](auto& w,auto& e){return m3::moveArrival(w,80000,e);}));editor.refresh();
    REQUIRE(engine.selectTargets({second}, second));editor.refresh();
    menu->keyPressed(juce::KeyPress(juce::KeyPress::upKey));menu->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    menu->activate(1);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);editor.refresh();verify();REQUIRE(!m3::find(engine.savedSession(),earlier));
    REQUIRE(m3::find(engine.savedSession(), second));

    // Replace the session after queuing Delete and before the binding receives it.
    menu = openMenu(*list, first, true);
    const auto beforeDelete = engine.savedSession();
    menu->activate(1);
    engine.restore(beforeDelete);
    pump();
    editor.refresh();
    REQUIRE(m3::find(engine.savedSession(), first));

    {
        auto temporary = std::make_unique<PreverbAudioProcessorEditor>(processor);
        nativeInputPeer(*temporary);
        temporary->refresh();
        auto* temporaryList = dynamic_cast<SwellList*>(named(*temporary, "swellList"));
        REQUIRE(temporaryList);
        openMenu(*temporaryList, second, false)->activate(1);
        temporary.reset();
        pump();
        REQUIRE(juce::Component::getCurrentlyModalComponent() == nullptr);
        REQUIRE(m3::find(engine.savedSession(), second));
    }

    // Same ID, new session: stale rename/select/delete are all rejected.
    rename=renameDialog(first, false);rename->getTextEditor("name")->setText("Must not cross restore");
    const auto oldEpoch=engine.snapshot().sessionEpoch;auto replacement=engine.savedSession();
    engine.restore(replacement);rename->exitModalState(1);juce::MessageManager::getInstance()->runDispatchLoopUntil(30);editor.refresh();
    REQUIRE(engine.snapshot().sessionEpoch!=oldEpoch && m3::find(engine.savedSession(),first)->name==m3::find(replacement,first)->name);
    REQUIRE(!engine.selectTargets({second},second,oldEpoch));REQUIRE(!engine.removeTarget(first,oldEpoch));
    REQUIRE(!engine.edit(first,[](auto& w,auto&){w.name="obsolete";return true;},oldEpoch));
    verify();
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));editor.refresh();REQUIRE(!list->isVisible());
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));editor.refresh();verify();
    snapshotEditor(editor,"chronological-list-integrated");
    units->setToggleState(false,juce::sendNotificationSync);engine.restore(baseline);
    waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
}
