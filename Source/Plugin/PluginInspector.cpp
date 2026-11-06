#include "PluginEditor.h"
#include <UI/MusicalPosition.h>
#include <UI/TimingEdit.h>
#include <UI/PerformText.h>
#include <UI/LevelAdjustment.h>
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
    const auto trimmed=text.trim();const auto s=trimmed.startsWithChar('+')?trimmed.substring(1):trimmed;
    if(s.isEmpty() || !s.containsOnly("-0123456789.") || s=="-" || s=="." || s=="-."
        || s.lastIndexOfChar('-')>0 || s.indexOfChar('.')!=s.lastIndexOfChar('.'))return false;
    value=s.getDoubleValue();return std::isfinite(value);
}
}
void PreverbAudioProcessorEditor::enableFocusedLevelReview()
{
    for(auto& cell:valueControls)cell.cancel();focusedLevelReview=true;
    auto& cell=valueControls[levelValue];
    cell.valueKnob().setTooltip(m3::ui::levelAdjustment::help);cell.valueField().setTooltip(m3::ui::levelAdjustment::help);
    cell.onAbsolute=[this](double db){commitValueEdit(levelValue,m3::ui::preciseValue(db));};refresh();
}
void PreverbAudioProcessorEditor::beginValueEdit(int index)
{
    ++valueResetRevision;
    valueWorkflowTokens[index]=processor.getWorkflowSnapshot().pass;
    const auto current=processor.swellEngine().snapshot();const auto w=m3::find(current.session,current.session.primary);
    valueAnchors[index]=w?std::optional<m3::Swell>(*w):std::nullopt;
    if(index==levelValue)levelGestureMinimum=focusedLevelReview && w?m3::ui::levelAdjustment::gestureMinimum(w->shape.gainDb):m3::minimumSwellLevelDb;
    valueInvocations[index]={current.sessionEpoch,current.session.selected,processor.getSongTime(),current.session.timing,
        processor.getCaptureSnapshot().manifest.sampleRate,processor.getHostPositionDiagnostics().samplePositionAvailable,w?w->render.content:juce::String{}};
    valueInvocations[index].clock=m3::gridClockContext(current.gridClock,processor.getHostPositionDiagnostics().isPlaying);
    valueTokens[index]=w?w->inputGeneration:0;
    valueRawStarts[index]=current.raw?current.raw->start:0;valueRawEnds[index]=current.raw?current.raw->end:0;
}
m3::ui::KnobValue::ResetAction PreverbAudioProcessorEditor::prepareValueReset(int index)
{
    const auto current=processor.swellEngine().snapshot();
    if(destroying || updating || current.sessionEpoch!=state.sessionEpoch || current.session.primary!=state.session.primary || current.session.selected!=state.session.selected)
        return {"Reset unavailable: selection changed",{}};
    const auto initial=initialValuePositions.find(current.session.primary);if(initial==initialValuePositions.end())return {"Reset unavailable",{}};
    beginValueEdit(index);if(!valueAnchors[index] || !valueIntentCurrent(index))return {"Reset unavailable",{}};
    const auto anchor=*valueAnchors[index];const auto invocation=valueInvocations[index];const auto input=valueTokens[index],pass=valueWorkflowTokens[index],revision=valueResetRevision;
    const auto rawStart=valueRawStarts[index],rawEnd=valueRawEnds[index];const auto positions=initial->second;
    const auto label=index<=sourceEndValue?"Reset to initial position":index==preRollValue?"Reset to automatic start":
        index==levelValue?"Reset to default (0 dB)":index==inCurveValue || index==outCurveValue?"Reset tension to default (0)":
        index==endOffsetValue?"Reset to default (0.020 s)":index==fadeOutValue?"Reset to default (20 ms)":"Reset to default (0 ms)";
    const juce::Component::SafePointer<PreverbAudioProcessorEditor> safe(this);
    return {label,[safe,index,anchor,invocation,input,pass,revision,rawStart,rawEnd,positions]
    {
        if(!safe || safe->destroying || safe->valueResetRevision!=revision)return;
        safe->valueAnchors[index]=anchor;safe->valueInvocations[index]=invocation;safe->valueTokens[index]=input;safe->valueWorkflowTokens[index]=pass;
        safe->valueRawStarts[index]=rawStart;safe->valueRawEnds[index]=rawEnd;
        safe->resetValueEdit(index,positions);safe->valueAnchors[index].reset();
    }};
}
void PreverbAudioProcessorEditor::resetValueEdit(int index,InitialValuePositions initial)
{
    if(!valueIntentCurrent(index)){processor.swellEngine().diagnostic("Reset superseded by changed session, selection, source or timing");refresh();return;}
    const auto anchor=*valueAnchors[index];const auto invocation=valueInvocations[index];
    processor.swellEngine().edit(anchor.id,[&](auto& w,auto& error)
    {
        const auto current=processor.swellEngine().savedSession();
        if(current.primary!=anchor.id || current.selected!=invocation.selected || w.inputGeneration!=valueTokens[index]){error="Reset selection/source changed";return false;}
        if(index==targetValue)return w.arrival!=initial.arrival && m3::moveArrival(w,initial.arrival,error);
        if(index==sourceStartValue)return w.sourceStart!=initial.start && m3::setSource(w,initial.start,w.sourceEnd,error);
        if(index==sourceEndValue)return w.sourceEnd!=initial.end && m3::setSource(w,w.sourceStart,initial.end,error);
        if(index==placementValue)return w.placement!=0 && m3::moveWet(w,0,error);
        const auto defaults=m3::initialShape(invocation.time.rate);auto shape=w.shape;
        if(index==levelValue)shape.gainDb=defaults.gainDb;
        if(index==inCurveValue)shape.inTension=defaults.inTension;
        if(index==outCurveValue)shape.outTension=defaults.outTension;
        if(index==preRollValue){shape.automaticStart=true;shape.startOffset=0;}
        if(index==endOffsetValue && !m3::checkedSubtract(defaults.endOffset,w.placement,shape.endOffset)){error="Reset wet endpoint overflow";return false;}
        if(index==fadeInValue)shape.fadeIn=defaults.fadeIn;
        if(index==fadeOutValue)shape.fadeOut=defaults.fadeOut;
        auto candidate=w;if(!m3::setShape(candidate,shape,valueRawStarts[index],valueRawEnds[index],error))return false;
        if(candidate.shape==w.shape && candidate.supportOffset==w.supportOffset)return false;
        w=std::move(candidate);return true;
    },invocation.epoch);
    refresh();
}
bool PreverbAudioProcessorEditor::valueIntentCurrent(int index)const
{
    if(!valueAnchors[index])return false;const auto& invocation=valueInvocations[index];
    const auto flow=processor.getWorkflowSnapshot();const auto current=processor.swellEngine().savedSession();const auto latest=m3::find(current,valueAnchors[index]->id);
    const auto currentClock=processor.swellEngine().transportClockSnapshot();
    const auto gridClock=m3::gridClockContext(currentClock,processor.getHostPositionDiagnostics().isPlaying);
    return flow.pass==valueWorkflowTokens[index] && flow.mode==workflow::Mode::live && invocation.epoch==processor.swellEngine().assignmentEpoch()
        && invocation.clock.sameAuthority(gridClock)
        && current.primary==valueAnchors[index]->id && current.selected==invocation.selected && latest && latest->inputGeneration==valueTokens[index]
        && latest->frozen==valueAnchors[index]->frozen && transportMappingMatches(invocation.time,processor.getSongTime())
        && invocation.accepted.valid==current.timing.valid && (!invocation.accepted.valid || transportMappingMatches(invocation.accepted,current.timing))
        && processor.getHostPositionDiagnostics().samplePositionAvailable==invocation.sampleAvailable
        && (index<=sourceEndValue || latest->render.content==invocation.render);
}
void PreverbAudioProcessorEditor::rebaseValueEdit(int index)
{
    if(!valueIntentCurrent(index)){valueControls[index].cancel();return;}
    const auto current=processor.swellEngine().savedSession();valueAnchors[index]=*m3::find(current,current.primary);
}
void PreverbAudioProcessorEditor::stepValueEdit(int index,double steps)
{
    if(!valueIntentCurrent(index) || !std::isfinite(steps) || steps==0 || !valueAnchors[index])return;
    const auto& anchor=*valueAnchors[index];const auto& invocation=valueInvocations[index];
    const bool fine=(juce::ModifierKeys::shiftModifier&valueModifierFlags)!=0;
    const auto direction=steps>0?1:-1;
    double displacement=steps*(fine?.1:1.);
    if(index==levelValue)displacement=steps*(fine?m3::swellLevelFineStepDb:m3::swellLevelStepDb);
    if(index==inCurveValue || index==outCurveValue)displacement*=.01;
    else if(index!=levelValue)
    {
        const auto kind=index==targetValue?m3::TimingAnchor::arrival:index==sourceStartValue?m3::TimingAnchor::sourceStart:index==sourceEndValue?m3::TimingAnchor::sourceEnd:
            index==placementValue?m3::TimingAnchor::move:index==preRollValue?m3::TimingAnchor::start:index==endOffsetValue?m3::TimingAnchor::end:index==fadeInValue?m3::TimingAnchor::fadeIn:m3::TimingAnchor::fadeOut;
        const auto sign=index==preRollValue || index==fadeOutValue?-1:1;
        if(timeline.effectiveSnap())
        {
            const auto initial=m3::timingAnchorSample(anchor,kind,valueRawStarts[index]);
            const auto grid=m3::HostGrid::make(invocation.accepted,invocation.time,invocation.rate,timeline.selectedGrid(),nullptr,invocation.sampleAvailable,&invocation.clock);
            const auto next=initial && grid?grid->adjacent(*initial,sign*direction):std::optional<int64_t>{};
            if(!next){processor.swellEngine().diagnostic("Snap unavailable: no next point in compatible fixed host timing");return;}
            displacement=static_cast<double>((static_cast<long double>(*next)-*initial)*sign);
        }
        else displacement=direction*std::max(1.,std::round(std::abs(displacement)*invocation.time.rate/1000.));
    }
    commitValueEdit(index,{},displacement/valueSpans[index]);
}
bool PreverbAudioProcessorEditor::commitValueEdit(int index,const juce::String& text,std::optional<double> delta)
{
    if(updating || destroying || !valueAnchors[index])return false;
    if(!valueIntentCurrent(index)){processor.swellEngine().diagnostic("Edit superseded by changed session, selection, source or fixed host timing");valueControls[index].cancel();refresh();return false;}
    const auto anchor=*valueAnchors[index];const auto current=processor.swellEngine().savedSession();
    const auto latest=m3::find(current,anchor.id);
    if(current.primary!=anchor.id || !latest)return false;
    if(latest->inputGeneration!=valueTokens[index])
    {processor.swellEngine().diagnostic("Edit superseded by changed source or host timing; try again");return false;}
    const auto invocation=valueInvocations[index];const auto time=invocation.time;const auto rate=time.rate;int64_t value=0;double scalar=0;
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
        else if(index==levelValue)scalar=std::clamp(anchor.shape.gainDb+static_cast<double>(displacement),levelGestureMinimum,static_cast<double>(m3::maximumSwellLevelDb));
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
        else if(!m3::parseMusicalPosition(text,invocation.accepted.valid?invocation.accepted:time,value)){processor.swellEngine().diagnostic("Enter bar.beat.fraction or choose exact samples in Settings");return false;}
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
        const auto accepted=processor.swellEngine().savedSession();
        if(accepted.primary!=anchor.id || accepted.selected!=invocation.selected || w.inputGeneration!=valueTokens[index]){error="Selection/source changed; start a new edit";return false;}
        if(delta && index!=levelValue && index!=inCurveValue && index!=outCurveValue)
        {
            const auto kind=index==targetValue?m3::TimingAnchor::arrival:index==sourceStartValue?m3::TimingAnchor::sourceStart:index==sourceEndValue?m3::TimingAnchor::sourceEnd:
                index==placementValue?m3::TimingAnchor::move:index==preRollValue?m3::TimingAnchor::start:index==endOffsetValue?m3::TimingAnchor::end:index==fadeInValue?m3::TimingAnchor::fadeIn:m3::TimingAnchor::fadeOut;
            const auto initial=m3::timingAnchorSample(anchor,kind,valueRawStarts[index]);int64_t change,requested;
            if(!initial || !sampleCount(static_cast<long double>(*delta)*valueSpans[index]*(index==preRollValue || index==fadeOutValue?-1:1),change) || !m3::checkedAdd(*initial,change,requested)){error="Timing candidate overflow";return false;}
            const auto grid=m3::HostGrid::make(invocation.accepted,time,invocation.rate,timeline.selectedGrid(),nullptr,invocation.sampleAvailable,&invocation.clock);long double origin=0;songOrigin(invocation.accepted,origin);
            return m3::applyTimingCandidate(w,anchor,kind,requested,valueRawStarts[index],valueRawEnds[index],timeline.effectiveSnap(),grid?&*grid:nullptr,origin,error);
        }
        if(index==targetValue)return m3::moveArrival(w,value,error);
        if(index==sourceStartValue)return m3::setSource(w,value,w.sourceEnd,error);
        if(index==sourceEndValue)return m3::setSource(w,w.sourceStart,value,error);
        if(index==placementValue)return m3::moveWet(w,value,error);
        auto shape=w.shape;
        if(index==levelValue){if(scalar<m3::minimumSwellLevelDb || scalar>m3::maximumSwellLevelDb){error="Level must be -60 to +12 dB";return false;}shape.gainDb=static_cast<float>(scalar);}
        if(index==inCurveValue || index==outCurveValue)
        {if(std::abs(scalar)>1){error="Curve tension must be -1 to +1";return false;}(index==inCurveValue?shape.inTension:shape.outTension)=scalar;}
        if(index==preRollValue){int64_t positioned,logical;
            if(!m3::checkedSubtract(w.arrival,value,positioned) || !m3::checkedSubtract(positioned,w.placement,logical)
                || !m3::checkedSubtract(logical,w.arrival,shape.startOffset)){error="Wet start overflow";return false;}shape.automaticStart=false;}
        if(index==endOffsetValue && !m3::checkedSubtract(value,w.placement,shape.endOffset)){error="Wet endpoint overflow";return false;}
        if(index==fadeInValue || index==fadeOutValue)
        {if(value<0){error="Fade length cannot be negative";return false;}(index==fadeInValue?shape.fadeIn:shape.fadeOut)=value;}
        return m3::setShape(w,shape,valueRawStarts[index],valueRawEnds[index],error);
    },invocation.epoch);
    // Refresh is listener-suppressed. The cell keeps its typed text/gesture anchor
    // until completion, while the rest of the editor shows actual canonical state.
    if(ok){const auto saved=processor.swellEngine().savedSession();if(const auto w=m3::find(saved,anchor.id))valueTokens[index]=w->inputGeneration;}
    refresh();return ok;
}
void PreverbAudioProcessorEditor::refreshValues(const m3::Swell* w,const SongTime& time)
{
    if(initialValueEpoch!=state.sessionEpoch){initialValuePositions.clear();initialValueEpoch=state.sessionEpoch;}
    if(w)initialValuePositions.try_emplace(w->id,InitialValuePositions{w->arrival,w->sourceStart,w->sourceEnd});
    const auto rate=time.rate;const auto valid=w && time.valid && std::isfinite(rate) && rate>0;
    const auto span=std::max(1.0,timeline.viewport().span());
    int64_t a=0,b=0;const auto shaped=valid && state.raw && m3::requestedShapeBounds(*w,state.raw->start,a,b);
    for(int index=0;index<valueCount;++index)
    {
        if(index<=sourceEndValue)valueControls[index].setCaption(juce::String(index==targetValue?"Target":index==sourceStartValue?"Source start":"Source end")+(sampleUnitsButton.getToggleState()?" (samples)":" (bars)"),
            juce::String(index==targetValue?"Target":index==sourceStartValue?"Source Start":"Source End")+(sampleUnitsButton.getToggleState()?" (samples)":" (bar.beat.fraction)"));
        if(index==placementValue)valueControls[index].setCaption(sampleUnitsButton.getToggleState()?"Move Wet (samples)":"Move Wet (ms)");
        if(index==inCurveValue || index==outCurveValue)valueControls[index].setCaption(index==inCurveValue?"Curve In":"Curve Out",index==inCurveValue?"Curve In (unitless, -1 to +1)":"Curve Out (unitless, -1 to +1)");
        const auto enabled=valid && (index<=sourceEndValue?(!w->frozen):shaped);
        const auto valueAvailable=valid && ((index!=preRollValue && index!=endOffsetValue) || shaped);
        juce::String text,exactText;double fraction=.5,domain=span;
        if(valueAvailable)
        {
            if(index<=sourceEndValue)
            {
                const auto sample=index==targetValue?w->arrival:index==sourceStartValue?w->sourceStart:w->sourceEnd;
                text=sampleUnitsButton.getToggleState()?juce::String(sample):m3::musicalPosition(sample,time);
                exactText=text;fraction=static_cast<double>((static_cast<long double>(sample)-timeline.viewport().first)/span);
            }
            else if(index==levelValue){text=juce::String(w->shape.gainDb,2);exactText=m3::ui::preciseValue(w->shape.gainDb);
                domain=focusedLevelReview?m3::ui::levelAdjustment::spanDb:m3::maximumSwellLevelDb-m3::minimumSwellLevelDb;
                fraction=focusedLevelReview?m3::ui::levelAdjustment::fraction(w->shape.gainDb):(w->shape.gainDb-m3::minimumSwellLevelDb)/domain;}
            else if(index==inCurveValue || index==outCurveValue)
            {const auto tension=index==inCurveValue?w->shape.inTension:w->shape.outTension;text=juce::String(tension,3);exactText=m3::ui::preciseValue(tension);fraction=(tension+1)/2;domain=2;}
            else
            {
                long double samples=0;
                if(index==preRollValue)samples=static_cast<long double>(w->arrival)-a;
                if(index==endOffsetValue)samples=static_cast<long double>(b)-w->arrival;
                if(index==fadeInValue || index==fadeOutValue)samples=static_cast<long double>(index==fadeInValue?w->shape.fadeIn:w->shape.fadeOut);
                if(index==placementValue)samples=static_cast<long double>(w->placement);
                text=index==placementValue && sampleUnitsButton.getToggleState()?juce::String(w->placement):
                    juce::String(static_cast<double>(samples/rate*(index==preRollValue || index==endOffsetValue?1:1000)),index==preRollValue || index==endOffsetValue?6:3);
                exactText=index==placementValue && sampleUnitsButton.getToggleState()?juce::String(w->placement):m3::ui::preciseValue(static_cast<double>(samples/rate*(index==preRollValue || index==endOffsetValue?1:1000)));
                if(index==fadeInValue || index==fadeOutValue){domain=shaped?std::max(1.0,static_cast<double>(static_cast<long double>(b)-a-1)):span;fraction=static_cast<double>(samples/domain);}
                else {domain=std::max(span,std::abs(static_cast<double>(samples))*2+1);fraction=.5+static_cast<double>(samples)/domain;}
            }
        }
        if(valueAvailable)
        {
            if(index<=sourceEndValue && !sampleUnitsButton.getToggleState())text=m3::ui::musicalReadout(index==targetValue?w->arrival:index==sourceStartValue?w->sourceStart:w->sourceEnd,time);
            else if(index>sourceEndValue && !(index==placementValue && sampleUnitsButton.getToggleState()))text=m3::ui::compactValue(exactText.getDoubleValue(),index==preRollValue || index==endOffsetValue?3:index==inCurveValue || index==outCurveValue?2:1);
        }
        if(!valueControls[index].editing())valueSpans[index]=domain;
        valueControls[index].display(w?juce::String(state.sessionEpoch)+"/"+w->id:juce::String{},text,fraction,enabled,exactText);
        const auto musical=index<=sourceEndValue && !sampleUnitsButton.getToggleState();long double origin=0;songOrigin(time,origin);
        const auto beat=rate*60/time.bpm*4/time.denominator;
        const auto units=index<=sourceEndValue?(musical?beat:1.):index==placementValue && sampleUnitsButton.getToggleState()?1.:index==preRollValue || index==endOffsetValue?rate:rate/1000.;
        double domainValue=0,minimum=-9223372036854775808./units,maximum=std::nextafter(9223372036854775808.,0.)/units,step=rate/1000./units,unitsPerFraction=domain/units;
        if(valueAvailable)
        {
            if(index<=sourceEndValue){const auto sample=index==targetValue?w->arrival:index==sourceStartValue?w->sourceStart:w->sourceEnd;
                domainValue=static_cast<double>((static_cast<long double>(sample)-(musical?origin:0))/units);
                if(index==sourceStartValue)maximum=static_cast<double>((static_cast<long double>(w->arrival)-(musical?origin:0))/units);
                if(index==sourceEndValue)minimum=static_cast<double>((static_cast<long double>(w->arrival)+1-(musical?origin:0))/units);}
            else if(index==levelValue){domainValue=w->shape.gainDb;minimum=m3::minimumSwellLevelDb;maximum=m3::maximumSwellLevelDb;step=m3::swellLevelStepDb;unitsPerFraction=domain;}
            else if(index==inCurveValue || index==outCurveValue){domainValue=index==inCurveValue?w->shape.inTension:w->shape.outTension;minimum=-1;maximum=1;step=.01;unitsPerFraction=2;}
            else {domainValue=index==placementValue?w->placement:index==preRollValue?static_cast<double>(w->arrival)-a:index==endOffsetValue?static_cast<double>(b)-w->arrival:index==fadeInValue?w->shape.fadeIn:w->shape.fadeOut;domainValue/=units;
                if(index==fadeInValue || index==fadeOutValue){minimum=0;maximum=static_cast<double>(b-a-1)/units;}}
        }
        valueControls[index].displayDomain({domainValue,minimum,maximum,step,unitsPerFraction,exactText});
    }
}
