#include "TestSupport.h"
#include <UI/Controls/TimelineZoomControls.h>
#include <UI/Controls/MusicalTimelineRuler.h>
#include <UI/Controls/LatchingActionButton.h>
#include <juce_gui_extra/juce_gui_extra.h>

using namespace m3::ui;
using namespace m3::ui::controls;
static void pump(){juce::MessageManager::getInstance()->runDispatchLoopUntil(30);}
static void click(juce::Component& target)
{
    const auto p=target.getLocalBounds().getCentre().toFloat();const auto time=juce::Time::getCurrentTime();
    auto event=[&](juce::ModifierKeys mods){return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),p,mods,0,0,0,0,0,&target,&target,time,p,time,1,false);};
    target.mouseDown(event(juce::ModifierKeys::leftButtonModifier));target.mouseUp(event({}));pump();
}
static void cases()
{
    WorkspaceTheme theme;TimelineZoomControls row;row.setLookAndFeel(&theme);
    row.setBounds(0,0,320,controlGeometry::height);row.setVisible(true);
    int step=0,fraction=0,fits=0,begins=0;double factor=0;
    row.beforeInvocation=[&]{++begins;};row.onZoomStep=[&](double value){++step;factor=value;};
    row.onZoomFraction=[&](double){++fraction;};row.onFit=[&]{++fits;};
    row.display({.5,true,true,true});pump();REQUIRE(step==0 && fraction==0 && fits==0);
    click(row.zoomInButton());REQUIRE(step==1 && factor==.8 && begins>0);
    row.zoomOutButton().keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();REQUIRE(step==2 && factor==1.25);
    row.fitButton().keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));pump();REQUIRE(fits==1);
    row.zoomSlider().keyPressed(juce::KeyPress(juce::KeyPress::rightKey));pump();REQUIRE(fraction==1);
    const auto afterKey=row.zoomSlider().getValue();
    const auto time=juce::Time::getCurrentTime();const auto p=row.zoomSlider().getLocalBounds().getCentre().toFloat();
    juce::MouseEvent e(juce::Desktop::getInstance().getMainMouseSource(),p,{},0,0,0,0,0,&row.zoomSlider(),&row.zoomSlider(),time,p,time,1,false);
    juce::MouseWheelDetails wheel{};wheel.deltaY=.1f;row.zoomSlider().mouseWheelMove(e,wheel);pump();
    REQUIRE(row.zoomSlider().getValue()>afterKey && fraction==2);
    row.zoomSlider().setValue(.9,juce::sendNotificationAsync);
    row.display({.25,true,true,true});pump();REQUIRE(fraction==2); // Accepted refresh cancels queued intent.
    row.fitButton().triggerClick();row.display({0,false,false,false});pump();REQUIRE(fits==1);
    row.zoomInButton().keyPressed(juce::KeyPress(juce::KeyPress::returnKey));pump();REQUIRE(step==2);
    row.display({std::numeric_limits<double>::quiet_NaN(),true,true,false});
    REQUIRE(!row.zoomSlider().isEnabled());
    row.display({2,true,false,true});REQUIRE(row.zoomSlider().getValue()==1);
    for(const int width:{TimelineZoomControls::minimumWidth(),320,480})
    {row.setSize(width,controlGeometry::height);for(auto* child:row.getChildren())REQUIRE(row.getLocalBounds().contains(child->getBounds()));
        REQUIRE(row.zoomSlider().getWidth()>=controlGeometry::height);
        for(const float scale:{1.f,1.25f,1.5f,2.f})REQUIRE(row.createComponentSnapshot(row.getLocalBounds(),true,scale).isValid());}
    REQUIRE(row.zoomInButton().getWantsKeyboardFocus() && row.zoomSlider().getWantsKeyboardFocus());
    row.addToDesktop(juce::ComponentPeer::windowIsTemporary);
    REQUIRE(row.zoomInButton().getAccessibilityHandler()!=nullptr);
    row.zoomInButton().grabKeyboardFocus();pump();REQUIRE(row.zoomInButton().hasKeyboardFocus(false));
    row.removeFromDesktop();
    REQUIRE(textFont(TextRole::ruler).getTypefacePtr()->getName().contains("Inter"));
    m3::SwellViewport view;view.first=-96000;view.last=384000;
    SongTime clock{true,0,0,48000,120,4,4};
    auto ticks=musicalTimelineTicks(view,1000,clock);
    REQUIRE(std::any_of(ticks.begin(),ticks.end(),[](const auto& t){return !t.bar && t.sample==24000;}));
    view.first=24000;view.last=72000;ticks=musicalTimelineTicks(view,600,clock);
    REQUIRE(std::any_of(ticks.begin(),ticks.end(),[](const auto& t){return t.label=="1.2" && t.sample==24000;}));
    clock.numerator=6;clock.denominator=8;view.first=0;view.last=72000;ticks=musicalTimelineTicks(view,900,clock);
    REQUIRE(std::any_of(ticks.begin(),ticks.end(),[](const auto& t){return t.label=="1.2" && t.sample==12000;}));
    clock.bpm=0;REQUIRE(musicalTimelineTicks(view,900,clock).empty());
    // Valid fixed timing with a huge meter keeps bar offsets supported while the
    // fine denominator beat count is far beyond int. Density must stay pixel-bound.
    clock={true,0,0,48000,120,INT_MAX,INT_MAX};view.first=-1.e12;view.last=1.e12;
    const auto fineBeat=clock.rate*60/clock.bpm*4/clock.denominator;
    REQUIRE(view.span()/fineBeat>static_cast<double>(INT_MAX));
    // Check the numerical conversion directly too: the ruler's density gate
    // would otherwise mask a regression in this calculation for tiny beats.
    for(const auto [width,budget]:{std::pair{64,6},std::pair{1024,66}})
    {
        REQUIRE(m3::ui::controls::detail::visibleMusicalBeatCount(view.span(),fineBeat,width)==budget);
        ticks=musicalTimelineTicks(view,width,clock);
        REQUIRE(!ticks.empty() && ticks.size()<=static_cast<size_t>(budget));
        for(const auto& tick:ticks)
            REQUIRE(tick.bar && std::isfinite(tick.sample) && tick.sample>=view.first && tick.sample<=view.last);
    }
    clock={true,0,0,48000,120,4,4};view.first=-96000;view.last=384000;
    const auto image=[&](SongTime timing){juce::Image result(juce::Image::ARGB,500,34,true);juce::Graphics g(result);
        paintMusicalTimelineRuler(g,{0,0,500,34},view,timing);return result;};
    auto available=image(clock);clock.valid=false;auto unavailable=image(clock);int marked=0;
    for(int x=0;x<500;++x)for(int y=0;y<34;++y){marked+=available.getPixelAt(x,y).getAlpha()!=0;REQUIRE(unavailable.getPixelAt(x,y).getAlpha()==0);}
    REQUIRE(marked>0);row.setLookAndFeel(nullptr);
    LatchingActionButton snap("Snap");snap.setLookAndFeel(&theme);snap.setBounds(0,0,96,34);snap.setVisible(true);
    bool requested=true,ctrl=false;int edits=0;
    const auto display=[&]{snap.display(requested && !ctrl);};
    snap.onClick=[&]{if(snap.isEnabled()){requested=!requested;++edits;display();}};
    display();REQUIRE(snap.getToggleState() && edits==0);
    ctrl=true;display();REQUIRE(!snap.getToggleState() && requested && edits==0);
    ctrl=false;display();REQUIRE(snap.getToggleState() && requested && edits==0);
    click(snap);REQUIRE(!requested && !snap.getToggleState() && edits==1);
    ctrl=true;display();ctrl=false;display();REQUIRE(!snap.getToggleState() && !requested && edits==1);
    snap.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));pump();REQUIRE(requested && snap.getToggleState() && edits==2);
    snap.triggerClick();snap.setEnabled(false);pump();REQUIRE(requested && edits==2);
    snap.display(true);REQUIRE(edits==2);snap.setEnabled(true);
    for(const float scale:{1.f,1.25f,1.5f,2.f})REQUIRE(snap.createComponentSnapshot(snap.getLocalBounds(),true,scale).isValid());
    snap.addToDesktop(juce::ComponentPeer::windowIsTemporary);snap.grabKeyboardFocus();pump();
    REQUIRE(snap.hasKeyboardFocus(false) && snap.getAccessibilityHandler()!=nullptr);
    auto* accessibility=snap.getAccessibilityHandler();snap.display(true);
    REQUIRE(accessibility->getCurrentState().isCheckable() && accessibility->getCurrentState().isChecked());
    snap.display(false);REQUIRE(!accessibility->getCurrentState().isChecked() && edits==2);
    snap.display(true);REQUIRE(accessibility->getCurrentState().isChecked() && edits==2);
    REQUIRE(snap.getLocalBounds().toFloat().contains(snap.focusInkBounds()));
    snap.removeFromDesktop();ctrl=true;display();click(snap);
    REQUIRE(!requested && !snap.getToggleState() && edits==3);ctrl=false;display();REQUIRE(!snap.getToggleState());
    snap.setLookAndFeel(nullptr);
}

// Small binding example: the workspace owns bounds, logarithmic domain and Fit.
class Example final:public juce::Component
{
public:
    Example()
    {
        setLookAndFeel(&theme);addAndMakeVisible(zoom);view.first=0;view.last=384000;
        zoom.onZoomStep=[this](double factor){view.zoom(factor);clamp();};
        zoom.onZoomFraction=[this](double fraction){view.zoom(spanFor(fraction)/view.span());clamp();};
        zoom.onFit=[this]{view.first=0;view.last=384000;refresh();};refresh();
    }
    ~Example()override{setLookAndFeel(nullptr);}
    void resized()override{zoom.setBounds(getLocalBounds().reduced(spacing::section).removeFromTop(controlGeometry::height));}
    void paint(juce::Graphics& g)override
    {
        g.fillAll(background);auto area=getLocalBounds().reduced(spacing::section);area.removeFromTop(controlGeometry::height+spacing::related);
        paintMusicalTimelineRuler(g,area.removeFromTop(controlGeometry::height),view,{true,0,0,48000,120,4,4});
        applyTextRole(g,TextRole::helper);g.drawText("Horizontal zoom only | workspace owns Fit | no audio or Snap",area,juce::Justification::centred);
    }
private:
    static double spanFor(double fraction){return 384000*std::pow(480./384000,fraction);}
    void clamp(){const auto span=std::clamp(view.span(),480.,384000.);view.last=view.first+span;refresh();}
    void refresh(){zoom.display({std::log(384000/view.span())/std::log(384000/480.),view.span()>480,view.span()<384000,true});repaint();}
    WorkspaceTheme theme;TimelineZoomControls zoom;m3::SwellViewport view;
};
class Review final:public juce::DocumentWindow
{
public:
    Review():DocumentWindow("Shared timeline controls - Export handover",background,DocumentWindow::closeButton)
    {setUsingNativeTitleBar(true);setContentOwned(new Example,true);centreWithSize(560,210);setVisible(true);}
    void closeButtonPressed()override{setVisible(false);juce::MessageManager::getInstance()->stopDispatchLoop();}
};
class SnapExample final:public juce::Component,private juce::Timer
{
public:
    SnapExample()
    {
        setLookAndFeel(&theme);for(auto* component:std::initializer_list<juce::Component*>{&on,&off,&disabled,&help})addAndMakeVisible(component);
        on.setTitle("Snap requested On");off.setTitle("Snap requested Off");disabled.setEnabled(false);disabled.display(true);
        on.onClick=[this]{requestedOn=!requestedOn;refresh();};off.onClick=[this]{requestedOff=!requestedOff;refresh();};
        applyTextRole(help,TextRole::helper);help.setText("Saved On / Saved Off / Disabled\nHold Ctrl to bypass; release to restore. Control preview only.",juce::dontSendNotification);
        refresh();startTimerHz(30);
    }
    ~SnapExample()override{stopTimer();setLookAndFeel(nullptr);}
    void resized()override
    {auto area=getLocalBounds().reduced(spacing::section);auto row=area.removeFromTop(controlGeometry::height);
        for(auto* button:{&on,&off,&disabled}){button->setBounds(row.removeFromLeft(96));row.removeFromLeft(spacing::related);}
        area.removeFromTop(spacing::related);help.setBounds(area);}
    void paint(juce::Graphics& g)override{g.fillAll(background);}
private:
    void timerCallback()override{refresh();}
    void refresh(){const auto bypass=juce::ModifierKeys::getCurrentModifiersRealtime().isCtrlDown();on.display(requestedOn && !bypass);off.display(requestedOff && !bypass);}
    WorkspaceTheme theme;LatchingActionButton on{"Snap"},off{"Snap"},disabled{"Snap"};juce::Label help;
    bool requestedOn=true,requestedOff=false;
};
class SnapReview final:public juce::DocumentWindow
{
public:
    SnapReview():DocumentWindow("Issue #46 - Snap toggle control",background,DocumentWindow::closeButton)
    {setUsingNativeTitleBar(true);setContentOwned(new SnapExample,true);centreWithSize(560,190);setVisible(true);}
    void closeButtonPressed()override{setVisible(false);juce::MessageManager::getInstance()->stopDispatchLoop();}
};
int main(int argc,char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    return runTest([&]{cases();
        const auto snapOutput=juce::SystemStats::getEnvironmentVariable("PREVERB_SNAP_OUTPUT",{});
        if(snapOutput.isNotEmpty())
        {
            SnapExample example;example.setSize(560,190);example.setVisible(true);
            juce::File file(snapOutput);REQUIRE(file.getParentDirectory().createDirectory().wasOk());
            auto stream=file.createOutputStream();REQUIRE(stream);juce::PNGImageFormat png;
            REQUIRE(png.writeImageToStream(example.createComponentSnapshot(example.getLocalBounds()),*stream));
        }
        const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_TIMELINE_OUTPUT",{});
        if(output.isNotEmpty())
        {
            Example example;example.setSize(560,210);example.setVisible(true);
            juce::File directory(output);REQUIRE(directory.createDirectory().wasOk());
            for(const float scale:{1.f,1.25f,1.5f,2.f})
            {auto stream=directory.getChildFile("shared-timeline-"+juce::String(juce::roundToInt(scale*100))+".png").createOutputStream();
                REQUIRE(stream);juce::PNGImageFormat png;REQUIRE(png.writeImageToStream(example.createComponentSnapshot(example.getLocalBounds(),true,scale),*stream));}
        }
        if(argc>1 && juce::String(argv[1])=="--review"){Review window;juce::MessageManager::getInstance()->runDispatchLoop();}
        if(argc>1 && juce::String(argv[1])=="--review-snap"){SnapReview window;juce::MessageManager::getInstance()->runDispatchLoop();}});
}
