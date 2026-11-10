# transport-swell-playback Specification

## Purpose
Play verified prepared swells against the host Song timeline so users hear their reverse reverb in context with live source audio during normal playback.

## Requirements

### Requirement: Minor performance variations preserve the accepted baseline
Mapped source SHALL remain protected during ordinary Capture and all Live/Finalise playback regardless of performance variation. Only explicit Recapture SHALL permit replacement. Saved regeneration-tolerance values SHALL remain compatible metadata and SHALL NOT authorize implicit source updates; marker sensitivity and live dry SHALL remain independent.

#### Scenario: Small variation and accumulated drift
- **WHEN** repeated upstream performances vary slightly and later change materially with Recapture off
- **THEN** neither variation replaces mapped source or recalibrates its wet baseline
- **AND** the user must return to Capture and explicitly arm Recapture to adopt the changed performance

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
Only in Live, with saved live playback enabled and valid compatible host transport playing, SHALL the plugin mix prepared wet at its absolute positioned sample intervals. Each block SHALL use intersecting Song-relative offsets, honoring placement/shape without retriggering. Capture and Finalise SHALL emit no host-scheduled wet. Missing elapsed lead-in SHALL NOT replay retroactively.

#### Scenario: Start inside a swell
- **WHEN** Live transport starts halfway through a resident prepared swell
- **THEN** its wet joins at that absolute offset with the disclosed admission ramp, not at render start

#### Scenario: Block boundaries and negative lead-in
- **WHEN** Live blocks of varying sizes cross event starts, ends or song sample zero
- **THEN** samples match the positioned reference with exclusive ends and no missing/repeated samples; negative lead-in is heard only if the host processes those positions

#### Scenario: Same timeline in Capture or Finalise
- **WHEN** DAW playback visits a ready swell in Capture or Finalise
- **THEN** no scheduled wet plays even though its verified audio remains retained for later Live use

### Requirement: Playback combines eligible overlapping swells
Playback SHALL include all current, enabled, unmuted swells intersecting the block and honor session solo filtering. Selection and manual audition isolation SHALL not change live membership. Each swell SHALL retain its IR, shape, gain, placement and wet gap/overlap; the original source arrival SHALL remain fixed.

#### Scenario: Hear neighbouring swells
- **WHEN** two eligible swells overlap and a different list item is selected
- **THEN** both contributions are heard at their actual offsets, independent of selection and Isolate selected

#### Scenario: Mute and solo
- **WHEN** a swell is disabled or muted, or excluded by an active eligible solo
- **THEN** its wet contribution is excluded using a short transition without moving any source event

### Requirement: Realtime Dry + Wet output has linked peak protection

Realtime Song and Loop Dry + Wet SHALL use one linked gain on the combined stereo output, with a −1 dBFS sample-peak ceiling when a wet contribution is mixed. The guard SHALL add no latency and return to exactly unity after recovery. Existing wet calibration, including the approved +6 dB target, and existing wet overlap protection SHALL remain independent.

#### Scenario: Dry and wet peaks coincide

- **WHEN** prepared wet and incoming dry sum above the ceiling in active live Dry + Wet
- **THEN** both channels receive the same gain and the emitted sample peaks stay within the ceiling
- **AND** the gain reduction keeps the dry-to-wet ratio at that sample and does not move the swell

#### Scenario: The live sum has headroom

- **WHEN** the summed peak remains below the ceiling and the guard is idle
- **THEN** incoming dry is at unity and prepared wet follows the saved gain and protection plan

#### Scenario: Protection recovers

- **WHEN** a peak subsides during active live playback
- **THEN** guard gain returns to unity without exceeding the ceiling on later peaks
- **AND** the result is independent of audio callback block boundaries

#### Scenario: Dry-only passthrough and other output paths

- **WHEN** there is no active live wet contribution or its stop transition has finished
- **THEN** Dry + Wet passes the incoming dry unchanged and a subsequent live run starts at unity
- **AND** Wet Only, Audition, Learn capture, host offline rendering and internal Export keep their existing protection and timing

#### Scenario: Offline rendering begins during Loop playback

- **WHEN** an offline callback disables Loop playback while wet audio is sounding or its stop fade is in progress
- **THEN** the existing wet stop fade is emitted without realtime output peak protection
- **AND** a realtime callback disabling the same playback retains linked output protection through the wet fade

### Requirement: Live routing is independent of manual monitoring
Within Live, saved enable, Dry + Wet/Wet Only routing and wet level SHALL remain independent of audition, defaulting to enabled Dry + Wet at 0 dB. Dry + Wet SHALL add prepared wet to incoming dry at unity before applying live output peak protection. Wet Only SHALL suppress dry only during enabled valid Live playback. Capture/Finalise SHALL preserve dry passthrough without modifying saved Live preferences.

#### Scenario: Play in context
- **WHEN** a prepared project plays in Live default Dry + Wet
- **THEN** output is the live input plus protected positioned wet, with the linked live output guard acting only when needed and with no captured-dry replay or audition blend/trim

#### Scenario: Route wet only and disable
- **WHEN** Wet Only is selected during Live playback and live playback is subsequently disabled
- **THEN** enabled Wet Only outputs prepared wet and disabling restores dry/removes wet with disclosed short routing ramps

#### Scenario: Enter Finalise from Wet Only
- **WHEN** Finalise is entered from Live with Wet Only selected
- **THEN** scheduled wet stops through the existing bounded transition, normal DAW output becomes dry passthrough and returning to Live restores the saved preference

### Requirement: Wet level protection is transparent
Playback SHALL follow the saved gain policy. Legacy sessions SHALL retain their -6 dBFS fixed aggregate guard and -60 to 0 dB wet level. Musical sessions SHALL use the approved musical ceiling and actual-aggregate preparation from swell-gain-staging, with -60 to +12 dB user wet level. Gain SHALL ramp over 5 ms, protection SHALL remain fixed for an accepted schedule, and stored renders SHALL remain unchanged. The wet ceiling SHALL not be claimed for dry plus wet output.

#### Scenario: Loud overlapping IR results
- **WHEN** several loud prepared swells overlap
- **THEN** their protected aggregate stays within the saved policy's wet ceiling, relative contributions remain intact, and attenuation is visible separately from user gain

#### Scenario: Boost a musical schedule with headroom
- **WHEN** musical live wet level changes from 0 to +6 dB and the actual sum has sufficient headroom
- **THEN** positioned wet rises by the corresponding amplitude ratio after the 5 ms ramp while live dry is unchanged
- **AND** the same saved wet level and fixed protection plan apply to offline export

#### Scenario: Publication pages do not change the balance
- **WHEN** playback moves between prepared pages of the same accepted schedule, seeks or loops
- **THEN** its musical protection gain stays unchanged, including when a loud member's own peak lies outside the current page

#### Scenario: Accepted source-matched version
- **WHEN** a source-matched swell plays while an IR/source replacement and its recalculated baseline are pending
- **THEN** live and offline snapshots use the accepted audio with its corresponding saved baseline, never the replacement's baseline on the old audio
- **AND** looping over unchanged captured performances does not continually remeasure the reference or alter the balance

#### Scenario: Energy method survives export and looping
- **WHEN** source-energy calibration is accepted with its method revision and RMS/reference metadata
- **THEN** live and offline paths use that same saved baseline and disclose sample-peak protection separately
- **AND** neither host block partitions, page boundaries nor a changed IR recording level introduces repeated normalization during playback

### Requirement: Transport discontinuities do not replay old positions
Song seeking and looping SHALL abandon the old wet position and join prepared audio at the new absolute offset. Stopping, disabling or losing valid timing SHALL stop wet with a bounded click-prevention transition. Old-position audio SHALL not continue beyond that transition. Host transport SHALL never be changed by plugin playback controls.

#### Scenario: Loop through a swell
- **WHEN** a warm loop wraps into the middle of a swell
- **THEN** playback joins its loop destination offset and neither restarts the render nor continues the previous cursor

#### Scenario: Stop on nonzero audio
- **WHEN** Song transport stops on a nonzero wet sample while processing continues
- **THEN** wet reaches zero through a disclosed ramp no longer than 5 ms and does not restart until a subsequent valid Play

### Requirement: Timing incompatibility is explicit
Song live wet SHALL require valid absolute host samples and fixed rate/tempo/meter compatible with accepted assets. A fresh valid playing run SHALL establish an ephemeral sample-to-PPQ mapping independent of parked/source origin. Each run SHALL retain its mapping within the existing one-sample origin tolerance; unsupported changes SHALL suppress wet with a reason. Fresh validation SHALL precede resumption. Stored sample positions, edits and assets SHALL remain unchanged.

#### Scenario: Change tempo or sample rate
- **WHEN** the host mapping becomes incompatible with prepared assets
- **THEN** those swells are unavailable for live playback and the workspace explains the incompatibility; default Dry + Wet continues passing dry

#### Scenario: Playing clock differs from parked source mapping
- **GIVEN** verified ready wet at fixed absolute sample positions and a compatible fixed rate, tempo and meter
- **WHEN** a fresh playing run supplies a valid mapping whose origin differs from the parked/source mapping, including the recorded approximately 16-sample difference
- **THEN** the playing mapping is validated independently and eligible wet plays at its stored absolute sample positions
- **AND** no saved mapping, source, render, placement or persistent identity is rewritten or offset, and a resident valid window remains eligible for a warm join

#### Scenario: Zero-offset transport remains compatible
- **WHEN** a fresh playing run has the same origin as parked/source timing
- **THEN** existing absolute-position output, routing and gain behavior remain unchanged

#### Scenario: Mapping changes inside a playing run
- **WHEN** origin changes by more than the existing one-sample allowance during an established playing run
- **THEN** affected live wet is suppressed with the actual timing reason and the established run mapping is retained
- **AND** seeking, looping, preparation misses and revision updates do not silently establish a replacement baseline

#### Scenario: Invalid timing does not create a new run
- **WHEN** host timing is missing, nonfinite or out of range, or a callback range is invalid
- **THEN** affected wet is unavailable and that observation cannot establish a valid playing mapping or masquerade as a confirmed Stop
- **AND** returning timing is freshly validated against the retained run mapping before wet resumes

#### Scenario: Confirmed Stop followed by a new Play
- **WHEN** valid stopped transport is followed by a fresh valid playing run
- **THEN** a new runtime mapping is validated independently, with no automatic audition and no persistent edit
- **AND** only complete current assets can join at the new absolute Song position without replaying elapsed audio

#### Scenario: Session and workflow admission are independent
- **WHEN** state is restored, the audio session is replaced, or Perform is re-entered while the host is already playing
- **THEN** live admission requires fresh runtime validation and current asset dependencies, and no stale run identity can admit old work
- **AND** disabled live output remains disabled and Capture, Audition, internal Export and host offline timing behavior retain their existing contracts

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
Only verified current or explicitly frozen wet SHALL enter live playback, except a disclosed retained contribution during an eligible IR-only replacement. Changed source, shape, placement or mapping SHALL invalidate affected publications; cancelled generations SHALL not publish. Identical/unrelated capture SHALL preserve output. Manual missing-source recovery SHALL remain ineligible unless explicitly frozen while ready; frozen assets SHALL follow the render-freeze requirement.

#### Scenario: Recapture while listening
- **WHEN** capture commits identical source followed by changed source inside one swell's dependencies
- **THEN** identical capture does not disrupt it; changed capture removes its outdated contribution until a complete current replacement is ready, while unaffected swells remain playable

#### Scenario: Saved asset cannot verify
- **WHEN** reopen finds missing or corrupt source, IR or wet assets
- **THEN** affected wet remains excluded with an actionable diagnostic and unaffected verified swells remain eligible

#### Scenario: Replace only the IR
- **WHEN** a user requests a different IR for a verified ready swell without changing its other dependencies
- **THEN** its accepted old contribution remains eligible with its original verified IR/render identity while the replacement prepares, and only a complete latest valid replacement becomes current

### Requirement: Live edits publish without unsafe boundaries
Complete playback revisions SHALL become active at processing boundaries with short, disclosed transitions of at most 5 ms. Changes SHALL preserve absolute timing and existing fade/tension/placement semantics; partial results SHALL never play. Preparing new audio SHALL not play already elapsed lead-in or reset the whole Song cursor.

#### Scenario: Move a swell during playback
- **WHEN** a wet body is moved and its revised preparation completes
- **THEN** the new revision uses its new absolute interval with a bounded transition, without shifting dry input or restarting unrelated swells

### Requirement: Capture excludes all generated output
Capture/Recapture SHALL read original plugin input before routing, prepared wet, manual audition and diagnostic tone. Live and Finalise SHALL enqueue no source capture. Generated audio SHALL NOT enter the stored source through mode transitions or stale capture jobs.

#### Scenario: Wet plays over silent input
- **WHEN** Live plays prepared wet over zero dry input
- **THEN** no capture is enqueued and zero input cannot become a new wet-driven detection/convolution source

#### Scenario: Enter Capture after generated output
- **WHEN** an instance changes to Capture after Live wet or internal preview
- **THEN** newly admitted source contains only original input and no retired wet/preview/tone samples

### Requirement: Workspace distinguishes live and manual playback
The workspace SHALL show live controls, host playhead and live readiness separately from manual Audition. Enabled Song playback SHALL stop and inhibit manual audition while playing; disabling live playback SHALL retain independent manual preview behavior. Stopping Song playback SHALL not automatically start audition. Preparation alone SHALL never request host Play.

#### Scenario: Start Song during audition
- **WHEN** live playback is enabled and the host starts while manual audition is active
- **THEN** manual output transitions out within 5 ms and Song-positioned wet takes priority without sustained duplicate output; the UI identifies the active path

### Requirement: Live preferences restore without starting voices
Live enable, routing and wet level SHALL persist independently of audition preferences in bounded versioned metadata, with explicit migration from existing projects. The saved gain policy SHALL determine valid gain ranges and restoration behavior. Restore SHALL clear active voices and require fresh host timing and validated assets. Restoring state SHALL not request host transport or resume manual audition.

#### Scenario: Reopen an older project
- **WHEN** a project without live playback preferences is restored
- **THEN** enabled Dry + Wet with 0 dB user wet gain is selected, existing swell edits and audition preferences are preserved, and wet waits for valid playing transport and preparation
- **AND** its gain policy remains legacy-compatible until explicitly changed

#### Scenario: Restore positive musical wet level
- **WHEN** a musical-policy project saved with +4 dB live wet level reopens
- **THEN** +4 dB restores unchanged under its saved policy and validated ceiling, without autoplay or clamping to 0 dB

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
Small outlined circular Start/End handles SHALL stay on the waveform centre line at their exact times. Outlined circular fade/tension controls SHALL remain on the curve and stack in paint order at low zoom, with topmost hit priority. Only genuine coincidences in both time and amplitude SHALL receive a small offset. Zooming SHALL separate distinct times without editing envelope metadata.

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
- **AND** zero fades at unity naturally separate from the endpoint circles without an artificial offset

### Requirement: IR replacement retains one bounded accepted contribution
An eligible IR-only replacement SHALL retain at most one verified accepted contribution per swell, separately from requested state and pending work. Failed or superseded requests SHALL not replace accepted audio. Source, timing, shape or placement changes, deletion and invalid assets SHALL revoke retention; activity/solo/live routing SHALL still govern audibility. Retention SHALL NOT become a blanket stale-render fallback.

#### Scenario: Rapid selection and failed preparation
- **WHEN** response B then C are requested while accepted response A is audible, and B completes late or C fails
- **THEN** B cannot publish, A remains the sole retained contribution while its dependencies remain valid, and C's failure is reported without muting unaffected neighbours or claiming C is ready

#### Scenario: Source or editing changes during replacement
- **WHEN** source dependencies, shape, placement or host mapping change while an old contribution is retained
- **THEN** retention is revoked and normal invalidation/repreparation applies; deleting or disabling the swell cannot leave a ghost contribution

#### Scenario: Offline bounce during a pending replacement
- **WHEN** host offline rendering snapshots a session with an eligible accepted contribution and an unfinished IR-only request
- **THEN** that snapshot renders the accepted audible version consistently, discloses the pending requested version and does not switch mid-export to a background completion
- **AND** no unavailable wet is fabricated and separate plugin instances retain independent snapshots

### Requirement: Completed IR replacements join the current Song position safely
A complete latest IR replacement SHALL become active at a processing boundary with a disclosed transition of at most 5 ms at the current absolute Song offset. It SHALL not restart elapsed lead-in, sustain duplicate old/new output or restart neighbouring swells. Preparation, asset work, lifetime retirement and budget checks SHALL remain outside the real-time callback, whose mixing SHALL stay bounded.

#### Scenario: Handover during an overlapping swell
- **WHEN** a replacement is ready during Song playback with another swell overlapping
- **THEN** the old/new contribution transitions at the same Song position without an abrupt discontinuity or doubled steady-state level, while the neighbour's timing and samples are unchanged

#### Scenario: Replacement while stopped or during manual audition
- **WHEN** an IR replacement finishes with Song stopped
- **THEN** it becomes eligible for subsequent playback without starting a voice, moving host transport or automatically restarting manual audition

#### Scenario: Replacement exceeds resources
- **WHEN** the old contribution plus pending preparation or handover cannot fit the supported memory budget
- **THEN** the request reports resource pressure, retains eligible accepted output and does not silently raise the budget, truncate audio or allocate/block on the live callback

### Requirement: Host offline rendering honors workflow mode
Host offline rendering SHALL never capture source or settle an unfinished Capture pass implicitly. Live SHALL keep accepted-snapshot available-member export behavior and instance isolation. Capture/Finalise SHALL contribute dry passthrough without scheduled wet. Separately initiated internal preview SHALL not redefine host export routing.

#### Scenario: Pending capture at bounce entry
- **WHEN** a host bounce begins while source is provisional or Stop validation is settling
- **THEN** no provisional source becomes accepted because of the bounce and no offline input is captured
- **AND** output follows the admitted workflow mode and any unavailable contribution is explained

#### Scenario: Two different modes export together
- **WHEN** interleaved offline callbacks reach a Live instance and a Finalise instance
- **THEN** the Live instance uses its own accepted wet snapshot/routing while Finalise passes dry without scheduled wet

### Requirement: Mode output changes retain bounded audio safety
Mode gating SHALL reuse bounded click-prevention transitions of at most 5 ms and retire audio ownership outside realtime processing. Returning to Live SHALL join current absolute time rather than resume an old cursor. Background preparation SHALL never start host transport or an internal voice.

#### Scenario: Return during playback
- **WHEN** the user returns from Finalise to Live during valid DAW playback
- **THEN** eligible wet joins the current position safely without replaying prior lead-in or duplicating neighbours

#### Scenario: Worker completion in Finalise
- **WHEN** a render or IR replacement completes while Finalise is selected
- **THEN** accepted state can update under its normal dependency guards but scheduled wet remains bypassed

### Requirement: Live admission diagnostics distinguish failure gates
Developer Diagnostics SHALL distinguish configuration/workflow exclusion, unavailable publication, incompatible timing, invalid callback range and missing prepared coverage. Observations SHALL identify their callback or worker context and expose mapping values, preparation/publication revisions, window bounds and retry outcome without modifying session state or adding unbounded callback work.

#### Scenario: Published mapping is rejected while playing
- **GIVEN** a publication exists and enabled Perform playback rejects incoming host timing
- **WHEN** developer Diagnostics is inspected
- **THEN** it identifies the rejected timing dimension and displays source/published and observed sample, PPQ, rate, tempo, meter, inferred origins and origin difference with enough precision to assess the supported tolerance
- **AND** callback range validity, publication revision and current position are visible without presenting worker observations as the same callback

#### Scenario: A valid playing origin differs from source origin
- **WHEN** a separately validated playing run has an origin offset from parked/source timing
- **THEN** Diagnostics reports the run identity, its baseline and validation result separately from the informational source/publication origin delta
- **AND** a supported cross-state offset is not falsely reported as a current mapping rejection, while a rejected change within the run remains identifiable

#### Scenario: Stop clears a play-gated failure
- **WHEN** transport stops after a mapping rejection
- **THEN** stopped readiness is distinguished from successful playing admission and a bounded last-rejection observation remains identifiable as historical evidence
- **AND** no stopped observation claims wet was audible or a window was newly prepared solely because the playing flag cleared

#### Scenario: Disabled output and preparation misses
- **WHEN** workflow, saved live enable, routing, wet level or unavailable coverage explains absent wet
- **THEN** Diagnostics exposes the relevant configuration or preparation reason separately from timing rejection, including current/prepared revision and published coverage when available

#### Scenario: Inspect without disturbing the failing instance
- **WHEN** Diagnostics opens, refreshes or closes during playback or while stopped
- **THEN** project state, source/render identities, assets, transport and output remain unchanged
- **AND** callback observation publication performs no allocation, deallocation, lock, blocking wait, file access or logging

### Requirement: Transient live preparation rejection recovers at current position
Recoverable preparation or publication rejection SHALL remain eligible for bounded worker retry while current dependencies and timing are valid. Recovery SHALL validate the latest session, source/render identity and mapping before admitting complete prepared wet at current Song time. Unsupported timing and invalid assets SHALL remain explicitly unavailable; recovery SHALL NOT reinterpret stored positions or fabricate ready output.

#### Scenario: Stopped preparation followed by moving Song playback
- **GIVEN** an instance with 61 eligible swells has a stopped prepared window
- **WHEN** compatible Song playback starts before a swell and advances through it
- **THEN** available wet follows its absolute position and later windows remain eligible for preparation without an Export roundtrip, reset or recapture

#### Scenario: A transient publication attempt fails
- **WHEN** a preparation/publication attempt is transiently rejected and its blocking condition subsequently clears while playback continues
- **THEN** a later bounded worker attempt can publish a fully revalidated current window and wet joins at the then-current offset with the existing bounded transition
- **AND** elapsed wet is not replayed and retry status remains distinct from permanent timing incompatibility

#### Scenario: Dependencies change during retry
- **WHEN** source, render, session generation or mapping changes before a pending retry publishes
- **THEN** obsolete work is rejected and only complete output validated against the latest compatible dependencies can be admitted

#### Scenario: Genuine incompatibility persists
- **WHEN** rate, tempo, meter or the established playing-run mapping remains unsupported
- **THEN** affected wet stays unavailable with its actual reason and stored positions and accepted assets remain intact
- **AND** retry does not relax mapping guards or report readiness without validation

#### Scenario: Worker revalidates across clock transitions
- **WHEN** preparation spans a confirmed Stop/Play, restore or live admission transition
- **THEN** publication revalidates current run identity, timing, source/render dependencies and coverage or remains retryable
- **AND** a matching render revision or resident window alone does not certify a superseded clock validation, while still-valid resident PCM can be reused without changing its positions

### Requirement: Offline bounces validate their own runtime mapping
Each fresh host offline bounce SHALL establish a valid fixed-tempo runtime mapping independently of parked/source PPQ origin. Assets SHALL retain compatible rate, tempo and meter. Subsequent wet-required callbacks SHALL match the established bounce mapping within one sample. Stored source/render positions and identities SHALL remain unchanged.

#### Scenario: Export origin differs from prepared origin
- **GIVEN** compatible ready wet at absolute sample positions
- **WHEN** a fresh offline run supplies a valid origin offset, including the observed 16-sample difference at 48 kHz and 122 BPM
- **THEN** available swells render at their original absolute sample offsets with either Playing flag value and irregular blocks
- **AND** unchanged source/render assets, calibration and gain plans are retained without compensating sample shifts

#### Scenario: Genuine drift during required wet
- **WHEN** an established offline run changes origin beyond one sample, rate, tempo or meter while wet remains current or due
- **THEN** export fails visibly with precise failing timing and does not silently replace the baseline
- **AND** the existing completed-wet tail exception remains in force

#### Scenario: Lifecycle and separate instances
- **WHEN** offline processing is entered anew, preparation restarts, or an existing supported backward export restart occurs
- **THEN** the instance establishes a fresh mapping independently of other instances
- **AND** zero-length/invalid callbacks cannot admit wet, while accepted pending IR contributions retain their original snapshot rules

### Requirement: Offline timing failures are shareable
Offline timing errors SHALL retain the prepared, bounce-reference and failing host timing values. Diagnostics SHALL offer a keyboard-accessible Copy diagnostics action that copies the full current report and retained error without changing saved session state.

#### Scenario: Copy a failed bounce report
- **WHEN** the owner invokes Copy diagnostics after offline timing failure
- **THEN** the clipboard includes precise timing/error information, current capture/gain observations, Snap/grid availability and live/worker observations
- **AND** the action changes no source, swell, routing, transport or saved preference

### Requirement: Live timing rejection retains its causal observations
Diagnostics SHALL retain bounded ephemeral evidence of a live run's initial observation, last admitted callback before its first timing rejection, and that first rejection. Each SHALL identify its run/session/reset, callback range, timing and admission result. Run-reference-to-host origin deltas SHALL be explicit and distinct from source/publication deltas. Missing or incoherent evidence SHALL remain unavailable. Collection SHALL preserve existing timing admission and real-time safety.

#### Scenario: Source offset remains stable during playback
- **GIVEN** valid fixed rate, tempo and meter with a playing origin different from accepted source timing
- **WHEN** callbacks remain compatible with the established run
- **THEN** diagnostics distinguishes the informational source offset from the validated run origin without claiming a live rejection
- **AND** available prepared wet retains its existing absolute-position admission and output

#### Scenario: Recorded run origin changes
- **GIVEN** a run starts at sample 3399344 and PPQ 143.999322222222219 at 48000 Hz, 122 BPM and 4/4
- **WHEN** that same run observes sample 3795967 and PPQ 160.801390972220531
- **THEN** evidence identifies the approximately minus-16-sample run-origin delta, the first rejecting callback and the last preceding admitted observation if one exists
- **AND** a near-zero source/publication-to-host delta cannot be presented as successful run validation or permission to replace the reference

#### Scenario: Later symptoms cannot replace the first cause
- **WHEN** additional rejected callbacks, seeks, loops, window publications or preparation misses follow a run's first timing rejection
- **THEN** the run's first rejection and its preceding admitted observation remain retained separately from the existing latest historical rejection
- **AND** neither evidence collection nor those events silently create a new run baseline

#### Scenario: New run and unavailable preceding evidence
- **WHEN** a confirmed Stop/Play, session replacement or explicit audio reset establishes a new run under the existing lifecycle
- **THEN** evidence is associated with the new identity and prior observations cannot be labelled as its causal sequence
- **AND** if no callback was admitted before its rejection, or a coherent snapshot cannot be read, the preceding evidence is explicitly unavailable

#### Scenario: Inspect evidence without changing the project
- **WHEN** diagnostics is opened, refreshed or closed while playback continues, or the project is saved and reopened
- **THEN** observation does not change admission, routing, wet samples, source/render identities, saved definitions or host transport, and diagnostic history is not persisted in the project
- **AND** callback evidence uses bounded work without allocation, deallocation, locks, blocking, file access, text formatting or logging

### Requirement: Host reactivation rejoins current Song time
After host processing resumes during running transport, eligible prepared wet SHALL rejoin the current absolute Song offset without requiring a loop wrap or restart. Reactivation SHALL preserve accepted source, renders, edits and saved Live preferences. Recovery SHALL retain timing/asset validation, bounded preparation and callback safety.

#### Scenario: Host skips processing while transport advances
- **GIVEN** Live playback is enabled with compatible prepared swells
- **WHEN** the host disables processing, advances transport and re-enables processing before the next loop wrap
- **THEN** resident eligible wet resumes at the current absolute position and subsequent scheduled swells play before that wrap
- **AND** missed audio is not replayed or shifted to compensate for the suspension

#### Scenario: Re-enable inside a swell
- **WHEN** compatible processing resumes inside a resident swell's playback interval
- **THEN** wet joins that interval's current sample offset using the existing transition of at most 5 ms
- **AND** it does not restart the swell or continue its pre-disable cursor

#### Scenario: Repeated cycles preserve output and state
- **WHEN** host disable/re-enable repeats at different valid Song positions
- **THEN** wet remains aligned without duplicate contributions or stale suppression and accepted source, prepared render identities, edits and saved preferences remain unchanged
- **AND** another plugin instance remains independent

#### Scenario: Cold resumed position
- **WHEN** compatible processing resumes outside resident prepared coverage
- **THEN** bounded preparation reports missing coverage and retries for the current position without requiring a loop or restart
- **AND** complete eligible wet joins the then-current offset once ready without replaying elapsed audio

#### Scenario: Reactivation does not bypass admission
- **WHEN** processing resumes with unavailable timing, incompatible rate/tempo/meter, invalid assets, disabled saved Live output or a workflow outside Live
- **THEN** existing exclusion and diagnostic rules remain effective and no stale wet is admitted
- **AND** an ordinary seek, loop, missing callback interval or internal Live toggle alone does not authorize replacement of an established incompatible run mapping

### Requirement: Loop Cue requires independent live phase

Loop live playback SHALL use the running FL Studio host grid, the verified Rack Clock grid, or a verified legacy Phrase reference. Capture timing SHALL supply only the source grid and exact recorded extent. Standby SHALL check the active live route, pass dry and explain an unmet Cue prerequisite. Clock and FL Studio host playback SHALL repeat at the accepted sample length.

#### Scenario: Accepted capture lacks matching recurrence

- **WHEN** a legacy Phrase-only patch lacks recurrence matching the accepted take
- **THEN** the take remains valid and the legacy route cannot enable Cue

#### Scenario: Verify Phrase while still in Learn

- **WHEN** Rack receives valid Clock pulses in Learn
- **THEN** the current clock run can establish the grid used in Perform
- **AND** a legacy Phrase-only route may still verify recurrence for the accepted take in Learn
- **AND** Learn plays no Loop wet and entering Perform requires a fresh Cue

#### Scenario: Valid fractional reference

- **WHEN** required legacy marker intervals alternate within the verified adjacent integer lengths
- **THEN** phase stays verified within the epoch drift bound and each observed marker fixes its actual downbeat
- **AND** the accepted audio extent and render sample rate are preserved

#### Scenario: Dry audio is deliberately out of phase

- **WHEN** prepared wet is cued using valid live timing with dry audio offset from the target downbeat
- **THEN** wet follows the live timing without manual confirmation, and Dry + Wet keeps the live dry at unity when the output guard is idle

#### Scenario: Cue with a running grid

- **WHEN** a coherent prepared Loop is in Perform and FL Studio Pattern or Playlist playback or Rack Clock supplies valid bar timing
- **THEN** UI and supported MIDI Cue are eligible without a recurring Phrase input

### Requirement: Loop Cue schedules a complete future entry

An eligible Cue SHALL target the earliest future downbeat with room for the full negative lead-in. Playlist and Rack Clock routes SHALL schedule that downbeat from their respective bar grids. A legacy Phrase-only route SHALL use the observed phrase boundary. Playback SHALL repeat until stopped or invalidated.

#### Scenario: Cue arrives too late for the next lead-in

- **WHEN** the next possible boundary leaves insufficient time for the complete lead-in
- **THEN** Cue targets the earliest later boundary that fits it and plays no partial launch at the missed boundary

#### Scenario: Marker and Cue share an offset

- **WHEN** a timing marker and Cue arrive at the same sample
- **THEN** the marker is evaluated before Cue and the launch targets a future boundary

### Requirement: Perform Stop and live sync loss end Loop wet

MIDI Perform Stop SHALL cancel a queued launch or finish the current pass without mixing the next lead-in. Repeated Stop notes SHALL leave an existing finish request in place. The Stop Loop button SHALL finish the current pass and then offer Stop Now. MIDI and UI Stop Now SHALL cancel a queued launch or start the short wet fade immediately. Live timing loss or playback authority loss SHALL cancel queued launches and fade active wet. When MIDI commands coincide, Stop Now SHALL take precedence over Stop, and either stop SHALL take precedence over Cue.

#### Scenario: Capture commands and boundaries are independent

- **WHEN** Capture Start/Arm, Capture Stop or an ordinary phrase marker arrives during live Loop playback
- **THEN** the capture roles and marker do not request Perform Stop or Cue and a valid scheduled repeat continues

#### Scenario: Lost reference, seek or reset

- **WHEN** a required clock or marker is late or invalid, a seek or reset occurs, or the live timing or session becomes incompatible
- **THEN** any queued launch is cancelled and active wet fades. Dry + Wet keeps dry at unity after the wet fade ends
- **AND** restored timing leaves the scheduler in Standby until a fresh Cue

#### Scenario: Stop Loop is clicked during playback

- **WHEN** the musician clicks Stop Loop during the lead-in or current pass
- **THEN** the current pass continues to its end and the next lead-in is suppressed
- **AND** clicking Stop Now before completion starts the short fade immediately

#### Scenario: MIDI Stop and Stop Now are distinct

- **WHEN** a Stop note arrives while the Loop is playing
- **THEN** it finishes the current pass and suppresses the next lead-in even if another Stop note arrives
- **AND** a Stop Now note arriving before completion starts the short fade immediately

#### Scenario: FL Studio reaches a declared repeat boundary

- **WHEN** consecutive playing callbacks retain their musical mapping and repeat range and meet its exact end and start samples
- **THEN** a queued Cue and an active Loop retain their scheduled phase across the wrap
- **AND** an early jump to the same start sample is handled as a seek

### Requirement: Loop wet composition counts each cycle contribution once
Loop playback SHALL mix every eligible prepared swell with source-relative placement and exclusive end bounds. All scheduled negative leads and tails that overlap another cycle SHALL be included exactly once. Phase correction SHALL be bounded and keep the prepared content at its native rate.

#### Scenario: Lead and tail span multiple cycles
- **WHEN** an eligible swell has a lead or tail longer than one phrase and overlaps other swells or repeats
- **THEN** the wet sum equals the sample-level sum of the intersecting scheduled contributions
- **AND** the first entry contains only contributions from the first scheduled cycle and later cycles

#### Scenario: A downbeat differs from the earliest prediction
- **WHEN** a valid marker arrives inside its rounding window
- **THEN** bounded correction aligns the wet downbeat to the observed sample and retains the complete lead and tail content

### Requirement: Loop playback pins coherent prepared revisions

Loop wet SHALL use a complete prepared revision of the accepted source at the host sample rate. Queued launches SHALL require current render dependencies. An audible pass SHALL retain its last complete mix while a compatible edit prepares, then crossfade to the complete successor at the current Loop position without re-cueing. An intentionally silent mix SHALL allow a playing Loop to keep its position. Publication and retirement SHALL occur outside the callback. Resource admission SHALL account for pinned data and worst-case overlapping contributions within the shared preparation budget.

#### Scenario: Edits invalidate a queued or active revision

- **WHEN** placement or render dependencies change while a Loop is queued or playing
- **THEN** a queued launch with obsolete dependencies is cancelled
- **AND** an audible Loop keeps its phase and uses the last complete mix until a compatible successor crossfades in during repeating playback
- **AND** a change to the accepted source identity or sample-rate compatibility cancels or fades the old playback

#### Scenario: Prepared workload exceeds the supported bound

- **WHEN** the pinned memory or worst-case scheduled overlap exceeds the admission budget
- **THEN** Cue reports the limiting resource and dry continues without partial or truncated wet
- **AND** an existing compatible pass retains its last complete mix
- **AND** the callback performs no allocation, deallocation, disk I/O, convolution, blocking or locking

#### Scenario: Fixed-grid Loop peak protection follows the rendered mix

- **WHEN** the host or Rack Clock grid plays prepared Loop wet audio at a fixed sample period
- **THEN** wet protection uses measured aggregate peaks, including the partial overlap at launch and finish, while preserving the requested balance below the ceiling
- **AND** variable phase playback retains a conservative peak bound
