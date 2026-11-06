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

    const auto rowEvent=[&](int row)
    {
        list->scrollToEnsureRowIsOnscreen(row);const auto p=juce::Point<float>{static_cast<float>(list->getWidth()-12),static_cast<float>(list->getRowPosition(row,true).getCentreY())};
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),p,juce::ModifierKeys::rightButtonModifier,0,0,0,0,0,list,list,juce::Time::getCurrentTime(),p,juce::Time::getCurrentTime(),1,false);
    };
    const auto popup=[]()->juce::Component*
    {
        auto& desktop=juce::Desktop::getInstance();for(int i=desktop.getNumComponents();--i>=0;)if(desktop.getComponent(i)->getName()=="menu")return desktop.getComponent(i);
        return nullptr;
    };
    const auto renameDialog=[&](const juce::String& id)
    {
        const auto row=list->rowFor(id);list->getModel()->listBoxItemClicked(row,rowEvent(row));auto* menu=popup();REQUIRE(menu);
        menu->keyPressed(juce::KeyPress(juce::KeyPress::downKey));menu->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
        juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
        auto* dialog=dynamic_cast<juce::AlertWindow*>(juce::Component::getCurrentlyModalComponent());REQUIRE(dialog && dialog->getTextEditor("name"));return dialog;
    };
    // Rename remains pinned while its original row moves.
    auto* rename=renameDialog(first);rename->getTextEditor("name")->setText("Pending rename follows ID");
    REQUIRE(engine.edit(first,[](auto& w,auto& e){return m3::moveArrival(w,0,e);}));editor.refresh();
    rename->exitModalState(1);juce::MessageManager::getInstance()->runDispatchLoopUntil(30);editor.refresh();verify();
    REQUIRE(m3::find(engine.savedSession(),first)->name=="Pending rename follows ID" && m3::find(engine.savedSession(),earlier)->name=="ZZZ arbitrary earlier");

    // A menu opened before a reorder deletes its original ID, not its old row.
    const auto row=list->rowFor(earlier);list->getModel()->listBoxItemClicked(row,rowEvent(row));auto* menu=popup();REQUIRE(menu);
    REQUIRE(engine.edit(earlier,[](auto& w,auto& e){return m3::moveArrival(w,80000,e);}));editor.refresh();
    menu->keyPressed(juce::KeyPress(juce::KeyPress::upKey));menu->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);editor.refresh();verify();REQUIRE(!m3::find(engine.savedSession(),earlier));

    // Same ID, new session: stale rename/select/delete are all rejected.
    rename=renameDialog(first);rename->getTextEditor("name")->setText("Must not cross restore");
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
