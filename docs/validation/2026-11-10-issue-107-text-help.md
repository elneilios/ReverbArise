# Issue #107 P3 text, help and notices

## Approval and baseline

The owner approved the six-surface native text specimen and its final softer
aqua footer help on 10 November 2026. The candidate uses existing #00C2CC ink.
Its sample-data layout checks covered Learn, Perform, Export, Settings,
Diagnostics and Storage/Recovery, five scenes, two sizes and 100%/125% scale.
Production binding and actual host/DPI coverage require separate evidence.

The implementation baseline is main at `a9302bb`, merged P8 PR #145.
Peak-guard PR #146 merged on the independent Loop branch. Its DSP, processor
and guide changes remain owned there. P3 preserves message authority when the
branches meet.

## Consumer dispositions

| Group | Presentation and verification |
| --- | --- |
| Header, mode headings and transport | Existing semantic paint, HeaderInformation, NoticeBar and TransportReadout retained and verified |
| Editor hoverHelp | Shared helper role with the approved aqua information ink |
| Learn captions and scope/help | Shared caption and measured wrapped labels, existing field feedback retained |
| Perform headings/counts/status and empty states | Shared text roles and explicit single-line or wrapped presentation |
| Export captions, legend, readiness and status | Existing role-based canvas text, fixed readout and notice authority verified, overflow corrected within their existing composition |
| Settings captions, help, About and Appearance | Shared single-line and measured wrapped labels, existing local field feedback retained |
| Diagnostics detail and captions | Measured multiline layout, full-copy values and existing scroll/focus container |
| Storage inventory, portable backup and working folder | Existing semantic roles, measured wrapped prose and full-description single-line paths |
| Native recovery alert | Retained JUCE AlertWindow presentation with existing guarded actions |
| Hidden compatibility labels | Retained hidden with no new controls exposed |

## Shared implementation

`WrappedLabel` extends the existing single-line label family. Its measured
height and paint path use the same text layout, including character wrapping for
unbroken paths. Ordinary text changes update full descriptions and context
without replacing a binding's custom explanation. Unchanged refresh stays
silent. `contextHelpInk` names the approved softer aqua footer role.

Settings pages use the existing FocusViewport policy for text-driven content
height. Diagnostics messages are measured before following rows are placed.
Learn and Export allocate measured help/status height within their existing
composition. Storage panels use the shared measurement for their current scroll
content, retaining pinned actions and operation ownership.

The source guard's exact debt record drops nine migrated editor font
constructions and both Settings font constructions. Two hidden editor label
setups retain their existing disposition. The inventory was not regenerated.

The Learn UI fixture previously checked for manually inserted newline
characters and a fixed helper box. It now checks actual wrapped line count and
measured height, retaining its content, font, spacing and model assertions.

The first integrated Storage run caught an extra keyboard stop introduced by
the Settings page viewport. The viewport now leaves Tab navigation to the
selected page's controls while retaining focus reveal. The component case
checks that policy, and the production Storage fixture exercises Tab from the
connected tabs into About before its guarded asset operations.

## Validation results

Validation used the warm MSVC Release build in this worktree at `a9302bb`
plus this change. The supported `scripts/build.ps1 -Configuration Release -Test`
configured and built successfully; CTest initially passed 96/99 cases in
386.98 seconds. Its logs are retained under
`out/build-logs/Release/20261110-230538-16095af2/`.

The three failures were resolved individually: the Learn fixture now checks
measured wrapping instead of inserted newlines; the Settings viewport no
longer takes an extra Tab stop; the unchanged Reset test passed when launched
as a visible native validation window rather than background CTest.

After those corrections, `scripts/build.ps1 -Configuration Release` passed
(logs `out/build-logs/Release/20261110-231357-56270cef/`). The six affected
CTest cases all passed in 76.07 seconds: Learn generation UI, Swell UI including
shared text component cases, Perform hover help, responsive editor, responsive
scale and Storage Settings. See `out/build-logs/p3-text-focused-tests.log`.
The visible `preverb_perform_composition_preview --test-knob-resets` retry
passed with exit 0; see `out/build-logs/p3-text-native-reset.log`. These are
supplementary reruns, not a second full 99-case run.

The broader run also passed actual editor composition, readability, diagnostics,
Export controls/bindings, long-text Storage inventory, portable storage,
working-folder and recovery tests, plus audio and persistence regression cases.
Integrated responsive cases exercise minimum/larger layouts and 100%/125%
software scale. The approved six-surface specimen supplies the normal, long,
error, unavailable and empty design matrix; binding tests supply production
message/operation checks. No material deviation from that design was introduced.

UI standards passed across 85 files with 48 explicitly bounded occurrences;
strict OpenSpec validation, user-guide currency and `git diff --check` passed.
FL Studio, physical Windows DPI changes and a live screen reader were not
retested for this presentation-only change. Accessible metadata and unchanged
refresh are covered by component assertions; that is not a live reader claim.
