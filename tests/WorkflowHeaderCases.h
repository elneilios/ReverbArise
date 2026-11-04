#pragma once

static void workflowHeaderCases()
{
    struct Case {const char* name;workflow::Snapshot state;workflow::Readiness ready;};
    const std::array<Case,10> cases{{
        {"empty",{},{}},
        {"dry-only",{workflow::Mode::live},{true,true,false}},
        {"ready",{workflow::Mode::live},{true,true,true}},
        {"recording",{workflow::Mode::capture,workflow::Phase::recording},{true,true,true}},
        {"draining",{workflow::Mode::capture,workflow::Phase::draining},{true,true,true}},
        {"validating",{workflow::Mode::capture,workflow::Phase::validating},{true,true,true}},
        {"failed",{workflow::Mode::capture,workflow::Phase::failed},{true,true,true}},
        {"incompatible",{workflow::Mode::live},{true,false,true}},
        {"frozen-recovery",{workflow::Mode::live},{false,true,true,true}},
        {"export",{workflow::Mode::finalise},{true,true,true}}
    }};
    m3::ui::WorkspaceTheme theme;m3::ui::WorkflowHeader header;header.setLookAndFeel(&theme);header.addToDesktop(0);header.setVisible(true);
    const auto approvedFont=[](const juce::Font& font){return font.getTypefacePtr()->getName()=="Inter";};
    const auto navigationFont=m3::ui::navigationFont();
    REQUIRE(approvedFont(navigationFont) && navigationFont.getTypefaceStyle().containsIgnoreCase("Semibold"));
    REQUIRE(navigationFont.getTypefacePtr()->getStyle().containsIgnoreCase("Semibold"));
    for(const auto face:{m3::ui::InterFace::regular,m3::ui::InterFace::semibold,m3::ui::InterFace::bold,m3::ui::InterFace::italic})
    {const auto pointer=m3::ui::interTypeface(face);REQUIRE(pointer && pointer->getName()=="Inter" && pointer->getSupportedVariables().empty());}
    juce::Label semibold;semibold.setFont(navigationFont);REQUIRE(theme.getLabelFont(semibold).getTypefacePtr()==navigationFont.getTypefacePtr());
    for(auto* child:header.getChildren())if(auto* label=dynamic_cast<juce::Label*>(child))REQUIRE(approvedFont(theme.getLabelFont(*label)));
    unsigned requests=0;header.onModeRequested=[&](auto){++requests;};
    for(const auto& item:cases)for(int width:{1068,1360,1560})
    {
        header.setSize(width,82);header.display(m3::ui::presentWorkflow(item.state,item.ready));
        REQUIRE(header.modeSelector().selectedId()==static_cast<int>(item.state.mode)+1 && requests==0);
        REQUIRE(header.settingsAction().getY()==header.modeSelector().getY() && header.settingsAction().getBottom()==header.modeSelector().getBottom());
        for(const char* name:{"brandWordmark","brandTagline"}){const auto component=findNamed(header,name);REQUIRE(component && component->getBounds().getCentreY()==header.modeSelector().getBounds().getCentreY());}
        auto description=dynamic_cast<m3::ui::controls::HeaderInformation*>(findNamed(header,"workflowRole"));REQUIRE(description);
        const auto descriptionFont=m3::ui::fontFor(*description,m3::ui::bodySize);
        REQUIRE(!descriptionFont.isItalic() && approvedFont(descriptionFont));
        REQUIRE(description->getY()>=header.modeSelector().getBottom()+4);
        REQUIRE(description->getBounds().getCentreY()==62 && descriptionFont.getHeight()==14);
        REQUIRE(description->getTitle()==m3::ui::modeSubtitle(item.state.mode));
        REQUIRE(description->accentBounds()==juce::Rectangle<int>(0,8,2,16));
        REQUIRE(juce::GlyphArrangement::getStringWidth(descriptionFont,description->getTitle())<=description->textBounds().getWidth());
        const auto informationImage=description->createComponentSnapshot(description->getLocalBounds());
        REQUIRE(informationImage.getPixelAt(0,16)==m3::ui::aqua);
        REQUIRE(!findNamed(header,"workflowStatus")); // Operational messages have their own inline bar.
        for(const auto target:{workflow::Mode::capture,workflow::Mode::live,workflow::Mode::finalise})
        {
            const auto segment=header.modeSelector().segment(static_cast<int>(target)+1);REQUIRE(segment);
            REQUIRE(segment->isEnabled()==(workflow::entryBlock(item.state,target,item.ready)==workflow::EntryBlock::none));
            if(!segment->isEnabled()){const auto accessible=segment->getAccessibilityHandler();REQUIRE(accessible && accessible->getDescription().isNotEmpty());}
        }
        const auto children=header.getChildren();
        for(int i=0;i<children.size();++i){REQUIRE(header.getLocalBounds().contains(children[i]->getBounds()));
            for(int j=i+1;j<children.size();++j)REQUIRE(!children[i]->getBounds().intersects(children[j]->getBounds()));}
        snapshotPanel(header,"header-"+juce::String(item.name)+"-"+juce::String(width));
    }
    header.setLookAndFeel(nullptr);header.removeFromDesktop();
}
