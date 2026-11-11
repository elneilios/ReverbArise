#pragma once
#include "Controls/ChoiceSelector.h"
#include "Controls/NumericInputControl.h"
#include <Core/ExportState.h>

namespace m3::ui
{
// Loop-domain composition of existing controls. Session/revision ownership stays
// in the workspace binding; accepted-state refresh never emits an edit.
class LoopExportControls final:public juce::Component
{
public:
    using Settings=m3::LoopExportOptions;
    LoopExportControls()
    {
        for(auto* child:std::initializer_list<juce::Component*>{&grid,&repeats,&gridLabel,&repeatsLabel})addAndMakeVisible(child);
        gridLabel.setText("Lead-in grid",juce::dontSendNotification);
        repeatsLabel.setText("Repeats",juce::dontSendNotification);
        for(auto* label:{&gridLabel,&repeatsLabel}){applyTextRole(*label,TextRole::controlLabel);label->setBorderSize({});}
        grid.setName("loopExportGrid");grid.setTitle("Lead-in grid");
        grid.setTooltip("Round the WAV start back to this grid, preserving the full first swell. Loop Bar 1 stays in place.");
        grid.beforeInvocation=[this]{begin();};
        grid.onChange=[this]{if(!refreshing){auto next=accepted;next.gridBeats=grid.getSelectedId();commit(next);}};
        repeats.setName("loopExportRepeats");repeats.setTitle("Loop repeats");
        repeats.setEditable(true,true,false);repeats.setKeyboardType(juce::TextInputTarget::numericKeyboard);
        repeats.setColour(juce::Label::textColourId,text);repeats.setColour(juce::Label::outlineColourId,outline);
        repeats.setTooltip("Number of Loop passes in the WAV. The first lead-in and final tail are included once.");
        repeats.onBegin=[this]{begin();};
        repeats.onEditorShow=[this]{begin();};
        repeats.onTextChange=[this]{if(!refreshing){const auto entry=repeats.getText().trim();auto next=accepted;
            if(entry.isNotEmpty() && entry.length()<=4 && entry.containsOnly("0123456789"))next.repeats=entry.getIntValue();
            if(next.repeats<1 || next.repeats>maximumRepeats){display(accepted);return;}commit(next);}};
        repeats.onStep=[this](int direction){auto next=accepted;next.repeats=std::clamp(next.repeats+direction,1,maximumRepeats);commit(next);};
        setMeter(4,true,true);display({});
    }
    void setMeter(int beatsPerBar,bool beatAvailable,bool meterAvailable)
    {
        if(meter==beatsPerBar && haveBeat==beatAvailable && haveMeter==meterAvailable)return;
        meter=beatsPerBar;haveBeat=beatAvailable;haveMeter=meterAvailable;
        const auto selected=accepted.gridBeats;
        grid.clear(juce::dontSendNotification);
        grid.addItem("1 beat",1);
        grid.setItemEnabled(1,beatAvailable);
        grid.setChoiceReason(1,beatAvailable?juce::String{}:"This capture has no verified beat grid.");
        if(beatsPerBar>1)for(int bars:{1,2,4}){
            const auto id=beatsPerBar*bars;
            grid.addItem(juce::String(bars)+(bars==1?" bar":" bars"),id);
            grid.setItemEnabled(id,beatAvailable && meterAvailable);
            grid.setChoiceReason(id,beatAvailable && meterAvailable?juce::String{}:"This capture has no verified time signature.");
        }
        grid.setSelectedId(selected,juce::dontSendNotification);
        grid.setEnabled(beatAvailable);
        grid.setTooltip(beatAvailable?"Round the WAV start back to this grid, preserving the full first swell. Loop Bar 1 stays in place.":"This capture has no verified beat grid. Musical lead-in alignment is unavailable.");
    }
    void display(Settings value)
    {
        const juce::ScopedValueSetter<bool> guard(refreshing,true);accepted=value;
        grid.setSelectedId(value.gridBeats,juce::dontSendNotification);
        if(!repeats.isBeingEdited())repeats.setText(juce::String(value.repeats),juce::dontSendNotification);
    }
    Settings settings()const noexcept{return accepted;}
    static int preferredHeight() noexcept
    {return responsiveGeometry::captionHeight+spacing::labelHelp+responsiveGeometry::controlHeight+controls::SelectorPresentation::focusMargin;}
    void resized()override
    {
        using namespace responsiveGeometry;
        auto area=getLocalBounds();
        for(auto* label:{&gridLabel,&repeatsLabel})label->setFont(componentTextFont(*this,TextRole::controlLabel));
        auto count=area.removeFromRight(controlGeometry::valueWidth*2);area.removeFromRight(groupGap);
        gridLabel.setBounds(area.removeFromTop(captionHeight));area.removeFromTop(spacing::labelHelp);
        grid.setRowBounds(area.removeFromTop(controlHeight));
        repeatsLabel.setBounds(count.removeFromTop(captionHeight));count.removeFromTop(spacing::labelHelp);
        repeats.setBounds(count.removeFromTop(controlHeight));
    }
    std::function<void()> beforeInvocation;
    std::function<void(Settings)> onChange;
    controls::ChoiceSelector grid;
    controls::NumericInputControl repeats;
    static constexpr int maximumRepeats=Settings::maximumRepeats;
private:
    void begin(){if(beforeInvocation)beforeInvocation();}
    void commit(Settings value){if(value==accepted){display(accepted);return;}display(value);if(onChange)onChange(value);}
    Settings accepted;bool refreshing=false;
    int meter=-1;bool haveBeat=false,haveMeter=false;
    juce::Label gridLabel,repeatsLabel;
};
}
