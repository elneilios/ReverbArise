#pragma once

#include <juce_core/juce_core.h>
#include <cmath>
#include <charconv>
#include <optional>

namespace m3
{
struct LoopPlacement
{
    static constexpr int maxRepeats = 1024;
    double ppq = 0;
    int repeats = 1;

    bool valid() const noexcept
    {
        return std::isfinite(ppq) && ppq >= 0 && ppq < 0x1p31 && repeats > 0 && repeats <= maxRepeats;
    }
    bool operator==(const LoopPlacement&) const = default;
};

inline bool readLoopPlacement(const juce::XmlElement& state, std::optional<LoopPlacement>& result)
{
    result.reset();
    for (const auto* child : state.getChildIterator())
    {
        if (!child->hasTagName("LoopPlacement")) continue;
        const auto count = child->getStringAttribute("repeats");
        const auto position = child->getStringAttribute("ppq");
        const auto* cursor = position.toRawUTF8();
        const auto end = cursor + position.getNumBytesAsUTF8();
        double ppq = 0;
        const auto parsed = std::from_chars(cursor,end,ppq);
        const LoopPlacement placement{ppq,count.getIntValue()};
        if (result || child->getStringAttribute("version") != "1" || child->getNumAttributes() != 3
            || child->getNumChildElements() != 0 || !state.getChildByName("AcceptedLoopSource")
            || state.getStringAttribute("stateVersion") != "8" || parsed.ec != std::errc{} || parsed.ptr != end
            || juce::String(placement.repeats) != count || !placement.valid())
            return false;
        result = placement;
    }
    return true;
}
}
