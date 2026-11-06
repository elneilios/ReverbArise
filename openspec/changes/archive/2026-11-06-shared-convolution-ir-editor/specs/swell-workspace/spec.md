# Spec Delta

## ADDED Requirements

### Requirement: Sound editing has consistent context-bound interactions
Learn Sound and Perform Sound SHALL expose the same IR choice, import, accepted/requested feedback and deliberate-selection interactions. Scope SHALL identify future-generation defaults in Learn and the selected swell in Perform. Refresh or switching context SHALL NOT submit edits, inherit settings into existing swells or regenerate audio.

#### Scenario: Edit two independent contexts
- **WHEN** a user changes Learn's accepted default and then opens Perform Sound for an existing swell
- **THEN** Perform retains that swell's assignment and Learn retains the new default
- **AND** changing that swell's IR changes neither the default nor any other swell or instance

#### Scenario: Refresh and incidental input
- **WHEN** accepted state refreshes, Sound tabs switch, catalogue order changes or the pointer wheel crosses an IR selector
- **THEN** no assignment is submitted and selection follows managed content identity rather than row position
- **AND** pending requests remain distinguishable from accepted defaults and retained audible responses

### Requirement: Sound intents retain their original scope
IR commands and asynchronous completions SHALL retain their invocation's session and context ownership. Learn SHALL enforce workflow-pass locks; Perform SHALL enforce selected-swell identity, input revision, mode and freeze guards. Stale, invalid, cancelled or unavailable commands SHALL preserve accepted assignments and retained output rather than edit a replacement target.

#### Scenario: Selection or session changes during a choice
- **WHEN** a menu or file chooser opened for swell A completes after selecting B, deleting A or restoring a session
- **THEN** it addresses the original valid A or is cancelled; it never addresses B or a replacement session
- **AND** freezing A or superseding its input invalidates the obsolete command

#### Scenario: Pass and mode guards apply to every route
- **WHEN** a default request crosses DAW Play, Stop settlement or a workflow-pass change, or a selected-swell request crosses into a disallowing mode
- **THEN** the existing command guards reject obsolete publication even if activation bypasses disabled UI
- **AND** the editor reconciles to accepted values with a meaningful unavailable or failure reason

#### Scenario: Import is pending or fails
- **WHEN** import is cancelled, decoding fails or a newer request supersedes pending work
- **THEN** accepted identities, instance-local retained choices and last-good output follow existing retention rules
- **AND** feedback never claims an unaccepted request is the current audible response

### Requirement: Shared Sound preserves creation and audio ownership
Sound defaults SHALL enter new swells only through existing accepted Generate or manual-creation transactions. Existing automatic, manually edited, manually created and frozen swells SHALL retain their ownership rules. Sharing the editor SHALL NOT change source PCM, saved assignments, render/cache identities, gain calibration or routing, or introduce a migration or blanket regeneration.

#### Scenario: Generate snapshots an accepted default
- **WHEN** explicit or eligible Generate on Stop commits after a default is accepted
- **THEN** eligible automatic definitions use the transaction's accepted default snapshot with existing cancellation and provenance rules
- **AND** manual, edited and frozen definitions remain protected and a later default edit does not live-update committed swells

#### Scenario: Manual creation and subsequent editing
- **WHEN** a user creates a manual swell and later changes the future default or edits an automatic swell's IR
- **THEN** the manual swell retains its creation-time assignment and manual ownership
- **AND** the automatic swell's explicit edit follows the existing manual-edited provenance policy

#### Scenario: Save reopen and unchanged audio
- **WHEN** an existing project opens and both Sound contexts are inspected without an edit
- **THEN** captured bytes, saved IR/render references, exact envelope/Level, durable identities and accepted audio remain unchanged
- **AND** audition, host playback, plugin WAV and offline output retain their existing content and distinct admission policies

### Requirement: Sound access remains readable and limited to delivered processing
Both Sound contexts SHALL retain discoverable labelled access, readable production typography, keyboard focus, accessible scope/status and bottom context help at supported compact and larger sizes/scales. Existing exact-value edits SHALL remain available. Detection SHALL stay Learn-only and final Dry/Wet mixing Export-owned. Unsupported processing controls SHALL remain absent.

#### Scenario: Compact and scaled native review
- **WHEN** Learn Sound and Perform Sound are reviewed at 900x600/100% and supported fitting 125% presentation plus larger sizes
- **THEN** IR choice/import/status, full focus outlines and Perform's existing exact Level edit remain reachable without clipping or hiding Reset
- **AND** long labels, empty selection, unavailable assets, frozen and busy states remain understandable without additional waveform editors

#### Scenario: Review the real control family
- **WHEN** the shared family and integrated composition are ready
- **THEN** a prepared native preview is launched directly for scoped owner look/feel approval before wider rollout
- **AND** component/binding evidence, actual host/DPI coverage and guide currency are recorded separately

#### Scenario: Future processing remains future work
- **WHEN** the convolution foundation is displayed
- **THEN** no Character, Tone, HP, LP, Stretch or Profile placeholder appears and no new Learn Level default is introduced
- **AND** IR identity remains distinct from processing Character and Export Mix Character

## MODIFIED Requirements

### Requirement: Learn exposes independent progressive capture controls
Learn SHALL separate Source Capture/Recapture from Automatic Swells. Automatic Swells off SHALL hide sensitivity, Auto-generate on Stop and Generate controls while retaining their values and explaining dry-only learning. Sound SHALL retain access to the accepted Default IR independently of detection. On SHALL expose real supported generation settings and explicit Generate, with a retained Auto-generate on Stop choice. Unsupported Character algorithms/parameters SHALL be absent rather than selectable placeholders.

#### Scenario: Dry-only controls
- **WHEN** Automatic Swells is switched off while Learn is idle
- **THEN** dry capture and Recapture stay independently available, generation preferences hide, and switching on again restores their accepted values
- **AND** Sound remains accessible for the accepted future default without enabling detection or generating swells

#### Scenario: Character processing is unavailable
- **WHEN** the build has no implemented Character processing
- **THEN** no inactive/fictitious Character choices or parameter controls appear, and a bundled static IR is not presented as a Character algorithm
