# Preverb Hall

`PreverbHall.wav` is an original, deterministic generated stereo hall-like impulse
response: 48 kHz, 16-bit PCM, 1.8 seconds. It combines independently seeded filtered
noise with exponential decay and sparse early reflections. No recordings or
third-party impulse responses were used. It may be bundled and redistributed with
Preverb under the repository's license; the generated audio is also dedicated to
the public domain under CC0-1.0.

Reproduce with `python scripts/generate_default_ir.py`. The script records its
seed and generation parameters and prints the PCM SHA-256. Runtime embeds the WAV
bytes, verifies decoding, and publishes normal immutable per-instance IR assets.

Expected PCM SHA-256: `52b531d5bd0d77e94770b7796a034798349b05c3d80a71466c070e49cdc91648`.
Expected WAV SHA-256: `b7275a27fc7629a9a1a7191abda4fbb3c8a46372dc9071e36dc1112e4839b458`.
