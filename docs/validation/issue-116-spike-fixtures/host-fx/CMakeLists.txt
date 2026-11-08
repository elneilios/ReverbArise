cmake_minimum_required(VERSION 3.22)
project(HostFxInputSpike VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JUCE_BUILD_EXTRAS OFF)
set(JUCE_BUILD_EXAMPLES OFF)
add_subdirectory("${JUCE_SOURCE_DIR}" juce EXCLUDE_FROM_ALL)

option(PROBE_MIDI_INPUT "Build the separate Note Gate probe" OFF)
option(PROBE_CAPTURE "Build the one-take capture spike" OFF)
if(PROBE_CAPTURE)
    set(PROBE_MIDI_INPUT ON)
    set(probe_name "ReverbArise Host-FX Capture Spike")
    set(probe_code HfCp)
    set(probe_bundle org.reverbarise.hostfxcapture)
elseif(PROBE_MIDI_INPUT)
    set(probe_name "ReverbArise Host-FX Note Probe")
    set(probe_code HfNt)
    set(probe_bundle org.reverbarise.hostfxnoteprobe)
else()
    set(probe_name "ReverbArise Host-FX Input Probe")
    set(probe_code HfIp)
    set(probe_bundle org.reverbarise.hostfxinputprobe)
endif()

juce_add_plugin(host_fx_input_probe
    COMPANY_NAME "ReverbArise experiments"
    BUNDLE_ID "${probe_bundle}"
    PLUGIN_MANUFACTURER_CODE RaSp
    PLUGIN_CODE "${probe_code}"
    PRODUCT_NAME "${probe_name}"
    NEEDS_MIDI_INPUT ${PROBE_MIDI_INPUT}
    FORMATS VST3
    VST3_CATEGORIES Fx
    COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(host_fx_input_probe PRIVATE Probe.cpp)
target_compile_definitions(host_fx_input_probe PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    PROBE_MIDI_INPUT=$<BOOL:${PROBE_MIDI_INPUT}>
    PROBE_CAPTURE=$<BOOL:${PROBE_CAPTURE}>
    PROBE_TRACE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(host_fx_input_probe PRIVATE
    juce::juce_audio_utils juce::juce_recommended_config_flags)

if(PROBE_CAPTURE)
    enable_testing()
    add_executable(capture_spike_test CaptureSpikeTest.cpp)
    add_test(NAME capture_spike_test COMMAND capture_spike_test)
endif()
