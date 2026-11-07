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
};
}
