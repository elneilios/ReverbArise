# ReverbArise — Agent Guidance

ReverbArise is a Windows VST3 reverse-reverb effect built with C++20 and JUCE. Existing identifiers may still say `Preverb`; preserve host/session compatibility unless explicitly tasked with a migration.

## Sources of truth
- **Future requirements:** Read the assigned GitHub issue, its approved mockups, and only the directly relevant dependencies. Do not load the full backlog.
- **Current behaviour:** `openspec/specs/` records the system as implemented. Differences from planned GitHub work are intentional. Use the existing, unmodified OpenSpec workflows to propose, apply, and archive changes.
- **Documentation:** Start with `docs/README.md` to locate relevant current technical guidance. `docs/product-vision.md` gives high-level direction; `docs/archive/` is historical, not current instruction.
- **Document placement:** Put new technical guides in `docs/development/`, UI/design guidance in `docs/design/`, and test evidence or metrics in `docs/validation/`. Keep the `docs/` root for `README.md` and `product-vision.md`; use dated `docs/archive/YYYY-MM-DD/` folders for superseded historical material. Update affected links when moving files.

## Contribution workflow
- Follow [CONTRIBUTING.md](CONTRIBUTING.md): work on an issue-specific branch, open a PR for owner review, and do not push directly to `main` or self-merge.

## Implementation safeguards
- **Local Windows validation:** Use `scripts/build.ps1 -Configuration Debug|Release [-Test]`; see [development setup](docs/development/development.md). Its per-build-directory lock prevents overlapping wrapper operations. Do not run raw CMake/MSBuild against an active wrapper's tree; independent worktrees use separate trees. Report fixture checks separately from native MSVC/CTest results.
- Inspect affected code and focused tests before changing behaviour; prefer extending working mechanisms over replacing them.
- Keep the real-time audio callback free of disk I/O, blocking, and unbounded allocation or processing.
- Protect captured source, render revisions, persistent identities, cache assets, and saved DAW projects. Do not silently rename or discard durable state.
- Follow approved issue mockups and existing design conventions. Do not introduce speculative controls or expose unimplemented DSP.
- For UI work, follow [UI development standards](docs/development/ui-development-standards.md): reuse shared controls, consolidate affected interaction setup during planned refactors, and include component/binding tests. Ask the owner to approve each new/revised library control family's look and interaction feel before wider rollout. Shared styling alone is not a shared control library; do not require a separate whole-plugin migration before feature work.
- Stay within the assigned issue. Flag necessary adjacent work rather than implementing it unrequested.
- Update living technical guidance for implemented changes; preserve dated validation evidence.

## Token and cost discipline
- Read selectively: use targeted searches and file sections; do not traverse all archived OpenSpec changes or documentation by default.
- Keep plans, progress summaries, and tool output concise. Prefer diffs and focused test results over full logs.
- Implement in reviewable increments; test impacted functionality before widening the test scope.
- If an unexpected architectural migration or major scope expansion appears necessary, report it before proceeding.
