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
    captureClock = 1 << 8, captureArm = 1 << 9, captureStop = 1 << 10,
    finish = 1 << 15 // MIDI Stop; unlike the local Stop button, a repeat is idempotent.
};

// Internal annotation on a Rack Clock pulse. Zero means the beat was not tagged.
inline constexpr unsigned rackBeatShift = 12;
inline constexpr uint16_t rackBeatMask = 7 << rackBeatShift;
inline constexpr uint16_t rackDownbeat = 1 << rackBeatShift;

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

enum class MidiProtocol { rack, flStudio };

// JUCE MIDI iteration exposes bytes directly. Creating MidiMessage objects
// here could allocate for SysEx. Keep the budget over all events, including
// unrelated channels and messages, and discard the whole batch on failure.
inline void readLoopObservations(const juce::MidiBuffer& midi, int frames, MidiObservations& out,
                                 bool includeCapture = true, MidiProtocol protocol = MidiProtocol::rack) noexcept
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
        if (high)
        {
            switch (message.data[1])
            {
                case 12: role = phrase; break;
                case 16: role = cue; break;
                case 17: role = finish; break;
                case 18: role = stop; break;
                case 24: role = captureArm; break;
                case 25: role = captureStop; break;
                default: break;
            }
        }
        if (protocol == MidiProtocol::rack) switch (message.data[1])
        {
            case 14: role = high ? gateRise : gateLow; break;
            case 19: if (high) role = bar; break;
            case 21: if (high) role = beat; break;
            case 22: if (high) role = reset; break;
            case 23: if (high) role = captureClock; break;
            default: break;
        }
        if (!includeCapture && (role & (gateRise | gateLow | captureArm | captureStop))) continue;
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

inline void addHostBeat(MidiObservations& observations, int offset) noexcept
{
    if (observations.error != MidiObservations::Error::none) return;
    unsigned index = 0;
    while (index < observations.size && observations.samples[index].sampleOffset < offset)
        ++index;
    if (index < observations.size && observations.samples[index].sampleOffset == offset)
    {
        observations.samples[index].roles |= captureClock;
        return;
    }
    if (observations.size == observations.samples.size())
    {
        observations.size = 0;
        observations.error = MidiObservations::Error::overflow;
        return;
    }
    for (unsigned i = observations.size; i > index; --i)
        observations.samples[i] = observations.samples[i - 1];
    observations.samples[index] = {offset, captureClock};
    ++observations.size;
}
}
