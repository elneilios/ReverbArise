#pragma once
#include "UiGeometry.h"
#include <algorithm>

namespace m3::ui
{
// Issue #58 owner-approved mockup revision 4. Native checkpoint precedes rollout.
// Semantic tokens remain separate even when their current values coincide.
namespace responsiveGeometry
{
inline constexpr int minimumWidth=900,minimumHeight=600;
inline constexpr int outerMargin=12,contentPadding=8,groupGap=12,dividerClearance=8;
inline constexpr int entryInset=6,controlHeight=32,entryHeight=26,dialSize=36;
inline constexpr int headerHeight=64,primaryHeight=36,noticeTop=40,noticeHeight=24;
inline constexpr int listWidth=160,inspectorWidth=276,headingHeight=24;
inline constexpr int tabsHeight=32,flagsHeight=32,captionHeight=16,valueRowHeight=52;
inline constexpr int auditionHeight=48,footerHeight=20,navigationRowHeight=38,readoutHeight=24;
inline constexpr int zoomMinimum=212,snapWidth=96,gridWidth=136,navigationGap=8;
inline constexpr int inlineNavigationMinimum=472;
inline constexpr int inspectorContentTop=headingHeight+flagsHeight+tabsHeight+contentPadding;
inline constexpr int soundModuleHeight=236;
inline constexpr int laneCaptionHeight=20,laneCaptionGap=8;
inline constexpr int laneCaptionBand=laneCaptionHeight+2*laneCaptionGap;
inline constexpr int overviewHeadingWidth=132;
inline constexpr int exportTileHeight=176;
inline constexpr float noticeMinimum=11;
}

struct ResponsiveTypography
{
    float heading=16,navigation=14,body=12,caption=11,value=11,notice=12;
    static ResponsiveTypography at(int width,int height) noexcept
    {
        const auto small=std::clamp(std::min((width-900)/200.0f,(height-600)/250.0f),0.0f,1.0f);
        const auto large=std::clamp(std::min((width-1100)/300.0f,(height-850)/150.0f),0.0f,1.0f);
        return {16+2*small+3*large,14+2*small+2*large,12+small+large,
            11+small+large,11+small+large,12+small+large};
    }
};

struct ResponsiveWorkspaceLayout
{
    juce::Rectangle<int> header,notice,body,work,list,detail,inspector,inspectorHeading,flags,tabs,fields,resets;
    juce::Rectangle<int> audition,overview,position,help,toolbar,zoom,snap,grid,readout;
    bool inlineNavigation=false;
    explicit ResponsiveWorkspaceLayout(juce::Rectangle<int> bounds)
    {
        using namespace responsiveGeometry;
        const auto x=bounds.getX()+outerMargin,width=std::max(0,bounds.getWidth()-2*outerMargin);
        header={x,bounds.getY()+outerMargin,width,headerHeight};
        notice={x,header.getY()+noticeTop,width,noticeHeight};
        help={x,bounds.getBottom()-footerHeight,width,footerHeight};
        position={x,help.getY()-footerHeight,width,footerHeight};
        const auto overviewHeight=juce::roundToInt(std::clamp(48.0+(bounds.getHeight()-600)*.128,48.0,100.0));
        overview={x,position.getY()-overviewHeight,width,overviewHeight};
        audition={x,overview.getY()-auditionHeight,width,auditionHeight};
        work={x,header.getBottom()+contentPadding,width,std::max(0,audition.getY()-header.getBottom()-contentPadding)};
        body={x,work.getY(),width,std::max(0,help.getY()-work.getY())};
        auto area=work;list=area.removeFromLeft(std::min(listWidth,area.getWidth()));area.removeFromLeft(std::min(groupGap,area.getWidth()));
        inspector=area.removeFromRight(std::min(inspectorWidth,area.getWidth()));area.removeFromRight(std::min(groupGap,area.getWidth()));
        inlineNavigation=area.getWidth()>=inlineNavigationMinimum;
        auto navigation=area.removeFromBottom((inlineNavigation?1:2)*navigationRowHeight+contentPadding);
        navigation.removeFromBottom(contentPadding);toolbar=navigation;
        readout=area.removeFromBottom(readoutHeight);detail=area;
        if(inlineNavigation)
        {
            grid=navigation.removeFromRight(gridWidth);navigation.removeFromRight(navigationGap);
            snap=navigation.removeFromRight(snapWidth);navigation.removeFromRight(navigationGap);zoom=navigation;
        }
        else
        {
            zoom=navigation.removeFromTop(navigationRowHeight);
            grid=navigation.removeFromRight(gridWidth);navigation.removeFromRight(navigationGap);snap=navigation.removeFromRight(snapWidth);
        }
        auto controls=inspector.reduced(contentPadding,0).withTrimmedRight(1);
        inspectorHeading=controls.removeFromTop(headingHeight);flags=controls.removeFromTop(flagsHeight);tabs=controls.removeFromTop(tabsHeight);
        controls.removeFromTop(contentPadding);controls.removeFromBottom(contentPadding);resets=controls.removeFromBottom(controlHeight);
        controls.removeFromBottom(contentPadding);fields=controls;
    }
    juce::Line<float> listDivider()const noexcept{return dividerAt(list.getRight());}
    juce::Line<float> inspectorDivider()const noexcept{return dividerAt(inspector.getX());}
private:
    juce::Line<float> dividerAt(int x)const noexcept
    {
        const auto start=work.getY()+responsiveGeometry::dividerClearance;
        const auto end=std::max(start,work.getBottom()-responsiveGeometry::dividerClearance);
        return {{static_cast<float>(x),static_cast<float>(start)},{static_cast<float>(x),static_cast<float>(end)}};
    }
};
}
