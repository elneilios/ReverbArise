"""Reproduce Preverb's original stereo hall IR. No third-party audio inputs."""
from pathlib import Path
import hashlib
import math
import struct
import wave

RATE, FRAMES, SEED = 48000, 86400, 0x50525642

def generate():
    state = SEED
    channels = [0.0, 0.0]
    data = bytearray()
    for frame in range(FRAMES):
        t = frame / RATE
        for channel in range(2):
            state = (1664525 * state + 1013904223) & 0xffffffff
            noise = state / 2147483648.0 - 1.0
            channels[channel] = 0.7 * channels[channel] + 0.3 * noise
            attack = min(1.0, frame / 180.0)
            value = 0.035 * channels[channel] * attack * math.exp(-5.8 * t)
            for delay, gain in ((0.013, .18), (.027, .12), (.043, .08), (.071, .05)):
                if frame == round((delay + channel * .0017) * RATE):
                    value += gain
            data.extend(struct.pack('<h', round(max(-1, min(1, value)) * 32767)))
    return data

if __name__ == '__main__':
    destination = Path(__file__).resolve().parents[1] / 'resources' / 'PreverbHall.wav'
    destination.parent.mkdir(parents=True, exist_ok=True)
    pcm = generate()
    with wave.open(str(destination), 'wb') as output:
        output.setparams((2, 2, RATE, FRAMES, 'NONE', 'not compressed'))
        output.writeframes(pcm)
    print('PCM SHA-256:', hashlib.sha256(pcm).hexdigest())
