#pragma once
#include <UI/Controls/ExportLevelsControl.h>
namespace exportLevelsReview
{
using namespace m3::ui;
using namespace m3::ui::controls;
class Panel final:public juce::Component
{
public:
    Panel(){addAndMakeVisible(levels);addAndMakeVisible(feedback);
        feedback.setText("Candidate presets: review controls here; audio listening approval remains separate.",juce::dontSendNotification);
        levels.onChange=[this](auto pair){feedback.setText("Dry "+juce::String(pair.dry*100,1)+"% / Wet "+juce::String(pair.wet*100,1)+"%",juce::dontSendNotification);};
        setSize(1100,850);}
    void resized()override{auto area=getLocalBounds().reduced(workspaceGeometry::outerInset(getWidth()));
        feedback.setBounds(area.removeFromBottom(controlGeometry::labelHeight));
        area.removeFromLeft(area.getWidth()*2/3);levels.setBounds(area.removeFromTop((controlGeometry::labelHeight+spacing::labelHelp+controlGeometry::height+spacing::related)*3));}
    void paint(juce::Graphics& g)override{g.fillAll(background);}
    ExportLevelsControl levels;juce::Label feedback;
};
inline void cases()
{
    ExportLevelsControl levels;unsigned edits=0;ExportLevelsControl::Pair last;
    levels.onChange=[&](auto pair){++edits;last=pair;};
    levels.display({.87321f,.45678f});REQUIRE(edits==0 && levels.selector.getSelectedId()==1);
    levels.selector.setSelectedId(5,juce::sendNotificationSync);REQUIRE(edits==1 && last==ExportLevelsControl::Pair(1,1));
    levels.dry.setValue(37,juce::sendNotificationSync);REQUIRE(edits==2 && last.dry==.37f && last.wet==1 && levels.selector.getSelectedId()==1);
    levels.wet.setValue(25,juce::sendNotificationSync);REQUIRE(edits==3 && last.dry==.37f && last.wet==.25f);
    levels.display({1,.25f});REQUIRE(edits==3 && levels.selector.getSelectedId()==3);
    levels.display({1,.250001f});REQUIRE(edits==3 && levels.selector.getSelectedId()==1);
    for(auto pair:std::array<ExportLevelsControl::Pair,3>{{{0,0},{0,1},{1,0}}}){levels.display(pair);REQUIRE(edits==3);}
    // Bindings, rather than shared controls, own session rejection. A rejected
    // invocation reconciles the display to the accepted pair without recursion.
    unsigned session=1,invocation=1,commits=0;ExportLevelsControl::Pair accepted{0,1};
    levels.beforeInvocation=[&]{invocation=session;};
    levels.onChange=[&](auto pair){if(invocation==session){accepted=pair;++commits;}levels.display(accepted);};
    levels.beforeInvocation();++session;levels.dry.setValue(50,juce::sendNotificationSync);
    REQUIRE(commits==0 && levels.dry.getValue()==0 && levels.wet.getValue()==100);
    levels.beforeInvocation();levels.dry.setValue(50,juce::sendNotificationSync);
    REQUIRE(commits==1 && accepted.dry==.5f && accepted.wet==1);
    levels.setSize(304,240);
    {
        auto& dry=levels.dry;
        const auto position=dry.getLookAndFeel().getSliderLayout(dry).sliderBounds.getCentre().toFloat();
        const auto now=juce::Time::getCurrentTime();
        const juce::MouseEvent fine(juce::Desktop::getInstance().getMainMouseSource(),position,
            juce::ModifierKeys::ctrlModifier,0,0,0,0,0,&dry,&dry,now,position,now,0,false);
        juce::MouseWheelDetails wheel{};wheel.deltaY=.1f;
        dry.mouseWheelMove(fine,wheel);
        REQUIRE(commits==2 && accepted.dry==.501f && accepted.wet==1 && levels.selector.getSelectedId()==1);
    }
    levels.onChange=[&](auto pair){++edits;last=pair;};
    const auto id=levels.selector.getSelectedId();
    const auto p=levels.selector.getLocalBounds().getCentre().toFloat();const auto now=juce::Time::getCurrentTime();
    juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(),p,{},0,0,0,0,0,&levels.selector,&levels.selector,now,p,now,0,false);
    juce::MouseWheelDetails wheel{};wheel.deltaY=.1f;levels.selector.mouseWheelMove(event,wheel);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(levels.selector.getSelectedId()==id && edits==3);
    for(const auto size:std::array<juce::Point<int>,4>{{{1100,850},{1400,900},{1400,1000},{1600,1000}}}){
        Panel panel;panel.setSize(size.x,size.y);REQUIRE(panel.getLocalBounds().contains(panel.levels.getBounds()));
        for(auto* child:panel.levels.getChildren())REQUIRE(panel.levels.getLocalBounds().contains(child->getBounds()));
    }
    {
        Panel panel;panel.addToDesktop(0);panel.setVisible(true);panel.toFront(true);
        auto& control=panel.levels;auto& selector=control.selector;unsigned changes=0,prepared=0;
        ExportLevelsControl::Pair pair{.87321f,.45678f};control.display(pair);
        control.beforeInvocation=[&]{++prepared;};control.onChange=[&](auto value){pair=value;++changes;};
        const auto pump=[]{juce::MessageManager::getInstance()->runDispatchLoopUntil(40);};
        selector.showPopup();auto* popup=selector.activePopup();REQUIRE(popup && prepared==1);
        popup->inspect(0,true);popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
        REQUIRE(selector.isPopupActive() && changes==0 && selector.getSelectedId()==1);
        popup->inspect(4,true);REQUIRE(changes==0 && pair==ExportLevelsControl::Pair(.87321f,.45678f));
        popup->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));pump();REQUIRE(changes==0);
        selector.showPopup();popup=selector.activePopup();REQUIRE(popup && prepared==2);
        popup->inspect(4,true);popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();
        REQUIRE(changes==1 && pair==ExportLevelsControl::Pair(1,1) && selector.getSelectedId()==5);
        control.display(pair);REQUIRE(changes==1);
    }
}
}
