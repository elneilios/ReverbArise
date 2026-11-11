# Musical gain staging

New ReverbArise instances use the listening-approved **Source energy +6 dB**
balance. Swell **Level at 0 dB** means that calibrated starting point. Level
is adjustable from -12 to +12 dB. Reset wet shape returns Level to 0 dB and
normal timing/fade defaults without accumulating gain or recalibrating edits.

Level's durable domain is finite -60 to +12 dB, including fractional saved values
that are not multiples of the editor's 1 dB coarse / 0.1 dB fine steps. The
semantic constants live with `Shape`. All new edits use the same -12 to +12 dB
limits, including numeric and accessibility entry. Older saved values below
-12 dB retain their exact gain, including in frozen swells. The dial rests at
-12 dB. Pressing it brings the older value to -12 dB. Valid typed entry applies
the chosen gain.
Double-click resets only Level to 0 dB. Both gain policies retain their
calibration and raw assets. Saved sessions use the same schema and asset format.
Level is non-parameter plugin state, without a normalized host automation mapping.

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

## Generated headroom

The worker checks the positioned wet mix at Level 0 dB and Live Wet 0 dB before
accepting newly generated audio. Song uses the aligned sample sum. Loop includes
repeat overlap and partial passes at entry and finish. Routes with variable phase
use a conservative bound. If the generated mix would exceed **-1 dBFS sample peak**,
the worker reduces the new batch by a common amount and saves that amount with its
starting gains. For an existing mix, the worker allocates the available headroom to
the new batch while preserving the accepted swells. The starting gain and calibration
are separate, and neither rewrites captured audio, raw convolution or the IR.

Level applies the displayed dB adjustment around that saved starting gain.
Changing one swell leaves other swell gains alone. Live Wet scales the wet mix.
A later edit can exceed full scale. Playback preserves the float signal, and the
status and Diagnostics show measured overload. The wet default ceiling leaves
sample headroom for generated material. The total can still exceed full scale
when incoming dry is hot or gain is added in the DAW.

## Audition, live playback and export

These paths share the accepted per-swell starting gains and Level. Wet Only
comparisons with equal membership and explicit gain settings agree apart from
playback boundary ramps. Dry + Wet adds unity dry to the wet contribution at
Live Wet gain. Audition uses captured dry, while realtime playback uses live dry.

Song export uses its independent Dry and Wet levels. Loop export Wet 100% uses
the same wet gain as Perform, including Live Wet. Preview and the Float32 WAV
preserve finite samples above full scale and show the measured peak. Export pins
the accepted render and gain for each swell until delivery completes.

## FL Studio acceptance

The source and aqua shaped-wet lanes use the same amplitude scale. At the default
Wave zoom of 0 dB, equal sample amplitudes have equal relative heights. Changing
Level grows or shrinks the wet waveform. Wave zoom magnifies both lanes equally
(-24 to +48 dB); type a value or double-click the slider to reset to 0 dB.
Large amplitudes may reach the display bounds: reduce Wave zoom to inspect them.
Grey is an independently scaled raw-convolution reference, not a level comparison.
These waveforms show shaped swell contributions before Live Wet and routing.
Their peaks are sample amplitudes. Display zoom changes no audio.
Knob value boxes follow dragging continuously and
accept typed values, including signed Level values such as `+6`.

Check a hot generated overlap at default gain in Song and across a Loop wrap.
Then raise one swell by +12 dB. Confirm that a later, disjoint swell retains its
level, and that the overload appears if the wet peak exceeds full scale. Compare
Wet Only across Perform, Audition and export at matching gain. In Dry + Wet,
check that coincident peaks pass through without ducking the dry signal.
