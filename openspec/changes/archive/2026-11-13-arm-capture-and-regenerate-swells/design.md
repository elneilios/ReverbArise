# Design

## Context

See [the proposal](proposal.md). The processor currently starts Song capture on the first playing callback or after entering Learn while playing. Loop already reserves a pass through Arm and handles route-specific starts and finishes. Learn generation already prepares a private replacement against a pinned candidate snapshot, and the request records whether it came from Stop.

## Goals / Non-Goals

**Goals**

- Use the existing workflow and capture transactions to authorize one take for either source mode.
- Reuse candidate analysis, validation and atomic publication for the explicit rebuild.
- Reuse the existing Learn latch, primary command button, layout tokens and feedback surfaces.

**Non-Goals**

- Host timing and Loop capture-method redesign, detector tuning, or changes to the per-swell render regeneration commands.
- State-format migration or changes to the managed asset storage policy.

## Decisions

1. **Reserve capture before the callback can record.** Use the shared Action Button + State Field in the common Learn capture section, with Arm as the latching action and capture status in the adjoining field. Route activation through a processor command that checks the invoking session/pass and pins mode and permission. Use a single pass reservation for waiting, recording and settlement, extending workflow authority where Song currently begins directly. For Song, a valid playing block consumes the wait, DAW Stop finishes, and switching Arm off cancels. Loop retains its existing local timing and Stop semantics, including the verified external Start/Arm route. Require a fresh authorization on restore and after every terminal result.
2. **Pin the replacement policy with the request.** Use the existing explicit versus Stop distinction in Learn generation. The explicit policy uses only frozen definitions for collision and retained arrangement geometry. It bypasses suppression and identity matching, builds fresh definitions with the current accepted defaults, and checks real source windows. Preserve the saved suppression list for automatic reconciliation. Keep automatic Stop reconciliation, including stable matching and manual protection. Routing the explicit action through Diagnostics reset would mutate persisted state before the replacement succeeds.
3. **Commit the arrangement once.** Reuse the existing generation authority and prepublication checks, including the current Song transport guard and the accepted Loop path during continuing playback. Bind visible and secondary explicit Learn invocations to the same action. Capture arming or a newer musical edit invalidates older generation. Retire old jobs by identity on successful commit, preserve frozen definitions and current render metadata, and let interrupted accepted work resume after an unsuccessful attempt. An empty rebuild reports the reason and leaves the prior set available. Preserve the managed assets under the existing retention policy.
4. **Place the shared generation action below both Learn tabs.** Reserve a fixed lower action area outside the capture settings viewport and Sound controls. Centre the swell icon and label as a group in the taller primary command. Use the accepted swell set and saved deletion history to choose the label. Use one centred line for manual placement guidance or the disabled or busy reason. Route the replacement explanation to the existing footer hover help and accessible description. Reserve the action area before laying out either tab so scrollable settings stay accessible at compact sizes.

## Risks / Trade-offs

- **Learn sidebar alignment.** Place the shared tabs and Sound editor using their visible edges and existing focus margins. Enable leading-edge alignment for Learn's Sound editor so the preset navigation and contextual IR File selector line up with their labels. Keep the shared focus painter and selector-to-import spacing.

- **Arm appearance is under native review.** The record dot and outline use the shared danger colour when disarmed. The armed action uses a solid danger fill with a dark dot. The adjoining field shows the capture state, and the icon retains the accessible name “Arm”.

- **Unfrozen creative edits are replaceable.** Keep the replacement explanation next to the action and preserve every accepted definition if the proposed rebuild has no viable new swells or cannot commit.
- **Capture and publication can race.** Check instance/session/pass and settings authority at reservation and settlement. Exercise stale commands, duplicate Stop, restore, in-flight wet work and mode changes in focused fixtures. Keep callback handling bounded and nonblocking.
- **Taller geometry can clip at compact size.** Launch one native preview for owner review of Arm placement and the button height, icon and interaction feel. Review both Learn tabs at minimum size at 100% and 125% where supported. Any new token, variant or scoped exception requires explicit owner approval before adoption, with standards revision and component coverage updated together.

## Migration Plan

Existing projects keep their source, definitions, preferences and references. They reopen disarmed, including older Song projects and saves made during a pending take. Updated Learn guidance explains how to start an additive Song take and when to use Recapture Audio. A successful explicit rebuild saves the resulting arrangement through the existing format.
