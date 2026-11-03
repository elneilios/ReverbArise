# ReverbArise — UI Layout & Visual Style Guide

**Version:** 1.0 · **Purpose:** Design reference and implementation handoff · **Status:** Visual direction approved; numerical design tokens proposed for validation against the approved mockups.

This document combines two design references: **(1) Layout Structure / Border Specification** and **(2) UI Visual Style Guide**. The product name is **ReverbArise** (previously Preverb). The objective is a professional, minimal, high-contrast audio-plugin interface: dark charcoal, aqua/teal highlights, carefully restrained yellow timing markers, and a large waveform editor.

> **Reference images:** The two supplied visual guides are included in the companion asset package. The written descriptions below are the implementation reference; where generative imagery includes contradictory or speculative controls, preserve the previously agreed product scope instead.

---

## Part I — Layout structure and borders

![Layout structure — separator and box map](ReverbArise_Layout_Structure_Reference.png)

### 1. Overall structure

The plugin has **eight numbered functional regions**, arranged in six principal horizontal bands: a full-width header (1); global controls (2); a three-column main workspace (3–5); audition (6); overview (7); and transport information (8).

```text
┌──────────────────────────────────────────────────────────────────────────┐
│ 1  HEADER: waveform-A wordmark, tagline, undo/redo, settings, presets   │
├──────────────────────────────────────────────────────────────────────────┤
│ 2  GLOBAL CONTROLS: Auto-marker [On/Off]   Sensitivity [knob + value]    │
├───────────────┬──────────────────────────────────┬───────────────────────┤
│ 3  SWELLS     │ 4  MAIN EDITOR                    │ 5  SELECTED SWELL     │
│               │                                  │                       │
│ Count, add,   │ Ruler; source waveform;          │ Name / enabled       │
│ select, enable│ wet waveform / envelope;         │ Individual IR        │
│ name, IR      │ transient/arrival marker;        │ Pre-roll, end offset │
│               │ handles for start, peak, end;    │ fade in/out, gain,   │
│               │ target/readout and zoom          │ curve, numeric entry │
├───────────────┴──────────────────────────────────┴───────────────────────┤
│ 6  AUDITION: Play / Stop   Dry ←───── mix ─────→ Wet   editable %         │
├──────────────────────────────────────────────────────────────────────────┤
│ 7  OVERVIEW: ruler +  ┌──────────────────────────────────────────────┐  │
│                        │ full-song capture / waveform / swell map   │  │
│                        └──────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────────────────────┤
│ 8  TRANSPORT / INFO: song position  |  BPM  |  time signature            │
└──────────────────────────────────────────────────────────────────────────┘
```

The ASCII lines above explain regions; **they do not mean every region has a visible rectangular border**.

### 2. Actual border rules

| Border treatment | Where used | Where *not* used |
|---|---|---|
| **Thin horizontal rule** | Across major section transitions: after header, between working area and audition, around overview/transport divisions where present in the approved design | Do not box every control or label |
| **Thin vertical rule** | Between the Swells list, main editor, and Selected Swell inspector | No full rectangular panels enclosing each column |
| **Subtle internal separator** | A few small group boundaries, such as waveform lanes or headings, where necessary for readability | Avoid repeated heavy/double rules |
| **Full rounded outline** | **Overview waveform viewport** inside the overview section | Do not add rounded cards around global controls, left list, editor, or inspector |
| **Control outline** | Interactive numeric fields, IR dropdown, buttons, toggles and selected-list highlight | Not a substitute for container/layout lines |

The **background is one continuous dark-charcoal field**. Large filled cards, separate raised panels, drop shadows, heavy outlines, gradients and skeuomorphic bevels are out of scope for this visual language.

### 3. Region notes

**1 — Header:** Waveform-shaped **A** within the *ReverbArise* wordmark; subtitle “Reverse Reverb for Creative Swells”; optional existing undo/redo, settings and preset controls. Do not invent new audio processing controls here.

**2 — Global controls:** Auto-marker on/off, sensitivity knob and typeable percentage. These govern automatic transient suggestions across the captured song.

**3 — Swells (left):** Counter, manual add, scrollable list of swells. Each item identifies the swell, its enabled state and its **own IR assignment**; selected item has a thin aqua highlight. In the empty state display “No swells yet” and a short instruction.

**4 — Main editor (center):** Main visual priority. Source dry waveform on top, selected wet swell / envelope below; a time ruler, transient/target markers, directly draggable start/peak/end handles, selected timing and zoom. **End Offset** permits the wet swell to end before or after the target, including an overlap with the uninterrupted dry input. In the initial state display a restrained “Press play to begin mapping your song” prompt in place of waveforms.

**5 — Selected Swell (right):** Context inspector, blank instructional state if nothing selected. For a selection: swell title, per-swell IR selector; Pre-roll/Start; End Offset (overlap); Fade In; Fade Out; Level; Curve. Knobs and typeable exact values mirror changes made by dragging main-editor handles. Do not repeat the envelope graph as a second editor here.

**6 — Audition:** Play/Stop and one continuous Dry ↔ Wet mix slider with editable percentage (100% = wet only). Audition settings apply to audition only, **not** the plugin’s main mixer output. No redundant “Wet Only” mode button.

**7 — Overview:** Header must read **“Overview”** (not “Song Overview”). Contains the **only major rounded outline** around the complete-song timeline. Shows actually captured regions versus unmapped gaps, current viewport and swell locations; empty state “No audio mapped yet”. Do not invent waveform data for uncaptured regions.

**8 — Transport/info:** Position, host tempo and time signature as reported by the DAW. This region is informational, not a duplicate set of host transport controls.

### 4. Spacing and responsive behavior

- Keep generous margins around the outer canvas and clear visual gaps between controls.
- Use a stable three-column work area; allocate most width to the center waveform editor, a narrower swell list at left and a moderate inspector at right.
- Preserve vertical room for waveform manipulation; do not shrink the source/wet lanes to accommodate more buttons.
- Use consistent small spacing between related labels and inputs, with noticeably larger space between major sections.
- Suggested **starting values**, not extracted measurements: outer padding `16–24px`, section rhythm `8–16px`, control spacing `4–12px`.
- Validate at actual JUCE plugin window sizes and DPI scales; the mockup is a visual reference, not pixel-exact production geometry.

---

## Part II — Visual style guide

![Visual style guide — brand, type, palette and widgets](ReverbArise_Visual_Style_Reference.png)

### 5. Brand and approved mark

- **Name:** ReverbArise (capital **R** and **A**).
- **Wordmark:** “Reverb” light/white + “Arise” aqua. The **A in Arise is the stylized rising waveform/envelope glyph**, integrated into the word rather than duplicated as an unrelated mark to the left.
- **Tagline in approved mockup:** *Reverse Reverb for Creative Swells*.
- Use the approved UI wordmark as the primary reference. The standalone square “app icon” shown in the generative style-guide sheet is a **conceptual illustration**, not independently approved vector artwork. **Do not hand-draw a different logo shape based only on that square.** A vector production logo/icon should be extracted/refined from the selected waveform-A concept in a separate asset task.

### 6. Color palette

These values are **proposed color tokens transcribed from the generated style-guide illustration**. They are suitable implementation starting points but should be sampled/visually checked against the approved screenshot before final sign-off.

| Token | Proposed hex | Intended usage |
|---|---|---|
| `color.background` | `#0F1418` | Uniform deep charcoal base (no gradient) |
| `color.accent.primary` | `#00EFFF` | Selected swell, waveform envelope, active controls, wave-A / Arise wordmark |
| `color.accent.hover` | `#00C2CC` | Muted aqua, hover/secondary emphasis |
| `color.marker` | `#FFC700` | Target/arrival marker, playhead and timing/range accents |
| `color.text.primary` | `#F5F7F9` | Headers, labels and important values |
| `color.text.secondary` | `#C7CDD4` | Explanations, less prominent values and help text |
| `color.separator` | `#4A5058` | Delicate borders/rules, subdued structural separation |
| `color.outline` | `#6B737C` | Interactive field and occasional content viewport outlines |

Use neutral grey waveforms for **source input**, aqua for **wet swell**, and yellow for the **target position**. Aqua should not flood large surfaces; its impact comes from selective use. Use a subtle (not glowing) highlight on the selected swell.

### 7. Typography

**Reference font:** Inter, or a legally bundled/system-available metrically similar modern sans-serif. Do **not** distribute any third-party font binary without checking its license and packaging requirements.

**Approved Windows implementation (#16):** Segoe UI is the intentional system-font
choice for the native JUCE UI. The owner approved its Regular body text, true
Semibold navigation, Bold headings and Italic tagline/mode description in the
header's native visual reviews. Explicit system faces replaced the bundled Inter
variable-font default after its weight appeared too light and navigation weights
needed reliable resolution. Segoe UI is obtained from Windows, not redistributed;
the supplied Inter asset and OFL notice remain packaged. This is the approved
implementation of the visual hierarchy below, not an automatic fallback or a
claim of identical Inter metrics. Keep the accepted sizes, off-white contrast
and alignment; changing font family or component typography requires review.

| Style | Intended content |
|---|---|
| Inter **Bold** | Plugin brand, main headings, Swells / Selected Swell / Overview |
| Inter **SemiBold** | Section labels and emphasized selected swell name |
| Inter **Medium** | Control captions, target/pre-roll/length labels, numeric display |
| Inter **Regular** | Helper text, descriptions and secondary metadata |
| Inter *Italic* | Tagline and occasional supplemental instructions |

Prefer clear type size/weight contrast over decorative typography; align captions, knobs and their numeric fields on a consistent grid. Keep the empty-state headline prominent but not oversized.

### 8. Controls and interaction styling

| Component | Appearance | Required interaction |
|---|---|---|
| Active button | Solid aqua fill, dark icon | Primary audition Play |
| Secondary button | Charcoal background, thin grey/white outline | Stop, add/delete, zoom controls |
| Toggle | Aqua track when on; outlined inactive state when off | Auto-marker, enabled state |
| Knob | Thin aqua value arc with muted remainder | Coarse mouse drag + fine adjust; numeric readout |
| Numeric field | Small outlined field, legible units | **Typeable exact values**, validated to real parameter domain |
| Dropdown | Outlined rectangle and small chevron | Per-swell IR selection; available options only |
| Slider | Thin aqua filled portion, subdued remainder | Audition dry/wet mix; precise value entry |
| Selection | Thin aqua outline / restrained fill | Current swell in left list, selected viewport |
| Handle | Small outlined circular point on bright aqua line | Edit swell beginning/curve/peak/end directly in the editor |

Avoid duplicate ways of displaying the *same graph*: envelope handles belong in the main editor; knobs and precise numeric entry belong in Selected Swell. The same data is always synchronized in both directions.

### 9. Iconography

Simple, consistent stroke icons. Approved motifs shown in the source sheet include:

- Undo and redo curved arrows
- Settings gear; presets folder; dropdown chevron
- Play triangle, stop square; add/zoom plus and minus
- Thin yellow target crosshair and timing/range glyphs
- Grey source-audio bars / aqua wet waveform
- Outlined circular envelope control points

Use a consistent stroke weight and align icon geometry optically. Icons are supportive—not decorative or large status illustrations in every region.

### 10. Waveform design

- Dry/source waveform: muted off-white/grey, clear attack and decay.
- Wet/swell waveform: saturated aqua, drawn below the source.
- Envelope curve: a thin bright aqua line overlaid on wet waveform, with draggable handles at important points.
- Target: distinct narrow yellow dashed vertical line, consistent across both lanes.
- Positive **End Offset**: wet envelope extends beyond the yellow target marker over the unchanged dry event; negative offset leaves a gap before the target.
- Overview waveform: neutral grey, with aqua swell indicators and a single thin aqua viewport highlight.
- Mapping coverage is a genuine data state: uncaptured zones must not be displayed as if recorded.

### 11. States

**Initial/empty:** Swells (0), no waveform content, concise “Press play to begin mapping your song”, inspector “No swell selected”, Overview “No audio mapped yet”. Auto-marker ON and sensitivity visible, default IR configured internally. Do not force a file import or a Generate command.

**Capturing/processing:** Auto-map incoming dry audio at DAW positions, detect candidate attacks and schedule per-swell convolution automatically. Subtle non-blocking state feedback is preferable to heavyweight progress panels.

**Ready/selected:** Left list shows per-swell IR; source and wet waveforms + draggable envelope occupy the center; inspector shows relevant selected-swell knobs and numeric fields; overview remains visible; audition can mix captured dry with wet.

**Updating:** Edits to IR/source rendering inputs cause background rerender only where necessary. Edits to fade, level and playback range should apply without unnecessary full convolution. Preserve the last good completed render until replacement is ready.

### 12. Scope discipline

The style mockup is an **end-goal appearance**, not permission to add new DSP features. In particular, do not add EQ high/low cut, general stereo-width or decay controls, redundant wet-only buttons, repeated envelope graphs, CPU meters, speculative tabs, fake capture data, or a prominent Generate/Regenerate workflow solely to match an image. Diagnostics may remain in a developer-facing area separate from the main UX.

---

## Handoff checklist

- [ ] Preserve all eight layout regions and the three-column central workspace.
- [ ] Thin lines rather than filled cards; Overview contains the only large rounded full-box viewport.
- [ ] Approved charcoal/aqua/yellow visual roles; **waveform A inside Arise**.
- [ ] Source waveform + wet waveform with selected, manipulable envelope.
- [ ] Per-swell IR selection; knobs plus typeable exact numbers.
- [ ] Positive/negative end offset around unchanged dry event.
- [ ] Auto-marker on/off and sensitivity.
- [ ] One audition Dry↔Wet mix slider; wet-only equals 100%.
- [ ] Overview visible in both empty and loaded states, titled **Overview**.
- [ ] No speculative or duplicated controls; preserve source/processing correctness.

### Source artwork

1. `ReverbArise_Layout_Structure_Reference.png` — thin borders vs. a full Overview content box.
2. `ReverbArise_Visual_Style_Reference.png` — colors, typography, wordmark, icons, waveform styling, and interactive elements.

Both are included in the accompanying ZIP with this Markdown file so the relative image links render when unpacked together.
