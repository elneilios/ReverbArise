#pragma once
#include "HostGrid.h"
#include <juce_core/juce_core.h>

namespace m3
{
struct PerformNavigationState
{
    bool hasView=false,snap=true;
    double first=-24000,last=384000;
    GridDivision grid=GridDivision::quarterBeat;
    bool operator==(const PerformNavigationState&)const=default;
};
inline bool validPerformView(double first,double last)noexcept
{
    return std::isfinite(first) && std::isfinite(last) && std::isfinite(last-first)
        && first>=-9223372036854775808. && last<9223372036854775808. && last-first>=1;
}
inline PerformNavigationState normalizedPerformNavigation(PerformNavigationState state)
{
    if(!std::any_of(gridChoices.begin(),gridChoices.end(),[&](const auto& choice){return choice.division==state.grid;}))state.grid=GridDivision::quarterBeat;
    if(!state.hasView || !validPerformView(state.first,state.last))
    {state.hasView=false;state.first=-24000;state.last=384000;}
    return state;
}
inline juce::var performNavigationMetadata(PerformNavigationState value)
{
    const auto state=normalizedPerformNavigation(value);auto* object=new juce::DynamicObject;
    object->setProperty("snap",state.snap);
    for(const auto& choice:gridChoices)if(choice.division==state.grid)object->setProperty("grid",juce::String(choice.key.data()));
    if(state.hasView){auto* view=new juce::DynamicObject;view->setProperty("first",state.first);view->setProperty("last",state.last);object->setProperty("view",juce::var(view));}
    return juce::var(object);
}
// Optional UI fields are independently tolerant. Malformed preferences never
// turn an otherwise valid musical/Export session into a failed restore.
inline PerformNavigationState parsePerformNavigation(const juce::var& object)
{
    PerformNavigationState state;if(!object.isObject())return state;
    if(object["snap"].isBool())state.snap=static_cast<bool>(object["snap"]);
    if(object["grid"].isString())for(const auto& choice:gridChoices)
        if(object["grid"].toString()==juce::String(choice.key.data()))state.grid=choice.division;
    const auto view=object["view"];
    if(view.isObject())
    {
        const auto first=view["first"],last=view["last"];
        const auto numeric=[](const juce::var& value){return value.isDouble() || value.isInt() || value.isInt64();};
        if(numeric(first) && numeric(last) && validPerformView(static_cast<double>(first),static_cast<double>(last)))
        {state.hasView=true;state.first=static_cast<double>(first);state.last=static_cast<double>(last);}
    }
    return state;
}
}
