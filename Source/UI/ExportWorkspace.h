#pragma once
#include "WorkspaceTheme.h"
#include "SongBarRuler.h"
#include <Core/MusicalPosition.h>
#include "Controls/ActionButton.h"
#include "Controls/FileDragTile.h"
#include "Controls/ExternalFileDrag.h"
#include "Controls/SliderControl.h"
#include "Controls/NoticeBar.h"
#include "Controls/TimelineNavigation.h"
#include <DSP/SwellEngine.h>

namespace m3::ui
{
class ExportWorkspace final:public juce::Component
{
public:
    explicit ExportWorkspace(SwellEngine& owner):engine(owner)
    {
        setName("exportWorkspace");
        for(auto* c:std::initializer_list<juce::Component*>{&play,&stop,&mix,&tile,&save,&back,&fitButton,&navigation})addAndMakeVisible(c);
        fitButton.setName("exportFit");navigation.setName("exportNavigation");
        fitButton.onClick=[this]{navigation.fit();};navigation.onChange=[this]{requestDetail();repaint();};
        navigation.onPosition=[this](int64_t sample){if(state.prepared && engine.seekExport(sample,state.session,state.pass,state.prepared->plan->identity)){
            state.previewPosition=std::clamp(sample,*state.prepared->metrics.audibleStart,*state.prepared->metrics.audibleEnd-1);state.cursor.reset();repaint();}};
        navigation.setTooltip("Click or drag: position preview. Shift-drag: pan. Wheel: zoom. Shift-wheel: pan. Home: Fit.");
        play.setName("exportPlay");stop.setName("exportStop");mix.setName("exportMix");tile.setName("exportDrag");save.setName("exportSave");back.setName("exportReturn");
        setTitle("Song-aligned export preview");mix.setTitle("Export mix");play.setTitle("Play export preview");stop.setTitle("Stop export preview");
        play.setTooltip("Preview from the positioned playhead, or the first audible sample by default");stop.setTooltip("Stop internal preview; DAW transport is unchanged");
        mix.setRange(0,100,.1);mix.setNumDecimalPlacesToDisplay(1);mix.setTextValueSuffix(" %");
        mix.setTooltip("Linear dry/wet blend: 0% dry only, 100% wet swells only");
        mix.beforeInvocation=[this]{mixSession=state.session;mixPass=state.pass;};
        mix.onValueChange=[this]{if(!refreshing && engine.setExportMix(static_cast<float>(mix.getValue()/100),mixSession.value_or(engine.assignmentEpoch()),mixPass.value_or(state.pass))){state.phase=ExportPhase::preparing;state.prepared.reset();display(engine.snapshot());}};
        play.onClick=[this]{engine.playExport(engine.assignmentEpoch());};stop.onClick=[this]{engine.stop();};
        save.onClick=tile.onSave=[this]{chooseSave();};back.onClick=[this]{if(onReturn)onReturn();};
        tile.onDrag=[this]{if(state.prepared)dragRequest=engine.deliverExport(engine.assignmentEpoch(),state.prepared->file,{},false,true);};
    }
    std::function<void()> onReturn;
    void display(const EditorSnapshot& editor)
    {
        const juce::ScopedValueSetter<bool> update(refreshing,true);
        if(state.session!=editor.songExport.session || state.pass!=editor.songExport.pass)navigation.cancelGesture();
        if(editor.songExport.phase==ExportPhase::preparing && state.phase!=ExportPhase::preparing)pendingSince=juce::Time::getMillisecondCounterHiRes();
        if(state.session!=editor.songExport.session){displayed.reset();identity.clear();rangeIdentity.clear();}
        state=editor.songExport;
        displayed=state.prepared?state.prepared:state.previous;
        if(lastDelivery!=state.delivery){lastDelivery=state.delivery;localMessage.clear();}
        if(!mix.isMouseButtonDown() && !mix.hasKeyboardFocus(true))mix.setValue(editor.session.exportState.wet*100,juce::dontSendNotification);
        const auto ready=state.phase==ExportPhase::ready && state.prepared!=nullptr;
        tile.setStatusText(ready?"Ready: "+juce::String(editor.session.exportState.wet*100,1)+"% Wet | song-aligned":state.phase==ExportPhase::error?"Error: requested WAV not ready":"Preparing requested WAV");
        play.setEnabled(ready && state.prepared->metrics.audibleStart.has_value());stop.setEnabled(state.cursor.has_value() || state.previewRequested);
        save.setEnabled(ready && !state.delivering && !choosing);tile.setEnabled(ready && !state.delivering && !choosing);
        fitButton.setEnabled(displayed!=nullptr);navigation.setEnabled(displayed!=nullptr);
        navigation.setPositioningEnabled(ready);
        if(displayed && (rangeIdentity!=displayed->plan->identity || (ready && identity!=displayed->plan->identity))){
            const auto reset=ready || rangeIdentity.isEmpty();rangeIdentity=displayed->plan->identity;localMessage.clear();const auto& p=*displayed->plan;const auto& m=displayed->metrics;
            navigation.setRange(static_cast<double>(m.audibleStart.value_or(p.origin)),static_cast<double>(m.audibleEnd.value_or(p.end)),static_cast<double>(p.origin),static_cast<double>(p.end),p.timing.rate,reset);
        }
        if(ready)identity=state.prepared->plan->identity;
        if(dragRequest && state.delivery==dragRequest && !state.delivering){
            dragRequest=0;
            if(state.dragReady){
                const auto result=controls::invokeExternalFileDrag(state.deliveryPath,state.dragReady,juce::ModifierKeys::getCurrentModifiersRealtime().isLeftButtonDown(),
                    [this](const auto& path){return juce::DragAndDropContainer::performExternalDragDropOfFiles({path},false,&tile);});
                localMessage=result==controls::ExternalDragResult::started?"Place the WAV at song origin. Its managed file is retained.":
                    result==controls::ExternalDragResult::released?"Drag released before verification. Use Save WAV, or drag again.":"External dragging is unavailable. Use Save WAV.";
            }else localMessage=state.deliveryMessage;
        }
        repaint();
    }
    std::optional<controls::NoticeMessage> notice()const
    {
        using controls::NoticeMessage;using controls::NoticeTone;
        if(state.phase==ExportPhase::error)return NoticeMessage{"Export unavailable: "+state.message,state.message,NoticeTone::error};
        if(state.prepared && !state.prepared->plan->warnings.empty()){
            juce::String detail="Uncaptured or unavailable dry contributes silence; source coverage is unchanged.\n";
            for(const auto& w:state.prepared->plan->warnings)detail+=juce::String(w.start)+" to "+juce::String(w.end)+(w.kind==ExportCoverage::gap?": uncaptured\n":": unavailable\n");
            return NoticeMessage{"Missing dry: "+juce::String(state.prepared->plan->warnings.size())+" ranges exported as silence",detail,NoticeTone::warning};
        }
        return NoticeMessage{"DAW wet is bypassed in Export", "Use Preview for this mix. Stay in Export while playing the delivered WAV to avoid doubling swells.",NoticeTone::information};
    }
    void resized()override
    {
        auto body=getLocalBounds();right=body.removeFromRight((body.getWidth()-workspaceGeometry::columnGap)/3);
        body.removeFromRight(workspaceGeometry::columnGap);left=body;
        auto a=right;mixHeading=a.removeFromTop(controlGeometry::headingHeight);a.removeFromTop(spacing::related);
        mix.setBounds(a.removeFromTop(controlGeometry::height));a.removeFromTop(spacing::section);
        tile.setBounds(a.removeFromTop(std::min(a.getHeight()-2*controlGeometry::height-spacing::section-spacing::group,a.getWidth()*3/4)));a.removeFromTop(spacing::section);
        save.setBounds(a.removeFromTop(controlGeometry::height));a.removeFromTop(spacing::group);back.setBounds(a.removeFromTop(controlGeometry::height));
        a=left;heading=a.removeFromTop(controlGeometry::height);fitButton.setBounds(heading.removeFromRight(controlGeometry::valueWidth));heading.removeFromRight(spacing::related);a.removeFromTop(spacing::labelHelp);
        caption=a.removeFromTop(learnTypography::secondaryLineHeight*2);a.removeFromTop(spacing::group);
        ruler=a.removeFromTop(controlGeometry::labelHeight);a.removeFromTop(learnGeometry::rulerViewportGap);
        auto playback=a.removeFromBottom(controlGeometry::height*2+spacing::group);
        auto buttons=playback.removeFromTop(controlGeometry::height);play.setBounds(buttons.removeFromLeft(controlGeometry::height));buttons.removeFromLeft(spacing::related);
        stop.setBounds(buttons.removeFromLeft(controlGeometry::height));buttons.removeFromLeft(spacing::group);timeArea=buttons;
        playback.removeFromTop(spacing::related);statusArea=playback;
        legend=a.removeFromBottom(controlGeometry::labelHeight);a.removeFromBottom(spacing::related);chart=a;navigation.setBounds(chart);requestDetail();
    }
    void paint(juce::Graphics& g)override
    {
        g.setColour(separator);g.drawVerticalLine(left.getRight()+workspaceGeometry::columnGap/2,0.f,static_cast<float>(getHeight()));
        applyTextRole(g,TextRole::sectionHeading);g.drawText("Export Preview",heading,juce::Justification::left);g.drawText("Export Mix (Dry / Wet)",mixHeading,juce::Justification::left);
        const auto previous=displayed && (state.phase!=ExportPhase::ready || displayed!=state.prepared);
        applyTextRole(g,TextRole::helper);
        const auto captionText=previous?juce::String("Previous ")+juce::String(displayed->plan->wet*100,1)+"% Wet | "+(state.phase==ExportPhase::error?"Preparation failed":"Updating preview...")
            :state.detailMessage.isNotEmpty()?juce::String("Waveform detail unavailable; accepted WAV retained."):juce::String("Click/drag: preview position. Shift-drag: pan. Wheel: zoom. WAV starts at song origin.");
        g.drawFittedText(captionText,caption,juce::Justification::topLeft,2);
        g.setColour(outline);g.drawRoundedRectangle(controlGeometry::insideStroke(chart,rule),radius,rule);
        if(!displayed){if(state.phase!=ExportPhase::error && juce::Time::getMillisecondCounterHiRes()-pendingSince<600)return;applyTextRole(g,TextRole::emptyBody);g.drawFittedText(state.phase==ExportPhase::error?"Export unavailable. See the message above.":"Preparing your song-aligned mix",chart.reduced(spacing::section),juce::Justification::centred,2);return;}
        const auto& p=*displayed->plan;const auto& m=displayed->metrics;
        const auto first=navigation.viewport().first,last=navigation.viewport().last;
        const auto x=[&](double at){return chart.getX()+static_cast<float>((at-first)/(last-first)*chart.getWidth());};
        applyTextRole(g,TextRole::ruler);
        const auto ticks=makeSongBarRuler(first,last,chart.getWidth(),p.timing.rate,p.timing.bpm,p.timing.numerator,p.timing.denominator,p.timing.sample,p.timing.ppq);
        if(ticks.empty())g.drawText(musicalPosition(navigation.viewport().sample(0,0,chart.getWidth()),p.timing)+" to "+musicalPosition(navigation.viewport().sample(chart.getWidth(),0,chart.getWidth()),p.timing),ruler,juce::Justification::left);
        for(const auto& tick:ticks){
            const auto width=std::min(controlGeometry::rulerLabelWidth,chart.getWidth());
            auto label=juce::Rectangle<float>(std::clamp(x(tick.samplePosition)-width*.5f,static_cast<float>(chart.getX()),static_cast<float>(chart.getRight()-width)),static_cast<float>(ruler.getY()),static_cast<float>(width),static_cast<float>(ruler.getHeight()));
            g.drawText(juce::String(tick.bar)+".1",label,juce::Justification::centred);
        }
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
            if(m.audibleStart){if(p.wet<1)draw(detailed?detail->dry:m.dryPeaks,text);if(p.wet>0)draw(detailed?detail->wet:m.wetPeaks,aqua); }
            else {applyTextRole(g,TextRole::emptyBody);g.drawText("No audible signal in this mix",chart,juce::Justification::centred);}
            const auto cursor=state.cursor?state.cursor:state.previewPosition;
            if(cursor){g.setColour(previewCursor);g.drawVerticalLine(juce::roundToInt(x(static_cast<double>(*cursor))),static_cast<float>(chart.getY()),static_cast<float>(chart.getBottom()));}
        }
        applyTextRole(g,TextRole::helper);
        auto key=legend;auto wetKey=key.removeFromLeft(controlGeometry::valueWidth*2);auto dryKey=key.removeFromLeft(controlGeometry::valueWidth*2);
        const auto legendEntry=[&](juce::Rectangle<int> box,juce::Colour colour,const juce::String& label,bool active){
            auto dot=box.removeFromLeft(spacing::related).toFloat().withSizeKeepingCentre(spacing::tight*2.f,spacing::tight*2.f);g.setColour(colour);
            if(active)g.fillEllipse(dot);else g.drawEllipse(dot,rule);box.removeFromLeft(spacing::tight);applyTextRole(g,TextRole::helper);g.drawText(label+(active?"":" (off)"),box,juce::Justification::left);};
        legendEntry(wetKey,aqua,"Wet swells",p.wet>0);legendEntry(dryKey,text,"Dry source",p.wet<1);
        applyTextRole(g,TextRole::helper);g.drawText("Shared scale: "+juce::String(juce::Decibels::gainToDecibels(m.displayGain),1)+" dB",key,juce::Justification::centredRight);
        const auto current=state.cursor.value_or(state.previewPosition.value_or(m.audibleStart.value_or(p.origin)));
        g.drawText("Preview (audible)  "+seconds((current-m.audibleStart.value_or(p.origin))/p.timing.rate)+" / "+seconds((m.audibleEnd.value_or(p.origin)-m.audibleStart.value_or(p.origin))/p.timing.rate)+" | WAV "+seconds(displayed->file.frames/p.timing.rate),timeArea,juce::Justification::centredLeft);
        auto status=localMessage.isNotEmpty()?localMessage:state.deliveryMessage.isNotEmpty()?state.deliveryMessage:
            p.wet==1?juce::String("Wet layer: place at song origin. Stay in Export during WAV playback."):juce::String("Dry-inclusive replacement: mute the original source. Stay in Export during WAV playback.");
        if(!m.audibleStart)status="No audible signal. The song-aligned WAV contains silence.";
        g.drawFittedText(status,statusArea,juce::Justification::topLeft,2);
    }
    void resetView(){navigation.cancelGesture();state={};displayed.reset();identity.clear();rangeIdentity.clear();}
    const SwellViewport& viewport()const noexcept{return navigation.viewport();}
    juce::Rectangle<int> waveformBounds()const noexcept{return chart;}
private:
    std::pair<int64_t,int64_t> detailBounds()const{const auto& view=navigation.viewport();auto a=view.sample(0,0,navigation.getWidth()),b=view.sample(navigation.getWidth(),0,navigation.getWidth());if(a>INT64_MIN)--a;if(b<INT64_MAX)++b;return {a,b};}
    void requestDetail(){if(displayed && navigation.getWidth()>0){const auto bounds=detailBounds();engine.requestExportDetail(bounds.first,bounds.second,static_cast<size_t>(navigation.getWidth()*2),state.session,displayed->plan->identity);}}
    static juce::String seconds(double value){return juce::String(value,3)+" s";}
    void chooseSave()
    {
        if(choosing || !state.prepared || state.phase!=ExportPhase::ready)return;
        choosing=true;const auto file=state.prepared->file;const auto session=state.session,pass=state.pass;
        chooser=std::make_unique<juce::FileChooser>("Save song-aligned WAV",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile(file.path.fromLastOccurrenceOf("/",false,false)),"*.wav");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting,
            [safe=juce::Component::SafePointer<ExportWorkspace>(this),session,pass,file](const auto& dialog){if(safe){safe->choosing=false;if(dialog.getResult()!=juce::File{})safe->engine.deliverExport(session,file,dialog.getResult(),true,false,pass);}});
    }
    SwellEngine& engine;ExportSnapshot state;std::shared_ptr<const PreparedExport> displayed;juce::String identity,rangeIdentity,localMessage;
    double pendingSince=0;bool refreshing=false,choosing=false;uint64_t dragRequest=0,lastDelivery=0;std::optional<uint64_t> mixSession,mixPass;
    std::unique_ptr<juce::FileChooser> chooser;
    controls::ActionButton play{"",controls::CommandIcon::play,true},stop{"",controls::CommandIcon::stop},save{"Save WAV...",controls::CommandIcon::save},back{"Return to Perform",controls::CommandIcon::returnBack};
    controls::ActionButton fitButton{"Fit"};controls::TimelineNavigation navigation;
    controls::SliderControl mix;controls::FileDragTile tile;
    juce::Rectangle<int> left,right,heading,caption,mixHeading,ruler,chart,legend,timeArea,statusArea;
};
}
