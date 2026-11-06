# Design

Implementation closeout, 6 November 2026: the owner approved the audit/contracts.
The branch is reconciled to merged #80 at `717c890`; living inventory and contracts
reflect its actual shared IR/icon ownership. #80 consumed standards revisions
8–10, so documentation integration uses revision 11. Initial observations below
record the proposal's historical `24cd287` baseline. Remaining command work is
proposed separately as `unify-command-button-states`; #108/#8 remain independent.

## Context

See `proposal.md` for motivation and `specs/ui-design-system/spec.md` for the additional contracts. This design is required because ownership crosses production controls, workspace composition and auxiliary windows.

Observed baseline: checkout `24cd287`, production standards revision 7. `Source/UI/WorkspaceTheme.h` supplies embedded fonts, semantic tokens and text roles, but its legacy button painter selects behavior through names. `ActionButton`, `IconActionButton` and `LatchingActionButton` independently paint portions of command state; the current icon family changes glyph ink on hover while ordinary commands highlight the border. These are observations, not approval of a new common rendering.

`DeliberateSelector` shares no-wheel selection and invocation capture; Settings also uses plain selectors. `NumericInputControl` supplies slider value entry and step buttons, while `KnobValue` owns a separate exact-entry lifecycle. `PluginSettings.cpp` composes plain labels, help and legacy controls. `EllipsisLabel`, `NoticeBar`, bottom contextual help, `FocusViewport` and `PresentationSurface` already provide reusable mechanisms. `Tests/ControlLibraryCases.h` covers parts of keyboard, disabled, focus, rename invariance and numeric interaction; it is not a complete family state matrix.

The issue describes a #80 shared-icon correction that is absent from this checkout. Treat #80 as a concrete integration dependency for the button slice, not evidence that a fix has landed. Re-read its delivered implementation and approval scope when preparing that slice.

## Goals / Non-Goals

**Goals:** Make actual consumer ownership discoverable, turn existing standards into usable family contracts, and prepare independently deliverable migration packets. Distinguish observed, approved, proposed, legacy and untested behavior. Keep future feature work able to reuse a family without waiting for all migrations.

**Non-Goals:** This umbrella's implementation does not modify production control code, command/model/audio behavior or test infrastructure. It does not introduce new tokens, numerical geometry, a universal configurable widget, or a compulsory whole-plugin conversion. Child changes own their concrete APIs, code, regression tests and scoped approval. Creating GitHub issues, publishing a PR or installing a VST3 is outside this proposal-creation request.

## Decisions

### 1. Keep one umbrella and separate family delivery

The owner selected #107 as the umbrella. Its initial deliverable is the audit, shared contracts and roadmap; smaller child OpenSpec changes implement family migrations. Each child PR uses `Refs #107`, not a premature closure reference. A child can span main and auxiliary consumers of its family while remaining bounded by an explicit consumer list. The umbrella remains open until accepted slices have been delivered or explicitly deferred by the owner with rationale.

Alternative: one all-control implementation change. Rejected because its review and rollback boundaries would combine unrelated interaction risks. Alternative: independent styling tickets without shared contracts. Rejected because each could recreate the same policies differently.

### 2. Use one living inventory and one contract authority

During umbrella Apply, publish:

- `docs/design/ui-control-inventory.md`: consumer inventory, observed state differences, approval/debt disposition and family roadmap.
- `docs/development/ui-control-contracts.md`: family contracts and developer reuse guidance linked from the existing UI standards.
- `docs/validation/YYYY-MM-DD-issue-107-control-audit.md`: actual audit revision, commands/method, evidence references and limitations. Choose the execution date when delivered.

Update `docs/README.md`, the existing production style guide and development standards to link these authorities. Do not copy token tables or historical acceptance narratives into multiple documents. Increment the standards revision when the contracts are owner accepted; candidate contracts remain explicitly proposed until then. Code/token changes and their revision evidence belong to the affected child delivery.

Inventory rows identify consumer/source reference, surface, family/policy owner, semantic role, applicable states, input/accessibility policy, approved reference, concrete difference and disposition/child reference. Group truly identical consumers with an explicit member list; no implicit screen-wide compliance. Include nested editor/stepper controls and dynamically created popup, row and window interactions. Mark state entries not applicable with a reason rather than forcing a slider state onto a label.

Alternative: inventory only control classes. Rejected because plain JUCE consumers and per-screen setup are precisely where drift can hide.

### 3. Define contracts by family and semantic responsibility

Every contract records supported states and precedence, presentation authority, input lifecycle, focus/accessibility, enabled/read-only/unavailable behavior, overflow and refresh rules, extension points, named exceptions and required evidence. Family matrices cover idle/hover/pressed/focus/disabled and selected/latched/editing/invalid/busy only where meaningful; combinations such as focused-and-disabled must have a documented resolution.

Appearance variants use explicit family/semantic ownership. Component names may remain for diagnostics, lookup or test discovery, but must not choose migrated appearance. Primary, destructive and latching semantics remain distinct. This umbrella specifies the contract fields without pre-approving an enum API, painter extraction, color value or revised native look.

For fields, compose the existing input with caption/help/validation presentation rather than replace working numeric mechanisms. Record Return, Escape, focus-loss, active-edit refresh, rejection and exact-value behavior per input type. Bindings provide validation outcomes and text; library presentation owns readable feedback and accessible relationships. Bottom contextual help remains shared; local validation has an explicit relationship to it and notices, without popup tooltip hosts or forced speech. Existing parsing, clamping or rejection behavior cannot silently change to fit a new field wrapper.

Alternative: a single universal control or global identical interaction rule. Rejected because specialized semantics, no-wheel selectors and exact musical entry need distinct, named policies.

### 4. Prepare this family roadmap

The order below is a proposed dependency order, not approval to implement or a demand that later feature work wait. Audit findings can split a large family into bounded consumer groups while retaining one contract authority.

| Slice | Reuse / boundary | Principal proof and approval |
| --- | --- | --- |
| Commands, glyph actions and latches | Existing action families, theme legacy branches, including Settings/Diagnostics; reconcile #80 first | Semantic state rendering, rename/relabel invariance, keyboard/accessibility activation, disabled and invocation guards; owner reviews any revised surface or feel |
| Selectors and menus | Existing deliberate/invocation selectors, IR policy and popup mechanisms | Wheel/keyboard/popup policy, dismissal/focus, unavailable choices and stale invocation rejection |
| Text, help and feedback | Existing text roles, ellipsis, context-help footer and notices | Long text, accessible names/relationships, help/validation precedence, no forced speech; reusable caption/help composition |
| Numeric and text fields | Existing numeric input, slider value fields and KnobValue entry; depends on feedback contract | Exact typing, Return/Escape/focus loss, invalid/rejected edits, active refresh, units and accessible validation; bindings retain domain rules |
| Knobs and sliders | Existing gesture, step and invocation implementations | Coarse/fine drag, wheel, keyboard, reset, exact value agreement and rejection; retain prior scoped knob focus approval |
| Switches and selection toggles | Existing Switch, InvocationToggle and legitimate latch distinctions | Accepted state, disabled/read-only behavior, keyboard and accessible state; no optimistic model changes |
| Navigation, lists and scrolling | Existing segmented selector, swell list, focus viewport and tab mechanisms | Focus reveal, selection/scroll preservation, menus, keyboard access and compact/scaled bounds |
| Specialized surfaces and auxiliary shells | Existing timeline/overview/handle components, presentation surface and owned windows | Domain hit geometry, specialized focus/accessibility, transform/scroll access; native chrome/choosers retain native policy |

An audited family already meeting its contract can be closed with source/test/approval evidence and no code migration. Known debt stays explicit; avoid manufacturing refactors to fill every row.

### 5. Separate evidence by what it proves

Umbrella acceptance checks inventory coverage against actual constructors/member declarations and dynamic control creation; verifies every family has a contract and every difference a disposition; checks links, exact source references and approval status. It does not claim new native or host verification from source inspection.

Each child packet names component/binding tests and existing native fixtures to extend. Applicable tests include state rendering and combined states, keyboard and accessibility activation, disabled interactions, name/label invariance, silent refresh, rejected/stale edits and geometry/long-content cases. Use the repository build wrapper for native MSVC/CTest and run the UI standards guard; never regenerate debt to bypass failures.

Child review covers 900x600 at 100%, a representative intermediate size, 1100x850 and a documented larger size, plus supported 100%/125% user magnification. Settings/Diagnostics retain comfortable typography and use their bounded scroll/focus behavior. Actual Windows 100/125/150/200% DPI and FL Studio coverage are separate pass/fail/unsupported/untested records. Launch a prepared native preview directly for new or revised family look/feel approval before broader adoption. Reuse unchanged scoped approvals and passing automated edge cases; request a final brief host smoke only for changed host-facing behavior or a concrete unresolved host risk.

## Risks / Trade-offs

- [Audit becomes a replacement backlog] -> Define source-level coverage and packet fields now; publish explicit accepted/deferred dispositions rather than generic follow-up tasks.
- [#80 overlaps button work] -> Reconcile the delivered component and approval evidence before selecting child consumers; do not duplicate its correction.
- [A shared wrapper changes commit or async semantics] -> Preserve bindings and invocation capture; require child rejection, session and active-edit tests.
- [Contracts imply unreviewed appearance approval] -> Mark candidates clearly and require scoped native review before adoption; reference current revision 7 and subsequent approved revisions explicitly.
- [Audit becomes stale] -> Each family PR updates its consumer rows, debt and evidence; contracts are referenced rather than copied.
- [Broad migration blocks unrelated features] -> Reuse or extend the affected family within the relevant reviewed child boundary; no whole-plugin prerequisite.

## Migration Plan

1. Apply this umbrella change to publish the full audit, candidate contracts and family packets, then obtain owner review of that concrete documentation and proposed exceptions. No production migration occurs here.
2. After acceptance, propose child changes against the shared contracts; each names exact consumers and its own OpenSpec/owner review gates. Later child changes can update contracts only with explicit scoped review.
3. Deliver and validate each child independently, keeping unmigrated behavior visible. Update living guidance, revision/evidence and umbrella progress together.
4. Finish the umbrella only after its audit acceptance and explicit disposition of all slices. Archive changes through the normal workflow. Roll back a problematic child independently while updating the inventory honestly; never revert saved state, captures or assets as part of a UI rollback.
