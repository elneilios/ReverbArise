// Issues #46/#58 native layout checkpoints. The #58 composition is review-only
// until upstream UI integration and final combined acceptance are complete.
#include "AudioTestFiles.h"
#include "WorkflowTestSupport.h"
#include <Plugin/PluginEditor.h>
#include <UI/Controls/TimelineZoomControls.h>
#include <UI/PerformLayout.h>
#include <UI/TimingEdit.h>
#include <UI/PerformText.h>
#include <UI/Controls/MusicalTimelineRuler.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <map>
#if JUCE_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace preview46
{
using namespace m3::ui;
static juce::Component* find(juce::Component& root,const juce::String& name)
{
    if(root.getName()==name)return &root;
    for(auto* child:root.getChildren())if(auto* result=find(*child,name))return result;
    return nullptr;
}
template<class T>static T& require(juce::Component& root,const juce::String& name)
{auto* result=dynamic_cast<T*>(find(root,name));if(!result)throw std::runtime_error("Missing native fixture control: "+name.toStdString());return *result;}
template<class F>static void wait(F predicate)
{
    const auto end=juce::Time::getMillisecondCounter()+30000;
    while(!predicate() && juce::Time::getMillisecondCounter()<end)
        juce::MessageManager::getInstance()->runDispatchLoopUntil(10);
    REQUIRE(predicate());
}
struct Head final:juce::AudioPlayHead
{PositionInfo position;juce::Optional<PositionInfo> getPosition()const override{return position;}};

using Layout=PerformLayout;

class Composition final:public juce::Component,private juce::Timer
{
public:
    explicit Composition(bool independentRefresh=true)
    {
        setLookAndFeel(&theme);
        processor=std::make_unique<PreverbAudioProcessor>(directory.file);
        processor->swellEngine().setAutomaticProcessing(false);
        processor->swellEngine().setPostRoll(.25); // Fixture dry coverage excludes a deliberate gap.
        head.position.setBpm(120);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
        head.position.setIsPlaying(true);processor->setPlayHead(&head);processor->prepareToPlay(48000,4096);
        juce::AudioBuffer<float> audio(2,4096);juce::MidiBuffer midi;
        // Real test capture with a deliberately uncaptured middle interval.
        for(int block=0;block<48;++block)
        {
            const auto first=(block<24?block:block+24)*4096;
            head.position.setTimeInSamples(first);head.position.setPpqPosition(first/24000.0);
            audio.clear();for(int i=0;i<4096;++i)
            {
                const auto local=(first+i)%48000;
                const auto value=static_cast<float>(.25*std::exp(-local/6500.0)*std::sin(local*.07));
                audio.setSample(0,i,value);audio.setSample(1,i,value*.7f);
            }
            processor->processBlock(audio,midi);REQUIRE(processor->flushCaptureForTesting());
        }
        finishCaptureFixture(*processor,head.position);REQUIRE(processor->flushCaptureForTesting());
        auto& engine=processor->swellEngine();
        REQUIRE(engine.addTarget(48000));firstId=engine.savedSession().primary;
        REQUIRE(engine.addTarget(245760));secondId=engine.savedSession().primary;
        wait([&]{return engine.savedSession().defaultIR.present();});
        REQUIRE(engine.selectTargets({firstId,secondId},firstId));
        std::vector<float> impulse(24000);for(size_t i=0;i<impulse.size();++i)
            impulse[i]=static_cast<float>(.12*std::exp(-static_cast<double>(i)/4000)*std::cos(i*.031));
        const auto ir=directory.file.getChildFile("Native preview - real test impulse.wav");
        writeTestWav(ir,1,48000,impulse);engine.loadIR(ir);
        wait([&]{const auto s=engine.savedSession();return s.swells[0].irName==ir.getFileName() && s.swells[1].irName==ir.getFileName();});
        engine.generate(true);wait([&]{return engine.snapshot().auditionReady;});
        baseline=engine.savedSession();baseline.selected={firstId};baseline.primary=firstId;
        engine.restore(baseline);
        wait([&]{return engine.snapshot().raw!=nullptr;});
        editor=std::make_unique<PreverbAudioProcessorEditor>(*processor);
        if(independentRefresh)editor->stopAutomaticRefreshForReview();
        editor->refresh();
        // Borrow the actual production components and their domain bindings.
        // The hidden owning editor stays alive; only this fixture reparents them.
        for(const auto* name:{"workflowHeader","workspaceNotice","swellList","swellTimeline","songOverview",
                "inspectorViewport","audition","stopAudition","auditionWetOnly","hostInfo",
                "editorReadout"})
        {
            auto* component=find(*editor,name);REQUIRE(component);
            borrowed.push_back({component,component->getParentComponent()});addAndMakeVisible(component);parts[name]=component;
        }
        auto& list=require<controls::SwellList>(*this,"swellList");
        const auto selection=list.onSelection;
        list.onSelection=[this,selection](auto ids,auto primary){if(selection)selection(std::move(ids),std::move(primary));refresh();};
        const auto toggle=list.onToggleEnabled;
        list.onToggleEnabled=[this,toggle](auto id){if(toggle)toggle(std::move(id));refresh();};
        const auto envelope=timeline().onEnvelopeEdited;
        timeline().onEnvelopeEdited=[this,envelope]{if(envelope)envelope();refresh();};
        for(auto* label:{&swellsHeading,&countLabel,&inspectorHeading,&auditionHeading,&emptyList,&emptyInspector,&emptyTimeline,&monitor})
        {addAndMakeVisible(label);label->setInterceptsMouseClicks(false,false);}
        applyTextRole(swellsHeading,TextRole::sectionHeading);applyTextRole(inspectorHeading,TextRole::sectionHeading);
        swellsHeading.setText("Swells",juce::dontSendNotification);applyTextRole(countLabel,TextRole::helper,juce::Justification::centredRight);
        applyTextRole(auditionHeading,TextRole::sectionHeading);
        inspectorHeading.setText("Selected Swell",juce::dontSendNotification);auditionHeading.setText("Audition",juce::dontSendNotification);
        applyTextRole(monitor,TextRole::helper);
        for(auto* label:{&emptyList,&emptyInspector,&emptyTimeline})
            applyTextRole(*label,TextRole::helper,juce::Justification::centred);
        emptyList.setText("No swells yet\n\nAdd a swell with +",juce::dontSendNotification);
        emptyInspector.setText("No swell selected\n\nSelect a swell to edit",juce::dontSendNotification);
        emptyTimeline.setText("No source audio\n\nUse Learn to capture your song",juce::dontSendNotification);
        for(auto* button:{&snap,&add})addAndMakeVisible(button);
        addAndMakeVisible(zoom);addAndMakeVisible(grid);
        zoom.onZoomStep=[this](double factor){timeline().zoom(factor);refreshZoom();parts.at("songOverview")->repaint();};
        zoom.onFit=[this]{timeline().fitSelection();refreshZoom();parts.at("songOverview")->repaint();};
        zoom.onZoomFraction=[this](double fraction){timeline().setZoomFraction(fraction);refreshZoom();parts.at("songOverview")->repaint();};
        for(auto* child:timeline().getChildren())if(auto* label=dynamic_cast<juce::Label*>(child))
            if(label->getText()=="Wave zoom")
            {label->setText("Height",juce::dontSendNotification);applyTextRole(*label,TextRole::controlLabel);
                label->setTooltip("Waveform display height only; does not change audio gain or horizontal zoom.");}
        snap.setClickingTogglesState(true);snap.setToggleState(true,juce::dontSendNotification);
        snap.setTooltip("Layout specimen only. Ctrl bypass and new grid snapping follow layout approval.");
        snap.setEnabled(false);
        timeline().setSnap(false); // New musical snapping deliberately not implemented at this checkpoint.
        grid.addItemList({"1/16 beat","1/8 beat","1/4 beat","1 beat","1 bar","2 bars","4 bars"},1);
        grid.setSelectedId(3,juce::dontSendNotification);grid.setTooltip("Layout specimen only; no rhythmic-grid edit binding yet.");
        grid.setEnabled(false);
        add.setTooltip("Add a real manual fixture swell at the current test playhead");
        add.onClick=[this]{processor->swellEngine().addTarget(processor->getSongTime().sample);refresh();};
        setSize(1100,850);setScene(0);startTimerHz(10);
    }
    ~Composition()override
    {
        stopTimer();for(auto [component,parent]:borrowed)parent->addChildComponent(component);
        editor.reset();processor->setPlayHead(nullptr);processor.reset();setLookAndFeel(nullptr);
    }
    m3::SwellTimeline& timeline(){return *dynamic_cast<m3::SwellTimeline*>(parts.at("swellTimeline"));}
    void setScene(int next)
    {
        scene=next;auto state=baseline;
        if(scene==1){state.swells.clear();state.selected.clear();state.primary.clear();}
        if(scene==2 || scene==3)
        {
            state.swells.front().name="Selected fixture swell with a deliberately long descriptive name";
            const auto original=state.swells.front();
            for(int n=0;n<198;++n){auto w=original;w.id=juce::Uuid().toDashedString();
                w.name="Long fixture swell name with a deliberate truncation check "+juce::String(n+1);state.swells.push_back(w);}
            if(scene==3){state.selected={firstId,secondId};state.primary=firstId;}
        }
        processor->swellEngine().restore(state);
        if(scene!=1)wait([&]{const auto s=processor->swellEngine().snapshot();const auto* w=m3::find(s.session,s.session.primary);
            return w && w->phase==m3::Phase::ready && s.raw && s.auditionReady;});
        refresh();timeline().fitSelection();refreshZoom();
        revealSelection();
    }
    void revealSelection()
    {
        auto& list=require<m3::ui::controls::SwellList>(*this,"swellList");
        const auto primary=processor->swellEngine().savedSession().primary;const auto row=list.rowFor(primary);
        if(row>=0){list.scrollToEnsureRowIsOnscreen(row);REQUIRE(list.isRowSelected(row));
            REQUIRE(list.getLocalBounds().contains(list.getRowPosition(row,true)));}
    }
    void verifyLiveRefresh()
    {
        // Dispatch both timer and paint messages, unlike static snapshot checks.
        for(const auto specimen:{5,4,0})
        {
            setScene(specimen);const auto passes=layoutPasses;
            for(int i=0;i<90;++i)
            {
                juce::MessageManager::getInstance()->runDispatchLoopUntil(13);
                REQUIRE(scene==specimen);
                REQUIRE(layoutPasses==passes);
                auto& header=require<WorkflowHeader>(*this,"workflowHeader");
                REQUIRE(!header.modeSelector().segment(1)->isEnabled() && !header.modeSelector().segment(3)->isEnabled());
                if(specimen==4 || specimen==5)REQUIRE(timeline().grabPoints().empty());
                else REQUIRE(!timeline().grabPoints().empty());
            }
        }
    }
    void verifyNavigation()
    {
        setScene(0);const auto saved=processor->swellEngine().savedSession();const auto captureRevision=processor->getCaptureSnapshot().manifest.revision;
        const auto selected=m3::find(saved,saved.primary);REQUIRE(selected);
        const auto lane=timeline().sourceLane();
        const auto selectedPixel=timeline().viewport().pixel(selected->arrival,lane.getX(),lane.getWidth());
        timeline().zoom(.8);
        REQUIRE(std::abs(timeline().viewport().pixel(selected->arrival,lane.getX(),lane.getWidth())-selectedPixel)<.001);
        const auto playhead=processor->getSongTime().sample;
        timeline().setViewport({static_cast<double>(playhead)-10000,static_cast<double>(playhead)+10000});
        const auto playheadPixel=timeline().viewport().pixel(playhead,lane.getX(),lane.getWidth());timeline().zoom(.8);
        REQUIRE(std::abs(timeline().viewport().pixel(playhead,lane.getX(),lane.getWidth())-playheadPixel)<.001);
        timeline().setViewport({100000,140000});const auto centre=timeline().viewport().pixel(120000,lane.getX(),lane.getWidth());timeline().zoom(.8);
        REQUIRE(std::abs(timeline().viewport().pixel(120000,lane.getX(),lane.getWidth())-centre)<.001);
        const auto validView=timeline().viewport();timeline().setZoomFraction(std::numeric_limits<double>::quiet_NaN());
        REQUIRE(timeline().viewport().first==validView.first && timeline().viewport().last==validView.last);
        timeline().setZoomFraction(1);REQUIRE(std::abs(timeline().viewport().span()-timeline().minimumViewSpan())<.001);
        timeline().setZoomFraction(0);const auto bounds=timeline().navigationBounds();
        REQUIRE(timeline().viewport().first==bounds.first && timeline().viewport().last==bounds.last);
        const auto point=juce::Point<float>{lane.getX()+lane.getWidth()*.3f,lane.getCentreY()};
        const auto before=timeline().viewport().sample(point.x,lane.getX(),lane.getWidth());
        const auto now=juce::Time::getCurrentTime();
        juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(),point,juce::ModifierKeys::ctrlModifier,0,0,0,0,0,&timeline(),&timeline(),now,point,now,1,false);
        juce::MouseWheelDetails wheel{};wheel.deltaY=.1f;timeline().mouseWheelMove(event,wheel);
        REQUIRE(std::abs(timeline().viewport().sample(point.x,lane.getX(),lane.getWidth())-before)<=1);
        for(int n=0;n<100;++n)timeline().zoom(.01);
        REQUIRE(std::abs(timeline().viewport().span()-timeline().minimumViewSpan())<.001);
        timeline().navigateOverview(1);REQUIRE(timeline().viewport().last<=bounds.last);
        auto& overview=require<m3::SwellOverview>(*this,"songOverview");
        REQUIRE(overview.waveformBounds().contains(overview.detailBounds()));
        for(int n=0;n<100;++n)timeline().zoom(100);
        REQUIRE(timeline().viewport().first==bounds.first && timeline().viewport().last==bounds.last);
        const auto after=processor->swellEngine().savedSession();REQUIRE(saved.primary==after.primary && saved.selected==after.selected && saved.swells.size()==after.swells.size());
        for(size_t n=0;n<saved.swells.size();++n)
        {const auto& a=saved.swells[n];const auto& b=after.swells[n];
            REQUIRE(a.id==b.id && a.render==b.render && a.ir==b.ir && a.shape==b.shape && a.sourceStart==b.sourceStart && a.sourceEnd==b.sourceEnd && a.arrival==b.arrival && a.placement==b.placement);
            REQUIRE(a.inputGeneration==b.inputGeneration && a.shapeGeneration==b.shapeGeneration);}
        REQUIRE(processor->getCaptureSnapshot().manifest.revision==captureRevision);
    }
    void verifyGestures();
    void verifyEditorBindings();
    void verifyFit();
    void verifyKnobs();
    void verifyFocusedLevel();
    void verifyKnobResets();
    void verifyAudition();
    void verifyAuditionStrip();
    void verifyReadability();
    void verifyHoverHelp();
    void verifyResponsive(const juce::File& output);
    void verifyResponsiveScale();
    std::unique_ptr<PreverbAudioProcessorEditor> productionEditor()
    {stopTimer();return std::make_unique<PreverbAudioProcessorEditor>(*processor);}
    void refresh()
    {
        editor->refresh();const auto state=processor->swellEngine().snapshot();
        auto presentation=presentWorkflow(processor->getWorkflowSnapshot(),processor->getWorkflowReadiness());
        for(auto& choice:presentation.choices)if(choice.mode!=workflow::Mode::live)
        {choice.available=false;choice.reason="Isolated Perform composition review";}
        require<WorkflowHeader>(*this,"workflowHeader").display(presentation);
        countLabel.setText("("+juce::String(state.session.swells.size())+")",juce::dontSendNotification);
        monitor.setText(state.dryStatus.isNotEmpty()?state.dryStatus:"Preview only | captured dry and prepared wet | host routing unchanged",juce::dontSendNotification);
        emptyList.setVisible(scene==1);emptyInspector.setVisible(scene==1);
        parts.at("inspectorViewport")->setVisible(scene!=1);
        const bool noAudio=scene==1;emptyTimeline.setVisible(noAudio);timeline().setVisible(!noAudio);
        if(noAudio)
        {
            timeline().setData({}, {},processor->getSongTime(),false);
            require<m3::ui::controls::SwellList>(*this,"swellList").display({}, {},{});
        }
        else if(scene==4 || scene==5)
        {
            // Controlled loading/error display injection; audio/shape remains real.
            auto shown=state;for(auto& w:shown.session.swells){w.phase=scene==4?m3::Phase::rendering:m3::Phase::error;w.status=scene==4?"Preparing wet render":"Fixture: unavailable render";}
            shown.raw.reset();shown.shaped.reset();timeline().setData(processor->getCaptureSnapshot(),shown,processor->getSongTime());
            auto& status=require<juce::Label>(*parts.at("inspectorViewport"),"selectedStatus");
            status.setText(scene==4?"Rendering - fixture state":"Error - fixture state",juce::dontSendNotification);
            parts.at("inspectorViewport")->setEnabled(false);
            std::vector<m3::ui::controls::SwellListRow> rows;
            for(const auto& w:shown.session.swells)rows.push_back({w.id,w.name,w.irName,w.status,w.arrival,w.enabled,false,false,w.frozen,scene==5});
            require<m3::ui::controls::SwellList>(*this,"swellList").display(std::move(rows),shown.session.selected,shown.session.primary,shown.sessionEpoch);
        }
        else parts.at("inspectorViewport")->setEnabled(true);
        refreshZoom();
        // Bindings repaint changed data. Geometry belongs only to resized(), not
        // the polling loop: reapplying every child font/bounds caused extra work.
    }
    void resized()override
    {
        if(parts.empty())return;++layoutPasses;Layout l(getLocalBounds());
        parts.at("workflowHeader")->setBounds(l.header);
        const auto selector=l.header.getWidth()<1200?330:360;
        parts.at("workspaceNotice")->setBounds(l.header.getX(),l.header.getY()+46,l.header.getWidth()-36-12-selector-16,32);
        auto left=l.list.reduced(0,spacing::related);const auto titleY=left.getY();auto title=left.removeFromTop(36);title.removeFromRight(42);countLabel.setBounds(title.removeFromRight(44));swellsHeading.setBounds(title);add.setBounds(l.list.getRight()-36,titleY+2,34,34);
        parts.at("swellList")->setBounds(left.withTrimmedTop(8));emptyList.setBounds(left.reduced(6,30));
        auto right=l.inspector.reduced(0,spacing::related);inspectorHeading.setBounds(right.removeFromTop(36));right.removeFromTop(8);
        parts.at("inspectorViewport")->setBounds(right);emptyInspector.setBounds(right);
        layoutInspector(right.getWidth());
        timeline().setBounds(l.detail.withTrimmedTop(spacing::related));parts.at("editorReadout")->setBounds(l.readout);emptyTimeline.setBounds(l.detail.reduced(16));
        auto row=l.toolbar.reduced(performGeometry::toolbarInset);grid.setBounds(row.removeFromRight(performGeometry::gridWidth));row.removeFromRight(performGeometry::toolbarGap);
        snap.setBounds(row.removeFromRight(performGeometry::snapWidth));row.removeFromRight(performGeometry::toolbarGap);zoom.setBounds(row);
        auto audition=l.audition.reduced(0,8);auto line=audition.removeFromTop(36);
        auditionHeading.setBounds(line.removeFromLeft(112));parts.at("audition")->setBounds(line.removeFromLeft(50));line.removeFromLeft(8);
        parts.at("stopAudition")->setBounds(line.removeFromLeft(50));line.removeFromLeft(24);
        parts.at("auditionWetOnly")->setBounds(line.removeFromLeft(controlGeometry::valueWidth+2*spacing::related));
        monitor.setBounds(audition.withTrimmedLeft(112));
        parts.at("songOverview")->setBounds(l.overview.reduced(0,8));parts.at("hostInfo")->setBounds(l.info);
    }
    void layoutInspector(int width)
    {
        auto& viewport=*dynamic_cast<juce::Viewport*>(parts.at("inspectorViewport"));auto* content=viewport.getViewedComponent();REQUIRE(content);
        content->setSize(width-16,650);auto area=content->getLocalBounds().reduced(2,4);
        auto& status=require<juce::Label>(*content,"selectedStatus");status.setBounds(area.removeFromTop(24));applyTextRole(status,TextRole::helper);
        auto flags=area.removeFromTop(36);for(auto* c:content->getChildren())if(auto* b=dynamic_cast<juce::ToggleButton*>(c))
            if(b->getButtonText()=="Enabled" || b->getButtonText()=="Solo" || b->getButtonText()=="Freeze")b->setBounds(flags.removeFromLeft((width-20)/3));
        require<juce::Label>(*content,"performSoundScope").setBounds(area.removeFromTop(controlGeometry::feedbackHeight));
        auto& ir=require<controls::ConvolutionIrEditor>(*content,"selectedIrEditor");ir.setBounds(area.removeFromTop(ir.preferredHeight(area.getWidth())));area.removeFromTop(spacing::related);
        const char* names[]={"arrivalSample","wetPlacement","sourceSampleStart","sourceSampleEnd","preRoll","endOffset","fadeInMs","fadeOutMs","curveInTension","curveOutTension","swellLevel"};
        for(int i=0;i<11;i+=2)
        {
            auto row=area.removeFromTop(64);auto first=row.removeFromLeft((row.getWidth()-8)/2);row.removeFromLeft(8);
            for(int n=i;n<std::min(i+2,11);++n)
            {
                auto& cell=require<KnobValue>(*content,juce::String(names[n])+"Cell");cell.setBounds(n==i?first:row);
                // Test-only production-role composition; no shared KnobValue mutation.
                auto a=cell.getLocalBounds();cell.valueKnob().setBounds(a.removeFromLeft(42).withSizeKeepingCentre(42,42));a.removeFromLeft(6);
                for(auto* c:cell.getChildren())if(auto* label=dynamic_cast<juce::Label*>(c)){applyTextRole(*label,TextRole::controlLabel);label->setBounds(a.removeFromTop(24));}
                cell.valueField().setBounds(a.removeFromTop(28));
            }
        }
        area.removeFromTop(12);auto reset=area.removeFromTop(34);
        require<juce::TextButton>(*content,"resetWetShape").setBounds(reset.removeFromLeft((reset.getWidth()-8)/2));reset.removeFromLeft(8);
        require<juce::TextButton>(*content,"resetSwellMenu").setBounds(reset);
    }
    void paint(juce::Graphics& g)override
    {
        g.fillAll(background);Layout l(getLocalBounds());g.setColour(separator);
        for(int y:{l.header.getBottom(),l.work.getBottom(),l.audition.getBottom(),l.overview.getBottom()})
            g.drawHorizontalLine(y,static_cast<float>(l.header.getX()),static_cast<float>(l.header.getRight()));
        for(int x:{l.list.getRight()+8,l.inspector.getX()-8})
            g.drawVerticalLine(x,static_cast<float>(l.work.getY()+spacing::related),static_cast<float>(l.work.getBottom()-spacing::related));
    }
    juce::var measure()
    {
        Layout l(getLocalBounds());auto* o=new juce::DynamicObject;
        const auto rect=[&](const char* name,juce::Rectangle<int> r){o->setProperty(name,juce::var(juce::Array<juce::var>{r.getX(),r.getY(),r.getWidth(),r.getHeight()}));};
        rect("size",getLocalBounds());rect("list",l.list);rect("inspector",l.inspector);rect("timeline",timeline().getBounds());
        rect("sourceLane",timeline().sourceLane().toNearestInt());rect("wetLane",timeline().wetLane().toNearestInt());rect("toolbar",l.toolbar);
        rect("overview",parts.at("songOverview")->getBounds());o->setProperty("scene",scene);
        REQUIRE(l.detail.getWidth()>=552 && l.detail.getHeight()>=370);
        REQUIRE(l.toolbar.contains(zoom.getBounds()) && l.toolbar.contains(grid.getBounds()));
        REQUIRE(zoom.getWidth()>=controls::TimelineZoomControls::minimumWidth());
        std::array<juce::Component*,6> controls{&zoom.zoomOutButton(),&zoom.zoomSlider(),&zoom.zoomInButton(),&zoom.fitButton(),&snap,&grid};
        for(size_t i=0;i<controls.size();++i){REQUIRE(controls[i]->getHeight()==36 && controls[i]->getWidth()>=34);
            const auto bounds=getLocalArea(controls[i],controls[i]->getLocalBounds());
            const auto focus=bounds.toFloat().expanded(controlGeometry::focusOutset+controlGeometry::focusStroke*.5f);
            REQUIRE(l.toolbar.toFloat().contains(focus));
            if(controls[i]->isEnabled())REQUIRE(controls[i]->getWantsKeyboardFocus());
            for(size_t j=0;j<i;++j)REQUIRE(!bounds.intersects(getLocalArea(controls[j],controls[j]->getLocalBounds())));}
        REQUIRE(timeline().sourceLane().getHeight()>=90 && timeline().wetLane().getHeight()>=90);
        REQUIRE(swellsHeading.getFont().getTypefacePtr()==interTypeface(InterFace::semibold));
        REQUIRE(countLabel.getFont().getTypefacePtr()==interTypeface(InterFace::regular));
        REQUIRE(swellsHeading.getFont().getHeight()==21 && countLabel.getFont().getHeight()==14);
        REQUIRE(timeline().getY()>=l.header.getBottom()+spacing::related);
        REQUIRE(parts.at("swellList")->getBottom()<=l.work.getBottom()-spacing::related);
        REQUIRE(parts.at("inspectorViewport")->getBottom()<=l.work.getBottom()-spacing::related);
        REQUIRE(!find(*this,"autoMarkers") && !find(*this,"markerSensitivity"));
        REQUIRE(parts.at("songOverview")->getBottom()<=l.overview.getBottom()-8);
        REQUIRE(parts.at("audition")->getY()>=l.audition.getY()+8);
        REQUIRE(!snap.isEnabled() && !grid.isEnabled()); // Layout specimens cannot imply implemented edits.
        auto& header=require<WorkflowHeader>(*this,"workflowHeader");
        for(auto* c:header.getChildren())REQUIRE(header.getLocalBounds().contains(c->getBounds()));
        for(auto* c:find(*parts.at("inspectorViewport"),"arrivalSampleCell")->getChildren())
            if(auto* label=dynamic_cast<juce::Label*>(c))REQUIRE(label->getFont().getTypefacePtr()==interTypeface(InterFace::medium));
        return juce::var(o);
    }
private:
    void refreshZoom()
    {
        const auto span=timeline().viewport().span();
        zoom.display({timeline().zoomFraction(),span>timeline().minimumViewSpan(),span<timeline().navigationBounds().span(),scene!=1});
    }
    void timerCallback()override{refresh();}
    TestDirectory directory;WorkspaceTheme theme;Head head;
    std::unique_ptr<PreverbAudioProcessor> processor;std::unique_ptr<PreverbAudioProcessorEditor> editor;
    std::map<juce::String,juce::Component*> parts;std::vector<std::pair<juce::Component*,juce::Component*>> borrowed;
    m3::Session baseline;juce::String firstId,secondId;int scene=0;int layoutPasses=0;
    juce::Label swellsHeading,countLabel,inspectorHeading,auditionHeading,emptyList,emptyInspector,emptyTimeline,monitor;
    juce::TextButton snap{"Snap"},add{"+"};
    controls::TimelineZoomControls zoom;InvocationComboBox grid;
};

#include "PerformGestureCases.h"
#include "PerformEditorBindingCases.h"
#include "PerformFitCases.h"
#include "PerformKnobCases.h"
#include "FocusedLevelCases.h"
#include "PerformKnobResetCases.h"
#include "PerformAuditionCases.h"
#include "PerformAuditionStripCases.h"
#include "PerformReadabilityCases.h"
#include "PerformHoverHelpCases.h"
#include "ResponsiveEditorCases.h"
#include "ResponsiveScaleCases.h"
class Review final:public juce::Component
{
public:
    Composition composition;
    explicit Review(bool independentRefresh=true):composition(independentRefresh)
    {
        addAndMakeVisible(composition);addAndMakeVisible(scenes);addAndMakeVisible(sizes);addAndMakeVisible(note);
        scenes.addItemList({"Sparse / selected","Empty","Dense / long labels","Existing multiple selection","Rendering display","Error display"},1);
        scenes.setSelectedId(1,juce::dontSendNotification);scenes.onChange=[this]{composition.setScene(scenes.getSelectedId()-1);};
        sizes.addItemList({"1100 x 850","1400 x 900","1400 x 1000","1600 x 1000"},1);sizes.setSelectedId(1,juce::dontSendNotification);
        sizes.onChange=[this]{const std::array<juce::Point<int>,4> dimensions{{{1100,850},{1400,900},{1400,1000},{1600,1000}}};
            const auto p=dimensions[static_cast<size_t>(sizes.getSelectedId()-1)];setSize(p.x,p.y+44);};
        note.setText("Layout checkpoint | real test audio | new Snap/grid not connected",juce::dontSendNotification);applyTextRole(note,TextRole::helper);
        setSize(1100,894);
    }
    void resized()override{auto a=getLocalBounds();auto row=a.removeFromTop(44).reduced(6);scenes.setBounds(row.removeFromLeft(210));row.removeFromLeft(8);sizes.setBounds(row.removeFromLeft(146));row.removeFromLeft(12);note.setBounds(row);composition.setBounds(a);}
private:juce::ComboBox scenes,sizes;juce::Label note;
};
class IntegratedReview final:public juce::Component
{
public:
    explicit IntegratedReview(std::unique_ptr<Review> scope,bool slimAudition=false,bool selectionReview=false,bool compact=false,bool irReview=false,bool levelReview=false):fixture(std::move(scope))
    {
        editor=fixture->composition.productionEditor();addAndMakeVisible(*editor);
        if(compact)editor->enableResponsiveReview();
        if(irReview || levelReview){require<controls::SegmentedSelector>(*editor,"learnSections").onRequest(2);require<controls::SegmentedSelector>(*editor,"inspectorSections").onRequest(3);}
        addAndMakeVisible(scenes);addAndMakeVisible(sizes);addAndMakeVisible(note);
        scenes.addItemList({"Sparse / selected","Empty swells","Dense / long labels"},1);scenes.setSelectedId(1,juce::dontSendNotification);
        scenes.onChange=[this]{fixture->composition.setScene(scenes.getSelectedId()-1);editor->refresh();};
        if(levelReview)
        {
            scenes.clear();scenes.addItemList({"Level -12 dB","Level 0 dB","Level +12 dB","Extended -30 dB","Extended -60 dB"},1);
            scenes.setSelectedId(2,juce::dontSendNotification);
            scenes.onChange=[this]{const std::array<float,5> values{-12,0,12,-30,-60};auto& cell=require<KnobValue>(*editor,"swellLevelCell");
                cell.valueField().setText(juce::String(values[static_cast<size_t>(scenes.getSelectedId()-1)]),false);cell.valueField().onReturnKey();};
        }
        if(compact)sizes.addItemList({"900 x 600","1000 x 700","1100 x 850","1400 x 900"},1);
        else sizes.addItemList({"1100 x 850","1400 x 900","1400 x 1000","1600 x 1000"},1);
        sizes.setSelectedId(1,juce::dontSendNotification);
        sizes.onChange=[this,compact]{const std::array<juce::Point<int>,4> dimensions=compact?std::array<juce::Point<int>,4>{{{900,600},{1000,700},{1100,850},{1400,900}}}:std::array<juce::Point<int>,4>{{{1100,850},{1400,900},{1400,1000},{1600,1000}}};const auto size=dimensions[static_cast<size_t>(sizes.getSelectedId()-1)];if(compact){editor->setPresentationSize(size.x,size.y);setSize(editor->getWidth(),editor->getHeight()+44);}else setSize(size.x,size.y+44);};
        applyTextRole(note,TextRole::helper);note.setText(irReview?"#80 | Shared IR in Learn / Perform Sound | no audio device":compact?"#58 combined #51 / #68 / #73 / #93 | no audio device":selectionReview?"#93 | Select a swell to auto-Fit | Pan/zoom away and re-click to Fit again | no audio device":slimAudition?"Perform clarity checkpoint | 52 px audition strip | no audio device":"Integrated Perform | Snap/grid connected | no audio device",juce::dontSendNotification);setSize(compact?900:1100,compact?644:894);
        if(selectionReview){auto& zoom=require<controls::TimelineZoomControls>(*editor,"performZoom");zoom.beforeInvocation();zoom.onZoomFraction(.55);}
        if(levelReview)note.setText("#108 | Focused dial / extended entry | native control review",juce::dontSendNotification);
    }
    void resized()override
    {
        {const juce::ScopedValueSetter<bool> guard(layingOutEditor,true);auto area=getLocalBounds();auto strip=area.removeFromTop(44).reduced(6);scenes.setBounds(strip.removeFromLeft(210));strip.removeFromLeft(8);sizes.setBounds(strip.removeFromLeft(146));strip.removeFromLeft(12);note.setBounds(strip);editor->setBounds(area);}
        if(editor->getWidth()!=getWidth() || editor->getHeight()+44!=getHeight())setSize(editor->getWidth(),editor->getHeight()+44);
    }
    void childBoundsChanged(juce::Component* child)override
    {if(child==editor.get() && !layingOutEditor)setSize(editor->getWidth(),editor->getHeight()+44);}
    void verifyCompactBaseline()const{REQUIRE(editor->getWidth()==900 && editor->getHeight()==600);}
    void capture(const juce::File& directory)
    {
        REQUIRE(directory.createDirectory().wasOk());editor->stopAutomaticRefreshForReview();
        for(int scene=0;scene<3;++scene)
        {fixture->composition.setScene(scene);editor->refresh();editor->setSize(1100,850);const auto snapshot=editor->createComponentSnapshot(editor->getLocalBounds());
            auto output=directory.getChildFile(juce::String(scene==0?"selected":scene==1?"empty":"dense")+"-1100.png").createOutputStream();REQUIRE(output && juce::PNGImageFormat().writeImageToStream(snapshot,*output));
            if(scene==0){auto& control=require<controls::LatchingActionButton>(*editor,"performSnap");const auto point=control.getLocalBounds().getCentre().toFloat();const auto now=juce::Time::getCurrentTime();
                editor->mouseEnter(juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,{},0,0,0,0,0,&control,&control,now,point,now,0,false));
                const auto hovered=editor->createComponentSnapshot(editor->getLocalBounds());auto help=directory.getChildFile("hover-help-1100.png").createOutputStream();REQUIRE(help && juce::PNGImageFormat().writeImageToStream(hovered,*help));
                editor->mouseExit(juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),{-100,-100},{},0,0,0,0,0,editor.get(),editor.get(),now,{-100,-100},now,0,false));}
        }
    }
private:
    std::unique_ptr<Review> fixture;
    std::unique_ptr<PreverbAudioProcessorEditor> editor;
    bool layingOutEditor=false;
    juce::ComboBox scenes,sizes;juce::Label note;
};
}

int main(int argc,char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    return runTest([&]
    {
        using namespace preview46;const auto command=argc>1?juce::String(argv[1]):juce::String{};
        auto content=std::make_unique<Review>(command!="--test-competing-refresh");
        if(command=="--test-responsive"){content->composition.verifyResponsive(argc>2?juce::File(argv[2]):juce::File{});std::cout<<"Compact editor actual control bounds, divider clearance, mode routes and source/render identity checks passed.\n";return;}
        if(command=="--test-responsive-scale"){content->composition.verifyResponsiveScale();std::cout<<"Actual editor scale/input/preferences/recovery/upstream integration checks passed.\n";return;}
        if(command=="--capture-integrated" || command=="--capture-audition-strip"){REQUIRE(argc==3);IntegratedReview review(std::move(content),command=="--capture-audition-strip");review.capture(juce::File(argv[2]));return;}
        if(command=="--test-level"){content->composition.verifyFocusedLevel();std::cout<<"Focused Level endpoints, extended persistence, absolute accessibility and stale-binding checks passed.\n";return;}
        if(command=="--review-integrated" || command=="--review-audition-strip" || command=="--review-selection" || command=="--review-responsive" || command=="--review-ir-integrated" || command=="--review-level")
        {
            class Window final:public juce::DocumentWindow
            {
            public:explicit Window(std::unique_ptr<Review> scope,bool slim,bool selection,bool compact,bool irReview,bool levelReview):DocumentWindow(levelReview?"Issue #108 - Focused Level review":irReview?"Issue #80 - Shared IR integrated review":compact?"Issue #58 - Compact responsive UI checkpoint":selection?"Issue #93 - Swell selection navigation":slim?"Issue #46 - Audition strip layout checkpoint":"Issue #46 - Integrated Perform review",background,closeButton)
                {setUsingNativeTitleBar(true);auto* review=new IntegratedReview(std::move(scope),slim,selection,compact,irReview,levelReview);setContentOwned(review,true);setResizable(true,false);setResizeLimits(compact?900:1100,compact?644:894,2000,1444);centreWithSize(compact?900:1100,compact?644:924);if(compact)review->verifyCompactBaseline();setVisible(true);}
                void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
            };
            Window window(std::move(content),command=="--review-audition-strip",command=="--review-selection",command=="--review-responsive" || command=="--review-ir-integrated" || command=="--review-level",command=="--review-ir-integrated",command=="--review-level");juce::MessageManager::getInstance()->runDispatchLoop();return;
        }
        if(command=="--test-navigation"){content->composition.verifyNavigation();std::cout<<"Perform navigation anchor/limits/Overview/identity checks passed.\n";return;}
        if(command=="--test-gestures"){content->composition.verifyGestures();std::cout<<"Perform grid/translation/modifier/stale-owner/source checks passed.\n";return;}
        if(command=="--test-editor"){content->composition.verifyEditorBindings();std::cout<<"Perform actual editor layout/navigation/restore/Export binding checks passed.\n";return;}
        if(command=="--test-fit"){content->composition.verifyFit();std::cout<<"Perform selected/pending/overlap/sparse/empty Fit checks passed.\n";return;}
        if(command=="--test-knobs"){content->composition.verifyKnobs();std::cout<<"Perform knob grid/accepted feedback/guards/focus/accessibility/font checks passed.\n";return;}
        if(command=="--test-knob-resets"){content->composition.verifyKnobResets();std::cout<<"Perform knob double-click/menu/default/initial/stale reset checks passed.\n";return;}
        if(command=="--test-audition"){content->composition.verifyAudition();std::cout<<"Perform looping audition/toggle/live gain/legacy/stale/status checks passed.\n";return;}
        if(command=="--test-audition-strip"){content->composition.verifyAuditionStrip();std::cout<<"Perform slim audition row/gaps/divider/heading checks passed.";return;}
        if(command=="--test-readability"){content->composition.verifyReadability();std::cout<<"Perform precision/ruler/grouping/mode/footer checks passed.";return;}
        if(command=="--test-hover-help"){content->composition.verifyHoverHelp();std::cout<<"Perform bottom hover-help routing/lifetime/layout checks passed.";return;}
        if(command=="--test-refresh" || command=="--test-competing-refresh")
        {content->composition.verifyLiveRefresh();std::cout<<"Live refresh state/idle-layout checks passed.\n";return;}
        if(argc>1 && juce::String(argv[1])=="--review")
        {
            class Window final:public juce::DocumentWindow
            {
            public:Window(std::unique_ptr<Review> c):DocumentWindow("Issue #46 - Perform layout checkpoint",background,closeButton)
                {setUsingNativeTitleBar(true);setContentOwned(c.release(),true);setResizable(true,false);setResizeLimits(1100,894,2000,1444);centreWithSize(1100,924);setVisible(true);}
                void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
            };
            WorkspaceTheme theme;Window window(std::move(content));window.setLookAndFeel(&theme);juce::MessageManager::getInstance()->runDispatchLoop();window.setLookAndFeel(nullptr);return;
        }
        auto& composition=content->composition;
        const auto output=juce::SystemStats::getEnvironmentVariable("PREVERB_PERFORM_OUTPUT",{});juce::Array<juce::var> measurements;
        juce::File directory(output);if(output.isNotEmpty())REQUIRE(directory.createDirectory().wasOk());
        const char* scenes[]={"sparse","empty","dense","multiple","rendering","error"};
        for(int scene=0;scene<6;++scene)
        {
            composition.setScene(scene);
            for(auto size:std::array<juce::Point<int>,10>{{{1100,850},{1199,850},{1200,850},{1201,850},{1239,850},{1240,850},{1241,850},{1400,900},{1400,1000},{1600,1000}}})
            {
                composition.setSize(size.x,size.y);composition.refresh();composition.revealSelection();measurements.add(composition.measure());
                for(float scale:{1.f,1.25f,1.5f,2.f})
                {
                    const auto picture=composition.createComponentSnapshot(composition.getLocalBounds(),true,scale);REQUIRE(picture.isValid());
                    REQUIRE(picture.getWidth()==juce::roundToInt(size.x*scale));
                    if(scale==1.f)
                    {
                        const Layout l(composition.getLocalBounds());
                        // The complete rules must survive opaque child painting.
                        for(const int y:{l.header.getBottom(),l.work.getBottom(),l.audition.getBottom(),l.overview.getBottom()})
                            for(int x=l.header.getX();x<l.header.getRight();++x)REQUIRE(picture.getPixelAt(x,y)==separator);
                        for(const int x:{l.list.getRight()+8,l.inspector.getX()-8})
                        {
                            REQUIRE(picture.getPixelAt(x,l.work.getY()+6)==background);
                            REQUIRE(picture.getPixelAt(x,l.work.getBottom()-6)==background);
                        }
                        REQUIRE(picture.getPixelAt(l.overview.getCentreX(),l.overview.getBottom()-4)==background);
                    }
                    if(output.isNotEmpty() && (size.x==1100 || (size.x==1600 && (scene==0 || scene==2))))
                    {
                        auto stream=directory.getChildFile(juce::String(scenes[scene])+"-"+juce::String(size.x)+"x"+juce::String(size.y)+"-"+juce::String(juce::roundToInt(scale*100))+".png").createOutputStream();
                        REQUIRE(stream && juce::PNGImageFormat().writeImageToStream(picture,*stream));
                    }
                }
            }
        }
        if(output.isNotEmpty())REQUIRE(directory.getChildFile("measurements.json").replaceWithText(juce::JSON::toString(juce::var(measurements))));
        composition.verifyLiveRefresh();
        std::cout<<"Perform composition: 60 geometry/typeface/focus cases; 240 software-scale renders and live refresh checks passed.\n";
    });
}
