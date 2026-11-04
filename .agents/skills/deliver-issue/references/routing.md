# Issue-to-OpenSpec routing

## Discovery

- Fetch the GitHub issue, relevant comments, linked issues/PRs and acceptance criteria. Establish the intended outcome, compatibility constraints and explicitly excluded scope.
- Inspect active branch/worktree/PR state before creating a branch. Follow `CONTRIBUTING.md` for continuing existing work.
- Search **active OpenSpec changes** for explicit issue links, then semantically relevant changes. Check status and planning scope; do not assume that matching titles indicate ownership.
- If multiple changes could apply, or one spans multiple issues, identify the overlap and select the authoritative change only when supported. Ask before merging, superseding or duplicating scope.
- Review relevant canonical `openspec/specs/` to distinguish implemented behaviour from the requested future behaviour.

## Decision table

| Condition | Route | Constraint |
| --- | --- | --- |
| Existing change covers the issue and is ready to implement | Vanilla Apply | Its tasks/specs govern implementation |
| Existing change covers the issue but is incomplete, blocked or stale | Standard OpenSpec planning/update path | Repair artifacts or resolve conflicts before Apply |
| No change; outcome and technical choices are clear | Vanilla Propose | Stop when proposal artifacts are finished; await a fresh request before Apply |
| No change; important host, architecture, data-safety or product behaviour is unknown | Vanilla Explore, then Propose | Explore does not implement; obey approval requirements for artifact writes |
| Existing change only partly overlaps | Assess adjustment versus a new, separate change | Preserve issue traceability; avoid duplicate or conflicting specs |
| Issue is already covered by an open implementation PR | Continue its branch/PR | Do not create duplicate PRs |

**Complexity means uncertainty, not line count.** A large but well-defined UI change may go directly to Propose; a small race-condition fix may merit Explore. Do not use Explore merely to add ceremony.

## Tracking convention

When authoring *new* OpenSpec proposal artifacts, include an explicit GitHub issue reference (for example `GitHub issue: #11`) in a suitable existing section if the schema permits it. Do not rewrite existing proposals solely to add tracking. Preserve relationships where one change spans multiple issues and cite the relationship in the PR.

## Approval and lifecycle boundaries

- A request to deliver an issue authorises issue discovery and normal branch/PR work, not exceptions to upstream OpenSpec workflow permissions.
- Vanilla Explore requires separate confirmation before writing planning artifacts unless the user has explicitly requested that scoped write.
- Vanilla Propose ends after planning; **a new user request is required to Apply**.
- Do not manufacture an all-in-one Explore → Propose → Apply pipeline that bypasses these phase boundaries.
- When a change is blocked or a material design decision is missing, report the decision needed rather than inventing requirements.
