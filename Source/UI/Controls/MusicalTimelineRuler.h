#pragma once
#include <UI/SongBarRuler.h>
#include <UI/SwellViewport.h>
#include <UI/WorkspaceTheme.h>

namespace m3::ui::controls
{
struct MusicalRulerTick {double sample;juce::String label;bool bar;};
inline std::vector<MusicalRulerTick> musicalTimelineTicks(const SwellViewport& view,int width,const SongTime& timing)
{
    if(!timing.valid || width<=0)return {};
    const auto bars=makeSongBarRuler(view.first,view.last,width,timing.rate,timing.bpm,
        timing.numerator,timing.denominator,timing.sample,timing.ppq);
    std::vector<MusicalRulerTick> result;
    for(const auto& tick:bars)result.push_back({tick.samplePosition,juce::String(tick.bar)+".1",true});
    const auto beat=timing.rate*60/timing.bpm*4/timing.denominator;
    const auto bar=beat*timing.numerator;
    const auto pixels=beat*width/view.span();
    // Minor ticks never exceed the visible pixel budget, including deep zoom.
    if(!std::isfinite(beat) || beat<=0 || !std::isfinite(bar) || !std::isfinite(pixels)
        || pixels<spacing::group || !std::isfinite(view.span()) || view.span()<=0)return result;
    // Ask the existing bar authority for a nearby anchor, also when zoomed inside one bar.
    const auto anchors=makeSongBarRuler(view.first-bar,view.first+bar,controlGeometry::valueWidth*4,
        timing.rate,timing.bpm,timing.numerator,timing.denominator,timing.sample,timing.ppq);
    if(anchors.empty())return result;
    const auto& anchor=anchors.front();
    const auto first=std::ceil((view.first-anchor.samplePosition)/beat-1.e-9);
    const auto count=std::min(static_cast<int>(width/spacing::group)+2,static_cast<int>(std::ceil(view.span()/beat))+2);
    for(int i=0;i<count;++i)
    {
        const auto index=first+i;const auto sample=anchor.samplePosition+index*beat;
        if(sample>view.last)break;if(sample<view.first)continue;
        const auto beatInBar=static_cast<int>(std::fmod(std::fmod(index,timing.numerator)+timing.numerator,timing.numerator));
        if(beatInBar==0)continue; // Existing major ticks already cover visible bar starts.
        juce::String label;
        if(pixels>=controlGeometry::rulerLabelWidth+spacing::related)
            label=juce::String(anchor.bar+static_cast<int64_t>(std::floor(index/timing.numerator)))+"."+juce::String(beatInBar+1);
        result.push_back({sample,label,false});
    }
    std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.sample<b.sample;});
    return result;
}
// A composable painter; accepted fixed timing and sample bounds belong to the caller.
// Reuses the existing bar authority and sample/pixel transform. No transport or grid.
inline void paintMusicalTimelineRuler(juce::Graphics& g,juce::Rectangle<int> bounds,
                                     const SwellViewport& view,const SongTime& timing)
{
    if(bounds.isEmpty() || !timing.valid)return;
    const juce::Graphics::ScopedSaveState saved(g);g.reduceClipRegion(bounds);
    const auto ticks=musicalTimelineTicks(view,bounds.getWidth(),timing);
    applyTextRole(g,TextRole::ruler);
    int labelRight=bounds.getX()-spacing::tight;
    for(const auto& tick:ticks)
    {
        const auto x=static_cast<float>(view.pixel(tick.sample,bounds.getX(),bounds.getWidth()));
        const auto bottom=static_cast<float>(bounds.getBottom());
        g.drawLine(x,bottom-(tick.bar?spacing::labelHelp:spacing::tight),x,bottom,rule);
        const auto& label=tick.label;if(label.isEmpty())continue;
        const auto width=std::max(controlGeometry::rulerLabelWidth,
            static_cast<int>(juce::GlyphArrangement::getStringWidth(textFont(TextRole::ruler),label))+spacing::tight);
        const auto left=std::clamp(juce::roundToInt(x),bounds.getX(),std::max(bounds.getX(),bounds.getRight()-width));
        if(left<labelRight+spacing::tight)continue;
        g.drawText(label,juce::Rectangle<int>{left,bounds.getY(),width,std::max(0,bounds.getHeight()-spacing::labelHelp)},juce::Justification::centredLeft);
        labelRight=left+width;
    }
}
}
