#pragma once
#include <juce_core/juce_core.h>
#include <vector>

namespace m3::ui::controls
{
enum class PopupRowRole { choice, action, destructiveAction };

// A snapshot of presentation, not model authority. Completion is still subject
// to the owner's captured binding/lifetime checks.
struct PopupChoice
{
    int id=0;
    juce::String label,reason;
    bool eligible=true,selected=false;
    PopupRowRole role=PopupRowRole::choice;

    bool representsChoice() const noexcept { return role == PopupRowRole::choice; }
    bool hasSelectionMarker() const noexcept { return representsChoice() && selected; }
};

class PopupInteraction final
{
public:
    explicit PopupInteraction(std::vector<PopupChoice> snapshot):choices(std::move(snapshot)){}
    const std::vector<PopupChoice>& items()const noexcept{return choices;}
    bool valid(int index)const noexcept{return index>=0 && static_cast<size_t>(index)<choices.size();}
    bool done()const noexcept{return finished;}
    int inspected()const noexcept{return pointer>=0?pointer:keyboard;}
    int keyboardIndex()const noexcept{return keyboard;}
    int heldIndex()const noexcept{return held;}
    void pointerEnter(int index){pointer=!finished && valid(index)?index:-1;}
    void pointerLeave(int index){if(pointer==index)pointer=-1;}
    void pointerExit(){pointer=-1;held=-1;}
    void pointerDown(int index){keyboard=-1;held=!finished && valid(index)?index:-1;}
    void pointerUp(){held=-1;}
    void keyboardInspect(int index){pointer=-1;held=-1;keyboard=!finished && valid(index)?index:-1;}
    void navigationLost(){keyboard=-1;}
    void cancel(){finished=true;pointer=keyboard=held=-1;}
    // Return an item ID at most once, never permission for an unavailable row.
    int complete(int index)
    {
        if(finished || !valid(index) || !choices[static_cast<size_t>(index)].eligible || choices[static_cast<size_t>(index)].id==0)return 0;
        const auto result=choices[static_cast<size_t>(index)].id;
        cancel();return result;
    }
private:
    std::vector<PopupChoice> choices;
    int pointer=-1,keyboard=-1,held=-1;
    bool finished=false;
};
}
