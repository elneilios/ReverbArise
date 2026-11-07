#pragma once
#include "ChoiceSelector.h"
#include "SliderControl.h"
#include <array>

namespace m3::ui::controls
{
// Generic interaction stays with shared primitives. The binding owns session
// validation and commits the pair atomically; refresh never emits an edit.
class ExportLevelsControl final:public juce::Component
{
public:
    struct Pair {float dry=0,wet=1;bool operator==(const Pair&)const=default;};
    struct Preset {const char* name;Pair levels;};
    inline static constexpr std::array<Preset,5> candidates{{
        {"Wet Layer",{0,1}},{"Subtle",{1,.25f}},{"Balanced",{1,.5f}},
        {"Cinematic",{1,1}},{"Wet Forward",{.65f,1}}}};
    ExportLevelsControl()
    {
        for(auto* child:std::array<juce::Component*,6>{&selector,&dry,&wet,&characterLabel,&dryLabel,&wetLabel})addAndMakeVisible(child);
        characterLabel.setText("Mix Character",juce::dontSendNotification);
        dryLabel.setText("Dry",juce::dontSendNotification);wetLabel.setText("Wet",juce::dontSendNotification);
        for(auto* label:{&characterLabel,&dryLabel,&wetLabel})applyTextRole(*label,TextRole::controlLabel);
        selector.setName("exportMixCharacter");selector.setTitle("Mix Character");
        selector.setTooltip("Musical starting points set only the Dry and Wet levels; unmatched levels display Custom.");
        selector.addItem("Custom",1);for(size_t n=0;n<candidates.size();++n)selector.addItem(candidates[n].name,static_cast<int>(n)+2);
        selector.setItemEnabled(1,false);
        selector.setChoiceReason(1,"Custom reflects the accepted Dry/Wet pair; choose a named preset to change it");
        selector.beforeInvocation=[this]{if(beforeInvocation)beforeInvocation();};
        selector.onChange=[this]{if(refreshing)return;const auto index=selector.getSelectedId()-2;
            if(index>=0 && index<static_cast<int>(candidates.size()))commit(candidates[static_cast<size_t>(index)].levels);};
        for(auto* slider:{&dry,&wet}){
            slider->setRange(0,100,.1);slider->setNumDecimalPlacesToDisplay(1);slider->setTextValueSuffix(" %");
            slider->beforeInvocation=[this]{if(beforeInvocation)beforeInvocation();};
        }
        dry.setName("exportDry");dry.setTitle("Dry output level");wet.setName("exportWet");wet.setTitle("Wet output level");
        dry.setTooltip("Captured dry contribution: 100% is unity gain.");wet.setTooltip("Accepted wet contribution: 100% is unity gain.");
        dry.onValueChange=[this]{if(!refreshing)commit({static_cast<float>(dry.getValue()/100),accepted.wet});};
        wet.onValueChange=[this]{if(!refreshing)commit({accepted.dry,static_cast<float>(wet.getValue()/100)});};
        display({});
    }
    void display(Pair pair)
    {
        const juce::ScopedValueSetter<bool> guard(refreshing,true);accepted=pair;
        if(!dry.hasKeyboardFocus(true) && !dry.isMouseButtonDown())dry.setValue(pair.dry*100,juce::dontSendNotification);
        if(!wet.hasKeyboardFocus(true) && !wet.isMouseButtonDown())wet.setValue(pair.wet*100,juce::dontSendNotification);
        int id=1;for(size_t n=0;n<candidates.size();++n)if(pair==candidates[n].levels){id=static_cast<int>(n)+2;break;}
        selector.setSelectedId(id,juce::dontSendNotification);
    }
    void setResponsivePresentation(){responsive=true;resized();}
    int preferredHeight()const noexcept
    {
        return responsive?3*(responsiveGeometry::captionHeight+spacing::labelHelp+responsiveGeometry::controlHeight)+2*responsiveGeometry::contentPadding
            :3*(controlGeometry::labelHeight+spacing::labelHelp+controlGeometry::height+spacing::related);
    }
    void resized()override
    {
        auto area=getLocalBounds();
        for(auto row:std::array<std::pair<juce::Label*,juce::Component*>,3>{{{&characterLabel,&selector},{&dryLabel,&dry},{&wetLabel,&wet}}}){
            if(responsive)row.first->setFont(componentTextFont(*this,TextRole::controlLabel));
            row.first->setBounds(area.removeFromTop(responsive?responsiveGeometry::captionHeight:controlGeometry::labelHeight));area.removeFromTop(spacing::labelHelp);
            const auto bodyRow=area.removeFromTop(responsive?responsiveGeometry::controlHeight:controlGeometry::height);
            if(row.second==&selector)selector.setRowBounds(bodyRow);else row.second->setBounds(bodyRow);
            area.removeFromTop(responsive?responsiveGeometry::contentPadding:spacing::related);
        }
    }
    std::function<void()> beforeInvocation;
    std::function<void(Pair)> onChange;
    ChoiceSelector selector;SliderControl dry,wet;
private:
    void commit(Pair pair){if(pair==accepted)return;display(pair);if(onChange)onChange(pair);}
    Pair accepted;bool refreshing=false,responsive=false;
    juce::Label characterLabel,dryLabel,wetLabel;
};
}
