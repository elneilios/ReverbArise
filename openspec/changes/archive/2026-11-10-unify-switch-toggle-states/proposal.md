# Proposal

## Why

Learn switches and the legacy Perform and Settings checkboxes handle hover, press and focus differently. P6 under GitHub issue #107 applies the approved comparison preview across the plugin while keeping the Swell Inspector's flags compact.

## What Changes

- Use labeled switches for on/off settings in Learn and Settings where there is room. Keep Enabled, Solo and Freeze as inline checkboxes in the Swell Inspector so their labels and states are easy to scan.
- Give both styles quiet hover, distinct held feedback, thin outer aqua corner brackets for keyboard or accessibility focus, and readable unavailable states. Clicking the label or indicator uses the same action.
- Preserve immediate application, accepted-state refresh and existing workflow, pass and selected-swell guards.
- Review both shared controls in one production-component native preview before rollout. Record the disposition of every P6 consumer, including hidden compatibility controls.

## Capabilities

### New Capabilities

### Modified Capabilities

- `ui-design-system` — define the switch and inline checkbox roles, their shared interaction states and their guarded activation.

## Impact

- Covers Learn capture controls, the three Perform inspector flags, and the visible on/off controls on the Playback, Processing, Swell, Audition and Recovery Settings pages.
- Retains the hidden Diagnostics and exact-coordinate controls as compatibility seams. The Swells list row enable target remains with the list family. Existing command latches, #8 recovery behavior, DSP and saved project formats are outside P6.
