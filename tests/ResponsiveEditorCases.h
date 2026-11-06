#pragma once

// Exercise the actual editor and model-bound controls, including software pixel
// checks. These checks do not stand in for the owner native/DPI checkpoint.
void Composition::verifyResponsive(const juce::File& output)
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    auto panel=productionEditor();panel->stopAutomaticRefreshForReview();panel->enableResponsiveReview({},PresentationFitContext{{4000,3000},{0,0}});
    auto& settings=dynamic_cast<juce::TabbedComponent&>(*panel->settingsPanel());
    auto& swellSettings=*settings.getTabContentComponent(2);
    const auto checkSettingsRoutes=[&]
    {
        REQUIRE(require<juce::ComboBox>(swellSettings,"fadeInCurve").isVisible());
        REQUIRE(require<juce::ComboBox>(swellSettings,"fadeOutCurve").isVisible());
        int actions=0;
        for(auto* child:swellSettings.getChildren())if(auto* button=dynamic_cast<juce::Button*>(child))
            if(button->getButtonText()=="Mute" || button->getButtonText()=="Full response")
            {REQUIRE(button->isVisible());++actions;}
        REQUIRE(actions==2);
    };
    const auto original=engine.savedSession();const auto sourceRevision=processor->getCaptureSnapshot().manifest.revision;
    const auto checkVisible=[&](const char* name)
    {
        auto& child=require<juce::Component>(*panel,name);REQUIRE(child.isVisible());
        const auto bounds=panel->getLocalArea(&child,child.getLocalBounds());REQUIRE(!bounds.isEmpty());REQUIRE(panel->getLocalBounds().contains(bounds));
    };
    const auto capture=[&](const juce::String& name)
    {
        const auto image=panel->createComponentSnapshot(panel->getLocalBounds());REQUIRE(image.isValid());
        if(output.getFullPathName().isNotEmpty() && (panel->getWidth()==900 || (panel->getWidth()==1400 && panel->getHeight()==1000)))
        {REQUIRE(output.createDirectory().wasOk());auto stream=output.getChildFile(name+".png").createOutputStream();REQUIRE(stream && juce::PNGImageFormat().writeImageToStream(image,*stream));}
        return image;
    };
    for(auto size:{juce::Point<int>{900,600},{955,700},{956,700},{957,700},{1000,700},{1099,849},{1100,850},{1101,851},{1199,850},{1200,850},{1201,850},{1400,900},{1399,999},{1400,1000},{1401,1001},{1600,1000}})
    {
        panel->setSize(size.x,size.y);panel->refresh();
        checkSettingsRoutes();
        const ResponsiveWorkspaceLayout layout(panel->getLocalBounds());
        checkVisible("resetWetShape");checkVisible("resetSwellMenu");
        auto& notice=require<controls::NoticeBar>(*panel,"workspaceNotice");
        REQUIRE(notice.currentMessage() && notice.completeMessageFits());
        REQUIRE(notice.displayedTextHeight()>=responsiveGeometry::noticeMinimum);
        auto& reset=require<juce::Component>(*panel,"resetWetShape");REQUIRE(reset.getParentComponent()==find(*panel,"presentationContent"));
        REQUIRE(reset.getBottom()<=layout.work.getBottom()-responsiveGeometry::contentPadding);
        REQUIRE(require<controls::TimelineZoomControls>(*panel,"performZoom").getWidth()>=controls::TimelineZoomControls::minimumWidth());
        REQUIRE(panel->timelineComponent().sourceLane().getHeight()>=70);REQUIRE(panel->timelineComponent().wetLane().getHeight()>=70);
        auto& overview=require<m3::SwellOverview>(*panel,"songOverview");REQUIRE(overview.waveformBounds().getHeight()>=28);
        auto& tabs=require<controls::SegmentedSelector>(*panel,"inspectorSections");
        for(int section=1;section<=3;++section)
        {
            tabs.onRequest(section);panel->refresh();
            checkSettingsRoutes();
            checkVisible("resetWetShape");
            for(const auto& cellName:std::vector<juce::String>{"arrivalSampleCell","sourceSampleStartCell","sourceSampleEndCell","preRollCell","endOffsetCell","fadeInMsCell","fadeOutMsCell","swellLevelCell","curveInTensionCell","curveOutTensionCell","wetPlacementCell"})
            {
                auto& cell=require<KnobValue>(*panel,cellName);
                if(!cell.isVisible())continue;
                REQUIRE(cell.getLocalBounds().contains(cell.valueKnob().getBounds()));REQUIRE(cell.getLocalBounds().contains(cell.valueField().getBounds()));
                const auto field=panel->getLocalArea(&cell.valueField(),cell.valueField().getLocalBounds());REQUIRE(layout.fields.contains(field));
                REQUIRE(field.getHeight()>=responsiveGeometry::entryHeight);
            }
            const auto image=capture("perform-"+juce::String(section)+"-"+juce::String(size.x)+"x"+juce::String(size.y));
            for(const auto x:{layout.list.getRight(),layout.inspector.getX()})
            {
                REQUIRE(image.getPixelAt(x,layout.work.getY()+responsiveGeometry::dividerClearance-2)==background);
                REQUIRE(image.getPixelAt(x,layout.work.getBottom()-responsiveGeometry::dividerClearance+2)==background);
                REQUIRE(image.getPixelAt(x,layout.work.getCentreY())==separator);
            }
            for(const auto y:{layout.work.getBottom(),layout.audition.getBottom(),layout.overview.getBottom()})
                REQUIRE(image.getPixelAt(layout.header.getCentreX(),y)==separator);
        }
        REQUIRE(processor->requestWorkflowMode(workflow::Mode::capture));panel->refresh();
        checkSettingsRoutes();
        auto& learnTabs=require<controls::SegmentedSelector>(*panel,"learnSections");
        for(int section=1;section<=2;++section)
        {
            learnTabs.onRequest(section);panel->refresh();checkVisible("learnSections");
            capture("learn-"+juce::String(section)+"-"+juce::String(size.x)+"x"+juce::String(size.y));
            // View-only dense state: exercise the real Learn components without
            // enabling generation in the processor or scheduling audio work.
            auto enabledView=engine.snapshot();enabledView.session.automaticProcessing=true;enabledView.session.autoMarkersEnabled=true;
            auto& learn=require<LearnWorkspace>(*panel,"learnWorkspace");
            auto& sourceHeading=require<juce::Label>(learn,"learnSourceCaptureHeading");
            REQUIRE(sourceHeading.getBorderSize().getLeft()==0);
            REQUIRE(sourceHeading.getX()==require<juce::Label>(learn,"learnRecaptureLabel").getX());
            auto& scope=require<juce::Label>(learn,"learnSoundScope");
            auto& irLabel=require<juce::Label>(learn,"learnIRLabel");
            auto& irStatus=require<juce::Label>(learn,"learnIRStatus");
            REQUIRE(irLabel.getY()==learnTabs.getBottom()+responsiveGeometry::contentPadding);
            REQUIRE(scope.getY()==irStatus.getBottom()+responsiveGeometry::contentPadding);
            REQUIRE(scope.getX()==irLabel.getX() && scope.getBorderSize().getLeft()==0);
            auto manualView=enabledView;manualView.session.automaticProcessing=false;manualView.session.autoMarkersEnabled=false;
            learn.display(processor->getCaptureSnapshot(),manualView,processor->getSongTime(),processor->getWorkflowSnapshot(),true);
            for(const auto* name:{"learnIRLabel","learnDefaultIR","learnImportIR","learnIRStatus"})
            {
                auto& control=require<juce::Component>(learn,name);
                REQUIRE(control.isVisible()==(section==2));
                if(section==2)checkVisible(name);
            }
            REQUIRE(!require<juce::Component>(learn,"learnMarkerSensitivity").isVisible());
            learn.display(processor->getCaptureSnapshot(),enabledView,processor->getSongTime(),processor->getWorkflowSnapshot(),true);
            for(const auto* name:{"learnGenerateSwells","learnGenerateReason","learnManualSwellsHelp"})checkVisible(name);
            if(section==1){checkVisible("learnAutoGenerateOnStop");checkVisible("learnMarkerSensitivity");}
            else{checkVisible("learnDefaultIR");checkVisible("learnImportIR");}
            const auto& generate=require<juce::Component>(learn,"learnGenerateSwells");
            const auto& reason=require<juce::Component>(learn,"learnGenerateReason");
            const auto& manual=require<juce::Component>(learn,"learnManualSwellsHelp");
            REQUIRE(generate.getBottom()<=reason.getY() && reason.getBottom()<=manual.getY());
            for(const auto* name:{"learnAutoGenerateOnStop","learnMarkerSensitivity","learnDefaultIR","learnImportIR"})
            {
                auto& control=require<juce::Component>(learn,name);
                if(control.isVisible())REQUIRE(control.getBottom()+responsiveGeometry::contentPadding<=generate.getY());
            }
        }
        REQUIRE(processor->requestWorkflowMode(workflow::Mode::finalise));panel->refresh();
        wait([&]{return engine.snapshot().songExport.phase==m3::ExportPhase::ready || engine.snapshot().songExport.phase==m3::ExportPhase::error;});
        REQUIRE(engine.snapshot().songExport.phase==m3::ExportPhase::ready);panel->refresh();
        checkSettingsRoutes();
        for(const auto* name:{"exportPlay","exportStop","exportLevels","exportMixCharacter","exportDry","exportWet","exportSave","exportReturn","exportDrag"})checkVisible(name);
        auto& levels=require<controls::ExportLevelsControl>(*panel,"exportLevels");
        for(auto* slider:{&levels.dry,&levels.wet})
        {
            bool checked=false;
            for(auto* child:slider->getChildren())if(auto* numeric=dynamic_cast<controls::NumericInputControl*>(child))
            {REQUIRE(std::abs(numeric->getFont().getHeight()-ResponsiveTypography::at(size.x,size.y).value)<.01f);checked=true;}
            REQUIRE(checked);
        }
        REQUIRE(levels.selector.getBottom()+spacing::labelHelp<=levels.dry.getY());
        REQUIRE(levels.dry.getBottom()+spacing::labelHelp<=levels.wet.getY());
        REQUIRE(levels.getLocalBounds().contains(levels.wet.getBounds()));
        REQUIRE(levels.getBottom()+responsiveGeometry::groupGap<=require<juce::Component>(*panel,"exportDrag").getY());
        capture("export-"+juce::String(size.x)+"x"+juce::String(size.y));
        auto acceptedExport=engine.snapshot();const auto identity=acceptedExport.songExport.prepared->plan->identity;
        auto& exportView=require<ExportWorkspace>(*panel,"exportWorkspace");
        for(const auto phase:{m3::ExportPhase::preparing,m3::ExportPhase::error})
        {
            auto transient=acceptedExport;transient.songExport.phase=phase;
            transient.songExport.message="Requested mix is unavailable. Existing accepted audio and the last prepared WAV are retained.";
            exportView.display(transient);
            REQUIRE(!require<juce::Component>(exportView,"exportSave").isEnabled());
            REQUIRE(!require<juce::Component>(exportView,"exportDrag").isEnabled());
            checkVisible("exportReturn");REQUIRE(panel->createComponentSnapshot(panel->getLocalBounds()).isValid());
            REQUIRE(engine.snapshot().songExport.prepared->plan->identity==identity);
        }
        exportView.display(acceptedExport);
        REQUIRE(processor->requestWorkflowMode(workflow::Mode::live));panel->refresh();
    }
    const auto after=engine.savedSession();REQUIRE(original.swells.size()==after.swells.size());
    for(size_t i=0;i<original.swells.size();++i)
    {const auto& a=original.swells[i];const auto& b=after.swells[i];REQUIRE(a.id==b.id && a.sourceStart==b.sourceStart && a.sourceEnd==b.sourceEnd && a.shape==b.shape && a.ir==b.ir && a.render==b.render && a.inputGeneration==b.inputGeneration);}
    REQUIRE(processor->getCaptureSnapshot().manifest.revision==sourceRevision);
    for(const int scene:{1,2})
    {
        setScene(scene);panel->setPresentationSize(900,600);panel->refresh();
        if(scene==2)wait([&]{const auto restored=engine.snapshot();return std::all_of(restored.session.swells.begin(),restored.session.swells.end(),[](const auto& swell){return swell.phase==m3::Phase::ready;});});
        for(const auto mode:{workflow::Mode::capture,workflow::Mode::live,workflow::Mode::finalise})
        {
            const bool admitted=processor->requestWorkflowMode(mode);panel->refresh();
            if(scene==1 && mode==workflow::Mode::finalise)
            {REQUIRE(!admitted && processor->getWorkflowSnapshot().mode==workflow::Mode::live);continue;}
            REQUIRE(admitted);
            const auto before=engine.savedSession();const auto jobs=engine.snapshot().renderCount;
            for(const int scale:{125,100})
            {REQUIRE(panel->setPresentationScale(scale));panel->refresh();REQUIRE(panel->createComponentSnapshot(panel->getLocalBounds()).isValid());}
            REQUIRE(engine.snapshot().renderCount==jobs && engine.savedSession().swells.size()==before.swells.size());
            if(mode==workflow::Mode::live)
            {
                auto& action=require<juce::Component>(*panel,"resetWetShape");
                REQUIRE(action.isVisible()==!before.primary.isEmpty());
                REQUIRE(require<juce::Component>(*panel,"inspectorSections").isVisible()==!before.primary.isEmpty());
            }
        }
    }
}
