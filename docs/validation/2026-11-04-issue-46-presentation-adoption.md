# Issue #46 — approved presentation adoption

The owner answered **Approve revised presentation** on 4 November 2026 for the
[bottom-help native checkpoint](2026-11-04-issue-46-hover-footer.md). This accepts
the revised three-column presentation and 52-pixel audition strip before rollout.

Production now uses that profile directly: inline audition status/equal gaps,
grouped Timing/Envelope/Level inspector with full Source labels and reachable
reset rows, compact readouts retaining exact editing, shared musical ruler,
concise annotations, readable musical footer/aligned headings, and normal-font
bottom context help for all existing hover providers. The temporary preview-only
flag and legacy presentation branches are removed. Standards revision 5 records
the scoped approval. No stored precision, schema, audio callback or Export-domain
behavior changes are introduced by adoption.

Native strip/readability/hover cases now construct the ordinary production editor;
there is no helper call that can conceal a production/preview difference. The
strip test's obsolete “production stays 72 until approval” assertion is replaced
with direct 52-pixel/default lane-bounds assertions. Existing geometry, interaction,
precision, mode, persistence and audio checks remain.

Final build results and bundle identity follow below. Final FL Studio listening/
Stop/mode/save-reopen acceptance remains task 6.2; prior editing approvals stand.
