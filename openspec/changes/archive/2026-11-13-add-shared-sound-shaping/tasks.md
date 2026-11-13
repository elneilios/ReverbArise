# Tasks

## 1. Accepted settings and ownership

- [x] 1.1 Add and validate typed shared settings for Learn and per-swell assignments. Verify round trips for Off, active cutoff extremes and nonzero Damping. Verify older sessions preserve Level, Ping Pong settings and exact accepted audio without migration renders.
- [x] 1.2 Pass accepted settings through manual and automatic creation, preset changes and explicit regeneration. Verify that Learn affects new swells, matched existing swells retain their shaping, and changing one Perform swell leaves the others intact.

## 2. Wet render preparation

- [x] 2.1 Implement source cuts in the render worker and verify exact Off bypass, both cutoff extremes, crossed cutoffs, mono and stereo responses, stable coefficients at supported rates and unchanged dry PCM.
- [x] 2.2 Implement the approved full-response damping range and reuse the Ping Pong per-hit path. Verify that matched-level hall, plate and Custom IR clips span a full swell to a short accent, that the Ping Pong echo grid is unchanged, and that silent and short inputs are handled. Document the processing and listening evidence.
- [x] 2.3 Extend render dependencies and the existing replacement transaction. Verify neutral cache reuse, invalidation for canonical edits, fixed-Level Audition under musical gain matching, and retained audio during failure, supersession, Freeze and offline snapshots.

## 3. Shared Sound editor

- [x] 3.1 Add the common knobs using the existing editor and focus behavior. Verify logarithmic cutoff travel, Off, exact entry, keyboard and accessible readout, per-knob reset, immediate gesture feedback and one settled request. Verify Level and Envelope keep their existing update paths.
- [x] 3.2 Capture native Learn and Perform previews at 900×600 and an expanded size, including Custom IR and the disabled and preparing states. Obtain owner review of the layout and interaction. Update living Sound guidance and run the user-guide currency check.

## 4. Integrated verification

- [x] 4.1 Compare the installed build in FL Studio with the same hall, plate and Custom IR sources. Check audible Damping and cuts at fixed Level, existing projects reopening unchanged, and consistent accepted Wet Audition and offline output. Record the results and untested coverage.
- [x] 4.2 At the repository review gate, run focused checks and the standard Release PR regression set. Record the tested revision, native preview, listening results and review status for the PR targeting the Sound Preset epic branch.
