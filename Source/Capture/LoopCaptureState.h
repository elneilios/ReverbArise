#pragma once

#include "CaptureManifest.h"
#include "LoopCapturePreference.h"

inline bool readLoopState(const juce::XmlElement& state, CaptureSourceKind& kind, LoopCapturePreference& preference)
{
    const juce::XmlElement* source = nullptr;
    const juce::XmlElement* next = nullptr;
    for (auto* child : state.getChildIterator())
    {
        if (child->hasTagName("AcceptedLoopSource"))
        {
            if (source != nullptr) return false;
            source = child;
        }
        else if (child->hasTagName("NextCapture"))
        {
            if (next != nullptr) return false;
            next = child;
        }
    }
    if ((source || next) && state.getStringAttribute("stateVersion") != "8")
        return false;
    if (source)
    {
        const auto revision=state.getStringAttribute("manifestRevision");
        const auto number=revision.getLargeIntValue();
        if (source->getStringAttribute("version") != "1" || source->getNumAttributes() != 1
            || source->getNumChildElements() != 0 || state.getStringAttribute("manifestReference").isEmpty()
            || number <= 0 || juce::String(number)!=revision)
            return false;
        kind = CaptureSourceKind::loop;
    }
    if (!next)
        return true;
    const auto enabled = next->getStringAttribute("enabled"), method = next->getStringAttribute("method"),
        unit = next->getStringAttribute("unit"), count = next->getStringAttribute("count");
    if (next->getStringAttribute("version") != "1" || next->getNumAttributes() != 5 || next->getNumChildElements() != 0
        || (enabled != "0" && enabled != "1") || (method != "duration" && method != "endSignal")
        || (unit != "bars" && unit != "beats") || count != juce::String(count.getIntValue()))
        return false;
    preference = { enabled == "1", method == "duration" ? LoopCapturePreference::Method::duration : LoopCapturePreference::Method::endSignal,
                   unit == "bars" ? LoopCapturePreference::Unit::bars : LoopCapturePreference::Unit::beats,
                   static_cast<unsigned>(count.getIntValue()) };
    return preference.valid();
}
