# Tasks

## 1. Reproduce the activation boundary

- [x] 1.1 Extend the existing processor/fake-playhead harness with managed prepared, identifiable stereo wet; exercise advancing transport with omitted callbacks, release/prepare and framework bypass separately. Verify the baseline symptom or passing outcome for each sequence and identify the first failing clock/admission/preparation gate. Record commands, revision, host/build facts available and synthetic-versus-host limitations in `docs/validation/2026-11-06-issue-103-host-reactivation.md`. If no sequence reproduces, obtain concrete host evidence before changing behavior.

## 2. Repair and prove current-time recovery

- [x] 2.1 Repair only the reproduced runtime lifecycle/admission/preparation fault using the design's existing mechanisms. Verify a regression that fails on baseline and passes after repair for an interior offset and later scheduled swells before wrap, comparing nonconstant stereo output to an independent absolute-position oracle after the bounded transition. Document the demonstrated cause and recovery boundary in `docs/development/perform-live-diagnostics.md`; stop for owner review if a changed timing policy is required.
- [x] 2.2 Extend focused processor/transport coverage for repeated cycles, cold preparation and held publication across reactivation/restore, irregular blocks, seek/loop and independent instances. Verify current-time recovery, no duplicate/elapsed contribution and rejection of obsolete publication; retain existing #73 origin/rate/tempo/meter and invalid-timing checks, internal Live-disable and workflow gating. Verify callback allocation/deallocation guards and source/render asset identities plus saved definitions/preferences remain unchanged; record focused native results in the dated evidence.
- [x] 2.3 Use maintain-user-guide to review delivered behavior and update offline guide currency, adding prose only where needed. Verify `scripts/check-user-guide.ps1` passes and affected documentation links resolve; record the review outcome in the issue evidence.

## 3. Integration and review delivery

- [x] 3.0 Investigate the baseline native UI crash as authorized by the owner on 2026-11-06. Verify stack-reserve-only reproduction against the original executable, apply the demonstrated test-only MSVC stack headroom repair, and confirm the unchanged broad UI test passes through the normal wrapper with the existing renderer, driver and assertions. Record evidence and update local validation guidance; preserve all required gates and production behavior.

- [x] 3.1 Run `scripts/build.ps1 -Configuration Debug -Test` and `scripts/build.ps1 -Configuration Release -Test` sequentially under the wrapper's build lock, plus applicable repository validation gates and strict OpenSpec validation. Verify all required checks pass and record native results separately from fixture checks, with warm/cache state and skipped host coverage explicit.
- [ ] 3.2 Consolidate final host verification into one brief FL Studio smoke of the actual enable/disable control: audible running playback, disable while transport advances, re-enable inside a swell and hear later swells before wrap, including a repeated cycle. Use install-test-vst3 if a test installation is needed, record exact build/control and owner outcome, and avoid asking the owner to repeat automated edge cases. If host coverage is unavailable, record it as unresolved rather than claiming acceptance.
- [ ] 3.3 Review the final scoped diff and evidence, commit on the issue branch and open or update one owner-review PR using `Closes #103` only if fully resolved, otherwise `Refs #103`. Verify the PR links this change and accurately states root cause, validation and remaining limits; do not merge or close the issue independently.

## Workflow follow-up

- Sync/archive only through the vanilla OpenSpec workflows when separately requested after the applicable review gates.
