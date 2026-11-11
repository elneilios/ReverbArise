# song-export Specification

## Purpose
Provide a trustworthy full-song dry/wet composition whose explicit preview and durable WAV delivery use the same accepted audio, gain plan and absolute song alignment.

## Requirements

### Requirement: Export uses one immutable accepted composition
Export SHALL snapshot accepted source/timing, eligible wet versions, shapes/placements, gain-law revision and independent Dry/Wet levels into an immutable revision. Selection, Perform isolation and future IR defaults SHALL NOT filter its full-timeline membership. Eligibility SHALL preserve enable, mute and solo policy. An unfinished replacement SHALL use its verified accepted version or block current readiness with the affected identity.

#### Scenario: Multiple and overlapping swells
- **WHEN** Export prepares several eligible swells separated by gaps or overlapping at different positions
- **THEN** each shaped/positioned wet contribution appears once at its absolute position, captured dry appears once and a different selected swell does not change the composition

#### Scenario: Pending replacement
- **WHEN** an eligible swell has a pending requested IR and a verified accepted old render
- **THEN** that accepted version is pinned for the revision and its pending replacement is disclosed without switching partway through preview or writing
- **AND WHEN** no accepted required wet exists
- **THEN** current mixdown readiness is blocked with an identity/reason rather than presenting an incomplete wet layer as complete

### Requirement: Mix endpoints use a shared disclosed gain plan
Export SHALL sum independently scaled dry and accepted wet, each level in [0,1] with 1 meaning unity. Saved per-swell calibration, Level, shape and edge treatment SHALL remain shared by preview and WAV. Export SHALL NOT apply aggregate wet attenuation, combined attenuation, normalization or limiting. It SHALL preserve finite above-unity Float32 samples and disclose measured peak/overload. Monitoring boundary and live gain-transition ramps SHALL remain separately disclosed and excluded from file samples.

#### Scenario: Endpoint and blend oracle
- **WHEN** the same snapshot is prepared at Dry/Wet 0/100, 100/0, 100/100, 35/65 and 0/0
- **THEN** its samples equal respectively accepted wet, captured dry, dry plus wet, 0.35 dry plus 0.65 wet, and silence, without aggregate attenuation
- **AND** correlated wet overlaps retain their relative levels and original source/render bytes remain unchanged

#### Scenario: Monitoring preferences are independent
- **WHEN** Export mix changes while Perform has nonzero monitoring trim or a different live wet level/routing
- **THEN** only Export's composition preference changes, those other preferences remain saved unchanged and excluded monitoring factors are not silently baked into the file

### Requirement: Missing dry becomes warned silence
Dry-inclusive compositions SHALL substitute zero samples for uncaptured or unavailable dry spans within the export range. Captured silence, gaps and missing/corrupt dry assets SHALL remain distinguishable. A persistent accessible warning SHALL identify the affected ranges and substitution, including missing entire dry coverage. When Dry is zero, absent dry SHALL NOT block readiness or imply a missing wet dependency.

#### Scenario: Sparse or missing dry
- **WHEN** a composition with nonzero Dry crosses an uncaptured gap or an unreadable dry asset
- **THEN** that interval contributes zero dry, valid dry/wet remains sample-aligned and preview/file readiness carries a visible warned-silence condition
- **AND** the original source manifest is not changed or relabelled captured silence

#### Scenario: Change to wet only
- **WHEN** the same session switches to Dry 0% / Wet 100% with independently verified wet
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
Export Play SHALL use verified accepted layers from the parked preview position, or first audible sample by default. It SHALL continue through the audible end without relying on completed-WAV availability. Stop SHALL affect only internal preview. The absolute cursor SHALL remain independent of DAW transport. An all-zero mix SHALL not invent an audible start. Monitoring-only boundary and gain-transition ramps SHALL remain disclosed separately from file samples.

#### Scenario: Ephemeral preview seeking
- **WHEN** a current ready mix is clicked or dragged at an absolute waveform position
- **THEN** the preview position clamps to its audible range; stopped preview remains stopped and playing preview continues from that position after bounded prefetch with anti-click ramps
- **AND** latest seek intent supersedes older work, Stop revokes pending starts, and composition/file identity, source/render assets and persisted state remain unchanged
- **AND** pending/failed mixes, stale session/pass intents and mode exit cannot seek or autoplay a previous revision
- **AND** pending WAV preparation alone does not stop an already-running preview whose verified layer dependencies remain valid

#### Scenario: Preview versus decoded WAV
- **WHEN** a prepared independent-level mix is previewed and its WAV decoded
- **THEN** corresponding steady-gain interior samples, gain, overlap and timing agree within documented float tolerance, excluding only disclosed boundary and live gain-transition ramps from exact comparison

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
Mix/source/session changes SHALL invalidate current mixdown readiness without deleting older completed files. Async completions SHALL verify originating instance/session, immutable dependencies and request generation before publication. Only the current ready revision SHALL enable Save/Drag; callbacks, mode changes and preview completion SHALL NOT start export or playback automatically. Gain-only edits SHALL preserve already-running preview when its verified dependencies remain valid.

#### Scenario: Rapid mix and session changes
- **WHEN** different Dry/Wet pair requests finish out of order, or a different project restores during preparation
- **THEN** only the latest matching request can become current and stale work cannot publish or dirty the new session

#### Scenario: Edit during a prepared revision
- **WHEN** background accepted wet/source changes after a revision was prepared
- **THEN** the old file remains immutable, current readiness is visibly pending and any already-started write/preview remains pinned or stops explicitly without mixing revisions

### Requirement: Completed WAV files publish atomically and durably
Export SHALL write stereo 32-bit floating-point WAV at the verified composition rate, retaining channel order and a filename identifying both levels and gain-law revision. A completed managed revision SHALL be verified and atomically published before reference or dragging. Earlier published files SHALL never be silently overwritten, renamed or deleted by edits, Clear, mode changes or later exports. Failed publication SHALL retain prior valid files and report failure.

#### Scenario: File durability
- **WHEN** a Dry 100% / Wet 65% WAV has been dragged into the Playlist and another mix is exported
- **THEN** the new file uses a distinct immutable revision and the previously referenced file retains identical bytes/path

#### Scenario: Disk failure and cancellation
- **WHEN** writing, close, verification or publication fails or is cancelled
- **THEN** no partial destination becomes Ready and existing committed export/source/render files remain valid

### Requirement: Drag and Save deliver the same current revision
Drag WAV SHALL offer the current, background-verified ready revision to the OS as soon as the original pointer crosses the drag threshold. Save WAV SHALL deliver the same composition through a native chooser with explicit overwrite confirmation and safe completion ownership. Cancelled or unsupported dragging SHALL retain the durable file and leave Save available. A failed save or stale chooser SHALL NOT replace another session's output or report success.

#### Scenario: Drag into FL Studio
- **WHEN** the user drags a ready WAV into the target Windows FL Studio Playlist
- **THEN** a real complete file is offered, placement at song origin aligns it with source and the original file survives drag cancellation or later edits

#### Scenario: Save fallback
- **WHEN** external dragging is unsupported or cancelled and Save WAV is used
- **THEN** the same composition can be saved to a chosen path, an existing file requires explicit overwrite approval and a cancelled chooser changes no published file

#### Scenario: A new revision becomes ready during a pointer gesture
- **WHEN** a user presses on a ready Save or drag surface and the ready WAV, Export pass or session changes before the click or drag threshold is reached
- **THEN** the gesture cannot open a chooser, request delivery or drag a replacement WAV
- **AND** a new eligible interaction can deliver the new ready revision

#### Scenario: A pending native save becomes stale
- **WHEN** a save was invoked by pointer, keyboard or accessibility and the native chooser returns after its original WAV, Export pass, session or workspace becomes stale
- **THEN** the completion cannot copy, overwrite or report success for either the original or replacement WAV
- **AND** cancellation or teardown leaves committed files intact

#### Scenario: A drag handoff outlives the original gesture
- **WHEN** the queued native handoff runs after the original pointer gesture was released, cancelled, superseded or invalidated by a revision, Export pass or session change
- **THEN** no OS drag starts even if a new pointer press is active
- **AND** a drag attempt cannot fall through to Save on release
- **AND** a fresh eligible Save or drag remains available

#### Scenario: Drag a ready WAV
- **WHEN** the original primary gesture crosses the threshold and its prepared WAV is still current and present
- **THEN** Export offers that verified file to the operating system at most once for the gesture without a second full-file integrity scan
- **AND** a missing ready file cannot be offered as a valid drag

#### Scenario: Move between Windows drop targets
- **WHEN** an active drag moves between a target that accepts the WAV and one that rejects it
- **THEN** the cursor shows the WAV badge on the accepting target and the stop sign on the rejecting target
- **AND** the normal cursor and dotted drag affordance return after drop or cancellation

### Requirement: Export guidance prevents duplicate playback assumptions
Export SHALL explain that DAW scheduled wet is bypassed and advise staying in Export while playing its WAV. Wet-only delivery SHALL be labelled for layering; a dry-inclusive replacement SHALL warn to mute/replace original source to avoid doubling dry. Both level percentages and current preparation/failure/warning state SHALL be visible without a redundant Export Summary panel.

#### Scenario: Layer versus replace
- **WHEN** the export mix moves between Dry 0% / Wet 100% and any pair with nonzero Dry
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

### Requirement: Playing mix edits preserve the advancing playhead
While Export preview is playing, preset or level changes SHALL take effect with a bounded click-free monitoring gain transition at the advancing absolute playhead, without stopping, rewinding, reconvolving or waiting for WAV publication. A level edit from stopped SHALL NOT start preview. Playback SHALL remain finite and SHALL stop safely on invalid dependencies, mode/project changes, 0/0 or unrecoverable starvation.

#### Scenario: Change preset during playback
- **WHEN** the user changes Mix Character while verified Export preview is playing
- **THEN** the cursor advances without a preparation gap or reset and the sound transitions to the selected pair at the current timeline position
- **AND** captured source, accepted wet renders and previously published WAVs remain unchanged

#### Scenario: Rapid level edits with delayed file publication
- **WHEN** several fader or preset edits occur while WAV preparation is delayed
- **THEN** preview keeps advancing, the latest pair wins and older work cannot reinstate an earlier pair or cursor
- **AND** Save/Drag stay disabled until a verified file matches the latest pair and dependencies

#### Scenario: Stop revokes an in-flight mix change
- **WHEN** Stop, mode exit or project restore occurs during a playing gain edit
- **THEN** pending work cannot restart preview, carry its cursor into another session or publish another session's file

#### Scenario: Zero pair and finite end
- **WHEN** a playing edit reaches 0/0 or playback reaches its finite accepted end
- **THEN** preview stops without rewinding or looping
- **AND** changing back from 0/0 to a nonzero pair does not start playback until Play is explicitly invoked
- **AND** newly established audible bounds behind the current cursor cause a stop rather than a jump to a new audible start

### Requirement: Preview presentation distinguishes file readiness
The accepted preview target pair, waveform/legend and advancing cursor SHALL describe the verified audible preview independently of WAV readiness. Retained summaries and pending peak measurements SHALL be identified honestly. A pending or failed WAV SHALL NOT be offered as current, and file preparation alone SHALL NOT interrupt valid active preview.

#### Scenario: Preview pair accepted before the file is ready
- **WHEN** the playing preview accepts a new pair while its WAV or waveform detail is still preparing
- **THEN** controls and status identify the new target, a retained summary is labelled previous and current Save/Drag remain unavailable
- **AND** transition smoothing is disclosed as monitoring-only; the eventual WAV uses the stable target pair without the transition ramp

#### Scenario: Storage cannot sustain preview
- **WHEN** verified layer data cannot be supplied within the bounded preview buffering
- **THEN** preview stops with an honest starvation/error state without blocking the audio callback or claiming seamless output

### Requirement: Mix Character selects only a pair of levels
Export SHALL offer an accessible Mix Character selector and independently editable Dry and Wet percentages. A preset SHALL set only the pair atomically; direct edits SHALL retain the other level. Names SHALL derive from exact canonical pairs, with Custom for unmatched pairs. Preset values SHALL require owner listening and native interaction approval before final rollout.

#### Scenario: Preset selection and custom edit
- **WHEN** the user selects a preset and then adjusts one fader to an unmatched pair
- **THEN** the preset changes both levels in one intent without intermediate publication, the later edit changes only its fader, and the selector displays Custom
- **AND** no third gain stage, convolution or source/render modification occurs

#### Scenario: Matching pair and refresh
- **WHEN** a direct edit exactly matches an approved preset or the editor refreshes
- **THEN** the matching name can reappear, saved precision is preserved and refresh generates no edit or preparation request

### Requirement: Silent pair disables current delivery
At Dry 0% / Wet 0%, Export SHALL show a silent empty-preview state, omit an audible start and disable preview, seeking, Save and drag. Previous ready files SHALL remain immutable and SHALL NOT be offered as the current silent pair.

#### Scenario: Zero pair after a ready composite
- **WHEN** both levels become zero after a ready nonzero composition
- **THEN** current readiness is revoked, no prior WAV is offered and the silent state does not schedule live wet or publish a misleading ready file

### Requirement: Pair state preserves legacy file history
The authoritative saved mix SHALL be the independent pair, not a preset mode. New gain-law identities SHALL NOT reuse legacy crossfade files as current revisions. Legacy single-crossfade preferences SHALL reset explicitly to Dry 0% / Wet 100% while retaining historical file references, managed assets, unrelated creative state and VST3 identity.

#### Scenario: Restore old developer project
- **WHEN** a legacy project containing a crossfade preference and completed WAV history is restored
- **THEN** the preference resets with a documented version transition, old file descriptors and bytes remain intact and only a new-law matching revision can become current

#### Scenario: Save during pending preparation
- **WHEN** a new pair is saved before preparation finishes and later restored
- **THEN** the requested pair survives independently of the older completed reference and stale work cannot publish into the restored session

### Requirement: Export presentation reflects accepted independent levels
The white dry and aqua wet waveform contributions, percentage legend, peak warning and audible Fit SHALL reflect the accepted pair on one disclosed amplitude scale. Pending/error presentation SHALL distinguish retained accepted audio from requested levels. The approved dominant waveform/ruler, below-waveform Zoom/Fit and preview transport, and right-column delivery actions SHALL remain.

#### Scenario: Overlap and cancellation
- **WHEN** unity layers overlap above 0 dBFS or cancel in antiphase
- **THEN** PCM remains the transparent sum, actual peak status is truthful, separate layers retain their contributions and audible Fit follows the sum without trimming or retiming WAV delivery

#### Scenario: Rapid edit with retained waveform
- **WHEN** another pair is preparing or fails while a previous waveform is retained
- **THEN** the retained accepted pair is identified, the requested pair cannot relabel it and Save/drag cannot deliver it as current
