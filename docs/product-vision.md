<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/logo/png/wordmark/reverbarise-wordmark-dark-1024px.png">
    <img src="../resources/logo/png/wordmark/reverbarise-wordmark-light-1024px.png" alt="ReverbArise" width="480">
  </picture>
</p>

<p align="center"><em>Product Vision</em></p>

**Edition:** 1.0 · **Product direction:** Approved

## 1. Product identity

**ReverbArise** is a creative reverse-reverb audio effect for music producers. It transforms captured notes, chords and musical moments into expressive swells that rise towards their arrivals. Instead of bouncing, reversing and manually aligning reverb clips, producers shape transitions directly in a musical, waveform-led workspace.

Its primary home is a Windows VST3 audio effect in a DAW such as FL Studio.

*Reverse Reverb for Creative Swells.*

## 2. The creative workflow

**Learn → Perform → Export** is one coherent creative journey, with shared segmented navigation in the top header.

**Learn — Capture audio.** Play source material through the effect, from a Song arrangement or Pattern. ReverbArise maps the dry audio and can optionally identify musical arrivals and prepare reverse swells. Producers can also capture dry material alone and add swells manually later.

**Perform — Edit & Play.** Work with the captured dry source and wet swells on a shared timeline. Shape their position, source ranges, envelopes, impulse responses and available creative character. Make precise individual edits or apply shared changes to multiple swells; hear the result in musical context.

**Export — Preview & Deliver.** The target workflow is to hear the intended output, choose a dry/wet blend, and drag or save an aligned WAV to the DAW. Wet-only exports can layer above the original source; blended exports can replace it. Exporting is optional: users may continue working in Perform. The current foundation provides dry DAW passthrough and explicit prepared-swell preview; the dedicated WAV-delivery workspace is tracked separately in issue #6.

## 3. Three workspace designs

Each mode uses the same restrained charcoal, aqua, white/grey and yellow visual language, with space devoted to its primary task.

### Learn — Controls → waveform

![Approved Learn workspace](https://github.com/user-attachments/assets/4d0197cb-74ef-4e63-9f98-547bfccff715)

A narrow column of capture and automatic-swell controls sits beside a large full-song dry waveform. Options appear only where relevant.

### Perform — Waveform at the centre

![Approved Perform workspace](https://github.com/user-attachments/assets/e6e1580d-9740-4598-b1eb-037921e631e6)

A chronological Swells list, aligned source/wet waveform editor and context-sensitive inspector make timing and sound design direct and understandable.

### Export — Waveform → delivery

![Approved Export workspace](https://github.com/user-attachments/assets/1a5be2f4-9784-4dd5-980f-b2e842d2cf45)

The export waveform previews the chosen blend: aqua reverse swells grow **before** their arrival points, while white/grey dry transients attack at the arrival and decay afterwards. Export controls sit beside the preview.

These approved mockups express the visual direction, not mandatory pixel dimensions. Their older Capture/Live/Finalise labels are superseded by Learn/Perform/Export. The shared header precedes the dedicated Learn (#11) and Export (#6) layouts; the names do not imply those workspaces are already delivered.

## 4. Product principles

- **Musical first:** Interact with swells, arrivals and timing rather than engineering steps.
- **Non-destructive:** Preserve source recordings and intentional manual edits.
- **Visually truthful:** Distinguish dry source, rising wet swells, captured silence and unmapped gaps.
- **Automation with agency:** Prepare the repetitive work automatically without limiting creative control.
- **Consistency:** Make all three modes feel like one focused instrument.
- **Respect the DAW:** Integrate predictably with playback, timing and export.
