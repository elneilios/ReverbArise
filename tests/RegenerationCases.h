#pragma once
#include "LearnGenerationSupport.h"

static void regenerationCases(bool review=false)
{
    GenerationFixture fixture;auto& engine=fixture.engine();auto& processor=*fixture.processor;
    engine.setLearningAutomatic(false);engine.setAutoMarkers(false);engine.setPreferences(0,.5f,.1,false);
    fixture.record();REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));
    REQUIRE(engine.addTarget(9600));
    await([&]{return engine.savedSession().swells[0].phase==m3::Phase::ready;});
    const auto id=engine.savedSession().primary;
    PreverbAudioProcessorEditor editor(processor);editor.stopAutomaticRefreshForReview();editor.enableResponsiveReview();editor.setVisible(true);
    auto* gear=dynamic_cast<juce::Button*>(named(editor,"diagnostics"));REQUIRE(gear);gear->onClick();
    auto* tabs=editor.settingsPanel();tabs->selectPage(m3::ui::SettingsPage::recovery);
    auto* all=dynamic_cast<juce::Button*>(named(editor,"regenerateAll"));
    auto* feedback=dynamic_cast<juce::Label*>(named(editor,"regenerationFeedback"));REQUIRE(all && feedback);
    const auto invoke=[&]{
        if(all->isEnabled())invokeCommand(*all);
        else all->onClick(); // Retain the model-level rejection oracle beneath disabled UI.
        REQUIRE(feedback->getDescription()==feedback->getText());};
    const auto count=engine.snapshot().renderCount;invoke();REQUIRE(feedback->getText()=="Queued 1 for regeneration.");
    await([&]{return engine.savedSession().swells[0].phase==m3::Phase::ready && engine.snapshot().renderCount==count+1;});
    editor.refresh();REQUIRE(feedback->getText()=="Queued 1 for regeneration.");
    invoke();await([&]{return engine.savedSession().swells[0].phase==m3::Phase::ready && engine.snapshot().renderCount==count+2;});
    REQUIRE(engine.setFrozen(id,true));editor.refresh();invoke();REQUIRE(feedback->getText()=="No eligible swells to queue. Skipped 1 frozen.");
    for(const auto scale:{100,125})
    {
        REQUIRE(editor.setPresentationScale(scale));editor.refresh();
        REQUIRE(editor.presentationState()->effective().effective.scalePercent==scale);
        REQUIRE(feedback->getParentComponent()->getLocalBounds().contains(feedback->getBounds()));
        REQUIRE(feedback->getY()>=all->getBottom()+m3::ui::spacing::tight);
        auto* details=named(editor,"openDiagnostics");REQUIRE(details);
        REQUIRE(details->getParentComponent()->getLocalBounds().contains(details->getBounds()));
        snapshotPanel(*tabs,"regeneration-recovery-"+juce::String(scale));
    }
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::finalise));editor.refresh();
    REQUIRE(!all->isEnabled());invoke();REQUIRE(feedback->getText().contains("requires Perform mode"));
    REQUIRE(processor.requestWorkflowMode(workflow::Mode::live));
    auto empty=engine.savedSession();empty.swells.clear();empty.selected.clear();empty.primary.clear();
    engine.restore(empty);editor.refresh();REQUIRE(feedback->getText().isEmpty() && feedback->getDescription().isEmpty());
    REQUIRE(all->isEnabled());invoke();REQUIRE(feedback->getText()=="No eligible swells to queue.");
    if(review)
    {
        REQUIRE(editor.setPresentationScale(100));editor.refresh();
        auto* window=tabs->findParentComponentOfClass<juce::DocumentWindow>();REQUIRE(window);window->toFront(true);
        while(window->isVisible())juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    }
}
