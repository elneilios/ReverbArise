#pragma once

#include "Controls/ActionButton.h"
#include "Controls/ChoiceSelector.h"
#include "Controls/SegmentedSelector.h"
#include "Controls/SwellList.h"
#include "Controls/TimelineZoomControls.h"
#include "ResponsivePresentation.h"
#include "KnobValue.h"
#include "PerformText.h"
#include "WaveformDisplay.h"
#include <DSP/DefaultImpulse.h>
#include <DSP/LoopPerformance.h>

namespace m3::ui
{
class LoopPerformWorkspace final : public juce::Component
{
public:
    LoopPerformWorkspace()
    {
        setName("loopPerformWorkspace");
        addAndMakeVisible(swells);
        swells.setName("loopSwellList");
        swells.setMultipleSelectionEnabled(false);
        swells.onReveal=[this](const auto&,uint64_t){zoomTo(0);};
        addAndMakeVisible(sections);
        sections.setName("loopInspectorSections");
        sections.setTitle("Selected swell section");
        sections.onRequest=[this](int id){section=id;showSection();resized();};
        addAndMakeVisible(zoom);
        zoom.setName("loopTimelineZoom");
        zoom.onZoomFraction=[this](double value){zoomTo(value);};
        zoom.onZoomStep=[this](double factor){zoomTo(zoomFraction-std::log(factor)/std::log(16.0));};
        zoom.onFit=[this]{centre.reset();zoomTo(0);};
        for (size_t field=0;field<knobs.size();++field)
        {
            auto& knob=knobs[field];
            addChildComponent(knob);
            knob.configure(fieldNames[field],fieldIds[field],"Type milliseconds or drag the knob. Shift-drag for fine adjustment.");
            knob.setCompactPresentation(true);
            knob.onBegin=knob.onRebase=[this,field]
            {
                const auto [low,high]=limits(field);
                knobEdits[field]={view.binding,view.edit.*members[field],high-low};
            };
            knob.onEnd=[this,field]{knobEdits[field].reset();};
            knob.onDelta=[this,field](double delta)
            {
                if(const auto& anchor=knobEdits[field])
                    setField(field,anchor->sample+anchor->span*delta,anchor->binding);
            };
            knob.onStep=[this,field](double steps)
            {
                if(const auto& anchor=knobEdits[field])
                    setField(field,anchor->sample+steps*std::max(1.0,view.rate/1000.0),anchor->binding);
            };
            knob.onAbsolute=[this,field](double ms)
            {
                if(const auto& anchor=knobEdits[field]) setField(field,ms*view.rate/1000.0,anchor->binding);
            };
            knob.onCommit=[this,field](const juce::String& input)
            {
                const auto& anchor=knobEdits[field];
                if(!anchor || anchor->binding!=view.binding)return false;
                auto value=input.trim().toStdString();
                const auto* first=value.data();
                const auto* last=first+value.size();
                if(first!=last && *first=='+')++first;
                double ms=0;
                const auto parsed=std::from_chars(first,last,ms,std::chars_format::general);
                const auto [low,high]=limits(field);
                const auto samples=ms*view.rate/1000.0;
                return parsed.ec==std::errc{} && parsed.ptr==last && std::isfinite(samples)
                    && samples>=low-.5 && samples<=high+.5 && setField(field,samples,anchor->binding);
            };
            knob.setResetAction([this,field]
            {
                const auto binding=view.binding;
                return KnobValue::ResetAction{"Reset "+juce::String(fieldNames[field]),
                    [this,field,binding]{setField(field,LoopPerformance::defaultEdit(view.frames,view.rate).*members[field],binding);}};
            });
        }
        addAndMakeVisible(resetShape);
        addAndMakeVisible(resetAll);
        resetShape.setName("loopResetShape");
        resetAll.setName("loopResetAll");
        resetShape.setTooltip("Reset Move Wet, lead-in and fade out. Keep the source range and reverb.");
        resetAll.setTooltip("Reset the source range, reverb and wet shape for this swell.");
        resetShape.beforeInvocation=resetAll.beforeInvocation=[this]{resetBinding=view.binding;};
        resetShape.onClick=[this]{reset(false);};
        resetAll.onClick=[this]{reset(true);};
        getProperties().set("contextHelp","Drag the source range ends or move the wet waveform. When zoomed in, drag Overview to navigate.");
        addChildComponent(impulseLabel);
        impulseLabel.setText("Reverb",juce::dontSendNotification);
        impulseLabel.setBorderSize({});
        applyTextRole(impulseLabel,TextRole::controlLabel);
        addChildComponent(impulse);
        impulse.setTitle("Loop impulse response");
        impulse.setTooltip("Select a bundled reverb sound for this Loop swell.");
        int id=1;
        for (const auto& entry:bundledImpulses()) impulse.addItem(entry.name,id++);
        impulse.beforeInvocation=[this]{impulseBinding=view.binding;};
        impulse.onChange=[this]
        {
            const auto selected=impulse.getSelectedId();
            const auto choices=bundledImpulses();
            if (updating || !onEdit || selected<=0 || static_cast<size_t>(selected)>choices.size()) return;
            auto edit=view.edit;
            edit.impulse=choices[static_cast<size_t>(selected-1)].id;
            onEdit(edit,impulseBinding);
        };
        addAndMakeVisible(audition);addAndMakeVisible(stop);addAndMakeVisible(status);
        audition.setName("loopAudition");stop.setName("loopStopAudition");
        stop.setTitle("Stop Audition");
        audition.setTooltip("Play the full prepared wet swell once, including its leading audio.");
        stop.setTooltip("Fade the current audition without stopping the live patch.");
        audition.beforeInvocation=[this]{auditionBinding=view.binding;};
        audition.onClick=[this]{if(onAudition)onAudition(auditionBinding);};
        stop.onClick=[this]{if(onStop)onStop();};
        addAndMakeVisible(cue);addAndMakeVisible(stopLoop);addAndMakeVisible(syncStatus);
        cue.setName("loopCue");stopLoop.setName("loopStop");
        cue.setTooltip("Queue the wet loop for a phrase start with enough time for the lead-in.");
        stopLoop.setTooltip("Cancel a queued Loop or fade the wet playback. The live patch keeps playing.");
        cue.beforeInvocation=[this]{cueBinding=view.binding;};
        cue.onClick=[this]{if(onCue)onCue(cueBinding);};
        stopLoop.onClick=[this]{if(onStopLoop)onStopLoop();};
        applyTextRole(status,TextRole::helper);
        status.setBorderSize({});
        applyTextRole(syncStatus,TextRole::helper);
        syncStatus.setBorderSize({});
        addAndMakeVisible(positionReadout);
        positionReadout.setName("loopPositionReadout");
        applyTextRole(positionReadout,TextRole::helper,juce::Justification::centred);
        showSection();
    }

    std::function<bool(LoopPerformance::Edit,uint64_t)> onEdit;
    std::function<void(uint64_t)> onAudition;
    std::function<void()> onStop;
    std::function<void(uint64_t)> onCue;
    std::function<void()> onStopLoop;

    void display(LoopPerformance::View next,const LoopCapture::Waveform& wave,bool compatible)
    {
        updating=true;
        if(view.binding!=next.binding){zoomFraction=0;centre.reset();drag.reset();navigating=false;}
        view=std::move(next);
        dry=wave;
        editable=compatible && view.rate>0 && view.frames>0;
        const auto enabled=editable;
        if(!enabled){drag.reset();navigating=false;}
        for(size_t i=0;i<knobs.size();++i)
        {
            const auto [low,high]=limits(i);
            const auto sample=view.edit.*members[i];
            const auto unit=view.rate>0?1000.0/view.rate:1.0;
            const auto ms=sample*unit;
            const auto span=std::max(1,high-low);
            knobs[i].displayDomain({ms,low*unit,high*unit,std::max(unit,1.0),span*unit,compactValue(ms,2)+" ms"});
            knobs[i].display(juce::String(static_cast<juce::int64>(view.binding)),compactValue(ms,2),
                             static_cast<double>(sample-low)/span,enabled && low<high,preciseValue(ms));
        }
        resetShape.setEnabled(enabled);
        resetAll.setEnabled(enabled);
        impulse.setEnabled(enabled);
        int id=1;
        for (const auto& entry:bundledImpulses())
        {
            if (view.edit.impulse==entry.id){impulse.setSelectedId(id,juce::dontSendNotification);break;}
            ++id;
        }
        const juce::String selected="loopSwell";
        const auto* ir=m3::findBundledImpulse(view.edit.impulse);
        if(view.frames>0)
            swells.display({{selected,"Loop swell",ir?ir->name:juce::String{},view.wet?"Ready":view.status}},
                           {selected},selected,view.binding);
        else
            swells.display({}, {}, {},view.binding);
        zoom.display({zoomFraction,enabled && zoomFraction<1,enabled && zoomFraction>0,enabled});
        using Sync=LoopPlayback::Status;
        const auto active=view.sync==Sync::queued || view.sync==Sync::leadIn || view.sync==Sync::repeating || view.sync==Sync::stopping;
        audition.setEnabled(enabled && view.wet!=nullptr && !view.playing && !active);
        stop.setEnabled(view.playing);
        cue.setEnabled(enabled && view.cueReady && view.sync==Sync::standby);
        stopLoop.setEnabled(active && view.sync!=Sync::stopping);
        const juce::String message=!compatible?"The Loop take needs its original processing rate."
            :view.playing?"Auditioning Loop swell":view.status;
        status.setText(message,juce::dontSendNotification);
        status.setTooltip(message);
        juce::String sync;
        switch(view.sync)
        {
            case Sync::waiting: sync="Waiting for phrase reference"; break;
            case Sync::standby: sync=view.cueReady?"Ready to cue":"Preparing wet"; break;
            case Sync::queued: sync="Cue queued"; break;
            case Sync::leadIn: sync="Playing lead-in"; break;
            case Sync::repeating: sync="Repeating wet"; break;
            case Sync::stopping: sync="Stopping wet"; break;
            case Sync::referenceMismatch: sync="Phrase marker missing or period differs"; break;
            case Sync::syncLost: sync="Sync lost. Cue again once ready."; break;
        }
        if(!compatible)sync="Original rate required";
        else if(!active && view.cueStatus.isNotEmpty())sync=view.cueStatus;
        syncStatus.setText(sync,juce::dontSendNotification);
        syncStatus.setTooltip(sync);
        auto timingText=view.rate>0?"Phrase length  "+seconds(view.frames):juce::String{};
        if (view.loopPosition && active && view.rate>0)
            timingText=( *view.loopPosition<0?"Lead-in  ":"Phrase position  ")+seconds(*view.loopPosition)+"   |   "+timingText;
        positionReadout.setText(timingText,juce::dontSendNotification);
        updating=false;
        repaint();
    }

    void resized() override
    {
        using namespace responsiveGeometry;
        const auto bodyTop=outerMargin+headerHeight+contentPadding;
        const ResponsiveWorkspaceLayout layout({-outerMargin,-bodyTop,getWidth()+2*outerMargin,
                                                getHeight()+bodyTop+footerHeight});
        listDivider=layout.listDivider();
        inspectorDivider=layout.inspectorDivider();
        auto listArea=layout.list.withTrimmedRight(contentPadding);
        listHeading=listArea.removeFromTop(headingHeight);
        listArea.removeFromTop(contentPadding);
        swells.setBounds(listArea);
        auto inspector=layout.inspector.reduced(contentPadding,0);
        heading=inspector.removeFromTop(headingHeight);
        sections.setBounds(inspector.removeFromTop(tabsHeight));
        inspector.removeFromTop(contentPadding);
        if(section==1)
        {
            knobs[2].setBounds(inspector.removeFromTop(valueRowHeight));
            knobs[0].setBounds(inspector.removeFromTop(valueRowHeight));
            knobs[1].setBounds(inspector.removeFromTop(valueRowHeight));
        }
        else if(section==2)
        {
            auto row=inspector.removeFromTop(valueRowHeight);
            knobs[3].setBounds(row.removeFromLeft((row.getWidth()-contentPadding)/2));
            knobs[4].setBounds(row.withTrimmedLeft(contentPadding));
        }
        else
        {
            impulseLabel.setBounds(inspector.removeFromTop(captionHeight));
            impulse.setBounds(inspector.removeFromTop(controlHeight));
        }
        auto resets=layout.resets;
        resetShape.setBounds(resets.removeFromLeft((resets.getWidth()-contentPadding)/2));
        resetAll.setBounds(resets.withTrimmedLeft(contentPadding));
        auto footer=layout.audition.reduced(0,contentPadding);
        const auto monitorWidth=footer.getWidth()*2/5;
        auto monitor=footer.removeFromLeft(monitorWidth);
        auditionHeading=monitor.removeFromLeft(listWidth/2);
        audition.setBounds(monitor.removeFromLeft(controlHeight+contentPadding));
        monitor.removeFromLeft(spacing::related);
        stop.setBounds(monitor.removeFromLeft(controlHeight+contentPadding));
        monitor.removeFromLeft(contentPadding);
        status.setBounds(monitor);
        footer.removeFromLeft(groupGap);
        loopHeading=footer.removeFromLeft(listWidth*3/4);
        cue.setBounds(footer.removeFromLeft(snapWidth));
        footer.removeFromLeft(spacing::related);
        stopLoop.setBounds(footer.removeFromLeft(snapWidth));
        footer.removeFromLeft(contentPadding);
        syncStatus.setBounds(footer);
        bands={layout.audition.getY(),layout.overview.getY(),layout.overview.getBottom()};
        auto position=layout.position;
        positionReadout.setBounds(position);
        auto overview=layout.overview.reduced(0,contentPadding);
        overviewHeading=overview.removeFromLeft(overviewHeadingWidth);
        overviewChart=overview;
        auto area=layout.work.withLeft(layout.detail.getX()).withRight(layout.detail.getRight());
        auto tools=area.removeFromBottom(navigationRowHeight).reduced(0,contentPadding/2);
        zoom.setBounds(tools);
        readout=area.removeFromBottom(readoutHeight);
        ruler=area.removeFromTop(learnTypography::secondaryLineHeight);
        dryChart=area.removeFromTop(area.getHeight()/2).withTrimmedBottom(contentPadding);
        wetChart=area;
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if(!event.mods.isLeftButtonDown() || !editable)return;
        if(overviewChart.contains(event.getPosition()) && zoomFraction>0)
        {
            navigating=true;
            navigate(event.position.x);
            return;
        }
        if(ruler.getWidth()<=0)return;
        const auto range=visibleRange();
        const auto hit=[&](int sample){return std::abs(event.position.x-xFor(sample,range))<=spacing::related;};
        size_t field=members.size();
        if(dryChart.contains(event.getPosition()))
        {
            const auto start=hit(view.edit.first),end=hit(view.edit.end);
            if(start || end)
                field=start && (!end || event.position.x<(xFor(view.edit.first,range)+xFor(view.edit.end,range))/2)?0:1;
        }
        else if(wetChart.contains(event.getPosition()) && view.wet && !view.wet->empty()
                && event.position.x>=xFor(view.wet->front().startSample,range)
                && event.position.x<=xFor(wetEnd(),range))
            field=2;
        if(field<members.size())
        {
            drag=Drag{field,view.binding,view.edit.*members[field],view.edit.*members[field],
                      event.position.x,(range.last-range.first)/ruler.getWidth()};
        }
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if(navigating){navigate(event.position.x);return;}
        if(!drag)return;
        const auto [low,high]=limits(drag->field);
        drag->proposed=juce::roundToInt(std::clamp(drag->initial+(event.position.x-drag->x)*drag->samplesPerPixel,
                                                  static_cast<double>(low),static_cast<double>(high)));
        repaint();
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        navigating=false;
        const auto completed=std::exchange(drag,std::nullopt);
        if(completed)setField(completed->field,completed->proposed,completed->binding);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        applyTextRole(g,*this,TextRole::sectionHeading);
        g.drawText("Swells",listHeading,juce::Justification::left);
        g.drawText("Selected Swell",heading,juce::Justification::left);
        g.drawText("Audition",auditionHeading,juce::Justification::left);
        g.drawText("Loop Playback",loopHeading,juce::Justification::left);
        g.drawText("Overview",overviewHeading,juce::Justification::left);
        g.setColour(separator);
        for(auto y:bands)g.drawHorizontalLine(y,0,static_cast<float>(getWidth()));
        g.drawLine(listDivider,rule);
        g.drawLine(inspectorDivider,rule);
        if (view.frames<=0 || view.rate<=0) return;
        std::vector<CapturePeak> source;
        for (int i=0;i<dry.size;++i)
        {
            const auto a=(i*view.frames+dry.size-1)/dry.size;
            const auto b=((i+1)*view.frames+dry.size-1)/dry.size;
            const auto& peak=dry.peaks[static_cast<size_t>(i)];
            source.push_back({a,static_cast<uint32_t>(b-a),peak.minimum,peak.maximum});
        }
        const auto [fullFirst,fullLast]=fullRange();
        const auto range=visibleRange();
        const auto [first,last]=range;
        const auto positionFor=[&](int64_t sample){return xFor(sample,range);};
        const auto zero=positionFor(0);
        applyTextRole(g,*this,TextRole::ruler);
        if(first!=0)g.drawText(seconds(first),ruler,juce::Justification::left);
        if(view.frames<=last)
            g.drawText("Loop end  "+juce::String(static_cast<double>(view.frames)/view.rate,3)+" s",ruler,juce::Justification::right);
        const auto draw=[&](const std::vector<CapturePeak>& peaks,juce::Rectangle<int> chart,juce::Colour colour,double from,double to,int64_t shift=0)
        {
            g.setColour(outline);g.drawRoundedRectangle(controlGeometry::insideStroke(chart,rule),radius,rule);
            uint64_t visited=0;
            const auto columns=waveformColumns(peaks,from,to,chart.getWidth(),shift,
                [](const CapturePeak& p){return std::pair{p.minimum,p.maximum};},visited,true);
            const auto centre=static_cast<float>(chart.getCentreY()),height=chart.getHeight()*.45f;
            g.setColour(colour);
            for (size_t i=0;i<columns.size();++i)
            {
                const auto& column=columns[i];
                if (!column.present) continue;
                const auto top=centre-std::clamp(column.maximum,-1.0f,1.0f)*height;
                const auto bottom=centre-std::clamp(column.minimum,-1.0f,1.0f)*height;
                const auto x=chart.getX()+static_cast<float>(i);
                g.drawLine(x,top,x,std::max(top+rule,bottom),rule);
            }
        };
        const auto draft=[&](size_t field){return drag && drag->field==field?drag->proposed:view.edit.*members[field];};
        const auto trimLine=dryChart.getBottom()-spacing::related;
        const auto sourceStart=positionFor(draft(0)),sourceEnd=positionFor(draft(1));
        const auto dotRadius=static_cast<float>(spacing::tight);
        {
            const juce::Graphics::ScopedSaveState scope(g);
            g.reduceClipRegion(dryChart);
            g.setColour(timing.withAlpha(.12f));
            g.fillRect(sourceStart,static_cast<float>(dryChart.getY()),sourceEnd-sourceStart,static_cast<float>(dryChart.getHeight()));
        }
        draw(source,dryChart,text,first,last);
        {
            const juce::Graphics::ScopedSaveState scope(g);
            g.reduceClipRegion(dryChart);
            g.setColour(timing);
            g.drawHorizontalLine(trimLine,sourceStart,sourceEnd);
            for(const auto x:{sourceStart,sourceEnd})
                g.drawEllipse(x-dotRadius,trimLine-dotRadius,dotRadius*2,dotRadius*2,rule);
        }
        if (view.wet)draw(*view.wet,wetChart,aqua,first,last,draft(2)-view.edit.placement);
        const auto half=overviewChart.getHeight()/2;
        draw(source,overviewChart.withHeight(half),text,fullFirst,fullLast);
        if(view.wet)draw(*view.wet,overviewChart.withTrimmedTop(half),aqua,fullFirst,fullLast);
        if(zoomFraction>0)
        {
            const auto overviewX=[&](double sample){return static_cast<float>(overviewChart.getX()+overviewChart.getWidth()*(sample-fullFirst)/(fullLast-fullFirst));};
            g.setColour(aqua);
            g.drawRect(juce::Rectangle<float>{overviewX(first),static_cast<float>(overviewChart.getY()),
                overviewX(last)-overviewX(first),static_cast<float>(overviewChart.getHeight())},rule);
        }
        if(first<=0 && last>0)
        {
            g.setColour(timing);
            g.drawVerticalLine(juce::roundToInt(zero),static_cast<float>(dryChart.getY()),static_cast<float>(wetChart.getBottom()));
            g.drawText("Loop start",ruler.withLeft(juce::roundToInt(zero)+spacing::tight),juce::Justification::left);
        }
        applyTextRole(g,*this,TextRole::helper);
        g.drawText("Dry",dryChart.reduced(spacing::related),juce::Justification::topLeft);
        g.drawText("Wet",wetChart.reduced(spacing::related),juce::Justification::topLeft);
        g.drawText("Lead-in  "+seconds(-fullFirst),readout,juce::Justification::left);
        const auto playhead=view.auditionPosition?view.auditionPosition:
            view.loopPosition?std::optional<int64_t>{*view.loopPosition}:std::nullopt;
        if (playhead && *playhead>=first && *playhead<last)
        {
            g.setColour(aqua);
            g.drawVerticalLine(juce::roundToInt(positionFor(*playhead)),static_cast<float>(dryChart.getY()),static_cast<float>(wetChart.getBottom()));
        }
        if(playhead && *playhead>=fullFirst && *playhead<fullLast)
        {
            const auto x=overviewChart.getX()+overviewChart.getWidth()*(*playhead-fullFirst)/(fullLast-fullFirst);
            g.setColour(aqua);
            g.drawVerticalLine(juce::roundToInt(x),static_cast<float>(overviewChart.getY()),static_cast<float>(overviewChart.getBottom()));
        }
    }

private:
    void showSection()
    {
        sections.display({{1,"Timing","Set the captured interval and move the wet swell."},
                          {2,"Envelope","Adjust the lead-in and fade out."},
                          {3,"Sound","Choose the reverb used for this swell."}},section);
        for(size_t i=0;i<knobs.size();++i)
        {
            const bool visible=section==(i<3?1:2);
            if(!visible && knobs[i].editing())knobs[i].cancel();
            knobs[i].setVisible(visible);
        }
        impulse.setVisible(section==3);
        impulseLabel.setVisible(section==3);
    }
    void zoomTo(double fraction)
    {
        zoomFraction=std::clamp(fraction,0.0,1.0);
        zoom.display({zoomFraction,zoomFraction<1,zoomFraction>0,true});
        repaint();
    }
    std::pair<int,int> limits(size_t field) const
    {
        switch(field)
        {
            case 0:return {0,std::max(0,view.edit.end-1)};
            case 1:return {std::min(view.frames,view.edit.first+1),view.frames};
            case 2:return {-view.frames,view.frames};
            case 3:return {0,view.frames};
            default:return {0,view.rate};
        }
    }
    bool setField(size_t field,double samples,uint64_t binding)
    {
        if(!editable || updating || !onEdit || binding!=view.binding || !std::isfinite(samples))return false;
        const auto [low,high]=limits(field);
        auto edit=view.edit;
        edit.*members[field]=juce::roundToInt(std::clamp(samples,static_cast<double>(low),static_cast<double>(high)));
        return onEdit(std::move(edit),binding);
    }
    void reset(bool all)
    {
        if(!editable || !onEdit || resetBinding!=view.binding)return;
        for(auto& knob:knobs)knob.cancel();
        auto edit=LoopPerformance::defaultEdit(view.frames,view.rate);
        if(!all){edit.first=view.edit.first;edit.end=view.edit.end;edit.impulse=view.edit.impulse;}
        onEdit(std::move(edit),resetBinding);
    }
    struct Range {double first,last;};
    int64_t wetEnd()const
    {const auto& last=view.wet->back();return last.startSample+last.frameCount;}
    Range fullRange() const
    {
        if(!view.wet || view.wet->empty())return {0,static_cast<double>(view.frames)};
        return {static_cast<double>(std::min<int64_t>(0,view.wet->front().startSample)),
                static_cast<double>(std::max<int64_t>(view.frames,wetEnd()))};
    }
    Range visibleRange()const
    {
        const auto [first,last]=fullRange();
        const auto span=std::max(1.0,(last-first)/std::pow(16.0,zoomFraction));
        const auto target=centre?first+*centre*(last-first):0.0;
        const auto start=std::clamp(target-span/2,first,last-span);
        return {start,start+span};
    }
    float xFor(int64_t sample,Range range)const
    {return static_cast<float>(ruler.getX()+ruler.getWidth()*(sample-range.first)/(range.last-range.first));}
    void navigate(float x)
    {
        if(overviewChart.getWidth()<=0)return;
        centre=std::clamp(static_cast<double>(x-overviewChart.getX())/overviewChart.getWidth(),0.0,1.0);
        repaint();
    }
    juce::String seconds(double sample) const {return juce::String(sample/view.rate,3)+" s";}
    struct KnobEdit {uint64_t binding;int sample,span;};
    struct Drag {size_t field;uint64_t binding;int initial,proposed;float x;double samplesPerPixel;};
    static constexpr std::array<const char*,5> fieldNames{"Source Start (ms)","Source End (ms)","Move Wet (ms)","Lead-in (ms)","Fade Out (ms)"};
    static constexpr std::array<const char*,5> fieldIds{"loopSourceStart","loopSourceEnd","loopPlacement","loopLead","loopFadeOut"};
    static constexpr std::array members{&LoopPerformance::Edit::first,&LoopPerformance::Edit::end,&LoopPerformance::Edit::placement,
                                        &LoopPerformance::Edit::lead,&LoopPerformance::Edit::fadeOut};
    controls::SwellList swells;
    controls::SegmentedSelector sections;
    controls::TimelineZoomControls zoom;
    std::array<KnobValue,5> knobs;
    controls::ChoiceSelector impulse;
    controls::ActionButton audition{"Audition",controls::CommandIcon::play,true},stop{"Stop",controls::CommandIcon::stop};
    controls::ActionButton cue{"Cue Loop",controls::CommandIcon::none,true},stopLoop{"Stop Loop"};
    controls::ActionButton resetShape{"Reset",controls::CommandIcon::none,controls::CommandVariant::destructive};
    controls::ActionButton resetAll{"Reset All",controls::CommandIcon::none,controls::CommandVariant::destructive};
    juce::Label impulseLabel,status,syncStatus,positionReadout;
    juce::Rectangle<int> heading,listHeading,auditionHeading,loopHeading,overviewHeading,overviewChart,readout,ruler,dryChart,wetChart;
    juce::Line<float> listDivider,inspectorDivider;
    std::array<int,3> bands{};
    LoopPerformance::View view;
    LoopCapture::Waveform dry;
    std::array<std::optional<KnobEdit>,5> knobEdits{};
    std::optional<Drag> drag;
    uint64_t impulseBinding=0,auditionBinding=0,cueBinding=0,resetBinding=0;
    std::optional<double> centre;
    double zoomFraction=0;
    int section=1;
    bool updating=false,editable=false,navigating=false;
};
}
