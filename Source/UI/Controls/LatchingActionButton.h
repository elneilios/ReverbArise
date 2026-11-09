#pragma once
#include "ActionButton.h"

namespace m3::ui::controls
{
// The owner displays accepted effective state (for example a temporary bypass).
// This family has no preference storage, modifier polling or model mutation.
class LatchingActionButton final:public ActionButton
{
public:
    explicit LatchingActionButton(const juce::String& label,CommandVariant variant=CommandVariant::neutral)
        :ActionButton(label,CommandIcon::none,variant)
    {setClickingTogglesState(true);}
    void display(bool effective){setToggleState(effective,juce::dontSendNotification);}
protected:
    bool effectiveLatchState()const noexcept override{return getToggleState();}
};
}
