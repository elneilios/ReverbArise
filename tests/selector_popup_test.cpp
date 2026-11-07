#include "TestSupport.h"
#include <UI/Controls/PopupInteraction.h>
#include <UI/Controls/ChoicePopup.h>
#include <UI/Controls/ChoiceSelector.h>
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
int main(int argc,char** argv){return runTest([&]
{
    juce::ScopedJuceInitialiser_GUI gui;
    if(argc>1 && juce::String(argv[1])=="--review-selectors"){selectorReview::run();return;}
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
    std::cerr<<"Selector input and lifetime\n";selectorCases();
    std::cerr<<"Presentation and geometry\n";presentationCases();
    std::cerr<<"Field feedback\n";fieldFeedbackCases();
    std::cout<<"Selector popup state, native input, accessibility, focus, geometry and lifetime cases passed.\n";
});}
