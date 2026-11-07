"""Disposable Windows experiment. Pass the DLL built from the repository's WavPack pin."""

import argparse
from array import array
from contextlib import contextmanager
import ctypes as ct
import hashlib
import json
from pathlib import Path
import statistics
import subprocess
import sys
import tempfile
import time
import zlib

RATES = (44100, 48000, 96000)
SIGNALS = ("dense", "sparse", "low", "over-unity", "subnormal", "signed-zero")
SECONDS = 5
MAX_ENCODED = 8 * 1024 * 1024
CHANNELS = 2
BYTES_PER_FRAME = CHANNELS * 4
MODE_LOSSLESS = 2
MODE_FLOAT = 8
OPEN_UTF8_WITH_CORRECTION = 0x81
CONFIG_HYBRID = 8
WRITE_BLOCK = ct.CFUNCTYPE(ct.c_int, ct.c_void_p, ct.c_void_p, ct.c_int32)


class WavPackConfig(ct.Structure):
    _fields_ = [
        ("bitrate", ct.c_float), ("shaping_weight", ct.c_float),
        ("bits_per_sample", ct.c_int), ("bytes_per_sample", ct.c_int),
        ("qmode", ct.c_int), ("flags", ct.c_int), ("xmode", ct.c_int),
        ("num_channels", ct.c_int), ("float_norm_exp", ct.c_int),
        ("block_samples", ct.c_int32), ("worker_threads", ct.c_int32),
        ("sample_rate", ct.c_int32), ("channel_mask", ct.c_int32),
        ("md5_checksum", ct.c_ubyte * 16), ("md5_read", ct.c_ubyte),
        ("num_tag_strings", ct.c_int), ("tag_strings", ct.c_void_p),
    ]


def require(condition, message):
    if not condition:
        raise ValueError(message)


class WavPack:
    def __init__(self, dll):
        library = ct.CDLL(str(dll.resolve()))
        ptr, integer, wide = ct.c_void_p, ct.c_int, ct.c_int64
        signatures = {
            "OpenFileOutput": (ptr, WRITE_BLOCK, ptr, ptr),
            "SetConfiguration64": (integer, ptr, ct.POINTER(WavPackConfig), wide, ptr),
            "PackInit": (integer, ptr),
            "PackSamples": (integer, ptr, ptr, ct.c_uint32),
            "FlushSamples": (integer, ptr),
            "CloseFile": (ptr, ptr),
            "OpenFileInput": (ptr, ct.c_char_p, ct.c_char_p, integer, integer),
            "GetMode": (integer, ptr),
            "GetSampleRate": (ct.c_uint32, ptr),
            "GetNumChannels": (integer, ptr),
            "GetBitsPerSample": (integer, ptr),
            "GetFloatNormExp": (integer, ptr),
            "GetNumSamples64": (wide, ptr),
            "UnpackSamples": (ct.c_uint32, ptr, ptr, ct.c_uint32),
            "SeekSample64": (integer, ptr, wide),
            "GetNumErrors": (integer, ptr),
            "LossyBlocks": (integer, ptr),
            "GetLibraryVersionString": (ct.c_char_p,),
        }
        for name, (result, *arguments) in signatures.items():
            function = getattr(library, "Wavpack" + name)
            function.restype = result
            function.argtypes = arguments
            setattr(self, name, function)

    def encode(self, path, raw, rate, hybrid=False):
        frames = len(raw) // BYTES_PER_FRAME
        config = WavPackConfig(bits_per_sample=32, bytes_per_sample=4,
                               num_channels=CHANNELS, float_norm_exp=127,
                               sample_rate=rate, channel_mask=3)
        if hybrid:
            config.flags = CONFIG_HYBRID
            config.bitrate = 2.0
        # The encoder may modify its input words.
        words = ct.create_string_buffer(raw, len(raw))
        with path.open("wb") as output:
            callback_error = []

            @WRITE_BLOCK
            def write(_, data, size):
                try:
                    require(0 <= size <= MAX_ENCODED - output.tell(), "WavPack output exceeds budget")
                    return int(output.write(ct.string_at(data, size)) == size)
                except (OSError, ValueError) as error:
                    callback_error.append(error)
                    return 0

            encoder = self.OpenFileOutput(write, None, None)
            require(encoder, "WavPack encoder open failed")
            try:
                require(self.SetConfiguration64(encoder, ct.byref(config), frames, None)
                        and self.PackInit(encoder) and self.PackSamples(encoder, words, frames)
                        and self.FlushSamples(encoder) and not callback_error, "WavPack encode failed")
            finally:
                self.CloseFile(encoder)

    @contextmanager
    def open(self, path, rate, frames):
        require(0 < path.stat().st_size <= MAX_ENCODED and 0 < frames <= max(RATES) * SECONDS,
                "WavPack input exceeds benchmark budget")
        error = ct.create_string_buffer(128)
        decoder = self.OpenFileInput(str(path).encode("utf-8"), error, OPEN_UTF8_WITH_CORRECTION, 0)
        require(decoder, f"WavPack decoder open failed: {error.value!r}")
        try:
            required_mode = MODE_LOSSLESS | MODE_FLOAT
            require(self.GetMode(decoder) & required_mode == required_mode,
                    "WavPack requires lossless float32 with correction data if hybrid")
            require((self.GetSampleRate(decoder), self.GetNumChannels(decoder),
                     self.GetBitsPerSample(decoder), self.GetFloatNormExp(decoder),
                     self.GetNumSamples64(decoder)) == (rate, CHANNELS, 32, 127, frames),
                    "WavPack format or frame count differs")
            yield decoder
        finally:
            self.CloseFile(decoder)

    def read(self, decoder, frames):
        output = ct.create_string_buffer(frames * BYTES_PER_FRAME)
        require(self.UnpackSamples(decoder, output, frames) == frames
                and self.GetNumErrors(decoder) == 0 and not self.LossyBlocks(decoder),
                "Corrupt, incomplete, or lossy WavPack audio")
        return output.raw


def fixture(signal, frames):
    # Generate IEEE-754 words directly. No conversion can normalize signed zeros
    # or flush subnormals before the codec sees them.
    state = 0x28E31C39
    words = array("I")
    exponents = {"dense": 124, "sparse": 124, "low": 101, "over-unity": 128}
    for index in range(frames * CHANNELS):
        state ^= state << 13 & 0xFFFFFFFF
        state ^= state >> 17
        state ^= state << 5 & 0xFFFFFFFF
        sign = state & 0x80000000
        fraction = state & 0x007FFFFF
        if signal == "signed-zero":
            word = sign
        elif signal == "subnormal":
            word = sign | (fraction or 1)
        elif signal == "sparse" and index // CHANNELS % 48000 >= 1000:
            word = 0
        else:
            word = sign | (exponents[signal] << 23) | fraction
        words.append(word)
    return words.tobytes()


def read_deflate(path, size):
    require(0 < path.stat().st_size <= MAX_ENCODED and 0 < size <= max(RATES) * SECONDS * BYTES_PER_FRAME,
            "DEFLATE input exceeds benchmark budget")
    decoder = zlib.decompressobj()
    raw = decoder.decompress(path.read_bytes(), size + 1)
    require(len(raw) == size and decoder.eof and not decoder.unconsumed_tail and not decoder.unused_data,
            "Incomplete, oversized, or trailing DEFLATE data")
    return raw


def measured(action):
    wall, cpu = time.perf_counter(), time.process_time()
    value = action()
    return value, {"wall_ms": (time.perf_counter() - wall) * 1000,
                   "cpu_ms": (time.process_time() - cpu) * 1000}


def peak_working_set():
    # PROCESS_MEMORY_COUNTERS begins with two DWORDs followed by eight SIZE_Ts.
    class Counters(ct.Structure):
        _fields_ = [("size", ct.c_uint32), ("faults", ct.c_uint32),
                    ("peak", ct.c_size_t), ("remaining", ct.c_size_t * 7)]
    kernel = ct.WinDLL("kernel32", use_last_error=True)
    kernel.GetCurrentProcess.restype = ct.c_void_p
    psapi = ct.WinDLL("psapi", use_last_error=True)
    psapi.GetProcessMemoryInfo.argtypes = [ct.c_void_p, ct.POINTER(Counters), ct.c_uint32]
    counters = Counters(size=ct.sizeof(Counters))
    require(psapi.GetProcessMemoryInfo(kernel.GetCurrentProcess(), ct.byref(counters), counters.size),
            "Could not read peak working set")
    return counters.peak


def benchmark(dll, codec, rate, signal):
    frames = rate * SECONDS
    raw = fixture(signal, frames)
    digest = hashlib.sha256(raw).hexdigest()
    wavpack = WavPack(dll) if codec == "wavpack" else None
    with tempfile.TemporaryDirectory(prefix="ra-codec-") as temporary:
        path = Path(temporary) / ("payload.wv" if wavpack else "payload.deflate")

        def encode():
            if wavpack:
                wavpack.encode(path, raw, rate)
            else:
                path.write_bytes(zlib.compress(raw, 6))

        def decode():
            if wavpack:
                with wavpack.open(path, rate, frames) as decoder:
                    return wavpack.read(decoder, frames)
            return read_deflate(path, len(raw))

        _, encode_time = measured(encode)
        decoded, decode_time = measured(decode)
        require(decoded == raw and hashlib.sha256(decoded).hexdigest() == digest,
                f"Bitwise round-trip failed for {codec}/{rate}/{signal}")
        del decoded

        def open_and_read(offset):
            if wavpack:
                with wavpack.open(path, rate, frames) as decoder:
                    if offset:
                        require(wavpack.SeekSample64(decoder, offset), "WavPack seek failed")
                    return wavpack.read(decoder, 1)
            start = offset * BYTES_PER_FRAME
            return read_deflate(path, len(raw))[start:start + BYTES_PER_FRAME]

        def median_latency(offset):
            expected = raw[offset * BYTES_PER_FRAME:(offset + 1) * BYTES_PER_FRAME]
            times = []
            for _ in range(7):
                sample, timing = measured(lambda: open_and_read(offset))
                require(sample == expected, "Random read changed float bits")
                times.append(timing["wall_ms"])
            return statistics.median(times)

        return {
            "codec": codec, "version": wavpack.GetLibraryVersionString().decode() if wavpack else zlib.ZLIB_RUNTIME_VERSION,
            "rate": rate, "signal": signal, "frames": frames, "float32_sha256": digest,
            "raw_bytes": len(raw), "encoded_bytes": path.stat().st_size,
            "encode": encode_time, "decode": decode_time,
            "open_first_frame_ms": median_latency(0),
            "open_seek_middle_ms": median_latency(frames // 2),
            "peak_working_set_bytes": peak_working_set(),
        }


def faults(dll):
    rate = 96000
    frames = rate * SECONDS
    raw = fixture("dense", frames)
    sha = hashlib.sha256(raw).digest()
    wavpack = WavPack(dll)
    passed = []
    with tempfile.TemporaryDirectory(prefix="ra-codec-fault-") as temporary:
        path = Path(temporary) / "damaged.wv"

        def decode_wavpack():
            with wavpack.open(path, rate, frames) as decoder:
                return wavpack.read(decoder, frames)

        for codec, decode in (("wavpack", decode_wavpack), ("deflate", lambda: read_deflate(path, len(raw)))):
            if codec == "wavpack":
                wavpack.encode(path, raw, rate)
                encoded = path.read_bytes()
            else:
                encoded = zlib.compress(raw, 6)
            changed = bytearray(encoded)
            changed[len(changed) // 2] ^= 1
            for name, data in (("corrupt", changed), ("truncated", encoded[:len(encoded) // 2]),
                               ("input-over-budget", bytes(MAX_ENCODED + 1))):
                path.write_bytes(data)
                try:
                    require(hashlib.sha256(decode()).digest() == sha, "Payload checksum changed")
                except (ValueError, zlib.error):
                    passed.append(f"{codec}:{name}")
                else:
                    raise AssertionError(f"{codec} accepted {name}")
        wavpack.encode(path, raw, rate, hybrid=True)
        try:
            decode_wavpack()
        except ValueError as error:
            require("correction" in str(error), str(error))
            passed.append("wavpack:missing-hybrid-correction")
        else:
            raise AssertionError("Hybrid decoded without correction")
        wavpack.encode(path, raw, rate)
        try:
            with wavpack.open(path, rate, frames // 2):
                raise AssertionError("WavPack exceeded requested output budget")
        except ValueError as error:
            require("frame count" in str(error), str(error))
            passed.append("wavpack:output-over-budget")
        path.write_bytes(zlib.compress(raw, 6))
        try:
            read_deflate(path, len(raw) // 2)
        except ValueError:
            passed.append("deflate:output-over-budget")
        else:
            raise AssertionError("DEFLATE exceeded requested output budget")
    return passed


def run_worker(dll, *arguments):
    result = subprocess.run([sys.executable, __file__, str(dll), *arguments],
                            capture_output=True, text=True, timeout=30)
    require(result.returncode == 0, f"{arguments}: {result.stderr}")
    return json.loads(result.stdout)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("wavpack_dll", type=Path)
    parser.add_argument("--worker", nargs=3, metavar=("CODEC", "RATE", "SIGNAL"))
    parser.add_argument("--fault-worker", action="store_true")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    if args.worker:
        codec, rate, signal = args.worker
        print(json.dumps(benchmark(args.wavpack_dll, codec, int(rate), signal)))
        return
    if args.fault_worker:
        print(json.dumps(faults(args.wavpack_dll)))
        return
    if args.output is None:
        parser.error("--output is required for the benchmark matrix")
    rows = []
    for rate in RATES:
        for signal in SIGNALS:
            for codec in ("wavpack", "deflate"):
                # A fresh process isolates the peak working set and bounds a
                # stalled decoder without entering the plugin or audio callback.
                rows.append(run_worker(args.wavpack_dll, "--worker", codec, str(rate), signal))
    result = {"python": sys.version.split()[0], "rows": rows,
              "faults_rejected": run_worker(args.wavpack_dll, "--fault-worker")}
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"{len(rows)} exact round trips and {len(result['faults_rejected'])} rejected faults")


if __name__ == "__main__":
    main()
