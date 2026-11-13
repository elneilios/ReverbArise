# Shared Sound shaping review

The owner approved the wider offline Damping comparison and the native layout with High Pass and Low Pass labels. The offline comparison used Preverb Hall, Intimate Plate and a synthetic mono resonant room with level-matched wet output. The maximum setting shortened the response to a near-arrival accent. The cutoff comparison included 230 Hz and 2.4 kHz high-pass settings and 1 kHz and 140 Hz low-pass settings.

The Windows Release native preview on the issue worktree based on `f440ae2` passed with `--test-ping-pong`. It checked exact Off bypass, both cutoff endpoints from 8 to 384 kHz, crossed cutoffs, independent channel state and silent and single-frame responses. The fixture verified that a new manual swell inherited Learn shaping while the existing swell retained its assignment. It also checked Hall Damping and neutral reset, Plate Damping, retention of shaping on switching to Custom IR, Freeze, and the accepted-state handover after a held High Pass render.

Learn and Perform captures at 900×600 and 1100×760 are in `out/shared-sound-shaping-approved`. Strict OpenSpec validation and the user-guide currency check passed. The native preview ran directly. The full Release regression and host listening and bounce checks remain pending.

The production Wet Audition comparison pairs Hall and Plate at 0% and 100% Damping, then a Custom assignment with High Pass Off and at 230 Hz. Across the full wet clip, maximum Damping reduced RMS by 4.8 dB for Hall and 5.0 dB for Plate at fixed Level. The Custom assignment in this native fixture uses the Plate PCM under the Custom identity. The earlier synthetic mono room comparison covers a distinct IR.

The plugin-only Release build passed and produced binary SHA-256 `205241fb3632d29f8ec08a75c3e563277b1dfceb1faae64489ba8d0706f2d3cb`. Windows cancelled the elevation request before the installer ran. The prior system VST3 remains in place.

## FL Studio and Release regression

The retry installed the candidate above and FL Studio 2026 loaded it from the system VST3 directory. The disposable piano project reopened with its earlier Ping Pong assignment Ready. Hall accepted 76% Damping with Level held at +2.4 dB and showed a shorter wet waveform. Wet Only Audition entered and left its looping state. A one-bar song-selection bounce with insert effects enabled produced a non-silent 48 kHz stereo WAV. The saved Hall assignment encoded state v14 and the accepted Damping setting. The High Pass and Low Pass gestures accepted 333 Hz and 223 Hz, and the settings carried into Intimate Plate. The full three-preset FL Studio comparison and sample comparison between Audition and bounce remain pending.

FL Studio exposed a rejected transition from a damped bundled preset to Ping Pong. The fix transfers Damping to the Ping Pong setting and preserves both filter values. The corrected source passed the focused native Release suite (5/5), including a regression for this transition, and the standard Release PR regression (15/15). The automatic generation ownership fixture also passed (1/1). The corrected Release VST3 built with SHA-256 `0a79977d1f4c301309c2832af1eefb73a16eff56cef1f5918dcc1a209a3a0e54`. Host verification above was on the preceding binary.
