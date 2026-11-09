#include "TestSupport.h"
#include <UI/Controls/PopupInteraction.h>
#include <UI/Controls/ChoicePopup.h>
#include <UI/Controls/ChoiceSelector.h>
#include "ActionMenuReview.h"
#include "FieldFeedbackReview.h"
#include "NumericTextFieldsReview.h"
#include "SelectorReview.h"
#if JUCE_WINDOWS
#include <windows.h>
#endif
using namespace m3::ui::controls;

static std::vector<PopupChoice> choices()
{return {{11,"Accepted",{},true,true},{42,"Other",{},true,false},{73,"Unavailable","Not currently eligible",false,false}};}

static void interactionCases()
{
    PopupInteraction state(choices());
    REQUIRE(state.inspected()==-1 && !state.done());
    REQUIRE(state.items()[0].selected);
    state.pointerEnter(1);REQUIRE(state.inspected()==1);
    state.pointerLeave(1);REQUIRE(state.inspected()==-1 && state.items()[0].selected);
    state.keyboardInspect(0);state.pointerEnter(1);REQUIRE(state.inspected()==1);
    state.pointerLeave(1);REQUIRE(state.inspected()==0 && state.keyboardIndex()==0);
    state.pointerEnter(2);state.pointerExit();REQUIRE(state.inspected()==0);
    state.navigationLost();REQUIRE(state.inspected()==-1);
    state.keyboardInspect(2);REQUIRE(state.inspected()==2);
    REQUIRE(state.complete(2)==0 && !state.done() && state.items()[0].selected);
    state.pointerEnter(1);state.pointerDown(1);REQUIRE(state.keyboardIndex()==-1 && state.heldIndex()==1);
    state.pointerUp();state.pointerLeave(1);REQUIRE(state.inspected()==-1);
    state.keyboardInspect(1);REQUIRE(state.complete(1)==42 && state.done());
    REQUIRE(state.complete(0)==0 && state.complete(1)==0 && state.inspected()==-1);
    state.pointerEnter(0);state.keyboardInspect(1);REQUIRE(state.inspected()==-1);
    PopupInteraction cancelled(choices());cancelled.cancel();REQUIRE(cancelled.complete(0)==0);
    PopupInteraction empty({});empty.pointerEnter(0);empty.keyboardInspect(-1);REQUIRE(empty.inspected()==-1 && empty.complete(0)==0);
    PopupInteraction invalid(choices());REQUIRE(invalid.complete(-1)==0 && invalid.complete(999)==0);
    invalid.pointerEnter(1);invalid.keyboardInspect(0);REQUIRE(invalid.inspected()==0);
    invalid.pointerLeave(1);REQUIRE(invalid.inspected()==0); // A late pointer exit cannot erase keyboard navigation.
}
static void pump(int ms=40){juce::MessageManager::getInstance()->runDispatchLoopUntil(ms);}
static juce::MouseEvent at(ChoicePopup& popup,int index,juce::ModifierKeys mods={})
{
    const auto p=index>=0?popup.getLocalPoint(&popup.rowComponent(index),popup.rowComponent(index).getLocalBounds().getCentre()):juce::Point<int>{0,0};
    const auto time=juce::Time::getCurrentTime();
    return {juce::Desktop::getInstance().getMainMouseSource(),p.toFloat(),mods,0,0,0,0,0,&popup,&popup,time,p.toFloat(),time,1,false};
}
static void popupCases()
{
    using m3::ui::WorkspaceTheme;
    const juce::Rectangle<int> work{100,100,800,600};
    for(const auto anchor:{juce::Rectangle<int>{100,100,250,34},{850,660,100,34},{0,0,20,20}})
        REQUIRE(work.contains(ChoicePopup::placedBounds(anchor,work,420,500)));
    REQUIRE(work.contains(ChoicePopup::placedBounds({100,100,20,20},work,10000,10000)));
    WorkspaceTheme theme;juce::Component window;window.setLookAndFeel(&theme);window.setBounds(80,80,900,600);window.addToDesktop(0);window.setVisible(true);window.toFront(true);
    juce::Component origin;window.addAndMakeVisible(origin);origin.setBounds(40,40,260,44);origin.setWantsKeyboardFocus(true);origin.setTitle("IR");
    int calls=0,id=-1;PopupDismissal why=PopupDismissal::cancel;
    auto completion=[&](int value,PopupDismissal reason){++calls;id=value;why=reason;};
    {
        ChoicePopup popup(origin,choices(),completion);popup.show(origin.getScreenBounds(),false);
        REQUIRE(popup.state().inspected()==-1);
        popup.mouseMove(at(popup,1));REQUIRE(popup.state().inspected()==1);
        popup.mouseMove(at(popup,-1));REQUIRE(popup.state().inspected()==-1);
        popup.inspect(0);popup.mouseMove(at(popup,1));REQUIRE(popup.state().inspected()==1);
        popup.mouseExit(at(popup,-1));REQUIRE(popup.state().inspected()==0);
        auto* unavailable=popup.rowComponent(2).getAccessibilityHandler();REQUIRE(unavailable);
        REQUIRE(!unavailable->getComponent().isEnabled());
        REQUIRE(unavailable->getDescription().contains("Not currently eligible"));
        REQUIRE(unavailable->getActions().invoke(juce::AccessibilityActionType::focus));
        REQUIRE(popup.state().keyboardIndex()==2);
        REQUIRE(unavailable->getActions().invoke(juce::AccessibilityActionType::press));
        REQUIRE(!popup.state().done() && calls==0);
        popup.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));REQUIRE(!popup.state().done());
        popup.keyPressed(juce::KeyPress(juce::KeyPress::upKey));popup.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));
        popup.activate(0);pump();REQUIRE(calls==1 && id==42 && why==PopupDismissal::commit);
    }
    {
        ChoicePopup popup(origin,choices(),completion);popup.show(origin.getScreenBounds(),false);
        popup.mouseDown(at(popup,0,juce::ModifierKeys::leftButtonModifier));popup.mouseUp(at(popup,0));popup.mouseUp(at(popup,0));
        pump();REQUIRE(calls==2 && id==11);
    }
    for(const auto key:{juce::KeyPress::escapeKey,juce::KeyPress::tabKey})
    {
        ChoicePopup popup(origin,choices(),completion);popup.show(origin.getScreenBounds(),true);popup.keyPressed(juce::KeyPress(key));pump();
        REQUIRE(id==0 && why==(key==juce::KeyPress::escapeKey?PopupDismissal::cancel:PopupDismissal::tabForward));
    }
    {
        ChoicePopup popup(origin,choices(),completion);popup.show(origin.getScreenBounds(),true);popup.inputAttemptWhenModal();pump();REQUIRE(id==0 && why==PopupDismissal::outside);
    }
    {
        auto source=std::make_unique<juce::Component>();window.addAndMakeVisible(*source);source->setBounds(origin.getBounds());
        ChoicePopup popup(*source,choices(),completion);popup.show(source->getScreenBounds(),false);source.reset();pump(100);REQUIRE(popup.state().done() && why==PopupDismissal::ownerGone && id==0);
    }
    window.setLookAndFeel(nullptr);
}
static juce::Image selectorImage(ChoiceSelector& selector)
{juce::Image image(juce::Image::ARGB,selector.getWidth(),selector.getHeight(),true);juce::Graphics g(image);g.fillAll(m3::ui::background);selector.paintEntireComponent(g,true);return image;}
static bool same(const juce::Image& a,const juce::Image& b)
{for(int y=0;y<a.getHeight();++y)for(int x=0;x<a.getWidth();++x)if(a.getPixelAt(x,y)!=b.getPixelAt(x,y))return false;return true;}
static void actionMenuCases()
{
    using namespace m3::ui;
    using Role = PopupRowRole;
    actionMenuReview::Panel panel;
    panel.addToDesktop(0);
    panel.setVisible(true);
    panel.toFront(true);
    auto& trigger = panel.fixture.reset;
    const juce::String reason = "Automatic start is unavailable because this fixture has no source range";
    const std::vector<PopupChoice> rows{
        {11, "Delete", {}, true, false, Role::action},
        {42, "Rename", {}, true, false, Role::destructiveAction},
        {73, "Reset to the initial value captured for this swell before the current edit was made", reason, false, false, Role::action},
        {74, "Delete", {}, true, true, Role::action}};
    int calls = 0, chosen = 0;
    PopupDismissal dismissal = PopupDismissal::cancel;
    const auto completion = [&](int id, PopupDismissal why) { ++calls; chosen = id; dismissal = why; };

    {
        ChoicePopup popup(trigger, rows, completion);
        popup.show(trigger.getScreenBounds(), false);
        const auto image = [&](int index)
        {
            auto& row = popup.rowComponent(index);
            juce::Image pixels(juce::Image::RGB, row.getWidth(), row.getHeight(), true, juce::SoftwareImageType{});
            juce::Graphics g(pixels);
            g.fillAll(background);
            row.paint(g);
            return pixels;
        };
        const auto fill = [](const juce::Image& pixels) { return pixels.getPixelAt(pixels.getWidth() - 2, pixels.getHeight() / 2); };
        const auto edge = [](const juce::Image& pixels) { return pixels.getPixelAt(0, pixels.getHeight() / 2); };
        REQUIRE(popup.state().inspected() == -1);
        REQUIRE(same(image(0), image(3)));
        const auto ordinaryState = popup.rowComponent(3).getAccessibilityHandler()->getCurrentState();
        REQUIRE(!ordinaryState.isSelectable() && !ordinaryState.isSelected());

        popup.mouseMove(at(popup, 0));
        REQUIRE(edge(image(0)) == aqua && fill(image(0)) == SelectorPresentation::raised());
        popup.mouseDown(at(popup, 0, juce::ModifierKeys::leftButtonModifier));
        REQUIRE(fill(image(0)) == SelectorPresentation::held());
        popup.mouseMove(at(popup, -1));
        REQUIRE(fill(image(0)) == background);
        popup.mouseUp(at(popup, -1));

        popup.inspect(0);
        popup.mouseMove(at(popup, 1));
        const auto redHover = fill(image(1));
        REQUIRE(edge(image(1)) == danger && redHover.getRed() > redHover.getGreen());
        popup.mouseExit(at(popup, -1));
        REQUIRE(popup.state().inspected() == 0 && fill(image(1)) == background);

        auto& unavailable = popup.rowComponent(2);
        auto* handler = unavailable.getAccessibilityHandler();
        REQUIRE(handler && !unavailable.isEnabled() && handler->getDescription() == reason);
        REQUIRE(unavailable.getHeight() > controlGeometry::height);
        popup.mouseDown(at(popup, 2, juce::ModifierKeys::leftButtonModifier));
        popup.mouseUp(at(popup, 2));
        REQUIRE(handler->getActions().invoke(juce::AccessibilityActionType::focus));
        REQUIRE(popup.state().keyboardIndex() == 2);
        popup.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
        REQUIRE(handler->getActions().invoke(juce::AccessibilityActionType::press));
        REQUIRE(calls == 0 && !popup.state().done());

        popup.mouseDown(at(popup, 1, juce::ModifierKeys::leftButtonModifier));
        const auto redHeld = fill(image(1));
        REQUIRE(edge(image(1)) == danger && redHeld.getRed() > redHover.getRed());
        popup.mouseUp(at(popup, 1));
        popup.mouseExit(at(popup, -1));
        popup.activate(0);
        REQUIRE(popup.state().inspected() == -1 && fill(image(1)) == background);
        pump();
        REQUIRE(calls == 1 && chosen == 42 && dismissal == PopupDismissal::commit);
    }
    for (const auto key : {juce::KeyPress::returnKey, juce::KeyPress::spaceKey, 0})
    {
        ChoicePopup popup(trigger, rows, completion);
        popup.show(trigger.getScreenBounds(), true);
        const auto before = calls;
        if (key == 0)
            REQUIRE(popup.rowComponent(0).getAccessibilityHandler()->getActions().invoke(juce::AccessibilityActionType::press));
        else
            popup.keyPressed(juce::KeyPress(key));
        popup.activate(1);
        pump();
        REQUIRE(calls == before + 1 && chosen == 11 && dismissal == PopupDismissal::commit);
    }

    auto& list = panel.fixture.swells;
    for (const bool rightClick : {true, false})
    {
        const auto row = list.getRowPosition(0, true);
        const juce::Point<int> point{rightClick ? row.getX() + 70 : row.getRight() - 12, row.getCentreY()};
        auto* surface = list.getComponentAt(point);
        REQUIRE(surface != nullptr && surface != &list);
        const auto position = surface->getLocalPoint(&list, point).toFloat();
        const auto now = juce::Time::getCurrentTime();
        const juce::ModifierKeys mods(rightClick ? juce::ModifierKeys::rightButtonModifier : juce::ModifierKeys::leftButtonModifier);
        const juce::MouseEvent click(juce::Desktop::getInstance().getMainMouseSource(), position, mods,
                                     0, 0, 0, 0, 0, surface, surface, now, position, now, 1, false);
        surface->mouseDown(click);
        surface->mouseUp(click);
        REQUIRE(panel.fixture.popup && panel.fixture.popup->state().items()[0].label == "Rename...");
        REQUIRE(panel.fixture.popup->state().items()[1].role == Role::destructiveAction);
        panel.fixture.popup->dismiss();
        pump();
    }

    for (const auto size : actionMenuReview::Panel::reviewSizes)
        for (const auto scale : {100, 125})
        {
            panel.setSize(size.x, size.y);
            panel.setReviewScale(scale);
            auto& fixture = panel.fixture;
            for (auto* control : std::array<juce::Component*, 3>{&fixture.swells, &fixture.reset, &fixture.edge})
                REQUIRE(panel.getLocalBounds().contains(panel.getLocalArea(control, control->getLocalBounds())));

            fixture.open(fixture.reset, true, false);
            REQUIRE(fixture.popup && fixture.popup->rowComponent(1).getHeight() > controlGeometry::height);
            fixture.popup->dismiss();
            pump();
            fixture.open(fixture.edge, false, true);
            REQUIRE(fixture.popup != nullptr);
            const auto bounds = fixture.popup->getScreenBounds();
            const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(bounds);
            REQUIRE(display && display->userArea.contains(bounds));
            REQUIRE(bounds.getRight() == display->userArea.getRight());
            fixture.popup->inputAttemptWhenModal();
            pump();
            REQUIRE(fixture.popup == nullptr);
        }
}
static void selectorCases()
{
    using namespace m3::ui;
    WorkspaceTheme theme;juce::Component window;window.setLookAndFeel(&theme);window.setBounds(80,80,900,600);window.addToDesktop(0);window.setVisible(true);window.toFront(true);
    ChoiceSelector selector;juce::TextButton sibling("Next");window.addAndMakeVisible(selector);window.addAndMakeVisible(sibling);
    selector.setBounds(30,30,280,44);sibling.setBounds(330,30,100,44);sibling.setWantsKeyboardFocus(true);
    selector.setTitle("Default IR");selector.addItem("Preverb Hall",11);selector.addItem("Other",42);selector.addItem("Custom",73);selector.setItemEnabled(73,false);selector.setChoiceReason(73,"Custom reflects the accepted pair; choose a named preset to change it");
    selector.setSelectedId(11,juce::dontSendNotification);
    int prepared=0,changes=0;selector.beforeInvocation=[&]{++prepared;};selector.onChange=[&]{++changes;};
    juce::SharedResourcePointer<FocusVisibility> provenance;provenance->pointerInput();selector.grabKeyboardFocus();
    auto* handler=selector.getAccessibilityHandler();REQUIRE(handler && handler->getActions().invoke(juce::AccessibilityActionType::focus));
    REQUIRE(selector.isFocusVisible());const auto focused=selectorImage(selector);
    const auto p=selector.getLocalBounds().getCentre().toFloat();const auto time=juce::Time::getCurrentTime();
    const auto event=[&](juce::ModifierKeys mods){return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),p,mods,0,0,0,0,0,&selector,&selector,time,p,time,1,false);};
    selector.mouseDown(event(juce::ModifierKeys::leftButtonModifier));REQUIRE(!selector.isFocusVisible());
    const auto held=selectorImage(selector);REQUIRE(!same(focused,held));selector.mouseUp(event({}));
    REQUIRE(selector.isPopupActive() && prepared==1 && changes==0 && selector.activePopup()->state().inspected()==-1);
    auto* popup=selector.activePopup();popup->mouseMove(at(*popup,1));popup->mouseExit(at(*popup,-1));REQUIRE(popup->state().inspected()==-1);
    popup->keyPressed(juce::KeyPress(juce::KeyPress::homeKey));REQUIRE(selector.isFocusVisible());
    popup->keyPressed(juce::KeyPress(juce::KeyPress::downKey));REQUIRE(selector.getSelectedId()==11 && changes==0);
    popup->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();REQUIRE(selector.getSelectedId()==42 && changes==1 && prepared==1 && !selector.isPopupActive() && selector.isFocusVisible());
    selector.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));REQUIRE(prepared==2);
    selector.activePopup()->keyPressed(juce::KeyPress(juce::KeyPress::endKey));selector.activePopup()->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));REQUIRE(changes==1 && selector.isPopupActive());
    selector.activePopup()->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));pump();REQUIRE(changes==1 && selector.getSelectedId()==42 && selector.hasKeyboardFocus(true));
    selector.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));selector.activePopup()->keyPressed(juce::KeyPress(juce::KeyPress::tabKey));pump();REQUIRE(sibling.hasKeyboardFocus(true));
    selector.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));selector.activePopup()->activate(0);selector.clear(juce::dontSendNotification);selector.addItem("Replacement",11);selector.setSelectedId(11,juce::dontSendNotification);pump();REQUIRE(changes==1 && selector.getText()=="Replacement");
    const auto before=prepared;selector.setEnabled(false);selector.showPopup();handler->getActions().invoke(juce::AccessibilityActionType::showMenu);REQUIRE(prepared==before && !selector.isPopupActive());
    selector.setEnabled(true);selector.setSelectedId(11,juce::dontSendNotification);selector.showPopup();selector.setEnabled(false);pump();REQUIRE(!selector.isPopupActive() && changes==1);
    selector.setEnabled(true);selector.mouseDown(event(juce::ModifierKeys::leftButtonModifier));selector.mouseUp(event({}));REQUIRE(selector.isPopupActive());
    selector.activePopup()->mouseDown(at(*selector.activePopup(),0,juce::ModifierKeys::leftButtonModifier));selector.activePopup()->mouseUp(at(*selector.activePopup(),0));pump();
    REQUIRE(selector.hasKeyboardFocus(true) && !selector.isFocusVisible() && changes==1);
    selector.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));selector.activePopup()->keyPressed(juce::KeyPress(juce::KeyPress::tabKey,juce::ModifierKeys::shiftModifier,0));pump();REQUIRE(sibling.hasKeyboardFocus(true));
    auto disappearing=std::make_unique<ChoiceSelector>();window.addAndMakeVisible(*disappearing);disappearing->setBounds(30,100,280,44);disappearing->addItem("A",1);disappearing->addItem("B",2);disappearing->setSelectedId(1,juce::dontSendNotification);
    disappearing->onChange=[&]{++changes;};disappearing->showPopup();disappearing->activePopup()->activate(1);disappearing.reset();pump();REQUIRE(changes==1);
    auto destroyedInPrepare=std::make_unique<ChoiceSelector>();window.addAndMakeVisible(*destroyedInPrepare);destroyedInPrepare->setBounds(30,100,280,44);destroyedInPrepare->beforeInvocation=[&]{destroyedInPrepare.reset();};destroyedInPrepare->showPopup();REQUIRE(!destroyedInPrepare);
    auto destroyedInCommit=std::make_unique<ChoiceSelector>();window.addAndMakeVisible(*destroyedInCommit);destroyedInCommit->setBounds(30,100,280,44);destroyedInCommit->addItem("A",1);destroyedInCommit->addItem("B",2);destroyedInCommit->setSelectedId(1,juce::dontSendNotification);
    destroyedInCommit->onChange=[&]{destroyedInCommit.reset();};destroyedInCommit->showPopup();destroyedInCommit->activePopup()->activate(1);pump();REQUIRE(!destroyedInCommit);

   #if JUCE_WINDOWS
    // A host control can take native focus without going through JUCE modal input.
    const auto hostControl=CreateWindowExW(WS_EX_TOOLWINDOW,L"EDIT",L"",WS_POPUP|WS_BORDER,
                                            400,160,200,30,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    REQUIRE(hostControl != nullptr);
    const juce::ScopeGuard destroyHostControl{[hostControl]{DestroyWindow(hostControl);}};
    ShowWindow(hostControl,SW_SHOWNOACTIVATE);

    const auto accepted=selector.getSelectedId();
    const auto changesBefore=changes;
    selector.showPopup();
    auto* openPopup=selector.activePopup();
    REQUIRE(openPopup && openPopup->isCurrentlyModal());
    REQUIRE(GetFocus() == static_cast<HWND>(openPopup->getWindowHandle()));

    SetFocus(hostControl);
    REQUIRE(GetFocus() == hostControl);
    // Check cancellation before dispatching the foreground polling timer.
    REQUIRE(!selector.isPopupActive());
    pump(100);
    REQUIRE(selector.activePopup() == nullptr);
    REQUIRE(juce::Component::getCurrentlyModalComponent() == nullptr);
    REQUIRE(GetFocus() == hostControl);
    REQUIRE(selector.getSelectedId() == accepted && changes == changesBefore);
   #endif
    window.setLookAndFeel(nullptr);
}
static void presentationCases()
{
    using namespace m3::ui;
    for(bool enabled:{false,true})for(bool hovered:{false,true})for(bool down:{false,true})
    {
        std::array<juce::Image,2> images;
        for(int focus=0;focus<2;++focus)
        {
            images[static_cast<size_t>(focus)]=juce::Image(juce::Image::ARGB,280,44,true);juce::Graphics g(images[static_cast<size_t>(focus)]);g.fillAll(background);
            SelectorPresentation::surface(g,{0,0,280,44},{enabled,hovered,down,focus!=0,false});
        }
        const auto body=commandBodyBounds({0,0,280,44});
        for(int y=body.getY();y<body.getBottom();++y)for(int x=body.getX();x<body.getRight();++x)REQUIRE(images[0].getPixelAt(x,y)==images[1].getPixelAt(x,y));
        REQUIRE(!same(images[0],images[1]));
        if(!enabled)REQUIRE(images[0].getPixelAt(140,22)==background);
        else REQUIRE(images[0].getPixelAt(140,22)==(down?SelectorPresentation::held():hovered?SelectorPresentation::raised():background));
    }
    WorkspaceTheme theme;juce::Component window;window.setLookAndFeel(&theme);window.setBounds(80,80,900,600);window.addToDesktop(0);window.setVisible(true);window.toFront(true);
    juce::Component content;window.addAndMakeVisible(content);content.setBounds(0,0,600,450);
    ChoiceSelector selector;content.addAndMakeVisible(selector);selector.setBounds(24,24,220,44);selector.setTitle("Long choice");
    const auto full=juce::String::fromUTF8("Imported \xe2\x80\x94 Cathedral North Balcony, stereo capture, long decay with full metadata");
    selector.addItem(full,1);selector.addItem("Custom",2);selector.setItemEnabled(2,false);selector.setChoiceReason(2,"Custom reflects the accepted mix and cannot be invoked as a preset");selector.setSelectedId(1,juce::dontSendNotification);
    REQUIRE(selector.getTooltip().contains(full));REQUIRE(selector.getAccessibilityHandler()->getDescription().contains(full));
    REQUIRE(selector.getAccessibilityHandler()->getValueInterface()->getCurrentValueAsString()==full);
    for(const auto factor:{1.f,1.25f})
    {
        content.setTransform(juce::AffineTransform::scale(factor));selector.showPopup();auto* popup=selector.activePopup();REQUIRE(popup);
        const auto& first=popup->rowComponent(0);auto& second=popup->rowComponent(1);
        REQUIRE(first.getHeight()>controlGeometry::height*factor);
        REQUIRE(first.getBottom()<=second.getY());
        REQUIRE(!second.isEnabled() && second.getAccessibilityHandler()->getDescription().contains("cannot be invoked"));
        popup->inspect(1,true);REQUIRE(popup->state().keyboardIndex()==1 && !popup->state().items()[1].eligible);
        popup->activate(1);REQUIRE(selector.getSelectedId()==1 && selector.isPopupActive());selector.hidePopup();pump();
    }
    selector.setSelectedId(2,juce::dontSendNotification);selector.showPopup();REQUIRE(selector.activePopup()->state().items()[1].selected);selector.hidePopup();pump();
    window.setLookAndFeel(nullptr);
    selectorReview::Panel preview;
    for(const auto size:{juce::Point<int>{900,600},{1000,700},{1100,850},{1400,1000}})for(const auto scale:{100,125})
    {
        preview.setSize(size.x,size.y);preview.setReviewScale(scale);
        for(auto* child:std::array<juce::Component*,5>{&preview.fixture.normal,&preview.fixture.longName,&preview.fixture.empty,&preview.fixture.unavailable,&preview.fixture.folder})REQUIRE(preview.fixture.getLocalBounds().contains(child->getBounds()));
        REQUIRE(commandBodyBounds(preview.fixture.normal.getBounds()).getHeight()==commandBodyBounds(preview.fixture.folder.getBounds()).getHeight());
        REQUIRE(commandBodyBounds(preview.fixture.normal.getBounds()).getY()==commandBodyBounds(preview.fixture.folder.getBounds()).getY());
    }
}
static void fieldFeedbackCases()
{
    using namespace m3::ui;
    REQUIRE(FieldPresentation::helperFont().getHeight()==FieldPresentation::helperHeight);
    REQUIRE(FieldPresentation::helperHeight<textStyle(TextRole::controlValue).height);
    REQUIRE(FieldPresentation::feedbackInk(true)==danger);
    for(bool hovered:{false,true})for(bool down:{false,true})for(bool focus:{false,true})
    {
        juce::Image image(juce::Image::ARGB,280,44,true,juce::SoftwareImageType{});juce::Graphics g(image);g.fillAll(background);
        SelectorPresentation::surface(g,{0,0,280,44},{true,hovered,down,focus,false},true);
        const auto body=commandBodyBounds({0,0,280,44});
        REQUIRE(image.getPixelAt(body.getX(),body.getCentreY())==danger);
        if(focus)REQUIRE(image.getPixelAt(1,1)==aqua);
    }
    std::cerr<<"Field border state matrix passed\n";
    WorkspaceTheme theme;juce::Component window;window.setLookAndFeel(&theme);window.setBounds(80,80,900,600);window.addToDesktop(0);window.setVisible(true);window.toFront(true);
    ChoiceSelector selector;selector.addItem("Accepted",1);selector.setSelectedId(1,juce::dontSendNotification);int changes=0;selector.onChange=[&]{++changes;};
    window.addAndMakeVisible(selector);selector.setBounds(30,30,280,44);auto* handler=selector.getAccessibilityHandler();REQUIRE(handler);
    selector.setValidationMessage("IR file not found");REQUIRE(handler->getDescription().contains("IR file not found"));
    selector.setValidationMessage({});REQUIRE(!handler->getDescription().contains("IR file not found") && changes==0 && selector.getSelectedId()==1);
    std::cerr<<"Field accessible metadata cleared\n";
    selectorReview::Panel preview;
    for(const auto size:{juce::Point<int>{900,600},{1000,700},{1100,850},{1400,1000}})for(const auto scale:{100,125})
    {
        preview.setSize(size.x,size.y);preview.setReviewScale(scale);
        const std::array<std::pair<ChoiceSelector*,selectorReview::Feedback*>,4> fields{{{&preview.fixture.normal,&preview.fixture.normalFeedback},{&preview.fixture.longName,&preview.fixture.longFeedback},{&preview.fixture.empty,&preview.fixture.emptyFeedback},{&preview.fixture.unavailable,&preview.fixture.unavailableFeedback}}};
        for(const auto& field:fields)
        {
            const auto body=commandBodyBounds(field.first->getBounds());
            REQUIRE(field.second->getX()==body.getX() && field.second->getY()==body.getBottom()+FieldPresentation::helperGap);
            REQUIRE(preview.fixture.getLocalBounds().contains(field.second->getBounds()));
            REQUIRE(field.second->isEnabled());
        }
    }
    std::cerr<<"Field layout matrix passed\n";
    REQUIRE(preview.fixture.normal.isEnabled() && preview.fixture.normalFeedback.isError());
    REQUIRE(!preview.fixture.emptyFeedback.isError() && !preview.fixture.unavailableFeedback.isError());
    preview.fixture.normal.setSelectedId(1,juce::sendNotificationSync);
    REQUIRE(!preview.fixture.normalFeedback.isError() && preview.fixture.normal.getValidationMessage().isEmpty());
    std::cerr<<"Field clearing passed\n";
    window.setLookAndFeel(nullptr);
}
static void wrappedFeedbackCases()
{
    using namespace m3::ui;
    const auto verify = [](const FieldPresentation::WrappedFeedback& feedback, juce::Rectangle<int> body)
    {
        const auto bounds = feedback.getBounds();
        const auto& layout = feedback.getLayout();
        REQUIRE(bounds.getX() == body.getX());
        REQUIRE(bounds.getWidth() == body.getWidth());
        REQUIRE(bounds.getY() == body.getBottom() + FieldPresentation::helperGap);
        REQUIRE(bounds.getHeight() >= layout.getHeight());
        for (const auto& line : layout)
        {
            REQUIRE(line.getLineBounds().getX() >= -0.01f);
            REQUIRE(line.getLineBounds().getRight() <= bounds.getWidth() + 0.01f);
            REQUIRE(line.getLineBounds().getBottom() <= bounds.getHeight() + 0.01f);
            for (const auto* run : line.runs)
                REQUIRE(run->font.getHeight() == FieldPresentation::helperHeight);
        }
    };

    FieldPresentation::WrappedFeedback sample;
    const juce::Rectangle<int> body{12, 40, 100, 34};
    sample.setMessage("Short help", false, body);
    verify(sample, body);
    REQUIRE(sample.getLayout().getNumLines() == 1);
    const auto unbroken = juce::String::repeatedString("LongFileName", 9);
    const juce::String words = "A longer field message should use all the lines that it needs to show its final instruction.";
    for (const auto& message : {words, unbroken})
    {
        sample.setMessage(message, true, body);
        verify(sample, body);
        REQUIRE(sample.getLayout().getNumLines() > 2);
        REQUIRE(sample.getLayout().getLine(sample.getLayout().getNumLines() - 1).stringRange.getEnd() == message.length());
    }
    sample.setMessage({}, false, body);
    REQUIRE(sample.getBounds().isEmpty() && sample.getLayout().getNumLines() == 0);

    using namespace fieldFeedbackReview;
    Panel panel;
    auto& preview = panel.fixture;
    for (const auto size : {juce::Point<int>{900, 600}, {1100, 850}})
        for (const int scale : {100, 125})
        {
            panel.setSize(size.x, size.y);
            panel.setScale(scale);
            const auto pair = commandBodyBounds(preview.start.getBounds()).getUnion(commandBodyBounds(preview.end.getBounds()));
            preview.setScene(Scene::normal);
            const auto normalActionY = preview.apply.getY();
            for (const auto scene : {Scene::longHelp, Scene::validation, Scene::unavailable})
            {
                preview.setScene(scene);
                verify(preview.singleFeedback, commandBodyBounds(preview.ir.getBounds()));
                verify(preview.pairFeedback, pair);
                REQUIRE(preview.pairFeedback.getBounds().getBottom() < preview.apply.getY());
                REQUIRE(preview.getLocalBounds().contains(preview.apply.getBounds()));
                REQUIRE(preview.apply.isEnabled() == (scene != Scene::unavailable));
                if (scene != Scene::unavailable)
                    REQUIRE(preview.apply.getY() > normalActionY);
            }
        }

    panel.setSize(900, 600);
    panel.setScale(100);
    panel.addToDesktop(0);
    panel.setVisible(true);
    panel.toFront(true);
    preview.setScene(Scene::validation);
    auto* endHandler = preview.end.getAccessibilityHandler();
    REQUIRE(endHandler && endHandler->getTitle() == "End excl.");
    REQUIRE(endHandler->getTextInterface() != nullptr);
    REQUIRE(endHandler->getDescription().contains("Enter a later bar"));
    REQUIRE(preview.start.getAccessibilityHandler()->getDescription() == endHandler->getDescription());
    REQUIRE(preview.ir.getAccessibilityHandler()->getDescription().contains("could not be read"));
    REQUIRE(preview.end.isInvalid() && !preview.start.isInvalid());

    juce::SharedResourcePointer<FocusVisibility> visibility;
    visibility->pointerInput();
    endHandler->grabFocus();
    pump();
    REQUIRE(preview.end.isFocusVisible() && preview.end.isInvalid());
    juce::Image painted(juce::Image::ARGB, preview.end.getWidth(), preview.end.getHeight(), true, juce::SoftwareImageType{});
    juce::Graphics graphics(painted);
    graphics.fillAll(background);
    preview.end.paintEntireComponent(graphics, false);
    const auto edge = commandBodyBounds(preview.end.getLocalBounds());
    REQUIRE(painted.getPixelAt(edge.getX(), edge.getCentreY()) == danger);
    REQUIRE(painted.getPixelAt(1, 1) == aqua);
    visibility->pointerInput();
    REQUIRE(!preview.end.isFocusVisible() && preview.end.isInvalid());
    preview.start.grabKeyboardFocus();
    visibility->pointerInput();
    preview.start.moveKeyboardFocusToSibling(true);
    pump();
    REQUIRE(preview.end.isFocusVisible());

    preview.end.setText("8", false);
    preview.end.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
    pump();
    REQUIRE(!preview.end.isInvalid() && !endHandler->getDescription().contains("Enter a later bar"));
    preview.ir.setSelectedId(2, juce::sendNotificationSync);
    REQUIRE(preview.ir.getValidationMessage().isEmpty());
    preview.setScene(Scene::unavailable);
    REQUIRE(endHandler->getDescription().contains("Unfreeze"));
    REQUIRE(preview.start.isEnabled() && preview.end.isEnabled() && !preview.apply.isEnabled());
    panel.removeFromDesktop();
}

static void numericTextFieldCases()
{
    using namespace numericTextFieldsReview;
    Panel panel;
    panel.setBounds(60, 60, 900, 600);
    panel.addToDesktop(0);
    panel.setVisible(true);
    panel.toFront(true);
    auto& fixture = panel.fixture;
    const auto key = [](juce::Component& control, int code)
    {
        REQUIRE(control.keyPressed(juce::KeyPress(code)));
        pump();
    };
    const auto checkFeedback = [](const Field& field)
    {
        const auto body = commandBodyBounds(field.editor.getBounds());
        REQUIRE(field.feedback.getX() == body.getX() && field.feedback.getWidth() == body.getWidth());
        REQUIRE(field.feedback.getY() == body.getBottom() + FieldPresentation::helperGap);
        REQUIRE(field.getLocalBounds().contains(field.feedback.getBounds()));
    };
    const auto checkLayout = [&](juce::Component& control)
    {
        REQUIRE(fixture.getLocalBounds().contains(control.getBounds()));
    };

    auto& timing = fixture.timing.editor;
    auto& level = fixture.level.editor;
    timing.grabKeyboardFocus();
    pump();
    REQUIRE(timing.isFocusVisible());
    juce::SharedResourcePointer<FocusVisibility> visibility;
    visibility->pointerInput();
    timing.setHighlightedRegion({0, 3});
    REQUIRE(timing.hasKeyboardFocus(false) && !timing.isFocusVisible());
    REQUIRE(timing.getHighlightedRegion() == juce::Range<int>(0, 3));
    auto* timingHandler = timing.getAccessibilityHandler();
    REQUIRE(timingHandler && timingHandler->getTitle() == "Source start (bars)");
    REQUIRE(timingHandler->getTextInterface() != nullptr);
    level.grabKeyboardFocus();
    visibility->pointerInput();
    timingHandler->grabFocus();
    pump();
    REQUIRE(timing.isFocusVisible());
    const auto normalTimingHeight = fixture.timing.feedback.getHeight();
    timing.selectAll();
    const auto selectedTiming = timing.getHighlightedRegion();
    timing.insertTextAtCaret("abc");
    REQUIRE(timing.getText() == "2.1.0" && timing.getHighlightedRegion() == selectedTiming);
    timing.insertTextAtCaret("b2.b5");
    REQUIRE(timing.getText() == "2.5");
    timing.moveKeyboardFocusToSibling(true);
    pump();
    REQUIRE(!timing.hasKeyboardFocus(false));
    REQUIRE(timing.isInvalid() && timing.getText() == "2.1.0");
    REQUIRE(timingHandler->getDescription().contains("choose exact samples"));
    REQUIRE(fixture.timing.feedback.getHeight() > normalTimingHeight);
    checkFeedback(fixture.timing);
    REQUIRE(fixture.level.getY() >= fixture.timing.getBottom());
    timing.setText("2.1.125", false);
    key(timing, juce::KeyPress::returnKey);
    REQUIRE(!timing.isInvalid() && timing.getText() == "2.1.125");
    timing.setText("8.2", false);
    key(timing, juce::KeyPress::escapeKey);
    REQUIRE(timing.getText() == "2.1.125");
    level.grabKeyboardFocus();
    level.setText("+13", false);
    key(level, juce::KeyPress::returnKey);
    REQUIRE(level.isInvalid() && level.getText() == "0.0");
    REQUIRE(level.getDescription().contains("-12 and +12 dB"));
    level.setText("+6.25", false);
    key(level, juce::KeyPress::returnKey);
    REQUIRE(!level.isInvalid() && level.getText() == "+6.25");

    fixture.setScene(Scene::compact);
    FieldNumericInput* compact = nullptr;
    for (auto* child : fixture.sensitivity.getChildren())
        if (auto* input = dynamic_cast<FieldNumericInput*>(child)) compact = input;
    REQUIRE(compact && compact->isEditable());
    auto* compactHandler = compact->getAccessibilityHandler();
    REQUIRE(compactHandler && compactHandler->getTitle() == "Event sensitivity (percent)");
    REQUIRE(compactHandler->getDescription() == fixture.sensitivityHelp);
    REQUIRE(compactHandler->getValueInterface() != nullptr);
    REQUIRE(compactHandler->getActions().invoke(juce::AccessibilityActionType::focus));
    pump();
    REQUIRE(compact->isFocusVisible());
    key(*compact, juce::KeyPress::returnKey);
    auto* editor = dynamic_cast<FieldTextEditor*>(compact->getCurrentTextEditor());
    REQUIRE(editor && editor->isFocusVisible() && editor->getHighlightedRegion().getLength() > 0);
    REQUIRE(editor->getRight() <= compact->incrementButton().getX());
    REQUIRE(editor->getAccessibilityHandler()->getDescription() == fixture.sensitivityHelp);
    editor->setText("50.5", false);
    editor->insertTextAtCaret("abc");
    REQUIRE(editor->getText() == "50.5");
    key(*editor, juce::KeyPress::returnKey);
    REQUIRE(compact->getCurrentTextEditor() == nullptr && fixture.sensitivity.getValue() == 50.5);
    key(compact->incrementButton(), juce::KeyPress::spaceKey);
    REQUIRE(std::abs(fixture.sensitivity.getValue() - 50.6) < 0.0001);
    REQUIRE(compact->decrementButton().getAccessibilityHandler()->getActions().invoke(juce::AccessibilityActionType::press));
    pump();
    REQUIRE(std::abs(fixture.sensitivity.getValue() - 50.5) < 0.0001);
    REQUIRE(compactHandler->getActions().invoke(juce::AccessibilityActionType::press));
    editor = dynamic_cast<FieldTextEditor*>(compact->getCurrentTextEditor());
    REQUIRE(editor && editor->isFocusVisible());
    editor->setText("18.5", false);
    key(*editor, juce::KeyPress::escapeKey);
    REQUIRE(compact->getCurrentTextEditor() == nullptr && fixture.sensitivity.getValue() == 50.5);
    compact->grabKeyboardFocus();
    pump();
    const auto pointer = juce::Point<float>{8, 10};
    const auto now = juce::Time::getCurrentTime();
    REQUIRE(compact->isEditableOnSingleClick());
    REQUIRE(compact->getLocalBounds().contains(pointer.toInt()));
    juce::MouseEvent click(juce::Desktop::getInstance().getMainMouseSource(), pointer, {},
                           0, 0, 0, 0, 0, compact, compact, now, pointer, now, 1, false);
    compact->mouseUp(click);
    editor = dynamic_cast<FieldTextEditor*>(compact->getCurrentTextEditor());
    REQUIRE(editor);
    REQUIRE(editor->hasKeyboardFocus(false));
    REQUIRE(!editor->isFocusVisible());
    key(*editor, juce::KeyPress::escapeKey);

    auto& standalone = fixture.standalone;
    key(standalone.incrementButton(), juce::KeyPress::spaceKey);
    REQUIRE(fixture.standaloneValue == 43 && fixture.sensitivity.getValue() == 50.5);
    key(standalone, juce::KeyPress::returnKey);
    editor = dynamic_cast<FieldTextEditor*>(standalone.getCurrentTextEditor());
    REQUIRE(editor);
    editor->setText("101", false);
    // JUCE delivers an outside click to its temporary editor's modal owner.
    static_cast<juce::Component&>(standalone).inputAttemptWhenModal();
    pump();
    REQUIRE(!standalone.isBeingEdited() && standalone.isInvalid() && standalone.getText() == "43");
    REQUIRE(standalone.getDescription().contains("0 to 100") && fixture.standaloneFeedback.getHeight() > 0);
    key(standalone, juce::KeyPress::returnKey);
    editor = dynamic_cast<FieldTextEditor*>(standalone.getCurrentTextEditor());
    REQUIRE(editor);
    editor->selectAll();
    editor->insertTextAtCaret("4a4");
    REQUIRE(editor->getText() == "44");
    key(*editor, juce::KeyPress::returnKey);
    REQUIRE(!standalone.isInvalid() && fixture.standaloneValue == 44);

    auto wheelOver = [stamp = now.toMilliseconds()](juce::Component& target, float delta, int modifiers = 0) mutable
    {
        const auto point = target.getLocalBounds().getCentre().toFloat();
        const juce::Time time(++stamp);
        const juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(), point,
                                     juce::ModifierKeys(modifiers), 0, 0, 0, 0, 0, &target, &target,
                                     time, point, time, 0, false);
        juce::MouseWheelDetails wheel{};
        wheel.deltaY = delta;
        target.mouseWheelMove(event, wheel);
    };
    wheelOver(standalone.incrementButton(), .12f);
    REQUIRE(fixture.standaloneValue == 45);
    wheelOver(standalone.decrementButton(), -.12f);
    REQUIRE(fixture.standaloneValue == 44);
    wheelOver(*compact, .12f);
    REQUIRE(std::abs(fixture.sensitivity.getValue() - 50.6) < 0.0001);
    wheelOver(compact->decrementButton(), -.12f);
    REQUIRE(std::abs(fixture.sensitivity.getValue() - 50.5) < 0.0001);

    auto& slider = fixture.sensitivity;
    slider.setValue(50, juce::dontSendNotification);
    wheelOver(slider, .12f);
    const auto normalWheel = slider.getValue() - 50.0;
    slider.setValue(50, juce::dontSendNotification);
    wheelOver(slider, .12f, juce::ModifierKeys::ctrlModifier);
    const auto fineWheel = slider.getValue() - 50.0;
    REQUIRE(fineWheel > 0 && fineWheel < normalWheel);
    wheelOver(slider, -.12f, juce::ModifierKeys::ctrlModifier);
    REQUIRE(std::abs(slider.getValue() - 50.0) < 0.0001);

    const auto drag = [&](int modifiers)
    {
        slider.setValue(50, juce::dontSendNotification);
        const auto source = juce::Desktop::getInstance().getMainMouseSource();
        const juce::Point<float> start{90, 17}, end{120, 17};
        const auto event = [&](juce::Point<float> at, int buttons, bool moved)
        {
            return juce::MouseEvent(source, at, juce::ModifierKeys(buttons), 0, 0, 0, 0, 0,
                                    &slider, &slider, now, start, now, 1, moved);
        };
        slider.mouseDown(event(start, juce::ModifierKeys::leftButtonModifier | modifiers, false));
        slider.mouseDrag(event(end, juce::ModifierKeys::leftButtonModifier | modifiers, true));
        slider.mouseUp(event(end, modifiers, true));
        return slider.getValue() - 50.0;
    };
    const auto normalDrag = drag(0);
    const auto ctrlDrag = drag(juce::ModifierKeys::ctrlModifier);
    REQUIRE(std::abs(ctrlDrag - normalDrag) < 0.0001);

    fixture.setScene(Scene::diagnostics);
    auto& exact = fixture.exact.editor;
    REQUIRE(exact.isReadOnly() && fixture.unavailable.editor.getDescription().contains("Unfreeze"));
    REQUIRE(!fixture.unavailable.editor.isEnabled() && !fixture.unavailable.editor.isInvalid());
    exact.grabKeyboardFocus();
    pump();
    REQUIRE(exact.isFocusVisible());
    exact.setHighlightedRegion({0, 5});
    exact.keyPressed(juce::KeyPress('7', {}, '7'));
    REQUIRE(exact.getText() == "96037");
    REQUIRE(exact.getHighlightedRegion() == juce::Range<int>(0, 5));
    checkFeedback(fixture.unavailable);

    fixture.setScene(Scene::bars);
    fixture.barEnd.editor.grabKeyboardFocus();
    fixture.barEnd.editor.setText("4", false);
    fixture.apply.grabKeyboardFocus();
    pump();
    REQUIRE(fixture.barEnd.editor.isInvalid() && !fixture.barStart.editor.isInvalid());
    REQUIRE(fixture.submittedRanges == 0);
    REQUIRE(fixture.barStart.editor.getDescription() == fixture.barEnd.editor.getDescription());
    REQUIRE(fixture.barFeedback.getHeight() > FieldPresentation::helperLineHeight);
    fixture.barEnd.editor.setText("8", false);
    key(fixture.barEnd.editor, juce::KeyPress::returnKey);
    REQUIRE(!fixture.barEnd.editor.isInvalid());
    REQUIRE(fixture.submittedRanges == 1);

    fixture.setScene(Scene::rename);
    fixture.name.editor.grabKeyboardFocus();
    fixture.name.editor.setText({}, false);
    fixture.cancel.grabKeyboardFocus();
    pump();
    REQUIRE(fixture.name.editor.isInvalid() && !fixture.name.editor.hasKeyboardFocus(false));
    fixture.name.editor.grabKeyboardFocus();
    key(fixture.name.editor, juce::KeyPress::returnKey);
    REQUIRE(fixture.name.isVisible() && fixture.name.editor.isInvalid() && fixture.acceptedName == "Room swell");
    REQUIRE(fixture.name.editor.getDescription().contains("Enter a name"));
    fixture.name.editor.setText("Hall swell", false);
    key(fixture.name.editor, juce::KeyPress::returnKey);
    REQUIRE(!fixture.name.isVisible() && fixture.acceptedName == "Hall swell");
    fixture.reopen.onClick();
    fixture.name.editor.setText("Uncommitted", false);
    key(fixture.name.editor, juce::KeyPress::escapeKey);
    REQUIRE(!fixture.name.isVisible() && fixture.acceptedName == "Hall swell");
    fixture.reopen.onClick();

    for (const auto size : {juce::Point<int>{900, 600}, {1100, 850}})
        for (const int scale : {100, 125})
        {
            panel.setSize(size.x, size.y);
            panel.setScale(scale);
            fixture.setScene(Scene::inspector);
            checkLayout(fixture.reset);
            REQUIRE(fixture.reset.getY() >= fixture.level.getBottom());
            fixture.setScene(Scene::compact);
            checkLayout(fixture.sensitivity);
            checkLayout(fixture.standaloneFeedback);
            REQUIRE(compact->getLocalBounds().contains(compact->incrementButton().getBounds()));
            fixture.setScene(Scene::bars);
            checkLayout(fixture.apply);
            REQUIRE(fixture.barFeedback.getBottom() < fixture.apply.getY());
            fixture.setScene(Scene::diagnostics);
            checkLayout(fixture.unavailable);
            fixture.setScene(Scene::rename);
            checkLayout(fixture.cancel);
            REQUIRE(fixture.name.getBottom() < fixture.cancel.getY());
        }
    panel.removeFromDesktop();
}

int main(int argc,char** argv){return runTest([&]
{
    juce::ScopedJuceInitialiser_GUI gui;
    if(argc>1 && juce::String(argv[1])=="--review-selectors"){selectorReview::run();return;}
    if(argc>1 && juce::String(argv[1])=="--review-action-menus"){actionMenuReview::run();return;}
    if(argc>1 && juce::String(argv[1])=="--review-field-feedback"){fieldFeedbackReview::run();return;}
    if(argc>1 && juce::String(argv[1])=="--test-field-feedback"){wrappedFeedbackCases();return;}
    if(argc>1 && juce::String(argv[1])=="--review-numeric-text-fields"){numericTextFieldsReview::run();return;}
    if(argc>1 && juce::String(argv[1])=="--test-numeric-text-fields"){numericTextFieldCases();return;}
    if(argc==3 && juce::String(argv[1])=="--capture-selectors")
    {
        selectorReview::Panel panel;panel.setSize(900,600);panel.addToDesktop(0);panel.setVisible(true);panel.toFront(true);
        const auto capture=[](juce::Component& component,const juce::File& path)
        {
            juce::Image image(juce::Image::ARGB,component.getWidth(),component.getHeight(),true,juce::SoftwareImageType{});juce::Graphics g(image);component.paintEntireComponent(g,true);REQUIRE(image.getPixelAt(0,0).getAlpha()==255);
            REQUIRE(path.getParentDirectory().createDirectory().wasOk());auto stream=path.createOutputStream();REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk() && juce::PNGImageFormat().writeImageToStream(image,*stream));
        };
        const juce::File path(argv[2]);capture(panel,path);panel.fixture.longName.showPopup();auto* popup=panel.fixture.longName.activePopup();REQUIRE(popup);popup->mouseMove(at(*popup,1));capture(*popup,path.getSiblingFile("selector-long-menu.png"));panel.fixture.longName.hidePopup();pump();return;
    }
    std::cerr<<"Selector interaction state\n";interactionCases();
    std::cerr<<"Native popup\n";popupCases();
    std::cerr<<"Action menu rows and preview geometry\n";actionMenuCases();
    std::cerr<<"Selector input and lifetime\n";selectorCases();
    std::cerr<<"Presentation and geometry\n";presentationCases();
    std::cerr<<"Field feedback\n";fieldFeedbackCases();
    std::cerr<<"Wrapped field feedback\n";wrappedFeedbackCases();
    std::cerr<<"Numeric and text field variants\n";numericTextFieldCases();
    std::cout<<"Selector popup state, native input, accessibility, focus, geometry and lifetime cases passed.\n";
});}
