#pragma once

namespace loopExportReview
{
using namespace m3;
using namespace m3::ui;

class Panel final:public juce::Component,private juce::Timer
{
public:
    Panel():engine(root.file,[]{return PreviewContext{};}),workspace(engine)
    {
        for(auto* child:std::initializer_list<juce::Component*>{&header,&notice,&workspace,&help})addAndMakeVisible(child);
        workflow::Snapshot flow;flow.mode=workflow::Mode::finalise;
        header.display(presentWorkflow(flow,{true,true,true,false}));
        header.modeInformation().display("Accepted Loop | 4 bars");
        notice.display(controls::NoticeMessage{"DAW wet is bypassed in Export","Use Preview to hear the exported wet audio.",controls::NoticeTone::information});
        applyTextRole(help,TextRole::helper);help.setBorderSize({});
        help.setText("Layout preview: try Mix Character, Dry/Wet, Lead-in grid and Repeats. Audio and file delivery are disabled.",juce::dontSendNotification);
        settings={2,16};
        workspace.onLoopSettings=[this](auto next,uint64_t session,uint64_t pass){
            if(session!=display.songExport.session || pass!=display.songExport.pass)return;
            settings=next;refresh();
        };
        workspace.onLoopLevels=[this](auto next,uint64_t session,uint64_t pass){
            if(session!=display.songExport.session || pass!=display.songExport.pass)return;
            levels=next;refresh();
        };
        refresh();setSize(900,600);startTimerHz(20);
    }
    void refresh()
    {
        // Presentation data only. Production entry/delivery remain unchanged
        // until the owner approves and the real finite plan is integrated.
        constexpr double rate=48000,bar=96000,period=4*bar,first=-2.4*bar;
        auto plan=std::make_shared<SongExportPlan>();
        plan->identity="loop-layout-"+juce::String(settings.repeats)+"-"+juce::String(settings.gridBeats)+"-"+juce::String(levels.dry)+"-"+juce::String(levels.wet);
        plan->timing={true,0,0,rate,120,4,4};plan->dry=levels.dry;plan->wet=levels.wet;
        const auto gridFrames=settings.gridBeats*bar/4;
        plan->origin=levels.wet>0?static_cast<int64_t>(std::floor(first/gridFrames)*gridFrames):0;
        plan->end=static_cast<int64_t>(settings.repeats*period+(levels.wet>0?bar/4:0));
        auto prepared=std::make_shared<PreparedExport>();prepared->plan=plan;
        if(levels.dry>0 || levels.wet>0){prepared->metrics.audibleStart=levels.wet>0?static_cast<int64_t>(first):0;prepared->metrics.audibleEnd=plan->end;}
        prepared->metrics.peak=.7f*levels.wet+.55f*levels.dry;prepared->metrics.displayGain=1;
        constexpr int bins=1200;
        const auto span=plan->end-plan->origin;
        for(int bin=0;bin<bins;++bin){
            const auto at=plan->origin+span*bin/bins;
            const auto count=std::max<int64_t>(1,span*(bin+1)/bins+plan->origin-at);
            const double centre=at+count*.5;
            double amplitude=0,dryAmplitude=0;
            for(int pass=0;pass<settings.repeats;++pass){
                const auto local=centre-pass*period;
                for(double event:{0.,2*bar}){
                    const auto delta=local-event;
                    if(delta>=first && delta<bar/4)
                        amplitude+=delta<0?.7*std::exp(delta/(bar*.25)):.7*std::exp(-delta/(bar*.04));
                    if(local>=0 && local<period && delta>=0 && delta<bar/8)
                        dryAmplitude+=.55*std::exp(-delta/(bar*.025));
                }
            }
            const auto peak=static_cast<float>(amplitude)*levels.wet;
            prepared->metrics.wetPeaks.push_back({at,static_cast<uint32_t>(count),-peak,peak});
            const auto dryPeak=static_cast<float>(dryAmplitude)*levels.dry;
            prepared->metrics.dryPeaks.push_back({at,static_cast<uint32_t>(count),-dryPeak,dryPeak});
        }
        display.songExport.session=7;display.songExport.pass=9;
        display.songExport.phase=levels.dry>0 || levels.wet>0?ExportPhase::ready:ExportPhase::idle;display.songExport.prepared=prepared;
        display.songExport.message=levels.dry>0 || levels.wet>0?juce::String{}:"Silent: raise Dry or Wet to prepare a WAV.";
        display.session.exportState.dry=levels.dry;display.session.exportState.wet=levels.wet;
        display.session.exportState.loop.initialized=true;display.session.exportState.loop.levels={levels.dry,levels.wet};
        workspace.setLoopPresentation(ExportWorkspace::LoopPresentation{settings,period,bar});
        workspace.display(display);
        for(const auto* name:{"exportPlay","exportStop","exportSave","exportDrag","exportReturn"})
            if(auto* action=find(workspace,name)){action->setEnabled(false);action->setHelpText("Layout preview only. Audio and file delivery will be enabled after integration.");}
    }
    void resized()override
    {
        const ResponsiveWorkspaceLayout layout(getLocalBounds());
        header.setResponsivePresentation(ResponsiveTypography::at(getWidth(),getHeight()));header.setBounds(layout.header);
        notice.setBounds(layout.notice);workspace.setResponsivePresentation();workspace.setBounds(layout.body);
        help.setBounds(layout.help);
    }
    void paint(juce::Graphics& g)override
    {
        g.fillAll(background);g.setColour(separator);
        const ResponsiveWorkspaceLayout layout(getLocalBounds());
        g.drawHorizontalLine(layout.body.getY()-responsiveGeometry::contentPadding/2,static_cast<float>(layout.body.getX()),static_cast<float>(layout.body.getRight()));
    }
    static juce::Component* find(juce::Component& root,const juce::String& name)
    {if(root.getName()==name)return &root;for(auto* child:root.getChildren())if(auto* result=find(*child,name))return result;return nullptr;}
    TestDirectory root;SwellEngine engine;ExportWorkspace workspace;EditorSnapshot display;
    LoopExportControls::Settings settings;
    controls::ExportLevelsControl::Pair levels;
private:
    void timerCallback()override
    {
        const auto point=getMouseXYRelative();auto* target=getComponentAt(point);
        for(auto* child=target;child && child!=this;child=child->getParentComponent()){
            if(auto* tip=dynamic_cast<juce::TooltipClient*>(child)){const auto text=tip->getTooltip();if(text.isNotEmpty()){help.setText(text,juce::dontSendNotification);return;}}
        }
    }
    WorkflowHeader header;controls::NoticeBar notice;juce::Label help;
};

inline void cases()
{
    WorkspaceTheme theme;Panel panel;panel.setLookAndFeel(&theme);
    auto& options=panel.workspace.loopOptions();unsigned changes=0;
    options.onChange=[&](auto){++changes;};
    options.display({2,16});REQUIRE(changes==0);
    options.repeats.setText("0",juce::sendNotificationSync);REQUIRE(options.settings().repeats==2 && changes==0);
    options.repeats.setText("3",juce::sendNotificationSync);REQUIRE(options.settings().repeats==3 && changes==1);
    options.setMeter(4,true,false);REQUIRE(!options.grid.isItemEnabled(4) && options.grid.isItemEnabled(1));
    options.setMeter(4,false,false);REQUIRE(!options.grid.isEnabled() && options.grid.getTooltip().contains("no verified beat"));
    options.setMeter(4,true,true);REQUIRE(options.grid.isEnabled() && options.grid.isItemEnabled(16));
    panel.refresh();
    nativeInputPeer(panel);
    auto& increment=options.repeats.incrementButton();increment.grabKeyboardFocus();
    REQUIRE(increment.hasKeyboardFocus(false));
    REQUIRE(static_cast<juce::Component&>(increment).keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));
    juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
    REQUIRE(options.settings().repeats==3);
    panel.removeFromDesktop();
    // Workspace binding must reject an invocation captured before a restore.
    // Test the real callback, rather than a second model of the guard.
    ExportWorkspace binding(panel.engine);binding.setResponsivePresentation();binding.setSize(900,500);
    binding.setLoopPresentation(ExportWorkspace::LoopPresentation{{2,16},384000,96000});binding.display(panel.display);
    unsigned committed=0;binding.onLoopSettings=[&](auto,uint64_t,uint64_t){++committed;};
    binding.loopOptions().beforeInvocation();auto restored=panel.display;++restored.songExport.session;binding.display(restored);
    binding.loopOptions().repeats.onStep(1);REQUIRE(committed==0 && binding.loopOptions().settings().repeats==2);
    binding.loopOptions().beforeInvocation();binding.loopOptions().repeats.onStep(1);REQUIRE(committed==1);
    binding.setPreviewDeviceRate(44100);binding.display(restored);
    auto* play=Panel::find(binding,"exportPlay");auto* save=Panel::find(binding,"exportSave");
    REQUIRE(play && save && !play->isEnabled() && save->isEnabled());
    REQUIRE(dynamic_cast<controls::ActionButton*>(play)->getTooltip().contains("48.0 kHz"));
    binding.setPreviewDeviceRate(48000);binding.display(restored);REQUIRE(play->isEnabled());
    // The shared pair uses one session/pass intent for a continuous fader edit.
    unsigned mixCommits=0;binding.onLoopLevels=[&](auto,uint64_t,uint64_t){++mixCommits;};
    binding.mixControl().beforeInvocation();++restored.songExport.pass;binding.display(restored);
    binding.mixControl().dry.setValue(50,juce::sendNotificationSync);REQUIRE(mixCommits==0 && binding.mixControl().dry.getValue()==0);
    binding.mixControl().beforeInvocation();binding.mixControl().dry.setValue(50,juce::sendNotificationSync);REQUIRE(mixCommits==1);
    binding.setLoopPresentation({});REQUIRE(!binding.loopOptions().isVisible() && binding.placementText().isEmpty());
    auto& mix=panel.workspace.mixControl();
    REQUIRE(mix.isVisible() && mix.selector.getText()=="Wet Layer" && mix.dry.getValue()==0 && mix.wet.getValue()==100);
    mix.beforeInvocation();mix.selector.setSelectedId(5,juce::sendNotificationSync);
    REQUIRE(panel.levels==controls::ExportLevelsControl::Pair(1,1) && mix.selector.getText()=="Cinematic");
    mix.beforeInvocation();mix.dry.setValue(35,juce::sendNotificationSync);mix.wet.setValue(65,juce::sendNotificationSync);
    REQUIRE(panel.levels==controls::ExportLevelsControl::Pair(.35f,.65f) && mix.selector.getText()=="Custom");
    REQUIRE(panel.workspace.placementText().contains("Mute the original dry"));
    REQUIRE(panel.display.songExport.prepared->plan->dry==.35f && panel.display.songExport.prepared->plan->wet==.65f);
    for(const auto size:std::array<juce::Point<int>,2>{{{900,600},{1400,900}}}){
        panel.setSize(size.x,size.y);panel.refresh();
        snapshotPanel(panel,"loop-export-mix-"+juce::String(size.x)+"x"+juce::String(size.y));
    }
    mix.beforeInvocation();mix.wet.setValue(0,juce::sendNotificationSync);
    REQUIRE(panel.display.songExport.prepared->plan->origin==0);
    mix.dry.setValue(0,juce::sendNotificationSync);
    REQUIRE(!panel.display.songExport.prepared->metrics.audibleStart && panel.display.songExport.phase==ExportPhase::idle);
    mix.beforeInvocation();mix.selector.setSelectedId(2,juce::sendNotificationSync);
    REQUIRE(panel.levels==controls::ExportLevelsControl::Pair(0,1));
    for(auto size:std::array<juce::Point<int>,2>{{{900,600},{1400,900}}}){
        panel.setSize(size.x,size.y);panel.refresh();
        REQUIRE(panel.getLocalBounds().contains(panel.workspace.getBounds()));
        const auto& w=panel.workspace;
        REQUIRE(w.getLocalBounds().contains(w.waveformBounds()) && w.waveformBounds().getHeight()>100);
        REQUIRE(w.getLocalBounds().contains(w.rulerBounds()));
        REQUIRE(w.getLocalBounds().contains(panel.workspace.loopOptions().getBounds()));
        REQUIRE(!w.waveformBounds().intersects(panel.workspace.loopOptions().getBounds()));
        REQUIRE(w.getLocalBounds().contains(mix.getBounds()));
        for(auto* child:mix.getChildren())REQUIRE(mix.getLocalBounds().contains(child->getBounds()));
        REQUIRE(w.placementText().startsWith("Place WAV"));
        for(auto* child:panel.workspace.loopOptions().getChildren())
            REQUIRE(panel.workspace.loopOptions().getLocalBounds().contains(child->getBounds()));
        for(const auto* name:{"exportSave","exportReturn","exportDrag"}){
            const auto* child=Panel::find(panel.workspace,name);REQUIRE(child && w.getLocalBounds().contains(child->getBounds()));
            if(juce::String(name)=="exportDrag")REQUIRE(child->getHeight()>=responsiveGeometry::exportTileHeight);
        }
        snapshotPanel(panel,"loop-export-"+juce::String(size.x)+"x"+juce::String(size.y));
    }
    panel.setLookAndFeel(nullptr);
}

inline void review()
{
    class Window final:public juce::DocumentWindow
    {
    public:
        Window():juce::DocumentWindow("ReverbArise #128 - Loop Export preview",background,juce::DocumentWindow::closeButton)
        {setUsingNativeTitleBar(true);setContentOwned(new Panel,true);setResizable(true,false);setResizeLimits(900,630,1600,1130);centreWithSize(900,630);setVisible(true);}
        void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
    };
    WorkspaceTheme theme;Window window;window.setLookAndFeel(&theme);
    juce::MessageManager::getInstance()->runDispatchLoop();window.setLookAndFeel(nullptr);
}
}
