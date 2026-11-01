# transport-swell-playback Specification

## Purpose
Play verified prepared swells against the host Song timeline so users hear their reverse reverb in context with live source audio during normal playback.

## Requirements

### Requirement: Minor performance variations preserve the accepted baseline
A saved change tolerance distinct from marker sensitivity SHALL allow minor performance variation to retain the committed capture and prepared swells. Comparisons SHALL use the retained baseline rather than accumulate pass-to-pass drift. Larger level, spectral or event changes SHALL commit and refresh dependencies. Exact comparison and explicit scoped recapture SHALL remain available; tolerance SHALL NOT fill gaps or change live dry.

#### Scenario: Small variation and accumulated drift
- **WHEN** successive performances vary within the configured tolerance and later depart materially from the retained baseline
- **THEN** small variations do not publish new captures or renders, while the material departure commits and refreshes affected swells

### Requirement: Host offline bounce prepares all required wet before mixing
The host-marked non-real-time path SHALL render a fixed snapshot at absolute Song positions using bounded synchronous pages, without relying on realtime prefetch or capturing export input. It SHALL export available contributions and warn about skipped swells. Fatal failures SHALL remain visible. Offline preparation MAY allocate, read and wait; live callback restrictions SHALL remain unchanged.

#### Scenario: Fast cold bounce
- **WHEN** the host renders a prepared project without sleeps, from a cold position, using irregular blocks and either Playing flag value
- **THEN** every eligible wet contribution matches the positioned reference and the selected dry routing, without capture or definition churn

### Requirement: Completed passes bound untouched automatic tails
An untouched automatic source window extending past an explicit loop end or observed closed backward pass SHALL end at that boundary when capture does not extend beyond it. Its unedited default wet overlap SHALL fit there too. Manual edits SHALL remain unchanged. Growing capture or gaps alone SHALL NOT imply a completed pass. Later genuine contiguous coverage beyond an inferred boundary SHALL permit the full automatic window again.

#### Scenario: Final attack at a completed loop end
- **WHEN** the final automatic attack has a valid IR and contiguous captured input through a completed loop/pass boundary shorter than its default extraction tail
- **THEN** it renders using that real input and a fitting default overlap, without padding missing samples or waiting forever for a tail beyond the completed pass

#### Scenario: Partial capture and protected edits
- **WHEN** a growing pass or uncaptured gap lacks a completed boundary, or a swell has manual source/shape/placement edits
- **THEN** the full defined interval remains required and protected edits are not shortened

### Requirement: Prepared wet follows absolute Song time
While live playback is enabled and valid host Song transport is playing, the plugin SHALL mix prepared wet at its positioned absolute sample interval. Each block SHALL read the intersecting samples at their Song-relative offsets, honoring placement and shape without retriggering at block boundaries. No audio before the host starting position SHALL be replayed retroactively.

#### Scenario: Start inside a swell
- **WHEN** transport starts halfway through a resident prepared swell
- **THEN** its wet contribution joins at that absolute offset with the disclosed admission ramp, not at the beginning of the render

#### Scenario: Block boundaries and negative lead-in
- **WHEN** blocks of varying sizes cross event starts, ends or song sample zero
- **THEN** samples match the positioned reference, with exclusive interval ends and no missing or repeated samples; negative lead-in is heard only if the host actually processes those positions

### Requirement: Playback combines eligible overlapping swells
Playback SHALL include all current, enabled, unmuted swells intersecting the block and honor session solo filtering. Selection and manual audition isolation SHALL not change live membership. Each swell SHALL retain its IR, shape, gain, placement and wet gap/overlap; the original source arrival SHALL remain fixed.

#### Scenario: Hear neighbouring swells
- **WHEN** two eligible swells overlap and a different list item is selected
- **THEN** both contributions are heard at their actual offsets, independent of selection and Isolate selected

#### Scenario: Mute and solo
- **WHEN** a swell is disabled or muted, or excluded by an active eligible solo
- **THEN** its wet contribution is excluded using a short transition without moving any source event

### Requirement: Live routing is independent of manual monitoring
The plugin SHALL expose saved live playback enable, Dry + Wet/Wet Only routing and wet level controls, defaulting to enabled Dry + Wet and 0 dB user wet gain. Dry + Wet SHALL pass live dry unchanged and add only prepared wet. Wet Only SHALL suppress dry only while live playback is enabled and Song transport is playing. Disabled or stopped live playback SHALL preserve dry passthrough.

#### Scenario: Play in context
- **WHEN** a prepared project plays in default Dry + Wet mode
- **THEN** output is the live input plus the protected positioned wet, with no captured-dry replay, audition blend or audition trim applied

#### Scenario: Route wet only and disable
- **WHEN** the user selects Wet Only during Song playback and subsequently disables live playback
- **THEN** enabled Wet Only outputs only prepared wet; disabling restores live dry and removes wet with disclosed short routing ramps

### Requirement: Wet level protection is transparent
Playback SHALL protect the aggregate wet branch to a -6 dBFS ceiling at 0 dB user gain, without boosting quiet audio or modifying stored renders. Protection SHALL use a disclosed fixed gain for each published schedule revision, preserving relative swell gains without block-dependent pumping. User wet level SHALL range from -60 to 0 dB and ramp over 5 ms. The ceiling SHALL not be claimed for dry plus wet output.

#### Scenario: Loud overlapping IR results
- **WHEN** several loud prepared swells overlap
- **THEN** their protected aggregate stays within the wet ceiling, relative contributions remain intact, and attenuation is visible separately from user gain

### Requirement: Transport discontinuities do not replay old positions
Seeking and looping SHALL abandon the old wet position and join prepared audio at the new absolute offset. Stopping, disabling or losing valid timing SHALL stop wet with a bounded click-prevention transition. Old-position audio SHALL not continue beyond that transition. Host transport SHALL never be changed by plugin playback controls.

#### Scenario: Loop through a swell
- **WHEN** a warm loop wraps into the middle of a swell
- **THEN** playback joins its loop destination offset and neither restarts the render nor continues the previous cursor

#### Scenario: Stop on nonzero audio
- **WHEN** Song transport stops on a nonzero wet sample while processing continues
- **THEN** wet reaches zero through a disclosed ramp no longer than 5 ms and does not restart until a subsequent valid Play

### Requirement: Timing incompatibility is explicit
Live wet SHALL require valid absolute host sample position and a compatible fixed sample-rate, tempo, meter and sample-to-PPQ mapping. Unsupported or changed timing SHALL suppress affected wet and identify the reason, without reinterpreting assets or altering stored edits. Returning to compatible timing SHALL require fresh validation before wet resumes.

#### Scenario: Change tempo or sample rate
- **WHEN** the host mapping becomes incompatible with prepared assets
- **THEN** those swells are unavailable for live playback and the workspace explains the incompatibility; default Dry + Wet continues passing dry

### Requirement: Preparation is bounded and unavailable audio is visible
Playback preparation SHALL remain inside the displayed 256 MiB PCM working budget, including pinned pages and concurrent render/audition work, without requiring the whole song resident. Only prepared samples SHALL play. A cold seek or resource miss SHALL report preparation or unavailable coverage, omit missing wet and join at the current position when ready, rather than replaying missed audio.

#### Scenario: Warm and cold seeks
- **WHEN** the host seeks first into a prepared window and then outside resident playback coverage
- **THEN** the warm destination plays immediately; the cold destination reports its miss, preserves the chosen dry routing and admits newly prepared wet at the then-current offset

#### Scenario: Resource pressure
- **WHEN** long-song assets exceed resident capacity
- **THEN** preparation remains bounded, pinned audio remains valid, and unavailable portions are reported rather than silently truncated or substituted with stale audio

### Requirement: The callback performs only bounded prepared mixing
The audio callback SHALL perform no file I/O, convolution, allocation, deallocation, blocking wait or mutex acquisition for live playback. Publication, prefetch and retirement SHALL occur outside it. Mixing work SHALL have a documented admission limit; exceeding it SHALL report unavailable playback rather than overrunning callback resources or dropping unreported members.

#### Scenario: Worker delay during playback
- **WHEN** the preparation worker is delayed during active playback and edits
- **THEN** the callback continues with valid pinned data or explicit missing coverage, with no waiting, resource lifetime violation or unbounded work

### Requirement: Playback validity tracks actual dependencies
Only verified current or explicitly frozen wet SHALL enter live playback. Changed unfrozen dependencies, shape, placement or mapping SHALL invalidate affected publications; cancelled generations SHALL not publish. Identical/unrelated capture SHALL preserve output. Manual missing-source recovery SHALL remain ineligible unless explicitly frozen while ready; frozen assets SHALL follow the render-freeze requirement.

#### Scenario: Recapture while listening
- **WHEN** capture commits identical source followed by changed source inside one swell's dependencies
- **THEN** identical capture does not disrupt it; changed capture removes its outdated contribution until a complete current replacement is ready, while unaffected swells remain playable

#### Scenario: Saved asset cannot verify
- **WHEN** reopen finds missing or corrupt source, IR or wet assets
- **THEN** affected wet remains excluded with an actionable diagnostic and unaffected verified swells remain eligible

### Requirement: Live edits publish without unsafe boundaries
Complete playback revisions SHALL become active at processing boundaries with short, disclosed transitions of at most 5 ms. Changes SHALL preserve absolute timing and existing fade/tension/placement semantics; partial results SHALL never play. Preparing new audio SHALL not play already elapsed lead-in or reset the whole Song cursor.

#### Scenario: Move a swell during playback
- **WHEN** a wet body is moved and its revised preparation completes
- **THEN** the new revision uses its new absolute interval with a bounded transition, without shifting dry input or restarting unrelated swells

### Requirement: Capture excludes all generated output
Mapping and scoped recapture SHALL read the original plugin input before dry routing, live wet, manual audition and diagnostic tone. Enabling playback or choosing Wet Only SHALL not change captured source samples.

#### Scenario: Wet plays over silent input
- **WHEN** mapping runs with zero dry input while prepared wet is audible
- **THEN** committed samples remain zero and the generated wet never becomes a new detection or convolution source

### Requirement: Workspace distinguishes live and manual playback
The workspace SHALL show live controls, host playhead and live readiness separately from manual Audition. Enabled Song playback SHALL stop and inhibit manual audition while playing; disabling live playback SHALL retain independent manual preview behavior. Stopping Song playback SHALL not automatically start audition. Preparation alone SHALL never request host Play.

#### Scenario: Start Song during audition
- **WHEN** live playback is enabled and the host starts while manual audition is active
- **THEN** manual output transitions out within 5 ms and Song-positioned wet takes priority without sustained duplicate output; the UI identifies the active path

### Requirement: Live preferences restore without starting voices
Live enable, routing and wet level SHALL persist independently of audition preferences in bounded versioned metadata, with explicit migration from existing projects. Restore SHALL clear active voices and require fresh host timing and validated assets. Restoring state SHALL not request host transport or resume manual audition.

#### Scenario: Reopen an older project
- **WHEN** a project without live playback preferences is restored
- **THEN** enabled Dry + Wet with 0 dB user wet gain is selected, existing swell edits and audition preferences are preserved, and wet waits for valid playing transport and preparation

### Requirement: Adjustable regeneration and per-swell render freeze
The system SHALL save independent Regeneration sensitivity and per-swell Freeze render preferences. Freeze SHALL require ready committed wet and retain verified audio across source changes, capture clear and reopen, at its saved rate/tempo/meter. Frozen shape/gain/placement/activity SHALL remain editable. Source/IR/support changes SHALL require unfreeze and current dependency validation.

#### Scenario: Keep an accepted performance fixed
- **WHEN** the user freezes a ready swell and subsequent source capture differs or is unavailable
- **THEN** live playback and offline export use its verified saved wet independently of current source and IR availability
- **AND** captured Dry + Wet audition still requires available dry context

#### Scenario: Separate detection and regeneration controls
- **WHEN** the user reduces Regeneration sensitivity
- **THEN** more performance variation is tolerated against the retained baseline without changing Marker sensitivity or explicit exact recapture
- **AND** both sensitivity preferences and frozen choices survive save/reopen

#### Scenario: Unfreeze an accepted render
- **WHEN** the user unfreezes a swell after its source changes
- **THEN** the system validates current dependencies and regenerates as needed
- **AND** source, IR and render-support editing become available again

### Requirement: Loop rounding and export instance isolation
Completed-pass fitting SHALL tolerate at most one captured sample beyond a loop end due to host rounding, while preserving manual/frozen edits and recognizing genuine later extension. Explicit loop bounds SHALL take precedence over inferred wrap bounds. Offline snapshots, output and errors SHALL remain independent for every plugin instance. Skipped assets SHALL identify their instance and reason visibly.

#### Scenario: Fractional tempo closes a pass
- **WHEN** capture ends one rounding sample beyond an advertised fractional-tempo loop end
- **THEN** the untouched final automatic swell uses genuine source through the loop boundary and becomes ready

#### Scenario: Two mixer tracks export independently
- **WHEN** two instances receive interleaved or parallel offline callbacks with different prepared source and IR settings
- **THEN** each outputs its own complete positioned wet without sharing playback state
- **AND** a required-asset failure on one instance remains explicit and does not suppress the other instance

#### Scenario: One waiting swell does not silence its neighbours
- **WHEN** one eligible swell is waiting for source or has an unavailable asset during export
- **THEN** all other available contributions render with the selected dry routing
- **AND** a prominent warning names the skipped swell and reason, without classifying the entire instance as failed

### Requirement: Offline tail timing does not invalidate completed wet
After all admitted wet intervals have ended, an offline host PPQ-origin reset with unchanged valid rate/tempo/meter SHALL preserve prior export results and selected dry routing. It SHALL NOT report completed wet as incomplete solely because of that reset. Mapping changes while wet remains required SHALL still fail explicitly, with sample/PPQ context.

#### Scenario: PPQ resets after the last swell
- **WHEN** offline callbacks advance beyond every prepared wet endpoint but PPQ freezes or resets
- **THEN** completed wet retains its successful or skipped-member status and Dry + Wet passes dry while Wet Only outputs silence

#### Scenario: PPQ changes during required wet
- **WHEN** the host mapping changes while an admitted swell is current or still due
- **THEN** the export error remains visible with timing context rather than claiming success

### Requirement: Audition position and dry span are explicit
The workspace SHALL display a distinct playhead at actual audition sample position in detail and overview, independent of host transport. It SHALL disappear after preview stops. A saved optional Isolate dry source control SHALL limit captured dry replay to the selected source interval while retaining contextual mode, listening duration, wet membership and live dry passthrough. Existing sessions SHALL retain contextual dry.

#### Scenario: Audition a selected transition
- **WHEN** audition advances with DAW transport stopped
- **THEN** its playhead follows the prepared preview and identifies the dry replay span
- **AND** isolated dry contains no replay outside the selected source interval

### Requirement: Envelope controls preserve time alignment
Start/End handles SHALL stay on the waveform centre line at their exact times. Fade/tension controls SHALL remain on the curve and stack in paint order at low zoom, with topmost hit priority. Only genuine coincidences in both time and amplitude SHALL receive a small offset. Zooming SHALL separate distinct times without editing envelope metadata.

#### Scenario: Full fade coincides with endpoint
- **WHEN** a fade or tension control genuinely shares a sample-coordinate anchor with another control
- **THEN** controls remain separately grabbable and their relationship to the actual curve stays clear

#### Scenario: Zoomed-out controls overlap
- **WHEN** distinct controls become close or overlap only because of zoom
- **THEN** they stay on the actual curve with topmost hit priority and separate naturally on zoom-in
- **AND** no long collision connector or arbitrary repacking moves them away

#### Scenario: Fade edits do not move endpoint handles vertically
- **WHEN** either fade is zero, full length or edited
- **THEN** Start/End stay at the waveform centre line while fade/tension controls follow the curve
- **AND** zero fades at unity naturally separate from the endpoint squares without an artificial offset
