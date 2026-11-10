#pragma once
#include <UI/WorkspaceTheme.h>
#include <cmath>
#include <optional>

namespace m3::ui::controls
{
struct DialTrackPresentation
{
    static float wheelAmount(const juce::MouseWheelDetails& wheel)
    {
        const auto amount=std::abs(wheel.deltaX)>std::abs(wheel.deltaY)?-wheel.deltaX:wheel.deltaY;
        return std::isfinite(amount)?(wheel.isReversed?-amount:amount):0.f;
    }

    static juce::Colour pointerInk(CommandState state)
    {
        return state.enabled && (state.hovered || state.pressed)
            ? outline.withAlpha(state.pressed?.29f:.14f)
            : juce::Colours::transparentBlack;
    }

    static void dial(juce::Graphics& g,juce::Rectangle<int> frame,CommandState state,float value,
                     juce::Slider::RotaryParameters angles,float fillOrigin,std::optional<float> home)
    {
        if(state.enabled && state.focused)paintCornerBrackets(g,frame,aqua);
        const auto centre=frame.toFloat().getCentre();
        const auto r=std::max(1.f,std::min(frame.getWidth(),frame.getHeight())*.5f-controlGeometry::focusPadding);
        const auto angle=[&](float position){return juce::jmap(position,angles.startAngleRadians,angles.endAngleRadians);};
        const auto alpha=state.enabled?1.f:.35f;
        const auto arc=[&](float from,float to,juce::Colour ink,float stroke)
        {
            if(from==to)return;
            juce::Path path;
            path.addCentredArc(centre.x,centre.y,r,r,0,angle(std::min(from,to)),angle(std::max(from,to)),true);
            g.setColour(ink.withMultipliedAlpha(alpha));
            g.strokePath(path,juce::PathStrokeType(stroke));
        };
        const auto feedback=pointerInk(state);
        if(!feedback.isTransparent())arc(0,1,feedback,7.f);
        arc(0,1,separator,3.f);
        arc(fillOrigin,value,aqua,state.enabled && state.pressed?4.f:3.f);
        const auto ray=[&](float fraction)
        {return juce::Point<float>{std::sin(angle(fraction)),-std::cos(angle(fraction))};};
        if(home)
        {
            const auto direction=ray(*home);
            g.setColour(outline.withMultipliedAlpha(alpha));
            g.drawLine({centre+direction*(r-4.f),centre+direction*(r-2.f)},rule);
        }
        g.setColour(text.withMultipliedAlpha(alpha));
        g.fillEllipse(juce::Rectangle<float>(5.f,5.f).withCentre(centre+ray(value)*r));
    }

    static void track(juce::Graphics& g,juce::Rectangle<int> frame,CommandState state,float first,float last,float value)
    {
        const auto feedback=pointerInk(state);
        if(!feedback.isTransparent())
        {
            g.setColour(feedback);
            g.fillRoundedRectangle(commandBodyBounds(frame).toFloat(),radius);
        }
        if(state.enabled && state.focused)paintCornerBrackets(g,frame,aqua);
        const auto y=static_cast<float>(frame.getCentreY());
        const auto alpha=state.enabled?1.f:.35f;
        g.setColour(separator.withMultipliedAlpha(alpha));
        g.drawHorizontalLine(juce::roundToInt(y),first,last);
        g.setColour(aqua.withMultipliedAlpha(alpha));
        const bool held=state.enabled && state.pressed;
        g.drawLine(first,y,value,y,held?4.f:3.f);
        g.setColour(text.withMultipliedAlpha(alpha));
        const auto marker=held?12.f:10.f;
        g.fillEllipse(juce::Rectangle<float>(marker,marker).withCentre({value,y}));
    }
};
}
