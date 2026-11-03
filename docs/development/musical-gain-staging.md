# Musical gain staging

New ReverbArise instances use the listening-approved **Source energy +6 dB**
balance. Swell **Level at 0 dB** means that calibrated starting point. Level
is adjustable from -60 to +12 dB. Reset wet shape returns Level to 0 dB and
normal timing/fade defaults without accumulating gain or recalibrating edits.

Calibration compares the strongest 20 ms RMS window in the local dry attack
with the strongest equivalent window in default shaped wet, then adds the
approved +6 dB target. The dry reference stops at 100 ms, source end or next
confirmed target, whichever comes first. Windows advance every 1 ms, including
the last valid window. A short reference uses a common shorter window for both
signals. One gain applies to both stereo channels. Quiet events produce quiet
swells, while arbitrary IR recording levels receive compensating calibration.
Tone, stereo width and decay still determine character.

This measures short-window intensity, not perceived loudness. Calibration stays
fixed while you edit Level, fades, tension, crop or wet position. Genuine source,
IR, extraction or reference changes recalculate it. Freeze saves the accepted
measurement with its audio, including when source is absent on reopen.

Dry/wet RMS floors are -60 dBFS. Baseline gain is bounded from -60 to +24 dB.
An extreme quiet IR can reach the cap. Silence stays silent. Missing or quiet
references report **Calibration unavailable** and apply no automatic baseline
boost. Capture the source and regenerate (unfreeze first if needed) to measure.

The actual positioned wet sum receives one fixed attenuation only when its
sample peak would exceed **-1 dBFS**. Level changes below this ceiling produce
the requested amplitude ratio. Once protection acts, further gain can be
ceiling-constrained; status/Diagnostics separates Level, calibration and
protection. This protects wet samples, not the final live dry-plus-wet sum or
true peaks. IR and raw convolution files are never rewritten by gain staging.

## Existing projects and rollback

Versions 1-7 retain **Legacy** with saved edits, -6 dBFS guard and original
output. Select **Settings > Playback > Gain staging > Musical (source energy
+6 dB)** to adopt explicitly. Numerical edits and assets are preserved; saved
Level becomes an adjustment around the new calibration. Adoption stops audition
and prepares eligible swells without reconvolution. Frozen swells lacking a
compatible measurement need source and unfreezing before calibration is possible.

Musical Live wet supports -60 to +12 dB; Legacy supports -60 to 0 dB. Before
returning to Legacy, lower positive Live wet to 0 dB or below. The switch rejects
an invalid level instead of clamping it. Save/reopen preserves policy, edits and
measurement without starting playback. Saved peak/zero-offset energy prototypes
keep their original revision and appear as **Saved musical prototype**; selecting
the approved Musical item is explicit adoption.

Session JSON is version 8 within the existing version-7 plugin wrapper. Older
binaries cannot interpret new gain revisions. Keep a pre-upgrade project copy
if you need rollback to an older binary.

## Audition, live playback and export

All three paths use accepted calibration and Level. Equal Wet Only membership
at unity controls agrees apart from documented boundary ramps. Audition isolation
can select different overlaps and therefore a different protection context.

Audition Dry + Wet uses linear blend (50% is half captured dry plus half wet),
a separate sum guard and saved -60 to 0 dB trim. Live Dry + Wet adds wet at the
saved Live wet level to unchanged incoming dry. Audition leaves live settings
alone. Export snapshots accepted audio, policy and gain per instance; pending
replacement work cannot change that bounce's snapshot.

## FL Studio acceptance

The source and aqua shaped-wet lanes use the same amplitude scale. At the default
Wave zoom of 0 dB, equal sample amplitudes have equal relative heights. Changing
Level grows or shrinks the wet waveform. Wave zoom magnifies both lanes equally
(-24 to +48 dB); type a value or double-click the slider to reset to 0 dB.
Large amplitudes may reach the display bounds: reduce Wave zoom to inspect them.
Grey is an independently scaled raw-convolution reference, not a level comparison.
These waveforms precede path-specific aggregate/blend protection and output trim;
their peaks are not an RMS/intensity measurement. Display zoom changes no audio.
Knob value boxes follow dragging continuously and
accept typed values, including signed Level values such as `+6`.

Use the Release build recorded in [gain-staging evidence](../validation/gain-staging-evidence.md) and note host,
rate and buffer. In a fresh instance capture quiet/strong musical events with
factory and imported IRs; check starting impact at Level 0 dB. Try -6/+6 dB
with headroom, then hot overlaps with visible protection. Compare Wet Only
audition/live/export with equal membership and unity trim/Live wet, accounting
for ramps. Check Dry + Wet in the mix, seeks/loops and two-track WAV export.
Save/reopen with calibration, edits and freeze. On an older project copy verify
Legacy restore and explicit adoption. The user passed the three grouped host
checks on 2026-11-02; recorded results and configuration limitations are in
[gain-staging evidence](../validation/gain-staging-evidence.md).
