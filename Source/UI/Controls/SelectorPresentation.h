#pragma once
#include <UI/WorkspaceTheme.h>
#include "FieldPresentation.h"

namespace m3::ui::controls
{
// C2 geometry approved at the 2026-11-07 native family checkpoint. Adoption is
// explicit per consumer. Existing typography, semantic colours and C1 focus
// geometry remain the authorities; these metrics own only popup composition.
struct SelectorPresentation
{
    static constexpr int popupMinimumWidth=220,popupMaximumWidth=420;
    static constexpr int popupPadding=4,rowPadding=8,checkWidth=22;
    static constexpr int focusMargin=static_cast<int>(controlGeometry::focusOutset+controlGeometry::focusStroke);
    static juce::Colour raised(){return background.interpolatedWith(outline,.22f);}
    static juce::Colour held(){return background.interpolatedWith(outline,.4f);}
    static void surface(juce::Graphics& g,juce::Rectangle<int> bounds,CommandState state,bool error=false)
    {
        paintCommandSurface(g,bounds,CommandVariant::neutral,state);
        const auto alpha=state.enabled?1.f:.35f;
        const auto fill=state.enabled && state.pressed?held():state.enabled && state.hovered?raised():background;
        const auto body=commandBodyBounds(bounds);
        g.setColour(fill.withMultipliedAlpha(alpha));g.fillRoundedRectangle(controlGeometry::insideStroke(body,rule),radius);
        const auto ink=commandInk(CommandVariant::neutral,state);
        g.setColour(FieldPresentation::borderInk(error,ink.border,state.enabled));g.drawRoundedRectangle(controlGeometry::insideStroke(body,ink.stroke),radius,ink.stroke);
    }
};
}
