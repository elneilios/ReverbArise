# Design

## Context

Production standards revision 22 is the baseline. The Swells list already uses a virtualized ListBox with scroll anchoring by stable ID. Row action hotspots are painted, and the row menu calls the single-target deletion path. Shared dropdowns and action menus use ChoicePopup. FocusViewport reveal is already enabled in the inspector, Settings and Diagnostics.

The owner approved the standalone JUCE preview on 10 November 2026 after trying the long list, compact inspector, square checkboxes, confirmed group delete and disabled hover on both the Swells menu and IR dropdown. The fixture used sample data. Production bindings, screen-reader behavior and FL Studio hosting were outside that preview.

## Goals / Non-Goals

**Goals**

- Keep the virtualized list and its selection, explicit reveal and refresh contracts while giving row actions real input and accessible semantics.
- Bind one destructive action to a captured set of targets and use the existing deletion and suppression semantics at the model boundary.
- Apply the approved popup state correction once for all consumers of the shared presenter and verify focus reveal across the scoped viewports.

**Non-Goals**

- A new list framework, alternate waveform navigation, new application scaling or shell behavior, or a migration of native OS choosers.
- Changes to saved project format, render algorithms, capture, or unrelated P8 slices.

## Decisions

1. Keep the existing SwellList virtualization and selection binding. Reuse the checkbox painter and behavior from BooleanToggle, shared command input and focus visibility for the three-dot action, and ChoicePopup for the menu. Give the square checkbox its full 18 × 18 indicator and reserve room for the brackets. Tab enters the primary row's two actions and then the inspector, with reverse traversal. Accessible row actions identify the swell by name. Inactive rows remain discoverable without making every row a Tab stop. Route right-click anywhere on a row and the three dots through the same captured action. Focus or row action invocation does not request waveform reveal. Resolve ownership and focus by stable ID and epoch across scrolling, recycling and reorder.
2. Capture the clicked row, selected IDs, session and relevant edit context when opening the menu. If the row is selected and the captured selection has several IDs, label Delete with that count and open an explicit confirmation using the shared neutral Cancel and destructive Delete buttons. Default focus is Cancel. A row outside the selection has single-target Delete. Rename stays disabled throughout a multiselect, with its dimmed label and accessible unavailable state. Recheck Rename eligibility on invocation and before a delayed edit commits.
3. Route group deletion through one guarded engine operation that accepts the captured IDs. Reuse existing session, edit eligibility, target suppression, runtime invalidation and persistence hooks. Check the full group, including suppression capacity, under the existing model lock before publishing any deletion. If necessary, stage the same session operation before publishing to make rejection atomic. Re-reading current selection or repeatedly issuing the existing single-row UI command would let selection changes or a later failure alter the result. Preserve unrelated selected IDs and the surviving primary or nearest valid focus fallback. Cancel issues no engine call.
4. Filter unavailable pointer targets once in the shared popup presenter for choice rows and actions. Keep keyboard inspection independent, existing dropdown reasons and checkmarks intact, and reasons for unavailable actions that need them. Remove the unused gutter in short popups and reserve scrollbar space only when content overflows. Test the actual dropdowns and action menus that consume this presenter, including IR, Export mix, Settings and knob Reset. Audit reachable dropdowns with disabled options for parity, keeping native file choosers separate. Use the existing palette and focus tokens.
5. Keep focus reveal on the existing FocusViewport usages in the compact inspector, Settings, nested Storage content and Diagnostics. Verify complete focus ink, fixed inspector Reset access, and normal panel wheel scrolling while honoring established input wheel policies. Correct only gaps found in those consumers.

## Risks / Trade-offs

- Virtual rows recycle across selection changes and while a menu is open. Keep binding authority and pending popup ownership outside recycled children and reject stale callbacks.
- Automatic swells use suppression records. Preflight the batch and publish only a complete accepted edit so one rejected member cannot leave a partially deleted group.
- Measuring with and without the scrollbar changes line wrapping. Verify both widths and input modes at normal and scaled sizes.
- Preview approval covers the agreed design. Verify the integrated bindings, Windows accessibility semantics and host coverage separately. Reopen scoped design review if the implemented interaction materially differs.

## Migration Plan

Land the shared popup correction, row actions, guarded deletion and focus viewport verification together as the P7 list and scrolling slice. Use the approved native fixture as the design checkpoint and prove the integrated editor behavior before review. Update standards, inventory, user guide and evidence during apply. Rollback requires no project conversion.
