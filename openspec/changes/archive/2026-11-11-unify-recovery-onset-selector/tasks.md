# Tasks

## 1. Approval and native verification

- [x] 1.1 Verify the Recovery selector through automated native checks using the existing shared-selector fixtures and the integrated Settings page.
- [x] 1.2 Record the owner's approval to reuse the existing dropdown design for this migration and proceed without a separate preview checkpoint.

## 2. Recovery adoption

- [x] 2.1 Adopt `ChoiceSelector` in Settings Recovery. Verify accessible name and empty guidance, wheel scroll routing, body alignment, focus brackets, and keyboard/pointer commit and dismissal using focused native component checks.
- [x] 2.2 Bind selection and refresh by sample identity. Verify that unchanged refresh leaves an open popup usable, retained choices survive reorder or viewport changes, the existing fallback and source-lane selection remain correct, and stale or destroyed invocations cannot select a replacement sample. Verify that only the existing Confirm path issues the confirmation command.
- [x] 2.3 Update the C2 contract and inventory disposition, including the approved preview evidence and the preserved Recovery operation boundary. Review user-guide currency and run the UI standards check.

## 3. Pre-PR integration

- [x] 3.1 Run the repository’s Release build and test gate, strict change validation and whitespace checks. Record native results and any host or physical DPI coverage limits in the PR referencing #107.

## Workflow follow-up

- After owner approval, sync and archive this change before the authorized merge.
- Reconcile #107’s remaining inventory and acceptance against merged main before closing the umbrella.
