# Tasks

## 1. Persistent Perform safety notice

- [ ] 1.1 Add the mode-only warning fallback to the existing workspace notice selector with the specified primary/detail copy, after all existing notices including IR preparation. Verify focused cases cover stopped/playing/empty/playback-disabled Perform, every existing override, override clearance, mode return and unchanged Learn/Export results; update historical-error assertions without weakening failure filtering.
- [ ] 1.2 Update workflow/Perform guidance and maintain the offline user guide using the repository skill. Verify documentation matches the actual primary/detail copy, labels explicit host controls accurately, and makes no automatic-mute, detection or defect-fixed claim; complete the guide currency checks applicable to the delivered UI change.

## 2. Integrated presentation and delivery

- [ ] 2.1 Verify existing NoticeBar/accessibility and editor binding/geometry coverage includes the persistent warning and replacement/restoration at real header message widths across 1100x850, 1400x900, 1400x1000 and 1600x1000. Use the repository Windows build wrapper for native validation, run `scripts/check-ui-standards.ps1`, and confirm the diff introduces no audio/processor/serialization changes. Record native results separately from fixtures; do not change standards/debt inventories to bypass failures.
- [ ] 2.2 Prepare a native integrated message preview and obtain scoped owner copy/layout acceptance, including the warning, a temporary overriding notice and restored warning. Reuse approved typography/colours/geometry without new control variants. Respect the owner's preference to perform computer actions; record actual host/DPI checks and any unavailable coverage, without repeating transport experiments or installing a plugin unless requested.
- [ ] 2.3 Record dated validation and update this separate review PR with `Refs #1`, checked task evidence, remaining limits and no defect-fixed claim. Verify OpenSpec change validation and `git diff --check`; keep issue #1 and PR #74 open and do not merge, Apply unrelated work or archive without the appropriate owner request.

## Workflow follow-up

- Implementation begins only after a new owner request to apply this proposal.
- Archive/sync belongs to the normal reviewed delivery workflow after implementation; it is not part of this planning step.
