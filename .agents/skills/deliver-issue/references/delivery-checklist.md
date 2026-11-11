# Delivery checklist

Follow `CONTRIBUTING.md`, existing review conventions and all applicable upstream OpenSpec stop conditions.

## Before implementation

- [ ] Issue scope, comments, acceptance criteria and dependencies have been read.
- [ ] Current branch/worktree and related open PRs checked to avoid duplicate work.
- [ ] Relevant active OpenSpec change selected or routed through vanilla planning.
- [ ] Any required new user request after OpenSpec Propose has been received.
- [ ] Branch is issue-specific, based on the appropriate up-to-date work, without destructive rebases or unrelated modifications.

## Before opening a draft PR

- [ ] Implementation is finished and owner decisions and manual acceptance are recorded. Test-dependent tasks remain pending.
- [ ] Vanilla Apply task-line validation and post-checkbox progress confirmation were honoured.
- [ ] PR records automated validation as pending and identifies any manual host checks performed.
- [ ] Data safety, compatibility, threading and plugin-state risks were reviewed where applicable.
- [ ] Living docs reflect implemented changes; dated evidence preserved; no unrelated generated output.
- [ ] Outstanding risks, manual validation and unresolved work are explicitly stated.
- [ ] Diff is issue-scoped and reviewable; commits have informative messages.

## Before marking the draft ready

- [ ] Review findings are addressed and the owner approves moving the draft to ready for review.
- [ ] Targeted checks for the final diff and the standard PR regression set pass. Commands, configurations, tested revision and results are recorded.
- [ ] Native MSVC/CTest results, fixtures and host/FL Studio checks are clearly distinguished.
- [ ] Keep the PR in draft on failure and refresh validation if tested inputs change before merge.

## PR body

Include:
1. **Issue:** link the issue(s) using the authoritative PR issue-reference convention in `CONTRIBUTING.md`.
2. **OpenSpec:** link/name the selected change and its status, when applicable.
3. **Summary:** what changed and why, including important design/compatibility choices.
4. **Verification:** exact relevant checks and outcomes; note tests not run.
5. **Review notes:** remaining risks, screenshots for UI when useful, and any owner-only host checks.

Open or update **one** PR targeting `main` and leave it open for owner merge. Apply the release packaging gate when requested by the owner.

## After completed delivery

- [ ] Apply [completed-work cleanup](completed-work-cleanup.md) to this task's regenerable output after validation, packaging and delivery finish; defer output still needed for acceptance, debugging or an active review.
- [ ] Preserve local changes, releases, rollback bundles, durable assets and validation evidence; verify Git status after cleanup.
- [ ] Report removed paths and measured reclaimed space, or state that no eligible output existed; explain retained output and any blocked cleanup.
