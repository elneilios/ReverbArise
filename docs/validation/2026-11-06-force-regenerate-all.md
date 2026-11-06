# Forced Regenerate all — 2026-11-06

Implementation: `063bb45`, on `codex/force-regenerate-all`, based on `df765d9`.
OpenSpec change: `force-regenerate-all`.

## Owner-authorized closeout

The owner authorized archiving and merging PR #104 on 2026-11-06. Two existing
regeneration requirements and one feedback requirement were synced into the
canonical `swell-regeneration` spec, preserving its purpose and unrelated blocks.
The changed spec passed strict validation; all 18 canonical specs passed normal
validation with existing unrelated length warnings retained. All five tasks are
complete. The change is archived at
`openspec/changes/archive/2026-11-06-force-regenerate-all/`.
Closeout changes affect specs, task placement and validation/guide records only;
the passing 75-test implementation is unchanged.

## Behavior and coverage

Every explicit All request queues eligible confirmed swells, including current
Ready, disabled and muted definitions. Frozen and importing-IR targets retain
ownership. Selected remains isolated. The existing worker, bounded memory and
latest-generation publication checks remain in use; no audio callback, source
format or session schema changed.

The engine returns command admission and queued/skipped counts. Recovery shows
these through the existing helper role immediately below its regeneration
buttons. The feedback survives fast completion and clears on session replacement;
it does not replace workspace errors or promise completed audio. All remains
available for empty/protected batches in Perform so it can explain no work.

Focused native executable checks passed:

- `preverb_source_edges_test`: repeated current/legacy/disabled/muted requests,
  frozen/import ownership, Selected isolation, codec restore, invalid-source
  isolation, previous complete asset retention and immutable captured samples.
- `preverb_manual_preparation_test` (within CTest): explicit All held at
  publication followed by target deletion or session replacement.
- `preverb_swell_ui_test --test-regeneration`: production button binding,
  retained/replaced queued feedback, frozen/empty/rejected results, session
  clearing and Recovery bounds with effective 100% and 125% presentation.

Native logical component captures were inspected at
`out/force-regenerate-all/regeneration-recovery-{100,125}.png`. These capture
the same logical component at each effective user magnification, rather than
claiming physical desktop-pixel or Windows-DPI coverage. The footer and action
groups remain within the page with the existing spacing and feedback tokens.

## Environment and verification notes

Windows x64, Visual Studio 2026/MSVC, Release; existing warm build tree retained.
Use `scripts/build.ps1` for build/configure/test operations and its directory lock.
The initial sandbox configure could not access the Windows SDK registry;
the authorized elevated wrapper succeeded. The first full run was 74/75:
one revised source-edge fixture still compared a restored asset against the
first render rather than the latest forced render. Its assertion was corrected,
failure-isolation counting was tightened, and the focused native check passed.
No production-code workaround was made for that assertion.

Final `scripts/build.ps1 -Configuration Release -Test` passed **75/75 native
MSVC/CTest tests in 288.19 seconds**. Logs:
`out/build-logs/Release/20261106-083904-c7e0f6f5/`. The wrapper started at
`df765d9 (dirty)`; those implementation/test files were committed unchanged as
`063bb45` during the run. Later changes are task tracking and this evidence only.

UI standards passed (61 files; 60 existing bounded definition/debt occurrences).
Strict validation of the active OpenSpec change, guide currency and diff checks
passed. The guide now documents Recovery navigation, forced current renders,
protected skips and admission versus completion.

No test build was installed and no new FL Studio or physical Windows-DPI test
was performed for this follow-up. Prior #58 host acceptance is not represented
as acceptance of this new command behavior. No additional host-specific concern
was found in the changed call path.
