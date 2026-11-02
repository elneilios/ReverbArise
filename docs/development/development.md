# Development Setup

## Requirements

- Windows x64
- Visual Studio 2026 Community with the **Desktop development with C++** workload, MSVC x64/x86 build tools, and a Windows SDK
- Visual Studio's **C++ CMake tools for Windows** component
- Git

The project pins JUCE 9.0.4 through CMake FetchContent. CMake downloads the source at configure time. The JUCE CMake API requires CMake 3.22 or newer; the Visual Studio 2026 installation supplies the tested CMake executable.

## License Scope

JUCE is used for this feasibility spike as an internal evaluation dependency only. Do not redistribute JUCE source or a JUCE-based binary until the distribution model and applicable JUCE license terms have been reviewed. Any later closed-source or commercial release requires the appropriate JUCE licensing path. This note is a project constraint, not legal advice.

## Configure, Build, and Test

Run these commands from the repository root in PowerShell:

```powershell
$cmake = "$env:ProgramFiles\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$ctest = Join-Path (Split-Path $cmake) "ctest.exe"

& $cmake --preset windows-msvc
& $cmake --build --preset windows-msvc-debug
& $ctest --preset windows-msvc-debug
```

The first configure fetches the pinned JUCE release and libwavpack 5.9.0 (commit
`5803634a030e2a11dba602ba057b89cc34486c67`). WavPack is linked statically for
background mono/stereo lossless PCM/float IR decoding, including WavPack content
named `.wav`. Its BSD notice is copied into the VST3 bundle's `Contents/Resources`.
Build output is placed under `out/build`, which is ignored by Git.

## Capture Cache (M1)

### M3 editable swell sessions

See [M3 editing](m3-editing.md), [regeneration](m3-regeneration.md),
[persistence](m3-persistence.md), and the separate [FL Studio acceptance guide](../validation/fl-studio-m3-validation.md).
M3 writes plugin state v3 with managed IR/render references while preserving capture
schema v1 and legacy plugin state v1/v2 restoration. Manual selected-swell audition
remains independent of host transport; Song-synchronized playback is M4.

Set `PREVERB_UI_M3_OUTPUT` to an output directory when running `preverb_swell_ui_test`
to render software PNGs for ready, shaped, stale, error, empty and many-swell states.

See [Automatic swell workspace](automatic-swell-workspace.md) for successor v4
state/defaults, automatic capture/detection/rendering, monitoring gain/blend and
the lean editor. [Host acceptance](../validation/fl-studio-automatic-workspace-validation.md)
keeps historical FL Studio checks separate from automated verification. Current
UI access and remaining acceptance are in [the UI checklist](../design/ui-diagnostics-and-acceptance.md);
Song transport playback is implemented. The older milestone notes below describe their historical versions.

### M2 source selection and reading

See [M2 rendering internals](m2-rendering.md) for IR channel/resampling conventions,
convolution alignment, numerical tolerance, and the preview memory budget.

M2 source controls select `[start bar, end bar)` in absolute Playlist coordinates,
independently of the tone controls. One host sample/PPQ/tempo/meter snapshot resolves
both boundaries with nearest-sample rounding. This supports fixed tempo and meter;
missing timing, incompatible sample rates, invalid bounds, and overflow are rejected.
The interval reader consumes an immutable committed manifest, decodes each intersecting
chunk once, retains covered silence, and reports uncaptured or invalid intervals.
It never substitutes zeros for missing source audio or repeatedly reads WAVs per sample.

Capture assets are stored under `%LOCALAPPDATA%\K7MA\Preverb\Projects\<project-id>\<instance-id>`. Plugin state version 2 stores stable UUIDs, a revision, and an immutable `manifests/<uuid>.json` reference, not captured PCM. Each manifest records absolute sample positions, coverage kind, sample rate, frame count, revision, relative asset path, and SHA-256 checksum. Audio chunks are immutable stereo 32-bit-float WAV files in `segments/`. Captured silence is represented as coverage metadata without an audio file; uncaptured gaps have no entry.

The worker assembles at most five seconds per chunk (also capped at 1,920,000 frames), splitting at timeline discontinuities and audio/silence boundaries. Chunk files are written and validated before a new manifest is atomically published. The previous committed map stays authoritative until publication succeeds. A failed capture or Clear leaves the previous manifest, coverage, and referenced chunks intact and reports an Uncommitted status. Retry capture or Clear after resolving storage errors. Temporary and orphaned files may remain and are not automatically purged; never delete the working cache as routine disposable data.

Project save records the currently committed revision. After stopping transport, wait for `Ready (committed)` and a stable revision before saving; pending chunks normally commit after 250 ms without incoming capture blocks. Closing flushes pending capture on the worker, but captures committed after the last project save are not automatically adopted on reopen. Older immutable manifests remain available so a saved project restores its own revision, even after later unsaved capture or Clear operations.

Restore runs asynchronously with mapping paused and a Loading status. It rebuilds compact stereo min/max peaks (512 frames per peak) one bounded chunk at a time; it does not retain full-song PCM in RAM. Missing or corrupt chunks appear unavailable in red, while unrelated valid audio and captured silence remain visible. Mapping pauses until assets are restored and the project reopened, or the user explicitly clears and recaptures. A host sample-rate mismatch retains the old map and pauses mapping until Clear and recapture at the new rate. Tone scheduling and dry passthrough remain independent. Version-1 project state preserves tone settings and starts a new empty capture session.

The M1 portability guarantee is reopening the same saved FL Studio project on the same PC and Windows user profile. Save As/duplicate-project isolation, Pack Project, Relink, and cross-PC moves are deferred. Copied projects can share asset identities; concurrent editing is unsupported. The plugin does not infer the `.flp` path. See [M1 validation](../validation/fl-studio-m1-validation.md) for the host acceptance workflow and outstanding checks.
