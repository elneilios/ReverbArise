# Tasks

## 1. Shared controls and native review

- [x] 1.1 Extend the shared switch and add the compact checkbox form using the same state policy. Verify on/off, hover, held, disabled on/off, independent focus brackets and bounded indicator ink in component tests.
- [x] 1.2 Verify one Tab stop and one guarded activation through the label, indicator, Space and accessibility, plus existing Return behavior. Cover pointer cancellation, explicit accessibility focus, disabled pre-invocation hooks and silent accepted-state refresh. Document the shared interaction policy.
- [x] 1.3 Build one JUCE preview showing a Learn row, Settings switches and inline inspector checkboxes, including long labels and unavailable state. Open it for owner approval before adopting either form in production. Check 900 × 600, a larger supported size and 100%/125% UI scale. Record the approved geometry, style revision and verification limits.

## 2. Production adoption

- [x] 2.1 Adopt the shared switch presentation for the three Learn controls. Verify clickable labels, capture locks, pass replacement, and the existing allowed turn-off path with the Learn pass and editor checks. Record L4 in the inventory.
- [x] 2.2 Adopt the inline checkbox for Enabled, Solo and Freeze. Verify all three fit in one row with contained brackets and fixed Reset at compact and scaled sizes. Exercise the real selection, session, readiness, retained render and audition paths. Record P4 in the inventory.
- [x] 2.3 Adopt the six S4 switches in Settings. Verify layout, labels, existing live routing and playback, mapping, mute, both audition isolation options and the existing test tone guard in the editor checks. Include wet-only audition with a retained disabled dry-isolation setting. Record S4 in the inventory.
- [x] 2.4 Adopt the two S5 Processing switches. Verify automatic processing and auto-refresh preserve invocation hooks, stale pass rejection and silent reconciliation. Record S5 and the hidden D3 compatibility dispositions, and verify the exact-coordinate and retained Learn controls still work where exercised.
- [x] 2.5 Review the adopted user flows against the offline guide and record currency. Update affected instructions only if the guide no longer matches the controls. Verify with `scripts/check-user-guide.ps1`.

## 3. Integration

- [x] 3.1 Run the Windows Release wrapper with `-Test`, the UI standards guard, strict OpenSpec change validation and whitespace checks. Reconcile the integrated evidence with the approved fixture, record actual DPI and FL Studio coverage separately, and prepare a review PR with `Refs #107`.
