#pragma once
#include "PresentationPreferences.h"
#include <algorithm>

namespace m3::ui
{
// Inputs are already host/desktop logical units. Chrome is measured separately
// and is not magnified with plugin content; never multiply Windows DPI again.
struct PresentationExtent
{
    int width=0,height=0;
    bool operator==(const PresentationExtent&)const=default;
};
struct PresentationFitContext
{
    PresentationExtent workArea,chrome;
    PresentationExtent available()const noexcept
    {return {std::max(0,std::max(0,workArea.width)-std::max(0,chrome.width)),std::max(0,std::max(0,workArea.height)-std::max(0,chrome.height))};}
};
inline PresentationExtent presentationExtent(PresentationPreferences value)noexcept
{
    if(!PresentationPreferences::validWidth(value.width) || !PresentationPreferences::validHeight(value.height)
        || !PresentationPreferences::validScale(value.scalePercent))return {};
    return {(value.width*value.scalePercent+99)/100,(value.height*value.scalePercent+99)/100};
}
inline bool presentationFits(PresentationPreferences value,PresentationFitContext context)noexcept
{
    if(!PresentationPreferences::validWidth(value.width) || !PresentationPreferences::validHeight(value.height)
        || !PresentationPreferences::validScale(value.scalePercent))return false;
    const auto requested=presentationExtent(value),available=context.available();
    return requested.width<=available.width && requested.height<=available.height;
}
struct FittedPresentation
{
    PresentationPreferences effective;
    bool supported=false,recovered=false;
};
inline FittedPresentation fitPresentation(PresentationPreferences desired,PresentationFitContext context)noexcept
{
    const auto original=desired;
    if(!PresentationPreferences::validWidth(desired.width))desired.width=900;
    if(!PresentationPreferences::validHeight(desired.height))desired.height=600;
    if(!PresentationPreferences::validScale(desired.scalePercent))desired.scalePercent=100;
    if(presentationFits(desired,context))return {desired,true,desired!=original};
    const auto available=context.available();
    // Preserve preferred magnification when a smaller workspace can fit it;
    // otherwise recover at 100%. Sub-100% text shrinking is never automatic.
    for(const int scale:{desired.scalePercent,100})
    {
        const PresentationPreferences baseline{900,600,scale};
        if(!presentationFits(baseline,context))continue;
        const PresentationPreferences effective{
            static_cast<int>(std::min<juce::int64>(desired.width,static_cast<juce::int64>(available.width)*100/scale)),
            static_cast<int>(std::min<juce::int64>(desired.height,static_cast<juce::int64>(available.height)*100/scale)),scale};
        return {effective,true,true};
    }
    return {{900,600,100},false,true};
}
}
