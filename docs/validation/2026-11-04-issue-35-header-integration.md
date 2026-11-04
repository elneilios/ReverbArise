# Issue #35 — approved header integration

On 4 November 2026 the owner approved the native candidate checkpoint with
“Approve and continue Issue #35”. This accepts the reviewed appearance and
interaction checkpoint. Windows scale/monitor transitions and FL Studio coverage
remain untested; approval does not invent that coverage.

The approved preview (process 53744) remained open in the elevated context and
was initially missed by the restricted process inventory. Its executable lock
caused the first integration link attempt to fail. After approval it was closed
gracefully via its window; the wrapper was then rerun. Candidate implementation
and test evidence are preserved in
[the 3 November report](2026-11-03-issue-35-header-navigation.md).

## Implemented integration

`WorkflowHeader` supplies semantic Learn/Perform/Export icons explicitly and
composes the approved `HeaderInformation` primitive in the existing bounds.
Perform uses “Shape and place reverse swells with precision”; Export retains
“Preview your prepared swells”. The editor opts its existing workspace notice
into the matching body ink. The review fixture now uses this production header
directly, with no candidate description overlay or alternate mode binding.

Segment sizing, focus/border painting, accepted selection, Settings and workspace
allocations are preserved. No audio, worker, workflow authority, parameter or
saved-state implementation changes. Non-header notices retain their defaults.
The global palette and legacy typography constants are unchanged.

## Verification

- `scripts/build.ps1 -Configuration Release -Test`: **38/38 CTest tests passed**,
  57.96 seconds; native MSVC build/configure passed. Logs:
  `out/build-logs/Release/20261104-013734-6314add3/`.
- UI coverage includes icon/no-icon native interaction, keyboard/accessibility,
  disabled/hover/focus states, shared-border clipping regressions, rejected/stale
  requests, regular embedded Inter and scoped notice appearance. Existing
  processor, source, capture, audio, persistence and feedback suites also pass.
- Production editor geometry was checked at 1100×850, 1400×900, 1400×1000 and
  1600×1000. Header widths 1068/1199/1200/1360/1560 cover the navigation-width
  boundary. All three modes, capture phases, failures and short/long severity
  notices retain their reserved bounds and complete accessible detail.
- Software raster/component samples cover 100/125/150/200% scale; glyph checks
  cover 14/16/20/24/32 px. These do not establish actual OS/host DPI acceptance.
- Native production screenshots inspected:
  [Export, 1100×850](2026-11-04-issue35-production-export.png),
  [Perform, 1400×900](2026-11-04-issue35-production-perform.png),
  [Learn with IR error](2026-11-04-issue35-production-learn-error.png).
  These preserve the approved control composition in real editor states.
- Living UI standards, layout/theme guidance and mode guidance now describe
  the implemented icon roles, regular information row and header-only ink.

## Review and coverage limits

Owner native component look/interaction approval is recorded above. Actual
Windows 100/125/150/200% scaling, fractional desktop origins, monitor transitions
and FL Studio acceptance of this header revision were **not performed**.
Debug was not built for this presentation-only integration. No new host-specific
behavior was introduced; any owner host check can be a single brief navigation,
Settings and notice smoke test at their usual scale, rather than repeating the
passing automated state/audio/persistence cases.

The existing OpenSpec change remains active for owner review. No canonical-spec
sync, archive, issue closure or PR merge is performed by this implementation.
