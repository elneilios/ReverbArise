# Delivery checklist

Follow `CONTRIBUTING.md`, existing review conventions and all applicable upstream OpenSpec stop conditions.

## Before implementation

- [ ] Issue scope, comments, acceptance criteria and dependencies have been read.
- [ ] Current branch/worktree and related open PRs checked to avoid duplicate work.
- [ ] Relevant active OpenSpec change selected or routed through vanilla planning.
- [ ] Any required new user request after OpenSpec Propose has been received.
- [ ] Branch is issue-specific, based on the appropriate up-to-date work, without destructive rebases or unrelated modifications.

## Before PR creation or update

- [ ] Each completed task was implemented and checked against its acceptance criteria.
- [ ] Vanilla Apply task-line validation and post-checkbox progress confirmation were honoured.
- [ ] Relevant focused and integration tests passed; mandatory final gates completed.
- [ ] Native MSVC/CTest results, fixtures and host/FL Studio checks are clearly distinguished.
- [ ] Data safety, compatibility, threading and plugin-state risks were reviewed where applicable.
- [ ] Living docs reflect implemented changes; dated evidence preserved; no unrelated generated output.
- [ ] Outstanding risks, manual validation and unresolved work are explicitly stated.
- [ ] Diff is issue-scoped and reviewable; commits have informative messages.

## PR body

Include:
1. **Issue:** link the issue(s) using the authoritative PR issue-reference convention in `CONTRIBUTING.md`.
2. **OpenSpec:** link/name the selected change and its status, when applicable.
3. **Summary:** what changed and why, including important design/compatibility choices.
4. **Verification:** exact relevant checks and outcomes; note tests not run.
5. **Review notes:** remaining risks, screenshots for UI when useful, and any owner-only host checks.

Open or update **one** PR targeting `main`, leave it open for owner review, and do not self-merge, independently close issues or archive OpenSpec changes.

## After completed delivery

- [ ] Apply [completed-work cleanup](completed-work-cleanup.md) to this task's regenerable output after validation, packaging and delivery finish; defer output still needed for acceptance, debugging or an active review.
- [ ] Preserve local changes, releases, rollback bundles, durable assets and validation evidence; verify Git status after cleanup.
- [ ] Report removed paths and measured reclaimed space, or state that no eligible output existed; explain retained output and any blocked cleanup.
