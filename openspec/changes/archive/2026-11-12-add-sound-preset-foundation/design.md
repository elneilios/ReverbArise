# Design

## Context

`SwellSession` stores future and per-swell IR references and labels. `SwellEngine` already has guarded default and selected-swell import commands, off-thread preparation and one accepted replacement publication point. `LearnGeneration`, `AutomaticGeneration`, `AutomaticTargets` and manual creation consume the accepted default through different entry points. The creative JSON writer emits v11 inside the processor's v8 XML envelope. Export currently derives a composition identity from a projection of that creative serializer.

## Goals / Non-Goals

**Goals**

- Establish one accepted definition and resolved response for each assignment while keeping existing asset and DSP paths usable by current callers.
- Keep restore and replacement transactional. Preserve accepted audio, identities and cache compatibility across migration.

**Non-Goals**

- UI and waveform previews (#191), generator and tempo policy (#192), host parameters, separate history (#185), asset relocation or new cache formats.

## Decisions

1. **Reuse the existing managed asset contract.** Give Sound assignments a stable preset ID, definition version and bounded, typed preset values. Reuse bundled catalogue IDs and an explicit Custom IR definition. Group the existing resolved `Asset` and its accepted label with the assignment or expose them through compatibility accessors during conversion. Use one authoritative response reference per owner. Keep existing IDs, labels and user gain intact. For this slice static definitions add no preset-specific DSP controls. Per-swell Level continues to live in `Shape::gainDb` and the future Level default lives on the session. Avoid duplicating those values in a generic settings map.

2. **Migrate from exact identities.** Use the bundled catalogue's known content identity, including supported historical equivalents only where the bytes can be established. Retain unmatched managed references as Custom IR with their saved label. Keep legacy absent references explicitly unresolved and retain a known missing reference for the ordinary recovery path. Do not use the display name, scan storage, install a replacement asset or force reconvolution during migration. Validate definition versions, parameter types, uniqueness and asset consistency within the existing state and catalogue bounds. Use creative JSON v12 while retaining XML v8 and the independent capture/portable formats if they can carry the accepted creative payload without semantic change.

3. **Keep defaults coherent at creation.** Pin the assignment and finite Learn Level in the existing guarded generation configuration, with a 0 dB default and a −12 to +12 dB domain for the new authoring command. Invalidate obsolete Generate/Stop intent on an accepted default edit without rerunning detection, changing the historical capture context, or marking current swells dirty. Apply Learn Level to genuinely new IDs through manual insertion and both automatic creation paths. Existing matched IDs preserve gain even when a successful explicit Generate replaces the IR under the current automatic eligibility policy. Reset All continues to use the current default response as an explicit edit and follows its existing gain reset policy. Use the existing non-parameter host dirty notification only for accepted changes.

4. **Extend the replacement transaction.** Have existing bundled/managed/file entry points produce a candidate accepted assignment. Carry it with the existing request and its captured target, workflow/session, input and request generations. Reuse #183's retained contribution, timing intent fit and bounded worker/render validation. Publish the assignment, render and effective shape together under the existing ownership checks. Keep state save, Song, Audition, offline and Export readers on coherent accepted snapshots. A commit can later be exposed as one undoable edit without creating history now. Preserve existing freeze and cheap gain/shape paths and support newer eligible requests through the existing supersession rules.

5. **Keep identities tied to effective render inputs.** Full convolution and portable dependency discovery continue to use the resolved managed reference, source and existing render inputs. Retain portable enumeration of all exact dependencies, including unused imports and historical contexts. Preserve the current export composition identity when only catalogue metadata is added by explicitly projecting the same audio-relevant serialization fields. Preserve prior complete export history and immutable output. A genuine audio dependency change still changes its existing render or composition identity.

## Risks / Trade-offs

- Misclassifying a legacy response could change its sound. Require exact identity matching, preserve the saved reference, and verify bundled, custom, missing and frozen fixtures.
- Updating only one creation or replacement path could leave metadata and audio out of sync. Exercise manual insertion, automatic creation/reconciliation, Generate, Stop, initial default installation, Reset All, restore and replacement through their normal entry points.
- Save or offline work could observe mixed revisions. Keep candidate state ephemeral and use existing guarded publication and snapshot boundaries.
- Creative codec changes can affect export hashing, portability readers and state size. Exercise existing composition identities and bounded-state and portable fixtures before marking the PR ready.

## Migration Plan

Accept supported creative v1–11 on read and emit v12 only after successful validation. Test fresh, migrated and current round trips using established project fixtures. Keep older project files and managed assets intact. Use the pre-upgrade project and binary for rollback if the old reader rejects a newly saved v12 project.
