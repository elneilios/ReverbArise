#pragma once
#include <juce_core/juce_core.h>
#include <functional>
namespace m3::ui::controls
{
enum class ExternalDragResult { notReady, released, unsupported, started };
// The binding supplies an independently verified immutable path. This seam owns
// no file and never creates, removes or renames one, regardless of OS outcome.
inline ExternalDragResult invokeExternalFileDrag(const juce::String& path,bool verified,bool leftButtonDown,
    const std::function<bool(const juce::String&)>& invoke)
{
    if(!verified || path.isEmpty())return ExternalDragResult::notReady;
    if(!leftButtonDown)return ExternalDragResult::released;
    return invoke && invoke(path)?ExternalDragResult::started:ExternalDragResult::unsupported;
}
}
