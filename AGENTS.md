# ReverbArise — Agent Guidance

ReverbArise is a Windows VST3 reverse-reverb effect built with C++20 and JUCE. Existing identifiers may still say `Preverb`; preserve host/session compatibility unless explicitly tasked with a migration.

## Sources of truth
- **Future requirements:** Read the assigned GitHub issue, its approved mockups, and only the directly relevant dependencies. Do not load the full backlog.
- **Current behaviour:** `openspec/specs/` records the system as implemented. Differences from planned GitHub work are intentional. Use the existing, unmodified OpenSpec workflows to propose, apply, and archive changes.
- **Documentation:** Start with `docs/README.md` to locate relevant current technical guidance. `docs/product-vision.md` gives high-level direction; `docs/archive/` is historical, not current instruction.
- **Document placement:** Put new technical guides in `docs/development/`, UI/design guidance in `docs/design/`, and test evidence or metrics in `docs/validation/`. Keep the `docs/` root for `README.md` and `product-vision.md`; use dated `docs/archive/YYYY-MM-DD/` folders for superseded historical material. Update affected links when moving files.

## Issue delivery orchestration
- For requests to deliver, implement, or continue a GitHub issue, use the repository skill [`deliver-issue`](.agents/skills/deliver-issue/SKILL.md). It resolves issue context, reuses existing OpenSpec changes and applies risk-aware verification.
- The skill orchestrates **vanilla** OpenSpec explore/propose/apply; it does not replace their generated instructions, approval gates or task tracking. Explicit requests for individual OpenSpec operations may use those workflows directly.
- Follow the specialist [Apply policy](.agents/skills/deliver-issue/references/apply-policy.md) for risk-based verification and task completion. Efficiency advice never waives mandatory OpenSpec instruction refreshes, owner approvals, required tests or final validation gates.
- Do not change generated OpenSpec skills or `openspec/config.yaml` to customise this delivery workflow.
- **Completed-work cleanup:** Automatically remove regenerable build output owned by the completed task once validation and delivery are finished, following the [cleanup procedure](.agents/skills/deliver-issue/references/completed-work-cleanup.md). Preserve active builds and reviews, local changes, releases, rollback bundles, durable assets and validation evidence. Do not delete a checkout or clean other chats' worktrees as routine task cleanup; report anything retained and why.

## Contribution workflow
- Follow [CONTRIBUTING.md](CONTRIBUTING.md): work on an issue-specific branch, open a PR for owner review, and do not push directly to `main` or self-merge.

## Implementation safeguards
- **Release packaging:** Use the [build-release skill](.agents/skills/build-release/SKILL.md) when preparing a release/alpha ZIP. Author release-specific notes against the previous shared build before running the existing packager; preserve its validation and immutable-output gates.
- **Local Windows validation:** Use `scripts/build.ps1 -Configuration Debug|Release [-Test]`; see [development setup](docs/development/development.md). Its per-build-directory lock prevents overlapping wrapper operations. Do not run raw CMake/MSBuild against an active wrapper's tree; independent worktrees use separate trees. Report fixture checks separately from native MSVC/CTest results.
- **FL Studio test installation:** When asked to deliver a build for local host testing, use the [install-test-vst3 skill](.agents/skills/install-test-vst3/SKILL.md) and `scripts/install-test-vst3.ps1` instead of manual copying. It builds Release and installs into the system Common Files/VST3 folder with a retained previous bundle. FL Studio does not scan VST3 in custom user search paths. See [test installation](docs/development/fl-studio-test-install.md).
- Inspect affected code and focused tests before changing behaviour; prefer extending working mechanisms over replacing them.
- Keep the real-time audio callback free of disk I/O, blocking, and unbounded allocation or processing.
- Protect captured source, render revisions, persistent identities, cache assets, and saved DAW projects. Do not silently rename or discard durable state.
- Follow approved issue mockups and existing design conventions. Do not introduce speculative controls or expose unimplemented DSP.
- **Production UI:** Read the [production style guide](docs/design/ReverbArise_UI_Layout_and_Style_Guide.md) and reuse its named tokens, text roles and [shared control policies](docs/development/ui-development-standards.md). Historical mockup numbers and legacy Perform styling are not validated production defaults. Learn comfortable spacing is approved; compact density values await Perform review.
- **UI changes and exceptions:** New tokens, visual variants and one-off exceptions require explicit owner review with scope, rationale and evidence before rollout; update code, standards revision and tests together. Future UI OpenSpec plans must identify affected control reuse, justified exceptions and review gates. Run `scripts/check-ui-standards.ps1`; do not bypass failures by regenerating its exact-occurrence debt inventory. Ordinary layout/domain arithmetic remains allowed; retain native/component validation.
- For UI work, follow [UI development standards](docs/development/ui-development-standards.md): reuse shared controls, consolidate affected interaction setup during planned refactors, and include component/binding tests. Ask the owner to approve each new/revised library control family's look and interaction feel before wider rollout. Shared styling alone is not a shared control library; do not require a separate whole-plugin migration before feature work.
- Minimize owner UAT: prove behavior with automated component, integration, audio and persistence tests wherever practical. Use lightweight native previews for early control feedback and consolidate FL Studio testing into one brief final smoke test. Do not ask the owner to repeat passing automated edge cases or prior acceptance; request extra host checks only for failures or concrete unresolved host behavior. Record untested host/DPI coverage honestly.
- At component/UI approval checkpoints, launch the prepared native preview for the owner directly when available; do not make them find and run a command or launcher. Avoid duplicate review windows, keep screenshots as supporting evidence, and preserve an open review window until the owner finishes with it.
- Stay within the assigned issue. Flag necessary adjacent work rather than implementing it unrequested.
- Update living technical guidance for implemented changes; preserve dated validation evidence.

## Token and cost discipline
- **Bound discovery:** Use targeted searches and file sections. Follow references for concrete dependencies or uncertainty; once affected call paths and acceptance criteria are clear, stop broad scans and record unresolved questions. Avoid wholesale archive or backlog reads.
- **Reuse warm environments:** Consult existing toolchain guidance and valid caches before probing or downloading. Preserve usable build trees and caches while work, debugging or acceptance is ongoing; avoid unexplained cache clearing or repeated cold setup. At completed delivery, apply the cleanup rule above. Respect the build lock described above.
- **Batch independent reads:** Group bounded read-only searches, status checks and file reads when practical. Keep mutations sequential and check exit codes.
- **Keep evidence proportionate:** Prefer diffs and focused result summaries over full logs. Record commands, revision, environment, results and limitations once; link to canonical docs and dated evidence elsewhere instead of duplicating narratives.
- **Report efficiency honestly:** Distinguish warm/cached from cold runs and record skipped checks explicitly. Claim speedups only with comparable measurements; preserve the validation distinctions above.
- If an unexpected architectural migration or major scope expansion appears necessary, report it before proceeding.
