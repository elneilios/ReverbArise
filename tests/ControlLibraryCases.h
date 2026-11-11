#pragma once
#include <UI/Controls/SegmentedSelector.h>
#include <UI/Controls/IconActionButton.h>
#include <UI/Controls/SliderControl.h>
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
        {2,"Perform","Shape and place reverse swells with precision",available,"Learn useful source audio first"},
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
        help.setText("Unavailable reasons stay accessible. Wheel does not switch modes.\nAqua fill marks selection; outer brackets mark keyboard focus.\nOther rows show component states; their requests do not change the model.",juce::dontSendNotification);
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
inline void connectedTabCases()
{
    struct WheelHost : juce::Component
    {
        int scrolls=0;
        void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&)override{++scrolls;}
    } host;
    SegmentedSelector tabs;
    host.setWantsKeyboardFocus(true);
    host.addAndMakeVisible(tabs);
    host.setSize(340,50);
    tabs.setBounds(0,0,330,36);
    host.addToDesktop(juce::ComponentPeer::windowIsTemporary);
    host.setVisible(true);
    tabs.display(choices(),1);
    int requests=0,last=0;
    tabs.onRequest=[&](int id){++requests;last=id;};
    auto* root=host.getAccessibilityHandler();
    auto* strip=tabs.getAccessibilityHandler();
    REQUIRE(root && strip);
    const auto checkNativeFocus=[&](int id)
    {
        auto* inspected=tabs.segment(id)->getAccessibilityHandler();
        REQUIRE(inspected && inspected->hasFocus(false) && !strip->hasFocus(false));
        REQUIRE(strip->getChildFocus()==inspected && root->getChildFocus()==inspected);
        REQUIRE(tabs.hasKeyboardFocus(false) && tabs.selectedId()==1);
    };
    tabs.grabKeyboardFocus();
    juce::MessageManager::getInstance()->runDispatchLoopUntil(10);
    checkNativeFocus(1);
    auto& middle=*tabs.segment(2);
    auto& button=static_cast<juce::Component&>(middle);
    const auto centre=button.getLocalBounds().getCentre().toFloat();
    const auto now=juce::Time::getCurrentTime();
    const auto mouse=[&](juce::Point<float> point,juce::ModifierKeys mods)
    {
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,mods,
            0,0,0,0,0,&button,&button,now,centre,now,1,point!=centre);
    };
    button.mouseDown(mouse(centre,juce::ModifierKeys::leftButtonModifier));
    REQUIRE(tabs.hasKeyboardFocus(false) && !tabs.isFocusVisible());
    REQUIRE(middle.isDown() && requests==0);
    button.mouseUp(mouse({-10,centre.y},{}));
    REQUIRE(!middle.isDown() && requests==0 && tabs.selectedId()==1);
    button.mouseDown(mouse(centre,juce::ModifierKeys::rightButtonModifier));
    button.mouseDrag(mouse(centre,juce::ModifierKeys::rightButtonModifier));
    button.mouseUp(mouse(centre,{}));
    REQUIRE(!middle.isDown() && requests==0);
    // JUCE's native mouseUp retains the old button mask in the event while
    // the input source already reports the released state.
    const auto primary=mouse(centre,juce::ModifierKeys::leftButtonModifier);
    REQUIRE(!primary.source.getCurrentModifiers().isAnyMouseButtonDown());
    button.mouseDown(primary);
    REQUIRE(middle.isDown() && requests==0);
    button.mouseUp(primary);
    REQUIRE(requests==1 && last==2 && tabs.selectedId()==1 && !tabs.isFocusVisible());
    checkNativeFocus(2);

    const auto body=middle.getBounds();
    const auto focusMargin=static_cast<int>(m3::ui::controlGeometry::focusOutset+m3::ui::controlGeometry::focusStroke);
    for(int id:{1,2,3})REQUIRE(tabs.getLocalBounds().contains(tabs.segment(id)->getBounds().expanded(focusMargin)));
    REQUIRE(tabs.segment(1)->getRight()==body.getX()+1 && body.getRight()==tabs.segment(3)->getX()+1);
    const auto surface=[&](juce::Button::ButtonState state)
    {
        middle.setState(state);
        return tabs.createComponentSnapshot(tabs.getLocalBounds());
    };
    const auto idle=surface(juce::Button::buttonNormal);
    const auto over=surface(juce::Button::buttonOver);
    const auto held=surface(juce::Button::buttonDown);
    const auto sample=body.getTopLeft()+juce::Point<int>{3,3};
    REQUIRE(idle.getPixelAt(sample.x,sample.y)!=over.getPixelAt(sample.x,sample.y));
    REQUIRE(over.getPixelAt(sample.x,sample.y)!=held.getPixelAt(sample.x,sample.y));
    REQUIRE(held.getPixelAt(body.getX(),body.getCentreY())==m3::ui::secondary);
    REQUIRE(over.getPixelAt(body.getX(),body.getCentreY())==m3::ui::outline.interpolatedWith(m3::ui::text,.5f));
    middle.setState(juce::Button::buttonNormal);

    tabs.keyPressed(juce::KeyPress(juce::KeyPress::homeKey));
    checkNativeFocus(1);
    tabs.keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
    checkNativeFocus(2);
    tabs.keyPressed(juce::KeyPress(juce::KeyPress::leftKey));
    checkNativeFocus(1);
    tabs.keyPressed(juce::KeyPress(juce::KeyPress::endKey));
    checkNativeFocus(3);
    REQUIRE(tabs.isFocusVisible() && tabs.focusedId()==3 && tabs.selectedId()==1);
    tabs.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));
    REQUIRE(requests==2 && last==3 && tabs.selectedId()==1);
    auto* accessible=middle.getAccessibilityHandler();
    REQUIRE(accessible && accessible->getActions().invoke(juce::AccessibilityActionType::focus));
    REQUIRE(tabs.isFocusVisible() && tabs.focusedId()==2 && accessible->getCurrentState().isFocused());
    checkNativeFocus(2);
    REQUIRE(!accessible->getCurrentState().isSelected());
    const auto focused=tabs.createComponentSnapshot(tabs.getLocalBounds());
    REQUIRE(focused.getPixelAt(body.getX()-focusMargin,0)==m3::ui::aqua);
    REQUIRE(accessible->getActions().invoke(juce::AccessibilityActionType::press));
    REQUIRE(requests==3 && last==2 && tabs.selectedId()==1);
    tabs.display(choices(false),1);
    checkNativeFocus(2);
    tabs.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    REQUIRE(requests==3 && tabs.focusedId()==2 && tabs.getTooltip().contains("Learn useful"));
    tabs.display(choices(),1);
    juce::MouseWheelDetails wheel{};wheel.deltaY=.1f;
    button.mouseWheelMove(mouse(centre,{}),wheel);
    REQUIRE(host.scrolls==1 && requests==3 && tabs.selectedId()==1);
    host.grabKeyboardFocus();
    REQUIRE(strip->getChildFocus()==nullptr && !accessible->hasFocus(false));
    host.removeFromDesktop();
}
inline void cases(bool withIcons=false)
{
    const auto choices=[withIcons](bool available=true)
    {
        auto values=controlReview::choices(available);
        if(withIcons){values[0].icon=m3::ui::controls::ModeIcon::learn;values[1].icon=m3::ui::controls::ModeIcon::perform;values[2].icon=m3::ui::controls::ModeIcon::exportAudio;}
        return values;
    };
    {
        m3::ui::controls::SliderControl value;value.setRange(0,100,.1);value.setValue(50,juce::dontSendNotification);value.setSize(304,34);
        unsigned begins=0,changes=0,gestureStarts=0,gestureEnds=0;value.beforeInvocation=[&]{++begins;};value.onValueChange=[&]{++changes;};
        value.onDragStart=[&]{++gestureStarts;};value.onDragEnd=[&]{++gestureEnds;};
        auto* field=dynamic_cast<m3::ui::controls::NumericInputControl*>(value.getChildComponent(0));REQUIRE(field);
        pointerClick(field->incrementButton());REQUIRE(value.getValue()==50.1 && changes==1 && begins>0);
        pointerClick(field->decrementButton());REQUIRE(value.getValue()==50 && changes==2);
        REQUIRE(gestureStarts==2 && gestureEnds==2);
        static_cast<juce::Component&>(field->incrementButton()).keyPressed(juce::KeyPress(juce::KeyPress::returnKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(value.getValue()==50.1);
        field->showEditor();REQUIRE(field->getCurrentTextEditor() && field->getCurrentTextEditor()->getRight()<=field->incrementButton().getX());
        field->getCurrentTextEditor()->setText("75.5",false);field->hideEditor(false);juce::MessageManager::getInstance()->runDispatchLoopUntil(30);REQUIRE(value.getValue()==75.5);
        value.setValue(100,juce::dontSendNotification);pointerClick(field->incrementButton());REQUIRE(value.getValue()==100);
        value.setValue(0,juce::dontSendNotification);pointerClick(field->decrementButton());REQUIRE(value.getValue()==0);
        value.setEnabled(false);pointerClick(field->incrementButton());REQUIRE(value.getValue()==0);value.setEnabled(true);
        REQUIRE(field->getLocalBounds().contains(field->incrementButton().getBounds()) && field->getLocalBounds().contains(field->decrementButton().getBounds()));
    }
    {
        using namespace m3::ui::controls;
        SliderControl track{{"Sensitivity (%)","Controls event detection."}};
        track.setRange(0,100,.1);track.setWheelSteps(1,.1);
        track.setSize(304,34);track.setValue(50,juce::dontSendNotification);
        auto stamp=juce::Time::currentTimeMillis();
        const auto event=[&](juce::Component& target,juce::Point<float> point,juce::Point<float> start,int flags)
        {
            const juce::Time now(++stamp);
            return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,flags,
                0,0,0,0,0,&target,&target,now,start,now,1,point!=start);
        };
        unsigned invocations=0;
        track.beforeInvocation=[&]{++invocations;};
        const auto wheelAt=[&](juce::Component& target,juce::Point<float> point,int flags,bool reversed=false)
        {
            juce::MouseWheelDetails wheel{};
            wheel.deltaY=.15f;
            wheel.isReversed=reversed;
            target.mouseWheelMove(event(target,point,point,flags),wheel);
        };
        const auto ctrl=juce::ModifierKeys::ctrlModifier;
        wheelAt(track,{80,17},0);
        REQUIRE(track.getValue()==51 && invocations==1);
        wheelAt(track,{80,17},ctrl,true);
        REQUIRE(std::abs(track.getValue()-50.9)<1.e-8 && invocations==2);
        wheelAt(track,{-2,17},ctrl);
        REQUIRE(std::abs(track.getValue()-50.9)<1.e-8 && invocations==2);
        auto* field=dynamic_cast<FieldNumericInput*>(track.getChildComponent(0));REQUIRE(field);
        wheelAt(*field,{15,10},0);
        REQUIRE(std::abs(track.getValue()-51)<1.e-8 && invocations==3);
        const auto drag=[&](int modifiers)
        {
            track.setValue(50,juce::dontSendNotification);
            const juce::Point<float> start{80,17},end{130,17};
            track.mouseDown(event(track,start,start,modifiers|juce::ModifierKeys::leftButtonModifier));
            track.mouseDrag(event(track,end,start,modifiers|juce::ModifierKeys::leftButtonModifier));
            track.mouseUp(event(track,end,start,modifiers));
            return track.getValue();
        };
        REQUIRE(drag(0)==drag(ctrl));
        track.setValue(99.9,juce::dontSendNotification);
        wheelAt(track,{80,17},0);REQUIRE(track.getValue()==100);
        track.addToDesktop(juce::ComponentPeer::windowIsTemporary);
        track.setVisible(true);
        track.setEnabled(false);
        auto* handler=track.getAccessibilityHandler();REQUIRE(handler && handler->getTitle()=="Sensitivity (%)");
        auto* accessible=handler->getValueInterface();REQUIRE(accessible && accessible->isReadOnly());
        accessible->setValue(60);wheelAt(track,{80,17},ctrl,true);REQUIRE(track.getValue()==100);
        track.setEnabled(true);
        handler=track.getAccessibilityHandler();
        REQUIRE(handler && handler->getActions().invoke(juce::AccessibilityActionType::focus));
        REQUIRE(track.isFocusVisible());
        accessible=handler->getValueInterface();
        const auto beforeAccessible=invocations;
        accessible->setValue(60);REQUIRE(track.getValue()==60 && invocations==beforeAccessible+1);
        track.setRange(0,100,1);track.setWheelSteps(1,1);
        wheelAt(track,{80,17},ctrl);REQUIRE(track.getValue()==61);
        auto press=event(track,{80,17},{80,17},juce::ModifierKeys::leftButtonModifier);
        track.mouseDown(press);track.mouseUp(event(track,{80,17},{80,17},0));
        REQUIRE(track.hasKeyboardFocus(false) && !track.isFocusVisible());
        track.removeFromDesktop();

        const auto surface=[&](CommandState state)
        {
            const juce::Rectangle<int> bounds{100,34};
            juce::Image image(juce::Image::ARGB,bounds.getWidth(),bounds.getHeight(),true);
            juce::Graphics g(image);
            DialTrackPresentation::track(g,bounds,state,10,90,50);
            return image;
        };
        const auto idle=surface({true,false,false,false});
        const auto hovered=surface({true,true,false,false});
        const auto held=surface({true,true,true,false});
        const auto focused=surface({true,false,false,true});
        const auto disabled=surface({false,true,true,true});
        REQUIRE(idle.getPixelAt(30,8).getAlpha()==0 && hovered.getPixelAt(30,8).getAlpha()>0);
        REQUIRE(held.getPixelAt(30,8).getAlpha()>hovered.getPixelAt(30,8).getAlpha());
        REQUIRE(focused.getPixelAt(1,1)==m3::ui::aqua && disabled.getPixelAt(1,1).getAlpha()==0);
        REQUIRE(disabled.getPixelAt(50,17).getAlpha()<idle.getPixelAt(50,17).getAlpha());
    }
    if(!withIcons)connectedTabCases();
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
    REQUIRE(icon.getLocalBounds().toFloat().contains(icon.outlineBounds(true).expanded(m3::ui::controlGeometry::focusStroke*.5f)));
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
    if(withIcons)for(int width:{330,360})
    {
        selector.setSize(width,36);
        for(int id:{1,2,3})
        {
            auto* button=selector.segment(id);
            const auto labelWidth=juce::GlyphArrangement::getStringWidth(m3::ui::navigationFont(),button->getButtonText());
            REQUIRE(16+8+labelWidth<=button->getWidth()-10);
            REQUIRE(button->getAccessibilityHandler()->getTitle()==button->getButtonText());
        }
    }
    // Internal joins are square and contiguous; only the outer silhouette rounds.
    const auto middle=selector.segment(2);const auto middleImage=middle->createComponentSnapshot(middle->getLocalBounds());
    REQUIRE(middleImage.getPixelAt(1,1)==m3::ui::background.interpolatedWith(m3::ui::outline,.07f));
    const auto firstImage=selector.segment(1)->createComponentSnapshot(selector.segment(1)->getLocalBounds());
    REQUIRE(firstImage.getPixelAt(firstImage.getWidth()-2,1)==m3::ui::aqua);
    REQUIRE(selector.segment(1)->getRight()==selector.segment(2)->getX()+1 && selector.segment(2)->getRight()==selector.segment(3)->getX()+1);
    // A hovered neighbour keeps every edge when another segment is selected,
    // including the left neighbour formerly overpainted by the selected child.
    for(int selected=1;selected<=3;++selected)for(int hovered=1;hovered<=3;++hovered)if(std::abs(selected-hovered)==1)
    {
        panel.icons[0]->grabKeyboardFocus();selector.display(choices(),selected);selector.grabKeyboardFocus();
        for(int id=1;id<=3;++id)selector.segment(id)->setState(juce::Button::buttonNormal);
        auto* hover=selector.segment(hovered);hover->setState(juce::Button::buttonOver);
        const auto frame=selector.createComponentSnapshot(selector.getLocalBounds());
        const auto shared=hovered<selected?hover->getRight()-1:hover->getX();
        const auto edge=m3::ui::outline.interpolatedWith(m3::ui::text,.5f);
        const auto body=hover->getBounds();
        REQUIRE(frame.getPixelAt(shared,body.getCentreY())==edge);
        REQUIRE(frame.getPixelAt(body.getCentreX(),body.getY())==edge && frame.getPixelAt(body.getCentreX(),body.getBottom()-1)==edge);
        REQUIRE(selector.selectedId()==selected && selector.focusedId()==selected);
        for(int id=1;id<=3;++id)REQUIRE(selector.getChildComponent(id-1)==selector.segment(id));
    }
    panel.groups[2].grabKeyboardFocus();selector.display(choices(),1);
    panel.groups[3].display(choices(),3);
    panel.groups[3].segment(2)->setState(juce::Button::buttonOver);panel.icons[3]->setState(juce::Button::buttonOver);
    REQUIRE(panel.groups[2].hasKeyboardFocus(true));snapshot(panel,"control-library-states");
    panel.setSize(560,540);
    for(const auto& group:panel.groups)for(int id:{1,2,3})REQUIRE(group.getLocalBounds().contains(group.segment(id)->getBounds()));
    snapshot(panel,"control-library-compact");
    panel.setLookAndFeel(nullptr);panel.removeFromDesktop();
}
inline void run(std::unique_ptr<juce::Component> content=std::make_unique<Panel>())
{
    class Window final:public juce::DocumentWindow
    {
    public:
        explicit Window(std::unique_ptr<juce::Component> body):juce::DocumentWindow("ReverbArise control review",m3::ui::background,juce::DocumentWindow::closeButton)
        {const auto width=body->getWidth(),height=body->getHeight();setUsingNativeTitleBar(true);setContentOwned(body.release(),true);centreWithSize(width,height+30);setVisible(true);}
        void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
    };
    m3::ui::WorkspaceTheme theme;Window window(std::move(content));window.setLookAndFeel(&theme);
    juce::MessageManager::getInstance()->runDispatchLoop();window.setLookAndFeel(nullptr);
}
}
