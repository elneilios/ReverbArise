# Bundled IR library

The library contains the exact original Preverb Hall plus five listening-approved
responses. Preverb Hall remains the default; adding the library does not change
existing swell assignments or saved future defaults. The five new responses are
original stereo 48 kHz/24-bit static WAV assets with immutable revision-1 hashes.

| Response | Duration | Intended reverse-swell use |
| --- | ---: | --- |
| Monumental | 10 s | Forceful dense climax, broad room reflections and long decay |
| Intimate Plate | 1 s | Tight, bright, compact delay structure and narrow image |
| Vintage Dark Hall | 4 s | Low resonant warmth and strong high-frequency damping |
| Shimmer Cloud | 7 s | Emphasized upper resonances and broad static coloration |
| Whisper | 0.5 s | Narrow, short, airy transient anticipation |

Shimmer Cloud is a static linear response. It does not pitch-shift input or
implement time-varying shimmer, modulation or chorus. All five use original
procedural audio, with no recorded or commercial reverb captures. Audio and
synthetic listening fixtures are CC0; see `scripts/IR_AUDIO_LICENSE.md`.

## Catalogue and compatibility

`bundledImpulses()` is the one descriptor source for released responses.
`preverb-hall`, revision 1, remains the default. The other released IDs are
`monumental`, `intimate-plate`, `vintage-dark-hall`, `shimmer-cloud`, and `whisper`.
A menu item number is never a saved sound ID. Older saved labels and managed
content remain available even if they differ from the current catalogue label.

| Descriptor | Meaning / verification |
| --- | --- |
| id/revision | Stable release identity; unknown IDs are rejected before mutation |
| name/description | Musical label and truthful sound description |
| license/generator | Audio redistribution and reproducible source revision |
| rate/channels/bits/frames | Decoder must match all format fields |
| bytes/size | Embedded content, not an external replaceable preset path |
| wavHash | SHA-256 of the complete packaged bytes |
| decodedHash | Existing canonical sampleHash: little-endian rate double, channel int32, frame uint64 and normalized-zero interleaved float32 |

The decoder admits both its temporary decode and result buffers against the
available working budget before allocation. Immutable managed IR content is the
authority on reopen; changing catalogue order/labels or adding content must not
reassign an old project. Legacy default decode/install and boolean assignment
entry points still forward to the exact original Hall descriptor. Explicit
catalogue assignment pins swell ID and input/session generation, respects Freeze,
and does not change future defaults. External WAV/WavPack remains unnormalized.

`preverb_swell_asset_test` covers valid original decode/install, packaged and
decoded checksum failures, mismatched frame metadata, low budget, unknown ID and
unchanged output on failure. `preverb_swell_engine_test` covers pinned explicit
assignment, custom future default, separate neighbours, freeze, invalid/empty
files, WavPack, superseded input and restored-session epoch. Existing UI and
automatic-coordinator fixtures exercise unchanged default behavior.

## Select and replace a response

The shared instance catalogue also retains successfully accepted custom imports,
even after every swell and the future default switch away from them. Entries are
deduplicated by content and reuse immutable managed WAVs; the original WAV/WavPack
file is not required after import. Failed, cancelled and superseded imports do
not become reusable choices. Saving/reopening retains the collection; copying a
session copies its choices, while later imports remain local to that instance.
Creative metadata v9 stores up to 4097 named managed identities within the
existing 8 MiB state bound. Capacity failures retain the previous accepted IR.
Legacy sessions seed known choices from accepted default/swell references;
formerly unused imports cannot be recovered by scanning storage. Keep a
pre-upgrade project copy when rollback to an older binary is required.

Use the existing Selected Swell dropdown for any of the six bundled sounds, or
the adjacent folder button for a WAV/WavPack. This changes only that swell. Its
source, arrival, placement, level, curves and activity preferences stay intact.
Freeze must be turned off before changing an IR. Descriptions identify the sound.
Monumental deliberately has greater intensity than the subtle responses.

An IR-only change on a verified ready swell keeps its accepted sound available while
the requested one prepares. Status names both responses. Hover for full names and
recovery details. The complete replacement joins the current Song position with
the existing transition of at most 5 ms. An active manual Audition crossfades to
the new mix at the current listening position, wrapping into the new window if
needed. Neighbours keep their positions and voices.

On failure, status identifies the requested IR, the cause and what to try next.
The previous sound remains available where its dependencies still verify. Source,
explicit envelope, placement and timing edits keep their normal invalidation
rules. Activity and routing still govern what plays. Old and new pages and
preparation share the 256 MiB budget. Long jobs service live pages between bounded
processing stages. If pages cannot fit, choose a shorter IR or retry while stopped.

A shorter response fits wet bounds and fades to the available interval. The
waveform and inspector show the resulting shape and indicate when timing was
adjusted. The original timing remains available when a longer IR supports it.
An explicit timing edit updates the corresponding intent. In the swell action
menu, Undo IR change and Redo IR change restore the response and its timing as
one edit. History lasts for the current editing session.

Host offline bounce snapshots the accepted IR at entry if a replacement is still
pending, and reports that choice. A background completion cannot switch the WAV
mid-bounce. The next bounce can use the completed replacement. Internal Export
also prepares from an accepted composition and reports an outstanding request.

Save and reopen use the accepted IR, effective timing and remembered timing
intent. A pending or failed request leaves that saved sound in place. Existing
projects use their saved shape as initial intent. Reopen verifies committed
assets without resuming playback.

## Reproduce and review assets

Python 3.10.6 standard library is the recorded generation environment; NumPy and
SciPy are not required. PRNG is explicitly LCG32, with seeds/filter/envelope/mode
parameters, authoring gains and exact output identities in `manifest.json`.
Generation does not overwrite `resources/PreverbHall.wav` or release resources.
`resources/irs/manifest.json` and `Source/DSP/BundledImpulseEntries.inc` record the
approved bytes; packaging rejects different content for an existing revision.

```powershell
python scripts/generate_ir_library.py --output out/ir-library-prototypes-v4
python tests/ir_generator_test.py
python scripts/package_ir_library.py --source out/ir-library-prototypes-v4 --check
& $cmake --build --preset windows-msvc-debug --target preverb_ir_review
& out/build/windows-msvc/Debug/preverb_ir_review.exe "$PWD/out/ir-library-prototypes-v4" "$PWD/out/ir-library-listening-v4"
python scripts/build_ir_review_page.py out/ir-library-listening-v4
```

The review tool uses actual production convolution, shape and audition mixing.
Each one-minute `*-comparison.wav` has four fixed 15-second slots: piano-like
chord, pad, voice-like phrase and pluck. Their original dry events are at
12, 27, 42 and 57 seconds. Long pre-roll is intentional; matching dry timing lets
you compare onset/transition against Hall. Mix is 50% wet, zero trim and initial
20 ms Smooth wet release, using existing disclosed monitoring protection.

Each response's subfolder also contains individual `-dry`, `-wet`, `-mix` and
`-forward` files. The forward reference has its own reported safety-only gain;
it is not a loudness-matched listening comparison. The individual clips begin
at their own file origin; use the comparison clips for identical Song alignment.
`render-report.json` contains absolute starts and raw/shaped/common-250ms-crop
levels, monitoring attenuation and frame counts. No runtime normalization is
added. Active RMS uses 20 ms windows with power within 40 dB of the loudest
window. This objective is not a guarantee of equal loudness on every source.
Open `out/ir-library-listening-v4/index.html` in a browser for source/mode selectors
and one player per response. No server or network connection is required.

You can also load prototype WAVs from `out/ir-library-prototypes` with the current
per-swell file loader and compare on real source material. Use the current
`out/ir-library-prototypes-v4` folder; the unversioned folder is the earlier
noise-based comparison, which did not pass character approval. They remain managed
imports. Released equivalents are now in `resources/irs`, with exactly those
approved WAV hashes.

## Integration acceptance

Native tests cover all 18 response/rate combinations, mono/asymmetric stereo
oracles, live ownership and failure cases, and a paced long-IR 96 kHz overlap and
pending-bounce test. Measurements distinguish conservative workspace estimates,
sampled engine accounting and process peak working set; they are not host CPU
guarantees. See the evidence log for actual configuration/build results.

The user approved final FL Studio integration on 2026-11-02: all six sounds,
live switching with retained audio, failed replacement feedback, long overlaps
and negative-time lead-in, two-instance bundled/custom assignments, shaped/frozen
swells, seeks/loops/final events, pending-choice WAV export and save/reopen.
The evidence log records the unchanged host setup and configuration limitations.

See [implementation evidence](../validation/ir-library-evidence.md) and the OpenSpec task list.

The current review also includes previous-revision players for Monumental and
Vintage Dark Hall. Three accepted sounds are held byte-for-byte unchanged by
explicit generator-test hashes. Monumental deliberately targets greater intensity
than the subtle presets; that creative level difference has listening approval.
