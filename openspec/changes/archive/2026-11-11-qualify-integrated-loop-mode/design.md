# Design

## Context

Rack fixtures from #133 and #126, the FL Studio probe from #127, and evidence from #128, #129 and #144 provide reusable inputs and expected outcomes. They cover different binaries. Use the latest integrated candidate and record its commit, installed VST3 hash, OS, host and module versions, sample rate, callback size, device, patch or project, routing and transport settings.

## Goals / Non-Goals

**Goals:** Verify the real musician workflow in each host, measure audio and timing where possible, and document the tested routes and limitations. Verify the deferred Windows DPI coverage and obtain one consolidated owner acceptance.

**Non-Goals:** New controls or timing behavior, legacy migration, qualification of additional hosts, publication of a release, and merging the integration branch into main.

## Decisions

1. **Use one installed candidate and disposable project copies.** Install through the existing test VST3 workflow and preserve the rollback bundle. Reuse the Rack patch generator and FL Studio audio/MIDI generator. Capture a fresh source before testing reopen, relocation and verified Restore. Keep originals and record the accepted identity and asset hashes.
2. **Report capture, edit grid, live phase and external commands separately.** In Rack use one pulse per beat and 4/4 for the simple route. Clock alone can supply live timing after a fresh run, and an aligned optional Phrase can set future launches. Test absent timing independently from the legacy Phrase-only mismatch case. In FL Studio use Playlist timing by default, test Pattern repeats and optional MIDI origin and controls, and preserve Song's host timeline requirements. Use MIDI numbers as the common reference.
3. **Separate measurements from listening.** Use a transient fixture for exact boundaries, wraps, placement and tails, then a long multi-swell fixture for physical monitoring, dry/wet balance, peak protection and resource bounds. Compare FL Studio bounce and aligned WAV import against the intended timeline, with event-driven offline limits recorded explicitly. A replay, waveform or status screenshot alone does not establish audible host playback.
4. **Measure real Windows DPI.** In a desktop session at each OS scale, record the Windows setting and host configuration alongside 900 × 600 Song and Loop Learn and Perform screenshots. Keep plug-in software scaling fixed and record it separately. Check text clipping, alignment, pointer hit targets, keyboard focus and interaction after reopening the editor. Record the host for every checked combination and any inability to complete a required check.
5. **Make readiness evidence traceable.** Use passing, failing, unsupported or unverified entries tied to a named test setup. Reuse the approved design and guide structure. Run native gates at the repository's approved readiness stage and the full suite if preparing a release package. Keep readiness open while required evidence or owner acceptance is outstanding.

## Risks / Trade-offs

- Host versions, wrapper routing and physical scaling can affect results. Record actual conditions and refresh affected runs if the candidate changes.
- Clock loss, a seek, stopped processing and an unsupported timeline are different observations. Use separate rows with the expected safe state and recovery action.
- Retain older evidence for diagnosis. Where a case cannot be measured on the integrated build, record the gap explicitly.
