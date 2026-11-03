# Asset storage audit — Issue #8

Baseline: `752b3a33865ca6ec3e2e7b4b13baac839509b8bb`, fetched origin/main on 2026-11-03. [Design](../../openspec/changes/portable-project-assets/design.md), [dated evidence](../validation/2026-11-03-asset-storage-investigation.md), [issue](https://github.com/elneilios/ReverbArise/issues/8).

## Evidence and source index

**Verified (source)** means traced code or inspected test assertions, not executed tests or host verification. **Derived** means arithmetic/code implications. **Unknown** means unmeasured runtime/host behavior. **Proposed** behavior is in the design. No user assets were enumerated, decoded or changed. Issue documentation links predate the docs reorganization: current guides are [development](development.md), [M3 persistence](m3-persistence.md), [workflow](workflow-modes.md). M3's v3 state and development's idle-commit guidance reflect older milestones: current processor state is v8 and workflow capture is Stop-transactional. These older paragraphs do not prove current commit timing.

Links pin the audited commit; named functions identify evidence sections.

| ID | Source | Relevant evidence |
| --- | --- | --- |
| A | [Processor](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Plugin/PluginProcessor.cpp) | defaultProjectsRoot, constructor, getStateInformation, setStateInformation |
| B | [CaptureSession](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Capture/CaptureSession.cpp) | store/restore/commit/clear, stage/prepare/accept/abort pass |
| C | [CaptureAssetStore](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Capture/CaptureAssetStore.cpp) | bounded chunks, validation, snapshots, atomic moves, temporary files |
| D | [CaptureWorker](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Capture/CaptureWorker.cpp) | batching, restoration, workflow transaction ownership |
| E | [SwellAssetStore](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/DSP/SwellAssetStore.cpp) | write/verify/readIR/readRender, reparse-point checks |
| F | [SwellEngine](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/DSP/SwellEngine.cpp) | instanceRoot, saved model, import/render/shape jobs, memory eviction |
| G | [SwellSession codec](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Core/SwellSession.cpp) | state v8, default IR, asset references, calibration, validation |
| H | [WavPack decoder](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/DSP/WavPackDecoder.cpp), [ContentHash](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Core/ContentHash.cpp) | float decode, lossless checks, normalized signed zero |
| I | [Capture tests](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/tests/persistent_capture_test.cpp), [asset tests](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/tests/swell_asset_test.cpp) | old snapshots, overlap, silence/gaps, failures, >unity float, equivalent imports, metadata |

## Locations, ownership and saved state

**Verified A–G:** default production root is `%LOCALAPPDATA%\K7MA\Preverb\Projects`. Both stores use `<projectId>/<instanceId>` below it. Processor root injection supports isolated tests. Fresh CaptureSession construction generates both UUIDs; no host association establishes a shared projectId for fresh plugin instances in one FLP. ProjectId is a plugin namespace, not verified FLP identity.

```text
Projects/<project-id>/<instance-id>/
  segments/<uuid>.wav
  manifests/<uuid>.json
  irs/<content>-<random8>.wav + .json
  renders/<content>-<random8>.wav + .json
  [manifest.json: mutable store API; saved workflow uses UUID snapshots]
```

**Verified A/G:** XML `PreverbState` v8 stores IDs, manifest reference/revision, tone/workflow and SwellSession JSON v8, without PCM. Session data includes stable definitions, source/arrival, edits, activity/freeze, timing, calibration, default IR and per-swell IR/render references. Capture manifest schema 1 and managed engine version 1 are separate version domains.

**Verified B/C:** restore uses saved UUIDs and exact snapshot, checks their agreement and builds peaks from bounded chunks. It does not adopt the latest snapshot. Legacy v1 starts empty capture with new IDs. Capture manifests alone are not complete session backups: IR/render/default references also live in host state. Unsupported state is rejected before valid definitions are replaced.

## Capture chunks and revisions

**Verified B–D:** immutable stereo float32 WAV chunks are limited to five seconds and 1,920,000 frames. Exact-zero silence uses coverage metadata; gaps have no range. SHA-256, rate and frame counts validate audio. Chunk temporary files are written/verified before atomic Windows publication; manifests reference completed chunks. Overview uses 512-frame min/max peaks instead of full-song resident PCM.

Workflow passes stage a private candidate, prepare a complete validated UUID snapshot at Stop and accept only matching token/epoch/base revision. Save/read uses the previous accepted map until acceptance. Abort drops private state but does not reclaim already written files. Ordinary capture fills unmapped positions; explicit Recapture allows replacement. Identical covered PCM can reuse identity. Partial overlap can rewrite unchanged left/right chunk slices into new files, adding redundant storage. This audit changes none of these semantics.

**Verified B/C/I:** Clear publishes empty coverage while retaining previous manifests/chunks. Failed publication retains the old committed reference. Corrupt/missing chunks remain unavailable, never fabricated silence; unrelated valid ranges remain accessible. File/rename calls alone do not prove power-loss durability.

## Managed IRs and immutable wet renders

**Verified E–H:** import decodes WAV/WavPack once into managed float32 WAV. IR channels are retained; wet files are full stereo untrimmed convolution results with absolute bounds, source dependency, IR content, timing and engine. File hashes protect bytes; decoded identity includes rate/channels/length and finite samples with signed zero normalized. Render identity adds dependency/timing. Musical content identity is not necessarily byte identity.

Every successful store write uses a random filename suffix. `write` does not look up equal content for reuse: hashes in filenames do not implement disk deduplication. Capture uses UUID files. This does not imply every edit writes: F reuses compatible renders and creates shaped previews in RAM. Shape/gain/trim/name/navigation do not inherently create another full convolution WAV. Source/IR changes may create new immutable render versions; a stale job may finish writing before publication authority is rejected.

**Verified E:** WAV and JSON publish in separate renames. A crash between them can leave a WAV missing valid metadata; verify rejects it. Individual failure branches remove some temporary files, but no general scavenger exists. Chunk `.tmp-UUID`, snapshot `.tmp`, managed `.part`/`.json.part` naming alone cannot establish disposability.

**Verified F/current swell-session spec:** Clear, definition deletion and RAM eviction do not delete stored audio. The 256 MiB working limit constrains resident audio/jobs, not disk history. A rebuildable render can still be essential to a frozen/old-engine project. Legacy paths/descriptors/hashes are part of saved-state compatibility.

## Save As, copied projects and duplicated instances

| Case | Source fact / implication | Host unknown |
| --- | --- | --- |
| Fresh instances | B generates new UUID pairs, normally separate stores | Actual host construction/load order |
| Save As / copied FLP | A restores serialized IDs verbatim; no filename-based fork or FLP path adapter in inspected processor | State/context supplied by each FL Studio version |
| Duplicated instance | Copied state reads original immutable files; unique new manifest/chunk names protect prior snapshots but do not create a new namespace | Actual duplicate/copy-paste sequence and concurrent processes |
| Future writes | Independent candidate maps and local transaction tokens isolate publication; same directory does not mean shared mutable map | Complete directory/host lifecycle isolation |
| Other PC/profile | Default root changes; FLP does not carry PCM | Second-profile transfer was not tested |

The canonical clone contract promises independent publication, not automatically distinct directories. Do not claim Save As provides independent ownership. [Image-Line's project manual](https://www.image-line.com/fl-studio-learning-content/fl-studio-online-manual/html/fformats_project.htm) excludes third-party plugin samples from zipped projects. [VST3 IStreamAttributes](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Change%2BHistory/3.6.0/IStreamAttributes.html) optionally provides state/preset path/context; it does not guarantee the current FLP path, nor is it consumed by this processor's byte-buffer state API. Reliable host path support is unknown, not proven impossible. Explicit folder selection/relink is the design fallback.

## Disk growth and old-project risk

**Derived:** stereo float32 payload is `seconds × rate × 8` bytes. At 48 kHz, three minutes is 65.92 MiB and five minutes is 109.86 MiB before WAV headers, JSON or allocation. Five minutes needs at least 60 chunks for contiguous nonzero audio; discontinuities and audio/silence boundaries increase count. Five seconds at 384 kHz reaches the frame cap. Unchanged/additive passes need not duplicate a whole song. Three changed full five-minute versions model 329.59 MiB, plus overlap slices and aborted writes.

Forty renders averaging an assumed eight seconds model 117.19 MiB per generation, 468.75 MiB across four generations at 48 kHz. Six stereo four-second IR writes add 8.79 MiB; repeated equal imports can multiply that. These are arithmetic scenarios, not measured render lengths, session totals or codec results. [Evidence](../validation/2026-11-03-asset-storage-investigation.md) defines sparse/rate cases and a later measured fixture.

**Risk:** current/previous capture manifests cannot enumerate IR/render references in arbitrary offline FLPs, backups or project copies. Inactivity, RAM eviction, absent current definitions and the newest manifest cannot prove historical unreachability. Captures/imported IRs may be sole surviving source. Exports possibly used by Playlist are independently durable. The export workspace is outside this audit; workflow guidance preserves exports but does not establish actual output locations. No export directory is asserted.

## Compression

**Verified H:** WavPack currently imports IRs; managed persistence remains WAV. Decoder requests correction data and rejects non-lossless mode, lossy blocks, errors and nonfinite floats. This is not existing compressed capture/render storage support.

**Proposed:** keep WAV first; later benchmark independent lossless WavPack chunks. [WavPack](https://www.wavpack.com/index.html) supports float32 lossless coding. [FLAC](https://xiph.org/flac/faq.html) does not support float samples; conversion to integer PCM would not preserve arbitrary float32. Generic byte compression preserves bits but needs bounded framing/recovery. No ratio/CPU savings have been measured.

Require bitwise round trips for >unity, low values, subnormals, signed zero and channel order, independently of normalized content hashes. Smaller chunks limit random-read/recovery costs but add file overhead; larger compression blocks increase seek/decode latency and corruption scope. Decode/encode uses worker CPU/scratch memory, never callback work. No silent normalize/dither/quantize/resample/hybrid fallback. Retain original legacy WAVs/checksums for rollback.

## #11 handoff

Read the directly relevant `learn-capture-workspace` design on `docs/11-learn-capture-workspace` at `dc003a744b6938d5dd05675b69d0be5f13ef14aa`. Its accepted per-pass generation/default-IR provenance is proposed behavior, absent from this baseline. Provisional waveform/pass state cannot be pack authority. Existing default IR is a pack dependency even with zero swells.

#8 proposes a version-aware visitor over accepted source/session/default IR, per-swell IR/render, calibration and #11 accepted provenance. #11 owns context semantics/schema; #8 adapts after that contract is finalized without another importer, UI or capture-commit change. Unknown dependency-bearing versions block complete-pack claims and cleanup. Direct chat coordination was attempted but rejected with `already has an active writer`; no acknowledgment/schema agreement is claimed. Reconcile at phase 0. No #11 files, UI or workflow code were edited.
