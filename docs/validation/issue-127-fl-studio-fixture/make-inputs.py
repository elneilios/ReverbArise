"""Generate disposable FL Studio Loop probe inputs using only Python's standard library."""

import argparse
import math
import random
import struct
import wave
from pathlib import Path


RATE = 48_000
FRAMES_PER_BEAT = RATE // 2
BEATS_PER_BAR = 4
BARS = 29
TICKS_PER_BEAT = 480
COMMANDS = ((18, 4, 62), (21, 1, 60), (24, 4, 62), (28, 1, 60))
PHRASE_MARKERS = tuple((bar, 1, 12) for bar in range(18, 29, 2))
PHRASE_TEST = (
    *PHRASE_MARKERS,
    (18, 1, 24),
    (19, 3, 25),
    (22, 3, 62),
    (25, 1, 60),
    (26, 3, 62),
    (29, 1, 60),
)
SHARED_PLAYBACK_TEST = (
    (20, 2, 12),  # Set beat one on the second beat of each Playlist bar.
    (20, 2, 16),  # Cue, then wait for a future shifted bar with room for the lead-in.
    (22, 3, 17),  # Finish the current pass.
    (24, 2, 16),
    (26, 1, 18),  # Stop Now.
)


def write_dry_audio(path):
    noise = random.Random(127)
    burst = []
    for index in range(RATE // 10):
        time = index / RATE
        body = 0.2 * math.sin(2 * math.pi * 173 * time)
        burst.append((0.65 * noise.uniform(-1, 1) + body) * math.exp(-time / 0.025))

    frames = bytearray(BARS * BEATS_PER_BAR * FRAMES_PER_BEAT * 2)
    for beat in range(0, BARS * BEATS_PER_BAR, 2):
        gain = 0.68 if beat % 8 == BEATS_PER_BAR else 0.44
        for index, sample in enumerate(burst):
            offset = 2 * (beat * FRAMES_PER_BEAT + index)
            struct.pack_into("<h", frames, offset, round(32_767 * gain * sample))

    with wave.open(str(path), "wb") as stream:
        stream.setparams((1, 2, RATE, 0, "NONE", "not compressed"))
        stream.writeframes(frames)


def midi_delta(ticks):
    parts = [ticks & 127]
    while ticks > 127:
        ticks >>= 7
        parts.append((ticks & 127) | 128)
    return bytes(reversed(parts))


def write_command_notes(path, notes):
    # MIDI tempo (120 BPM) and 4/4 meta-events.
    events = [(0, bytes.fromhex("ff 51 03 07 a1 20")), (0, bytes.fromhex("ff 58 04 04 02 18 08"))]
    for bar, beat, note in notes:
        tick = ((bar - 1) * BEATS_PER_BAR + beat - 1) * TICKS_PER_BEAT
        events.append((tick, bytes([0x90, note, 100])))
        events.append((tick + TICKS_PER_BEAT // 8, bytes([0x80, note, 0])))
    events.append((BARS * BEATS_PER_BAR * TICKS_PER_BEAT, bytes.fromhex("ff 2f 00")))

    track = bytearray()
    previous = 0
    for tick, event in sorted(events):
        track.extend(midi_delta(tick - previous))
        track.extend(event)
        previous = tick
    header = b"MThd" + struct.pack(">IHHH", 6, 0, 1, TICKS_PER_BEAT)
    path.write_bytes(header + b"MTrk" + struct.pack(">I", len(track)) + track)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    folder = parser.parse_args().output
    folder.mkdir(parents=True, exist_ok=True)
    write_dry_audio(folder / "Loop probe dry 120 BPM.wav")
    write_command_notes(folder / "Loop probe Cue and Stop.mid", COMMANDS)
    write_command_notes(folder / "Loop probe phrase and capture roles.mid", PHRASE_TEST)
    write_command_notes(folder / "Loop probe phrase only.mid", PHRASE_MARKERS)
    write_command_notes(folder / "Loop shared MIDI playback.mid", SHARED_PLAYBACK_TEST)


if __name__ == "__main__":
    main()
