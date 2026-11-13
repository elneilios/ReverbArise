# Spec Delta

## ADDED Requirements

### Requirement: Learn and Perform share the same shaping controls

Learn Sound and Perform Sound SHALL expose Level, Damping, High Pass and Low Pass through the same shared Sound editor for every preset and Custom IR. Preset-specific controls and contextual IR-file import SHALL remain available as applicable.

#### Scenario: Open Sound in either workspace
- **WHEN** a user opens Sound for an eligible assignment
- **THEN** the four shared knobs use the same labels, relative order, value formatting and interactions in both contexts
- **AND** the compact inspector exposes all four controls through its existing scroll and focus behavior

#### Scenario: Edit or reset a shaping knob
- **WHEN** a user drags a cutoff, enters an exact value, uses keyboard or accessibility input, or resets a knob
- **THEN** the cutoff dial traverses frequencies logarithmically and its accepted value displays in Hz or kHz, or as Off
- **AND** reset returns the selected knob to its neutral default
- **AND** the editor previews a drag immediately and submits a render-changing edit once the gesture settles

#### Scenario: Edit defaults or one swell
- **WHEN** a user changes a shaping control
- **THEN** Learn edits the accepted future defaults, while Perform edits only the selected swell
- **AND** unavailable or frozen controls use the established disabled interaction and contextual help

#### Scenario: A render-changing edit is pending
- **WHEN** a shaping edit needs background preparation
- **THEN** the prior accepted sound stays audible until a verified replacement is available
- **AND** the existing delayed Sound preparation overlay and actionable failure feedback apply
