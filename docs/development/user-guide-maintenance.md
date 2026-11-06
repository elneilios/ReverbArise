# User guide maintenance

Use [maintain-user-guide](../../.agents/skills/maintain-user-guide/SKILL.md) after delivered user-facing changes. Review affected code, canonical OpenSpec requirements and relevant active-change decisions; describe implemented behavior. An unchanged guide can be correct, but its currency review must explain why.

After the review:

```powershell
./scripts/review-user-guide.ps1 -Summary 'Describe reviewed behavior and updates, or why no prose changes were needed'
./scripts/check-user-guide.ps1
```

Commit `alpha-guide.html` and `user-guide-review.json` with the code. The record hashes production Source/, canonical specs, CMake inputs, relevant release/install scripts and guidance, guide/screenshot inputs and the maintenance skill. Text is normalized to UTF-8/LF; PNGs use exact bytes. There is no self-referential commit/time field. Hashes establish currency, not semantic correctness; the maintainer still reviews actual changes.

Packaging calls only the read-only check before building. Missing/stale review fails with changed paths. It never runs an agent, updates the review record or rewrites guide content. Assembly fills identity slots and embeds fresh genuine native screenshots/approved logo into ignored output using the existing release-guide mechanism. There is no skip-currency switch.

Reuse `preverb_learn_generation_ui_test --guide-only` with `PREVERB_UI_M3_OUTPUT` for native evidence. `--review-about` opens the real Settings About page for focused review. Keep screenshots and offline/host acceptance scoped to the actual build.
