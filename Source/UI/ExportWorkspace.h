#pragma once
#include "WorkspaceTheme.h"
#include <Core/TransportTime.h>
#include "Controls/ActionButton.h"
#include "Controls/FileDragTile.h"
#include "ExportFileInvocations.h"
#include "Controls/ExportLevelsControl.h"
#include "Controls/NoticeBar.h"
#include "Controls/TimelineNavigation.h"
#include "Controls/TimelineZoomControls.h"
#include "Controls/MusicalTimelineRuler.h"
#include <DSP/SwellEngine.h>
#include <cmath>
#include <limits>

namespace m3::ui
{
class ExportWorkspace final:public juce::Component
{
public:
    explicit ExportWorkspace(SwellEngine& owner, ExportFileInvocations::ChooseSave chooser = {},
                             ExportFileInvocations::StartExternalDrag nativeDrag = {})
        :engine(owner),fileInvocations(save,tile,
            [this](const auto& scope,const auto& destination){engine.deliverExport(scope.session,scope.file,destination,true,false,scope.pass);},
            [this](const auto& scope){return engine.readyExportFile(scope.session,scope.pass,scope.file);},
            nativeDrag ? std::move(nativeDrag) : ExportFileInvocations::StartExternalDrag{
                [this](const auto& file,const auto& canStart,const auto& canContinue)
                {return controls::dragReadyWav(file,tile,canStart,canContinue);}},
            std::move(chooser))
    {
        setName("exportWorkspace");
        for(auto* c:std::initializer_list<juce::Component*>{&play,&stop,&mix,&tile,&save,&back,&zoom,&navigation})addAndMakeVisible(c);
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
        play.setName("exportPlay");stop.setName("exportStop");mix.setName("exportLevels");tile.setName("exportDrag");save.setName("exportSave");back.setName("exportReturn");
        setTitle("Song-aligned export preview");mix.setTitle("Export mix");play.setTitle("Play export preview");stop.setTitle("Stop export preview");
        play.setTooltip("Preview from the positioned playhead, or the first audible sample by default");stop.setTooltip("Stop internal preview; DAW transport is unchanged");
        mix.beforeInvocation=[this]{mixSession=state.session;mixPass=state.pass;};
        mix.onChange=[this](controls::ExportLevelsControl::Pair pair){
            if(!refreshing)engine.setExportLevels(pair.dry,pair.wet,mixSession.value_or(engine.assignmentEpoch()),mixPass.value_or(state.pass));
            display(engine.snapshot());
        };
        play.onClick=[this]{engine.playExport(engine.assignmentEpoch());};stop.onClick=[this]{engine.stop();};
        back.onClick=[this]{if(onReturn)onReturn();};
        fileInvocations.onStatus=[this](const auto& message){localMessage=message;resized();repaint();};
    }
    std::function<void()> onReturn;
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
        mix.display({editor.session.exportState.dry,editor.session.exportState.wet});
        const auto ready=state.phase==ExportPhase::ready && state.prepared!=nullptr;
        fileInvocations.setCurrent(ready?std::optional{ExportFileInvocations::Scope{state.session,state.pass,state.prepared->file}}:std::nullopt);
        tile.setStatusText(ready?"Ready: "+levelText(*state.prepared->plan)+" | "+
            (state.prepared->plan->dry>0?juce::String("Mute source | "):juce::String{})+"Song-aligned":state.phase==ExportPhase::idle && state.message.startsWith("Silent:")?state.message:state.phase==ExportPhase::error?"Error: requested WAV not ready":"Preparing requested WAV");
        play.setEnabled(ready && state.prepared->metrics.audibleStart.has_value());stop.setEnabled(state.cursor.has_value() || state.previewRequested);
        save.setEnabled(ready && !state.delivering && !fileInvocations.isChoosing());
        tile.setEnabled(save.isEnabled());
        navigation.setEnabled(displayed!=nullptr);
        navigation.setPositioningEnabled(ready);
        if(displayed && (rangeIdentity!=displayed->plan->identity || (ready && identity!=displayed->plan->identity))){
            const auto reset=(!state.cursor && ready) || rangeIdentity.isEmpty();rangeIdentity=displayed->plan->identity;localMessage.clear();const auto& p=*displayed->plan;const auto& m=displayed->metrics;
            navigation.setRange(static_cast<double>(m.audibleStart.value_or(p.origin)),static_cast<double>(m.audibleEnd.value_or(p.end)),static_cast<double>(p.origin),static_cast<double>(p.end),p.timing.rate,reset);
        }
        if(ready)identity=state.prepared->plan->identity;
        updateZoom();
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
            :state.detailMessage.isNotEmpty()?juce::String("Waveform detail unavailable; accepted WAV retained."):juce::String("Left-click/drag: preview position. Middle-drag: pan. Wheel: zoom.");
        g.drawFittedText(captionText,caption,juce::Justification::topLeft,2);
        g.setColour(outline);g.drawRoundedRectangle(controlGeometry::insideStroke(chart,rule),radius,rule);
        if(!displayed){if(state.phase!=ExportPhase::error && juce::Time::getMillisecondCounterHiRes()-pendingSince<600)return;applyTextRole(g,*this,TextRole::emptyBody);g.drawFittedText(state.phase==ExportPhase::idle && state.message.startsWith("Silent:")?state.message:state.phase==ExportPhase::error?"Export unavailable. See the message above.":"Preparing your song-aligned mix",chart.reduced(spacing::section),juce::Justification::centred,2);return;}
        const auto& p=*displayed->plan;const auto& m=displayed->metrics;
        const auto first=navigation.viewport().first,last=navigation.viewport().last;
        const auto x=[&](double at){return chart.getX()+static_cast<float>((at-first)/(last-first)*chart.getWidth());};
        long double origin;
        if(::songOrigin(p.timing,origin))controls::paintMusicalTimelineRuler(g,ruler,navigation.viewport(),p.timing,this);
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
        g.drawText("Preview (song)  "+seconds((current-p.origin)/p.timing.rate)+" / "+seconds((p.end-p.origin)/p.timing.rate)+" | WAV "+seconds((p.end-p.origin)/p.timing.rate),timeArea,juce::Justification::centredLeft);
        g.drawFittedText(statusText(),statusArea,juce::Justification::topLeft,2);
    }
    void resetView(){fileInvocations.setCurrent({});navigation.cancelGesture();zoomInvocation.reset();state={};displayed.reset();identity.clear();rangeIdentity.clear();localMessage.clear();updateZoom();}
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
        mix.setBounds(controls.removeFromTop(mix.preferredHeight()));controls.removeFromTop(groupGap);
        const auto tileHeight=std::max(0,std::min(exportTileHeight,controls.getHeight()-2*controlHeight-2*groupGap));tile.setBounds(controls.removeFromTop(tileHeight));
        controls.removeFromTop(groupGap);save.setBounds(controls.removeFromTop(controlHeight));controls.removeFromTop(groupGap);back.setBounds(controls.removeFromTop(controlHeight));
        auto preview=left;heading=preview.removeFromTop(headingHeight);preview.removeFromTop(spacing::tight);caption=preview.removeFromTop(40);
        ruler=preview.removeFromTop(24);preview.removeFromBottom(contentPadding);
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
    static juce::String seconds(double value){return juce::String(value,3)+" s";}
    SwellEngine& engine;ExportSnapshot state;std::shared_ptr<const PreparedExport> displayed;juce::String identity,rangeIdentity,localMessage;
    double pendingSince=0;bool refreshing=false;uint64_t lastDelivery=0;std::optional<uint64_t> mixSession,mixPass;
    controls::ActionButton play{"",controls::CommandIcon::play,true},stop{"",controls::CommandIcon::stop},back{"Return to Perform",controls::CommandIcon::returnBack};
    controls::CapturedActionButton save{"Save WAV...",controls::CommandIcon::save};
    controls::TimelineZoomControls zoom;controls::TimelineNavigation navigation;std::optional<ZoomScope> zoomInvocation;
    controls::ExportLevelsControl mix;controls::FileDragTile tile;
    juce::Rectangle<int> left,right,heading,caption,mixHeading,ruler,chart,legend,timeArea,statusArea;
    ExportFileInvocations fileInvocations;
};
}
