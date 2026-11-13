# Learn capture, regeneration and sidebar validation

PR #206 covers the shared Arm and explicit rebuild workflow and the Learn presentation changes in #177 and #178. The owner approved the outline-to-fill record button and the aligned Learn sidebar in native previews.

## Native Windows verification

The Release wrapper built the VST3 and ran the standard PR regression set. The first run passed 14 of 15. The missing-timing fixture still expected an idle state after an explicit Arm. The corrected fixture verifies that capture waits armed without admitting provisional audio, then accepts valid timing on the same arm.

| Check | Result |
| --- | --- |
| `scripts/build.ps1 -Configuration Release -PrRegression` | 15 of 15 passed, including workflow processor, capture transactions and UI standards |
| `-TestTarget preverb_automatic_generation_test` | Passed after correcting the test fixture's noncanonical UUID |
| `-TestTarget preverb_learn_generation_test` | Passed, including rediscovery, frozen retention and generation authority |
| `-TestTarget preverb_learn_generation_ui_test` | Passed after using component titles for the off-desktop accessibility assertions. The suite covers Loop playback, shared Arm and Generate bindings, and both Learn tabs at compact 100% and 125% |
| `-TestTarget preverb_perform_composition_preview -TestFilter '^(preverb_responsive_editor_test|preverb_responsive_scale_test|preverb_selector_binding_test)$'` | 3 of 3 passed. The overlap check measures visible bounds after clipping by the scroll viewport |

Runs used Windows MSVC Release from `187abde` with the fixture corrections in the working tree. Visual preview checks covered the outlined and filled Arm states, shared action placement, the compact tab gutters, the Custom IR selector and Import spacing, and Sound keyboard focus brackets. The 125% measurement is the plug-in's UI scale.

## FL Studio smoke

Installed the Release VST3 with the repository installer and retained the prior bundle. The installed binary SHA256 was `57922F40A330BA3A6CA1A2AC546CB0775519F1FEF79B9503BF6E26993D969B5D`.

In FL Studio 2026, a new instance inserted on the scratch project's master while PAT playback was running opened disarmed with an empty source. With the plug-in's Loop Mode off, clicking Arm began capture during playback. DAW Stop settled a 1.8-second take. Generate opened Perform with one swell, which reached Ready. Deleting that swell in Swell settings left zero swells. Learn still showed the same 1.8-second capture. After reducing sensitivity from 50.0% to 35.3%, Regenerate opened Perform with one ready swell. The scratch project was left unsaved.

The host smoke used the installed Release build from before the draft review fixes. It covered Song capture in the plug-in while FL Studio was in PAT playback. Loop-mode capture in FL Studio and host DPI combinations remain unverified. The native fixtures cover Loop and the plug-in's 100% and 125% UI scales.

## Draft review fixes

The Song Stop callback publishes a pass-tagged endpoint before exposing draining to the capture worker. Settlement accepts that endpoint only for the matching validating pass, and transport loop observations preserve it while that pass settles.

An in-flight Sound replacement can finish staging even if Learn has queued a rebuild. A failed rebuild leaves the replacement eligible to publish. Successful retirement clears the old target's replacement state. The focused Release Learn generation fixture checks the failed-rebuild case and the accepted Song endpoint.

## FL Studio acceptance follow-up

The owner reported a moving yellow Learn playhead on fresh insertion while disarmed and wet remaining in standby after Regenerate with PAT playback running. The Learn playhead now follows host timing only during active Song recording. Leaving Perform fades Loop wet without setting the explicit Stop latch, so a matching repeat can pick up the rebuilt plan when it is ready.

Windows MSVC Release checks passed on the fix diff based on `3761953`. The focused `preverb_learn_generation_ui_test` covered disarmed, armed, recording and stopped playhead visibility. The focused `preverb_loop_playback_test` covered the wet fade on leaving Perform, automatic repeat on return and explicit Stop suppression. The standard `-PrRegression` set passed 15 of 15. FL Studio verification on the follow-up build is pending.
