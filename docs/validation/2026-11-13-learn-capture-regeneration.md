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

In FL Studio 2026, a new instance inserted on the scratch project's master while PAT playback was running opened disarmed with an empty source. With the plug-in's Loop Mode off, clicking Arm began capture during playback. DAW Stop settled a 1.8-second take. Generate opened Perform with one swell, which reached Ready. Loop-mode host capture, deletion and regeneration in the host, and host DPI combinations are separate from the passing native fixtures.
