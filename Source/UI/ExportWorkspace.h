#pragma once
#include "WorkspaceTheme.h"
#include <Core/TransportTime.h>
#include "Controls/ActionButton.h"
#include "Controls/FileDragTile.h"
#include "Controls/ExternalFileDrag.h"
#include "Controls/ExportLevelsControl.h"
#include "Controls/NoticeBar.h"
#include "Controls/TimelineNavigation.h"
#include "Controls/TimelineZoomControls.h"
#include "Controls/MusicalTimelineRuler.h"
#include "LoopExportControls.h"
#include <DSP/SwellEngine.h>
#include <cmath>
#include <limits>

namespace m3::ui
{
class ExportWorkspace final:public juce::Component
{
public:
    explicit ExportWorkspace(SwellEngine& owner):engine(owner)
    {
        setName("exportWorkspace");
        for(auto* c:std::initializer_list<juce::Component*>{&play,&stop,&mix,&tile,&save,&back,&zoom,&navigation})addAndMakeVisible(c);
        addChildComponent(loopControls);addChildComponent(placement);
        applyTextRole(placement,TextRole::helper,juce::Justification::topLeft);placement.setBorderSize({});
        loopControls.beforeInvocation=[this]{loopInvocation=std::pair{state.session,state.pass};};
        loopControls.onChange=[this](auto settings){
            if(loopPresentation && loopInvocation && loopInvocation->first==state.session && loopInvocation->second==state.pass && onLoopSettings)
                onLoopSettings(settings,state.session,state.pass);
            if(loopPresentation)loopControls.display(loopPresentation->settings);
        };
        zoom.setName("exportZoom");zoom.fitButton().setName("exportFit");navigation.setName("exportNavigation");
        zoom.beforeInvocation=[this]{zoomInvocation.reset();if(displayed)zoomInvocation=ZoomScope{state.session,state.pass,displayed->plan->identity};};
        zoom.onZoomStep=[this](double factor){if(zoomIntentCurrent())navigation.zoom(factor);};
        zoom.onZoomFraction=[this](double fraction){if(zoomIntentCurrent()){
            const auto low=navigation.minimumSpan(),high=navigation.maximumSpan();
            if(std::isfinite(fraction) && low>0 && high>low)
                navigation.zoom(high*std::pow(low/high,std::clamp(fraction,0.,1.))/navigation.viewport().span());}};
        zoom.onFit=[this]{if(zoomIntentCurrent())navigation.fit();};
        navigation.onChange=[this]{requestDetail();updateZoom();repaint();};
        navigation.onPosition=[this](int64_t sample){if(state.prepared && engine.seekExport(sample,state.session,state.pass,state.prepared->plan->identity)){
            state.previewPosition=std::clamp(sample,*state.prepared->metrics.audibleStart,*state.prepared->metrics.audibleEnd-1);state.cursor.reset();repaint();}};
        navigation.setPanGesture(controls::TimelineNavigation::PanGesture::rightDrag);
        navigation.setTooltip("Left-click or drag: position preview. Right-drag: pan. Wheel: zoom. Shift-wheel: pan. Home: Fit.");
        play.setName("exportPlay");stop.setName("exportStop");mix.setName("exportLevels");tile.setName("exportDrag");save.setName("exportSave");back.setName("exportReturn");
        setTitle("Song-aligned export preview");mix.setTitle("Export mix");play.setTitle("Play export preview");stop.setTitle("Stop export preview");
        play.setTooltip("Preview from the positioned playhead, or the first audible sample by default");stop.setTooltip("Stop internal preview; DAW transport is unchanged");
        mix.beforeInvocation=[this]{mixSession=state.session;mixPass=state.pass;};
        mix.onChange=[this](controls::ExportLevelsControl::Pair pair){
            if(loopPresentation){
                if(!refreshing && mixSession==state.session && mixPass==state.pass && onLoopLevels)
                    onLoopLevels(pair,state.session,state.pass);
                mix.display(requestedLevels);return;
            }
            if(!refreshing)engine.setExportLevels(pair.dry,pair.wet,mixSession.value_or(engine.assignmentEpoch()),mixPass.value_or(state.pass));
            display(engine.snapshot());
        };
        play.onClick=[this]{engine.playExport(engine.assignmentEpoch(),loopPresentation?deviceRate:0);};stop.onClick=[this]{engine.stop();};
        save.onClick=tile.onSave=[this]{chooseSave();};back.onClick=[this]{if(onReturn)onReturn();};
        tile.onDrag=[this]{if(state.prepared)dragRequest=engine.deliverExport(engine.assignmentEpoch(),state.prepared->file,{},false,true);};
    }
    std::function<void()> onReturn;
    struct LoopPresentation
    {
        LoopExportControls::Settings settings;
        double periodFrames=0,barFrames=0;
        int beatsPerBar=4;
        bool beatAvailable=true,meterAvailable=true;
    };
    std::function<void(LoopExportControls::Settings,uint64_t,uint64_t)> onLoopSettings;
    std::function<void(controls::ExportLevelsControl::Pair,uint64_t,uint64_t)> onLoopLevels;
    void setLoopPresentation(std::optional<LoopPresentation> value)
    {
        loopPresentation=std::move(value);
        save.setButtonText(loopPresentation?"Save WAV":"Save WAV...");
        loopControls.setVisible(loopPresentation.has_value());placement.setVisible(loopPresentation.has_value());
        if(loopPresentation){loopControls.setMeter(loopPresentation->beatsPerBar,loopPresentation->beatAvailable,loopPresentation->meterAvailable);loopControls.display(loopPresentation->settings);}
        else loopInvocation.reset();
        setTitle(loopPresentation?"Loop-aligned export preview":"Song-aligned export preview");
        resized();repaint();
    }
    LoopExportControls& loopOptions()noexcept{return loopControls;}
    controls::ExportLevelsControl& mixControl()noexcept{return mix;}
    juce::Label& placementHelp()noexcept{return placement;}
    void setPreviewDeviceRate(double rate){deviceRate=rate;}
    int displayedLoopRepeats()const noexcept
    {return !loopPresentation?0:displayed && displayed->plan->loop?displayed->plan->loop->repeats:loopPresentation->settings.repeats;}
    void display(const EditorSnapshot& editor)
    {
        const juce::ScopedValueSetter<bool> update(refreshing,true);
        const auto hadStatus=statusText().isNotEmpty();
        if(state.session!=editor.songExport.session || state.pass!=editor.songExport.pass)navigation.cancelGesture();
        if(editor.songExport.phase==ExportPhase::preparing && state.phase!=ExportPhase::preparing)pendingSince=juce::Time::getMillisecondCounterHiRes();
        if(state.session!=editor.songExport.session){displayed.reset();identity.clear();rangeIdentity.clear();}
        state=editor.songExport;
        displayed=state.phase==ExportPhase::idle && state.message.startsWith("Silent:")?nullptr:state.prepared?state.prepared:state.previous;
        if(state.cursor && state.previewLevels)for(const auto& candidate:{state.previous,state.prepared,state.preview})
            if(candidate && candidate->plan->dry==state.previewLevels->dry && candidate->plan->wet==state.previewLevels->wet)displayed=candidate;
        if(lastDelivery!=state.delivery){lastDelivery=state.delivery;localMessage.clear();}
        const auto levels=editor.session.exportState.levels(loopPresentation.has_value());
        requestedLevels={levels.dry,levels.wet};mix.display(requestedLevels);
        const auto ready=state.phase==ExportPhase::ready && state.prepared!=nullptr;
        tile.setStatusText(ready && loopPresentation?loopMixLabel(*state.prepared->plan)+" | "+juce::String(loopPresentation->settings.repeats)+(loopPresentation->settings.repeats==1?" pass":" passes"):ready?"Ready: "+levelText(*state.prepared->plan)+" | "+
            (state.prepared->plan->dry>0?juce::String("Mute source | "):juce::String{})+"Song-aligned":state.phase==ExportPhase::idle && state.message.startsWith("Silent:")?state.message:state.phase==ExportPhase::error?"Error: requested WAV not ready":"Preparing requested WAV");
        play.setEnabled(ready && state.prepared->metrics.audibleStart.has_value());stop.setEnabled(state.cursor.has_value() || state.previewRequested);
        juce::String previewHelp="Preview from the positioned playhead, or the first audible sample by default";
        if(loopPresentation){
            if(!ready)previewHelp="Preview is unavailable until the current mix is ready.";
            else if(!std::isfinite(deviceRate) || deviceRate<=0){play.setEnabled(false);previewHelp="Start the host audio device to preview this Loop. Saving the WAV does not need live playback.";}
            else if(deviceRate!=state.prepared->plan->timing.rate){play.setEnabled(false);previewHelp="Set the host audio rate to "+juce::String(state.prepared->plan->timing.rate/1000.,1)+" kHz to preview this Loop. The WAV stays at the captured rate.";}
        }
        play.setTooltip(previewHelp);play.setHelpText(previewHelp);
        save.setEnabled(ready && !state.delivering && !choosing);tile.setEnabled(ready && !state.delivering && !choosing);
        navigation.setEnabled(displayed!=nullptr);
        navigation.setPositioningEnabled(ready);
        if(displayed && (rangeIdentity!=displayed->plan->identity || (ready && identity!=displayed->plan->identity))){
            const auto reset=(!state.cursor && ready) || rangeIdentity.isEmpty();rangeIdentity=displayed->plan->identity;localMessage.clear();const auto& p=*displayed->plan;const auto& m=displayed->metrics;
            navigation.setRange(static_cast<double>(loopPresentation?p.origin:m.audibleStart.value_or(p.origin)),static_cast<double>(loopPresentation?p.end:m.audibleEnd.value_or(p.end)),static_cast<double>(p.origin),static_cast<double>(p.end),p.timing.rate,reset);
        }
        if(ready)identity=state.prepared->plan->identity;
        if(loopPresentation && displayed){
            const auto bars=loopPresentation->barFrames>0?-displayed->plan->origin/loopPresentation->barFrames:0;
            const auto text=(bars>0?"Place WAV "+juce::String(bars,2).trimCharactersAtEnd("0").trimCharactersAtEnd(".")+" bars before Loop Bar 1.":"Place WAV start at Loop Bar 1.")
                +(displayed->plan->dry>0?juce::String("\nMute the original dry when using this WAV."):juce::String{});
            placement.setText(text,juce::dontSendNotification);placement.setTooltip(text);
        }
        updateZoom();
        if(dragRequest && state.delivery==dragRequest && !state.delivering){
            dragRequest=0;
            if(state.dragReady){
                const auto result=controls::invokeExternalFileDrag(state.deliveryPath,state.dragReady,juce::ModifierKeys::getCurrentModifiersRealtime().isLeftButtonDown(),
                    [this](const auto& path){return juce::DragAndDropContainer::performExternalDragDropOfFiles({path},false,&tile);});
                localMessage=result==controls::ExternalDragResult::started?(loopPresentation?placement.getText():juce::String("Place the WAV at song origin. Its managed file is retained.")):
                    result==controls::ExternalDragResult::released?"Drag released before verification. Use Save WAV, or drag again.":"External dragging is unavailable. Use Save WAV.";
            }else localMessage=state.deliveryMessage;
        }
        if(hadStatus!=statusText().isNotEmpty())resized();
        repaint();
    }
    std::optional<controls::NoticeMessage> notice()const
    {
        using controls::NoticeMessage;using controls::NoticeTone;
        if(state.phase==ExportPhase::error)return NoticeMessage{"Export unavailable: "+state.message,state.message,NoticeTone::error};
        if(state.message.startsWith("Preview stopped:"))return NoticeMessage{state.message,"Playback stopped safely. WAV readiness and retained files are unchanged; resolve the reported preview dependency before pressing Play again.",NoticeTone::warning};
        if(state.cursor && state.phase==ExportPhase::preparing && (!displayed || !state.previewLevels || displayed->plan->dry!=state.previewLevels->dry || displayed->plan->wet!=state.previewLevels->wet))return NoticeMessage{"Preview continues | Current WAV preparing","The preview follows the accepted levels with a 5 ms monitoring-only gain transition. Current peak measurement/WAV publication is pending; Save and Drag are unavailable.",NoticeTone::information};
        const auto measured=state.phase==ExportPhase::ready?state.prepared:state.preview;
        if(measured && measured->metrics.peak>1){
            const auto peak=juce::String(juce::Decibels::gainToDecibels(measured->metrics.peak),2)+" dBFS";
            juce::String detail="Float32 WAV preserves above-unity samples; no automatic attenuation. Preview uses the same gains with monitoring boundary ramps.\n";
            for(const auto& range:measured->plan->warnings)detail+=juce::String(range.start)+" to "+juce::String(range.end)+(range.kind==ExportCoverage::gap?": uncaptured dry exported as silence\n":": unavailable dry exported as silence\n");
            return NoticeMessage{"Export peak "+peak+": downstream clipping possible"+(measured->plan->warnings.empty()?juce::String{}:juce::String(" | Missing dry exported as silence")),detail,NoticeTone::warning};
        }
        if(measured && !measured->plan->warnings.empty()){
            juce::String detail="Uncaptured or unavailable dry contributes silence; source coverage is unchanged.\n";
            for(const auto& w:measured->plan->warnings)detail+=juce::String(w.start)+" to "+juce::String(w.end)+(w.kind==ExportCoverage::gap?": uncaptured\n":": unavailable\n");
            return NoticeMessage{"Missing dry: "+juce::String(measured->plan->warnings.size())+" ranges exported as silence",detail,NoticeTone::warning};
        }
        return NoticeMessage{"DAW wet is bypassed in Export", "Use Preview for this mix. Stay in Export while playing the delivered WAV to avoid doubling swells.",NoticeTone::information};
    }
    void setResponsivePresentation(){responsive=true;mix.setResponsivePresentation();resized();}
    void resized()override
    {
        if(responsive){layoutResponsive();return;}
        auto body=getLocalBounds();right=body.removeFromRight((body.getWidth()-workspaceGeometry::columnGap)/3);
        body.removeFromRight(workspaceGeometry::columnGap);left=body;
        auto a=right;mixHeading=a.removeFromTop(controlGeometry::headingHeight);a.removeFromTop(spacing::related);
        mix.setBounds(a.removeFromTop((controlGeometry::labelHeight+spacing::labelHelp+controlGeometry::height+spacing::related)*3));a.removeFromTop(spacing::section);
        tile.setBounds(a.removeFromTop(std::min(a.getHeight()-2*controlGeometry::height-spacing::section-spacing::group,a.getWidth()*3/4)));a.removeFromTop(spacing::section);
        save.setBounds(a.removeFromTop(controlGeometry::height));a.removeFromTop(spacing::group);back.setBounds(a.removeFromTop(controlGeometry::height));
        a=left;heading=a.removeFromTop(controlGeometry::height);a.removeFromTop(spacing::labelHelp);
        caption=a.removeFromTop(learnTypography::secondaryLineHeight);a.removeFromTop(spacing::related);
        ruler=a.removeFromTop(controlGeometry::labelHeight);a.removeFromTop(learnGeometry::rulerViewportGap);
        a.removeFromBottom(spacing::related);
        auto buttons=a.removeFromBottom(controlGeometry::height);play.setBounds(buttons.removeFromLeft(controlGeometry::height));buttons.removeFromLeft(spacing::related);
        stop.setBounds(buttons.removeFromLeft(controlGeometry::height));buttons.removeFromLeft(spacing::group);timeArea=buttons;
        a.removeFromBottom(spacing::related);
        statusArea={};
        if(statusText().isNotEmpty()){
            statusArea=a.removeFromBottom(learnTypography::secondaryLineHeight);
            a.removeFromBottom(spacing::labelHelp);
        }
        auto toolbar=a.removeFromBottom(controlGeometry::height);
        zoom.setBounds(toolbar.removeFromLeft(std::max(controls::TimelineZoomControls::minimumWidth(),toolbar.getWidth()/2)));
        toolbar.removeFromLeft(spacing::related);legend=toolbar.withSizeKeepingCentre(toolbar.getWidth(),controlGeometry::labelHeight);
        a.removeFromBottom(spacing::related);chart=a;navigation.setBounds(chart);requestDetail();
    }
    void paint(juce::Graphics& g)override
    {
        g.setColour(separator);g.drawVerticalLine(left.getRight()+workspaceGeometry::columnGap/2,responsive?static_cast<float>(responsiveGeometry::dividerClearance):0.f,static_cast<float>(getHeight()-(responsive?responsiveGeometry::dividerClearance:0)));
        applyTextRole(g,*this,TextRole::sectionHeading);g.drawText("Export Preview",heading,juce::Justification::left);g.drawText("Export Levels",mixHeading,juce::Justification::left);
        const auto previous=displayed && (state.phase!=ExportPhase::ready || displayed!=state.prepared);
        applyTextRole(g,*this,TextRole::helper);
        const auto captionText=state.cursor && state.previewLevels?juce::String(displayed && (displayed->plan->dry!=state.previewLevels->dry || displayed->plan->wet!=state.previewLevels->wet)?"Previous waveform | ":"")+"Preview Dry "+juce::String(state.previewLevels->dry*100,1)+"% / Wet "+juce::String(state.previewLevels->wet*100,1)+"% | "+(state.phase==ExportPhase::ready?"WAV ready":state.phase==ExportPhase::error?"WAV failed":"WAV preparing")
            :previous?juce::String("Previous ")+levelText(*displayed->plan)+" | "+(state.phase==ExportPhase::error?"Preparation failed":"Updating preview...")
            :state.detailMessage.isNotEmpty()?juce::String("Waveform detail unavailable; accepted WAV retained."):juce::String("Left-click/drag: preview position. Right-drag: pan. Wheel: zoom.");
        g.drawFittedText(captionText,caption,juce::Justification::topLeft,2);
        g.setColour(outline);g.drawRoundedRectangle(controlGeometry::insideStroke(chart,rule),radius,rule);
        if(!displayed){if(state.phase!=ExportPhase::error && juce::Time::getMillisecondCounterHiRes()-pendingSince<600)return;applyTextRole(g,*this,TextRole::emptyBody);g.drawFittedText(state.phase==ExportPhase::idle && state.message.startsWith("Silent:")?state.message:state.phase==ExportPhase::error?"Export unavailable. See the message above.":"Preparing your song-aligned mix",chart.reduced(spacing::section),juce::Justification::centred,2);return;}
        const auto& p=*displayed->plan;const auto& m=displayed->metrics;
        const auto first=navigation.viewport().first,last=navigation.viewport().last;
        const auto x=[&](double at){return chart.getX()+static_cast<float>((at-first)/(last-first)*chart.getWidth());};
        long double origin;
        if(loopPresentation)paintLoopRuler(g,x);
        else if(::songOrigin(p.timing,origin))controls::paintMusicalTimelineRuler(g,ruler,navigation.viewport(),p.timing,this);
        else {applyTextRole(g,*this,TextRole::ruler);g.drawText("Musical timing unavailable",ruler,juce::Justification::left);}
        {
            juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(chart.reduced(1));
            for(const auto& coverage:p.warnings){g.setColour(coverage.kind==ExportCoverage::gap?capturedSilence:unavailableSource);
                g.fillRect(juce::Rectangle<float>(x(static_cast<double>(coverage.start)),static_cast<float>(chart.getY()),x(static_cast<double>(coverage.end))-x(static_cast<double>(coverage.start)),static_cast<float>(chart.getHeight())));}
            if(previous)g.setOpacity(.6f);
            const auto draw=[&](const std::vector<CapturePeak>& peaks,juce::Colour ink){g.setColour(ink);juce::Path path;
                const auto centre=static_cast<float>(chart.getCentreY()),amplitude=chart.getHeight()*.45f*m.displayGain;
                for(const auto& peak:peaks){if(peak.startSample+peak.frameCount<first || peak.startSample>last || (peak.minimum==0 && peak.maximum==0))continue;
                    const auto a=x(static_cast<double>(peak.startSample)),b=x(static_cast<double>(peak.startSample)+peak.frameCount);
                    path.addRectangle(a,centre-peak.maximum*amplitude,std::max(1.f,b-a),std::max(rule,(peak.maximum-peak.minimum)*amplitude));}
                g.fillPath(path);};
            const auto detail=state.detail;const auto bounds=detailBounds();const auto detailed=detail && detail->composition==p.identity && detail->first<=std::max(bounds.first,p.origin) && detail->end>=std::min(bounds.second,p.end);
            if(p.dry>0)draw(detailed?detail->dry:m.dryPeaks,text);if(p.wet>0)draw(detailed?detail->wet:m.wetPeaks,aqua);
            if(!m.audibleStart){applyTextRole(g,*this,TextRole::emptyBody);g.drawText("No audible signal in this mix",chart,juce::Justification::centred);}
            const auto previewAt=state.cursor?state.cursor:state.previewPosition;
            if(previewAt){g.setColour(previewCursor);g.drawVerticalLine(juce::roundToInt(x(static_cast<double>(*previewAt))),static_cast<float>(chart.getY()),static_cast<float>(chart.getBottom()));}
        }
        applyTextRole(g,*this,TextRole::helper);
        auto key=legend;key=key.removeFromRight(controlGeometry::valueWidth*4);
        auto wetKey=key.removeFromLeft(controlGeometry::valueWidth*2);auto dryKey=key;
        const auto legendEntry=[&](juce::Rectangle<int> box,juce::Colour colour,const juce::String& label,bool active){
            auto dot=box.removeFromLeft(spacing::related).toFloat().withSizeKeepingCentre(spacing::tight*2.f,spacing::tight*2.f);g.setColour(colour);
            if(active)g.fillEllipse(dot);else g.drawEllipse(dot,rule);box.removeFromLeft(spacing::tight);applyTextRole(g,*this,TextRole::helper);g.drawText(label+(active?"":" (off)"),box,juce::Justification::left);};
        legendEntry(wetKey,aqua,"Wet "+juce::String(p.wet*100,1)+"%",p.wet>0);
        legendEntry(dryKey,text,"Dry "+juce::String(p.dry*100,1)+"%",p.dry>0);
        applyTextRole(g,*this,TextRole::helper);
        const auto current=state.cursor.value_or(state.previewPosition.value_or(m.audibleStart.value_or(p.origin)));
        g.drawText((loopPresentation?loopMixLabel(p)+"  ":juce::String("Preview (song)  "))+seconds((current-p.origin)/p.timing.rate)+" / "+seconds((p.end-p.origin)/p.timing.rate)+(loopPresentation?juce::String{}:" | WAV "+seconds((p.end-p.origin)/p.timing.rate)),timeArea,juce::Justification::centredLeft);
        g.drawFittedText(statusText(),statusArea,juce::Justification::topLeft,2);
    }
    void resetView(){navigation.cancelGesture();zoomInvocation.reset();state={};displayed.reset();identity.clear();rangeIdentity.clear();updateZoom();}
    const SwellViewport& viewport()const noexcept{return navigation.viewport();}
    juce::Rectangle<int> waveformBounds()const noexcept{return chart;}
    juce::Rectangle<int> rulerBounds()const noexcept{return ruler;}
    juce::Rectangle<int> captionBounds()const noexcept{return caption;}
    juce::Rectangle<int> legendBounds()const noexcept{return legend;}
    juce::Rectangle<int> statusBounds()const noexcept{return statusArea;}
private:
    bool responsive=false;
    void layoutResponsive()
    {
        using namespace responsiveGeometry;
        auto area=getLocalBounds();right=area.removeFromRight(300);area.removeFromRight(18);left=area;
        auto controls=right.withTrimmedLeft(contentPadding);mixHeading=controls.removeFromTop(headingHeight);controls.removeFromTop(contentPadding);
        mix.setBounds(controls.removeFromTop(mix.preferredHeight()));
        controls.removeFromTop(groupGap);
        const auto helpHeight=loopPresentation?2*learnTypography::secondaryLineHeight+contentPadding:0;
        const auto actionsHeight=loopPresentation?controlHeight+groupGap:2*controlHeight+2*groupGap;
        const auto tileHeight=std::max(0,std::min(exportTileHeight,controls.getHeight()-actionsHeight-helpHeight));tile.setBounds(controls.removeFromTop(tileHeight));
        if(loopPresentation){controls.removeFromTop(contentPadding);placement.setBounds(controls.removeFromTop(2*learnTypography::secondaryLineHeight));}
        controls.removeFromTop(groupGap);
        if(loopPresentation){auto actions=controls.removeFromTop(controlHeight);save.setBounds(actions.removeFromLeft(actions.getWidth()*2/5));actions.removeFromLeft(groupGap);back.setBounds(actions);}
        else {save.setBounds(controls.removeFromTop(controlHeight));controls.removeFromTop(groupGap);back.setBounds(controls.removeFromTop(controlHeight));}
        auto preview=left;heading=preview.removeFromTop(headingHeight);preview.removeFromTop(spacing::tight);caption=preview.removeFromTop(loopPresentation?captionHeight:40);
        if(loopPresentation){preview.removeFromTop(contentPadding);loopControls.setBounds(preview.removeFromTop(LoopExportControls::preferredHeight()));preview.removeFromTop(contentPadding);}
        ruler=preview.removeFromTop(loopPresentation?2*captionHeight:24);preview.removeFromBottom(contentPadding);
        auto transport=preview.removeFromBottom(controlHeight);play.setBounds(transport.removeFromLeft(controlHeight));transport.removeFromLeft(contentPadding);stop.setBounds(transport.removeFromLeft(controlHeight));transport.removeFromLeft(contentPadding);timeArea=transport;
        preview.removeFromBottom(contentPadding);auto navigationRow=preview.removeFromBottom(controlHeight);zoom.setBounds(navigationRow.removeFromLeft(std::max(controls::TimelineZoomControls::minimumWidth(),navigationRow.getWidth()/2)));navigationRow.removeFromLeft(contentPadding);legend=navigationRow;
        preview.removeFromBottom(contentPadding);statusArea=preview.removeFromBottom(statusText().isNotEmpty()?20:0);chart=preview;navigation.setBounds(chart);requestDetail();
    }
    juce::String statusText()const
    {
        if(!displayed)return {};
        if(!displayed->metrics.audibleStart)return "No audible signal. The song-aligned WAV contains silence.";
        if(localMessage.isNotEmpty())return localMessage;
        if(state.deliveryMessage.isNotEmpty())return state.deliveryMessage;
        return {};
    }
    struct ZoomScope {uint64_t session,pass;juce::String composition;};
    template<class X>void paintLoopRuler(juce::Graphics& g,const X& x)
    {
        if(!displayed || !loopPresentation)return;
        const auto& plan=*displayed->plan;
        const auto label=[&](double at,const juce::String& text,bool first=false,bool last=false){
            const auto pixel=x(at);if(pixel<chart.getX()-1 || pixel>chart.getRight()+1)return;
            const auto width=std::min(chart.getWidth()/3,controlGeometry::valueWidth*2);
            const auto start=std::clamp(juce::roundToInt(pixel)-(last?width:first?0:width/2),ruler.getX(),ruler.getRight()-width);
            applyTextRole(g,*this,TextRole::helper);g.drawText(text,juce::Rectangle<int>{start,ruler.getY(),width,ruler.getHeight()},first?juce::Justification::left:last?juce::Justification::right:juce::Justification::centred);
            g.setColour(at==0?timing:separator);g.drawVerticalLine(juce::roundToInt(pixel),static_cast<float>(chart.getY()),static_cast<float>(chart.getBottom()));
        };
        const auto startBars=loopPresentation->barFrames>0?plan.origin/loopPresentation->barFrames:0;
        if(plan.origin==0)label(0,"WAV start / Loop Bar 1",true);
        else {label(static_cast<double>(plan.origin),"WAV start "+juce::String(startBars,2).trimCharactersAtEnd("0").trimCharactersAtEnd(".")+" bars",true);label(0,"Loop Bar 1");}
        if(displayedLoopRepeats()>1)label(loopPresentation->periodFrames,"Repeat 2");
        label(static_cast<double>(plan.end),"End",false,true);
    }
    bool zoomIntentCurrent()const{return displayed && zoomInvocation && zoomInvocation->session==state.session
        && zoomInvocation->pass==state.pass && zoomInvocation->composition==displayed->plan->identity;}
    void updateZoom()
    {
        zoom.setEnabled(displayed!=nullptr);if(!displayed){zoom.display({});return;}
        const auto low=navigation.minimumSpan(),high=navigation.maximumSpan(),span=navigation.viewport().span();
        if(!(low>0 && high>low && std::isfinite(high))){zoom.display({0,false,false,true});return;}
        const auto tolerance=std::numeric_limits<double>::epsilon()*std::max({1.,std::abs(navigation.viewport().first),std::abs(navigation.viewport().last),high})*2;
        const auto canIn=span>low+tolerance,canOut=span<high-tolerance;
        const auto fraction=!canIn?1.:!canOut?0.:std::clamp(std::log(high/span)/std::log(high/low),0.,1.);
        zoom.display({fraction,canIn,canOut,true});
    }
    std::pair<int64_t,int64_t> detailBounds()const{const auto& view=navigation.viewport();auto a=view.sample(0,0,navigation.getWidth()),b=view.sample(navigation.getWidth(),0,navigation.getWidth());if(a>INT64_MIN)--a;if(b<INT64_MAX)++b;return {a,b};}
    void requestDetail(){if(displayed && navigation.getWidth()>0){const auto bounds=detailBounds();engine.requestExportDetail(bounds.first,bounds.second,static_cast<size_t>(navigation.getWidth()*2),state.session,displayed->plan->identity);}}
    static juce::String levelText(const SongExportPlan& plan){return "Dry "+juce::String(plan.dry*100,1)+"% / Wet "+juce::String(plan.wet*100,1)+"%";}
    static juce::String loopMixLabel(const SongExportPlan& plan){return plan.dry>0?(plan.wet>0?"Dry + wet WAV":"Dry-only WAV"):"Wet-only WAV";}
    static juce::String seconds(double value){return juce::String(value,3)+" s";}
    void chooseSave()
    {
        if(choosing || !state.prepared || state.phase!=ExportPhase::ready)return;
        choosing=true;const auto file=state.prepared->file;const auto session=state.session,pass=state.pass;
        chooser=std::make_unique<juce::FileChooser>(loopPresentation?"Save Loop WAV":"Save song-aligned WAV",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile(file.path.fromLastOccurrenceOf("/",false,false)),"*.wav");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting,
            [safe=juce::Component::SafePointer<ExportWorkspace>(this),session,pass,file](const auto& dialog){if(safe){safe->choosing=false;if(dialog.getResult()!=juce::File{})safe->engine.deliverExport(session,file,dialog.getResult(),true,false,pass);}});
    }
    SwellEngine& engine;ExportSnapshot state;std::shared_ptr<const PreparedExport> displayed;juce::String identity,rangeIdentity,localMessage;
    double pendingSince=0;bool refreshing=false,choosing=false;uint64_t dragRequest=0,lastDelivery=0;std::optional<uint64_t> mixSession,mixPass;
    double deviceRate=0;
    std::unique_ptr<juce::FileChooser> chooser;
    controls::ActionButton play{"",controls::CommandIcon::play,true},stop{"",controls::CommandIcon::stop},save{"Save WAV...",controls::CommandIcon::save},back{"Return to Perform",controls::CommandIcon::returnBack};
    controls::TimelineZoomControls zoom;controls::TimelineNavigation navigation;std::optional<ZoomScope> zoomInvocation;
    controls::ExportLevelsControl mix;controls::FileDragTile tile;
    controls::ExportLevelsControl::Pair requestedLevels;
    LoopExportControls loopControls;juce::Label placement;
    std::optional<LoopPresentation> loopPresentation;
    std::optional<std::pair<uint64_t,uint64_t>> loopInvocation;
    juce::Rectangle<int> left,right,heading,caption,mixHeading,ruler,chart,legend,timeArea,statusArea;
};
}
