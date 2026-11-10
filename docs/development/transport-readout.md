# Perform transport readout

Issue #68's read-only `Controls/TransportReadout` and `TransportPresentation`
replace the editor's combined `hostInfo` label. The owner approved the native
look and feel on 5 November 2026, scoped to spacing, readability and stability
as values change. This does not establish FL Studio or physical DPI coverage.

The presentation binding uses existing musical display rounding (up to three
fractional beat digits) and compact tempo formatting (two decimal places), with
independent position, tempo and meter availability. Full semantic and exact
position text remain accessible and available through context-help metadata.
No host action, capture, persistence or audio work belongs in this control.

In Song Mode, Learn and Perform draw the timeline playhead only when the
host supplies usable song timing. If the host supplies a sample counter
without song position, tempo or time signature, the workspace notice explains
what Song Mode needs. Perform also checks the current playback clock against
the captured mapping. The footer continues to report individual host fields
as observed.

The component retains the existing footer's 15-logical-pixel embedded Inter
Regular and primary text ink. It introduces no background/highlight decoration.
Bar and Beat captions, the BPM unit and three separators have independent boxes;
numeric values align right. Group centring depends on container bounds alone.

Value widths reserve the larger of `unavailable` or worst-case embedded digit
metrics for signed six-digit bars, three-digit beats plus three decimals,
four-digit tempos plus two decimals, and two three-digit meter parts. Sign,
decimal and slash widths are measured separately. Existing tight and related
spacing tokens govern gaps. These are display budgets, not host-domain clamps.

Each value paints with bounded ellipsis if it exceeds its box; neighbouring slots
and the group stay fixed, and full formatted text remains in accessible/help
content. No automatic font compression occurs. Supported editor widths must fit
the complete group; smaller standalone containers clip within their component
and do not redefine the plugin's supported minimum size.

`preverb_transport_readout_test` checks geometry, actual glyph fit, carry rounding,
independent availability, oversized text, accessible detail and software rendering
at 100/125/150/200% for the four supported logical sizes. Set
`PREVERB_TRANSPORT_OUTPUT` to retain PNGs. Its `--review` entry point opens one
synthetic native preview and does not instantiate a processor or play audio.
Software-scale rendering does not establish physical Windows DPI/host acceptance.

`PerformReadabilityCases.h` additionally drives real processor host snapshots and
editor refresh across supported sizes, tempo/meter changes, seeks, unavailable
timing and mode return, checking value binding and durable-state/job invariance.
