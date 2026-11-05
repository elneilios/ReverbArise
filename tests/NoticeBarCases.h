#pragma once
#include <UI/Controls/NoticeBar.h>

namespace noticeReview
{
using m3::ui::controls::NoticeBar;
using m3::ui::controls::NoticeMessage;
using m3::ui::controls::NoticeTone;
class Panel final:public juce::Component
{
public:
    Panel()
    {
        addAndMakeVisible(header);header.display(m3::ui::presentWorkflow({workflow::Mode::live},{true,true,true}));
        prepareHeader(workflow::Mode::live);
        header.onModeRequested=[this](auto mode){prepareHeader(mode);};
        header.addAndMakeVisible(headerNotice);
        headerNotice.setHeaderAppearance(true);
        showNotice(1);
        for(size_t i=0;i<states.size();++i)
        {
            addAndMakeVisible(states[i]);auto values=iconChoices();
            if(i==2)for(auto& value:values){value.available=false;value.unavailableReason="Source commit/validation is settling";}
            states[i].display(values,static_cast<int>(i)+1);
        }
        states[1].segment(1)->setState(juce::Button::buttonOver);
        explanation.setText("Perform retains a playback caution. Current notices replace it until they clear:",juce::dontSendNotification);addAndMakeVisible(explanation);
        const std::array<NoticeMessage,3> examples{{
            {"Recording - stop your DAW when finished","Source is captured at the DAW song position.",NoticeTone::information},
            {"Recapture is armed - existing audio will be replaced","Turn Recapture off to keep existing source protected.",NoticeTone::warning},
            {"Unable to load the requested impulse response","The previous valid response remains available.",NoticeTone::error}
        }};
        for(size_t i=0;i<bars.size();++i){addAndMakeVisible(bars[i]);bars[i].setHeaderAppearance(true);bars[i].display(examples[i]);}
        addAndMakeVisible(noticeChoice);noticeChoice.addItem("Header: Perform warning / restore",1);
        noticeChoice.addItem("Header: temporary IR preparation",2);noticeChoice.addItem("Header: current error",3);
        noticeChoice.setSelectedId(1,juce::dontSendNotification);noticeChoice.onChange=[this]{showNotice(noticeChoice.getSelectedId());};
        setSize(1100,510);
    }
    static std::vector<m3::ui::controls::SegmentChoice> iconChoices()
    {
        auto values=controlReview::choices();
        const m3::ui::controls::ModeIcon icons[]{m3::ui::controls::ModeIcon::learn,m3::ui::controls::ModeIcon::perform,m3::ui::controls::ModeIcon::exportAudio};
        for(size_t i=0;i<values.size();++i)values[i].icon=icons[i];return values;
    }
    void prepareHeader(workflow::Mode mode)
    {
        currentMode=mode;
        header.display(m3::ui::presentWorkflow({mode},{true,true,true}));
        showNotice(noticeChoice.getSelectedId());
    }
    void showNotice(int choice)
    {
        m3::EditorSnapshot editor;
        std::optional<NoticeMessage> preparing;
        if(choice==2)preparing=NoticeMessage{"Preparing IR: Whisper","The current IR is retained until preparation succeeds.",NoticeTone::information};
        if(choice==3)editor.actionError="Unable to prepare the requested swell";
        headerNotice.display(m3::ui::workspaceNotice({currentMode},{},editor,preparing));
    }
    void paint(juce::Graphics& g)override
    {
        g.fillAll(m3::ui::background);g.setColour(m3::ui::separator);g.drawHorizontalLine(102,16,getWidth()-16.0f);
        g.setColour(m3::ui::secondary);g.setFont(m3::ui::fontFor(*this,14));
        const char* labels[]{"Learn selected; inactive neighbours", "Perform selected; Learn hovered", "Export selected; all choices disabled during settlement"};
        for(size_t i=0;i<states.size();++i)g.drawText(labels[i],370,302+static_cast<int>(i)*58,getWidth()-390,36,juce::Justification::centredLeft);
        g.drawText("Try header: click modes; Tab, arrows, Home/End, Enter/Space. Check spacing, focus and disabled states.",16,472,getWidth()-32,28,juce::Justification::centredLeft);
    }
    void resized()override
    {
        header.setBounds(16,16,getWidth()-32,82);explanation.setBounds(16,102,getWidth()-32,25);
        const auto column=header.modeSelector().getBounds();
        headerNotice.setBounds(0,46,column.getX()-16,32);
        for(size_t i=0;i<states.size();++i)states[i].setBounds(16,302+static_cast<int>(i)*58,330,36);
        for(size_t i=0;i<bars.size();++i)bars[i].setBounds(16,136+static_cast<int>(i)*48,getWidth()-32,NoticeBar::preferredHeight);
        noticeChoice.setBounds(16,270,460,28);
    }
    m3::ui::WorkflowHeader header;std::array<NoticeBar,3> bars;
    NoticeBar headerNotice;m3::ui::controls::HeaderInformation& information=header.modeInformation();
    std::array<m3::ui::controls::SegmentedSelector,3> states;
private:juce::Label explanation;juce::ComboBox noticeChoice;workflow::Mode currentMode=workflow::Mode::live;
};
inline void cases()
{
    m3::ui::WorkspaceTheme theme;Panel panel;panel.setLookAndFeel(&theme);panel.addToDesktop(0);panel.setVisible(true);
    {
        NoticeBar normal,scoped;normal.setSize(500,32);scoped.setSize(500,32);scoped.setHeaderAppearance(true);
        const NoticeMessage sample{"Body ink remains scoped","Detail",NoticeTone::warning};normal.display(sample);scoped.display(sample);
        const auto a=normal.createComponentSnapshot(normal.getLocalBounds()),b=scoped.createComponentSnapshot(scoped.getLocalBounds());
        bool changed=false;for(int y=0;y<32;++y)for(int x=0;x<500;++x)
        {if(x<38)REQUIRE(a.getPixelAt(x,y)==b.getPixelAt(x,y));else changed|=a.getPixelAt(x,y)!=b.getPixelAt(x,y);}
        REQUIRE(changed);
    }
    auto& bar=panel.bars[0];panel.header.modeSelector().grabKeyboardFocus();const auto focus=juce::Component::getCurrentlyFocusedComponent();
    REQUIRE(m3::ui::fontFor(bar,m3::ui::bodySize).getTypefacePtr()->getName()=="Inter");
    REQUIRE(m3::ui::fontFor(bar,m3::ui::bodySize).getTypefaceStyle().containsIgnoreCase("Regular"));
    REQUIRE(m3::ui::fontFor(bar,m3::ui::bodySize).getTypefacePtr()->getStyle().containsIgnoreCase("Regular"));
    REQUIRE(m3::ui::fontFor(bar,m3::ui::controlSize,juce::Font::bold).getTypefacePtr()->getName()=="Inter");
    const NoticeMessage failure{"Unable to load the requested impulse response","Previous valid response retained",NoticeTone::error};
    REQUIRE(bar.display(failure) && bar.isVisible() && !bar.getWantsKeyboardFocus());
    REQUIRE(juce::Component::getCurrentlyFocusedComponent()==focus && !panel.header.isCurrentlyBlockedByAnotherModalComponent());
    const auto accessible=bar.getAccessibilityHandler();REQUIRE(accessible && accessible->getTitle().startsWith("Error: ") && accessible->getDescription()==failure.detail);
    REQUIRE(!bar.display(failure));
    const auto image=bar.createComponentSnapshot(bar.getLocalBounds());REQUIRE(image.getPixelAt(1,NoticeBar::preferredHeight/2)==m3::ui::danger);
    REQUIRE(bar.getTooltip().contains(failure.detail));REQUIRE(bar.display(std::nullopt) && !bar.isVisible());REQUIRE(!bar.display(std::nullopt));
    REQUIRE(juce::Component::getCurrentlyFocusedComponent()==focus);
    for(int width:{1068,1199,1200,1360,1560})for(auto mode:{workflow::Mode::capture,workflow::Mode::live,workflow::Mode::finalise})
    {
        panel.setSize(width+32,510);panel.prepareHeader(mode);
        const auto column=panel.header.modeSelector().getBounds();
        REQUIRE(panel.information.getBounds()==column.withY(46).withHeight(32));
        REQUIRE(panel.information.accentBounds()==juce::Rectangle<int>(0,8,2,16));
        REQUIRE(panel.information.textBounds().getX()==10 && panel.information.textBounds().getRight()==column.getWidth()-8);
        REQUIRE(panel.headerNotice.getRight()+16==panel.information.getX());
        const auto font=m3::ui::fontFor(panel.information,m3::ui::bodySize);
        REQUIRE(font.getHeight()==14 && !font.isItalic());
        REQUIRE(font.getTypefacePtr()->getStyle().containsIgnoreCase("Regular"));
        const auto copy=mode==workflow::Mode::live?"Shape and place reverse swells with precision":m3::ui::modeSubtitle(mode);
        REQUIRE(juce::GlyphArrangement::getStringWidth(font,copy)<=panel.information.textBounds().getWidth());
        for(auto tone:{NoticeTone::information,NoticeTone::warning,NoticeTone::error})for(const auto text:{juce::String("Short notice"),juce::String::repeatedString("Long contextual detail ",12)})
        {
            panel.headerNotice.display({{text,"Full accessible detail",tone}});
            const auto image=panel.headerNotice.createComponentSnapshot(panel.headerNotice.getLocalBounds());
            const auto ink=tone==NoticeTone::information?m3::ui::aqua:tone==NoticeTone::warning?m3::ui::warning:m3::ui::danger;
            REQUIRE(image.getPixelAt(1,16)==ink);
            REQUIRE(panel.headerNotice.getTooltip().contains(text));
            REQUIRE(panel.headerNotice.getAccessibilityHandler()->getDescription()=="Full accessible detail");
        }
        const auto before=panel.information.getBounds();panel.headerNotice.display(std::nullopt);REQUIRE(panel.information.getBounds()==before);
        panel.headerNotice.display({{"Recording - stop your DAW when finished","Full detail",NoticeTone::information}});
        for(float scale:{1.0f,1.25f,1.5f,2.0f})
        {
            const auto frame=panel.header.createComponentSnapshot(panel.header.getLocalBounds(),true,scale);
            REQUIRE(frame.isValid() && frame.getWidth()==juce::roundToInt(panel.header.getWidth()*scale));
        }
        snapshotPanel(panel,"header-icons-"+juce::String(static_cast<int>(mode))+"-"+juce::String(width));
    }
    panel.setSize(1100,510);panel.prepareHeader(workflow::Mode::live);
    for(const auto choice:{1,2,3,1})
    {
        panel.showNotice(choice);
        snapshotPanel(panel,"perform-warning-review-"+juce::String(choice));
    }
    bar.display({{"Recording - stop your DAW when finished","Source is captured at the DAW song position.",NoticeTone::information}});
    snapshotPanel(panel,"message-bar-controls");
    panel.setLookAndFeel(nullptr);panel.removeFromDesktop();
}
inline void run(){controlReview::run(std::make_unique<Panel>());}
}
