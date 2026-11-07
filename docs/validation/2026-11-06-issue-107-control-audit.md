# Issue #107 control audit — 6 November 2026

## Scope and revision

Change `audit-ui-control-contracts`, branch `codex/107-control-system-proposal`.
Initial source baseline: `24cd287`, standards revision 7. Final reconciled
baseline: merged #80 `717c890`, standards revisions 8–10. Owner-approved audit
and contract integration records revision 11; production control changes remain separate.
The owner requested Apply of the umbrella audit/contracts, explicitly retaining
production migrations in separate child changes.

Deliverables: [inventory and packets](../design/ui-control-inventory.md) and
[contracts](../development/ui-control-contracts.md). No product code,
test code, workflow configuration, canonical spec, cache, capture, asset or saved
state is changed by this audit. No child issue or external comment is created.

## Execution map and method

All tracked work is documentation/governance, low runtime risk. Inventory source
accuracy and approval status are the main risks. Contract acceptance is a real
owner decision, not implied by permission to Apply. The owner explicitly approved the audit and contracts on 6 November 2026 and
requested integration against merged #80 followed by a remaining-button proposal.
This acceptance covers contracts and roadmap, not new native family appearance.

Inspected actual member declarations in `Source/Plugin/PluginEditor.h`, creation,
reparenting, layout and callbacks in `PluginEditor.cpp`, `PluginSettings.cpp`,
`PluginPresentation.cpp`, and value/reset guards in `PluginInspector.cpp`.
Inspected Learn/Export/header composition, shared controls, specialized timeline
and overview declarations/interaction paths, popup and rename-dialog creation.
Commands used bounded `rg --files`, `rg -n` and PowerShell `Get-Content` sections;
declarations alone were not treated as proof of actual surface ownership.

Coverage includes all member groups on those screens, nested generated segments,
slider value labels/temporary editors/steppers, virtualized list RowSurface hit
regions, dynamic Rename/Delete menus and AlertWindow input/OK/Cancel, knob reset
menus, native chooser/drag boundaries and auxiliary window scroll/scale shells.
Constructor styling can be overridden later; inventory distinguishes final
composition and retained hidden seams. Noninteractive paint regions are listed
without pretending they have button press/activation states.

Source-inspected existing tests: `ControlLibraryCases.h`, `UiStandardsCases.h`,
`WorkflowHeaderCases.h`, `WorkspaceNoticeCases.h`, `WorkspaceFeedbackCases.h`,
`PerformKnobCases.h`, `PerformKnobResetCases.h`, `PerformNavigationStateCases.h`,
`SwellListBindingCases.h`, `ExportControlCases.h`, `ExportLevelsCases.h`,
`LearnTimelineCases.h`, `ResponsiveEditorCases.h`, `ResponsiveScaleCases.h`.
These are candidate reuse targets, not new test execution claims. Only tests
actually opened are counted as inspected; file discovery alone is not coverage.

## Issue #80 reconciliation

Initial connected reads found PR #109 open at `bedd2adc`; this was a historical
checkpoint, not its current state. The final integration fetched `origin/main`
and fast-forwarded only this worktree's issue branch to merged PR #109,
`717c890c4121cdfc84e20c988dedf7a6da0bc811`. No #108 or #8 branch/worktree was
modified. Source inspection confirms shared icon surface, ConvolutionIrEditor
label/selector/file/status, default-versus-selected binding, screen-owned scope
and collapsed feedback. Old standalone IR members were removed by #80; inventory
rows now name their actual replacement owners. #80's revisions 8–10 are preserved.

The [upstream evidence](2026-11-06-issue-80-shared-ir.md) records native/host
acceptance and 75/75 Release CTest. No upstream result is presented as a new audit
test. No runtime/source changes were authored by this integration.

## Verification results

- `openspec validate audit-ui-control-contracts --strict` and
  `openspec validate unify-command-button-states --strict`: **passed**.
- `scripts/check-ui-standards.ps1`: final reconciled **passed**, 63 files / 59
  explicitly bounded definition/debt occurrences; audit changed no guard/debt
  entries. The initial pre-#80 pass was 61/60; differences are merged #80 work.
- Read-only PowerShell Markdown link resolution: **passed**, all relative links
  in the three new documents resolve against their containing directories
  (initial package 66; final integrated documentation/planning package 157 links).
- Inventory: **49 explicitly named consumer groups**, checked against declarations,
  construction/reparenting and dynamic creation paths above. All map to C1–C8
  and P1–P8 or an explicit native boundary; no family is claimed newly compliant.
- Contract/packet presence check: **8/8 contracts and 8/8 packet definitions**.
- Source-confirmed native preview routes: `Tests/swell_ui_test.cpp` owns control,
  messages, Export, Learn and workspace modes; `Tests/perform_composition_preview.cpp`
  owns responsive/integrated editor modes. `--review-ir-editor` is upstream #109.
- `git diff --check`, plus explicit trailing-whitespace scan including untracked
  documents: **passed**, 14 Markdown files. All 16 changed/untracked paths are
  documentation or the two changes' planning artifacts. `git diff --exit-code
  origin/main -- Source Tests scripts openspec/config.yaml openspec/specs`:
  **passed**, no authored runtime/tests/guard/config or canonical-spec changes.
- Read-only declaration/design comparison: **passed**, exact 20 remaining
  persistent TextButton members all appear in the child design. Two existing
  latches are listed separately; the child's 10 tasks remain unchecked.
- Native MSVC build/CTest: **not run**, documentation-only audit with no runtime
  or test infrastructure changes. No new fixture, performance or audio result.
- Native visual/interaction review: **not run**; no revised production family is
  being introduced here. Child previews remain mandatory for revised families.
- Windows 100/125/150/200% DPI, monitor transitions and FL Studio: **not tested by
  this audit**. Logical-size and software-scale plans do not establish OS/host fit.
- Offline user guide: no runtime behavior changed; no guide content/currency
  regeneration warranted. Final documentation integration records this decision.

## Owner review and completion boundaries

**Accepted, 6 November 2026.** The owner stated: “I approve the audit and contracts.”
They requested integration against merged #80, a proposal for remaining buttons,
and explicitly left active #108 and #8 work alone. Acceptance covers the 49
consumer groups (updated to merged ownership), C1–C8 contracts and P1–P8 roadmap.
It does not waive native review for revised production families or introduce a
new token, visual exception or interaction implementation.

Documentation integration adds links through `docs/README.md` and UI standards,
and advances the production guide/current standards reference to revision **11**
(documented contract governance only). #80 already used revisions 8–10, so the
previous candidate's proposed revision 8 was superseded without renumbering its
accepted work. Numeric/visual authorities and previous approvals remain intact.
Offline guide content/currency is unchanged because runtime behavior is unchanged.

The child `unify-command-button-states` proposal covers only residual C1/P1 work;
no production button migration is performed. #108 Level/knobs, #8 storage/recovery
and shared-engine work, other control families and native dialog internals remain
outside it. #107 stays open for its implementation slices. No GitHub issue/message,
release, VST3 install, canonical spec sync or archive is performed by this request.

The umbrella Apply refresh confirms **13/13 complete**, state `all_done`. The
button child has **4/4 planning artifacts complete**, 10 implementation tasks
unchecked. Final checks above ran after integration and child artifact creation.
No MSVC/CTest or host repetition was warranted for documentation/planning only.
