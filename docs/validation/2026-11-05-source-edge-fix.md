# Source-edge fix validation — 5 November 2026

Local branch `codex/fix-swell-source-edges`, implementation `1571171`, based on
the accepted export-clock branch at `a5b36d2`. No push or PR. This retains the
fixed-tempo export repair and contains no experimental output protection or
issue #71 tempo-map changes. See [implementation guidance](../development/swell-source-edges.md)
and [original investigation](2026-11-05-swell-source-boundaries.md).

## Implementation and regressions

Only the worker-owned wet-source copy receives linked 1 ms raised-cosine ramps.
Capture, dry calibration reference input, generic exact convolution, output
gain/shape policy, coordinates and host latency are unchanged. Render processing
revision participates in existing asset identity without changing codecs.
Verified legacy and frozen renders retain their sound on restore. Explicit
regeneration upgrades eligible old renders; current ready renders are skipped
by All. Automatic generation compares original source fingerprints even with
versioned wet dependencies.

`preverb_source_edges_test` covers 44.1/48/96 kHz, exact common stereo gains,
unchanged middle, finite short inputs of 1–101 frames, silence, invalid input,
pre/mid-operation cancellation, no helper allocation/deallocation, independent
double direct-convolution reference, complete length and negative-time origin.
Its engine fixture verifies legacy restore without rendering, mixed old/current/
frozen/disabled Regenerate All selection, explicit selected regeneration,
identity/edit preservation, codec roundtrip, failed upgrade retaining complete
assets, exact captured PCM and capture revision preservation. Automatic-generation
coverage verifies no spurious change and honest unknown-revision handling.
Existing convolution, capture, RT allocation, publication and export-clock
regression tests remain unchanged.

## Native Windows gates

Environment: Windows x64, Visual Studio 18 2026 MSVC, JUCE 9.0.4 and WavPack 5.9
from the verified warm dependency sources; CMake build parallelism 4, serial
CTest. No cache clearing or package downloads. Same worktree/build tree reused;
these are incremental runs, not cold-build performance measurements.

- Focused Release: **7/7 passed**, 19.37 s. Source edges, manual preparation,
  SwellEngine, transport playback, generic convolution, source range and
  persistent capture.
- `scripts/build.ps1 -Configuration Release -Test`: **69/69 passed**, 190.35 s
  CTest. Configure/build/test exit 0. Log directory
  `out/build-logs/Release/20261105-130041-913233ed/`.
  Started at `a5b36d2 (dirty)`; that exact source was committed as `1571171`
  during CTest with no source changes during testing.
- `scripts/build.ps1 -Configuration Debug -Test`: **69/69 passed**, 518.60 s
  CTest at implementation `1571171`. Configure/build/test exit 0. Log directory
  `out/build-logs/Debug/20261105-130431-ffa43d13/`.
- `openspec validate condition-swell-source-edges --strict`: passed.
- `git diff --check`: passed.

These are native MSVC/CTest results. No PowerShell mock fixtures were substituted.
Installer scripts and UI code are unchanged. Full native coverage includes
existing UI/persistence tests; this is not FL Studio listening acceptance.

## Supplied four-source comparison

The native helper and production IR preparation/convolver were run on separate
copies of all four original cut windows with `Large Wide Echo Hall.wav`.
Against the independent NumPy double FFT model using the approved taper,
relative RMS error is **1.72e-7 to 1.86e-7**, maximum sample error **3.10e-7**.
All original result lengths are retained (215819/215819/208084/215819 frames).

Using the original fitted placement/envelopes and saved gains for comparison:

- Three crackle indicators (maximum second difference in each local interval)
  decrease **15.90 / 23.38 / 21.83 dB**.
- Wet energy above 8 kHz in the documented 6.8–8.25 s analysis window decreases
  **42.57 dB**. This is band energy, not overall loudness reduction.
- Wet excerpt RMS changes **−0.02136 dB**; pre-dry static matching is only
  **+0.000542 dB**. No time-varying dynamics processing was added.

This comparison holds the previous gains/envelopes to isolate conditioning;
it is not a fresh full DAW export. Native engine integration is independently
covered by the synthetic capture fixture. Actual musical calibration on explicit
regeneration uses original dry references and the new wet identity. Original
DAW/dry exports are not assumed phase-identical; no blind dry subtraction is
used. Local reproduction utility and private audio are ignored under
`out/breakup-analysis/`; metrics are in the adjacent JSON.

All **16** original files under the owner's managed `segments/` and `manifests/`
were SHA-256 checked before and after the native review and remained unchanged.
The later final source check also matches the same 16-file snapshot; its aggregate
hash and first captured chunk hash are recorded in the JSON.
No user project/capture files were written by this change or its tests.

## Test-build delivery and remaining host check

Release bundle is prepared under
`out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
Binary SHA-256: `50C1A55B33C5AEC45F23FDC626330CFC8C8EDE83CCE4AF21E49B5B057480EB7E`.
The release helper verified the x64 PE version and complete six-file bundle;
the prepared inventory is recorded in the adjacent JSON.

Installed on 5 November 2026 after the owner saved and closed FL Studio using
`scripts/install-test-vst3.ps1` via the test-install skill. The initial normal
attempt built successfully but Windows denied access to the system install lock;
it did not replace the bundle. The elevated retry completed with exit 0.

- Installed: `C:/Program Files/Common Files/VST3/ReverbArise.vst3`.
- Previous complete bundle retained:
  `out/vst3-install-backups/20261105-141058-1d36de86920d4dc7ad9c4396e9c0661d/ReverbArise.vst3`.
- Release configure/build passed at `42da906` with unchanged DSP implementation
  `1571171`; logs `out/build-logs/Release/20261105-141052-b15b7f7b/`.
- Full six-file installed/source inventories match; binary SHA-256 remains the
  tested value above. Backup inventory is also retained in the JSON.
- All 16 capture/manifest files match the immediate pre-install hash snapshot.
- CTest was not rerun during installation: the same source had already passed
  69/69 native Debug and 69/69 Release tests. No source code changed afterward.

Reopen FL Studio. In Options > Manage plugins, enable Verify plugins and Rescan
previously verified plugins, run Find installed plugins and confirm the resolved
ReverbArise path is the system bundle above. Installation is not host acceptance.

One final owner listening check remains: reopen the saved project, use
**Regenerate All** once (unfreeze only swells intended to change), then listen
to the first four overlapping swells and confirm DAW export. Existing capture
is sufficient; do not recapture. Until explicit regeneration, existing verified
wet renders deliberately preserve the old sound. Host listening and new-build
FL export acceptance are pending; automated export-clock regressions passed.
