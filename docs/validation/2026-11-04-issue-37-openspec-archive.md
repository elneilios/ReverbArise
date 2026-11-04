# Issue #37 — UI standards OpenSpec sync and archive

Base: latest main `5b41c57`. Proposal [PR #42](https://github.com/elneilios/ReverbArise/pull/42)
and implementation [PR #44](https://github.com/elneilios/ReverbArise/pull/44) are
merged; [Issue #37](https://github.com/elneilios/ReverbArise/issues/37) is closed
as completed. All four planning artifacts and all 10 tracked tasks were complete.

## Outcome and implementation cross-check

The vanilla archive workflow and inline `openspec-sync-specs` workflow created
[`ui-design-system`](../../openspec/specs/ui-design-system/spec.md) from the
change's sole delta: six added requirements and 12 scenarios. The authored
Purpose and all requirement/scenario text are preserved verbatim, under canonical
title and Requirements headings. No delta-operation headings remain.

| Requirement | Merged implementation / evidence reviewed |
| --- | --- |
| Production standards identify their approval baseline | Production guide revision 1 separates scoped Learn/header metrics and approval evidence from historical/provisional suggestions |
| Reusable patterns own appearance and interaction | Shared text roles in `WorkspaceTheme`, existing Controls families, `IrSelector`, Learn/header consumers and explicit remaining migration inventory |
| Density preserves invariant visual and interaction roles | Comfortable Learn profile and invariant roles documented; compact remains an approval process with no approved numerical values or runtime selector |
| Styling drift has actionable automated checks | Scoped PowerShell guard, exact-occurrence/count definitions/debt, diagnostic failures, command fixtures and CTest registration |
| Component and native evidence establish practical UI confidence | `UiStandardsCases`, existing native fixture modes and dated implementation evidence distinguish automated/owner/Windows DPI/FL Studio coverage |
| Standards evolve through traceable review | `AGENTS.md`, development standards, standards revision and scoped exception lifecycle require reuse and explicit review |

Conditional future compact-profile and revised-family scenarios describe the
implemented review policy, not a delivered compact UI or new control behaviour.
The spec adds no DSP, performance optimisation, host feature or unsupported
Windows DPI acceptance claim. Existing implementation evidence remains unchanged.

## Archive and verification

Archived to
[`2026-11-04-establish-production-ui-standards`](../../openspec/changes/archive/2026-11-04-establish-production-ui-standards/proposal.md).
All five archived files, including `.openspec.yaml`, match their pre-archive Git
blobs exactly. Internal proposal/design links still resolve after the move.

- `openspec validate --specs --strict`: 16/16 passed after sync.
- `openspec validate establish-production-ui-standards --strict`: passed before archive.
- `openspec validate --all --strict`: 18/18 passed before archive; 17/17 after.
- Canonical content comparison confirms the delta's complete Purpose and six
  requirement blocks are represented, with nothing remaining to sync.
- `openspec list --json` lists only `bound-waveform-paint-work`, still 0/7 tasks.
  Its files match main; no performance requirements were synced or implemented.
- The active `establish-production-ui-standards` folder is absent; the dated
  archive and canonical specification are present. `git diff --check` passes.

This branch changes only specs/archive placement and this housekeeping evidence.
No C++, tests, scripts, resources, build configuration or generated OpenSpec
workflow/configuration changes were made. Native builds and tests were not rerun
for this spec-only operation, as requested. Prior actual Windows DPI/host limits
remain recorded in [implementation evidence](2026-11-04-issue-37-ui-standards.md).

Submitted on `codex/37-archive-ui-standards`; PR review and merging remain with the owner.
