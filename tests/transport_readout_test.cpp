#include "TestSupport.h"
#include <UI/Controls/TransportReadout.h>
#include <UI/PerformLayout.h>
#include <limits>

using namespace m3;using namespace m3::ui;using controls::TransportReadout;

static void saveImage(juce::Component& component,const juce::String& tag,float scale)
{
    juce::Image image(juce::Image::ARGB,juce::roundToInt(component.getWidth()*scale),juce::roundToInt(component.getHeight()*scale),true,juce::SoftwareImageType{});
    juce::Graphics canvas(image);canvas.fillAll(background);canvas.addTransform(juce::AffineTransform::scale(scale));component.paintEntireComponent(canvas,true);
    const auto folder=juce::SystemStats::getEnvironmentVariable("PREVERB_TRANSPORT_OUTPUT",{});
    if(folder.isEmpty())return;
    juce::File directory(folder);REQUIRE(directory.createDirectory().wasOk());
    auto output=directory.getChildFile(tag+"-"+juce::String(juce::roundToInt(scale*100))+".png").createOutputStream();
    REQUIRE(output && juce::PNGImageFormat().writeImageToStream(image,*output));
}

class ReviewPanel final:public juce::Component,private juce::Timer
{
public:
    ReviewPanel()
    {
        addAndMakeVisible(title);applyTextRole(title,TextRole::sectionHeading);title.setText("Perform transport readout - issue #68",juce::dontSendNotification);
        addAndMakeVisible(note);applyTextRole(note,TextRole::helper);note.setText("Synthetic playback crosses bars 9/10 and 99/100; changes tempo, meter and availability. No host or audio actions.",juce::dontSendNotification);
        addAndMakeVisible(choices);choices.addItemList({"Playback transitions","Largest normal values","Unavailable","Oversized values"},1);choices.setSelectedId(1);choices.onChange=[this]{update();};
        addAndMakeVisible(readout);addAndMakeVisible(help);applyTextRole(help,TextRole::helper);help.setText("Numbers align right within fixed fields. Review spacing, readability and transitions; close when finished.",juce::dontSendNotification);
        setSize(1100,240);update();startTimer(700);
    }
    void paint(juce::Graphics& g)override{g.fillAll(background);}
    void resized()override
    {
        auto area=getLocalBounds().reduced(workspaceGeometry::outerInset(getWidth()));
        title.setBounds(area.removeFromTop(controlGeometry::headingHeight));note.setBounds(area.removeFromTop(controlGeometry::labelHeight));
        choices.setBounds(area.removeFromTop(controlGeometry::height).withWidth(280));area.removeFromTop(spacing::section);
        readout.setBounds(area.removeFromTop(performGeometry::infoHeight));help.setBounds(area.removeFromTop(controlGeometry::feedbackHeight));
    }
private:
    void timerCallback()override{++frame;update();}
    void update()
    {
        TransportPresentation value;
        if(choices.getSelectedId()==1)
        {
            const std::array<juce::String,8> bars{"9","10","99","100","100","99","10","9"};
            value.values={bars[frame%bars.size()],frame%2?"1":"4.999",frame%3?"82":"123.45",frame%3?"4 / 4":"12 / 8"};
            if(frame%9==8)value={};
        }
        if(choices.getSelectedId()==2)value.values={"-999999","999.999","9999.99","999 / 999"};
        if(choices.getSelectedId()==4)value.values={"-9223372036854775808","2147483647.999",juce::String(std::numeric_limits<double>::max(),2),"2147483647 / 2147483647"};
        readout.display(value);
    }
    TransportReadout readout;juce::Label title,note,help;juce::ComboBox choices;size_t frame=0;
};
class ReviewWindow final:public juce::DocumentWindow
{
public:
    ReviewWindow():DocumentWindow("ReverbArise #68 - native readout review",background,DocumentWindow::closeButton)
    {setUsingNativeTitleBar(true);setContentOwned(new ReviewPanel,true);setResizable(true,false);setResizeLimits(1100,240,2000,400);centreWithSize(1100,240);setVisible(true);}
    void closeButtonPressed()override{setVisible(false);juce::MessageManager::getInstance()->stopDispatchLoop();}
};

int main(int argc,char* argv[]){return runTest([&]
{
    juce::ScopedJuceInitialiser_GUI gui;WorkspaceTheme theme;
    if(argc>1 && juce::String(argv[1])=="--review")
    {
        ReviewWindow window;window.setLookAndFeel(&theme);juce::MessageManager::getInstance()->runDispatchLoop();window.setLookAndFeel(nullptr);return;
    }
    TransportReadout readout;readout.setLookAndFeel(&theme);readout.setSize(1068,36);readout.addToDesktop(0);readout.setVisible(true);
    REQUIRE(readout.readoutFont().getTypefacePtr()->getName()=="Inter");
    REQUIRE(readout.readoutFont().getHeight()==15 && !readout.getWantsKeyboardFocus());
    SongTime time{true,0,0,48000,120,4,4};
    for(auto size:{juce::Point<int>{1100,850},{1400,900},{1400,1000},{1600,1000}})
    {
        PerformLayout perform({0,0,size.x,size.y});readout.setSize(perform.info.getWidth(),perform.info.getHeight());const auto geometry=readout.geometry();
        REQUIRE(readout.getLocalBounds().contains(geometry.group));
        for(size_t i=0;i<4;++i)
        {
            REQUIRE(geometry.fields[i].contains(geometry.values[i]));
            if(i<3){REQUIRE(geometry.fields[i].getRight()<geometry.separators[i].getX());REQUIRE(geometry.separators[i].getRight()<geometry.fields[i+1].getX());}
        }
        // Independent PPQ oracle: at 120 BPM, each 4/4 bar is 96000 samples.
        for(const auto bar:{9,10,99,100,999999,-999999})for(const auto beat:{0.,.5,2.999,3.99999})
        {
            time.sample=static_cast<int64_t>(std::llround((static_cast<double>(bar)-1)*96000+beat*24000));
            time.ppq=static_cast<double>(time.sample)/24000.;
            auto value=transportPresentation(time,true,true,82.25,true,12,8);readout.display(value);
            REQUIRE(readout.geometry()==geometry);for(size_t i=0;i<4;++i)REQUIRE(!readout.overflows(i));
            if(beat==0.)REQUIRE(value.values[0]==juce::String(bar) && value.values[1]=="1");
            if(beat==3.99999)REQUIRE(value.values[0]==juce::String(bar+1) && value.values[1]=="1");
        }
        for(int mask=0;mask<8;++mask)
        {
            auto value=transportPresentation(time,(mask&1)!=0,(mask&2)!=0,9999.99,(mask&4)!=0,999,999);readout.display(value);
            REQUIRE(readout.geometry()==geometry);
            REQUIRE((value.values[0]=="unavailable")==!(mask&1));REQUIRE((value.values[2]=="unavailable")==!(mask&2));REQUIRE((value.values[3]=="unavailable")==!(mask&4));
            for(size_t i=0;i<4;++i)REQUIRE(!readout.overflows(i));
        }
        for(const auto bpm:{82.,123.45,9999.99})for(const auto meter:{4,6,12,999})
        {readout.display(transportPresentation(time,true,true,bpm,true,meter,8));REQUIRE(readout.geometry()==geometry);}
        TransportPresentation normal;normal.values={"-999999","999.999","9999.99","999 / 999"};readout.display(normal);
        for(size_t i=0;i<4;++i)REQUIRE(!readout.overflows(i));
        for(const auto scale:{1.f,1.25f,1.5f,2.f})saveImage(readout,"normal-"+juce::String(size.x)+"x"+juce::String(size.y),scale);
        readout.display({});for(const auto scale:{1.f,1.25f,1.5f,2.f})saveImage(readout,"unavailable-"+juce::String(size.x)+"x"+juce::String(size.y),scale);
        auto extreme=normal;extreme.values={"-9223372036854775808","2147483647.999",juce::String(std::numeric_limits<double>::max(),2),"2147483647 / 2147483647"};readout.display(extreme);
        REQUIRE(readout.geometry()==geometry);for(size_t i=0;i<4;++i)REQUIRE(readout.overflows(i));
        const auto handler=readout.getAccessibilityHandler();REQUIRE(handler);REQUIRE(handler->getDescription()==extreme.help());REQUIRE(readout.getTooltip()==extreme.help());
        saveImage(readout,"overflow-"+juce::String(size.x),1.f);
    }
    auto unavailable=transportPresentation(time,false,true,82.,true,6,8);REQUIRE(unavailable.values[0]=="unavailable" && unavailable.values[2]=="82" && unavailable.values[3]=="6 / 8");
    time.valid=false;REQUIRE(transportPresentation(time,true,false,0,false,0,0).values[1]=="unavailable");
    readout.removeFromDesktop();readout.setLookAndFeel(nullptr);std::cout<<"Transport readout geometry, fit, availability, accessibility and software-scale matrix passed.\n";
});}
