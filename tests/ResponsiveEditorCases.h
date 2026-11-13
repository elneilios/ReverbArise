#pragma once

// Exercise the actual editor and model-bound controls, including software pixel
// checks. These checks do not stand in for the owner native/DPI checkpoint.
void Composition::verifyResponsive(const juce::File& output)
{
    stopTimer();setScene(0);auto& engine=processor->swellEngine();
    auto panel=productionEditor();panel->stopAutomaticRefreshForReview();panel->enableResponsiveReview({},PresentationFitContext{{4000,3000},{0,0}});
    using Page=m3::ui::SettingsPage;auto& settings=*panel->settingsPanel();
    auto& swellSettings=*settings.page(Page::swell);
    const auto checkSettingsRoutes=[&]
    {
        REQUIRE(require<juce::ComboBox>(swellSettings,"fadeInCurve").isVisible());
        REQUIRE(require<juce::ComboBox>(swellSettings,"fadeOutCurve").isVisible());
        int actions=0;
        for(auto* child:swellSettings.getChildren())if(auto* button=dynamic_cast<juce::Button*>(child))
            if(button->getButtonText()=="Mute" || button->getButtonText()=="Full response")
            {REQUIRE(button->isVisible());++actions;}
        REQUIRE(actions==2);
        const std::array<int,5> expectedSwitches{1,3,1,2,1};
        for (int page=0;page<static_cast<int>(expectedSwitches.size());++page)
        {
            const auto* content=settings.page(static_cast<Page>(page));
            int switches=0;
            for(auto* child:content->getChildren())
                if(auto* toggle=dynamic_cast<controls::Switch*>(child))
                {
                    ++switches;
                    REQUIRE(toggle->getHeight()==controlGeometry::switchHeight);
                    REQUIRE(toggle->indicatorBounds().getWidth()==controlGeometry::switchTrackWidth);
                    REQUIRE(toggle->getLocalBounds().toFloat().contains(toggle->focusInkBounds()));
                    REQUIRE(juce::GlyphArrangement::getStringWidth(textFont(TextRole::controlLabel),toggle->getButtonText())
                            <= toggle->labelBounds().getWidth());
                }
            REQUIRE(switches==expectedSwitches[static_cast<size_t>(page)]);
        }
    };
    const auto original=engine.savedSession();const auto sourceRevision=processor->getCaptureSnapshot().manifest.revision;
    panel->setSize(900,600);panel->refresh();
    auto& target=require<KnobValue>(*panel,"arrivalSampleCell");
    auto& targetField=target.valueField();
    auto& targetFeedback=require<juce::Component>(*panel,"arrivalSampleFeedback");
    auto& hoverHelp=require<controls::EllipsisLabel>(*panel,"hoverHelp");
    auto& sampleUnits=require<juce::ToggleButton>(swellSettings,"sampleUnits");
    for(const bool samples:{false,true,false})
    {
        sampleUnits.setToggleState(samples,juce::dontSendNotification);
        sampleUnits.onClick();
        const auto* format=samples?"whole sample position":"bar.beat.fraction";
        const auto* otherFormat=samples?"bar.beat.fraction":"whole sample position";
        REQUIRE(targetFeedback.getBounds().isEmpty());
        REQUIRE(targetField.isAccessible());
        const auto description=targetField.getDescription();
        REQUIRE(description.contains(format) && !description.contains(otherFormat));
        for(auto* control:{static_cast<juce::Component*>(&targetField),static_cast<juce::Component*>(&target.valueKnob())})
        {
            const auto point=control->getLocalBounds().getCentre().toFloat();
            const auto now=juce::Time::getCurrentTime();
            panel->mouseEnter(juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,{},0,0,0,0,0,control,control,now,point,now,0,false));
            REQUIRE(hoverHelp.getText().contains(format) && !hoverHelp.getText().contains(otherFormat));
        }
    }
    REQUIRE(hoverHelp.getText().contains("Right-click for reset options"));
    targetField.setText("1",false);targetField.onReturnKey();panel->refresh();
    REQUIRE(!targetFeedback.getBounds().isEmpty() && target.getLocalBounds().contains(targetFeedback.getBounds()));
    target.setValidation({});panel->refresh();
    REQUIRE(targetFeedback.getBounds().isEmpty() && targetField.getDescription().contains("bar.beat.fraction"));
    const auto checkVisible=[&](const char* name)
    {
        auto& child=require<juce::Component>(*panel,name);REQUIRE(child.isVisible());
        const auto bounds=panel->getLocalArea(&child,child.getLocalBounds());REQUIRE(!bounds.isEmpty());REQUIRE(panel->getLocalBounds().contains(bounds));
    };
    const auto capture=[&](const juce::String& name)
    {
        const auto image=panel->createComponentSnapshot(panel->getLocalBounds());REQUIRE(image.isValid());
        if(output.getFullPathName().isNotEmpty() && (panel->getWidth()==900 || (panel->getWidth()==1400 && panel->getHeight()==1000)))
        {REQUIRE(output.createDirectory().wasOk());auto stream=output.getChildFile(name+".png").createOutputStream();REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk() && juce::PNGImageFormat().writeImageToStream(image,*stream));}
        return image;
    };
    const auto checkTabFit=[](const controls::SegmentedSelector& strip,float textHeight,bool withIcons=false)
    {
        const auto font=interFont(textHeight,InterFace::semibold);
        const auto focusMargin=static_cast<int>(controlGeometry::focusOutset+controlGeometry::focusStroke);
        for(auto* child:strip.getChildren())
        {
            const auto& button=dynamic_cast<const juce::Button&>(*child);
            const auto labelWidth=juce::GlyphArrangement::getStringWidth(font,button.getButtonText());
            REQUIRE(labelWidth+10+(withIcons?24:0)<=button.getWidth());
            REQUIRE(strip.getLocalBounds().contains(button.getBounds().expanded(focusMargin)));
        }
    };
    for(auto size:{juce::Point<int>{900,600},{955,700},{956,700},{957,700},{1000,700},{1099,849},{1100,850},{1101,851},{1199,850},{1200,850},{1201,850},{1400,900},{1399,999},{1400,1000},{1401,1001},{1600,1000}})
    {
        panel->setSize(size.x,size.y);panel->refresh();
        checkSettingsRoutes();
        const ResponsiveWorkspaceLayout layout(panel->getLocalBounds());
        checkVisible("resetWetShape");checkVisible("resetSwellMenu");
        auto& enabled=require<controls::BooleanToggle>(*panel,"swellEnabled");
        auto& solo=require<controls::BooleanToggle>(*panel,"swellSolo");
        auto& freeze=require<controls::BooleanToggle>(*panel,"freezeRender");
        for(auto* flag:{&enabled,&solo,&freeze})
        {
            REQUIRE(flag->getY()==enabled.getY() && flag->indicatorBounds().getWidth()==18);
            REQUIRE(flag->getLocalBounds().toFloat().contains(flag->focusInkBounds()));
            REQUIRE(juce::GlyphArrangement::getStringWidth(componentTextFont(*flag,TextRole::controlLabel),flag->getButtonText())
                    <= flag->labelBounds().getWidth());
        }
        REQUIRE(enabled.getRight()<=solo.getX() && solo.getRight()<=freeze.getX());
        auto& notice=require<controls::NoticeBar>(*panel,"workspaceNotice");
        REQUIRE(notice.currentMessage() && notice.completeMessageFits());
        REQUIRE(notice.displayedTextHeight()>=responsiveGeometry::noticeMinimum);
        auto& reset=require<juce::Component>(*panel,"resetWetShape");REQUIRE(reset.getParentComponent()==find(*panel,"presentationContent"));
        REQUIRE(reset.getBottom()<=layout.work.getBottom()-responsiveGeometry::contentPadding);
        REQUIRE(require<controls::TimelineZoomControls>(*panel,"performZoom").getWidth()>=controls::TimelineZoomControls::minimumWidth());
        REQUIRE(panel->timelineComponent().sourceLane().getHeight()>=70);REQUIRE(panel->timelineComponent().wetLane().getHeight()>=70);
        auto& overview=require<m3::SwellOverview>(*panel,"songOverview");REQUIRE(overview.waveformBounds().getHeight()>=28);
        auto& tabs=require<controls::SegmentedSelector>(*panel,"inspectorSections");
        const auto type=ResponsiveTypography::at(size.x,size.y);
        checkTabFit(require<WorkflowHeader>(*panel,"workflowHeader").modeSelector(),type.navigation,true);
        checkTabFit(tabs,type.body);
        for(int section=1;section<=3;++section)
        {
            tabs.onRequest(section);panel->refresh();
            checkSettingsRoutes();
            checkVisible("resetWetShape");
            if(section==3)
            {
                auto& ir=require<controls::SoundPresetEditor>(*panel,"selectedIrEditor");
                auto& level=require<KnobValue>(*panel,"swellLevelCell");
                REQUIRE(ir.getBounds()==layout.fields && ir.isParentOf(&level));
                REQUIRE(!ir.isParentOf(&require<juce::Component>(*panel,"resetWetShape")));
                REQUIRE(ir.getLocalArea(&level,level.getLocalBounds()).getY()>=
                        ir.getLocalArea(&ir.presetPicker(),ir.presetPicker().getLocalBounds()).getBottom());
            }
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
        checkTabFit(learnTabs,type.body);
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
            const auto& recapture=require<controls::Switch>(learn,"recaptureExisting");
            REQUIRE(sourceHeading.getX()==learn.getLocalArea(&recapture,recapture.labelBounds()).getX());
            auto& ir=require<controls::SoundPresetEditor>(learn,"learnIrEditor");
            REQUIRE(ir.getY()==learnTabs.getBottom()+responsiveGeometry::contentPadding);
            const auto gutter=require<juce::Label>(learn,"learnCaptureHeading").getX();
            REQUIRE(controls::commandBodyBounds(learnTabs.getBounds()).getX()==gutter);
            REQUIRE(sourceHeading.getX()==gutter);
            REQUIRE(learn.getLocalArea(&ir.pickerLabel(),ir.pickerLabel().getLocalBounds()).getX()==gutter);
            auto& previous=require<juce::Component>(ir,"soundPrevious");
            REQUIRE(learn.getLocalArea(&previous,controls::commandBodyBounds(previous.getLocalBounds())).getX()==gutter);
            if(ir.filePicker().isVisible())
            {
                const auto fileBody=learn.getLocalArea(&ir.filePicker(),controls::commandBodyBounds(ir.filePicker().getLocalBounds()));
                const auto importBody=learn.getLocalArea(&ir.importAction(),controls::commandBodyBounds(ir.importAction().getLocalBounds()));
                REQUIRE(fileBody.getX()==gutter && fileBody.getRight()<importBody.getX());
            }
            auto manualView=enabledView;manualView.session.automaticProcessing=false;manualView.session.autoMarkersEnabled=false;
            learn.display(processor->getCaptureSnapshot(),manualView,processor->getSongTime(),processor->getWorkflowSnapshot(),true);
            for(const auto* name:{"learnIRLabel","learnDefaultIR","levelCell"})
            {
                auto& control=require<juce::Component>(learn,name);
                bool shown=true;for(auto* ancestor=&control;ancestor && ancestor!=&learn;ancestor=ancestor->getParentComponent())shown=shown && ancestor->isVisible();
                REQUIRE(shown==(section==2));
                if(section==2)checkVisible(name);
            }
            REQUIRE(!require<juce::Component>(learn,"learnMarkerSensitivity").isVisible());
            learn.display(processor->getCaptureSnapshot(),enabledView,processor->getSongTime(),processor->getWorkflowSnapshot(),true);
            for(const auto* name:{"learnGenerateSwells","learnManualSwellsHelp"})checkVisible(name);
            if(section==1){checkVisible("learnAutoGenerateOnStop");checkVisible("learnMarkerSensitivity");}
            else{checkVisible("learnDefaultIR");checkVisible("levelCell");}
            const auto& generate=require<juce::Component>(learn,"learnGenerateSwells");
            const auto& manual=require<juce::Component>(learn,"learnManualSwellsHelp");
            REQUIRE(generate.getBottom()<=manual.getY());
            for(const auto* name:{"learnAutoGenerateOnStop","learnMarkerSensitivity","learnDefaultIR","levelCell"})
            {
                auto& control=require<juce::Component>(learn,name);
                auto bounds=learn.getLocalArea(&control,control.getLocalBounds());
                for(auto* ancestor=&control;ancestor && ancestor!=&learn;ancestor=ancestor->getParentComponent())
                {
                    if(!ancestor->isVisible()){bounds={};break;}
                    bounds=bounds.getIntersection(learn.getLocalArea(ancestor,ancestor->getLocalBounds()));
                }
                if(!bounds.isEmpty()) REQUIRE(bounds.getBottom()+responsiveGeometry::contentPadding<=generate.getY());
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
