// Issue #46 isolated native layout checkpoint. No production layout/control changes.
#include "AudioTestFiles.h"
#include "WorkflowTestSupport.h"
#include <Plugin/PluginEditor.h>
#include <UI/Controls/TimelineZoomControls.h>
#include <UI/PerformLayout.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <map>

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
{auto* result=dynamic_cast<T*>(find(root,name));REQUIRE(result);return *result;}
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
            processor->processBlock(audio,midi);juce::Thread::sleep(1);
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
                "inspectorViewport","audition","stopAudition","auditionBlend","resetAuditionTrim","hostInfo",
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
        parts.at("resetAuditionTrim")->setBounds(line.removeFromRight(138));line.removeFromRight(16);
        parts.at("auditionBlend")->setBounds(line);
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
        auto& irStatus=require<juce::Label>(*content,"irAssignmentStatus");irStatus.setBounds(area.removeFromTop(24));applyTextRole(irStatus,TextRole::helper);
        auto ir=area.removeFromTop(34);require<juce::TextButton>(*content,"loadIR").setBounds(ir.removeFromRight(34));ir.removeFromRight(6);
        require<juce::ComboBox>(*content,"selectedIR").setBounds(ir);area.removeFromTop(12);
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
}

int main(int argc,char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    return runTest([&]
    {
        using namespace preview46;const auto command=argc>1?juce::String(argv[1]):juce::String{};
        auto content=std::make_unique<Review>(command!="--test-competing-refresh");
        if(command=="--test-navigation"){content->composition.verifyNavigation();std::cout<<"Perform navigation anchor/limits/Overview/identity checks passed.\n";return;}
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
