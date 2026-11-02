"""Original CC0 static IR prototypes; Python stdlib only, no external audio inputs.

The default output is ignored prototype data, not released plugin resources.
Reproduce with: python scripts/generate_ir_library.py --output out/ir-library-prototypes
"""
import argparse
from array import array
import hashlib
import json
import math
from pathlib import Path
import platform
import struct
import sys
import wave

RATE = 48000
VERSION = "ir-library-prototype-v4"
PRESETS = [
    dict(id="monumental", name="Monumental", duration=10., decay=8.8, attack=.04,
         cutoff=6500., highpass=45., seed=0x4D4F4E31, modes=0., gain_db=13.,
         network_mix=.65, width=1.7, comb_ms=[61.3,71.9,89.3,101.9,127.7,149.3,173.9,197.3],
         allpass_ms=[3.1,7.3,13.7,17.3,23.9,31.1,43.7,59.3], damping_hz=6500., network_decay=8.8,
         reflections_ms=[31.,57.,93.,151.,229.,347.,481.,687.], reflection_gain=.035,
         front_presence=2.5, front_presence_seconds=.7),
    dict(id="intimate-plate", name="Intimate Plate", duration=1., decay=.85, attack=.003,
         cutoff=11000., highpass=140., seed=0x504C5431, modes=0., gain_db=3.2,
         network_mix=.8, width=.2, comb_ms=[7.1,9.7,13.3,17.9,23.3,29.9],
         allpass_ms=[.7,1.1,1.9], damping_hz=12500., network_decay=.85,
         reflections_ms=[], reflection_gain=0.),
    dict(id="vintage-dark-hall", name="Vintage Dark Hall", duration=4., decay=3.4, attack=.025,
         cutoff=850., highpass=65., seed=0x44524B31, modes=.35, gain_db=-3.2,
         network_mix=.8, width=.85, comb_ms=[43.7,67.1,83.9,107.3,139.9,167.3],
         allpass_ms=[5.3,11.9,19.9,31.1], damping_hz=700., network_decay=3.4,
         reflections_ms=[37.,79.,131.,211.], reflection_gain=.025,
         modal_frequencies=[83.,127.,173.,227.,317.,433.,617.], final_lowpass_hz=850.),
    dict(id="shimmer-cloud", name="Shimmer Cloud", duration=7., decay=6.3, attack=.09,
         cutoff=7800., highpass=200., seed=0x53484D31, modes=.8, gain_db=7.2,
         network_mix=0., width=1.),
    dict(id="whisper", name="Whisper", duration=.5, decay=.38, attack=.006,
         cutoff=6500., highpass=500., seed=0x57485331, modes=0., gain_db=3.7,
         network_mix=0., width=.15),
]
MODES = (587., 887., 1327., 1777., 2371., 3169., 4231., 5647.)


def spatial_network(p, frames, channel):
    """A static impulse through damped parallel delay loops and allpass diffusion.

    All coefficients/delays are fixed. This authors an IR, not a new runtime effect.
    Separated lateral reflections and modal room buildup replace generic whoosh.
    """
    output = [0.] * frames
    alpha = 1 - math.exp(-2 * math.pi * p["damping_hz"] / RATE)
    for bank, ms in enumerate(p["comb_ms"]):
        lag = round(ms * (1 + channel * (.023 + bank * .0009)) * RATE / 1000.)
        feedback = .001 ** (lag / (RATE * p["network_decay"]))
        delay = array("d", [0.]) * lag
        pos = 0
        damp = 0.
        for i in range(frames):
            value = delay[pos]
            damp += alpha * (value - damp)
            delay[pos] = (1. if i == 0 else 0.) + feedback * damp
            output[i] += value / len(p["comb_ms"])
            pos += 1
            if pos == lag:
                pos = 0
    for index, ms in enumerate(p["allpass_ms"]):
        lag = round(ms * (1 + channel * .071) * RATE / 1000.)
        delay = array("d", [0.]) * lag
        pos = 0
        for i, x in enumerate(output):
            y = delay[pos] - .67 * x
            delay[pos] = x + .67 * y
            output[i] = y
            pos += 1
            if pos == lag:
                pos = 0
    for index, ms in enumerate(p["reflections_ms"]):
        at = round((ms + channel * (4.1 + index * 1.7)) * RATE / 1000.)
        # Short band-limited lateral reflection rather than an added dry impulse.
        for n in range(96):
            if at + n < frames:
                output[at + n] += p["reflection_gain"] * math.sin(math.pi * n / 95) ** 2 * math.exp(-n / 20.) / (index + 1)
    return output


def response(p):
    """Distinct fixed spatial networks, noise diffusion and static resonant modes.

    Coefficients and envelopes are offline authorship, not runtime modulation.
    Fixed 0.35 energy per channel is a starting authoring calibration only.
    """
    frames = round(p["duration"] * RATE)
    result = []
    for channel in range(2):
        state = (p["seed"] + channel * 0x9E3779B9) & 0xFFFFFFFF
        lo = dc = 0.
        a = 1. - math.exp(-2. * math.pi * p["cutoff"] / RATE)
        b = 1. - math.exp(-2. * math.pi * p["highpass"] / RATE)
        samples = []
        for i in range(frames):
            state = (1664525 * state + 1013904223) & 0xFFFFFFFF
            white = state / 2147483648. - 1.
            lo += a * (white - lo)
            dc += b * (lo - dc)
            t = i / RATE
            onset = math.sin(min(1., t / p["attack"]) * math.pi / 2.) ** 2
            end = math.sin(min(1., (frames - 1 - i) / (RATE * .025)) * math.pi / 2.) ** 2
            frequencies = p.get("modal_frequencies", MODES)
            modal = sum(math.sin(2 * math.pi * f * (1 + channel * .0013) * t + channel * .7)
                        for f in frequencies) / len(frequencies) if p["modes"] else 0.
            texture = (lo - dc) * (1 - p["modes"]) + modal * p["modes"]
            samples.append(texture * onset * end * math.exp(-math.log(1000.) * t / p["decay"]))
        if p["network_mix"]:
            network = spatial_network(p, frames, channel)
            # Match component energy during authorship before the structural blend.
            ratio = math.sqrt(sum(x*x for x in samples) / sum(x*x for x in network))
            samples = [(1-p["network_mix"])*x + p["network_mix"]*ratio*y for x,y in zip(samples,network)]
        if p.get("final_lowpass_hz"):
            a = 1-math.exp(-2*math.pi*p["final_lowpass_hz"]/RATE)
            # Two fixed poles make the entire dark response, including first reflections, warm.
            for pole in range(2):
                state = 0.
                for i, x in enumerate(samples):
                    state += a*(x-state)
                    samples[i] = state
        if p.get("front_presence"):
            # An authored energy build near IR onset becomes the reverse swell's climax.
            samples = [x*(1+p["front_presence"]*math.exp(-i/(RATE*p["front_presence_seconds"]))) for i,x in enumerate(samples)]
        # The authoring window also covers delayed reflection copies.
        for i in range(frames - round(.025 * RATE), frames):
            samples[i] *= math.sin((frames - 1 - i) / (.025 * RATE) * math.pi / 2) ** 2
        result.append(samples)
    # Width is baked into a static stereo asset, never a new plugin control.
    mid = [(l+r)/2 for l,r in zip(*result)]
    side = [(l-r)/2*p["width"] for l,r in zip(*result)]
    result = [[m+s for m,s in zip(mid,side)], [m-s for m,s in zip(mid,side)]]
    mean_energy = sum(sum(v*v for v in channel) for channel in result)/2
    scale = math.sqrt(.35/mean_energy) * 10 ** (p["gain_db"]/20)
    result = [[v*scale for v in channel] for channel in result]
    return result


def quantize(channels):
    data = bytearray()
    decoded = [[], []]
    for l, r in zip(*channels):
        for c, v in enumerate((l, r)):
            assert math.isfinite(v) and abs(v) < 1., "Revise authored level; do not clip"
            n = round(v * 8388608.)
            assert -8388608 <= n < 8388608
            data.extend((n & 0xFFFFFF).to_bytes(3, "little"))
            decoded[c].append(n / 8388608.)
    return data, decoded


def metrics(channels):
    count = len(channels[0])
    report = []
    for x in channels:
        energy = sum(v * v for v in x)
        remaining = energy
        decay = {}
        for i, v in enumerate(x):
            remaining -= v * v
            for db in (10, 20, 40, 60):
                if db not in decay and remaining <= energy * 10 ** (-db / 10):
                    decay[db] = i / RATE
        # Documented first-order low/high residual energy split, not ideal bands.
        spectral = {}
        for freq in (250., 1000., 4000., 10000.):
            a = 1 - math.exp(-2 * math.pi * freq / RATE)
            low = total = 0.
            for v in x:
                low += a * (v - low)
                total += low * low
            spectral[str(int(freq))] = total / energy
        report.append(dict(peak=max(map(abs, x)), rms=math.sqrt(energy / count),
                           energy=energy, dc=sum(x) / count,
                           remaining_energy_crossing_seconds=decay,
                           first_order_lowpass_energy_fraction=spectral,
                           endpoints=[x[0], x[-1]]))
    cross = sum(a * b for a, b in zip(*channels))
    mono_energy = sum(((a + b) / 2) ** 2 for a, b in zip(*channels))
    side_energy = sum(((a - b) / 2) ** 2 for a, b in zip(*channels))
    return dict(channels=report, correlation=cross / math.sqrt(report[0]["energy"] * report[1]["energy"]),
                mono_fold_energy=mono_energy,
                side_to_mid_db=10 * math.log10(side_energy / mono_energy),
                mono_vs_mean_channel_db=10 * math.log10(mono_energy / ((report[0]["energy"] + report[1]["energy"]) / 2)))


def generate(output):
    output.mkdir(parents=True, exist_ok=True)
    entries = []
    for params in PRESETS:
        pcm, channels = quantize(response(params))
        dest = output / (params["id"] + ".wav")
        with wave.open(str(dest), "wb") as f:
            f.setparams((2, 3, RATE, len(channels[0]), "NONE", "not compressed"))
            f.writeframes(pcm)
        decoded = b"".join(struct.pack("<ff", a, b) for a, b in zip(*channels))
        canonical = struct.pack("<diQ", float(RATE), 2, len(channels[0])) + decoded
        entry = dict(parameters=params, revision=1, prototype=True, file=dest.name,
                     rate=RATE, channels=2, bits=24, frames=len(channels[0]),
                     seconds=len(channels[0]) / RATE,
                     wav_sha256=hashlib.sha256(dest.read_bytes()).hexdigest(),
                     pcm_sha256=hashlib.sha256(pcm).hexdigest(),
                     canonical_sample_sha256=hashlib.sha256(canonical).hexdigest(),
                     measurements=metrics(channels))
        entries.append(entry)
        print(params["name"], entry["wav_sha256"], flush=True)
    manifest = dict(generator=VERSION, generator_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                    author="ReverbArise contributors", python=sys.version, platform=platform.platform(),
                    license="CC0-1.0", source="Original procedural audio; no recordings or third-party effect captures",
                    prng="LCG32: state = (1664525 * state + 1013904223) modulo 2^32",
                    authoring_energy_target_per_channel=.35, static_modes_hz=MODES,
                    construction=dict(filter="fixed one-pole lowpass minus fixed one-pole lowpass residual",
                                      onset="sin(min(1,t/attack)*pi/2)^2", decay="exp(-log(1000)*t/decay)",
                                      end_window_seconds=.025, final_window_seconds=.025,
                                      seed_channel_increment=0x9E3779B9, modal_channel_frequency_ratio=1.0013,
                                      modal_channel_phase=.7,
                                      comb_channel_ratio="1 + channel * (0.023 + bank * 0.0009)",
                                      allpass_coefficient=.67, allpass_channel_ratio=1.071,
                                      reflections="96-sample sin^2 pulse * exp(-n/20) / (reflection index+1)",
                                      reflection_channel_ms="4.1 + index*1.7",
                                      spatial_blend="energy-matched components, then static mid/side width, then mean channel authoring gain",
                                      rounding="Python round, signed little-endian PCM24 scaled by 2^23; reject clipping"),
                    prototypes_pending_listening=True, entries=entries)
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path("out/ir-library-prototypes"))
    generate(parser.parse_args().output)
