#pragma once
#include "WorkflowPresentation.h"
#include "Controls/SegmentedSelector.h"
#include "Controls/IconActionButton.h"

namespace m3::ui
{
class WorkflowHeader final:public juce::Component
{
public:
    WorkflowHeader()
    {
        setName("workflowHeader");wordmark.setName("brandWordmark");selector.setName("workflowMode");
        selector.setTitle("Workflow mode");settings.setName("diagnostics");
        tagline.setText("Reverse Reverb for Creative Swells",juce::dontSendNotification);
        tagline.setFont(juce::FontOptions(headingSize,juce::Font::italic));tagline.setColour(juce::Label::textColourId,text);
        role.setName("workflowRole");status.setName("workflowStatus");
        for(auto* label:{&role,&status}){label->setFont(juce::FontOptions(captionSize));label->setBorderSize({});label->setColour(juce::Label::textColourId,secondary);}
        addAndMakeVisible(wordmark);addAndMakeVisible(tagline);addAndMakeVisible(selector);
        addAndMakeVisible(settings);addAndMakeVisible(role);addAndMakeVisible(status);
        selector.onRequest=[this](int id){const auto callback=onModeRequested;if(callback)callback(static_cast<workflow::Mode>(id-1));};
        settings.onClick=[this]{const auto callback=onSettings;if(callback)callback();};
        display(presentWorkflow({},{}));
    }
    std::function<void(workflow::Mode)> onModeRequested;
    std::function<void()> onSettings;
    void display(const WorkflowPresentation& presentation,const juce::String& detail={})
    {
        std::vector<controls::SegmentChoice> choices;
        for(const auto& choice:presentation.choices)choices.push_back({static_cast<int>(choice.mode)+1,choice.label,choice.subtitle,choice.available,choice.reason});
        selector.display(std::move(choices),static_cast<int>(presentation.selected)+1);
        const auto subtitle=modeSubtitle(presentation.selected);role.setText(subtitle,juce::dontSendNotification);role.setTooltip(subtitle);
        status.setText(presentation.status,juce::dontSendNotification);status.setTooltip(juce::String(presentation.status)+"\n"+detail);
        setDescription(juce::String(modeLabel(presentation.selected))+": "+subtitle+". "+presentation.status);
    }
    controls::SegmentedSelector& modeSelector()noexcept{return selector;}
    controls::IconActionButton& settingsAction()noexcept{return settings;}
    void resized()override
    {
        auto area=getLocalBounds();auto gear=area.removeFromRight(36);settings.setBounds(gear.withY(2).withHeight(36));
        area.removeFromRight(12);const auto column=area.removeFromRight(getWidth()<1200?330:360);
        selector.setBounds(column.withY(0).withHeight(36));role.setBounds(column.withY(40).withHeight(13));status.setBounds(column.withY(54).withHeight(13));
        area.removeFromRight(16);wordmark.setBounds(area.removeFromLeft(getWidth()<1200?250:290).withY(0).withHeight(46));
        area.removeFromLeft(16);tagline.setBounds(area.withY(0).withHeight(46));
    }
private:
    Wordmark wordmark;juce::Label tagline,role,status;
    controls::SegmentedSelector selector;
    controls::IconActionButton settings{controls::ActionIcon::settings};
};
}
