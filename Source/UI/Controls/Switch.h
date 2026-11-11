#pragma once

#include "BooleanToggle.h"

namespace m3::ui::controls
{
class Switch final : public BooleanToggle
{
public:
    explicit Switch(const juce::String& label, ToggleLabelSide side = ToggleLabelSide::afterIndicator)
        : BooleanToggle(label, ToggleForm::track, side) {}
};
}
