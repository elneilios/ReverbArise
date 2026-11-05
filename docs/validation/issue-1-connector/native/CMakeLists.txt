cmake_minimum_required(VERSION 3.22)
project(RAContextProbe VERSION 0.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
if(NOT EXISTS "${JUCE_ROOT}/CMakeLists.txt")
  message(FATAL_ERROR "Pass JUCE_ROOT pointing to the existing JUCE 9.0.4 source cache")
endif()
add_subdirectory("${JUCE_ROOT}" juce)
juce_add_plugin(ra_context_probe
  COMPANY_NAME "ReverbArise Investigation" BUNDLE_ID "org.reverbarise.spike.contextprobe.v1"
  PLUGIN_MANUFACTURER_CODE RASp PLUGIN_CODE RCp1
  PRODUCT_NAME "ReverbArise FL Context Probe" FORMATS VST3
  IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(ra_context_probe PRIVATE Probe.cpp Probe.h)
target_compile_definitions(ra_context_probe PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
  PROBE_EVIDENCE_DIR="${EVIDENCE_DIR}")
target_link_libraries(ra_context_probe PUBLIC juce::juce_audio_utils)
add_executable(probe_native_test NativeTest.cpp)
target_link_libraries(probe_native_test PRIVATE ra_context_probe)
enable_testing()
add_test(NAME probe_native_callback COMMAND probe_native_test)
