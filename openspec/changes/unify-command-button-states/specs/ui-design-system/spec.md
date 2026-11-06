# Spec Delta

## ADDED Requirements

### Requirement: Migrated command states follow explicit semantics
Migrated text and glyph command actions SHALL share reviewed, distinguishable idle, hover, pressed, focus and disabled feedback for equivalent semantic roles. Primary, destructive and latched variants SHALL remain explicit. Names and labels SHALL NOT select appearance. Neutral glyphs SHALL retain steady ink while border/background provide mouse feedback; semantic filled variants SHALL retain readable contrast. Specialized file-drag presentation SHALL retain its scoped behavior.

#### Scenario: Command metadata changes
- **WHEN** a migrated action is renamed or relabeled without changing its explicit semantic role
- **THEN** its state surface and content role remain unchanged apart from the intended text and accessible-name update
- **AND** an incidental name matching an old play, stop, reset or import identifier does not change its glyph or variant

#### Scenario: Focus and disabled combinations are rendered
- **WHEN** a migrated command is hovered, pressed, keyboard-focused or disabled, including applicable combined states
- **THEN** its reviewed state matrix provides contained focus and consistent semantic content/surface feedback
- **AND** disabled state does not make the action appear eligible or permit invocation
- **AND** focus uses aqua outer corner brackets with a small gap from the button border and does not mask hover or held-press feedback, including on destructive actions

#### Scenario: Shared IR import button aligns with its selector
- **WHEN** the shared IR field lays out its selector and import action
- **THEN** the import button's visible body matches the selector's top, bottom and height
- **AND** its original folder glyph fits within that body, the outside focus frame remains contained, and field feedback retains its existing gap below the inputs

### Requirement: Command activation preserves binding authority
Migrated commands SHALL support pointer, Return, Space and applicable accessibility activation through the same eligible command path. One activation SHALL issue at most one command. Disabled, stale or destroyed targets SHALL NOT gain permission through a new input path. Existing command effects, target/session guards, persistent data and audio behavior SHALL remain unchanged.

#### Scenario: Keyboard or accessibility invokes a selected action
- **WHEN** an eligible migrated action is activated through Return, Space or its accessibility press action
- **THEN** it issues the same guarded command as pointer activation exactly once
- **AND** the existing target/session and asynchronous completion rules still determine acceptance

#### Scenario: Activation is no longer eligible before delivery
- **WHEN** a migrated action is disabled, destroyed or its captured context becomes invalid before queued delivery
- **THEN** no stale command mutates another target or session
- **AND** existing model rejection and reconciliation remain authoritative

### Requirement: Latching commands preserve accepted effective state
Latching command presentation SHALL use the shared command state language while distinguishing accepted effective on/off state. Programmatic refresh SHALL be silent and SHALL NOT rewrite preferences or issue another command. Requested state and temporarily overridden effective state SHALL remain distinct where the current binding distinguishes them.

#### Scenario: Effective latch state refreshes during interaction
- **WHEN** Snap or audition listening state refreshes from its accepted binding state
- **THEN** the latch displays the accepted effective state without another invocation
- **AND** hover, press, focus and disabled rendering follow its reviewed on/off matrix without changing requested preference semantics
