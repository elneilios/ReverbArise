#pragma once

#include <juce_core/juce_core.h>
#include <cmath>
#include <charconv>
#include <optional>

namespace m3
{
struct LoopPlacement
{
    double ppq = 0;

    bool valid() const noexcept
    {
        return std::isfinite(ppq) && ppq >= 0 && ppq < 0x1p31;
    }
    bool operator==(const LoopPlacement&) const = default;
};

inline bool readLoopPlacement(const juce::XmlElement& state, std::optional<LoopPlacement>& result)
{
    result.reset();
    for (const auto* child : state.getChildIterator())
    {
        if (!child->hasTagName("LoopPlacement")) continue;
        const auto version = child->getStringAttribute("version");
        const auto legacy = version == "1";
        const auto count = child->getStringAttribute("repeats");
        const auto position = child->getStringAttribute("ppq");
        const auto* cursor = position.toRawUTF8();
        const auto end = cursor + position.getNumBytesAsUTF8();
        double ppq = 0;
        const auto parsed = std::from_chars(cursor,end,ppq);
        const LoopPlacement placement{ppq};
        if (result || (!legacy && version != "2") || child->getNumAttributes() != (legacy ? 3 : 2)
            || child->getNumChildElements() != 0 || !state.getChildByName("AcceptedLoopSource")
            || state.getStringAttribute("stateVersion") != "8" || parsed.ec != std::errc{} || parsed.ptr != end
            || (legacy && (juce::String(count.getIntValue()) != count || count.getIntValue() < 1 || count.getIntValue() > 1024))
            || !placement.valid())
            return false;
        result = placement;
    }
    return true;
}
}
