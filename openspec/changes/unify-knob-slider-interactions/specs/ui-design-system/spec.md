# Spec Delta

## ADDED Requirements

### Requirement: Knob and slider feedback separates value, pointer and focus
Migrated knobs and slider tracks SHALL use an aqua value arc or fill, an off-white position indicator, and quiet pointer hover and held feedback. Keyboard and accessibility focus SHALL show thin outer aqua corner brackets with a gap outside the control body. Pointer focus SHALL retain interaction without the brackets. Unavailable controls SHALL remain readable and ineligible for edits.

#### Scenario: Compare the same control by pointer and keyboard
- **WHEN** the user hovers, holds, clicks or tabs to a dial or slider track
- **THEN** hover and held feedback identify pointer interaction, and only keyboard or accessibility focus shows the outer brackets
- **AND** the indicator continues to reflect the accepted value

#### Scenario: Hover or hold a dial at any value
- **WHEN** the user hovers over or holds a dial with the pointer
- **THEN** quiet feedback follows the arc's sweep and leaves its centre and the gap between its ends clear

#### Scenario: Focus a track that has a numeric field
- **WHEN** focus moves between a slider track, its value field or its steppers
- **THEN** focus feedback identifies the actual focused control, with the P4 field and stepper behavior preserved

#### Scenario: Resize a dial with a companion value field
- **WHEN** a dial with an exact-entry field is laid out at different widths
- **THEN** the field uses the shared value width wherever space allows
- **AND** inline feedback aligns beneath the field and wraps within its value column

#### Scenario: Discover the Target entry format
- **WHEN** the user hovers over the Target dial or field
- **THEN** the hover-help footer explains the entry format without adding height to the Timing row
- **AND** the accessible field description exposes the format and validation errors still appear beneath the field

### Requirement: Knob and slider wheel adjustment has a fine mode
Migrated dials and tracks SHALL define coarse and fine wheel steps in their domain. Ctrl+wheel SHALL use the fine step where a smaller legal step exists. A discrete control SHALL keep its minimum legal step. Wheel adjustment SHALL honor direction, range and eligibility. Wheel input over numeric steppers or outside the dial or track SHALL follow its existing policy.

#### Scenario: Fine-adjust Level or a percentage track
- **WHEN** the user compares a normal wheel step with Ctrl+wheel over Level, Learn Event sensitivity, Export Dry or Export Wet
- **THEN** Level steps by 1 dB or 0.1 dB, and the percentage tracks step by 1 or 0.1 percentage points respectively
- **AND** Ctrl on a continuous track does not change ordinary pointer drag

#### Scenario: Fine-adjust snapped timing
- **WHEN** a timing dial is eligible and Snap is on
- **THEN** plain wheel follows adjacent grid points, while Ctrl+wheel uses an unsnapped increment for that gesture
- **AND** the Snap setting, existing Ctrl-drag bypass and established Shift fine behavior remain available

### Requirement: A knob can have an optional drag-only home detent
A knob configured with a home value SHALL attract a drag to that value within its configured band. Continued movement beyond the band SHALL continue the drag. A knob without a home SHALL keep its existing drag behavior. A home detent SHALL apply only to pointer drag and SHALL preserve the control's independent Reset action.

#### Scenario: Drag Perform Level through neutral
- **WHEN** the user drags Level toward 0 dB from either direction
- **THEN** the dial settles at the marked 12 o'clock position when the drag candidate is within ±0.3 dB
- **AND** dragging beyond the band moves past the detent

#### Scenario: Enter a Level close to neutral using another input
- **WHEN** the user enters or steps to a valid Level within the detent band using the wheel, keyboard, accessibility or exact entry
- **THEN** the value follows that input method's normal rules
- **AND** the detent does not round the value to 0 dB

### Requirement: Knob and slider gestures preserve their existing authority
Migrated controls SHALL preserve captured invocation authority, value limits, supported saved values, and reset, cancel and refresh rules. Stale or unavailable gestures SHALL publish no edit. Display-only controls SHALL change only the presentation they own.

#### Scenario: Selection or session changes during a gesture
- **WHEN** a drag, wheel action or Reset loses its captured swell, session, timing or workflow authority
- **THEN** it cannot change a replacement target or session
- **AND** refresh displays the accepted state without publishing an edit

#### Scenario: Adjust a display control or reopen a legacy Level
- **WHEN** a user changes zoom or Height, or opens a project with an older saved Level
- **THEN** zoom and Height affect only the display, and the saved Level follows the established display and first-touch behavior
