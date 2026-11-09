cmake_minimum_required(VERSION 3.22)
project(RackHostAdmissionProbe VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory("${JUCE_SOURCE_DIR}" juce EXCLUDE_FROM_ALL)

juce_add_plugin(rack_admission
    COMPANY_NAME "ReverbArise experiments"
    BUNDLE_ID "org.reverbarise.spike.rack-admission"
    PLUGIN_MANUFACTURER_CODE RaSp
    PLUGIN_CODE RaAd
    PRODUCT_NAME "Rack Host Admission Trace"
    NEEDS_MIDI_INPUT TRUE
    FORMATS VST3
    VST3_CATEGORIES Fx
    COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(rack_admission PRIVATE Trace.cpp)
target_compile_definitions(rack_admission PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0
    TRACE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/traces")
target_link_libraries(rack_admission PRIVATE juce::juce_audio_processors juce::juce_recommended_config_flags)
