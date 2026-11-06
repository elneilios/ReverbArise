# PR #98 integration and release preparation

Merged main `d987745` into the existing PR branch. The only conflict was the
user-guide currency record; retained main's approved guide and code/spec inputs,
reviewed the additive packaging changes and explicitly refreshed the combined
record. No plugin implementation changed relative to main.

Windows checks on the integrated tree:

- Windows PowerShell 5.1 and PowerShell 7: 20/20 focused release-notes checks each.
- Both shells: 66/66 simulated release-wrapper checks each.
- Both shells: installer fixture suite passed. The first sandboxed PowerShell
  5.1 attempt hit a directory-rename access denial; the normal-permissions retry
  passed. These use dummy bundles and do not install the plugin.
- User-guide currency passed read-only. Fixture results are not native CTest
  or host acceptance.

Owner corrected the actually shared baseline to `v0.1.0-alpha.rehearsal.3`,
`7ea715ba08bef612d7389c19ccd90764d91bd02c`; its existing manifest confirms the
identity. Reviewed delivered source, current guide and focused technical evidence
against that baseline. Authored notes for fresh `v0.1.0-alpha.2` before packaging;
existing alpha, alpha.1 and rehearsal outputs are preserved. Numeric version
remains 0.1.0. Packaging will run native Release/CTest on the final committed
source and verify its exact SHA, guide, release notes and ZIP inventory.

The owner explicitly requested merge of PR #98 followed by the next alpha
package. This authorizes that merge; it does not publish tags/releases, share
binaries or establish final exact-package FL Studio acceptance.
