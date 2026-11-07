#pragma once
#include "WorkflowPresentation.h"
#include "Controls/SegmentedSelector.h"
#include "Controls/IconActionButton.h"
#include "Controls/HeaderInformation.h"

namespace m3::ui
{
class WorkflowHeader final:public juce::Component
{
public:
    WorkflowHeader()
    {
        setName("workflowHeader");wordmark.setName("brandWordmark");tagline.setName("brandTagline");selector.setName("workflowMode");
        selector.setTitle("Workflow mode");settings.setName("diagnostics");
        tagline.setText("Reverse Reverb for Creative Swells",juce::dontSendNotification);
        applyTextRole(tagline,TextRole::tagline);
        role.setName("workflowRole");
        addAndMakeVisible(wordmark);addAndMakeVisible(tagline);addAndMakeVisible(selector);
        addAndMakeVisible(settings);addAndMakeVisible(role);
        selector.onRequest=[this](int id){const auto callback=onModeRequested;if(callback)callback(static_cast<workflow::Mode>(id-1));};
        settings.onClick=[this]{const auto callback=onSettings;if(callback)callback();};
        display(presentWorkflow({},{}));
    }
    std::function<void(workflow::Mode)> onModeRequested;
    std::function<void()> onSettings;
    void display(const WorkflowPresentation& presentation,const juce::String& detail={})
    {
        std::vector<controls::SegmentChoice> choices;
        for(const auto& choice:presentation.choices)
        {
            const auto icon=choice.mode==workflow::Mode::capture?controls::ModeIcon::learn
                :choice.mode==workflow::Mode::live?controls::ModeIcon::perform:controls::ModeIcon::exportAudio;
            choices.push_back({static_cast<int>(choice.mode)+1,choice.label,choice.subtitle,choice.available,choice.reason,icon});
        }
        selector.display(std::move(choices),static_cast<int>(presentation.selected)+1);
        const auto subtitle=presentation.choices[static_cast<size_t>(presentation.selected)].subtitle;role.display(subtitle);
        setDescription(juce::String(modeLabel(presentation.selected))+": "+subtitle);setHelpText(detail);
    }
    controls::SegmentedSelector& modeSelector()noexcept{return selector;}
    controls::IconActionButton& settingsAction()noexcept{return settings;}
    controls::HeaderInformation& modeInformation()noexcept{return role;}
    void setResponsivePresentation(ResponsiveTypography type)
    {responsive=true;wordmark.setOpticalAlignment(true);tagline.setVisible(false);selector.setTextHeight(type.navigation);role.setWrappedPresentation(true);resized();}
    void resized()override
    {
        if(responsive)
        {
            using namespace responsiveGeometry;
            settings.setBounds(getWidth()-36,0,36,primaryHeight);
            selector.setBounds(getWidth()-378,0,330,primaryHeight);
            wordmark.setBounds(0,6,201,25);role.setBounds(220,2,std::max(0,getWidth()-610),32);return;
        }
        constexpr int primaryHeight=46,controlHeight=36,controlY=(primaryHeight-controlHeight)/2;
        auto area=getLocalBounds();auto gear=area.removeFromRight(36);settings.setBounds(gear.withY(controlY).withHeight(controlHeight));
        area.removeFromRight(12);const auto column=area.removeFromRight(getWidth()<1200?330:360);
        selector.setBounds(column.withY(controlY).withHeight(controlHeight));role.setBounds(column.withY(primaryHeight).withHeight(32));
        area.removeFromRight(16);wordmark.setBounds(area.removeFromLeft(getWidth()<1200?250:290).withY(0).withHeight(primaryHeight));
        area.removeFromLeft(16);tagline.setBounds(area.withY(0).withHeight(primaryHeight));
    }
private:
    Wordmark wordmark;juce::Label tagline;
    controls::HeaderInformation role;
    controls::SegmentedSelector selector;
    controls::IconActionButton settings{controls::ActionIcon::settings};
    bool responsive=false;
};
}
