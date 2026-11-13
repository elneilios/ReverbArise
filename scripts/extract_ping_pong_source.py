"""Pack the measured Ping Pong bursts in audible order without changing PCM."""

import argparse
import hashlib
import io
import json
from pathlib import Path
import wave


SOURCE_HASH = "bc5ee590f3d4caff32d639d6b5d1b6320251ad53adee7fd7c7c34694cdede91d"
RATE = 48000
FRAMES = 120000
SAMPLE_BYTES = 3
SILENT = bytes(SAMPLE_BYTES)
STEM = "stereo-ping-pong-crescendo-v1"


def pack(reference):
    source = reference.read_bytes()
    if hashlib.sha256(source).hexdigest() != SOURCE_HASH:
        raise ValueError("Source checksum differs from the approved Ping Pong reference")
    with wave.open(io.BytesIO(source), "rb") as reader:
        actual = (reader.getnchannels(), reader.getsampwidth(), reader.getframerate(),
                  reader.getnframes(), reader.getcomptype())
        if actual != (2, SAMPLE_BYTES, RATE, FRAMES, "NONE"):
            raise ValueError(f"Unexpected source format: {actual}")
        original = reader.readframes(FRAMES)

    stride = SAMPLE_BYTES * 2
    runs = []
    for audible in range(FRAMES):
        offset = (FRAMES - 1 - audible) * stride
        left = original[offset:offset + SAMPLE_BYTES]
        right = original[offset + SAMPLE_BYTES:offset + stride]
        if left == right == SILENT:
            continue
        if left != SILENT and right != SILENT:
            raise ValueError("Overlapping channels in the source")
        channel = 0 if left != SILENT else 1
        if not runs or audible - runs[-1][1] > RATE // 20:
            runs.append([audible, audible, channel])
        if runs[-1][2] != channel:
            raise ValueError("Unexpected channel change inside a burst")
        runs[-1][1] = audible
    if len(runs) != 11 or [run[2] for run in runs] != [i % 2 for i in range(11)]:
        raise ValueError("The reference must contain 11 alternating bursts")

    packed = bytearray()
    restored = bytearray(len(original))
    bursts = []
    for first, last, channel in runs:
        packed_start = len(packed) // SAMPLE_BYTES
        peak_frame, amplitude = first, 0
        for audible in range(first, last + 1):
            offset = (FRAMES - 1 - audible) * stride + channel * SAMPLE_BYTES
            sample = original[offset:offset + SAMPLE_BYTES]
            packed.extend(sample)
            restored[offset:offset + SAMPLE_BYTES] = sample
            magnitude = abs(int.from_bytes(sample, "little", signed=True))
            if magnitude > amplitude:
                peak_frame, amplitude = audible, magnitude
        bursts.append({
            "audible_start_frame": first,
            "packed_start_frame": packed_start,
            "frames": last - first + 1,
            "channel": channel,
            "peak_offset_frames": peak_frame - first,
        })
    if restored != original:
        raise ValueError("The packed bursts cannot reconstruct the exact stereo PCM")

    wav = io.BytesIO()
    with wave.open(wav, "wb") as writer:
        writer.setparams((1, SAMPLE_BYTES, RATE, len(packed) // SAMPLE_BYTES, "NONE", ""))
        writer.writeframes(packed)
    wav_bytes = wav.getvalue()
    if len(packed) % 2:
        # RIFF requires a pad byte after an odd-sized chunk, outside its data length.
        padded = wav_bytes + b"\x00"
        wav_bytes = padded[:4] + (len(padded) - 8).to_bytes(4, "little") + padded[8:]
    manifest = {
        "preset_id": "stereo-ping-pong-crescendo",
        "source_version": 1,
        "reference_name": "12_Stereo_Ping_Pong_Crescendo.wav",
        "reference_pack": "ReverbArise Creative IR Crescendo v2",
        "reference_wav_sha256": SOURCE_HASH,
        "reference_frames": FRAMES,
        "sample_rate": RATE,
        "bits_per_sample": SAMPLE_BYTES * 8,
        "packed_wav_sha256": hashlib.sha256(wav_bytes).hexdigest(),
        "bursts": bursts,
    }
    return {
        f"{STEM}.wav": wav_bytes,
        f"{STEM}.json": (json.dumps(manifest, indent=2) + "\n").encode("utf-8"),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference", type=Path)
    parser.add_argument("--check", action="store_true", help="Compare the checked-in asset without writing")
    args = parser.parse_args()
    directory = Path(__file__).resolve().parents[1] / "resources" / "presets"
    for name, contents in pack(args.reference).items():
        path = directory / name
        if args.check:
            if not path.is_file() or path.read_bytes() != contents:
                parser.error(f"{path} differs from the reproducible asset")
        else:
            directory.mkdir(parents=True, exist_ok=True)
            path.write_bytes(contents)
    print("Verified 11 bursts and exact reconstruction of the reference stereo PCM")


if __name__ == "__main__":
    main()
