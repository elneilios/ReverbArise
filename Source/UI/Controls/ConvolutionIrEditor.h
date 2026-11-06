#pragma once
#include "IrSelector.h"
#include "IconActionButton.h"
#include "EllipsisLabel.h"
#include <optional>
#include <utility>
#include <vector>

namespace m3::ui::controls
{
// Presentation and deliberate edit lifecycle only. The binding owns identities,
// workflow validation, file dialogs and all model/asset commands.
class ConvolutionIrEditor final:public juce::Component
{
    // Field feedback wraps at its normal helper font, without shrinking text.
    class FieldMessage final:public juce::Label
    {
    public:void paint(juce::Graphics& g)override
        {
            g.setFont(getLookAndFeel().getLabelFont(*this));g.setColour(findColour(juce::Label::textColourId));
            g.drawFittedText(getText(),getLocalBounds(),juce::Justification::topLeft,2,1.f);
        }
    };
public:
    struct Choice
    {
        juce::String identity,name,description;
        bool operator==(const Choice&)const=default;
    };
    struct View
    {
        std::vector<Choice> choices;
        juce::String accepted,label="IR",status,reason;
        bool editable=false,error=false;
    };
    ConvolutionIrEditor()
    {
        setName("convolutionIrEditor");
        for(auto* child:std::array<juce::Component*,4>{&label,&selector,&fileButton,&status})addAndMakeVisible(child);
        selector.setName("irChoice");fileButton.setName("irImport");status.setName("irStatus");
        applyTextRole(label,TextRole::controlLabel);
        applyTextRole(status,TextRole::helper);
        for(auto* item:std::array<juce::Label*,2>{&label,&status})item->setBorderSize({});
        selector.beforeInvocation=[this]{invocationChoices=view.choices;if(beforeInvocation)beforeInvocation();};
        selector.onChange=[this]
        {
            if(refreshing || !selector.isEnabled())return;
            const auto entries=std::exchange(invocationChoices,std::nullopt).value_or(view.choices);
            const auto index=selector.getSelectedId()-1;
            if(index<0 || static_cast<size_t>(index)>=entries.size())return;
            const auto identity=entries[static_cast<size_t>(index)].identity;
            // Keep displaying accepted state until the binding publishes it.
            restoreSelection();
            if(onChoice)onChoice(identity);
        };
        fileButton.onClick=[this]{if(fileButton.isEnabled() && onImport)onImport();};
    }
    void display(View next)
    {
        const juce::ScopedValueSetter<bool> guard(refreshing,true);
        if(next.choices!=view.choices)
        {
            selector.hidePopup();selector.clear(juce::dontSendNotification);
            int id=0;for(const auto& entry:next.choices)selector.addItem(entry.name,++id);
        }
        view=std::move(next);label.setText(view.label,juce::dontSendNotification);
        restoreSelection();
        selector.setEnabled(view.editable);fileButton.setEnabled(view.editable);
        status.setText(view.status,juce::dontSendNotification);
        status.setColour(juce::Label::textColourId,view.error?danger:textStyle(TextRole::helper).ink);
        selector.setTitle(view.label);selector.setDescription(view.reason.isNotEmpty()?view.reason:"Choose an impulse response");
        juce::String help="Choose an impulse response";
        for(const auto& entry:view.choices)if(entry.identity==view.accepted){help+="\n"+entry.description;break;}
        if(view.reason.isNotEmpty())help+="\n"+view.reason;
        selector.setTooltip(help);fileButton.setTooltip("Import WAV or WavPack IR\n"+view.reason);
        fileButton.setDescription(view.reason.isNotEmpty()?view.reason:"Import an impulse response");fileButton.setHelpText(fileButton.getTooltip());
        status.setTooltip(view.status);status.setDescription(view.status);
        if(view.error && view.reason.isNotEmpty()){status.setTooltip(view.status+"\n"+view.reason);status.setDescription(status.getTooltip());}
        resized();
    }
    int preferredHeight(){return preferredHeight(getWidth());}
    int preferredHeight(int width)
    {
        refreshFonts();
        return responsiveGeometry::captionHeight+spacing::labelHelp+responsiveGeometry::controlHeight
            +(view.status.isEmpty()?0:spacing::tight+feedbackLines(width)*controlGeometry::feedbackHeight);
    }
    void resized()override
    {
        refreshFonts();
        auto area=getLocalBounds();
        label.setBounds(area.removeFromTop(responsiveGeometry::captionHeight));area.removeFromTop(spacing::labelHelp);
        auto row=area.removeFromTop(responsiveGeometry::controlHeight);
        fileButton.setBounds(row.removeFromRight(responsiveGeometry::controlHeight));row.removeFromRight(spacing::tight);selector.setBounds(row);
        // Label, input/action row and supporting text form one visual field.
        // Empty feedback occupies no space; hosts reflow adjacent fields using
        // preferredHeight(width) when a message appears or wraps.
        area.removeFromTop(spacing::tight);
        status.setVisible(view.status.isNotEmpty());
        const auto lines=feedbackLines(area.getWidth());
        status.setBounds(view.status.isNotEmpty()?area.removeFromTop(lines*controlGeometry::feedbackHeight):juce::Rectangle<int>{});
    }
    void lookAndFeelChanged()override{refreshFonts();}
    std::function<void()> beforeInvocation,onImport;
    std::function<void(const juce::String&)> onChoice;
    IrSelector selector;
    IconActionButton fileButton{ActionIcon::openFile};
    EllipsisLabel label;FieldMessage status;
private:
    int feedbackLines(int width)
    {
        const auto messageWidth=juce::GlyphArrangement::getStringWidth(getLookAndFeel().getLabelFont(status),view.status);
        return view.status.containsChar('\n') || messageWidth>width?2:1;
    }
    void refreshFonts()
    {
        // Label LookAndFeel resolves responsive typography exactly once.
        label.setFont(textFont(TextRole::controlLabel));
        status.setFont(textFont(TextRole::helper));
    }
    void restoreSelection()
    {
        int id=0;for(size_t i=0;i<view.choices.size();++i)if(view.choices[i].identity==view.accepted){id=static_cast<int>(i+1);break;}
        selector.setSelectedId(id,juce::dontSendNotification);
        selector.setTextWhenNothingSelected("IR unavailable");
    }
    View view;bool refreshing=false;
    std::optional<std::vector<Choice>> invocationChoices;
};
}
