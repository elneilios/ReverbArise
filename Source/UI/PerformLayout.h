#pragma once
#include "UiGeometry.h"

namespace m3::ui
{
// Scoped compact metrics approved in the native #46 revision-4 checkpoint.
// Learn/header comfortable geometry and Export composition retain their owners.
namespace performGeometry
{
inline constexpr int listWidth=180,wideListWidth=210,inspectorWidth=304,wideInspectorWidth=320;
inline constexpr int columnGap=16,columnInset=12,auditionHeight=72,overviewHeight=120,infoHeight=36;
inline constexpr int toolbarHeight=48,readoutHeight=26,toolbarInset=6,toolbarControlHeight=36;
inline constexpr int toolbarGap=8,snapWidth=96,gridWidth=136,bandInset=8;
}
struct PerformLayout
{
    juce::Rectangle<int> header,work,list,detail,inspector,audition,overview,info,toolbar,readout;
    explicit PerformLayout(juce::Rectangle<int> bounds,int auditionBandHeight=performGeometry::auditionHeight)
    {
        using namespace performGeometry;
        auto area=bounds.reduced(workspaceGeometry::outerInset(bounds.getWidth()));
        header=area.removeFromTop(workspaceGeometry::headerHeight);
        info=area.removeFromBottom(infoHeight);overview=area.removeFromBottom(overviewHeight);
        audition=area.removeFromBottom(auditionBandHeight);work=area;
        list=area.removeFromLeft(bounds.getWidth()<1200?listWidth:wideListWidth);area.removeFromLeft(columnGap);
        inspector=area.removeFromRight(bounds.getWidth()<1200?inspectorWidth:wideInspectorWidth);area.removeFromRight(columnGap);
        toolbar=area.removeFromBottom(toolbarHeight);readout=area.removeFromBottom(readoutHeight);detail=area;
    }
};
}
