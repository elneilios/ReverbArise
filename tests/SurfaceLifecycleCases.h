#pragma once
#include <UI/Controls/OwnedAlertWindow.h>
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/FieldTextEditor.h>

inline void surfaceLifecycleCases(const std::function<void()>& pump,bool requireNativeFocus=false)
{
    using namespace m3::ui;using namespace m3::ui::controls;
    WorkspaceTheme theme;juce::Component content;
    ActionButton opener{"Open"},fallback{"Fallback"},other{"Other"};
    FieldTextEditor field{"Draft"};
    content.setLookAndFeel(&theme);content.setSize(600,300);
    int x=20;for(auto* child:std::initializer_list<juce::Component*>{&opener,&fallback,&other,&field})
    {content.addAndMakeVisible(child);child->setBounds(x,30,120,36);x+=140;}
    struct Window:juce::DocumentWindow
    {Window():DocumentWindow("ReverbArise - Surface lifecycle native validation",background,closeButton){}void closeButtonPressed()override{}} window;
    window.setUsingNativeTitleBar(true);window.setContentNonOwned(&content,true);window.centreWithSize(620,360);window.setVisible(true);window.toFront(true);
    pump();
    if(requireNativeFocus)REQUIRE(juce::Process::isForegroundProcess() && window.getPeer()->isFocused());
    juce::SharedResourcePointer<FocusVisibility> input;
    SurfaceInvocation invocation;
    input->keyboardInput();opener.grabKeyboardFocus();
    opener.moveKeyboardFocusToSibling(true);REQUIRE(fallback.hasKeyboardFocus(false));
    fallback.moveKeyboardFocusToSibling(false);REQUIRE(opener.hasKeyboardFocus(false));
    int activations=0;opener.onClick=[&]{++activations;};
    REQUIRE(opener.keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));pump();REQUIRE(activations==1);
    auto first=invocation.begin(opener,&fallback);REQUIRE(first && invocation.pending());
    REQUIRE(!invocation.begin(opener,&fallback));
    other.grabKeyboardFocus();REQUIRE(first->complete() && !first->complete());pump();
    // Native foreground ownership is a separate condition from component focus.
    if(juce::Process::isForegroundProcess())REQUIRE(opener.hasKeyboardFocus(false) && opener.isFocusVisible());
    input->pointerInput();auto pointer=invocation.begin(opener,&fallback);REQUIRE(pointer);
    opener.setEnabled(false);REQUIRE(pointer->complete());pump();
    if(juce::Process::isForegroundProcess())REQUIRE(fallback.hasKeyboardFocus(false) && !fallback.isFocusVisible());
    opener.setEnabled(true);
    auto deleted=std::make_unique<ActionButton>("Disposable");content.addAndMakeVisible(*deleted);deleted->setBounds(20,100,120,36);
    auto gone=invocation.begin(*deleted,&fallback);deleted.reset();REQUIRE(gone && gone->complete());pump();
    if(juce::Process::isForegroundProcess())REQUIRE(fallback.hasKeyboardFocus(false));
    {SurfaceInvocation temporary;auto ticket=temporary.begin(opener,&fallback);REQUIRE(ticket);first=ticket;}
    REQUIRE(!first->complete());
    auto invalidated=invocation.begin(opener,&fallback);REQUIRE(invalidated);invocation.invalidate();
    REQUIRE(!invalidated->complete() && !invocation.pending());
    auto abandoned=invocation.begin(opener,&fallback);REQUIRE(abandoned && abandoned->complete());
    auto replacement=invocation.begin(other,&fallback);fallback.grabKeyboardFocus();pump();
    REQUIRE(fallback.hasKeyboardFocus(false));REQUIRE(replacement->complete());pump();
    fallback.grabKeyboardFocus();SurfaceFocusReturn::capture(opener,&fallback).post([]{return false;});pump();REQUIRE(fallback.hasKeyboardFocus(false));
    int closes=0,cancels=0;
    auto returnFocus=SurfaceFocusReturn::capture(opener,&fallback);
    SurfaceEscape escape(content,[&]{
        ++closes;
        returnFocus.useCurrentInput();
        returnFocus.post();
    });
    field.onEscapeKey=[&]{++cancels;};field.grabKeyboardFocus();pump();
    input->pointerInput();
    REQUIRE(!field.isFocusVisible());
    REQUIRE(!escape.keyPressed(juce::KeyPress(juce::KeyPress::returnKey),&field));
    REQUIRE(escape.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey),&field));
    REQUIRE(cancels==1 && closes==1 && returnFocus.keyboard);
    pump();
    if(requireNativeFocus || juce::Process::isForegroundProcess())
        REQUIRE(opener.hasKeyboardFocus(false) && opener.isFocusVisible());
    OwnedAlertWindow alert("Sample dialog","No operation is performed.",juce::MessageBoxIconType::NoIcon,&content);
    REQUIRE(alert.findColour(juce::AlertWindow::backgroundColourId)==background);
    REQUIRE(alert.findColour(juce::AlertWindow::textColourId)==text);
    REQUIRE(alert.getLookAndFeel().getAlertWindowTitleFont()==textFont(TextRole::sectionHeading));
    REQUIRE(alert.getLookAndFeel().getAlertWindowMessageFont()==textFont(TextRole::helper));
    alert.showNative(content);
    int completions=0;alert.enterModalState(true,juce::ModalCallbackFunction::create([&](int result){REQUIRE(result==0);++completions;}),false);
    REQUIRE(alert.isUsingNativeTitleBar() && alert.isCurrentlyModal());
    alert.userTriedToCloseWindow();alert.finish(1);pump();REQUIRE(completions==1 && !alert.isShowing());
    alert.setLookAndFeel(nullptr);
    window.setContentNonOwned(nullptr,false);content.setLookAndFeel(nullptr);
}
