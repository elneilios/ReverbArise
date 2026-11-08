cmake_minimum_required(VERSION 3.22)
project(RackLoopTimingSpike VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory("${JUCE_SOURCE_DIR}" juce EXCLUDE_FROM_ALL)

function(spike_plugin target code name source input)
    juce_add_plugin(${target}
        COMPANY_NAME "ReverbArise experiments"
        BUNDLE_ID "org.reverbarise.spike.${target}"
        PLUGIN_MANUFACTURER_CODE RaSp
        PLUGIN_CODE ${code}
        PRODUCT_NAME "${name}"
        NEEDS_MIDI_INPUT ${input}
        FORMATS VST3
        VST3_CATEGORIES Fx
        COPY_PLUGIN_AFTER_BUILD FALSE)
    target_sources(${target} PRIVATE ${source})
    target_compile_definitions(${target} PUBLIC
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0
        TRACE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/traces")
    target_link_libraries(${target} PRIVATE juce::juce_audio_processors juce::juce_recommended_config_flags)
endfunction()

spike_plugin(rack_clock RlCl "Rack Loop Fixture Clock" Clock.cpp FALSE)
spike_plugin(rack_trace RlTr "Rack Loop Timing Trace" Trace.cpp TRUE)
spike_plugin(rack_policy_clock RlPc "Rack Loop Policy Clock" Clock.cpp FALSE)
target_compile_definitions(rack_policy_clock PRIVATE POLICY_CLOCK=1)
spike_plugin(rack_policy RlPy "Rack Loop Policy Probe" Policy.cpp TRUE)

# Link the existing production DSP build. The harness is offline and local.
get_filename_component(REVERB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(PRODUCTION_BUILD "${REVERB_ROOT}/out/build/windows-msvc")
add_executable(loop_wet_spike WetSpike.cpp)
target_include_directories(loop_wet_spike PRIVATE "${REVERB_ROOT}/Source" "${JUCE_SOURCE_DIR}/modules")
target_compile_definitions(loop_wet_spike PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
target_link_libraries(loop_wet_spike PRIVATE
    "${PRODUCTION_BUILD}/Release/preverb_assets.lib"
    "${PRODUCTION_BUILD}/Release/preverb_dsp.lib"
    "${PRODUCTION_BUILD}/Release/preverb_session.lib"
    "${PRODUCTION_BUILD}/_deps/wavpack-build/Release/libwavpack.lib"
    bcrypt)

spike_plugin(rack_clkd_trace RlCt "Rack CLKD Capture Trace" clkd/ClkdTrace.cpp TRUE)

spike_plugin(rack_clkd_cue RlCq "Rack CLKD Cue Trace" clkd/CueTrace.cpp TRUE)

spike_plugin(rack_clkd_control RlCx "Rack CLKD Test Controls" clkd/CueControl.cpp FALSE)
