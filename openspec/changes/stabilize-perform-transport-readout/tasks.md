# Tasks

GitHub issue: #68. Planning only until a fresh user request authorizes Apply.

## 1. Read-only component and native review

- [x] 1.1 Add the focused TransportReadout control and independent presentation fields, reusing embedded typography/ink/spacing; verify native component tests prove identical field, separator and group geometry across bar 9/10 and 99/100, fractional beat carry, seeks, integral/fractional BPM, meter changes and independent unavailable states.
- [x] 1.2 Add measured text-fit, accessibility/help and bounded overflow coverage for the design's normal display envelope and extreme values; verify minimum/larger supported logical sizes and software 100/125/150/200% renders retain font size, full inspectable text and containment without overlap. Document the sizing/overflow policy with the component.
- [ ] 1.3 Prepare and directly launch one native preview with changing/unavailable values at minimum size, then record explicit owner look/feel acceptance before production rollout; verify the review record identifies any scoped token/variant exception, rationale and approval, retaining the open review window until the owner finishes.

## 2. Approved editor integration

- [ ] 2.1 After 1.3 acceptance, replace the editor's combined hostInfo label/setup with the reviewed component bound to existing snapshots; extend native editor/readability tests to verify correct values/availability, stable geometry through refresh and mode return, exact-position context help, and unchanged saved session, capture revision and render counts.
- [ ] 2.2 Update Perform guidance and production style/UI standards for the accepted readout policy and any required standards revision alongside tests; run scripts/check-ui-standards.ps1 without regenerating debt inventory, and use maintain-user-guide to record implementation/guide currency with scripts/check-user-guide.ps1 passing.

## 3. Integrated validation and review delivery

- [ ] 3.1 Run scripts/build.ps1 -Configuration Release -Test using the supported wrapper and preserve native MSVC/CTest results; validate the change strictly and run git diff --check. Record commands, revision, environment, size/scale coverage and explicit untested host/DPI limitations in dated docs/validation evidence; request only a brief final FL smoke if concrete unresolved host behavior remains.
- [ ] 3.2 Commit the issue-scoped implementation on its branch and open or update the single review PR with the issue/change links, owner acceptance, verification and remaining limitations; verify the PR targets main, uses Closes #68 only for full delivery, and remains open for owner review.

## Workflow follow-up

- Sync/archive only on a separate owner request using the vanilla OpenSpec workflows; do not self-merge or independently close the issue.
