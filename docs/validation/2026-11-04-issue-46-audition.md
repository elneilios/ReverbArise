# Issue #46 — audition refinement, 4 November 2026

The owner reported that the host editing looked/felt good, then requested finite
audition looping, a Wet only toggle default Off, removal of the row Trim reset
and a status review. Their balance clarification is the level of DAW transport
playback, not an equal crossfade. This explicit addition stays within selected
Perform audition; #5 arrangement playback and Export implementation remain separate.

## Implementation and boundaries

- AuditionVoice has an opt-in per-start loop flag; SwellEngine manual audition
  enables it. PreviewEngine defaults to one-shot; ExportVoice is unchanged.
  Cursor wraps within the pinned immutable buffer, retaining existing short edge
  ramps and callback ownership/retirement. Stop, rate/permission invalidation and
  DAW transport priority still stop it. A shorter replacement window restarts at
  its own start if the old cursor lies outside, without silently ending the loop.
- New instances use unity captured dry plus saved Live Wet gain. Wet only uses
  that wet contribution without captured dry. Existing audition wet/sum protection
  can attenuate the result and is disclosed. This matches nominal branch levels;
  it does not claim equality with downstream DAW processing or a different global
  membership/protection context. No live routing or Export mix control is changed.
- Optional schema-10 `auditionFollowsLive` defaults false when absent. Old projects
  retain mode/blend/trim until an explicit toggle; new instances initialize true
  and Off. Live-level mode retains but ignores legacy trim, with disabled/explained
  Settings trim. Legacy trim remains resettable there. Envelope version stays 8.
- Wet only reuses accepted LatchingActionButton unchanged. There is no new token,
  family painter or global density change. Its width derives from existing value
  width/related spacing. The old blend slider and row trim reset are removed.
- Ready/Looping/Preparing/Unavailable copy follows actual preparation/voice state;
  old-context errors are not presented as new-mode failures while preparation is
  pending. Missing dry suggests capture in Learn, with exact ranges in tooltip.
  Existing EllipsisLabel preserves helper font proportions. Cached wet, saved
  legacy balance and wet/mix protection remain disclosed.
- Session epoch/workflow-pass and disabled-state checks guard delayed toggle
  actions. Restores do not autoplay. Live Wet changes reprepare monitoring without
  convolution; saved musical/source/IR/render identities remain unchanged.

## Native validation history

Warm Windows/MSVC/JUCE caches retained; no dependency download or cache clearing.
Native results below are distinct from native fixture software scaling and actual
FL Studio/Windows DPI acceptance.

Initial Release wrapper (`20261104-194936-f68c91a1`) built successfully and ran
**51/54 passing**, 196.24 s. Failures were: a stale voice cursor after immediate
restore invalidation; the historical M2 processor oracle waiting for Wet Only
readiness despite the new Dry + wet default and uncaptured tail; and a native
reset popup missing during its timed focus setup. Snapshot publication now masks
invalidated playback immediately. Historical numerical/resource fixtures select
their old Wet Only semantics explicitly, while M2 separately asserts the new
instance default and continued looping. The popup fixture now establishes and
asserts native dial focus before opening its real menu; production reset code is
unchanged.

Focused iteration first caught a test lookup for a Settings control while the
Settings window was closed, then a mode-change test attempting Export before its
restored dependencies were ready. The test now proves inactive trim through peak
invariance and waits for actual readiness, preserving the production workflow gate.
Final focused iteration at that boundary: **5/5 passed**, 14.30 s. Logs:
`out/build-logs/Release/20261104-audition-focused-build-v3.log` and
`20261104-audition-focused-tests-v3.log`.

Additional final coverage includes unavailable dry with explicit Wet only recovery
and distinguishing pending preparation from stale error text. Final full native
results and exact build identities follow after the final wrapper run.

Tests cover repeated sample oracles and bounded positions for 1/2/3/19/100-frame
buffers, multiple wraps per block, allocation/deallocation guards, smooth/immediate
Stop, incompatible rate/permission, replacement and pin collection. Live-level
sample oracles compare both modes with TransportPlayback at -12/0/+6 dB below
protection. Legacy and malformed decoder cases, actual processor state round trip,
keyboard toggle, stale/disabled invocations, no autoplay, no convolution/assets
and Ready/Looping/empty/unavailable status have dedicated checks.

UI standards: 46 files / 61 bounded occurrences; unchanged debt inventory. Strict
OpenSpec: 20/20 at the implementation boundary. `git diff --check` passes.
Final host audition listening and Windows DPI coverage remain pending; earlier
layout, Snap and knob approvals are retained.

The full native Release wrapper at source `9060aab` passed **55/55**, **150.59 s**,
logs `out/build-logs/Release/20261104-200224-fee7b670`. This run included all new
binding, persistence and unavailable-source checks. During its review, the shorter
replacement handoff was tightened further: outgoing and incoming buffers now use
their own existing edge gain during crossfade, so restarting at a shorter window's
start cannot mute the outgoing sample abruptly. An explicit first-sample continuity
and allocation oracle accompanies that correction. A final wrapper run validates
that exact follow-up source before delivery.
