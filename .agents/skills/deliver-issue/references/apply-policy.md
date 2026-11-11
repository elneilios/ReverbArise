# Risk-aware execution policy for OpenSpec Apply

This policy is **additive** to the installed `openspec-apply-change` skill. In any conflict, follow vanilla Apply. In particular, read all `contextFiles` returned by Apply and, after each verified task, use its exact `sourcePath`/`line` guard, mark the checkbox, then rerun Apply instructions to verify updated progress.

## Establish an execution map

After vanilla status/instructions and required context loading, identify for each pending task:

1. **Type:** docs/assets, UI visual, UI interaction/state, general logic, DSP/timeline, real-time audio, concurrency/persistence, build/infrastructure, testing.
2. **Risk:** low, medium or high, considering failure impact and shared state. A visually simple UI parameter touching the audio thread is not automatically low-risk.
3. **Dependencies:** what must be built or verified first, and which tests can be shared.

Group compatible investigation, edits or test setup around logical integration boundaries; do not blindly process a fixed number of checkboxes. Preserve per-task acceptance and upstream task-progress verification.

## Verification matrix

| Risk and examples | During implementation | At meaningful boundary |
| --- | --- | --- |
| Low — docs, layout-only, static assets | Relevant formatting, links, asset or build checks | Consolidated doc/visual review where useful |
| Medium — UI bindings, general logic, state | Focused component/unit tests and affected build | Related integration/regression coverage |
| High — DSP, transport alignment, audio callback, races, atomic writes, restore, identity | Targeted edge, fault, lifetime, boundary, concurrency and recovery tests | Focused native regression for affected paths and specified host checks |

Increase focused coverage when shared interfaces or cross-cutting code affect more paths. Perform tests required by the specification or issue at any explicitly required stage.

## Efficiency without weakened correctness

- Read relevant code and affected focused tests before editing; prefer targeted searches and slices over repetitive full-file dumps.
- Combine related inspections and test preparations when useful. Avoid re-reading unchanged logs absent a new question.
- Run changed/focused tests frequently enough to catch errors early. Re-run a test when affected code, fixtures or environment changed, or new evidence casts doubt on its result.
- Reserve full project suites for the pre-PR gate in `CONTRIBUTING.md`, except for an explicitly required earlier gate. Task completion, integration checkpoints and evidence edits use focused checks.
- Use straightforward edits for simple docs; retain OpenSpec's task-line identity check rather than using brittle fixed line numbers.
- Keep evidence lean: record important commands/results, failures, recovery behaviour, architecture decisions and remaining coverage gaps; avoid copying routine long logs.
- Do not misrepresent fixtures as MSVC/CTest or automated tests as FL Studio validation.
- For Windows builds, use `scripts/build.ps1` and follow the focused CTest guidance in `docs/development/development.md`. Respect the build lock and worktree separation. Do not overlap raw CMake/MSBuild with the wrapper.
- Any multiline PowerShell supplied to the owner must be paste-safe in `& { ... }`, check relevant exit codes and report success only after it is confirmed.
- Do not ask the owner to repeat passing automated checks. Use a single consolidated host smoke test when genuinely necessary; record unverified DPI/host conditions honestly.

## Task completion

A task is complete only when its implementation and applicable acceptance checks are satisfied. Follow vanilla Apply's step-by-step progress updates, guarded checkbox editing and post-task instruction refresh **even if that administrative cost is noticeable**. Pause on blocked state, unexpected scope, unresolved requirements, failing critical tests or conflict between artifacts and code. Do not silently relax a requirement.

At each integration boundary, run focused tests for the affected paths and document meaningful outcomes. Before the PR, apply the `CONTRIBUTING.md` gate and review implemented behaviour against the selected specification.
