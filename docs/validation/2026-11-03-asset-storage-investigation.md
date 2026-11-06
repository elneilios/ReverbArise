# Issue #8 investigation evidence — 2026-11-03

## Scope

Baseline `752b3a33865ca6ec3e2e7b4b13baac839509b8bb`, latest origin/main when the isolated worktree was created. Branch `codex/8-portable-asset-design`. [Audit/source index](../development/asset-storage-audit.md). Static code/test inspection and arithmetic only; no native MSVC/CTest or FL Studio results are claimed.

Read #8 and empty comments, #11 and terminology clarification, relevant canonical specs, current storage/session/worker/engine code and focused persistence/asset test assertions. Read #11's in-flight design at dc003a7 without edits. Chat handoff failed because its chat had an active writer; no acknowledgment received.

## Capacity model

PCM only: `duration × sampleRate × channels × 4`; MiB = bytes / 1,048,576. Chunk counts assume contiguous nonzero source split every five seconds. Three changed passes retain three copies in this simplified model. Render duration is an assumption, not measured convolution.

| Model | 44.1 kHz MiB | 48 kHz MiB | 96 kHz MiB |
| --- | ---: | ---: | ---: |
| 180 s stereo source, 36 chunks/pass | 60.56 | 65.92 | 131.84 |
| 300 s stereo source, 60 chunks/pass | 100.94 | 109.86 | 219.73 |
| Three changed 300 s source versions | 302.81 | 329.59 | 659.18 |
| 300 s song, 50% mapped nonzero audio | 50.47 | 54.93 | 109.86 |
| 40 renders × 8 s × four versions | 430.66 | 468.75 | 937.50 |
| Six stereo IR writes × 4 s | 8.07 | 8.79 | 17.58 |

Sparse remaining positions can be mapped silence or gaps; these stay semantically distinct. Actual file count depends on intervals/boundaries. Ten equal sets of six IR writes model 87.89 MiB at 48 kHz if each write occurs; no particular UI behavior is asserted. Headers, descriptors, filesystem allocation, scratch and exports are additional unmeasured costs.

Non-destructive reproduction:

```powershell
$rows = foreach ($rate in 44100,48000,96000) {
  foreach ($seconds in 180,300,900,150,1280,24) {
    [pscustomobject]@{ Rate=$rate; Seconds=$seconds; Bytes=$seconds*$rate*8; MiB=[math]::Round($seconds*$rate*8/1MB,2) }
  }
}
$rows | Format-Table
```

## Future measured fixture — not performed

After separate approval, use a disposable root and deterministic 3–5 minute stereo fixture: dense audio, zero spans, discontinuous sections, unchanged pass, three changed passes, forty swells, six available IR choices, four generations, shape-only edits, failed publication and aborted staging. Report bytes/counts separately for source, all manifest JSON, IR WAV/JSON, render WAV/JSON, prior revisions, scratch, quarantine and exports. Include logical bytes versus volume allocation, inaccessible files and duplicate bytes by exact file hash versus decoded identity. Unknown references are not orphans.

Record fixture seed, build/engine/rate, input/output hashes, chunk boundaries, per-step counts, worker CPU/wall/RSS. Count whole history and latest reachability separately; verify shape-only edits produce no full-render writes. Cover clones, Save As, simultaneous writers, older snapshots, disk-full and recovery. This is an experiment specification, not evidence it ran.

## Limits

No build, native tests, FL Studio launch, second-profile transfer, real-song capture, compression benchmark or user-asset scan occurred. Inspected tests were not rerun. No stored source/render was moved, purged or repaired. Host Save As/path support, clean-profile portability, power-loss durability, NTFS allocation and actual codec performance remain unverified. Artifact validation results will be recorded after authoring.

## Completed planning checks

- Installed OpenSpec 1.16.0 workflow used without modifying tooling/configuration. The system `openspec` shim had no active nvm Node; invoked the installed OpenSpec entrypoint with the bundled Node runtime instead, without changing machine configuration.
- `openspec status --change portable-project-assets`: all four planning artifacts present. Both declared capabilities have their specification delta. All 34 future implementation tasks remain unchecked.
- `openspec validate portable-project-assets --strict`: passed.
- Relative links in the new audit/evidence/planning documents: 10 checked and resolved; artifact files are ordinary files, not reparse links.
- Capacity-model rows reproduced with the documented PowerShell arithmetic; all displayed rounded values matched.
- `git diff --check`: passed; staged scope checked to contain documentation and the new proposal only, with no Source/tests/scripts/canonical-spec/tooling changes.

These checks validate planning structure and documentation, not storage safety of an implemented feature. All runtime/host/codec limitations above remain open.
