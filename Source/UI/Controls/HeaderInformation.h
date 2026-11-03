#pragma once
#include <UI/WorkspaceTheme.h>

namespace m3::ui::controls
{
inline const juce::Colour headerBodyInk{0xffc7cdd4};

// Candidate information-row primitive. Screen owns its reserved column bounds.
class HeaderInformation final:public juce::Component,public juce::SettableTooltipClient
{
public:
    void display(const juce::String& value)
    {if(value==description)return;description=value;setTitle(value);setDescription(value);setTooltip(value);repaint();}
    juce::Rectangle<int> accentBounds()const{return {0,(getHeight()-16)/2,2,16};}
    juce::Rectangle<int> textBounds()const{return getLocalBounds().withTrimmedLeft(10).withTrimmedRight(8);}
    void paint(juce::Graphics& g)override
    {
        g.setColour(aqua);g.fillRect(accentBounds());g.setColour(headerBodyInk);g.setFont(fontFor(*this,bodySize));
        g.drawText(description,textBounds(),juce::Justification::centredLeft);
    }
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler()override
    {return std::make_unique<juce::AccessibilityHandler>(*this,juce::AccessibilityRole::staticText);}
private:
    juce::String description;
};
}
