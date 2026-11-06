# Tasks

These tasks deliver the umbrella audit, contracts and roadmap only. Production migrations are separate child changes. All tasks remain unchecked until Apply; proposal generation does not complete them.

## 1. Publish the consumer inventory

- [x] 1.1 Create `docs/design/ui-control-inventory.md` with the audit baseline, explicit surface/family coverage and row schema from design decision 2; verify its headings cover Learn, Perform, Export, header, Settings, Diagnostics, specialized interactions and native boundaries.
- [x] 1.2 Populate command, glyph, latch, toggle, selector and menu consumers, including nested and dynamic instances; verify rows against actual declarations/creation paths and record concrete applicable state differences, source references and approval/debt disposition. Reconcile #80's delivered icon scope and evidence before assigning residual button debt.
- [x] 1.3 Populate labels/help/notices, numeric/text entries, knobs/sliders, navigation/lists/scrolling, timeline/overview/handles and auxiliary shell consumers; verify all concrete consumers are covered or explicitly grouped, with compact/scaled and comfortable auxiliary policies distinguished and unknown coverage marked untested.
- [x] 1.4 Record audit method, revision, inspected tests and limitations in dated `docs/validation/YYYY-MM-DD-issue-107-control-audit.md`; verify inventory rows link to existing source/evidence and do not claim native or host checks that were not run.

## 2. Publish shared family contracts

- [x] 2.1 Create `docs/development/ui-control-contracts.md` with explicit command/icon/latch, switch, selector/menu and navigation/list contracts; verify each has applicable state precedence, semantic variants, pointer/keyboard/accessibility behavior, disabled/read-only rules, silent refresh and existing approved presentation references.
- [x] 2.2 Add text/help/notice and numeric/text-field contracts using existing primitives; verify caption/input/help/validation composition, accessible relationships, overflow, feedback precedence and Return/Escape/focus-loss/rejection/active-refresh behavior are specified without changing binding validation, exact values or session authority.
- [x] 2.3 Add knob/slider, specialized surface and auxiliary shell contracts; verify existing gesture/reset/invocation mechanisms, specialized hit geometry, focus reveal and native chooser/window policies have explicit owners and that every inventory family maps to a contract or reasoned native boundary.
- [x] 2.4 Mark observed, approved, proposed and legacy policies distinctly and record all justified differences; verify the contracts do not silently approve new tokens, alter prior accepted knob focus, introduce label/name-based appearance selection or mandate identical interaction across unrelated families.

## 3. Prepare bounded family delivery packets

- [x] 3.1 Add the family roadmap and a delivery packet for each unresolved family or bounded subgroup to the inventory; verify each names exact consumers, reusable controls/policies, dependencies, compatibility guards, proposed exceptions, native approval gates and completion evidence. Families requiring no code change must cite proof instead of invented migration work.
- [x] 3.2 Specify per-packet automated and native verification, reusing existing component/binding fixtures; verify state/combined-state rendering, keyboard/accessibility activation, disabled behavior, rename/relabel invariance, stale/rejected edits and silent refresh are covered where applicable. Include build-wrapper/UI-guard commands and supported minimum/intermediate/larger/user-scale review, separating actual DPI/host results and preserving existing guard debt rules.
- [x] 3.3 Present the concrete inventory, candidate contracts and packets for owner review, record the decision and any scoped exception disposition in dated evidence; verify no proposed visual or interaction variant is labeled production approved without its required native review. Leave this task unchecked while owner review is pending.

## 4. Integrate the accepted umbrella guidance

- [x] 4.1 After umbrella contract acceptance, update `docs/README.md`, UI development standards and production style guide links/revision record together; verify links resolve, numerical authorities are referenced rather than duplicated and all incomplete child migrations remain explicit debt. Record user-guide currency review as no runtime change unless actual findings require a separate scoped follow-up.
- [x] 4.2 Validate the OpenSpec change with `openspec validate audit-ui-control-contracts --strict`, run `scripts/check-ui-standards.ps1`, and check the complete diff for documentation-only scope; verify every inventory row and family packet has a disposition and record commands/results plus skipped native MSVC/CTest/host checks honestly in the audit evidence.

## Workflow follow-up

- Use the existing owner-reviewed PR workflow for the umbrella audit and contracts, with `Refs #107`; do not close the umbrella on this documentation delivery.
- Propose each accepted implementation slice as a separate child OpenSpec change and PR. Creating GitHub child issues or posting updates requires a separate explicit user instruction.
- Archive this audit/contract change through the normal workflow after its own tasks and owner review are complete; subsequent child delivery is tracked by #107 and the living roadmap, not unfinished tasks in this change.
- Keep #107 open until accepted family slices are delivered or the owner explicitly defers remaining work with rationale. Do not self-merge or independently close the issue.
