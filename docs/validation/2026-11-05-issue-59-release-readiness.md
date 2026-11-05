# Alpha release-readiness extension for PR #63

## Scope and risk

Owner requested a concise guide-maintenance skill/currency gate, generated Settings About build identity/copy action, guide instructions and tests. Existing `package-private-alpha` Apply change is extended with tasks 5.1–5.5; original final-main/host/licensing tasks remain pending. No generated OpenSpec tooling/config changes, DSP/state migration, new UI tokens/families, installer system or publication.

Guide maintenance is low-risk authoring plus a medium-risk read-only release gate. Build identity/argument plumbing and stale-metadata rejection are medium-risk infrastructure. About uses existing shared control/text mechanisms with native component/binding checks. Final integration reuses the supported Release wrapper, existing CTest and existing native screenshot mechanism. No callback I/O or runtime Git query.

## Implementation decisions

- `maintain-user-guide` reviews affected delivered code, canonical specs and approved active changes; it is integrated into deliver-issue. Guide source stays static/offline. The explicit review command records normalized UTF-8/LF text hashes and exact PNG hashes without a self-referential commit/clock. Packaging checks currency before building and never invokes an agent or writes tracked documentation.
- CMake generates one matching header/JSON identity from product version, explicit tag/candidate and the actual source root's full Git SHA/status. Ordinary build/install defaults reset release labels; explicit alpha build/install arguments preserve the intended label. No timestamps, random IDs or workspace paths enter identity. Packaging checks generated metadata; native UI checks compare compiled/displayed values to JSON. No unrelated parent Git identity is borrowed for exported source.
- About is a sixth page in the existing Settings container. It uses section/control/helper text roles, EllipsisLabel and unchanged ActionButton, without moving the original pages. Copy reports product version, build identifier, full SHA and source status; no project/audio data. Guide documents Settings → About and Copy build information. Numeric guide version is filled from release identity, not a handwritten value.

## Focused evidence

- Warm Release configure/build passes, logs `out/build-logs/Release/20261105-014511-c6424f8a/`; initial development identity is local/modified and is not represented as a released alpha.
- Focused native checks pass **4/4**: compiled product version, deterministic build-information contract, guide currency and Learn/UI About bindings. The first fresh CTest shell lacked Get-FileHash in the new currency fixture; the fixture now uses managed SHA-256 rather than depending on shell module autoload. Re-run **4/4**, 5.19 s. This is separate from final full regression.
- Currency fixtures cover missing/stale/added/deleted production input, canonical spec, guide, zero-byte source, CRLF/LF stability and no mutation by checks. Build identity fixtures cover source SHA, version/candidate, local reset, dirty source, repeat generation, invalid inputs and unavailable-root identity. Packaging fixtures cover missing review before any build, unchanged review during packaging and stale About identity rejection.
- Native `--guide-only` evidence captures the actual About page alongside the existing workflow images; the development capture is visually inspected at the unchanged 640×570 Settings size. Full commit/source-status text fits and the copy action's accessibility/focus uses the approved family. No browser/DAW/physical DPI acceptance is implied.

Final source, complete suite/fixture counts, immutable internal artifact and directly launched native preview are recorded below when complete. Previously installed alpha is not modified by this work. PR remains open/unmerged; no tag, publication or external binary sharing.

Final focused script runs: release orchestration **61 checks each** in Windows PowerShell 5.1/PowerShell 7 (candidate, SHA and dirty metadata rejected independently); build wrapper **74 assertions each**; installer fixtures pass in both shells. These simulate orchestration/build failures and are distinct from native results. Currency fixtures pass in both shells, including exact zero-byte/LF/CRLF input handling. Skill validation passes. CMake identity fixtures pass after the source-root guard addition.

The guide was reviewed through maintain-user-guide against delivered code, canonical specs and approved active changes. Settings About/copy instructions and generated guide version were added; earlier Learn/capture, manual/automatic generation, Perform live/DAW rendering and optional Export instructions remain grounded in working bindings. The deliberate review record passes `check-user-guide.ps1`; matching hashes are currency evidence, not an automated substitute for semantic review. Final Release packaging will verify the record without changing it.

## First full run and focused correction

Source `d4289d4f6a7e92608624471f71b088963a13ae7a`: Release compiled identity correctly identified `v0.1.0-alpha.rehearsal.4`, full SHA and unmodified source. CTest **67/68**, 375.56 s; broad `preverb_swell_ui_test` reported a SegFault (no call stack), so release packaging correctly failed before producing any completed ZIP. Full logs: `out/build-logs/Release/20261105-020645-0ff7d43a/`. This is not reported as a passing full run.

The broad Settings fixture now explicitly returns to Recovery after visiting all six pages before its existing Recovery assertions/actions. Copy binding uses an injectable per-editor text writer: production defaults to JUCE's system clipboard; automated fixtures capture the full report without modifying the desktop clipboard. No assertion, timeout or readiness gate is removed/relaxed. The failure did not supply a stack trace, so a more specific crash-cause claim is not made.

Warm rebuild passes (`out/build-logs/Release/20261105-021609-e942c1a3/`). Focused metadata/currency/Learn About/broad UI rerun **4/4**, 34.98 s, including **27.45 s** broad UI with native screenshot checkpoints under `out/readiness-ui-regression/`. The new copy sink preserves the report/state assertions; the live review still uses the real clipboard action. Follow-up full validation is required below, not inferred from this focused pass.

## Final clean-source rehearsal

Source **`54e8a7f5559b7da5334ebb42daaabbd9dc65ff46`**, clean committed branch. Command: `scripts/release.ps1 -ReleaseTag v0.1.0 -Candidate alpha.rehearsal.4` (no Publish). Warm existing MSVC Release tree/cache; Visual Studio 18 2026, MSVC 19.51.36260.0, CMake 4.3.1-msvc1. Wrapper configure/build, complete native CTest **68/68**, **204.66 s**, built version/x64 PE/notices, generated identity validation, same-source native guide capture and immutable packaging all pass. Logs: `out/build-logs/Release/20261105-022312-19d08d06/`. The earlier failing full run remains documented above; timing is not a claimed speedup.

Candidate output: `out/releases/v0.1.0-alpha.rehearsal.4/`. Fresh extraction: `out/alpha-review/v0.1.0-alpha.rehearsal.4/`. Independently checked archive SHA against both checksum and manifest, `Assert-ReleaseArchive`, and all **23** extracted file paths/lengths/hashes plus exact inventory count. SHA-256:

- ZIP: `11334b052530d3ac05a28e06ab328cc43f0ad6d4ac8e06d392bf2d7d23190285`.
- Module: `b3e2e102af408d5a09230affe2161f6ac85cddf06f8b94f75c509daa73c2dfdc`.

Manifest buildInformation and guide provenance agree on the full source SHA; running identity is version **0.1.0**, candidate **alpha.rehearsal.4**, identifier **v0.1.0-alpha.rehearsal.4**, unmodified source. Guide contains no unresolved slots, includes About/copy instructions, and all **five embedded PNGs** decode with exact provenance dimensions/hashes (approved logo + four workflow shots). Packaging left tracked content clean, including guide source/review record; subsequent `check-user-guide.ps1` passes read-only. Generation determinism/reset is proven separately by the CMake fixture, not inferred from archive timestamps or a second native package build.

Final `check-ui-standards.ps1`: **50 files / 61 unchanged bounded occurrences**. `openspec validate --all --strict`: **23/23**. Skill validation and diff whitespace checks pass. No source changes followed this successful rehearsal; later commits only retain evidence and task completion. No redundant full native rerun for those administrative changes.

Retained [manifest](2026-11-05-issue-59-alpha4-manifest.json), [guide provenance](2026-11-05-issue-59-alpha4-guide-provenance.json) and [native About screenshot](2026-11-05-issue-59-about.png). Inspected the final 640×570 About image: approved shared styling, three metadata rows including full SHA, and Copy action fit. Directly launched `out/build/windows-msvc/Release/preverb_learn_generation_ui_test.exe --review-about`, process **42324**, confirmed `settingsWindow`; left open for owner review with the production clipboard binding. Automated Copy assertions use an isolated sink and verify the complete report without altering the desktop clipboard. Native tests also assert bounds/accessibility and unchanged serialized session, capture revision and render count.

Limits: Release/native fixtures and software rendering are verified; no new FL Studio, physical DPI/screen-reader, Debug, clean-machine or second-DAW acceptance is claimed. Browser security blocks local-file automation, so newest extracted HTML browser rendering is not claimed; earlier owner offline acceptance is not substituted for final task 4.2. Previously installed rehearsal.3 is unchanged. Final clean-main package, consolidated host acceptance and explicit redistribution approval remain pending in tasks 4.1–4.4. No tag push, publication, external binary sharing, merge or issue closure.

## Owner approval

2026-11-05: owner replied **“Approved”** after presentation of the completed release-readiness extension and the directly launched native About preview. Recorded as approval of the presented extension, including the About section's appearance and interaction. The explicit instruction to leave PR #63 unmerged and unpublished remains in force. This approval does not supply the separate final clean-main, extracted-guide/host or redistribution acceptance required by tasks 4.1–4.4. Evidence-only update; implementation and verified candidate are unchanged.
