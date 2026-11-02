# ReverbArise

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="resources/logo/png/wordmark/reverbarise-wordmark-dark-1024px.png">
    <img src="resources/logo/png/wordmark/reverbarise-wordmark-light-1024px.png" alt="ReverbArise" width="480">
  </picture>
</p>

A Windows VST3 reverse-reverb plugin built with C++20 and JUCE. The current development build retains the legacy **Preverb Spike** name and compatibility identifiers.

The plugin captures source audio on the song timeline, detects arrivals and
prepares editable reverse-reverb swells. It supports WAV/WavPack impulse
responses, wet envelope editing, contextual audition, saved swell sessions,
transport-synchronized playback and host offline rendering.

This is an actively developed prototype. Automated verification and FL Studio
acceptance results are recorded separately; see the validation guides before
assuming a feature has completed host acceptance.

See the [documentation index](docs/README.md) and [Product Vision](docs/product-vision.md). Future requirements and approved mockups live in [GitHub Epic #3](https://github.com/elneilios/ReverbArise/issues/3); canonical OpenSpec specifications describe current implemented behaviour.

## Build and test

The tested environment is Windows x64 with Visual Studio 2026's Desktop
development with C++ workload and CMake tools. From a shell with CMake on PATH:

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

CMake fetches pinned JUCE and WavPack dependencies. The plugin bundle is written
to `out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/Preverb Spike.vst3`.
See [development setup](docs/development/development.md) for full requirements and commands.

## Project references

- [Product vision](docs/product-vision.md)
- [UI design guide](docs/design/ReverbArise_UI_Layout_and_Style_Guide.md)
- [Automatic swell workspace](docs/development/automatic-swell-workspace.md)
- [Transport playback](docs/development/transport-swell-playback.md)
- [FL Studio transport validation](docs/validation/fl-studio-transport-playback-validation.md)
- [OpenSpec requirements](openspec/specs/) and [change history](openspec/changes/)

Source lives in `Source/`, automated checks in `tests/`, and the original
generated default IR and its provenance in [resources](resources/README.md).
Build output, downloaded dependencies and local DAW projects are excluded from
version control. Captured audio and managed render assets live outside this
repository; a Git checkout does not back up FL Studio projects or their caches.

## Distribution

No project-wide open-source license has been selected. This repository does not
grant redistribution rights to the project code. See the dependency licensing
constraint in [development setup](docs/development/development.md#license-scope) before
distributing JUCE-based binaries. The generated default IR has its own CC0
dedication documented in `resources/README.md`.
