# song-export Spec Delta

## Purpose

Provide a trustworthy full-song dry/wet composition whose explicit preview and durable WAV delivery use the same accepted audio, gain plan and absolute song alignment.

## ADDED Requirements

### Requirement: Export uses one immutable accepted composition
Export SHALL snapshot accepted source/timing, eligible wet versions, shapes/placements, gain policy and mix percentage into an immutable revision. Selection, Perform isolation and future IR defaults SHALL NOT filter its full-timeline membership. Eligibility SHALL preserve enable, mute and solo policy. An unfinished replacement SHALL use its verified accepted version or block current readiness with the affected identity.

#### Scenario: Multiple and overlapping swells
- **WHEN** Export prepares several eligible swells separated by gaps or overlapping at different positions
- **THEN** each shaped/positioned wet contribution appears once at its absolute position, captured dry appears once and a different selected swell does not change the composition

#### Scenario: Pending replacement
- **WHEN** an eligible swell has a pending requested IR and a verified accepted old render
- **THEN** that accepted version is pinned for the revision and its pending replacement is disclosed without switching partway through preview or writing
- **AND WHEN** no accepted required wet exists
- **THEN** current mixdown readiness is blocked with an identity/reason rather than presenting an incomplete wet layer as complete

### Requirement: Mix endpoints use a shared disclosed gain plan
Export SHALL use dry gain 1-w and wet gain w for wet fraction w in [0,1]. At 100% it SHALL require no dry contribution; at 0% it SHALL include no wet contribution. Saved per-swell calibration, Level and shape SHALL precede aggregate protection. Existing policy ceilings SHALL be reused without upward normalization; any branch/sum attenuation SHALL be disclosed and identical in preview and file output.

#### Scenario: Endpoint and blend oracle
- **WHEN** the same snapshot is prepared at 0%, 50%, 65% and 100% Wet
- **THEN** its samples equal respectively dry, half dry plus half wet, 35% dry plus 65% wet, and wet, subject to the same disclosed protection plan
- **AND** correlated wet overlaps retain their relative levels and original source/render bytes remain unchanged

#### Scenario: Monitoring preferences are independent
- **WHEN** Export mix changes while Perform has nonzero monitoring trim or a different live wet level/routing
- **THEN** only Export's composition preference changes, those other preferences remain saved unchanged and excluded monitoring factors are not silently baked into the file

### Requirement: Missing dry becomes warned silence
Dry-inclusive compositions SHALL substitute zero samples for uncaptured or unavailable dry spans within the export range. Captured silence, gaps and missing/corrupt dry assets SHALL remain distinguishable. A persistent accessible warning SHALL identify the affected ranges and substitution, including missing entire dry coverage. At 100% Wet, absent dry SHALL NOT block readiness or imply a missing wet dependency.

#### Scenario: Sparse or missing dry
- **WHEN** a 50% or dry-only composition crosses an uncaptured gap or an unreadable dry asset
- **THEN** that interval contributes zero dry, valid dry/wet remains sample-aligned and preview/file readiness carries a visible warned-silence condition
- **AND** the original source manifest is not changed or relabelled captured silence

#### Scenario: Change to wet only
- **WHEN** the same session switches to 100% Wet with independently verified wet
- **THEN** no dry is read or mixed and the dry-substitution warning no longer describes that wet-only composition as incomplete

### Requirement: Mix range preserves song origin and complete tails
WAV frame zero SHALL represent the verified song origin. The composition SHALL retain leading silence and all nonnegative contributions through the latest accepted dry extent or eligible wet endpoint with checked arithmetic. Pre-origin wet SHALL be clipped at origin without shifting arrivals or replaying omitted lead-in. View fitting and preview start SHALL NOT trim or retime the delivered file.

#### Scenario: Late source and negative pre-roll
- **WHEN** source starts at bar 9 and a wet swell extends before song origin
- **THEN** both delivery paths retain song-origin alignment, include the valid wet portion at/after origin and all later tails, and never shift the dry event to file frame zero

#### Scenario: Incompatible mapping or excessive range
- **WHEN** timing/rate cannot establish reliable origin or bounds overflow a supported output container
- **THEN** publication is unavailable with the precise reason and no implicit resampling, stretching, wrapping or shortened success occurs

### Requirement: Explicit preview matches the prepared export
Export Play SHALL use the prepared composition from the parked preview position, or first audible sample by default. It SHALL continue through the audible end. Stop SHALL affect only internal preview. The absolute cursor SHALL remain independent of DAW transport. An all-zero mix SHALL not invent an audible start. Monitoring-only boundary ramps SHALL remain disclosed separately from file samples.

#### Scenario: Ephemeral preview seeking
- **WHEN** a current ready mix is clicked or dragged at an absolute waveform position
- **THEN** the preview position clamps to its audible range; stopped preview remains stopped and playing preview continues from that position after bounded prefetch with anti-click ramps
- **AND** latest seek intent supersedes older work, Stop revokes pending starts, and composition/file identity, source/render assets and persisted state remain unchanged
- **AND** pending/failed mixes, stale session/pass intents and mode exit cannot seek or autoplay a previous revision

#### Scenario: Preview versus decoded WAV
- **WHEN** a prepared 0/50/100% mix is previewed and its WAV decoded
- **THEN** corresponding interior samples, gain, overlap and timing agree within documented float tolerance, with only disclosed preview start/stop ramps excluded from exact comparison

#### Scenario: Bypass and live input
- **WHEN** DAW playback runs while Export preview is idle or explicitly playing
- **THEN** normal DAW output retains dry passthrough with no capture or scheduled wet; explicit preview remains a separate contribution and never records its own output or starts/seeks the DAW

### Requirement: Preparation is bounded and never blocks realtime audio
Composition decoding, waveform analysis, mixing, validation and writing SHALL run outside the callback with bounded buffers and cancellation. Full-song PCM SHALL NOT be retained solely to support long or sparse exports. The callback SHALL consume prepared audio without allocation/deallocation, locks, file access or waits. Worker starvation SHALL stop/fail preview honestly instead of omitting samples from a successful file.

#### Scenario: Long sparse arrangement
- **WHEN** a range exceeds the resident working budget or contains a long silent lead-in
- **THEN** preparation/writing processes bounded pages and verified zero spans, memory stays within the shared budget and delivered sample count/alignment remains exact

#### Scenario: Slow storage
- **WHEN** a worker cannot keep preview pages supplied or file writing fails
- **THEN** live dry remains responsive, preview reports its underrun/failure and a partial file is never exposed as a completed revision

#### Scenario: Temporary coherent timing read miss
- **WHEN** the worker temporarily cannot read coherent host timing while an accepted composition is unchanged
- **THEN** preparation waits without creating an invalid dependency revision or canceling that accepted preview
- **AND** actual callback timing gates remain enforced and valid but incompatible timing is not treated as compatible

### Requirement: Latest mix intent owns readiness
Mix/source/session changes SHALL invalidate current mixdown readiness without deleting older completed files. Async completions SHALL verify originating instance/session, immutable dependencies and request generation before publication. Only the current ready revision SHALL enable Save/Drag; callbacks, mode changes and preview completion SHALL NOT start export or playback automatically.

#### Scenario: Rapid mix and session changes
- **WHEN** 50%, 100% and 65% requests finish out of order, or a different project restores during preparation
- **THEN** only the latest matching request can become current and stale work cannot publish or dirty the new session

#### Scenario: Edit during a prepared revision
- **WHEN** background accepted wet/source changes after a revision was prepared
- **THEN** the old file remains immutable, current readiness is visibly pending and any already-started write/preview remains pinned or stops explicitly without mixing revisions

### Requirement: Completed WAV files publish atomically and durably
Export SHALL write stereo 32-bit floating-point WAV at the verified composition rate, retaining channel order and a mix-identifying filename. A completed managed revision SHALL be verified and atomically published before reference or dragging. Earlier published files SHALL never be silently overwritten, renamed or deleted by edits, Clear, mode changes or later exports. Failed publication SHALL retain prior valid files and report failure.

#### Scenario: File durability
- **WHEN** a 65% WAV has been dragged into the Playlist and another mix is exported
- **THEN** the new file uses a distinct immutable revision and the previously referenced file retains identical bytes/path

#### Scenario: Disk failure and cancellation
- **WHEN** writing, close, verification or publication fails or is cancelled
- **THEN** no partial destination becomes Ready and existing committed export/source/render files remain valid

### Requirement: Drag and Save deliver the same current revision
Drag WAV SHALL initiate external file dragging only after the current revision exists and verifies. Save WAV SHALL deliver the same composition through a native chooser with explicit overwrite confirmation and safe completion ownership. Cancelled/unsupported external dragging SHALL retain the durable file and leave Save available. A failed save or stale chooser SHALL NOT replace another session's output or report success.

#### Scenario: Drag into FL Studio
- **WHEN** the user drags a ready WAV into the target Windows FL Studio Playlist
- **THEN** a real complete file is offered, placement at song origin aligns it with source and the original file survives drag cancellation or later edits

#### Scenario: Save fallback
- **WHEN** external dragging is unsupported or cancelled and Save WAV is used
- **THEN** the same composition can be saved to a chosen path, an existing file requires explicit overwrite approval and a cancelled chooser changes no published file

### Requirement: Export guidance prevents duplicate playback assumptions
Export SHALL explain that DAW scheduled wet is bypassed and advise staying in Export while playing its WAV. Wet-only delivery SHALL be labelled for layering; a dry-inclusive replacement SHALL warn to mute/replace original source to avoid doubling dry. Mix percentage and current preparation/failure/warning state SHALL be visible without a redundant Export Summary panel.

#### Scenario: Layer versus replace
- **WHEN** the export mix moves between 100% and an intermediate percentage
- **THEN** filename/guidance switches truthfully between wet-layer and dry-inclusive replacement use, while normal DAW routing preferences remain unchanged


### Requirement: Waveform inspection reuses accepted composition authority
Bounded viewport summaries SHALL use the same accepted layers, placement and
fixed gain plan as the current composition. Navigation/detail requests SHALL be
coalesced and ownership checked without changing mix/file identity, reconvolving,
blocking realtime audio or retaining full-song PCM. Retained prior presentation
SHALL remain distinct from requested composition readiness.

#### Scenario: Zoom without changing export
- **WHEN** an accepted export is inspected at a different horizontal range
- **THEN** detailed peaks agree with accepted layer samples, file bytes/identity remain unchanged and stale range results cannot replace a newer view
