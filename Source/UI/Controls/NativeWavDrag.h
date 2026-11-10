#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace m3::ui::controls
{
enum class WavDragResult { dropped, cancelled, unavailable };

// Runs the platform's copy-only drag loop on the message thread.
WavDragResult dragReadyWav(const juce::File& file, juce::Component& source,
                          const std::function<bool()>& canStart, const std::function<bool()>& canContinue);
}
