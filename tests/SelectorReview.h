#pragma once
#include <UI/Controls/ChoiceSelector.h>
#include <UI/Controls/IconActionButton.h>
#include <UI/Controls/FocusViewport.h>
#include <UI/Controls/EllipsisLabel.h>

namespace selectorReview
{
using namespace m3::ui;
using namespace m3::ui::controls;
class Fixture final:public juce::Component
{
public:
    Fixture()
    {
        for(auto* control:std::array<juce::Component*,9>{&normal,&longName,&empty,&unavailable,&folder,&normalLabel,&longLabel,&emptyLabel,&unavailableLabel})addAndMakeVisible(control);
        for(auto* label:{&normalLabel,&longLabel,&emptyLabel,&unavailableLabel})applyTextRole(*label,TextRole::controlLabel);
        normalLabel.setText("Default IR",juce::dontSendNotification);longLabel.setText("Long labels / unavailable choices",juce::dontSendNotification);
        emptyLabel.setText("Empty catalogue",juce::dontSendNotification);unavailableLabel.setText("No selected swell",juce::dontSendNotification);
        normal.setTitle("Default IR");longName.setTitle("Long IR choice");empty.setTitle("Empty catalogue");unavailable.setTitle("Selected swell IR");
        longName.addItem("Imported - Cathedral North Balcony, stereo capture, long decay - extended room response",1);longName.addItem("Preverb Hall",2);longName.addItem("Imported - Studio Room",3);
        longName.setItemEnabled(3,false);longName.setChoiceReason(3,"Source file is missing");longName.setSelectedId(1,juce::dontSendNotification);
        longName.setTooltip("Long values retain their full accessible name; the menu wraps without shrinking.");
        empty.setTextWhenNothingSelected("No choices available");empty.setEnabled(false);empty.setDescription("No choices are available in this preview scene");
        unavailable.setTextWhenNothingSelected("No swell selected");unavailable.setEnabled(false);unavailable.setDescription("Select a swell to change its sound");
        normal.onChange=[this]{message="Accepted Default IR: "+normal.getText();repaint();};
        longName.onChange=[this]{message="Accepted long-name choice: "+longName.getText();repaint();};
        folder.onClick=[this]{message="Folder action alignment reference; no file operation in this preview";repaint();};
        addMouseListener(this,true);setScene(1);
    }
    ~Fixture()override{removeMouseListener(this);}
    void setScene(int scene)
    {
        normal.hidePopup();longName.hidePopup();normal.clear(juce::dontSendNotification);
        normal.addItem("Preverb Hall",1);normal.addItem("Short Plate",2);normal.addItem("Wide Chamber",3);normal.setSelectedId(1,juce::dontSendNotification);normal.setEnabled(scene!=3 && scene!=4);
        if(scene==2){normal.clear(juce::dontSendNotification);for(int i=0;i<40;++i)normal.addItem("Imported Room "+juce::String(i+1)+" - long catalogue choice",i+1);normal.setSelectedId(1,juce::dontSendNotification);}
        if(scene==3){normal.clear(juce::dontSendNotification);normal.setTextWhenNothingSelected("No choices available");}
        if(scene==4){normal.clear(juce::dontSendNotification);normal.addItem("Missing imported IR",1);normal.setSelectedId(1,juce::dontSendNotification);}
        normal.setDescription(scene==3?"No choices are available":scene==4?"Imported source is unavailable in this demonstration":"Choose an impulse response");
        normal.setTooltip(normal.getDescription());sceneNumber=scene;message="Keyboard: Tab, Enter, arrows, Escape. Pointer: hover, hold and leave.";repaint();
    }
    void resized()override
    {
        const auto inset=spacing::section;const auto width=(getWidth()-3*inset)/2;
        normalLabel.setBounds(inset,184,width,24);normal.setBounds(inset,208,width-48,44);folder.setBounds(inset+width-44,208,44,44);
        longLabel.setBounds(2*inset+width,184,width,24);longName.setBounds(2*inset+width,208,width,44);
        unavailableLabel.setBounds(inset,324,width,24);unavailable.setBounds(inset,348,width,44);
        emptyLabel.setBounds(2*inset+width,324,width,24);empty.setBounds(2*inset+width,348,width,44);
    }
    void paint(juce::Graphics& g)override
    {
        g.fillAll(background);applyTextRole(g,*this,TextRole::sectionHeading);g.drawText("Selectors - native family review",24,16,getWidth()-48,30,juce::Justification::centredLeft);
        applyTextRole(g,*this,TextRole::helper);g.drawText("Approved quiet menu direction - production consumers not migrated yet",24,50,getWidth()-48,24,juce::Justification::centredLeft);
        const auto width=(getWidth()-2*spacing::section-3*spacing::group)/4;
        const std::array<const char*,4> names{{"Idle","Hover","Mouse held","Keyboard focus"}};
        for(int index=0;index<4;++index)
        {
            const auto x=spacing::section+index*(width+spacing::group);applyTextRole(g,*this,TextRole::helper);g.drawText(names[static_cast<size_t>(index)],x,94,width,20,juce::Justification::centredLeft);
            const juce::Rectangle<int> bounds{x,118,width,44};SelectorPresentation::surface(g,bounds,{true,index==1,index==2,index==3,false});
            applyTextRole(g,*this,TextRole::controlValue);g.drawText("Preverb Hall",commandBodyBounds(bounds).reduced(spacing::labelHelp,0),juce::Justification::centredLeft);
            const auto centre=commandBodyBounds(bounds).withTrimmedLeft(width-30).toFloat().getCentre();juce::Path arrow;arrow.startNewSubPath(centre.x-4,centre.y-2);arrow.lineTo(centre.x,centre.y+2);arrow.lineTo(centre.x+4,centre.y-2);g.setColour(text);g.strokePath(arrow,juce::PathStrokeType(rule*2));
        }
        applyTextRole(g,*this,TextRole::helper);
        g.drawFittedText(sceneNumber==4?"Imported source unavailable (preview error scene)":sceneNumber==3?"No catalogue choices are available":"Defaults apply to new swells only",24,260,(getWidth()-72)/2,44,juce::Justification::topLeft,2,1.f);
        g.drawFittedText("Select a swell to change its sound.",24,400,(getWidth()-72)/2,40,juce::Justification::topLeft,2,1.f);
        g.drawFittedText("No choices are available in this preview scene.",getWidth()/2+12,400,(getWidth()-72)/2,40,juce::Justification::topLeft,2,1.f);
        g.setColour(separator);g.drawHorizontalLine(460,24.f,static_cast<float>(getWidth()-24));
        applyTextRole(g,*this,TextRole::helper);g.drawFittedText(message,24,474,getWidth()-48,42,juce::Justification::topLeft,2,1.f);
    }
    ChoiceSelector normal,longName,empty,unavailable;
    IconActionButton folder{ActionIcon::openFile};
private:
    void mouseMove(const juce::MouseEvent& event)override
    {
        if(auto* tooltip=dynamic_cast<juce::TooltipClient*>(event.eventComponent))
        {const auto help=tooltip->getTooltip();if(help.isNotEmpty() && help!=message){message=help;repaint();}}
    }
    juce::Label normalLabel,longLabel,emptyLabel,unavailableLabel;
    juce::String message;
    int sceneNumber=1;
};

class Panel final:public juce::Component
{
public:
    Panel()
    {
        setLookAndFeel(&theme);
        for(auto* component:std::array<juce::Component*,4>{&sizes,&scales,&scenes,&viewport})addAndMakeVisible(component);
        sizes.addItemList({"900 x 600","1000 x 700","1100 x 850","1400 x 1000"},1);sizes.setSelectedId(1,juce::dontSendNotification);sizes.setTitle("Review size");
        scales.addItemList({"100% UI scale","125% UI scale"},1);scales.setSelectedId(1,juce::dontSendNotification);scales.setTitle("Review scale");
        scenes.addItemList({"Normal","Dense / long catalogue","Empty","Unavailable / error"},1);scenes.setSelectedId(1,juce::dontSendNotification);scenes.setTitle("Review scene");
        sizes.onChange=[this]{if(onSize){const std::array<juce::Point<int>,4> values{{{900,600},{1000,700},{1100,850},{1400,1000}}};onSize(values[static_cast<size_t>(sizes.getSelectedId()-1)]);}};
        scales.onChange=[this]{fixture.normal.hidePopup();fixture.longName.hidePopup();resized();};scenes.onChange=[this]{fixture.setScene(scenes.getSelectedId());};
        viewport.setViewedComponent(&fixture,false);viewport.setFocusRevealEnabled(true);
        setSize(900,600);
    }
    ~Panel()override{viewport.setViewedComponent(nullptr,false);setLookAndFeel(nullptr);}
    void resized()override
    {
        sizes.setBounds(12,10,160,32);scales.setBounds(184,10,160,32);scenes.setBounds(356,10,230,32);
        viewport.setBounds(getLocalBounds().withTrimmedTop(54));
        const auto scale=scales.getSelectedId()==2?1.25f:1.f;fixture.setTransform(juce::AffineTransform::scale(scale));
        fixture.setSize(juce::roundToInt((viewport.getWidth()-viewport.getScrollBarThickness())/scale),juce::jmax(530,juce::roundToInt(viewport.getHeight()/scale)));
    }
    void paint(juce::Graphics& g)override{g.fillAll(background);}
    void setReviewScale(int percent){scales.setSelectedId(percent==125?2:1,juce::sendNotificationSync);resized();}
    Fixture fixture;
    std::function<void(juce::Point<int>)> onSize;
private:
    WorkspaceTheme theme;juce::ComboBox sizes,scales,scenes;FocusViewport viewport;
};
inline void run()
{
    class Window final:public juce::DocumentWindow
    {
    public:
        Window():juce::DocumentWindow("ReverbArise - #107 selector family review",background,closeButton)
        {
            setUsingNativeTitleBar(true);auto* panel=new Panel;setContentOwned(panel,true);
            panel->onSize=[this](juce::Point<int> size){setContentComponentSize(size.x,size.y);centreWithSize(getWidth(),getHeight());};
            setResizable(true,false);setResizeLimits(700,530,1800,1300);centreWithSize(getWidth(),getHeight());setVisible(true);
        }
        void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
    };
    Window window;juce::MessageManager::getInstance()->runDispatchLoop();
}
}
