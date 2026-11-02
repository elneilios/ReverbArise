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
        header.onModeRequested=[this](auto mode){header.display(m3::ui::presentWorkflow({mode},{true,true,true}));};
        explanation.setText("Normal Perform stays clean. Important notices use this shared inline bar:",juce::dontSendNotification);addAndMakeVisible(explanation);
        const std::array<NoticeMessage,3> examples{{
            {"Recording - stop your DAW when finished","Source is captured at the DAW song position.",NoticeTone::information},
            {"Recapture is armed - existing audio will be replaced","Turn Recapture off to keep existing source protected.",NoticeTone::warning},
            {"Unable to load the requested impulse response","The previous valid response remains available.",NoticeTone::error}
        }};
        for(size_t i=0;i<bars.size();++i){addAndMakeVisible(bars[i]);bars[i].display(examples[i]);}
        setSize(1100,300);
    }
    void paint(juce::Graphics& g)override{g.fillAll(m3::ui::background);g.setColour(m3::ui::separator);g.drawHorizontalLine(86,16,getWidth()-16.0f);}
    void resized()override
    {
        header.setBounds(16,16,getWidth()-32,70);explanation.setBounds(16,98,getWidth()-32,25);
        for(size_t i=0;i<bars.size();++i)bars[i].setBounds(16,136+static_cast<int>(i)*48,getWidth()-32,NoticeBar::preferredHeight);
    }
    m3::ui::WorkflowHeader header;std::array<NoticeBar,3> bars;
private:juce::Label explanation;
};
inline void cases()
{
    m3::ui::WorkspaceTheme theme;Panel panel;panel.setLookAndFeel(&theme);panel.addToDesktop(0);panel.setVisible(true);
    auto& bar=panel.bars[0];panel.header.modeSelector().grabKeyboardFocus();const auto focus=juce::Component::getCurrentlyFocusedComponent();
    const NoticeMessage failure{"Unable to load the requested impulse response","Previous valid response retained",NoticeTone::error};
    REQUIRE(bar.display(failure) && bar.isVisible() && !bar.getWantsKeyboardFocus());
    REQUIRE(juce::Component::getCurrentlyFocusedComponent()==focus && !panel.header.isCurrentlyBlockedByAnotherModalComponent());
    const auto accessible=bar.getAccessibilityHandler();REQUIRE(accessible && accessible->getTitle().startsWith("Error: ") && accessible->getDescription()==failure.detail);
    REQUIRE(!bar.display(failure));
    const auto image=bar.createComponentSnapshot(bar.getLocalBounds());REQUIRE(image.getPixelAt(1,NoticeBar::preferredHeight/2)==m3::ui::danger);
    REQUIRE(bar.getTooltip().contains(failure.detail));REQUIRE(bar.display(std::nullopt) && !bar.isVisible());REQUIRE(!bar.display(std::nullopt));
    REQUIRE(juce::Component::getCurrentlyFocusedComponent()==focus);
    bar.display({{"Recording - stop your DAW when finished","Source is captured at the DAW song position.",NoticeTone::information}});
    snapshotPanel(panel,"message-bar-controls");
    panel.setLookAndFeel(nullptr);panel.removeFromDesktop();
}
inline void run(){controlReview::run(std::make_unique<Panel>());}
}
