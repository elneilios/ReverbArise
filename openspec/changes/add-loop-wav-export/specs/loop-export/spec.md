# Spec Delta

## Purpose

Provide a finite Loop WAV with independent Dry/Wet levels, preserved wet lead-in and tails, predictable grid placement, and matching preview, drag and save delivery independent of live synchronization.

## ADDED Requirements

### Requirement: Loop export pins accepted capture-local authority
Loop Export SHALL snapshot the accepted source, exact Loop extent, capture-local timing, eligible accepted wet revisions, placements, shapes, calibration, independent Dry/Wet levels, repeat count and lead-in grid into one immutable composition. It SHALL NOT require current host position, running transport or live Phrase recurrence. Song export SHALL retain its existing behavior.

#### Scenario: Export without live synchronization
- **WHEN** a Clock-only take has an accepted musical grid and verified eligible wet but Cue Loop has no usable live clock
- **THEN** Loop export can prepare from its accepted capture-local coordinates independently of Cue readiness
- **AND** current selection and Perform isolation do not filter the export's enabled, unmuted, solo-eligible membership

#### Scenario: Pending replacement and revision change
- **WHEN** an eligible wet replacement is pending or accepted dependencies change during preparation
- **THEN** a verified accepted version is explicitly pinned or preparation identifies the missing member
- **AND** superseded work cannot become the current revision or alter earlier completed files

### Requirement: Initial lead-in rounds outward to the selected grid
Loop Export SHALL preserve all included initial negative wet audio and round WAV start backward to the selected verified beat or bar grid, inserting silence as needed. WAV frame zero and Loop Bar 1 SHALL be distinct labelled coordinates. The drag tile SHALL show placement guidance directly below its action title, followed by a gap and the file details. Missing accepted beat or meter authority SHALL disable affected musical grid choices with a clear explanation rather than invent timing.

#### Scenario: Four-bar placement
- **WHEN** the earliest contribution starts at minus 2.4 bars and the lead-in grid is four bars
- **THEN** WAV origin is minus four bars, with 1.6 bars of leading silence
- **AND** guidance explains that to place Loop Bar 1 at project bar 17 the WAV starts at bar 13

#### Scenario: No negative wet
- **WHEN** all included initial contributions begin at or after Loop Bar 1, including a dry-only mix with Wet at zero
- **THEN** WAV origin is Loop Bar 1 with no extra grid block of silence

#### Scenario: Incomplete musical metadata
- **WHEN** a take lacks verified meter needed for a bar grid
- **THEN** the bar-grid option is unavailable with its reason and no live transport value silently reinterprets the capture

### Requirement: Finite repeats preserve exact wrap composition
Loop Export SHALL compose the requested positive integer count of Loop passes with one initial lead-in, internal negative wet and tail overlaps summed exactly once, and the final scheduled pass's complete eligible tail. It SHALL NOT anticipate an unscheduled next pass. Pass boundaries SHALL derive from the exact accepted extent without cumulative rounding drift, including non-whole-bar and fractional-sample periods.

#### Scenario: Overlapping repeats and long tails
- **WHEN** negative wet and tails cross one or several Loop boundaries in a multi-repeat export
- **THEN** decoded samples equal the reference sum of only the scheduled passes, each contribution appearing once
- **AND** the file retains the first pass's lead-in and last pass's full tail without adding a next pass's lead-in

#### Scenario: Fractional periods and capture methods
- **WHEN** Duration, End on Signal or End on repeat produces a valid exact extent whose beat mapping has a fractional sample period
- **THEN** repeat spacing follows that extent using absolute boundary conversion and remains aligned over many repeats

### Requirement: Preview and delivery share the Perform wet reference
Loop Export preview, waveform and stereo Float32 WAV SHALL share one finite mix at the accepted rate. Dry 100% SHALL be unity captured dry. Wet 100% SHALL use Perform's Live Wet and Loop overlap protection before the combined output guard. Wet fader edits SHALL scale that reference. File peak warnings SHALL describe the measured result.

#### Scenario: Preview and WAV comparison
- **WHEN** the same Loop is mixed at Dry/Wet 0/100, 100/0, 100/100 and 35/65, previewed and delivered through Drag WAV and Save WAV
- **THEN** steady interior preview samples and both decoded files agree within documented floating-point tolerance
- **AND** the composition equals respectively Perform wet, repeated captured dry, dry plus Perform wet, and 0.35 dry plus 0.65 Perform wet
- **AND** source/render bytes remain unchanged and peak warnings describe the actual file
- **AND** any preview-only boundary ramps are disclosed

#### Scenario: Export an edited Perform loop as a wet stem
- **WHEN** a user fine-tunes a Loop in Perform and exports at the default Dry 0% / Wet 100%
- **THEN** the stem uses the same accepted swell levels and effective wet gain as Perform
- **AND** repeat count and lead-in placement preserve the finite composition without changing the effective wet gain
- **AND** missing dry does not change the wet reference or prevent verified wet-only export
- **AND** original dry remains a separate host mixer decision

### Requirement: Dry audio repeats once per scheduled pass
A nonzero Dry level SHALL include accepted capture-local dry audio exactly once per scheduled pass using the same exact boundaries as wet composition. Dry SHALL be silent before the first pass and after the final pass; wet lead-in and tails SHALL NOT repeat dry outside those bounds. Missing dry coverage SHALL follow the existing warned-silence policy, without fabricating source or changing accepted assets. Dry at zero SHALL require no dry reads.

#### Scenario: Dry and wet across repeat boundaries
- **WHEN** two passes are exported with Dry 100% / Wet 100% and overlapping wet lead-in and tails
- **THEN** dry occupies only the two scheduled pass intervals, without duplication at their shared boundary
- **AND** dry remains zero in initial lead-in padding and final wet-only tails

#### Scenario: Missing dry and wet-only export
- **WHEN** nonzero Dry references missing captured coverage
- **THEN** affected coverage becomes explicitly warned silence while verified contributions stay aligned
- **AND** setting Dry to zero removes the dry dependency and its substitution warning for the current composition

### Requirement: Loop mix controls reuse the shared Export pair
Loop Export SHALL reuse Mix Character and independent Dry/Wet faders, defaulting a fresh Loop mix to Wet Layer (Dry 0%, Wet 100%). Presets SHALL change only the level pair atomically; direct edits SHALL retain the other level and display Custom for unmatched pairs. The pair SHALL govern both Preview and WAV. Dry-inclusive guidance SHALL warn against doubling the original dry. At 0/0, current preview, Save and Drag SHALL be disabled without deleting earlier files.

#### Scenario: Character selection and manual fader edit
- **WHEN** a user selects an existing Mix Character preset and then changes one fader
- **THEN** Preview, waveform, measured peaks and eventual WAV follow the same latest accepted pair
- **AND** repeat count, lead-in grid, captured audio and render assets are unchanged, with file readiness pending until the matching revision verifies

#### Scenario: Silent pair and retained history
- **WHEN** both Dry and Wet are set to zero after a completed export
- **THEN** the current silent mix cannot preview, Save or Drag, and earlier immutable WAV files remain intact

### Requirement: Loop delivery is bounded atomic and revision-safe
Loop export SHALL preflight sample rate, duration, arithmetic, output-container size, memory, disk and required asset revisions. Preparation and file work SHALL stay outside realtime processing with bounded cancellable buffers. Only a complete verified current revision SHALL enable Save and Drag; stale completions, chooser results and partial writes SHALL NOT replace current state or overwrite durable managed files.

#### Scenario: Excessive range or storage failure
- **WHEN** repeat/grid settings exceed supported bounds, storage fails or cancellation interrupts publication
- **THEN** no shortened or partial file is reported Ready and the reason is shown
- **AND** prior completed WAVs, capture, renders and caches remain valid

#### Scenario: Session changes during delivery
- **WHEN** a project restores or a newer composition supersedes a pending write, drag verification or Save chooser
- **THEN** stale work cannot publish into the new session or offer the previous revision as current

### Requirement: Loop Export extends the approved workspace
Loop Export SHALL reuse the existing Export workspace, Mix Character, Dry/Wet controls and shared interaction conventions for preview, zoom, file dragging and saving. Repeat/grid controls, coordinate labels and placement help SHALL use approved production roles, accessible names and truthful disabled reasons. A revised native preview SHALL receive owner approval before rollout; Song controls and behavior SHALL remain compatible.

#### Scenario: Native review and host placement
- **WHEN** the proposed Loop Export layout is ready for integration
- **THEN** the owner is shown the native preview with Mix Character, Dry/Wet faders, repeat/grid controls, WAV start, Loop Bar 1 and delivery guidance
- **AND** automated component checks and a focused FL Studio import check verify that the approved controls and exported placement agree
