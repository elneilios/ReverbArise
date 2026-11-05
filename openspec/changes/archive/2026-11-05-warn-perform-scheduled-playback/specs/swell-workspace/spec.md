# Spec Delta

## MODIFIED Requirements

### Requirement: Important notices use a conditional inline message bar
Important notices SHALL use a nonmodal reserved row beneath the branding, with readable severity and accessible detail. One message SHALL NOT shift the workspace, cover controls or steal focus. The global-settings divider SHALL remain visible. Routine idle/ready information SHALL be absent; relevant notices SHALL remain until resolution. Perform SHALL retain its safety fallback when no other notice is active. Identical refresh SHALL NOT repeat announcements.

#### Scenario: Clean idle workspace
- **WHEN** Perform is idle with no other notice, or Learn is idle with Recapture off and no relevant error
- **THEN** Perform shows its safety warning while Learn retains its clean message-free state, and the normal compact layout is retained in both

#### Scenario: Recording and failure
- **WHEN** recording, source validation, Recapture arming or a relevant failure requires attention
- **THEN** the highest-priority current notice is visible beneath the branding, its text and severity are accessible, and all existing controls remain reachable at the minimum window size
- **AND** the divider above global settings stays visible below the reserved row in both notice and clean states

#### Scenario: Notice resolves
- **WHEN** the underlying higher-priority notice condition clears
- **THEN** Perform restores its safety fallback while other modes follow their existing notice policy, without changing layout or parameters, moving audio or interrupting an active control edit

## ADDED Requirements

### Requirement: Perform explains independent prepared playback
Perform SHALL show a persistent warning-level fallback explaining that prepared swells can remain audible during DAW playback independently of source audibility. Other current actionable, error and transient progress notices SHALL take precedence, regardless of severity. The fallback SHALL resume when they clear and provide accessible/help detail about saved Song timing, Pattern playback and explicit output controls, without implying detected host state.

#### Scenario: Remain in Perform
- **WHEN** Perform is selected with no higher-priority notice, including while stopped, playing, empty or with live playback disabled
- **THEN** the reserved message bar shows “Caution: In Perform, swells can remain audible during DAW playback, even if the source is silent or muted.” at warning severity without a timeout or dismiss preference
- **AND** its full help/accessibility detail explains that saved Song-positioned swells can precede notes and continue through silence, Pattern playback or upstream Playlist/instrument mute may not stop them, and effect bypass or mixer mute can stop the swells

#### Scenario: Temporary message takes precedence
- **WHEN** a current error, actionable warning or transient preparation notice becomes applicable in Perform
- **THEN** that notice replaces the fallback according to existing notice priority, including information-level IR preparation after its existing debounce
- **AND WHEN** the higher-priority notice clears
- **THEN** the warning returns without a mode switch, focus change, control interruption or layout movement

#### Scenario: Switch modes or reopen
- **WHEN** the user enters Learn or Export, returns to Perform, or reopens the editor in Perform
- **THEN** Learn and Export retain their existing notices, and Perform recomputes its current warning/override without persisting a new preference

#### Scenario: Refresh and inspect safely
- **WHEN** the warning is repeatedly refreshed, resized or inspected with accessibility or context help
- **THEN** its warning semantics and full explanatory detail remain available without repeated identical announcements, clipped controls or additional layout space
- **AND** rendering, capture eligibility, Learn Pattern capture, dry routing, live/audition enable, source/render identities and saved state remain unchanged by presentation
