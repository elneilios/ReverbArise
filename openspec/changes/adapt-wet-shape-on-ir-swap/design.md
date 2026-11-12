# Design

## Context

`renderReverseConvolution` produces an unplaced interval from `sourceStart - (preparedIrFrames - 1)` through the exclusive render input end. A shorter IR moves the available start later. `SwellEngine::renderJob` currently rejects the new render when `shapeBounds` fails and `retainedIR` has an entry. The current retention check also compares the complete shape, so writing the fitted shape into the accepted swell early can invalidate the old contribution.

`importJob` currently writes the requested IR to the model before preparation. `savedSession`, live playback, internal Export and host bounce can observe the request through different paths. A successful file import alone is insufficient evidence that the swell can play the new response.

## Goals / Non-Goals

**Goals**

- Keep one coherent accepted definition throughout selection, rendering, audition, save and export.
- Fit the response-dependent shape in sample coordinates and keep the existing absolute arrival and placement.
- Reuse the worker, revision checks, asset storage and audio handover. Use the current UI control library.

**Non-Goals**

- General-purpose swell edit history, per-IR preset banks, new IR formats, host automation changes and a new error or control family.

## Decisions

### Fit in the render's sample coordinates

Keep arrival-relative timing intent separately from the accepted effective timing where the two differ. Resolve automatic start against the candidate's raw start. Resolve explicit offsets against the existing arrival before adding placement. Intersect the requested half-open interval with the verified raw interval. Keep any endpoint still supported. If the intersection is empty, use the full verified interval. Use checked sample arithmetic and validate the result through the shared shape contract.

Keep compatible fade lengths, including intentional zero-length fades. For an affected fade, cap its length at the fitted span minus one sample so the release fits before the exclusive end. Existing envelope overlap, curve types, tensions and wet gain still apply. This endpoint-first policy preserves timing that still fits. Proportional rescaling of the whole shape would also move valid handles, and resetting would discard the curve. A valid one-sample span has zero-length fades.

Derive each swap from the remembered intent to avoid cumulative loss across rapid browsing. An explicit edit replaces the intent of the affected timing control. Treat commands that replace the whole shape, including Wet Reset and Full response, as deliberate new intent. Keep normal support-extension behavior for explicit edits. Render candidate support from the accepted source interval and verified usable support, with existing coverage, phrase and resource guards. Use an intended end beyond the source interval only if that support can actually be prepared. If the base interval is available, an unsupported extension inherited from the shape can be fitted to verified support. If required source coverage is missing, report the source problem. Never extend stored capture or synthesize missing dry input to make an IR selection succeed.

### Stage replacement until the audio is ready

Keep the requested IR, candidate shape, render asset and gain preparation in request-owned worker state. Use the same route for bundled, managed and new file selections, including targets with no previously retained live voice. Guard commits with swell identity, current request and edit generations, session, freeze, capture and timing checks. Check supersession before registering a new choice or updating UI status. A request that fails after decode leaves the accepted IR, shape, render and history in place.

Evaluate `retainedCompatible` against the accepted definition during preparation. Pass the candidate effective shape through waveform preparation, calibration, headroom and live-window preparation. Stage any headroom changes required by existing mix policy with the replacement. At handover, publish a coherent revision with the existing bounded transition at the current Song position. If there is no active live window, complete acceptance when the candidate is verified and ready for the next applicable consumer. Reuse current Loop eligibility and freeze rules. Keep preparation, validation, caching and retirement off the audio callback. Preserve the existing memory ceiling.

While manual Audition is active, prepare a complete replacement monitor mix off the audio thread with the same audition mode, activity, overlap and dry-context rules. Publish it at acceptance using the audition voice's bounded crossfade and current absolute cursor. If the cursor is outside the shorter listening window, crossfade to the corresponding wrapped position in the new window. Keep at most the previous and current monitor mix pinned during the transition and count both against the budget. Failed or superseded work leaves the current mix playing. Stop, selection and incompatible dependency changes still take precedence. A replacement completed while stopped never starts Audition.

A bounce already underway keeps its entry snapshot. Internal Export uses an accepted composition and discloses a pending newer request if it offers the previous composition.

### Record scoped undo and redo at acceptance

Store bounded per-swell history for successful replacements. Each record carries the two IR identities, intended and effective timing, and the matching immutable render references. Restore through the same preparation and validation path. Advance history only after successful publication. Apply the IR and timing fields to the current swell so unrelated gain, placement and activity edits survive. An explicit later timing edit invalidates conflicting swap history for that swell. A new successful replacement clears its redo branch. An undo request during preparation supersedes the pending choice before traversing any available accepted history. Frozen swells reject swap history actions with the existing unfreeze guidance.

Keep history for the current editing session and protect its referenced managed assets for its lifetime. Save the accepted definition and recoverable intent so longer responses can restore timing after reopen. This gives the required single-step undo without introducing a plugin-wide history contract or putting render audio in undo records.

### Bind feedback to the same request and accepted revision

Reuse Perform's shared IR selector, local help and NoticeBar roles, list status, waveform and inspector bindings. Use an existing action-menu row or shared command button for scoped undo and redo, with keyboard and accessible invocation. Review placement, labels and interaction with the owner using a native preview. No new visual token or control variant is planned.

Use a concise informational message such as “Wet timing adjusted to fit this IR” for a successful fit. For failures, report the request, specific cause and supported next action. For example, “This IR exceeds the render memory limit. Try a shorter response. Previous IR still available.” Show the final sentence only when true. Keep detailed diagnostics accessible when the primary copy must be short. Use existing feedback severity and precedence. Distinguish a decode error, unavailable source and exhausted render resources. Suppress obsolete results and clear resolved feedback.

## Risks / Trade-offs

- Prematurely replacing shape can revoke retained audio or serialize a mixed revision. Keep the candidate private until all relevant acceptance checks pass.
- Preserving both shapes adds session fields and requires validation of arithmetic and malformed state. Store only the needed timing intent and use the existing parser's compatibility checks.
- Restoring history can fail if assets or source are missing or the swell is frozen. Leave the accepted result and history position in place and give the appropriate recovery.
- A 46-swell host project may expose real preparation limits beyond native fixtures. Keep the reported project intact and test using a working copy with access to its capture and IR dependencies.

## Migration Plan

Load earlier project state with its accepted saved shape as initial intent. Validate new optional timing data and preserve the valid effective shape if optional data is unusable. Check the existing state readers before choosing the compatible encoding. Preserve managed IR, source and render identities. Keep a baseline copy of the FL Studio fixture and its required assets for reopening with the prior build.
