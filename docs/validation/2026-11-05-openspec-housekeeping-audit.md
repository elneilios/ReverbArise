# Unarchived OpenSpec audit — 5 November 2026

Audited clean, fetched `origin/main` / `HEAD`
`cbe5e0fa2ef03f5f7cbea4623a9d0b2b3c3027b5` in the Windows worktree
`C:\Users\neila\.codex\worktrees\11b7\k7ma-preverb`.
The owner authorized archiving technically completed changes in a housekeeping
PR. This is a specification/documentation change; production code, tests, build
scripts, generated OpenSpec workflows, configuration and user assets are unchanged.

## Disposition

| Change | Tasks | Implementation/review evidence | Disposition |
| --- | --- | --- | --- |
| `realign-perform-workspace` (#46) | 21/21 | Merged [PR #48](https://github.com/elneilios/ReverbArise/pull/48); [native and host acceptance](2026-11-04-issue-46-main-reconciliation.md) | Synced and archived |
| `preview-and-generate-swells` (#38) | 20/20 | Merged [PR #56](https://github.com/elneilios/ReverbArise/pull/56); [integrated Release/Debug 63/63 and retained owner acceptance](2026-11-04-issue-38-main-reconciliation.md) | Synced and archived |
| `fix-manual-swell-generation` (#55) | 8/8 | Merged [PR #62](https://github.com/elneilios/ReverbArise/pull/62); [Release 64/64 and owner first-insertion/Ready/Audition acceptance](2026-11-04-issue-55-manual-preparation.md) | Synced and archived |
| `diagnose-learn-capture-failures` (#54) | 12/12 | Merged [PR #65](https://github.com/elneilios/ReverbArise/pull/65); [Release 66/66, focused Debug and capture fault/pressure evidence](2026-11-04-issue-54-capture-failure.md) | Synced and archived; retain host/root-cause limits below |
| `bound-waveform-paint-work` (#25 follow-up) | 0/7 | Proposal explicitly describes future implementation requiring approval; [original audit](2026-11-03-issue-25-performance-audit.md) does not implement caching | Remains active, unchanged |
| `portable-project-assets` (#8), separate branch | 0/34 | Open planning [PR #31](https://github.com/elneilios/ReverbArise/pull/31); branch task file confirms every implementation task unchecked | Not on main; untouched |
| `package-private-alpha` (#59), separate branch | 8/12 | Open [PR #63](https://github.com/elneilios/ReverbArise/pull/63); final clean-main package, extraction, final-binary host acceptance and redistribution review remain unchecked | Not on main; untouched |

All four archived changes use `spec-driven`, with all four planning artifacts
done and all 61 tracked implementation tasks complete. The merged PR metadata
and local merge ancestry agree. Focused source/test registration inspection
confirmed the implemented Learn generation, manual preparation/first insertion,
capture failure and Perform navigation/audition paths remain present. The audit
uses the existing executed validation evidence; it does not claim new native or
host execution.

The #54 task contract permits completing delivery with the remaining host smoke
explicitly reported. Its merged PR retains that limitation. The original owner's
FL Studio failure cause is still unconfirmed; native small-block failures under
load demonstrate pressure/retry behavior, not that original cause. No new host,
physical DPI, screen-reader or combined-binary acceptance is inferred for any
change. Historical evidence and its pre-merge lifecycle statements remain intact;
current merged PR metadata establishes the later review disposition. No issue
was independently closed by this housekeeping operation.

## Spec synchronization and preservation

Used the unmodified archive workflow and inline sync workflow. Refreshed archive
inputs, artifact status and specs instructions for each selected change; the
instruction responses contained no project rules/context/operation guidance.
The CLI resolved a repo-local planning root. Delta paths came from each status
response's `artifactPaths.specs.existingOutputPaths`.

Applied deltas in implementation order: #46, #38, #55, #54. Across six existing
capabilities this adds 33 requirements, modifies 22 and removes two obsolete
audition percentage requirements. No new capability or Purpose was introduced.
Existing titles, Purpose sections and 95 unrelated requirement blocks survive unchanged.
All delta operations matched the canonical content before the archive moves;
normal spec validation passed after each change's sync.

Three small canonical wording reconciliations keep the combined implemented
contract coherent: idle Learn sensitivity refreshes candidates rather than only
future passes; Diagnostics-independent first use allows explicit Generate or
opted-in Stop; supported sizes refer to the mode-appropriate workspace rather
than the retired eight-region Perform composition. These reflect #38/#46's
implemented behavior and introduce no new runtime requirements.

Moved all **25 files byte-for-byte**, including `.openspec.yaml`, into:

- [2026-11-05-realign-perform-workspace](../../openspec/changes/archive/2026-11-05-realign-perform-workspace/)
- [2026-11-05-preview-and-generate-swells](../../openspec/changes/archive/2026-11-05-preview-and-generate-swells/)
- [2026-11-05-fix-manual-swell-generation](../../openspec/changes/archive/2026-11-05-fix-manual-swell-generation/)
- [2026-11-05-diagnose-learn-capture-failures](../../openspec/changes/archive/2026-11-05-diagnose-learn-capture-failures/)

## Verification and limits

Reused installed Node **26.13.0** and OpenSpec **1.16.0** by prepending the
existing Node install to this process's PATH; the default nvm shim had no active
version. No tool installation, global setting change, build or cache reset.

| Check | Result |
| --- | --- |
| `openspec list --json` | Four completed changes absent; only unchanged `bound-waveform-paint-work`, 0/7, remains active on main |
| `openspec validate --all` | **18/18 pass**, exit 0; eight length warnings below |
| `openspec validate --all --strict` | **14/18 pass, four fail**, exit 1; only the eight length warnings below |
| Archive file inventory/SHA-256 comparison | **25/25 byte-identical**, no missing/extra files |
| Relative Markdown links in changed documents and moved artifacts | **44/44** local targets resolve |
| `git diff --check` | Pass |
| Native MSVC/CTest, UI lint, FL Studio | Not rerun: no production, test, UI or build changes; prior evidence retains its original scope |

Strict validation exposes approved MODIFIED description lengths after they reach
canonical specs. No structural validation errors were reported. The sync workflow
explicitly says not to trim/split existing MODIFIED requirements just to meet the
length limit, so this PR preserves their meaning and records the debt honestly.
It does not claim strict validation passes:

| Capability | Requirement | Description characters |
| --- | --- | --- |
| `audition-monitoring` | Monitoring protection and trim remain transparent | 656 |
| `audition-monitoring` | Group monitoring preserves dry and relative wet levels | 542 |
| `audition-monitoring` | Manual playback boundaries ramp safely | 599 |
| `automatic-swell-processing` | Future swells inherit a valid bundled IR | 517 |
| `automatic-swell-processing` | Marker-control changes invalidate obsolete analysis | 601 |
| `song-timeline-audio` | Accepted source retains deterministic generation provenance | 515 |
| `workflow-modes` | Validated useful Stop commits automatically enter Live | 539 |
| `workflow-modes` | Creative mode names are separate from durable identity | 572 |

Archive preservation checks used a pre-sync inventory retained under ignored
`out/openspec-audit/`; no audit helper is part of the PR. Owner review and merge
remain outstanding for this housekeeping branch.
