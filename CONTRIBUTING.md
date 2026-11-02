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
- Prefer concise plans and summaries, targeted file reads and limited test logs to minimise agent token cost.

## Open a pull request

Open **one PR per coherent change**, targeting `main`. Include:

- Linked GitHub issue and OpenSpec change (when applicable).
- Short summary of behaviour changed, key implementation decisions and any migration or data-safety implications.
- Test results and remaining manual validation or known limitations.
- Screenshots only when useful for a UI change.

Do not treat creating a PR as permission to merge it. **Leave the PR open for the owner to review and merge.** Address requested changes on the same branch and PR.

If a task starts on `main`, move to a feature branch before making commits. For existing in-flight work, avoid disruptive branch switches or rebases without checking the current state first.
