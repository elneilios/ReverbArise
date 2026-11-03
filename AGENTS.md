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
- Minimize owner UAT: prove behavior with automated component, integration, audio and persistence tests wherever practical. Use lightweight native previews for early control feedback and consolidate FL Studio testing into one brief final smoke test. Do not ask the owner to repeat passing automated edge cases or prior acceptance; request extra host checks only for failures or concrete unresolved host behavior. Record untested host/DPI coverage honestly.
- At component/UI approval checkpoints, launch the prepared native preview for the owner directly when available; do not make them find and run a command or launcher. Avoid duplicate review windows, keep screenshots as supporting evidence, and preserve an open review window until the owner finishes with it.
- Stay within the assigned issue. Flag necessary adjacent work rather than implementing it unrequested.
- Update living technical guidance for implemented changes; preserve dated validation evidence.

## Efficient execution and validation
- **Start with the smallest useful context.** Read the assigned issue, current branch/worktree status, relevant canonical spec(s), and directly affected code/tests. Follow references only when a concrete dependency or uncertainty requires them. Once the affected call paths and acceptance criteria are clear, stop broad discovery; record unresolved questions instead of repeatedly scanning the repository.
- **Use a staged validation ladder.** For documentation/planning-only changes, run link, format, OpenSpec and diff/scope checks; do not compile unchanged C++ solely for formality. For implementation, run the smallest meaningful tests during iteration, then the full relevant native build/CTest matrix once at the final integration gate or after changes that invalidate its result. Run extra builds only for a stated regression risk or failure. Never represent skipped validation as passed.
- **Reuse proven environment and work.** Check existing toolchain/preset guidance and available caches before probing or downloading. Use `scripts/build.ps1` and its cooperative build-directory lock for native builds. Do not run multiple wrappers, raw CMake or MSBuild concurrently against one binary tree; independent worktrees are fine. Do not silently clear caches or repeat cold builds.
- **Reduce tool-call overhead.** Batch independent read-only searches, status checks and targeted file reads when practical. Prefer changed-file diffs and test failure summaries over repeated full file reads and log dumps. Avoid issuing one command per line/file when a bounded grouped command answers the same question. Keep potentially mutating commands sequential and check their exit codes.
- **Keep evidence proportional and reusable.** Link to existing canonical docs and dated logs instead of copying the same narrative into the proposal, design, validation report and PR. Capture exact commands, revision, environment, results and limitations once; summarize and link elsewhere. For a focused change, avoid creating a new long document unless it records a genuine contract or lasting evidence.
- **Respect decision gates.** Planning-only requests stop after reviewable OpenSpec artifacts and planning validation; do not implement or run broad native tests. Implementation requests may proceed within their approved scope, but separate owner review is required before PR merge or OpenSpec archive/sync when specified. Do not pause for routine confirmations already covered by the issue; surface material trade-offs or safety risks before taking irreversible action.
- **Report efficiency honestly.** Distinguish cached/warm from cold builds and fixtures from native tests. Do not claim speedups without comparable measurements. Never sacrifice audio-thread safety, data durability, saved-project compatibility, required tests or owner approvals for shorter runs.

## Token and cost discipline
- Read selectively: use targeted searches and file sections; do not traverse all archived OpenSpec changes or documentation by default.
- Keep plans, progress summaries, and tool output concise. Prefer diffs and focused test results over full logs.
- Implement in reviewable increments; test impacted functionality before widening the test scope.
- If an unexpected architectural migration or major scope expansion appears necessary, report it before proceeding.
