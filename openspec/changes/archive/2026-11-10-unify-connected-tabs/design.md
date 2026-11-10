# Design

## Context

The C7 inventory identifies three `SegmentedSelector` consumers (H2, L5 and P5) and the Settings `juce::TabbedComponent` (S7). The segmented control already separates inspected and accepted choices. Settings currently constructs Playback through About from `settingsPages` and appends the Storage viewport separately. The approved native comparison established a common connected strip and put About after Storage.

## Goals / Non-Goals

**Goals**

- Share tab rendering, input handling and accessibility across the four strips. Keep page selection and workflow rules with their existing bindings.
- Preserve Settings page instances, auxiliary window scaling and viewport behavior while making tab order independent of page identity.

**Non-Goals**

- Swell list interaction, viewport/scrolling policy, and independent feature work are outside this child change.

## Decisions

1. **Extend the shared segmented control.** Reuse its navigation, accepted display and guarded request model for all four strips. Keep responsive text roles, existing workflow icons and accessible names. The strip owns the connected outline and seam paint order, with room for the focused tab's brackets outside its body. Reuse named theme roles, the shared focus visibility policy and bracket painter. Use the preview to check selected, hover and held colors together before registering any additional token or variant.
2. **Keep inspection local and acceptance in the binding.** Keyboard navigation moves focus within the strip. The existing workflow, Learn and inspector bindings decide which requests take effect and refresh the accepted selection. Settings accepts eligible page requests and updates its page and strip together. Programmatic refresh and selection update the display without invoking a tab action. Keep Tab traversal through the active page and ensure hidden pages cannot capture focus.
3. **Use a stable Settings page identity.** Bind the shared strip to the existing page components via one ordered mapping with About as the terminal entry. Keep the existing Appearance page before Storage when responsive presentation supplies it. Preserve the content host and viewport responsibilities. Route the direct Storage action through its page identity, and update tests that currently depend on JUCE tab internals or About's old index. Preserve the initial Playback page and existing behavior when the window closes or reopens.

## Risks / Trade-offs

- Focus brackets can clip against adjacent tabs or content in the compact layout. Check combined selected, pointer and focus states at responsive breakpoints and at 100% and 125% UI scale. Keep the existing #58 clearances and logical-to-physical scaling path.
- A visual or accessibility change could bypass #35 workflow guards or expose an inspected tab as selected. Check accepted and rejected requests, exactly-once activation, unavailable reasons, and the reported selected state through the same shared path.
- Reordering the strip could break Storage shortcuts or retarget tests to the wrong content. Assert page identity and visible content separately from display order, including the Storage viewport and About's build information actions.

## Migration Plan

Exercise the shared control in one native JUCE preview with representative editor strips and the real Settings layout constraints. Carry forward the approved design and use this checkpoint to verify the revised component before wider rollout. Then adopt it in the four strips, verify the bindings, and record the implemented contract and test evidence. Reverting the rollout restores the prior strips without a data or session migration.
