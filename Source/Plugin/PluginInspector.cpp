#include "PluginEditor.h"
#include <UI/MusicalPosition.h>
#include <cmath>

namespace
{
bool sampleCount(long double value,int64_t& result)
{
    if(!std::isfinite(value) || value<=-9223372036854775808.0L || value>=9223372036854775808.0L)return false;
    result=static_cast<int64_t>(std::llround(value));return true;
}
bool decimal(const juce::String& text,double& value)
{
    const auto s=text.trim();if(s.isEmpty() || !s.containsOnly("-0123456789.") || s=="-" || s=="." || s=="-."
        || s.lastIndexOfChar('-')>0 || s.indexOfChar('.')!=s.lastIndexOfChar('.'))return false;
    value=s.getDoubleValue();return std::isfinite(value);
}
}
void PreverbAudioProcessorEditor::beginValueEdit(int index)
{
    const auto current=processor.swellEngine().snapshot();const auto w=m3::find(current.session,current.session.primary);
    valueAnchors[index]=w?std::optional<m3::Swell>(*w):std::nullopt;valueTime=processor.getSongTime();
    valueTokens[index]=w?w->inputGeneration:0;
    valueRawStarts[index]=current.raw?current.raw->start:0;valueRawEnds[index]=current.raw?current.raw->end:0;
}
bool PreverbAudioProcessorEditor::commitValueEdit(int index,const juce::String& text,std::optional<double> delta)
{
    if(updating || destroying || !valueAnchors[index])return false;
    const auto anchor=*valueAnchors[index];const auto current=processor.swellEngine().savedSession();
    const auto latest=m3::find(current,anchor.id);
    if(current.primary!=anchor.id || !latest)return false;
    if(latest->inputGeneration!=valueTokens[index] || !sameMusicalMapping(valueTime,processor.getSongTime()))
    {processor.swellEngine().diagnostic("Edit superseded by changed source or host timing; try again");return false;}
    const auto time=valueTime;const auto rate=time.rate;int64_t value=0;double scalar=0;
    if(!time.valid || !std::isfinite(rate) || rate<=0){processor.swellEngine().diagnostic("Compatible host timing is required");return false;}
    if(delta)
    {
        if(*delta==0)return true;
        const auto displacement=static_cast<long double>(*delta)*valueSpans[index];int64_t offset=0;
        if(index<=sourceEndValue)
        {
            const auto initial=index==targetValue?anchor.arrival:index==sourceStartValue?anchor.sourceStart:anchor.sourceEnd;
            if(!sampleCount(displacement,offset) || !m3::checkedAdd(initial,offset,value)){processor.swellEngine().diagnostic("Sample position overflow");return false;}
        }
        else if(index==levelValue)scalar=std::clamp(anchor.shape.gainDb+static_cast<double>(displacement),-60.0,12.0);
        else if(index==inCurveValue || index==outCurveValue)scalar=std::clamp((index==inCurveValue?anchor.shape.inTension:anchor.shape.outTension)+static_cast<double>(displacement),-1.0,1.0);
        else
        {
            int64_t initial=0,a=0,b=0;
            if(index==placementValue)initial=anchor.placement;
            else if(index==fadeInValue || index==fadeOutValue)initial=index==fadeInValue?anchor.shape.fadeIn:anchor.shape.fadeOut;
            else if(!m3::requestedShapeBounds(anchor,valueRawStarts[index],a,b)
                || !(index==preRollValue?m3::checkedSubtract(anchor.arrival,a,initial):m3::checkedSubtract(b,anchor.arrival,initial)))return false;
            if(!sampleCount(displacement,offset) || !m3::checkedAdd(initial,offset,value)){processor.swellEngine().diagnostic("Timing value overflow");return false;}
            if(index==fadeInValue || index==fadeOutValue)value=std::max(int64_t{0},value);
        }
    }
    else if(index<=sourceEndValue || (index==placementValue && sampleUnitsButton.getToggleState()))
    {
        if(sampleUnitsButton.getToggleState())
        {value=text.getLargeIntValue();if(juce::String(value)!=text){processor.swellEngine().diagnostic("Enter an exact integer sample value");return false;}}
        else if(!m3::parseMusicalPosition(text,time,value)){processor.swellEngine().diagnostic("Enter bar.beat.fraction or choose exact samples in Settings");return false;}
    }
    else
    {
        if(!decimal(text,scalar)){processor.swellEngine().diagnostic("Enter a finite numeric value");return false;}
        if(index!=levelValue && index!=inCurveValue && index!=outCurveValue
            && !sampleCount(static_cast<long double>(scalar)*rate/(index==preRollValue || index==endOffsetValue?1:1000),value))
        {processor.swellEngine().diagnostic("Timing value overflow");return false;}
    }
    const auto ok=processor.swellEngine().edit(anchor.id,[&](auto& w,auto& error)
    {
        if(index==targetValue)return m3::moveArrival(w,value,error);
        if(index==sourceStartValue)return m3::setSource(w,value,w.sourceEnd,error);
        if(index==sourceEndValue)return m3::setSource(w,w.sourceStart,value,error);
        if(index==placementValue)return m3::moveWet(w,value,error);
        auto shape=w.shape;
        if(index==levelValue){if(scalar<-60 || scalar>12){error="Level must be -60 to +12 dB";return false;}shape.gainDb=static_cast<float>(scalar);}
        if(index==inCurveValue || index==outCurveValue)
        {if(std::abs(scalar)>1){error="Curve tension must be -1 to +1";return false;}(index==inCurveValue?shape.inTension:shape.outTension)=scalar;}
        if(index==preRollValue){int64_t positioned,logical;
            if(!m3::checkedSubtract(w.arrival,value,positioned) || !m3::checkedSubtract(positioned,w.placement,logical)
                || !m3::checkedSubtract(logical,w.arrival,shape.startOffset)){error="Wet start overflow";return false;}shape.automaticStart=false;}
        if(index==endOffsetValue && !m3::checkedSubtract(value,w.placement,shape.endOffset)){error="Wet endpoint overflow";return false;}
        if(index==fadeInValue || index==fadeOutValue)
        {if(value<0){error="Fade length cannot be negative";return false;}(index==fadeInValue?shape.fadeIn:shape.fadeOut)=value;}
        return m3::setShape(w,shape,valueRawStarts[index],valueRawEnds[index],error);
    });
    // Refresh is listener-suppressed. The cell keeps its typed text/gesture anchor
    // until completion, while the rest of the editor shows actual canonical state.
    if(ok){const auto saved=processor.swellEngine().savedSession();if(const auto w=m3::find(saved,anchor.id))valueTokens[index]=w->inputGeneration;}
    refresh();return ok;
}
void PreverbAudioProcessorEditor::refreshValues(const m3::Swell* w,const SongTime& time)
{
    const auto rate=time.rate;const auto valid=w && time.valid && std::isfinite(rate) && rate>0;
    const auto span=std::max(1.0,timeline.viewport().span());
    int64_t a=0,b=0;const auto shaped=valid && state.raw && m3::requestedShapeBounds(*w,state.raw->start,a,b);
    for(int index=0;index<valueCount;++index)
    {
        if(index<=sourceEndValue)valueControls[index].setCaption(juce::String(index==targetValue?"Target":index==sourceStartValue?"Source Start":"Source End")+(sampleUnitsButton.getToggleState()?" (samples)":""));
        if(index==placementValue)valueControls[index].setCaption(sampleUnitsButton.getToggleState()?"Move Wet (samples)":"Move Wet (ms)");
        const auto enabled=valid && (index<=sourceEndValue?(!w->frozen):shaped);
        juce::String text;double fraction=.5,domain=span;
        if(valid)
        {
            if(index<=sourceEndValue)
            {
                const auto sample=index==targetValue?w->arrival:index==sourceStartValue?w->sourceStart:w->sourceEnd;
                text=sampleUnitsButton.getToggleState()?juce::String(sample):m3::musicalPosition(sample,time);
                fraction=static_cast<double>((static_cast<long double>(sample)-timeline.viewport().first)/span);
            }
            else if(index==levelValue){text=juce::String(w->shape.gainDb,1);fraction=(w->shape.gainDb+60)/72;domain=72;}
            else if(index==inCurveValue || index==outCurveValue)
            {const auto tension=index==inCurveValue?w->shape.inTension:w->shape.outTension;text=juce::String(tension,3);fraction=(tension+1)/2;domain=2;}
            else
            {
                long double samples=0;
                if(index==preRollValue)samples=static_cast<long double>(w->arrival)-a;
                if(index==endOffsetValue)samples=static_cast<long double>(b)-w->arrival;
                if(index==fadeInValue || index==fadeOutValue)samples=static_cast<long double>(index==fadeInValue?w->shape.fadeIn:w->shape.fadeOut);
                if(index==placementValue)samples=static_cast<long double>(w->placement);
                text=index==placementValue && sampleUnitsButton.getToggleState()?juce::String(w->placement):
                    juce::String(static_cast<double>(samples/rate*(index==preRollValue || index==endOffsetValue?1:1000)),3);
                if(index==fadeInValue || index==fadeOutValue){domain=shaped?std::max(1.0,static_cast<double>(static_cast<long double>(b)-a-1)):span;fraction=static_cast<double>(samples/domain);}
                else {domain=std::max(span,std::abs(static_cast<double>(samples))*2+1);fraction=.5+static_cast<double>(samples)/domain;}
            }
        }
        if(!valueControls[index].editing())valueSpans[index]=domain;
        valueControls[index].display(w?w->id:juce::String{},text,fraction,enabled);
    }
}
