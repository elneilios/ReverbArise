---
name: maintain-user-guide
description: Review delivered ReverbArise behavior and maintain the offline HTML user guide and its release currency record. Use after user-facing code/spec changes and before alpha packaging; does not implement features or publish releases.
---

Maintain `docs/development/alpha-guide.html` as the musician-facing guide for delivered behavior.

- Run `scripts/check-user-guide.ps1` to identify changed inputs. Review the delivered diff and affected call paths, relevant `openspec/specs/` requirements and directly relevant approved issue/active-change decisions. Canonical specs can intentionally lag active changes; do not describe backlog plans as functioning features or sync/archive specs as part of this review.
- Update only affected instructions, limitations, troubleshooting and feedback. Preserve the production tagline, Learn capture requiring DAW playback, Perform playback/DAW rendering and optional Export distinction. Settings About identifies the actual running build; do not hardcode future version/SHA values into prose.
- Keep one static self-contained HTML file: embedded media, no scripts, external resources or network dependence. For changed screenshots reuse `preverb_learn_generation_ui_test --guide-only`, `PREVERB_UI_M3_OUTPUT` and the existing release-guide assembly; never invent UI or substitute prototype images. Reuse approved logo assets.
- Check changed control names and flows against working production bindings. Document untested host/rendering coverage honestly. Use focused checks and existing native evidence; avoid repeating owner acceptance for unrelated changes.
- After semantic review, explicitly run `scripts/review-user-guide.ps1 -Summary '<what was reviewed and updated, or why no prose change was needed>'`, then `scripts/check-user-guide.ps1`. The record covers normalized content hashes, not a self-referential commit or clock. Commit guide/review changes with the implementation.
- Packaging only validates currency and assembles already-reviewed content. Never refresh the record from packaging, bypass a stale check, or treat matching hashes as proof that a semantic review occurred. A stale check means review first, then record deliberately. No release/tag/distribution authorization is granted.

See `docs/development/releases.md` and `docs/development/product-version.md` for authoritative build/release contracts.
