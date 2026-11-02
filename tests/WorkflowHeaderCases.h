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
    unsigned requests=0;header.onModeRequested=[&](auto){++requests;};
    for(const auto& item:cases)for(int width:{1068,1360,1560})
    {
        header.setSize(width,70);header.display(m3::ui::presentWorkflow(item.state,item.ready));
        REQUIRE(header.modeSelector().selectedId()==static_cast<int>(item.state.mode)+1 && requests==0);
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
