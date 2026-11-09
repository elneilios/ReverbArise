#pragma once

#include <array>
#include <cstdint>
#include <juce_audio_basics/juce_audio_basics.h>

namespace loop_capture
{
enum MidiRole : uint16_t
{
    gateRise = 1 << 0, gateLow = 1 << 1, phrase = 1 << 2, bar = 1 << 3, beat = 1 << 4,
    reset = 1 << 5, cue = 1 << 6, stop = 1 << 7,
    captureClock = 1 << 8, captureArm = 1 << 9, captureStop = 1 << 10
};

struct MidiObservation
{
    int sampleOffset = 0;
    uint16_t roles = 0;
};

struct MidiObservations
{
    enum class Error { none, invalidOffset, overflow, ambiguousGate };
    static constexpr unsigned maximumEvents = 128;
    std::array<MidiObservation, maximumEvents> samples;
    unsigned size = 0;
    Error error = Error::none;
};

// JUCE MIDI iteration exposes bytes directly. Creating MidiMessage objects
// here could allocate for SysEx. Keep the budget over all events, including
// unrelated channels and messages, and discard the whole batch on failure.
inline void readLoopObservations(const juce::MidiBuffer& midi, int frames, MidiObservations& out) noexcept
{
    using Error = MidiObservations::Error;
    out.size = 0;
    out.error = Error::none;
    unsigned seen = 0;
    auto fail = [&](Error error) { out.size = 0; out.error = error; };
    for (const auto message : midi)
    {
        if (++seen > MidiObservations::maximumEvents)
            return fail(Error::overflow);
        const auto offset = message.samplePosition;
        if (offset < 0 || offset >= frames)
            return fail(Error::invalidOffset);
        if (message.numBytes != 3)
            continue;
        const auto status = message.data[0];
        // The Loop note protocol uses channel 1.
        if (status != 0x90 && status != 0x80)
            continue;
        const auto high = status == 0x90 && message.data[2] != 0;
        uint16_t role = 0;
        switch (message.data[1])
        {
            case 14: role = high ? gateRise : gateLow; break;
            case 12: if (high) role = phrase; break;
            case 19: if (high) role = bar; break;
            case 21: if (high) role = beat; break;
            case 22: if (high) role = reset; break;
            case 16: if (high) role = cue; break;
            case 17: if (high) role = stop; break;
            case 23: if (high) role = captureClock; break;
            case 24: if (high) role = captureArm; break;
            case 25: if (high) role = captureStop; break;
            default: break;
        }
        if (role == 0)
            continue;
        // MidiBuffer iterates in ascending sample order. Preserve simultaneous
        // roles; the admission state machine decides which have authority.
        if (out.size == 0 || out.samples[out.size - 1].sampleOffset != offset)
            out.samples[out.size++] = {offset, role};
        else
            out.samples[out.size - 1].roles |= role;
    }
    for (unsigned index = 0; index < out.size; ++index)
    {
        const auto roles = out.samples[index].roles;
        if ((roles & (gateRise | gateLow)) == (gateRise | gateLow) && !(roles & reset))
            return fail(Error::ambiguousGate);
    }
}
}
