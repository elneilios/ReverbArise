#include <UI/Controls/ActionButtonStateField.h>

namespace
{
using namespace m3::ui;
using namespace m3::ui::controls;

class PlaybackRow final : public juce::Component
{
public:
    enum class Phase { ready, queued, playing, finishing, unavailable };

    explicit PlaybackRow(Phase initial) : phase(initial)
    {
        auditionHeading.setText("Audition", juce::dontSendNotification);
        loopHeading.setText("Loop", juce::dontSendNotification);
        for (auto* heading : {&auditionHeading, &loopHeading})
        {
            applyTextRole(*heading, TextRole::sectionHeading);
            heading->setColour(juce::Label::textColourId, text);
            heading->setBorderSize({});
        }
        for (auto* child : std::array<juce::Component*, 6>{
                 &auditionHeading, &play, &mix, &loopHeading, &placement, &loop})
            addAndMakeVisible(*child);

        showAudition();
        play.action().onClick = [this] { auditionPlaying = !auditionPlaying; showAudition(); };
        mix.onClick = [this] { mix.setButtonText(mix.getButtonText() == "Dry + wet" ? "Wet only" : "Dry + wet"); };
        placement.setState("Bar 1 : Beat 1", "bar 1, beat 1");
        placement.action().onClick = [this] { placement.setState("Bar 17 : Beat 1", "bar 17, beat 1"); };
        const auto supported = initial != Phase::unavailable;
        placement.action().setEnabled(supported);
        placement.setActionHelp(supported ? "Set Loop beat one to the Playlist playhead"
                                          : "Requires a host song position");
        loop.action().onClick = [this]
        {
            switch (phase)
            {
                case Phase::ready: phase = Phase::queued; break;
                case Phase::queued: phase = Phase::ready; break;
                case Phase::playing: phase = Phase::finishing; break;
                case Phase::finishing: phase = Phase::ready; break;
                case Phase::unavailable: return;
            }
            showPhase();
        };
        showPhase();
    }

    void resized() override
    {
        const auto height = getWidth() < 1000 ? 32 : 36;
        auto row = getLocalBounds().withSizeKeepingCentre(getWidth(), height);
        const auto auditionSpace = 148 + (row.getWidth() - 828) / 3;
        const auto playWidth = std::min(auditionSpace, play.preferredWidth());
        const auto headingWidth = [](const juce::Label& heading)
        {
            return static_cast<int>(std::ceil(juce::GlyphArrangement::getStringWidth(
                textFont(TextRole::sectionHeading), heading.getText()))) + spacing::related;
        };
        auditionHeading.setBounds(row.removeFromLeft(headingWidth(auditionHeading)));
        play.setBounds(row.removeFromLeft(playWidth));
        mix.setBounds(row.removeFromLeft(100));
        row.removeFromLeft(auditionSpace - playWidth);
        row.removeFromLeft(spacing::tight);
        divider = row.removeFromLeft(static_cast<int>(rule));
        row.removeFromLeft(spacing::related);
        loopHeading.setBounds(row.removeFromLeft(headingWidth(loopHeading)));
        placement.setBounds(row.removeFromLeft(std::min(254, placement.preferredWidth())));
        row.removeFromLeft(spacing::tight);
        loop.setBounds(row);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(separator);
        g.fillRect(divider);
    }

private:
    void showAudition()
    {
        play.action().setPlaybackIcon(auditionPlaying);
        play.action().setTitle(auditionPlaying ? "Stop audition" : "Play audition");
        play.setActionVariant(auditionPlaying ? CommandVariant::destructive : CommandVariant::primary);
        play.setActionHelp(auditionPlaying ? "Stop audition preview" : "Play audition preview");
        play.setState(auditionPlaying ? "Auditioning" : "Ready | 2 swells");
    }

    void showPhase()
    {
        const auto index = static_cast<size_t>(phase);
        const std::array<juce::String, 5> actions{"Cue Loop", "Cancel Loop", "Stop Loop", "Stop Now", "Cue Loop"};
        const std::array<juce::String, 5> states{"Ready", "Waiting for bar", "Playing loop",
                                                "Finishing the current pass before stopping", "Waiting for clock"};
        loop.action().setButtonText(actions[index]);
        loop.setActionVariant(phase == Phase::ready || phase == Phase::unavailable
                                  ? CommandVariant::neutral : CommandVariant::destructive);
        loop.action().setEnabled(phase != Phase::unavailable);
        loop.setActionHelp(states[index]);
        loop.setState(states[index]);
    }

    Phase phase;
    bool auditionPlaying = false;
    juce::Label auditionHeading, loopHeading;
    ActionButtonStateField play{"Play Audition", CommandIcon::play, CommandVariant::primary, "Audition status", 40, 120};
    ActionButton mix{"Dry + wet"};
    ActionButtonStateField placement{"Set Loop Start", CommandIcon::none, CommandVariant::primary, "Loop start", 130};
    ActionButtonStateField loop{"Cue Loop", CommandIcon::none, CommandVariant::neutral, "Loop status", 112};
    juce::Rectangle<int> divider;
};

class PreviewPanel final : public juce::Component, private juce::Timer
{
public:
    PreviewPanel()
    {
        title.setText("Perform", juce::dontSendNotification);
        applyTextRole(title, TextRole::sectionHeading);
        addAndMakeVisible(title);
        for (size_t i = 0; i < rows.size(); ++i)
        {
            captions[i].setText(names[i], juce::dontSendNotification);
            applyTextRole(captions[i], TextRole::helper);
            addAndMakeVisible(captions[i]);
            addAndMakeVisible(*rows[i]);
        }
        applyTextRole(hoverHelp, TextRole::helper);
        hoverHelp.setColour(juce::Label::textColourId, contextHelpInk);
        hoverHelp.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(hoverHelp);
        startTimerHz(20);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(spacing::group);
        title.setBounds(bounds.removeFromTop(controlGeometry::headingHeight));
        for (size_t i = 0; i < rows.size(); ++i)
        {
            captions[i].setBounds(bounds.removeFromTop(controlGeometry::labelHeight));
            rows[i]->setBounds(bounds.removeFromTop(controlGeometry::height + spacing::tight));
            bounds.removeFromTop(spacing::tight);
        }
        hoverHelp.setBounds(bounds.removeFromBottom(controlGeometry::labelHeight));
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(background);
        g.setColour(separator);
        for (const auto* row : rows)
            g.drawHorizontalLine(row->getY(), static_cast<float>(row->getX()), static_cast<float>(row->getRight()));
    }

private:
    void timerCallback() override
    {
        const auto point = getMouseXYRelative();
        juce::String message;
        for (auto* component = getComponentAt(point); component && component != this;
             component = component->getParentComponent())
        {
            message = component->getProperties()["contextHelp"].toString();
            if (message.isEmpty())
                if (auto* client = dynamic_cast<juce::TooltipClient*>(component))
                    message = client->getTooltip();
            if (message.isNotEmpty())
                break;
        }
        hoverHelp.setText(message, juce::dontSendNotification);
    }

    const std::array<juce::String, 5> names{"Ready to cue", "Queued", "Playing", "Finishing", "Unsupported host"};
    juce::Label title;
    EllipsisLabel hoverHelp;
    std::array<juce::Label, 5> captions;
    PlaybackRow ready{PlaybackRow::Phase::ready}, queued{PlaybackRow::Phase::queued},
        playing{PlaybackRow::Phase::playing}, finishing{PlaybackRow::Phase::finishing},
        unavailable{PlaybackRow::Phase::unavailable};
    const std::array<PlaybackRow*, 5> rows{&ready, &queued, &playing, &finishing, &unavailable};
};

class PreviewWindow final : public juce::DocumentWindow
{
public:
    PreviewWindow() : DocumentWindow("ReverbArise - Audition and Loop review", background, closeButton)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new PreviewPanel, false);
        setResizable(true, false);
        setResizeLimits(900, 460, 1600, 850);
        centreWithSize(1100, 460);
        setVisible(true);
    }

    void closeButtonPressed() override { juce::MessageManager::getInstance()->stopDispatchLoop(); }
};
}

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;
    WorkspaceTheme theme;
    PreviewWindow window;
    window.setLookAndFeel(&theme);
    juce::MessageManager::getInstance()->runDispatchLoop();
    window.setLookAndFeel(nullptr);
}
