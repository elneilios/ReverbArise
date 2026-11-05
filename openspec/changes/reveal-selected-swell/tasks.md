# Tasks

Owner review supersedes the original span-preserving plan: explicit list selection now applies the same pan/zoom as Fit. Checks for the first implementation remain historical evidence; revised acceptance tasks are reopened below.

## 1. Complete-context framing

- [x] 1.1 Reuse the existing contextual Fit operation for explicit selected-swell framing, removing the custom span-preserving navigation helper/method; verify existing selected/pending/overlap/sparse/empty/boundary Fit and initial-Perform regressions through the Windows build wrapper.
- [x] 1.2 Update docs/development/perform-navigation.md to distinguish automatic list Fit, initial handoff and passive refresh; verify the description against the revised binding and unchanged contextual Fit tests.

## 2. Identity-bound list integration

- [x] 2.1 Bind explicit accepted list selection and same-primary row reactivation to contextual Fit after coherent refresh, retaining epoch/mode/primary guards and Ctrl/Shift selection; verify real pointer/keyboard list and binding tests cover re-click after navigation, reorder/rename, empty selection, stale restore and checkbox/menu independence.
- [x] 2.2 Verify automatic framing equals manual Fit endpoints and that lanes/target/handles/ruler/Overview/toolbar, navigation persistence and mode return/reopen agree; verify passive render/refresh does not refit and selection navigation preserves creative settings, source/render/IR identities, generation count and host transport without starting playback.
- [x] 2.3 Apply maintain-user-guide to the revised automatic Fit behavior, update Perform instructions and record semantic currency; verify scripts/check-user-guide.ps1 passes.

## 3. Integrated validation and review

- [x] 3.1 Run scripts/build.ps1 -Configuration Debug -Test, scripts/check-ui-standards.ps1 and strict OpenSpec validation; record exact revision/environment, native MSVC/CTest results, cached versus cold builds and separately scoped host/DPI coverage in dated validation evidence.
- [x] 3.2 Launch the revised integrated native Perform preview directly, preserving any active review until the owner finishes; record explicit owner navigation-feel acceptance of automatic complete-region Fit before claiming approval, with separate approval required for any new material family/style exception before wider rollout.
- [x] 3.3 Complete one brief manual native Perform smoke for automatic list Fit, re-click after pan/zoom and Fit equivalence after automated checks; record native and actual FL Studio outcomes separately without asking the owner to repeat passing automated edge cases.
- [x] 3.4 Commit the implementation on the issue branch and open or update one PR targeting main with Closes #93, the owner-approved change to the original issue requirement, OpenSpec link, reused controls, test evidence and remaining gaps; verify the PR stays open for owner review with no main push or self-merge.

## Workflow follow-up

- Archive/sync through the vanilla workflow only when separately requested after delivery and required review.
