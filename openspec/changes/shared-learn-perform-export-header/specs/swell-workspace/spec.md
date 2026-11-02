## MODIFIED Requirements

### Requirement: Mode selector explains eligibility and transition locks
The shared header SHALL show prominent Learn, Perform and Export segments with concise roles/status and accessible unavailable reasons. Active, available and disabled states SHALL differ. Recording/Stop settlement SHALL lock selection and keep source provisional. Source readiness SHALL differ from pending wet work. Existing read-only workspaces SHALL remain until replaced; no unfinished export delivery or unverified host transport action SHALL be exposed.

#### Scenario: Insufficient material
- **WHEN** a fresh, silent-only or incompatible instance has no eligible source or no enabled prepared swell
- **THEN** Perform/Export availability reflects the distinct existing prerequisites and explains what is missing without treating a pending render as required for source-ready Perform

#### Scenario: Stop is settling
- **WHEN** Learn records or its Stop-triggered source transaction is draining or validating
- **THEN** no segment interaction can switch modes, the active mode remains visible, and status identifies recording or source settlement
- **AND** successful useful source acceptance selects Perform without waiting for queued wet rendering; empty/failed passes retain the foundation policy

## ADDED Requirements

### Requirement: Header navigation is accessible and truthful
The segmented selector SHALL support pointer and keyboard activation with visible focus, accessible names and selected/unavailable state. Disabled reasons SHALL be obtainable without attempting a transition. Selection SHALL reflect accepted model state, never an optimistic click. Refresh, repeated active selection and incidental wheel movement SHALL NOT initiate mode changes.

#### Scenario: Keyboard selection and stale availability
- **WHEN** a user focuses a mode and activates it with the keyboard while readiness changes
- **THEN** the same guarded request used by pointer activation determines acceptance, failure retains the accepted mode, and focus and a meaningful reason remain available

#### Scenario: Read unavailable reason
- **WHEN** a mode is unavailable because of recording, settlement, missing source or incompatible wet timing
- **THEN** its accessible description or equivalent focusable help exposes the actual reason even though activation is disabled

### Requirement: Shared header preserves approved composition
All three modes SHALL reuse the same header with the approved wordmark, italic tagline, three-segment selector and Settings action. It SHALL fit the supported minimum and larger sizes without clipping, overlapping or adding duplicate navigation. Current-mode subtitles and status SHALL describe available behavior without implying deferred WAV delivery exists.

#### Scenario: Minimum and larger editor sizes
- **WHEN** empty, ready, recording, settling and failure states are displayed at 1100x850 and supported larger logical sizes
- **THEN** all segments, Settings, brand and concise status remain readable and operable, with existing workspace controls reachable and no redundant mode dropdown

#### Scenario: Export remains a foundation workspace
- **WHEN** Export is selected before its dedicated delivery capability is implemented
- **THEN** the header reports dry DAW passthrough and explicitly initiated supported preview honestly, with no false ready-WAV or drag/export action

### Requirement: Header controls receive incremental owner acceptance
New or materially revised header control families SHALL receive explicit owner approval of appearance and interaction feel before wider rollout. Representative native/host examples SHALL cover focus, activation, disabled feedback and scale; appearance approval SHALL NOT imply untried interaction or host/DPI acceptance. Integrated header approval SHALL precede final host sign-off.

#### Scenario: Component checkpoint before integration
- **WHEN** the segmented selector and Settings action are ready in a representative control fixture
- **THEN** the owner is asked for look-and-feel feedback, requested adjustments are resolved, and approval or untested behavior is recorded before dependent integration proceeds

#### Scenario: Integrated checkpoint
- **WHEN** the approved controls are integrated into the shared header across modes
- **THEN** the owner reviews composition and interaction at supported sizes before final FL Studio acceptance, with remaining gaps explicitly open

#### Scenario: Minimal host acceptance
- **WHEN** automated component/state/processor/persistence checks and native owner checkpoints pass
- **THEN** FL Studio UAT is consolidated into one brief final control-feel, mode-journey and save/reopen smoke test; detailed edge cases rely on automation, with extra host checks requested only for failures or specific unresolved host behavior
