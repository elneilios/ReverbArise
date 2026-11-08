import hashlib, json, math, runpy, struct, sys
from pathlib import Path
take = Path(sys.argv[1])
read_wav = runpy.run_path("../host-fx-spike/verify-capture.py")["read_wav"]
rate, pcm = read_wav(take)
frames = list(struct.iter_unpack("<ff", pcm))
assert rate == 48000 and len(frames) == 49153
error = [0.0, 0.0]
for phase, sample in enumerate(frames):
    tone = math.sin(phase * 0.0575958653)
    expected = (0.6, 0.5) if phase < 48 else (0.1 * tone, -0.08 * tone)
    for channel in range(2):
        error[channel] = max(error[channel], abs(sample[channel] - expected[channel]))
assert max(error) < 1e-7, error
print(json.dumps({"wav":take.name, "frames":len(frames), "rate":rate,
                  "max_abs_sample_error":error, "sha256":hashlib.sha256(take.read_bytes()).hexdigest()}, indent=2))
