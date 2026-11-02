#include "AudioTestFiles.h"
#include <Plugin/PluginEditor.h>
#include <source_location>
#include <map>

struct UIPlayHead final:juce::AudioPlayHead
{PositionInfo position;juce::Optional<PositionInfo> getPosition()const override{return position;}};
template<class F>static void waitUI(F f,const std::source_location loc=std::source_location::current())
{const auto end=juce::Time::getMillisecondCounter()+15000;while(!f()){if(juce::Time::getMillisecondCounter()>end)throw std::runtime_error("UI timeout "+std::to_string(loc.line()));juce::Thread::sleep(2);}}
static juce::Component* named(juce::Component& root,const juce::String& name)
{if(root.getName()==name)return &root;for(auto child:root.getChildren())if(auto result=named(*child,name))return result;return nullptr;}
static juce::TextButton* button(juce::Component& root,const juce::String& text)
{for(auto child:root.getChildren()){if(auto b=dynamic_cast<juce::TextButton*>(child);b && b->getButtonText()==text)return b;if(auto b=button(*child,text))return b;}return nullptr;}
static bool visibleOffscreen(juce::Component* control)
{for(;control;control=control->getParentComponent())if(!control->isVisible())return false;return true;}
static void snapshotEditor(PreverbAudioProcessorEditor& editor,const juce::String& tag)
{
    // A state or recovery panel may replace content, never the region geometry.
    std::array<juce::Rectangle<int>,4> regions;
    int index=0;for(const auto* name:{"swellList","swellTimeline","inspectorViewport","songOverview"})
    {const auto c=named(editor,name);REQUIRE(c);regions[static_cast<size_t>(index++)]=c->getBounds();}
    static std::map<std::pair<int,int>,decltype(regions)> geometry;
    const auto key=std::pair{editor.getWidth(),editor.getHeight()};
    if(const auto old=geometry.find(key);old!=geometry.end())REQUIRE(old->second==regions);else geometry.emplace(key,regions);
    const auto folder=juce::SystemStats::getEnvironmentVariable("PREVERB_UI_M3_OUTPUT",{});if(folder.isEmpty())return;
    juce::File directory(folder);REQUIRE(directory.createDirectory().wasOk());
    juce::Image image(juce::Image::ARGB,editor.getWidth(),editor.getHeight(),true,juce::SoftwareImageType{});juce::Graphics graphics(image);editor.paintEntireComponent(graphics,true);
    auto stream=directory.getChildFile(tag+".png").createOutputStream();REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk());REQUIRE(juce::PNGImageFormat().writeImageToStream(image,*stream));
    if(tag=="fresh-1400" || tag=="ready")for(const auto scale:{1.25f,1.5f,2.0f})
    {
        juce::Image scaled(juce::Image::ARGB,juce::roundToInt(editor.getWidth()*scale),juce::roundToInt(editor.getHeight()*scale),true,juce::SoftwareImageType{});
        juce::Graphics canvas(scaled);canvas.addTransform(juce::AffineTransform::scale(scale));editor.paintEntireComponent(canvas,true);
        auto output=directory.getChildFile(tag+"-scale-"+juce::String(juce::roundToInt(scale*100))+".png").createOutputStream();
        REQUIRE(output && output->setPosition(0) && output->truncate().wasOk());REQUIRE(juce::PNGImageFormat().writeImageToStream(scaled,*output));
    }
}
int main(){return runTest([]
{
    juce::ScopedJuceInitialiser_GUI gui;
    // A fresh instance must not inherit capture or status from the long-lived
    // editing fixture below. Keep matched baseline states independently useful.
    {
        TestDirectory freshRoot;auto freshOwner=std::make_unique<PreverbAudioProcessor>(freshRoot.file);auto& fresh=*freshOwner;
        REQUIRE(fresh.getName()=="Preverb Spike" && fresh.getParameters().isEmpty() && fresh.getNumPrograms()==1);
        REQUIRE(fresh.getCaptureSnapshot().manifest.ranges.empty());
        PreverbAudioProcessorEditor empty(fresh);empty.setVisible(true);empty.refresh();
        const auto* mark=dynamic_cast<m3::ui::Wordmark*>(named(empty,"brandWordmark"));REQUIRE(mark && mark->loaded());
        const auto* theme=dynamic_cast<m3::ui::WorkspaceTheme*>(&empty.getLookAndFeel());
        REQUIRE(theme && theme->interAvailable() && theme->font(13).getTypefacePtr()->getName().containsIgnoreCase("Inter"));
        const auto surface=empty.createComponentSnapshot(empty.getLocalBounds());
        REQUIRE(surface.getPixelAt(0,0)==m3::ui::background && surface.getPixelAt(empty.getWidth()-1,0)==m3::ui::background);
        REQUIRE(fresh.swellEngine().savedSession().swells.empty());
        REQUIRE(fresh.getCaptureSnapshot().manifest.ranges.empty() && !fresh.swellEngine().snapshot().playing);
        for(const auto size:{juce::Point<int>{1100,850},juce::Point<int>{1400,1000}})
        {empty.setSize(size.x,size.y);empty.refresh();snapshotEditor(empty,"fresh-"+juce::String(size.x));}
        juce::MemoryBlock saved;fresh.getStateInformation(saved);
        const auto xml=juce::AudioProcessor::getXmlFromBinary(saved.getData(),static_cast<int>(saved.getSize()));
        REQUIRE(xml && xml->hasTagName("PreverbState") && xml->getIntAttribute("stateVersion")==7);
    }
    TestDirectory root;auto processor=std::make_unique<PreverbAudioProcessor>(root.file);
    processor->swellEngine().setAutomaticProcessing(false);processor->swellEngine().setIsolateSelected(true);
    UIPlayHead head;head.position.setBpm(120);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});head.position.setIsPlaying(true);
    processor->prepareToPlay(48000,4096);processor->setPlayHead(&head);juce::MidiBuffer midi;juce::AudioBuffer<float> buffer(2,4096);
    processor->setMappingEnabled(true);
    for(int block=0;block<24;++block)
    {
        const auto start=block*4096;head.position.setTimeInSamples(start);head.position.setPpqPosition(start/24000.0);buffer.clear();
        for(int n=0;n<4096;++n){const auto local=(start+n)%48000;if(local<24000){const auto value=static_cast<float>(0.2*std::exp(-local/7000.0)*std::sin(local*0.05));buffer.setSample(0,n,value);buffer.setSample(1,n,value*0.6f);}}
        processor->processBlock(buffer,midi);juce::Thread::sleep(1);
    }
    processor->setMappingEnabled(false);REQUIRE(processor->flushCaptureForTesting());head.position.setIsPlaying(false);buffer.clear();processor->processBlock(buffer,midi);
    auto& engine=processor->swellEngine();
    {
        PreverbAudioProcessorEditor mapped(*processor);mapped.setVisible(true);mapped.setSize(1400,1000);mapped.refresh();
        REQUIRE(!processor->getCaptureSnapshot().manifest.ranges.empty() && engine.savedSession().swells.empty());
        snapshotEditor(mapped,"mapped-unselected");
    }
    REQUIRE(engine.addTarget(0));const auto first=engine.savedSession().primary;REQUIRE(engine.addTarget(48000));const auto second=engine.savedSession().primary;
    waitUI([&]{const auto state=engine.snapshot();return state.session.defaultIR.present() || state.defaultIRError.isNotEmpty();});
    if(engine.snapshot().defaultIRError.isNotEmpty())std::cerr<<engine.snapshot().defaultIRError<<"\n";
    REQUIRE(engine.savedSession().defaultIR.present());
    REQUIRE(engine.selectTargets({first,second},first));std::vector<float> ir(24000);for(size_t n=0;n<ir.size();++n)ir[n]=static_cast<float>(0.5*std::exp(-static_cast<double>(n)/3500)*std::sin(n*0.05));
    const auto file=root.file.getChildFile("musical-ir.wav");writeTestWav(file,1,48000,ir);engine.loadIR(file);
    waitUI([&]{return engine.savedSession().swells[0].irName=="musical-ir.wav" && engine.savedSession().swells[1].irName=="musical-ir.wav";});engine.generate(true);
    waitUI([&]{auto s=engine.snapshot();return s.auditionReady && s.session.swells[1].phase==m3::Phase::ready;});
    REQUIRE(engine.snapshot().raw->peak>1);
    const auto baseline=engine.savedSession();
    PreverbAudioProcessorEditor editor(*processor);editor.setVisible(true);editor.refresh();snapshotEditor(editor,"ready");
    const auto afterOpening=engine.savedSession();
    for(size_t i=0;i<baseline.swells.size();++i)
    {const auto& current=afterOpening.swells[i];const auto& prior=baseline.swells[i];
        REQUIRE(current.id==prior.id && current.ir==prior.ir && current.render==prior.render && current.shape==prior.shape);}
    {
        const auto enter=[&](const char* name,const char* value)
        {auto field=dynamic_cast<juce::TextEditor*>(named(editor,name));REQUIRE(field);field->setText(value,false);field->onReturnKey();editor.refresh();};
        for(const auto* name:{"arrivalSample","sourceSampleStart","sourceSampleEnd","preRoll","endOffset","fadeInMs","fadeOutMs","swellLevel","curveInTension","curveOutTension","wetPlacement"})
        {const auto cell=dynamic_cast<m3::ui::KnobValue*>(named(editor,juce::String(name)+"Cell"));REQUIRE(cell && visibleOffscreen(cell));REQUIRE(cell->getLocalBounds().contains(cell->valueField().getBounds()) && cell->getLocalBounds().contains(cell->valueKnob().getBounds()));}
        enter("preRoll","0.4");enter("endOffset","0.015");enter("fadeInMs","10");enter("fadeOutMs","20");enter("swellLevel","-3");
        enter("curveInTension","0.25");enter("curveOutTension","-0.4");enter("wetPlacement","25");
        auto edited=*m3::find(engine.savedSession(),first);REQUIRE(edited.placement==1200 && edited.shape.startOffset==-19200 && edited.shape.endOffset==720);
        REQUIRE(edited.shape.fadeIn==480 && edited.shape.fadeOut==960 && edited.shape.gainDb==-3 && edited.shape.inTension==.25 && edited.shape.outTension==-.4);
        enter("preRoll","0.45");enter("endOffset","0.05");edited=*m3::find(engine.savedSession(),first);
        REQUIRE(edited.shape.startOffset==-22800 && edited.shape.endOffset==1200 && edited.placement==1200);
        const auto retained=edited;enter("preRoll","99999999999999999999999999");enter("curveInTension","2");
        REQUIRE(m3::find(engine.savedSession(),first)->shape==retained.shape);
        auto curve=dynamic_cast<m3::ui::KnobValue*>(named(editor,"curveInTensionCell"));REQUIRE(curve);
        auto& knob=curve->valueKnob();knob.onDragStart();const auto start=knob.getValue();knob.setValue(start+.1,juce::sendNotificationSync);
        REQUIRE(std::abs(m3::find(engine.savedSession(),first)->shape.inTension-.45)<1e-6);
        REQUIRE(engine.selectTargets({second},second));editor.refresh();const auto otherShape=m3::find(engine.savedSession(),second)->shape;
        knob.setValue(.9,juce::sendNotificationSync);knob.onDragEnd();REQUIRE(m3::find(engine.savedSession(),second)->shape==otherShape);
        engine.restore(baseline);waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
        const auto shape=m3::find(engine.savedSession(),first)->shape;
        auto field=dynamic_cast<juce::TextEditor*>(named(editor,"fadeInMs"));REQUIRE(field);field->setText("77",false);field->onEscapeKey();
        REQUIRE(m3::find(engine.savedSession(),first)->shape==shape && field->getText().getDoubleValue()==0);
        auto irChoice=dynamic_cast<juce::ComboBox*>(named(editor,"selectedIR"));REQUIRE(irChoice && irChoice->getNumItems()==2 && irChoice->getItemText(0)=="Preverb Hall" && irChoice->getText()=="musical-ir.wav");
        snapshotEditor(editor,"inspector-default");
        auto scroll=dynamic_cast<juce::Viewport*>(named(editor,"inspectorViewport"));REQUIRE(scroll);scroll->setViewPosition(0,10000);snapshotEditor(editor,"inspector-scrolled");scroll->setViewPosition(0,0);
    }
    REQUIRE(engine.audition());buffer.clear();processor->processBlock(buffer,midi);editor.refresh();
    const auto auditionPosition=engine.snapshot().auditionSample;REQUIRE(auditionPosition && *auditionPosition==engine.snapshot().raw->start+buffer.getNumSamples());
    {const auto lane=editor.timelineComponent().sourceLane();const auto view=editor.timelineComponent().viewport();
        const auto play=static_cast<int>(view.pixel(static_cast<double>(*auditionPosition),lane.getX(),lane.getWidth()));
        const auto image=editor.timelineComponent().createComponentSnapshot(editor.timelineComponent().getLocalBounds());
        const auto color=image.getPixelAt(play,static_cast<int>(lane.getY()+2));REQUIRE(color.getRed()>230 && color.getBlue()>220 && color.getGreen()<210);}
    {const auto overview=named(editor,"songOverview");REQUIRE(overview);const auto image=overview->createComponentSnapshot(overview->getLocalBounds());int pink=0;
        for(int y=0;y<image.getHeight();++y)for(int x=0;x<image.getWidth();++x)if(image.getPixelAt(x,y).getARGB()==0xffffb4e9)++pink;REQUIRE(pink>20);}
    snapshotEditor(editor,"audition-playhead");buffer.clear();processor->processBlock(buffer,midi);REQUIRE(engine.snapshot().auditionSample==*auditionPosition+buffer.getNumSamples());
    engine.stop();buffer.clear();processor->processBlock(buffer,midi);editor.refresh();REQUIRE(!engine.snapshot().auditionSample);
    head.position.setTimeInSamples(0);head.position.setPpqPosition(0);head.position.setIsPlaying(true);
    waitUI([&]{buffer.clear();processor->processBlock(buffer,midi);const auto s=engine.snapshot();return !s.liveMissing && !s.liveIncompatible;});
    editor.refresh();snapshotEditor(editor,"live-playing");
    REQUIRE(!dynamic_cast<juce::TextButton*>(named(editor,"audition"))->isEnabled());
    {auto& timeline=editor.timelineComponent();const auto lane=timeline.sourceLane();const auto view=timeline.viewport();
        const auto x=static_cast<int>(view.pixel(0,lane.getX(),lane.getWidth()));const auto img=timeline.createComponentSnapshot(timeline.getLocalBounds());
        REQUIRE(x>=0 && x<img.getWidth());const auto colour=img.getPixelAt(x,static_cast<int>(lane.getY()+3));REQUIRE(colour.getRed()>200 && colour.getGreen()>150 && colour.getBlue()<200);}
    head.position.setTimeInSamples(12000);head.position.setPpqPosition(.5);buffer.clear();processor->processBlock(buffer,midi);editor.refresh();snapshotEditor(editor,"live-seek");
    {auto& timeline=editor.timelineComponent();const auto lane=timeline.sourceLane();const auto x=static_cast<int>(timeline.viewport().pixel(12000,lane.getX(),lane.getWidth()));
        const auto img=timeline.createComponentSnapshot(timeline.getLocalBounds());const auto colour=img.getPixelAt(x,static_cast<int>(lane.getY()+3));REQUIRE(colour.getRed()>200 && colour.getGreen()>150 && colour.getBlue()<200);}
    head.position.setTimeInSamples(0);head.position.setPpqPosition(0);buffer.clear();processor->processBlock(buffer,midi);editor.refresh();
    head.position.setIsPlaying(false);buffer.clear();processor->processBlock(buffer,midi);editor.refresh();
    REQUIRE(visibleOffscreen(named(editor,"arrivalSample")));
    auto musical=dynamic_cast<juce::TextEditor*>(named(editor,"arrivalSample"));REQUIRE(musical && musical->getText()=="1.1");
    auto diagnostics=dynamic_cast<juce::TextButton*>(named(editor,"diagnostics"));REQUIRE(diagnostics);
    REQUIRE(!visibleOffscreen(named(editor,"generate")));
    diagnostics->onClick();editor.refresh();REQUIRE(visibleOffscreen(named(editor,"generate")));snapshotEditor(editor,"diagnostics");
    editor.setSize(1100,850);editor.refresh();snapshotEditor(editor,"diagnostics-1100");
    auto inspectorScroll=dynamic_cast<juce::Viewport*>(named(editor,"inspectorViewport"));
    auto diagnosticScroll=dynamic_cast<juce::Viewport*>(named(editor,"diagnosticsViewport"));REQUIRE(inspectorScroll && diagnosticScroll);
    inspectorScroll->setViewPosition(0,10000);diagnosticScroll->setViewPosition(0,10000);
    REQUIRE(inspectorScroll->getViewPositionY()>0 && diagnosticScroll->getViewPositionY()>0);snapshotEditor(editor,"diagnostics-scrolled");
    inspectorScroll->setViewPosition(0,0);diagnosticScroll->setViewPosition(0,0);
    diagnostics->onClick();editor.setSize(1400,900);editor.refresh();
    auto regeneration=dynamic_cast<juce::Slider*>(named(editor,"regenerationSensitivity"));auto freeze=dynamic_cast<juce::ToggleButton*>(named(editor,"freezeRender"));
    REQUIRE(regeneration && freeze && visibleOffscreen(regeneration) && visibleOffscreen(freeze));
    const auto markerBefore=engine.savedSession().sensitivity;regeneration->setValue(12.5,juce::sendNotificationSync);
    REQUIRE(engine.savedSession().regenerationSensitivity==.125f && engine.savedSession().sensitivity==markerBefore);
    freeze->setToggleState(true,juce::dontSendNotification);freeze->onClick();REQUIRE(m3::find(engine.savedSession(),first)->frozen);
    freeze->setToggleState(false,juce::dontSendNotification);freeze->onClick();waitUI([&]{return m3::find(engine.savedSession(),first)->phase==m3::Phase::ready;});editor.refresh();
    auto marker=dynamic_cast<juce::Slider*>(named(editor,"markerSensitivity"));auto markerToggle=dynamic_cast<juce::ToggleButton*>(named(editor,"autoMarkers"));
    REQUIRE(marker && markerToggle && visibleOffscreen(marker) && visibleOffscreen(markerToggle));marker->setValue(12,juce::sendNotificationSync);
    markerToggle->setToggleState(false,juce::dontSendNotification);markerToggle->onClick();
    REQUIRE(std::abs(engine.savedSession().sensitivity-.12f)<1e-5 && !engine.savedSession().autoMarkersEnabled);
    auto liveToggle=dynamic_cast<juce::ToggleButton*>(named(editor,"livePlayback"));auto liveMode=dynamic_cast<juce::ComboBox*>(named(editor,"liveRouting"));auto liveLevel=dynamic_cast<juce::Slider*>(named(editor,"liveWetLevel"));
    REQUIRE(liveToggle && liveMode && liveLevel);
    REQUIRE(!visibleOffscreen(liveLevel));diagnostics->onClick();editor.refresh();REQUIRE(visibleOffscreen(liveLevel));diagnostics->onClick();editor.refresh();
    liveToggle->setToggleState(false,juce::dontSendNotification);liveToggle->onClick();liveMode->setSelectedId(2,juce::sendNotificationSync);liveLevel->setValue(-7.5,juce::sendNotificationSync);
    REQUIRE(!engine.savedSession().liveEnabled && engine.savedSession().liveRouting==m3::LiveRouting::wetOnly && engine.savedSession().liveWetDb==-7.5f);
    REQUIRE(engine.savedSession().auditionDb==0);engine.setLivePlayback(true,m3::LiveRouting::dryWet,0);editor.refresh();
    const auto monitorRenderCount=engine.snapshot().renderCount;
    auto dryWet=dynamic_cast<juce::TextButton*>(named(editor,"dryWet"));auto wetOnly=dynamic_cast<juce::TextButton*>(named(editor,"wetOnly"));
    auto blend=dynamic_cast<juce::Slider*>(named(editor,"auditionBlend"));REQUIRE(dryWet && wetOnly && blend);
    auto isolate=dynamic_cast<juce::ToggleButton*>(named(editor,"isolateSelected"));REQUIRE(isolate);
    isolate->setToggleState(false,juce::dontSendNotification);isolate->onClick();waitUI([&]{return engine.snapshot().auditionReady;});
    REQUIRE(!engine.savedSession().isolateSelected);
    isolate->setToggleState(true,juce::dontSendNotification);isolate->onClick();waitUI([&]{return engine.snapshot().auditionReady;});
    dryWet->onClick();waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    blend->setValue(35,juce::dontSendNotification);blend->onValueChange();waitUI([&]{return engine.snapshot().auditionReady;});
    wetOnly->onClick();waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    REQUIRE(engine.savedSession().wetBlend==.35f && engine.snapshot().renderCount==monitorRenderCount);
    engine.setPreferences(-24,.5f,.08,false);editor.refresh();
    dryWet->onClick();waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    auto dryIsolation=dynamic_cast<juce::ToggleButton*>(named(editor,"isolateDry"));REQUIRE(dryIsolation && dryIsolation->isEnabled());
    diagnostics->onClick();editor.refresh();REQUIRE(visibleOffscreen(dryIsolation));diagnostics->onClick();editor.refresh();
    dryIsolation->setToggleState(true,juce::dontSendNotification);dryIsolation->onClick();waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    REQUIRE(engine.savedSession().isolateDry && engine.snapshot().auditionDryEnd==engine.savedSession().swells[0].sourceEnd);
    snapshotEditor(editor,"isolated-dry");dryIsolation->setToggleState(false,juce::dontSendNotification);dryIsolation->onClick();
    waitUI([&]{return engine.snapshot().auditionReady;});
    wetOnly->onClick();waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    auto resetTrim=dynamic_cast<juce::TextButton*>(named(editor,"resetAuditionTrim"));REQUIRE(resetTrim && resetTrim->getButtonText().contains("-24"));
    resetTrim->onClick();REQUIRE(engine.savedSession().auditionDb==0);
    auto sampleUnits=dynamic_cast<juce::ToggleButton*>(named(editor,"sampleUnits"));REQUIRE(sampleUnits);
    sampleUnits->setToggleState(true,juce::dontSendNotification);sampleUnits->onClick();
    for(auto dimensions:{juce::Point<int>{1100,850},juce::Point<int>{1400,900},juce::Point<int>{1600,1000}})
    {editor.setSize(dimensions.x,dimensions.y);editor.refresh();for(auto child:editor.getChildren())REQUIRE(editor.getLocalBounds().contains(child->getBounds()));snapshotEditor(editor,"ready-"+juce::String(dimensions.x));}
    editor.setSize(1200,900);editor.refresh();
    auto& timeline=editor.timelineComponent();const auto oldCount=engine.snapshot().renderCount;const auto oldRender=engine.savedSession().swells[0].render.path;
    const auto oldViewport=timeline.viewport();timeline.page(8);timeline.next();timeline.previous();timeline.zoom(0.5);REQUIRE(engine.snapshot().renderCount==oldCount && engine.savedSession().swells[0].phase==m3::Phase::ready);
    timeline.navigateOverview(.8);const auto shifted=timeline.viewport();timeline.navigateOverview(.2);REQUIRE(shifted.first!=timeline.viewport().first && engine.snapshot().renderCount==oldCount);
    timeline.setViewport(oldViewport);timeline.setSnap(false);const auto raw=engine.snapshot().raw;const auto lane=timeline.wetLane();const auto view=timeline.viewport();
    const auto begin=static_cast<float>(view.pixel(static_cast<double>(raw->start),lane.getX(),lane.getWidth()));
    auto grabs=timeline.grabPoints();const auto startGrab=*std::find_if(grabs.begin(),grabs.end(),[](auto p){return p.kind==m3::EnvelopePoint::start;});
    REQUIRE(timeline.beginDrag(startGrab.point));const auto next=raw->start+500;
    timeline.dragTo({static_cast<float>(view.pixel(static_cast<double>(next),lane.getX(),lane.getWidth())),startGrab.point.y});
    waitUI([&]{return engine.snapshot().auditionReady;});REQUIRE(std::abs(engine.savedSession().swells[0].shape.startOffset-next)<=1);
    auto fade=dynamic_cast<juce::TextEditor*>(named(editor,"fadeInMs"));REQUIRE(fade);fade->setText("10.000",false);fade->onReturnKey();
    waitUI([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.savedSession().swells[0].shape.fadeIn==480);
    auto gain=dynamic_cast<juce::Slider*>(named(editor,"swellGain"));REQUIRE(gain);gain->setValue(3,juce::dontSendNotification);gain->onValueChange();
    waitUI([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.savedSession().swells[0].shape.gainDb==3);
    REQUIRE(engine.snapshot().renderCount==oldCount && engine.savedSession().swells[0].render.path==oldRender);
    editor.refresh();snapshotEditor(editor,"shaped");
    // Shared painted/hit geometry, on-curve tension, fine editing independent of Song Snap.
    auto coLocated=engine.savedSession().swells[0];coLocated.shape.automaticStart=false;coLocated.shape.startOffset=0;coLocated.shape.endOffset=1;coLocated.shape.fadeIn=coLocated.shape.fadeOut=0;
    {auto endpoints=coLocated;endpoints.shape.endOffset=100;const juce::Rectangle<float> testLane{0,0,520,220};
        m3::SwellViewport close;close.first=-20;close.last=120;
        for(int64_t fadeIn:{0,20,99})for(int64_t fadeOut:{0,20,99}){endpoints.shape.fadeIn=fadeIn;endpoints.shape.fadeOut=fadeOut;
            const auto points=m3::envelopeHandles(endpoints,raw->start,close,testLane);
            for(const auto& p:points)if(p.kind==m3::EnvelopePoint::start || p.kind==m3::EnvelopePoint::end){
                REQUIRE(p.point.y==testLane.getCentreY() && p.anchor.y==testLane.getCentreY());REQUIRE(p.point==p.anchor);}}}
    {auto spread=coLocated;spread.shape.endOffset=100;spread.shape.fadeIn=40;spread.shape.fadeOut=20;
        for(double span:{200.0,10000.0,100000000.0}){m3::SwellViewport compressed;compressed.first=-20;compressed.last=span;
            const auto points=m3::envelopeHandles(spread,raw->start,compressed,{0,0,520,220});
            for(const auto& p:points)REQUIRE(p.point==p.anchor);
            if(span>1000000)REQUIRE(m3::hitEnvelope(points,points[4].point)==m3::EnvelopePoint::outTension);}}
    {auto feedback=coLocated;feedback.shape.endOffset=100;feedback.shape.fadeIn=feedback.shape.fadeOut=99;
        m3::SwellViewport feedbackView;feedbackView.first=-20;feedbackView.last=500;
        const juce::Rectangle<float> feedbackLane{0,0,520,220};
        const auto points=m3::envelopeHandles(feedback,raw->start,feedbackView,feedbackLane);
        for(const auto& p:points){REQUIRE(std::abs(p.point.x-p.anchor.x)<.01f);
            auto close=feedbackView;close.first=static_cast<double>(p.sample)-4;close.last=static_cast<double>(p.sample)+4;
            const auto zoomed=m3::envelopeHandles(feedback,raw->start,close,feedbackLane);
            const auto target=*std::find_if(zoomed.begin(),zoomed.end(),[&](const auto& h){return h.kind==p.kind;});
            REQUIRE(m3::hitEnvelope(zoomed,target.point)==p.kind);}}
    auto coincidenceView=view;coincidenceView.first=-1;coincidenceView.last=2;
    auto coincident=m3::envelopeHandles(coLocated,raw->start,coincidenceView,lane);REQUIRE(coincident.size()==4);
    REQUIRE(m3::hitEnvelope(coincident,coincident[0].point)==m3::EnvelopePoint::end);
    for(auto grab:coincident)REQUIRE(m3::hitEnvelope(coincident,grab.point)==grab.kind);
    for(double span:{2.0,10000.0,100000000.0}){auto wide=coincidenceView;wide.last=span;
        const auto points=m3::envelopeHandles(coLocated,raw->start,wide,lane);
        const auto fade=*std::find_if(points.begin(),points.end(),[](const auto& p){return p.kind==m3::EnvelopePoint::fadeIn;});
        const auto out=*std::find_if(points.begin(),points.end(),[](const auto& p){return p.kind==m3::EnvelopePoint::fadeOut;});
        REQUIRE(fade.sample==points[1].sample && out.sample==points[0].sample);
        REQUIRE(fade.point==fade.anchor && out.point==out.anchor);
        REQUIRE(fade.anchor.y<points[1].anchor.y && out.anchor.y<points[0].anchor.y);
        auto genuine=coLocated;genuine.shape.endOffset=100;genuine.shape.fadeIn=1;genuine.shape.fadeOut=0;
        const auto same=m3::envelopeHandles(genuine,raw->start,wide,lane);
        const auto tension=*std::find_if(same.begin(),same.end(),[](const auto& p){return p.kind==m3::EnvelopePoint::inTension;});
        REQUIRE(tension.sample==same[1].sample && tension.anchor==same[1].anchor);
        REQUIRE(std::abs(tension.point.getDistanceFrom(tension.anchor)-24)<.001);}

    for(auto tensionValue:{-1.0,0.0,1.0})for(auto fadeLength:{int64_t{0},int64_t{99999}})for(auto placement:{int64_t{-1000000000},int64_t{0},int64_t{1000000000}}){
        auto extreme=coLocated;extreme.shape.startOffset=0;extreme.shape.endOffset=100000;extreme.shape.fadeIn=extreme.shape.fadeOut=fadeLength;
        extreme.shape.inTension=extreme.shape.outTension=tensionValue;extreme.placement=placement;
        auto extremeView=view;extremeView.first=0;extremeView.last=100000;
        const auto handles=m3::envelopeHandles(extreme,raw->start,extremeView,lane);
        for(const auto& handle:handles){REQUIRE(std::isfinite(handle.point.x) && std::isfinite(handle.point.y));
            if(lane.contains(handle.point)){
                auto close=extremeView;close.first=static_cast<double>(handle.sample)-4;close.last=static_cast<double>(handle.sample)+4;
                const auto zoomed=m3::envelopeHandles(extreme,raw->start,close,lane);const auto target=*std::find_if(zoomed.begin(),zoomed.end(),[&](const auto& h){return h.kind==handle.kind;});
                REQUIRE(m3::hitEnvelope(zoomed,target.point)==handle.kind);}
            if(handle.point!=handle.anchor)REQUIRE(std::any_of(handles.begin(),handles.end(),[&](const auto& other){return other.kind!=handle.kind && other.sample==handle.sample && other.anchor==handle.anchor;}));}}
    // Initial zero fade is directly grabbable without changing its bound.
    REQUIRE(engine.edit(first,[](auto& w,auto&){w.shape.fadeIn=w.shape.fadeOut=0;return true;}));waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    for(auto size:{juce::Point<int>{1100,850},juce::Point<int>{1400,900},juce::Point<int>{1600,1000}})
    {editor.setSize(size.x,size.y);editor.refresh();snapshotEditor(editor,"zero-fades-"+juce::String(size.x));diagnostics->onClick();editor.refresh();snapshotEditor(editor,"zero-fades-diagnostics-"+juce::String(size.x));diagnostics->onClick();}
    editor.setSize(1200,900);editor.refresh();
    auto zeroPoints=timeline.grabPoints();const auto zeroGrab=*std::find_if(zeroPoints.begin(),zeroPoints.end(),[](auto p){return p.kind==m3::EnvelopePoint::fadeIn;});
    const auto zeroBound=engine.savedSession().swells[0].shape.startOffset;
    REQUIRE(timeline.beginDrag(zeroGrab.point));timeline.dragTo(zeroGrab.point+juce::Point<float>{15,0});
    REQUIRE(engine.savedSession().swells[0].shape.fadeIn>0 && engine.savedSession().swells[0].shape.startOffset==zeroBound);
    waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    auto outPoints=timeline.grabPoints();const auto zeroOut=*std::find_if(outPoints.begin(),outPoints.end(),[](auto p){return p.kind==m3::EnvelopePoint::fadeOut;});
    const auto zeroEnd=engine.savedSession().swells[0].shape.endOffset;REQUIRE(timeline.beginDrag(zeroOut.point));timeline.dragTo(zeroOut.point+juce::Point<float>{-15,0});
    REQUIRE(engine.savedSession().swells[0].shape.fadeOut>0 && engine.savedSession().swells[0].shape.endOffset==zeroEnd);
    waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    const auto shapeBeforeHandles=engine.savedSession().swells[0].shape;
    // Regression: endpoint grab is displaced nine pixels from its real sample
    // at a viewport edge. Pressing and dragging without motion must not jump.
    for(bool snapState:{false,true}){timeline.setSnap(snapState);const auto oldView=timeline.viewport();int64_t a,b;const auto w=engine.savedSession().swells[0];
        REQUIRE(m3::requestedShapeBounds(w,engine.snapshot().raw->start,a,b));
        auto edgeView=oldView;edgeView.first=static_cast<double>(a);edgeView.last=static_cast<double>(b);timeline.setViewport(edgeView);
        const auto handles=timeline.grabPoints();const auto start=*std::find_if(handles.begin(),handles.end(),[](auto p){return p.kind==m3::EnvelopePoint::start;});
        REQUIRE(timeline.beginDrag(start.point));timeline.dragTo(start.point);
        const auto edgeEnd=*std::find_if(handles.begin(),handles.end(),[](auto p){return p.kind==m3::EnvelopePoint::end;});
        REQUIRE(timeline.beginDrag(edgeEnd.point));timeline.dragTo(edgeEnd.point);

        int64_t afterA,afterB;REQUIRE(m3::requestedShapeBounds(engine.savedSession().swells[0],engine.snapshot().raw->start,afterA,afterB));
        REQUIRE(afterA==a && afterB==b);
        const auto ends=timeline.grabPoints();const auto end=*std::find_if(ends.begin(),ends.end(),[](auto p){return p.kind==m3::EnvelopePoint::end;});
        const auto dryArrival=engine.savedSession().swells[0].arrival;
        REQUIRE(timeline.beginDrag(end.point));timeline.dragTo(end.point+juce::Point<float>{-100000,0});
        REQUIRE(m3::requestedShapeBounds(engine.savedSession().swells[0],engine.snapshot().raw->start,afterA,afterB) && afterB==afterA+1);
        timeline.dragTo(end.point);REQUIRE(m3::requestedShapeBounds(engine.savedSession().swells[0],engine.snapshot().raw->start,afterA,afterB));
        REQUIRE(afterA==a && afterB==b && engine.savedSession().swells[0].arrival==dryArrival);
        REQUIRE(engine.savedSession().swells[0].shape.fadeIn==shapeBeforeHandles.fadeIn && engine.savedSession().swells[0].shape.fadeOut==shapeBeforeHandles.fadeOut);
        REQUIRE(engine.edit(first,[&](auto& w,auto&){w.shape=shapeBeforeHandles;return true;}));
        timeline.setViewport(oldView);waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();}
    REQUIRE(engine.edit(first,[](auto& w,auto&){w.shape.fadeIn=4800;w.shape.fadeOut=6000;return true;}));
    waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();timeline.setSnap(true);
    grabs=timeline.grabPoints();REQUIRE(grabs.size()==6);
    for(auto grab:grabs)REQUIRE(m3::hitEnvelope(grabs,grab.point)==grab.kind);
    const auto tensionGrab=*std::find_if(grabs.begin(),grabs.end(),[](auto p){return p.kind==m3::EnvelopePoint::inTension;});
    REQUIRE(timeline.beginDrag(tensionGrab.point));timeline.dragTo(tensionGrab.point+juce::Point<float>{0,-24});
    REQUIRE(std::abs(engine.savedSession().swells[0].shape.inTension+.3)<.0001);
    {const auto points=timeline.grabPoints();const auto point=*std::find_if(points.begin(),points.end(),[](auto p){return p.kind==m3::EnvelopePoint::inTension;});
        REQUIRE(timeline.beginDrag(point.point));timeline.dragTo(point.point+juce::Point<float>{0,100000});REQUIRE(engine.savedSession().swells[0].shape.inTension==1);
        timeline.dragTo(point.point+juce::Point<float>{0,-100000});REQUIRE(engine.savedSession().swells[0].shape.inTension==-1);
        timeline.dragTo(point.point);REQUIRE(std::abs(engine.savedSession().swells[0].shape.inTension+.3)<.0001);}
    // No timer refresh: selected metadata and visible handle geometry are updated now.
    REQUIRE(timeline.grabPoints()[3].point!=tensionGrab.point);
    const auto retainedViewport=timeline.viewport();grabs=timeline.grabPoints();
    const auto endGrab=*std::find_if(grabs.begin(),grabs.end(),[](auto p){return p.kind==m3::EnvelopePoint::end;});
    REQUIRE(timeline.beginDrag(endGrab.point));timeline.dragTo({static_cast<float>(view.pixel(2003,lane.getX(),lane.getWidth())),endGrab.point.y});
    REQUIRE(std::abs(engine.savedSession().swells[0].shape.endOffset-2003)<=1);
    REQUIRE(engine.savedSession().swells[0].shape.fadeOut==6000 && engine.savedSession().swells[0].arrival==0);
    waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    REQUIRE(timeline.viewport().first==retainedViewport.first && engine.snapshot().renderCount==oldCount);
    const auto beforeKey=engine.savedSession().swells[0].shape.endOffset;
    REQUIRE(timeline.keyPressed(juce::KeyPress(juce::KeyPress::rightKey)));
    REQUIRE(engine.savedSession().swells[0].shape.endOffset==beforeKey+1);
    auto wetEndField=dynamic_cast<juce::TextEditor*>(named(editor,"audibleEnd"));REQUIRE(wetEndField && wetEndField->getText().getLargeIntValue()==beforeKey+1);
    waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    snapshotEditor(editor,"transition-handles");
    {const auto savedShape=engine.savedSession().swells[0].shape;const auto savedView=timeline.viewport();
        REQUIRE(engine.edit(first,[](auto& w,auto&){w.shape.automaticStart=false;w.shape.startOffset=-5000;w.shape.endOffset=0;
            w.shape.fadeIn=w.shape.fadeOut=4999;w.shape.inTension=.7;w.shape.outTension=-.7;return true;}));
        waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();auto compressed=timeline.viewport();compressed.first=-6000;compressed.last=18000;timeline.setViewport(compressed);
        snapshotEditor(editor,"full-fade-controls");compressed.first=-768000;compressed.last=1536000;timeline.setViewport(compressed);snapshotEditor(editor,"stacked-handles-far");
        compressed.first=-2506;compressed.last=-2496;timeline.setViewport(compressed);snapshotEditor(editor,"stacked-handles-close");
        REQUIRE(engine.edit(first,[&](auto& w,auto&){w.shape=savedShape;return true;}));
        waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();timeline.setViewport(savedView);}
    for(auto size:{juce::Point<int>{1100,850},juce::Point<int>{1400,900},juce::Point<int>{1600,1000}})
    {editor.setSize(size.x,size.y);editor.refresh();snapshotEditor(editor,"envelope-"+juce::String(size.x));
        diagnostics->onClick();editor.refresh();snapshotEditor(editor,"envelope-diagnostics-"+juce::String(size.x));diagnostics->onClick();}
    editor.setSize(1200,900);editor.refresh();
    auto tension=dynamic_cast<juce::Slider*>(named(editor,"outTension"));auto tail=dynamic_cast<juce::Slider*>(named(editor,"auditionTail"));REQUIRE(tension && tail);
    tension->setValue(-.37,juce::sendNotificationSync);tail->setValue(1.25,juce::sendNotificationSync);
    REQUIRE(engine.savedSession().swells[0].shape.outTension==-.37 && engine.savedSession().postRollSeconds==1.25);
    REQUIRE(engine.edit(first,[&](auto& w,auto&){w.shape=shapeBeforeHandles;return true;}));
    waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();timeline.setSnap(false);
    // Whole-body movement translates audio and envelope together without changing any crop/fade.
    const auto beforeMove=engine.savedSession().swells[0];auto moveLane=timeline.wetLane();int64_t moveA,moveB;
    REQUIRE(m3::requestedShapeBounds(beforeMove,raw->start,moveA,moveB));
    const juce::Point<float> body{static_cast<float>(timeline.viewport().pixel((moveA+moveB)*.5,moveLane.getX(),moveLane.getWidth())),moveLane.getBottom()-30};
    REQUIRE(timeline.beginDrag(body));timeline.dragTo(body+juce::Point<float>{20,0});
    REQUIRE(engine.savedSession().swells[0].placement>0 && engine.savedSession().swells[0].shape==beforeMove.shape && engine.savedSession().swells[0].arrival==beforeMove.arrival);
    waitUI([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.snapshot().renderCount==oldCount);
    editor.refresh();snapshotEditor(editor,"moved-body");auto moveField=dynamic_cast<juce::TextEditor*>(named(editor,"wetPlacement"));REQUIRE(moveField);moveField->setText("0",false);moveField->onReturnKey();
    waitUI([&]{return engine.snapshot().auditionReady;});REQUIRE(engine.savedSession().swells[0].placement==0);
    // Source-handle edits invalidate only the selected swell.
    const auto source=timeline.sourceLane();const auto xEnd=static_cast<float>(timeline.viewport().pixel(24000,source.getX(),source.getWidth()));
    REQUIRE(timeline.beginDrag({xEnd,source.getBottom()-5}));timeline.dragTo({static_cast<float>(timeline.viewport().pixel(20000,source.getX(),source.getWidth())),source.getBottom()-5});
    REQUIRE(engine.savedSession().swells[0].sourceEnd==20000 && engine.savedSession().swells[0].phase==m3::Phase::stale && engine.savedSession().swells[1].phase==m3::Phase::ready);
    editor.refresh();snapshotEditor(editor,"stale");
    auto field=dynamic_cast<juce::TextEditor*>(named(editor,"sourceSampleEnd"));REQUIRE(field);field->setText("24000",false);field->onReturnKey();
    auto generate=dynamic_cast<juce::TextButton*>(named(editor,"generate"));REQUIRE(generate);generate->onClick();waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    REQUIRE(engine.snapshot().sourceDetail && engine.snapshot().sourceDetail->peaks.size()>1000);
    timeline.setSnap(false);const auto sourceAgain=timeline.sourceLane();const auto targetPixel=static_cast<float>(timeline.viewport().pixel(0,sourceAgain.getX(),sourceAgain.getWidth()));
    REQUIRE(timeline.beginDrag({targetPixel,sourceAgain.getY()+15}));timeline.dragTo({static_cast<float>(timeline.viewport().pixel(13,sourceAgain.getX(),sourceAgain.getWidth())),sourceAgain.getY()+15});
    REQUIRE(engine.savedSession().swells[0].arrival==13 && engine.savedSession().swells[0].sourceStart==13 && engine.savedSession().swells[1].arrival==48000);
    editor.refresh();auto arrival=dynamic_cast<juce::TextEditor*>(named(editor,"arrivalSample"));REQUIRE(arrival);arrival->setText("0",false);arrival->onReturnKey();generate->onClick();waitUI([&]{return engine.snapshot().auditionReady;});
    auto suggest=button(editor,"Suggest onsets");REQUIRE(suggest);suggest->onClick();waitUI([&]{return !engine.snapshot().analyzing;});
    REQUIRE(!engine.snapshot().candidates.empty());timeline.zoom(4);editor.refresh();const auto beforeConfirmation=engine.savedSession().swells.size();
    auto confirm=button(editor,"Confirm");REQUIRE(confirm && confirm->isEnabled());confirm->onClick();REQUIRE(engine.savedSession().swells.size()==beforeConfirmation+1);
    auto remove=button(editor,"Delete");REQUIRE(remove);remove->onClick();REQUIRE(engine.savedSession().swells.size()==beforeConfirmation);
    auto add=button(editor,"+");REQUIRE(add);add->onClick();REQUIRE(engine.savedSession().swells.size()==beforeConfirmation+1);remove->onClick();
    REQUIRE(engine.selectTargets({first},first));editor.refresh();
    const auto saved=engine.savedSession();auto many=saved;many.swells.clear();
    engine.setAutomaticProcessing(true);
    const auto supported=engine.snapshot().raw;REQUIRE(supported);
    REQUIRE(engine.edit(first,[&](auto& w,auto& error){auto shape=w.shape;shape.endOffset=200000-w.arrival;return m3::setShape(w,shape,supported->start,supported->end,error);}));
    waitUI([&]{return find(engine.savedSession(),first)->status=="Waiting for source";});
    editor.refresh();timeline.fitSelection();REQUIRE(timeline.viewport().last>=200000);
    REQUIRE(find(engine.savedSession(),first)->sourceEnd==24000);
    const auto ghost=timeline.grabPoints();const auto ghostEnd=*std::find_if(ghost.begin(),ghost.end(),[](auto p){return p.kind==m3::EnvelopePoint::end;});
    REQUIRE(ghostEnd.point.x>=timeline.wetLane().getX() && ghostEnd.point.x<=timeline.wetLane().getRight());
    snapshotEditor(editor,"pending-support");snapshotEditor(editor,"waiting");
    REQUIRE(!dynamic_cast<juce::TextButton*>(named(editor,"audition"))->isEnabled());
    engine.restore(saved);waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    const auto shapeBefore=saved.swells[0].shape;
    REQUIRE(engine.edit(first,[&](auto& w,auto&){w.shape.automaticStart=false;w.shape.startOffset=raw->start-100;return true;}));
    waitUI([&]{return engine.savedSession().swells[0].phase==m3::Phase::error;});editor.refresh();
    auto invalidStart=dynamic_cast<juce::TextEditor*>(named(editor,"audibleStart"));REQUIRE(invalidStart && invalidStart->getText().getLargeIntValue()==raw->start-100);
    REQUIRE(engine.edit(first,[&](auto& w,auto&){w.shape=shapeBefore;return true;}));waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    for(int n=0;n<500;++n){auto w=saved.swells[0];w.id=juce::Uuid().toDashedString();w.name="Target "+juce::String(n+1);many.swells.push_back(w);}
    many.primary=many.swells[0].id;many.selected={many.primary};engine.restore(many);editor.refresh();
    const auto list=dynamic_cast<juce::ListBox*>(named(editor,"swellList"));REQUIRE(list && list->getModel()->getNumRows()==500 && list->getNumChildComponents()<30);snapshotEditor(editor,"many");
    engine.restore(saved);waitUI([&]{return engine.snapshot().auditionReady;});editor.refresh();
    auto advanced=dynamic_cast<juce::TextButton*>(named(editor,"advancedTiming"));REQUIRE(advanced);advanced->onClick();
    REQUIRE(visibleOffscreen(named(editor,"sourceSampleStart")));snapshotEditor(editor,"precise-timing");
    const auto bad=root.file.getChildFile("invalid.wav");REQUIRE(bad.replaceWithText("invalid fixture"));engine.loadIR(bad);waitUI([&]{return engine.savedSession().swells[0].phase==m3::Phase::error;});editor.refresh();snapshotEditor(editor,"error");
    REQUIRE(engine.assetDirectory().getChildFile(engine.savedSession().swells[0].ir.path).deleteFile());
    engine.setLivePlayback(true,m3::LiveRouting::dryWet,0);processor->setNonRealtime(true);buffer.clear();processor->processBlock(buffer,midi);editor.refresh();
    auto exportStatus=dynamic_cast<juce::Label*>(named(editor,"processingStatus"));REQUIRE(exportStatus && exportStatus->getText().startsWith("Export warning"));
    REQUIRE(engine.snapshot().offlineBlocks==1 && engine.snapshot().offlineStatus.startsWith("Export warning"));snapshotEditor(editor,"export-error");
    processor->setNonRealtime(false);processor->prepareToPlay(48000,4096);
    // The knob's floating proxy must never round an absolute sample through
    // double. Both typed and relative edits retain the odd sample above 2^53.
    {
        auto exact=engine.savedSession();exact.swells.resize(1);auto& large=exact.swells[0];
        large.id=juce::Uuid().toDashedString();large.arrival=large.sourceStart=9007199254740993LL;large.sourceEnd=large.arrival+24000;
        large.frozen=false;large.ir={};large.render={};large.shape=m3::initialShape(48000);large.placement=0;
        exact.selected={large.id};exact.primary=large.id;exact.automaticProcessing=false;
        engine.restore(exact);editor.refresh();
        auto units=dynamic_cast<juce::ToggleButton*>(named(editor,"sampleUnits"));REQUIRE(units);units->setToggleState(true,juce::dontSendNotification);units->onClick();editor.refresh();
        auto field=dynamic_cast<juce::TextEditor*>(named(editor,"arrivalSample"));REQUIRE(field && field->getText()=="9007199254740993");
        field->setText("9007199254740995",false);field->onReturnKey();REQUIRE(engine.savedSession().swells[0].arrival==9007199254740995LL);
        editor.refresh();auto cell=dynamic_cast<m3::ui::KnobValue*>(named(editor,"sourceSampleStartCell"));REQUIRE(cell);
        const auto before=engine.savedSession().swells[0].sourceStart;const auto span=editor.timelineComponent().viewport().span();
        auto& knob=cell->valueKnob();knob.onDragStart();const auto initial=knob.getValue();knob.setValue(initial-.001,juce::sendNotificationSync);knob.onDragEnd();
        REQUIRE(engine.savedSession().swells[0].sourceStart==before+static_cast<int64_t>(std::llround(-.001*span)));
        field->setText("9223372036854775808",false);field->onReturnKey();REQUIRE(engine.savedSession().swells[0].arrival==9007199254740995LL);
    }
    engine.restore({});editor.refresh();snapshotEditor(editor,"empty");
});}
