#pragma once
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/IconActionButton.h>
#include <UI/Controls/LatchingActionButton.h>
#include <UI/Controls/FileDragTile.h>
#include <UI/Controls/PresentationSurface.h>
#include <UI/Controls/FocusViewport.h>
#include <UI/Controls/DeliberateSelector.h>

namespace commandButtonCases
{
using namespace m3::ui;
using namespace m3::ui::controls;
inline void pump(){juce::MessageManager::getInstance()->runDispatchLoopUntil(35);}
inline juce::Image render(juce::Button& button,bool over=false,bool down=false)
{
    juce::Image image(juce::Image::ARGB,button.getWidth(),button.getHeight(),true,juce::SoftwareImageType{});
    juce::Graphics g(image);g.fillAll(background);
    if(auto* action=dynamic_cast<ActionButton*>(&button))action->paintButton(g,over,down);
    else {auto* icon=dynamic_cast<IconActionButton*>(&button);REQUIRE(icon);icon->paintButton(g,over,down);}
    return image;
}
inline bool same(const juce::Image& a,const juce::Image& b)
{
    if(a.getBounds()!=b.getBounds())return false;
    for(int y=0;y<a.getHeight();++y)for(int x=0;x<a.getWidth();++x)if(a.getPixelAt(x,y)!=b.getPixelAt(x,y))return false;
    return true;
}
inline void cases()
{
    // Independent semantic oracles for all applicable combined states.
    for(const auto variant:{CommandVariant::neutral,CommandVariant::primary,CommandVariant::destructive})
    for(bool enabled:{false,true})for(bool over:{false,true})for(bool down:{false,true})for(bool focus:{false,true})for(bool latched:{false,true})
    {
        const auto ink=commandInk(variant,{enabled,over,down,focus,latched});
        const auto alpha=enabled?1.f:.35f;const bool emphasized=variant==CommandVariant::primary || latched;
        const bool held=enabled && down, hovered=enabled && over;
        const bool dangerRole=variant==CommandVariant::destructive;
        REQUIRE(ink.content==(emphasized || (dangerRole && held)?background:dangerRole?danger:text).withMultipliedAlpha(alpha));
        REQUIRE(ink.stroke==(held || (dangerRole && hovered)?controlGeometry::focusStroke:rule));
        REQUIRE(ink.focus==aqua);
        if(!enabled){const auto idle=commandInk(variant,{false,false,false,focus,latched});REQUIRE(ink.fill==idle.fill && ink.border==idle.border && ink.glyph==idle.glyph);}
        if(held)REQUIRE(ink.border==(dangerRole?danger:aqua));
        else if(hovered)REQUIRE(ink.border==(dangerRole?danger:hover));
        else REQUIRE(ink.border==(dangerRole?danger:outline).withMultipliedAlpha(alpha));
        if(enabled && !emphasized && !dangerRole && held)REQUIRE(ink.fill==separator && ink.glyph==text);
        if(enabled && !emphasized && dangerRole && held)REQUIRE(ink.fill==danger && ink.content==background);
        if(enabled && !emphasized && !dangerRole)REQUIRE(ink.glyph==text);
        for(const auto bounds:{juce::Rectangle<int>{0,0,24,32},juce::Rectangle<int>{0,0,150,34}})
        {
            REQUIRE(bounds.toFloat().contains(controlGeometry::insideStroke(bounds,ink.stroke).expanded(ink.stroke*.5f)));
            juce::Image canvas(juce::Image::ARGB,bounds.getWidth()+8,bounds.getHeight()+8,true,juce::SoftwareImageType{});
            juce::Graphics g(canvas);paintCommandSurface(g,bounds.translated(4,4),variant,{enabled,over,down,focus,latched});
            for(int x=0;x<canvas.getWidth();++x)REQUIRE(canvas.getPixelAt(x,0).getAlpha()==0 && canvas.getPixelAt(x,canvas.getHeight()-1).getAlpha()==0);
            if(focus)
            {
                const auto frame=bounds.translated(4,4),body=commandBodyBounds(frame);
                REQUIRE(body.getX()-frame.getX()-controlGeometry::focusStroke==controlGeometry::focusOutset);
                REQUIRE(canvas.getPixelAt(frame.getX()+1,frame.getY()+4)==aqua);
                REQUIRE(canvas.getPixelAt(frame.getX()+3,frame.getY()+4).getAlpha()==0);
                REQUIRE(canvas.getPixelAt(frame.getCentreX(),frame.getY()+1).getAlpha()==0);
                juce::Image noFocus(juce::Image::ARGB,canvas.getWidth(),canvas.getHeight(),true,juce::SoftwareImageType{});
                juce::Graphics other(noFocus);paintCommandSurface(other,frame,variant,{enabled,over,down,false,latched});
                for(int y=body.getY();y<body.getBottom();++y)for(int x=body.getX();x<body.getRight();++x)
                    REQUIRE(canvas.getPixelAt(x,y)==noFocus.getPixelAt(x,y));
            }
        }
    }
    ActionButton ordinary("Command"),primary("Command",CommandIcon::none,true),dangerous("Reset",CommandIcon::none,CommandVariant::destructive);
    ActionButton add("Add swell",CommandIcon::add),play("Audition",CommandIcon::play,true);
    IconActionButton settings(ActionIcon::settings),folder(ActionIcon::openFile);
    LatchingActionButton latch("Wet only");
    for(auto* button:std::array<juce::Button*,8>{&ordinary,&primary,&dangerous,&add,&play,&settings,&folder,&latch})button->setSize(150,34);
    WorkspaceTheme theme;juce::Component host;host.setLookAndFeel(&theme);host.setSize(900,300);host.setWantsKeyboardFocus(true);
    // Native Rename dialog buttons keep a neutral fallback independent of metadata.
    for(int height:{28,34})for(const auto* label:{"OK","Cancel"})
    {
        juce::TextButton fallback(label);fallback.setSize(100,height);
        const auto paint=[&]{juce::Image frame(juce::Image::ARGB,100,height,true,juce::SoftwareImageType{});juce::Graphics g(frame);g.fillAll(background);
            theme.drawButtonBackground(g,fallback,background,true,true);theme.drawButtonText(g,fallback,true,true);return frame;};
        // JUCE's actual AlertWindow button is 28 high. Keep full-size type there.
        const auto fallbackFont=theme.getTextButtonFont(fallback,height);
        REQUIRE(fallbackFont.getHeight()<=height-2*spacing::tight);
        const auto expected=paint();
        for(const auto* name:{"audition","stopAudition","resetWetShape","resetSwellMenu","loadIR"})
        {fallback.setName(name);REQUIRE(same(expected,paint()));}
        unsigned calls=0;fallback.onClick=[&]{++calls;};fallback.triggerClick();pump();REQUIRE(calls==1);
    }
    int position=0;
    for(auto* button:std::array<juce::Button*,8>{&ordinary,&primary,&dangerous,&add,&play,&settings,&folder,&latch})
    {host.addAndMakeVisible(button);button->setBounds(8+(position%5)*165,20+(position/5)*50,150,34);++position;}
    host.addToDesktop(0);host.setVisible(true);host.toFront(true);REQUIRE(host.getPeer());host.getPeer()->grabFocus();host.grabKeyboardFocus();pump();
    for(bool enabled:{false,true})for(bool over:{false,true})for(bool down:{false,true})
    {
        ordinary.setEnabled(enabled);settings.setEnabled(enabled);folder.setEnabled(enabled);
        const auto expected=render(ordinary,over,down);
        for(auto* icon:{&settings,&folder})
        {
            const auto actual=render(*icon,over,down);
            // Surface parity at all edges, away from distinct content glyphs.
            for(int y=0;y<34;++y)for(int x=0;x<150;++x)if(x<12 || x>=138 || y<3 || y>=31)REQUIRE(actual.getPixelAt(x,y)==expected.getPixelAt(x,y));
        }
    }
    ordinary.setEnabled(true);settings.setEnabled(true);folder.setEnabled(true);
    // All enabled roles visibly distinguish hover from held press, even while focused.
    for(const auto variant:{CommandVariant::neutral,CommandVariant::primary,CommandVariant::destructive})
    for(bool focused:{false,true})for(bool latched:{false,true})
    {
        const auto hovered=commandInk(variant,{true,true,false,focused,latched});
        const auto held=commandInk(variant,{true,true,true,focused,latched});
        REQUIRE(hovered.fill!=held.fill || hovered.stroke!=held.stroke);
        const auto focusOnly=commandInk(variant,{true,false,false,focused,latched});
        REQUIRE(hovered.border!=focusOnly.border || hovered.stroke!=focusOnly.stroke);
    }
    for(auto* button:std::array<juce::Button*,8>{&ordinary,&primary,&dangerous,&add,&play,&settings,&folder,&latch})
    {
        const auto before=render(*button,true,true);
        for(const auto* name:{"audition","stopAudition","resetWetShape","resetSwellMenu","loadIR","unrelated"})
        {button->setName(name);REQUIRE(same(before,render(*button,true,true)));}
    }
    // Relabeling changes content only, not role/glyph or state surface.
    const auto addBefore=render(add);add.setButtonText("Reset shape");REQUIRE(same(addBefore,render(add)));
    const auto dangerBefore=render(dangerous);dangerous.setButtonText("Audition");const auto dangerAfter=render(dangerous);
    REQUIRE(dangerous.getAccessibilityHandler()->getTitle()=="Audition");
    REQUIRE(add.getAccessibilityHandler()->getTitle()=="Add swell");
    for(int x=0;x<150;++x)REQUIRE(dangerBefore.getPixelAt(x,1)==dangerAfter.getPixelAt(x,1));
    for(int width:{24,34,40,50}){add.setSize(width,32);play.setSize(width,32);REQUIRE(render(add).getPixelAt(width/2,16)!=background);const auto frame=render(play);REQUIRE(frame.getPixelAt(width/2,16)==background && frame.getPixelAt(width/2,7)==aqua);}
    latch.setSize(150,34);unsigned clicks=0,begins=0;
    latch.beforeInvocation=[&]{++begins;};latch.onClick=[&]{++clicks;};
    latch.display(false);const auto off=render(latch);latch.display(true);REQUIRE(!same(off,render(latch)) && clicks==0 && begins==0);
    auto* accessible=latch.getAccessibilityHandler();REQUIRE(accessible && accessible->getCurrentState().isCheckable() && accessible->getCurrentState().isChecked());
    REQUIRE(accessible->getValueInterface() && accessible->getValueInterface()->getCurrentValueAsString()=="On" && accessible->getValueInterface()->isReadOnly());
    REQUIRE(accessible->getActions().invoke(juce::AccessibilityActionType::toggle));pump();REQUIRE(clicks==1 && begins==1 && !latch.getToggleState());
    REQUIRE(accessible->getActions().invoke(juce::AccessibilityActionType::press));pump();REQUIRE(clicks==2 && begins==2 && latch.getToggleState());
    latch.setEnabled(false);accessible->getActions().invoke(juce::AccessibilityActionType::toggle);pump();REQUIRE(clicks==2 && begins==2 && latch.getToggleState());

    // Real callback dispatch, not just keyPressed return values.
    for(auto* command:std::array<CommandButton*,2>{&ordinary,&folder})
    {
        unsigned requests=0,captures=0;int epoch=1,pinned=0,accepted=0;
        command->beforeInvocation=[&]{++captures;pinned=epoch;};
        command->onClick=[&]{++requests;if(pinned==epoch)++accepted;};
        controlReview::pointerClick(*command);REQUIRE(requests==1 && captures==1 && accepted==1);
        command->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();REQUIRE(requests==2 && captures==2 && accepted==2);
        command->keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));pump();REQUIRE(requests==3 && captures==3 && accepted==3);
        auto* handler=command->getAccessibilityHandler();REQUIRE(handler && handler->getRole()==juce::AccessibilityRole::button && handler->getTitle().isNotEmpty());
        handler->getActions().invoke(juce::AccessibilityActionType::press);++epoch;pump();REQUIRE(requests==4 && captures==4 && accepted==3);
        command->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));command->setEnabled(false);pump();REQUIRE(requests==4 && captures==5);
        controlReview::pointerClick(*command);command->keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));handler->getActions().invoke(juce::AccessibilityActionType::press);pump();REQUIRE(requests==4 && captures==5);
        command->setEnabled(true);command->beforeInvocation={};command->onClick={};
    }
    unsigned destroyedClicks=0;
    {auto button=std::make_unique<ActionButton>("Queued");button->onClick=[&]{++destroyedClicks;};button->invokeCommand();button.reset();}pump();REQUIRE(destroyedClicks==0);
    {auto button=std::make_unique<ActionButton>("Destroy during capture");button->beforeInvocation=[&]{button.reset();};button->invokeCommand();REQUIRE(!button);}pump();
    {auto button=std::make_unique<ActionButton>("Destroy on click");button->onClick=[&]{button.reset();++destroyedClicks;};button->invokeCommand();pump();REQUIRE(!button && destroyedClicks==1);}

    FileDragTile tile;unsigned saves=0;tile.onSave=[&]{++saves;};tile.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();REQUIRE(saves==1);
    tile.setSize(280,230);REQUIRE(render(tile).isValid());

    // Real component focus is distinct from the resolver's combined-state cases.
    int x=8;
    for(auto* button:std::array<juce::Button*,5>{&ordinary,&primary,&dangerous,&settings,&latch})
    {button->setEnabled(true);host.addAndMakeVisible(button);button->setBounds(x,20,150,34);x+=165;}
    for(auto* button:std::array<juce::Button*,5>{&ordinary,&primary,&dangerous,&settings,&latch})
    {button->grabKeyboardFocus();pump();REQUIRE(button->hasKeyboardFocus(true));const auto frame=render(*button);REQUIRE(frame.getPixelAt(1,5)==aqua && frame.getPixelAt(75,1)==background);}
    // Focus ownership and its decoration are independent across input methods.
    for(auto* command:std::array<CommandButton*,2>{&ordinary,&settings})
    {
        unsigned delivered=0,captured=0;
        command->onClick=[&]{++delivered;};command->beforeInvocation=[&]{++captured;};
        auto* handler=command->getAccessibilityHandler();REQUIRE(handler);
        REQUIRE(handler->getActions().invoke(juce::AccessibilityActionType::focus));pump();
        REQUIRE(command->hasKeyboardFocus(true) && command->isFocusVisible() && delivered==0 && captured==0);
        const auto keyboardFrame=render(*command,true,true);
        controlReview::pointerClick(*command);pump();
        REQUIRE(command->hasKeyboardFocus(true) && !command->isFocusVisible() && delivered==1 && captured==1);
        const auto pointerFrame=render(*command,true,true);
        REQUIRE(pointerFrame.getPixelAt(1,5)==background && keyboardFrame.getPixelAt(1,5)==aqua);
        const auto body=commandBodyBounds(command->getLocalBounds());
        for(int y=body.getY();y<body.getBottom();++y)for(int x=body.getX();x<body.getRight();++x)
            REQUIRE(pointerFrame.getPixelAt(x,y)==keyboardFrame.getPixelAt(x,y));
        controlReview::pointerClick(*command);pump();
        REQUIRE(!command->isFocusVisible() && delivered==2 && captured==2);
        primary.grabKeyboardFocus();command->grabKeyboardFocus();pump();
        REQUIRE(command->hasKeyboardFocus(true) && !command->isFocusVisible()); // Pointer dialog-style restoration.
        command->moveKeyboardFocusToSibling(true);pump();
        auto* next=dynamic_cast<CommandButton*>(juce::Component::getCurrentlyFocusedComponent());
        REQUIRE(next && next!=command && next->isFocusVisible());
        next->moveKeyboardFocusToSibling(false);pump();REQUIRE(command->hasKeyboardFocus(true) && command->isFocusVisible());
        REQUIRE(command->keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));pump();
        REQUIRE(command->isFocusVisible() && delivered==3 && captured==3);
        REQUIRE(command->keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)));pump();
        REQUIRE(command->isFocusVisible() && delivered==4 && captured==4);
        controlReview::pointerClick(*command);pump();REQUIRE(!command->isFocusVisible());
        REQUIRE(handler->getActions().invoke(juce::AccessibilityActionType::focus));pump();
        REQUIRE(command->isFocusVisible() && delivered==5 && captured==5);
        REQUIRE(handler->getActions().invoke(juce::AccessibilityActionType::press));pump();
        REQUIRE(command->isFocusVisible() && delivered==6 && captured==6);
        command->onClick={};command->beforeInvocation={};
    }
    // Observing keyboard input in a different family must not consume its edit.
    {
        controlReview::pointerClick(ordinary);pump();REQUIRE(!ordinary.isFocusVisible());
        juce::TextEditor entry;host.addAndMakeVisible(entry);entry.setBounds(8,120,180,30);entry.grabKeyboardFocus();pump();
        REQUIRE(host.getPeer()->handleKeyPress(juce::KeyPress('q',{},'q')));pump();REQUIRE(entry.getText()=="q");
        ordinary.grabKeyboardFocus();pump();REQUIRE(ordinary.isFocusVisible());
    }
    ActionButton longLabel("Recapture and regenerate selected swell");longLabel.setSize(160,34);host.addAndMakeVisible(longLabel);
    REQUIRE(longLabel.getAccessibilityHandler()->getTitle()==longLabel.getButtonText());
    const auto labelFont=componentTextFont(longLabel,TextRole::controlLabel);
    REQUIRE(juce::GlyphArrangement::getStringWidth(labelFont,longLabel.getButtonText())>longLabel.getWidth());
    REQUIRE(render(longLabel).isValid());host.setLookAndFeel(nullptr);host.removeFromDesktop();
}

class Matrix final:public juce::Component
{
public:
    Matrix()
    {
        const juce::StringArray names{"Neutral text","Primary","Destructive","Add glyph","Play glyph","IR icon (reference)","Latch off","Latch on"};
        const juce::StringArray columns{"Interactive","Hover","Pressed","Tab focus","Disabled"};
        for(int i=0;i<5;++i){headings[i].setText(columns[i],juce::dontSendNotification);applyTextRole(headings[i],TextRole::controlLabel);addAndMakeVisible(headings[i]);}
        for(int r=0;r<8;++r)
        {
            labels[r].setText(names[r],juce::dontSendNotification);applyTextRole(labels[r],TextRole::controlLabel);addAndMakeVisible(labels[r]);
            for(int c=0;c<5;++c)
            {
                std::unique_ptr<juce::Button> button;
                if(r==0)button=std::make_unique<ActionButton>("Command");
                if(r==1)button=std::make_unique<ActionButton>("Generate",CommandIcon::none,true);
                if(r==2)button=std::make_unique<ActionButton>("Reset All",CommandIcon::none,CommandVariant::destructive);
                if(r==3)button=std::make_unique<ActionButton>("Add swell",CommandIcon::add);
                if(r==4)button=std::make_unique<ActionButton>("Audition",CommandIcon::play,true);
                if(r==5)button=std::make_unique<IconActionButton>(ActionIcon::openFile);
                if(r>=6){auto latch=std::make_unique<LatchingActionButton>("Wet only");latch->display(r==7);button=std::move(latch);}
                button->onClick=[this,name=names[r]]{feedback.setText(name+" activated - fixture only; Tab and Return/Space are available.",juce::dontSendNotification);};
                button->setEnabled(c!=4);if(c==1)button->setState(juce::Button::buttonOver);if(c==2)button->setState(juce::Button::buttonDown);
                addAndMakeVisible(*button);buttons[r*5+c]=std::move(button);
            }
        }
        applyTextRole(feedback,TextRole::helper);feedback.setText("Candidate family only. Tab focus is real; shaded states are specimens. No project/audio changes.",juce::dontSendNotification);addAndMakeVisible(feedback);
    }
    void paint(juce::Graphics& g)override{g.fillAll(background);}
    void resized()override
    {
        const int inset=16,labelWidth=160,gap=12,width=(getWidth()-2*inset-labelWidth-4*gap)/5;
        for(int c=0;c<5;++c)headings[c].setBounds(inset+labelWidth+c*(width+gap),12,width,30);
        const auto frameHeight=controlGeometry::height+2*static_cast<int>(controlGeometry::focusOutset+controlGeometry::focusStroke);
        for(int r=0;r<8;++r){labels[r].setBounds(inset,56+r*52,labelWidth-8,frameHeight);for(int c=0;c<5;++c){auto bounds=juce::Rectangle<int>(inset+labelWidth+c*(width+gap),56+r*52,width,frameHeight);if(r==3 || r==4 || r==5)bounds=bounds.withWidth(frameHeight);buttons[r*5+c]->setBounds(bounds);}}
        feedback.setBounds(inset,486,getWidth()-2*inset,40);
    }
private:
    std::array<std::unique_ptr<juce::Button>,40> buttons;
    std::array<juce::Label,8> labels;std::array<juce::Label,5> headings;juce::Label feedback;
};
class Panel final:public juce::Component
{
public:
    Panel()
    {
        viewport.setFocusRevealEnabled(true);viewport.setViewedComponent(&surface,false);surface.contentRoot().addAndMakeVisible(matrix);
        addAndMakeVisible(viewport);addAndMakeVisible(sizes);addAndMakeVisible(scales);addAndMakeVisible(styles);addAndMakeVisible(title);
        title.setText("#107 button family - native review candidate",juce::dontSendNotification);applyTextRole(title,TextRole::sectionHeading);
        sizes.addItem("900 x 600",1);sizes.addItem("1000 x 700",2);sizes.addItem("1100 x 850",3);sizes.addItem("1400 x 1000",4);sizes.setSelectedId(1,juce::dontSendNotification);sizes.setTitle("Logical review size");
        scales.addItem("100% UI scale",1);scales.addItem("125% UI scale",2);scales.setSelectedId(1,juce::dontSendNotification);scales.setTitle("User magnification");
        styles.addItem("Main responsive text",1);styles.addItem("Auxiliary comfortable text",2);styles.setSelectedId(1,juce::dontSendNotification);styles.setTitle("Text presentation policy");
        sizes.onChange=scales.onChange=styles.onChange=[this]{present();};setSize(900,600);present();
    }
    std::function<void(int,int)> onDimensions;
    std::function<void(int,int,bool)> onStyle;
    void present()
    {
        const std::array<juce::Point<int>,4> presets{{{900,536},{1000,636},{1100,786},{1400,936}}};
        const auto size=presets[static_cast<size_t>(sizes.getSelectedId()-1)];const auto percent=scales.getSelectedId()==2?125:100;
        if(onStyle)onStyle(size.x,size.y+64,styles.getSelectedId()==2);
        surface.present(size.x,size.y,percent);matrix.setBounds(0,0,size.x,size.y);surface.setSize(size.x*percent/100,size.y*percent/100);
        if(onDimensions)onDimensions(surface.getWidth(),surface.getHeight()+64);resized();
    }
    void paint(juce::Graphics& g)override{g.fillAll(background);}
    void resized()override{title.setBounds(16,6,getWidth()-32,24);sizes.setBounds(16,34,180,26);scales.setBounds(208,34,180,26);styles.setBounds(400,34,230,26);viewport.setBounds(getLocalBounds().withTrimmedTop(64));}
private:
    Matrix matrix;PresentationSurface surface;FocusViewport viewport;DeliberateSelector sizes,scales,styles;juce::Label title;
};
inline void run()
{
    class Window final:public juce::DocumentWindow
    {
    public:
        Window():DocumentWindow("ReverbArise - #107 button family review",background,closeButton)
        {
            setUsingNativeTitleBar(true);setResizable(true,false);setLookAndFeel(&theme);
            auto body=std::make_unique<Panel>();auto* panel=body.get();setContentOwned(body.release(),true);
            panel->onStyle=[this](int w,int h,bool auxiliary){theme.setResponsiveViewport(w,h);setLookAndFeel(auxiliary?&comfortableTheme:&theme);};
            panel->onDimensions=[this](int w,int h){const auto* display=juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();const auto available=display?display->userBounds.toNearestInt():juce::Rectangle<int>(0,0,w,h+80);setContentComponentSize(std::min(w,available.getWidth()-40),std::min(h,available.getHeight()-80));};
            panel->present();centreWithSize(getWidth(),getHeight());setVisible(true);
        }
        ~Window()override{clearContentComponent();setLookAndFeel(nullptr);}
        void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
    private:WorkspaceTheme theme,comfortableTheme;
    } window;
    juce::MessageManager::getInstance()->runDispatchLoop();
}
}
