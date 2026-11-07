#pragma once
#include <UI/WorkspaceTheme.h>

namespace m3::ui::controls
{
// Owner-approved field anatomy, 2026-11-07. Initially exercised in the C2 native
// preview; adoption by text/numeric input families remains their separate work.
struct FieldPresentation
{
    static constexpr float helperHeight=12;
    static constexpr int helperGap=4,helperLineHeight=17;
    static juce::Font helperFont(){return interFont(helperHeight,InterFace::regular);}
    static juce::Colour feedbackInk(bool error){return error?danger:textStyle(TextRole::helper).ink;}
    static juce::Colour borderInk(bool error,juce::Colour normal,bool enabled)
    {return error?danger.withMultipliedAlpha(enabled?1.f:.35f):normal;}
    static juce::Rectangle<int> feedbackBounds(juce::Rectangle<int> body,int width,int lines=2)
    {return {body.getX(),body.getBottom()+helperGap,width,lines*helperLineHeight};}

    class WrappedFeedback
    {
    public:
        void setMessage(const juce::String& message, bool error, juce::Rectangle<int> bodies)
        {
            layout = {};
            bounds = {};
            if (message.isEmpty() || bodies.isEmpty())
                return;

            const auto font = helperFont();
            juce::AttributedString attributedMessage;
            attributedMessage.setJustification(juce::Justification::topLeft);
            // JUCE still fits whole words first. This permits a split when even
            // an empty line cannot hold a word, such as a long file name.
            attributedMessage.setWordWrap(juce::AttributedString::byChar);
            attributedMessage.setLineSpacing(helperLineHeight - font.getHeight());
            attributedMessage.append(message, font, feedbackInk(error));
            layout.createLayout(attributedMessage, static_cast<float>(bodies.getWidth()));
            bounds = feedbackBounds(bodies, bodies.getWidth(), layout.getNumLines());
            bounds.setHeight(std::max(bounds.getHeight(), juce::roundToInt(std::ceil(layout.getHeight()))));
        }

        juce::Rectangle<int> getBounds() const { return bounds; }
        const juce::TextLayout& getLayout() const { return layout; }

        void paint(juce::Graphics& graphics) const
        {
            if (!bounds.isEmpty())
                layout.draw(graphics, bounds.toFloat());
        }

    private:
        juce::TextLayout layout;
        juce::Rectangle<int> bounds;
    };
};

// The associated inputs expose the full description to accessibility clients.
class FieldFeedbackLabel final : public juce::Component
{
public:
    FieldFeedbackLabel()
    {
        setAccessible(false);
        setInterceptsMouseClicks(false, false);
    }

    void arrange(const juce::String& message, bool error, juce::Rectangle<int> bodies)
    {
        feedback.setMessage(message, error, bodies);
        setBounds(feedback.getBounds());
        repaint();
    }

    void paint(juce::Graphics& graphics) override
    {
        juce::Graphics::ScopedSaveState state(graphics);
        graphics.setOrigin(-getX(), -getY());
        feedback.paint(graphics);
    }

private:
    FieldPresentation::WrappedFeedback feedback;
};
}
