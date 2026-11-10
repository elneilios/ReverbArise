#pragma once
#include "EllipsisLabel.h"

namespace m3::ui::controls
{
// One measured layout owns both wrapping and painting. Screens allocate its
// preferred height and keep following controls in their existing scroll region.
class WrappedLabel final : public EllipsisLabel
{
public:
    int preferredHeight(int width)const
    {
        if(getText().isEmpty())return 0;
        return static_cast<int>(std::ceil(textLayout(width).getHeight()))+getBorderSize().getTopAndBottom();
    }
    juce::TextLayout textLayout(int width)const
    {
        juce::AttributedString text;
        const auto font=getLookAndFeel().getLabelFont(const_cast<WrappedLabel&>(*this));
        text.setWordWrap(juce::AttributedString::byChar);
        text.setJustification(getJustificationType());
        text.setLineSpacing(std::max(0.f,textStyle(TextRole::helper).lineHeight-font.getHeight()));
        text.append(getText(),font,findColour(juce::Label::textColourId));
        juce::TextLayout layout;
        layout.createLayout(text,static_cast<float>(std::max(1,width-getBorderSize().getLeftAndRight())));
        return layout;
    }
    void paint(juce::Graphics& g)override
    {
        textLayout(getWidth()).draw(g,getBorderSize().subtractedFrom(getLocalBounds()).toFloat());
    }
};
}
