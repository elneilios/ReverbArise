#pragma once
#include <UI/Controls/SegmentedSelector.h>
#include <UI/Controls/IconActionButton.h>
#include <UI/WorkflowPresentation.h>

namespace controlReview
{
using m3::ui::controls::SegmentedSelector;
using m3::ui::controls::SegmentChoice;
using m3::ui::controls::IconActionButton;
using m3::ui::controls::ActionIcon;
inline std::vector<SegmentChoice> choices(bool available=true)
{
    return {{1,"Learn","Capture audio and discover musical moments",true,{}},
        {2,"Perform","Shape, edit and play your swells",available,"Learn useful source audio first"},
        {3,"Export","DAW wet bypassed; explicit preview only",available,"Export needs an enabled, compatible ready swell"}};
}
class Panel final:public juce::Component
{
public:
    Panel()
    {
        title.setText("Shared controls - checkpoint A",juce::dontSendNotification);title.setFont(juce::FontOptions(m3::ui::headingSize));addAndMakeVisible(title);
        note.setText("Native review fixture only - no DAW, capture or audio changes",juce::dontSendNotification);addAndMakeVisible(note);
        const juce::StringArray captions{"Interactive","No source","Keyboard focus","Hover","Settling"};
        for(size_t i=0;i<groups.size();++i)
        {
            labels[i].setText(captions[static_cast<int>(i)],juce::dontSendNotification);addAndMakeVisible(labels[i]);
            groups[i].display(choices(i!=1),i==2?2:1);addAndMakeVisible(groups[i]);
            icons[i]=std::make_unique<IconActionButton>(ActionIcon::settings);icons[i]->setName("review-icon-"+juce::String(i));addAndMakeVisible(*icons[i]);
        }
        auto settling=choices();for(auto& c:settling){c.available=false;c.unavailableReason="Source commit/validation is settling";}
        groups[4].display(settling,1);icons[4]->setEnabled(false);
        groups[3].segment(2)->setState(juce::Button::buttonOver);icons[3]->setState(juce::Button::buttonOver);
        groups[0].onRequest=[this](int id){groups[0].display(choices(),id);feedback.setText("Accepted "+juce::String(m3::ui::modeLabel(static_cast<workflow::Mode>(id-1))),juce::dontSendNotification);};
        icons[0]->onClick=[this]{feedback.setText("Settings action received (fixture only)",juce::dontSendNotification);};
        feedback.setText("Try the first row. Tab: focus. Arrows: inspect.\nEnter/Space: activate an available choice.",juce::dontSendNotification);addAndMakeVisible(feedback);
        help.setText("Unavailable reasons stay accessible. Wheel does not switch modes.\nAqua = accepted selection; the inset outline marks keyboard focus.\nOther rows show component states; their requests do not change the model.",juce::dontSendNotification);
        help.setJustificationType(juce::Justification::topLeft);addAndMakeVisible(help);
        setSize(720,540);
    }
    void paint(juce::Graphics& g)override{g.fillAll(m3::ui::background);}
    void resized()override
    {
        title.setBounds(24,18,getWidth()-48,30);note.setBounds(24,53,getWidth()-48,24);
        const auto width=std::min(390,getWidth()-250);
        for(size_t i=0;i<groups.size();++i){const auto y=100+static_cast<int>(i)*62;
            labels[i].setBounds(24,y,135,38);groups[i].setBounds(165,y,width,38);icons[i]->setBounds(180+width,y,38,38);}
        feedback.setBounds(24,410,getWidth()-48,45);help.setBounds(24,460,getWidth()-48,70);
    }
    std::array<SegmentedSelector,5> groups;
    std::array<std::unique_ptr<IconActionButton>,5> icons;
private:
    juce::Label title,note,feedback,help;std::array<juce::Label,5> labels;
};
inline void snapshot(Panel& panel,const juce::String& tag)
{
    const auto folder=juce::SystemStats::getEnvironmentVariable("PREVERB_UI_M3_OUTPUT",{});if(folder.isEmpty())return;
    const juce::File dir(folder);REQUIRE(dir.createDirectory().wasOk());
    auto output=dir.getChildFile(tag+".png").createOutputStream();REQUIRE(output && output->setPosition(0) && output->truncate().wasOk());
    REQUIRE(juce::PNGImageFormat().writeImageToStream(panel.createComponentSnapshot(panel.getLocalBounds()),*output));
}
inline void pointerClick(juce::Button& button)
{
    const auto point=button.getLocalBounds().getCentre().toFloat();const auto time=juce::Time::getCurrentTime();
    const auto event=[&](juce::ModifierKeys mods){return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,mods,0,0,0,0,0,&button,&button,time,point,time,1,false);};
    static_cast<juce::Component&>(button).mouseDown(event(juce::ModifierKeys::leftButtonModifier));static_cast<juce::Component&>(button).mouseUp(event({}));
}
inline void cases()
{
    m3::ui::WorkspaceTheme theme;Panel panel;panel.setLookAndFeel(&theme);panel.addToDesktop(0);panel.setVisible(true);
    auto& selector=panel.groups[0];unsigned requests=0;int last=0;
    selector.grabKeyboardFocus();selector.moveKeyboardFocusToSibling(true);REQUIRE(panel.icons[0]->hasKeyboardFocus(true));
    panel.icons[0]->moveKeyboardFocusToSibling(false);REQUIRE(selector.hasKeyboardFocus(true));
    selector.onRequest=[&](int id){++requests;last=id;};
    selector.display(choices(),1);selector.display(choices(),1);REQUIRE(requests==0 && selector.selectedId()==1);
    pointerClick(*selector.segment(2));REQUIRE(requests==1 && last==2 && selector.selectedId()==1); // Rejection never paints optimistic selection.
    selector.display(choices(),2);REQUIRE(selector.selectedId()==2 && requests==1);
    pointerClick(*selector.segment(2));REQUIRE(requests==1);
    selector.display(choices(false),1);pointerClick(*selector.segment(2));REQUIRE(requests==1);
    selector.keyPressed(juce::KeyPress(juce::KeyPress::homeKey));selector.keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
    REQUIRE(selector.focusedId()==2 && selector.selectedId()==1 && requests==1);
    REQUIRE(selector.getTooltip().contains("Learn useful"));
    selector.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));REQUIRE(requests==1);
    auto* handler=selector.segment(2)->getAccessibilityHandler();REQUIRE(handler && handler->getDescription().contains("Learn useful"));
    REQUIRE(handler->getRole()==juce::AccessibilityRole::radioButton && !handler->getComponent().isEnabled());
    selector.display(choices(),1);selector.keyPressed(juce::KeyPress(juce::KeyPress::endKey));
    REQUIRE(selector.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)) && requests==2 && last==3 && selector.selectedId()==1);
    REQUIRE(selector.segment(1)->getAccessibilityHandler()->getCurrentState().isSelected());
    REQUIRE(!selector.segment(3)->getAccessibilityHandler()->getCurrentState().isSelected());
    // A request accepted by stale visual availability is still checked by the binding.
    workflow::State state;selector.onRequest=[&](int id){++requests;REQUIRE(!state.requestMode(static_cast<workflow::Mode>(id-1),{}));};
    selector.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));REQUIRE(requests==3 && selector.selectedId()==1);
    auto* wheelTarget=selector.segment(2);const auto point=wheelTarget->getLocalBounds().getCentre().toFloat();const auto time=juce::Time::getCurrentTime();
    juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(),point,{},0,0,0,0,0,wheelTarget,wheelTarget,time,point,time,0,false);
    juce::MouseWheelDetails wheel{};wheel.deltaY=.1f;wheelTarget->mouseWheelMove(event,wheel);REQUIRE(requests==3 && selector.selectedId()==1);
    auto disposable=std::make_unique<SegmentedSelector>();disposable->display(choices(),1);
    disposable->onRequest=[&](int){disposable.reset();};
    disposable->keyPressed(juce::KeyPress(juce::KeyPress::endKey));disposable->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));REQUIRE(!disposable);
    auto& icon=*panel.icons[0];unsigned clicks=0;icon.onClick=[&]{++clicks;};
    pointerClick(icon);REQUIRE(clicks==1);
    icon.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(clicks==2);
    icon.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(clicks==3);
    icon.setEnabled(false);pointerClick(icon);icon.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(clicks==3);icon.setEnabled(true);
    icon.setName("unrelated-name");const auto a=icon.createComponentSnapshot(icon.getLocalBounds());icon.setName("renamed-again");
    const auto b=icon.createComponentSnapshot(icon.getLocalBounds());
    for(int y=0;y<a.getHeight();++y)for(int x=0;x<a.getWidth();++x)REQUIRE(a.getPixelAt(x,y)==b.getPixelAt(x,y));
    {auto removed=std::make_unique<IconActionButton>(ActionIcon::settings);removed->onClick=[&]{++clicks;};removed->triggerClick();removed.reset();}
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(clicks==3);
    selector.onRequest={};selector.display(choices(),1);panel.groups[2].grabKeyboardFocus();
    // Internal joins are square and contiguous; only the outer silhouette rounds.
    const auto middle=selector.segment(2);const auto middleImage=middle->createComponentSnapshot(middle->getLocalBounds());
    REQUIRE(middleImage.getPixelAt(1,1)==m3::ui::background);
    const auto firstImage=selector.segment(1)->createComponentSnapshot(selector.segment(1)->getLocalBounds());
    REQUIRE(firstImage.getPixelAt(firstImage.getWidth()-2,1)==m3::ui::aqua);
    REQUIRE(selector.segment(1)->getRight()==selector.segment(2)->getX()+1 && selector.segment(2)->getRight()==selector.segment(3)->getX()+1);
    panel.groups[3].segment(2)->setState(juce::Button::buttonOver);panel.icons[3]->setState(juce::Button::buttonOver);
    REQUIRE(panel.groups[2].hasKeyboardFocus(true));snapshot(panel,"control-library-states");
    panel.setSize(560,540);
    for(const auto& group:panel.groups)for(int id:{1,2,3})REQUIRE(group.getLocalBounds().contains(group.segment(id)->getBounds()));
    snapshot(panel,"control-library-compact");
    panel.setLookAndFeel(nullptr);panel.removeFromDesktop();
}
inline void run()
{
    class Window final:public juce::DocumentWindow
    {
    public:
        Window():juce::DocumentWindow("ReverbArise control review",m3::ui::background,juce::DocumentWindow::closeButton)
        {setUsingNativeTitleBar(true);setContentOwned(new Panel,true);centreWithSize(720,570);setVisible(true);}
        void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
    };
    m3::ui::WorkspaceTheme theme;Window window;window.setLookAndFeel(&theme);
    juce::MessageManager::getInstance()->runDispatchLoop();window.setLookAndFeel(nullptr);
}
}
