#pragma once
#include <UI/WorkspaceTheme.h>

namespace m3::ui::controls
{
// C2 candidate geometry for the native family checkpoint. No production opt-in
// before scoped approval. Existing typography, semantic colours and C1 focus
// geometry remain the authorities; these metrics own only popup composition.
struct SelectorPresentation
{
    static constexpr int popupMinimumWidth=220,popupMaximumWidth=420;
    static constexpr int popupPadding=4,rowPadding=8,checkWidth=22;
    static constexpr int focusMargin=static_cast<int>(controlGeometry::focusOutset+controlGeometry::focusStroke);
    static juce::Colour raised(){return background.interpolatedWith(outline,.22f);}
    static juce::Colour held(){return background.interpolatedWith(outline,.4f);}
    static void surface(juce::Graphics& g,juce::Rectangle<int> bounds,CommandState state)
    {
        paintCommandSurface(g,bounds,CommandVariant::neutral,state);
        const auto alpha=state.enabled?1.f:.35f;
        const auto fill=state.enabled && state.pressed?held():state.enabled && state.hovered?raised():background;
        const auto body=commandBodyBounds(bounds);
        g.setColour(fill.withMultipliedAlpha(alpha));g.fillRoundedRectangle(controlGeometry::insideStroke(body,rule),radius);
        const auto ink=commandInk(CommandVariant::neutral,state);
        g.setColour(ink.border);g.drawRoundedRectangle(controlGeometry::insideStroke(body,ink.stroke),radius,ink.stroke);
    }
};
}
