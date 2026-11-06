#pragma once
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/FileDragTile.h>
namespace exportControlReview
{
using namespace m3::ui;
using namespace m3::ui::controls;
class Panel final:public juce::Component
{
public:
    Panel():play("Play preview",CommandIcon::play,true),stop("Stop preview",CommandIcon::stop),save(juce::String("Save WAV")+juce::String::charToString(0x2026),CommandIcon::save),back("Return to Perform",CommandIcon::returnBack)
    {
        for(auto* c:std::array<juce::Component*,8>{&play,&stop,&save,&back,&tile,&disabled,&mix,&feedback})addAndMakeVisible(c);
        disabled.setEnabled(false);mix.setRange(0,100,.1);mix.setValue(65,juce::dontSendNotification);mix.setTextValueSuffix(" % Wet");mix.setSliderStyle(juce::Slider::LinearHorizontal);mix.setTextBoxStyle(juce::Slider::TextBoxRight,false,100,30);mix.setValueFont(interFont(bodySize,InterFace::medium));
        feedback.setFont(interFont(bodySize));feedback.setColour(juce::Label::textColourId,text);feedback.setText("Try keyboard, hover, focus and dragging. Review actions do not write files.",juce::dontSendNotification);
        play.onClick=[this]{feedback.setText("Preview Play requested",juce::dontSendNotification);};stop.onClick=[this]{feedback.setText("Preview Stop requested",juce::dontSendNotification);};
        save.onClick=back.onClick=tile.onSave=[this]{feedback.setText("Command activated",juce::dontSendNotification);};tile.onDrag=[this]{feedback.setText("File drag requested once for this gesture",juce::dontSendNotification);};
        setSize(940,550);
    }
    void resized()override
    {
        auto row=getLocalBounds().reduced(24);row.removeFromTop(70);mix.setBounds(row.removeFromTop(34));row.removeFromTop(24);
        auto controls=row.removeFromBottom(50);play.setBounds(controls.removeFromLeft(48).reduced(0,4));controls.removeFromLeft(10);stop.setBounds(controls.removeFromLeft(48).reduced(0,4));controls.removeFromLeft(24);
        save.setBounds(controls.removeFromLeft(210).reduced(0,4));controls.removeFromLeft(12);back.setBounds(controls.removeFromLeft(240).reduced(0,4));
        feedback.setBounds(row.removeFromBottom(34));row.removeFromBottom(12);auto left=row.removeFromLeft((row.getWidth()-24)/2);row.removeFromLeft(24);tile.setBounds(left);disabled.setBounds(row);
    }
    void paint(juce::Graphics& g)override
    {g.fillAll(background);g.setColour(text);g.setFont(interFont(headingSize,InterFace::semibold));g.drawText("Export controls - native component review",24,18,getWidth()-48,30,juce::Justification::left);}
    ActionButton play,stop,save,back;FileDragTile tile,disabled;SliderControl mix;juce::Label feedback;
};
inline void cases()
{
    WorkspaceTheme theme;Panel panel;panel.setLookAndFeel(&theme);unsigned clicks=0,begins=0;
    panel.save.onClick=[&]{++clicks;};panel.save.beforeInvocation=[&]{++begins;};
    REQUIRE(panel.save.keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(clicks==1 && begins==1);
    panel.save.setEnabled(false);panel.save.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(10);REQUIRE(clicks==1 && begins==1);
    panel.save.setEnabled(true);controlReview::pointerClick(panel.save);REQUIRE(clicks==2 && begins==2);
    unsigned drags=0,saves=0;panel.tile.onDrag=[&]{++drags;};panel.tile.onSave=[&]{++saves;};
    const auto start=panel.tile.getLocalBounds().getCentre().toFloat();const auto now=juce::Time::getCurrentTime();
    const auto event=[&](juce::Point<float> at,juce::ModifierKeys keys){return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),at,keys,0,0,0,0,0,&panel.tile,&panel.tile,now,start,now,1,true);};
    panel.tile.mouseDown(event(start,juce::ModifierKeys::leftButtonModifier));panel.tile.mouseDrag(event(start.translated(8,0),juce::ModifierKeys::leftButtonModifier));panel.tile.mouseDrag(event(start.translated(12,0),juce::ModifierKeys::leftButtonModifier));static_cast<juce::Component&>(panel.tile).mouseUp(event(start.translated(12,0),{}));
    REQUIRE(drags==1 && saves==0);panel.tile.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(saves==1);controlReview::pointerClick(panel.tile);REQUIRE(saves==2);
    panel.tile.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(saves==3);
    for(auto* b:std::array<ActionButton*,6>{&panel.play,&panel.stop,&panel.save,&panel.back,&panel.tile,&panel.disabled})REQUIRE(b->getLocalBounds().toFloat().contains(b->focusInkBounds()));
    snapshotPanel(panel,"export-controls-normal-disabled");nativeInputPeer(panel);panel.tile.grabKeyboardFocus();REQUIRE(panel.tile.hasKeyboardFocus(false));snapshotPanel(panel,"export-controls-focus");panel.removeFromDesktop();panel.setLookAndFeel(nullptr);
}
}
