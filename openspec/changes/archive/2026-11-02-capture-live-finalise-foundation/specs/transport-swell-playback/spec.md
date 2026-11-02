# Spec Delta

## MODIFIED Requirements

### Requirement: Minor performance variations preserve the accepted baseline
Mapped source SHALL remain protected during ordinary Capture and all Live/Finalise playback regardless of performance variation. Only explicit Recapture SHALL permit replacement. Saved regeneration-tolerance values SHALL remain compatible metadata and SHALL NOT authorize implicit source updates; marker sensitivity and live dry SHALL remain independent.

#### Scenario: Small variation and accumulated drift
- **WHEN** repeated upstream performances vary slightly and later change materially with Recapture off
- **THEN** neither variation replaces mapped source or recalibrates its wet baseline
- **AND** the user must return to Capture and explicitly arm Recapture to adopt the changed performance

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

### Requirement: Live routing is independent of manual monitoring
Within Live, saved enable, Dry + Wet/Wet Only routing and wet level SHALL remain independent of audition, defaulting to enabled Dry + Wet at 0 dB. Dry + Wet SHALL add only prepared wet to unchanged input. Wet Only SHALL suppress dry only during enabled valid Live playback. Capture/Finalise SHALL preserve dry passthrough without modifying saved Live preferences.

#### Scenario: Play in context
- **WHEN** a prepared project plays in Live default Dry + Wet
- **THEN** output is input plus protected positioned wet, without captured-dry replay or audition blend/trim

#### Scenario: Route wet only and disable
- **WHEN** Wet Only is selected during Live playback and live playback is subsequently disabled
- **THEN** enabled Wet Only outputs prepared wet and disabling restores dry/removes wet with disclosed short routing ramps

#### Scenario: Enter Finalise from Wet Only
- **WHEN** Finalise is entered from Live with Wet Only selected
- **THEN** scheduled wet stops through the existing bounded transition, normal DAW output becomes dry passthrough and returning to Live restores the saved preference

### Requirement: Capture excludes all generated output
Capture/Recapture SHALL read original plugin input before routing, prepared wet, manual audition and diagnostic tone. Live and Finalise SHALL enqueue no source capture. Generated audio SHALL NOT enter the stored source through mode transitions or stale capture jobs.

#### Scenario: Wet plays over silent input
- **WHEN** Live plays prepared wet over zero dry input
- **THEN** no capture is enqueued and zero input cannot become a new wet-driven detection/convolution source

#### Scenario: Enter Capture after generated output
- **WHEN** an instance changes to Capture after Live wet or internal preview
- **THEN** newly admitted source contains only original input and no retired wet/preview/tone samples

## ADDED Requirements

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
