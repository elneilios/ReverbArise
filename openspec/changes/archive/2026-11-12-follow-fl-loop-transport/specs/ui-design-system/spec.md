# Spec Delta

## ADDED Requirements

### Requirement: Action Button and State Field share a joined presentation

The control library SHALL provide a reusable Action Button + State Field. The action SHALL use the shared command semantics and occupy the left segment. The right segment SHALL show read-only state with a steady surface. The visible bodies SHALL share a continuous perimeter, a single seam and aligned top and bottom edges.

#### Scenario: The state text changes length

- **WHEN** the state text changes length
- **THEN** the pair keeps the same field width, using its default or the implementor's override within the available space
- **AND** the action and neighboring controls keep their positions

#### Scenario: A long value overflows

- **WHEN** the text extends beyond its field
- **THEN** the text glyphs fade at the clipped edge while the field surface, seam and outline remain steady
- **AND** hovering the field scrolls the text slowly to reveal its end, and leaving the field restores the leading text

#### Scenario: Command feedback changes beside a steady readout

- **WHEN** the action moves between idle, hover, held and disabled states for a supported command variant
- **THEN** its segment uses the existing variant feedback and the field retains its surface, legibility and accepted value
- **AND** the shared perimeter and seam stay continuous at supported display scales
- **AND** a disabled pair uses the disabled action's border colour for the field outline

#### Scenario: Keyboard and pointer focus affect the action

- **WHEN** keyboard navigation or accessibility focus reaches the pair
- **THEN** the action is the only tab stop when eligible and its brackets identify only its segment
- **AND** pointer activation hides the brackets under the shared focus policy while preserving the appropriate hover and held feedback

#### Scenario: The user interacts near the seam

- **WHEN** the eligible action segment is activated by pointer, keyboard or accessibility press
- **THEN** the shared guarded command path receives at most one invocation
- **AND** pointer input on the field does not invoke the action

#### Scenario: State changes independently of the command

- **WHEN** a command is pending, rejected or temporarily disabled
- **THEN** the field displays the latest accepted state, or explicit unavailable text if no state has been accepted
- **AND** the field exposes the full value as static accessible text and can disclose it through the host workspace's fixed hover-help footer
