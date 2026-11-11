# Contributing to ReverbArise

ReverbArise is a personal project. Contributions, including Codex-generated changes, use a **branch → pull request → owner review** workflow. Do not push changes directly to `main` or merge a PR without the owner's explicit approval.

## Start work

1. Read the assigned GitHub issue and relevant acceptance criteria; consult [AGENTS.md](AGENTS.md) for repository-specific engineering guidance.
2. Check the existing branch and PR before creating anything. **If work is already in progress, continue on that branch** unless asked to separate it. Do not duplicate an existing PR.
3. For new work, branch from up-to-date `main`, using a focused name such as `feat/10-three-mode-foundation`, `fix/1-pattern-playback`, or `docs/contribution-workflow`.
4. Use the existing OpenSpec workflow when the change warrants it. Canonical OpenSpec specifications describe the current implementation; GitHub issues describe intended future behaviour. Do not modify OpenSpec's generated tooling or configuration as part of routine work.

## Make changes

- Keep changes within the issue's approved scope and preserve compatibility and real-time audio safety.
- Commit cohesive increments with clear messages. Avoid unrelated cleanup, broad refactoring or generated build outputs.
- Run focused tests and checks for affected behaviour. Document what was and was not tested, including any FL Studio manual validation still required.
- Keep living technical docs current when implementation changes; don't rewrite archived historical records.
- UI contributions must follow the [UI development standards](docs/development/ui-development-standards.md), including shared control ownership, incremental migration and visual/interaction acceptance.
- Prefer concise plans and summaries, targeted file reads and limited test logs to minimise agent token cost.

## Open a pull request

Run the full Debug and Release CTest suites on the final revision before opening a PR that can affect plugin behaviour, build outputs or test execution. Use `./scripts/build.ps1 -Configuration Debug -Test` and `./scripts/build.ps1 -Configuration Release -Test`. During implementation, use focused checks for affected behaviour and integration paths. Apply an earlier full-suite gate only when explicitly required for the task or a separate release or host-test workflow.

For guidance-only changes, use relevant document checks. When updating an open PR, rerun the full suites before pushing changes that could invalidate the prior results. Reuse passing results after edits that leave the tested inputs intact, such as prose-only updates. Record the tested revision, results and any outstanding checks in the PR.

Open **one PR per coherent change**, targeting `main`. Include:

- Linked GitHub issue and OpenSpec change (when applicable).
- Short summary of behaviour changed, key implementation decisions and any migration or data-safety implications.
- Test results and remaining manual validation or known limitations.
- Screenshots only when useful for a UI change.

This section is authoritative for PR issue references: use `Closes #N` in the PR body for each issue the PR fully resolves, so GitHub closes it automatically when the owner approves and merges the PR into `main`. Use `Refs #N` for partial work that should leave the issue open.

Do not treat creating a PR as permission to merge it. **Leave the PR open for the owner to review and merge.** Address requested changes on the same branch and PR.
Agents must not self-merge or independently close issues; GitHub's automatic closure on owner-approved merge is permitted by the convention above.

If a task starts on `main`, move to a feature branch before making commits. For existing in-flight work, avoid disruptive branch switches or rebases without checking the current state first.
