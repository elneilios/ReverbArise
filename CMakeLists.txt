cmake_minimum_required(VERSION 3.22)

project(Preverb VERSION 0.1.0 LANGUAGES C CXX)

include(cmake/ProductVersion.cmake)
reverbarise_product_version("${CMAKE_CURRENT_SOURCE_DIR}" product_version)
if(NOT product_version STREQUAL PROJECT_VERSION)
    message(FATAL_ERROR "Product version reader disagrees with PROJECT_VERSION")
endif()

include(CTest)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Isolate upstream options from this project's CTest and static-library settings.
function(preverb_fetch_wavpack)
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(BUILD_TESTING OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(WAVPACK_BUILD_PROGRAMS OFF)
    set(WAVPACK_ENABLE_THREADS OFF)
    set(WAVPACK_ENABLE_ASM OFF)
    set(WAVPACK_ENABLE_DSD OFF)
    set(WAVPACK_INSTALL_DOCS OFF)
    set(WAVPACK_INSTALL_CMAKE_MODULE OFF)
    set(WAVPACK_INSTALL_PKGCONFIG_MODULE OFF)
    FetchContent_Declare(wavpack
        GIT_REPOSITORY https://github.com/dbry/WavPack.git
        GIT_TAG 5803634a030e2a11dba602ba057b89cc34486c67 # 5.9.0
    )
    FetchContent_MakeAvailable(wavpack)
    set(preverb_wavpack_source "${wavpack_SOURCE_DIR}" PARENT_SCOPE)
endfunction()
preverb_fetch_wavpack()

add_library(preverb_capture STATIC
    Source/Capture/CaptureWorker.cpp
    Source/Capture/CaptureSession.cpp
    Source/Capture/CaptureAssetStore.cpp
    Source/Capture/CaptureChunkStore.cpp
    Source/Capture/CaptureManifest.cpp
    Source/Capture/CaptureGenerationContext.cpp
    Source/Capture/SparseSongMap.cpp
    Source/Capture/SourceRangeReader.cpp
    Source/Capture/SourceVisitor.cpp
    Source/Capture/ScopedRecapture.cpp
)
target_include_directories(preverb_capture PUBLIC Source)
target_link_libraries(preverb_capture PUBLIC juce::juce_core juce::juce_audio_formats bcrypt)

add_library(preverb_playback STATIC
    Source/Playback/ScheduledTestTone.cpp
)
target_include_directories(preverb_playback PUBLIC Source)

add_library(preverb_dsp STATIC Source/DSP/ReverseConvolution.cpp Source/DSP/SourceEdgeConditioning.cpp Source/DSP/WavPackDecoder.cpp)
target_include_directories(preverb_dsp PUBLIC Source)
target_link_libraries(preverb_dsp PUBLIC juce::juce_dsp juce::juce_audio_formats PRIVATE WavPack::WavPack)

add_library(preverb_session STATIC Source/Core/SwellSession.cpp Source/Core/ExportState.cpp Source/Core/ContentHash.cpp)
target_include_directories(preverb_session PUBLIC Source)
target_link_libraries(preverb_session PUBLIC juce::juce_core bcrypt)
target_link_libraries(preverb_capture PUBLIC preverb_session)

add_library(preverb_analysis STATIC Source/Analysis/OnsetAnalysis.cpp Source/Analysis/AutomaticTargets.cpp
    Source/Analysis/AutomaticGeneration.cpp Source/Analysis/LearnCandidates.cpp)
target_include_directories(preverb_analysis PUBLIC Source)
target_link_libraries(preverb_analysis PUBLIC preverb_capture juce::juce_dsp)

juce_add_binary_data(preverb_resources HEADER_NAME PreverbResources.h NAMESPACE PreverbResources SOURCES resources/PreverbHall.wav
    resources/irs/Monumental.wav resources/irs/IntimatePlate.wav resources/irs/VintageDarkHall.wav resources/irs/ShimmerCloud.wav resources/irs/Whisper.wav)
juce_add_binary_data(preverb_ui_resources HEADER_NAME PreverbUIResources.h NAMESPACE PreverbUIResources SOURCES
    resources/icons/learn.svg
    resources/icons/perform.svg
    resources/icons/export.svg
    resources/logo/svg/reverbarise-wordmark-dark.svg
    resources/fonts/inter/Inter-Regular.ttf
    resources/fonts/inter/Inter-Medium.ttf
    resources/fonts/inter/Inter-SemiBold.ttf
    resources/fonts/inter/Inter-Bold.ttf
    resources/fonts/inter/Inter-Italic.ttf
)
add_library(preverb_assets STATIC Source/DSP/ExportFiles.cpp Source/DSP/SongMix.cpp Source/DSP/ExportSource.cpp Source/DSP/SongExport.cpp Source/DSP/SwellAssetStore.cpp Source/DSP/SwellShape.cpp Source/DSP/GainStaging.cpp Source/DSP/DefaultImpulse.cpp Source/DSP/AuditionMix.cpp)
target_include_directories(preverb_assets PUBLIC Source)
target_link_libraries(preverb_assets PUBLIC preverb_dsp preverb_session preverb_resources preverb_capture)

add_library(preverb_preview STATIC Source/Playback/ExportVoice.cpp Source/Playback/AuditionVoice.cpp Source/Playback/TransportPlayback.cpp Source/DSP/PreviewEngine.cpp Source/DSP/SwellEngine.cpp Source/DSP/SwellExport.cpp Source/DSP/SwellTransport.cpp Source/DSP/AutomaticCoordinator.cpp Source/DSP/LearnGeneration.cpp Source/DSP/SwellMonitoring.cpp)
target_include_directories(preverb_preview PUBLIC Source)
target_link_libraries(preverb_preview PUBLIC preverb_capture preverb_dsp preverb_session preverb_assets preverb_analysis)

juce_add_plugin(preverb_plugin
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "K7MA"
    BUNDLE_ID "com.k7ma.preverb"
    PLUGIN_MANUFACTURER_CODE K7Ma
    PLUGIN_CODE Prv1
    PRODUCT_NAME "ReverbArise"
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    VST3_CATEGORIES Fx Stereo
)

# Derived metadata, never an independent handwritten version authority.
set(PREVERB_RELEASE_TAG "" CACHE STRING "Explicit packaging tag; empty for ordinary builds")
set(PREVERB_RELEASE_CANDIDATE "" CACHE STRING "Explicit packaging candidate; empty for ordinary builds")
include(cmake/BuildInformation.cmake)
get_target_property(preverb_product_name preverb_plugin JUCE_PRODUCT_NAME)
reverbarise_build_information("${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" "${PROJECT_VERSION}" "${preverb_product_name}")
set_source_files_properties(Source/Plugin/PluginSettings.cpp Source/Plugin/PluginProcessor.cpp tests/learn_generation_ui_test.cpp
    PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/generated")
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/product-identity.json" CONTENT
"{\n  \"version\": \"${PROJECT_VERSION}\",\n  \"manufacturerCode\": \"$<TARGET_PROPERTY:preverb_plugin,JUCE_PLUGIN_MANUFACTURER_CODE>\",\n  \"pluginCode\": \"$<TARGET_PROPERTY:preverb_plugin,JUCE_PLUGIN_CODE>\",\n  \"bundleId\": \"$<TARGET_PROPERTY:preverb_plugin,JUCE_BUNDLE_ID>\",\n  \"productName\": \"$<TARGET_PROPERTY:preverb_plugin,JUCE_PRODUCT_NAME>\"\n}\n")

target_sources(preverb_plugin PRIVATE
    Source/Plugin/PluginEditor.cpp
    Source/Plugin/PluginInspector.cpp
    Source/Plugin/PluginSettings.cpp
    Source/Plugin/PluginPresentation.cpp
    Source/Plugin/PluginProcessor.cpp
    Source/UI/SwellTimeline.cpp
)

target_compile_definitions(preverb_plugin PUBLIC JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(preverb_plugin PUBLIC preverb_capture preverb_preview preverb_ui_resources)
target_link_libraries(preverb_plugin PRIVATE
    preverb_playback
    juce::juce_audio_utils
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

add_custom_command(TARGET preverb_plugin_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:preverb_plugin_VST3>/../Resources"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${preverb_wavpack_source}/COPYING"
        "$<TARGET_FILE_DIR:preverb_plugin_VST3>/../Resources/WavPack-LICENSE.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/inter/OFL.txt"
        "$<TARGET_FILE_DIR:preverb_plugin_VST3>/../Resources/Inter-OFL.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/resources/irs/LICENSE.md"
        "$<TARGET_FILE_DIR:preverb_plugin_VST3>/../Resources/IR-LICENSE.md"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/resources/irs/manifest.json"
        "$<TARGET_FILE_DIR:preverb_plugin_VST3>/../Resources/IR-manifest.json"
)

add_executable(preverb_build_smoke tests/build_smoke.cpp)
target_link_libraries(preverb_build_smoke PRIVATE juce::juce_core)

if(BUILD_TESTING)
    add_executable(preverb_presentation_policy_test tests/presentation_policy_test.cpp)
    target_include_directories(preverb_presentation_policy_test PRIVATE Source)
    target_link_libraries(preverb_presentation_policy_test PRIVATE juce::juce_core)
    add_test(NAME preverb_presentation_policy_test COMMAND preverb_presentation_policy_test)
    add_executable(preverb_source_edges_test tests/source_edges_test.cpp)
    target_link_libraries(preverb_source_edges_test PRIVATE preverb_preview)
    add_test(NAME preverb_source_edges_test COMMAND preverb_source_edges_test)
    add_executable(preverb_host_grid_test tests/host_grid_test.cpp)
    target_include_directories(preverb_host_grid_test PRIVATE Source)
    add_test(NAME preverb_host_grid_test COMMAND preverb_host_grid_test)
    add_executable(preverb_timeline_controls_test tests/timeline_controls_test.cpp)
    target_compile_definitions(preverb_timeline_controls_test PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_timeline_controls_test PRIVATE preverb_ui_resources juce::juce_gui_extra)
    target_include_directories(preverb_timeline_controls_test PRIVATE Source)
    add_test(NAME preverb_timeline_controls_test COMMAND preverb_timeline_controls_test)
    add_executable(preverb_swell_list_order_test tests/swell_list_order_test.cpp)
    target_link_libraries(preverb_swell_list_order_test PRIVATE preverb_session)
    add_test(NAME preverb_swell_list_order_test COMMAND preverb_swell_list_order_test)
    add_executable(preverb_swell_list_test tests/swell_list_test.cpp)
    target_compile_definitions(preverb_swell_list_test PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_swell_list_test PRIVATE preverb_ui_resources juce::juce_gui_extra)
    target_include_directories(preverb_swell_list_test PRIVATE Source)
    add_test(NAME preverb_swell_list_test COMMAND preverb_swell_list_test)
    find_program(PREVERB_UI_POWERSHELL NAMES powershell pwsh REQUIRED)
    add_test(NAME preverb_ui_standards_check COMMAND "${PREVERB_UI_POWERSHELL}"
        -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/scripts/check-ui-standards.ps1")
    add_test(NAME preverb_ui_standards_lint_test COMMAND "${PREVERB_UI_POWERSHELL}"
        -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/tests/ui_standards_lint_test.ps1")
    math(EXPR product_version_code "(${PROJECT_VERSION_MAJOR} << 16) + (${PROJECT_VERSION_MINOR} << 8) + ${PROJECT_VERSION_PATCH}")
    add_executable(preverb_product_version_test tests/product_version_test.cpp)
    target_include_directories(preverb_product_version_test PRIVATE Source "${CMAKE_BINARY_DIR}/generated")
    target_compile_definitions(preverb_product_version_test PRIVATE
        $<TARGET_PROPERTY:preverb_plugin,COMPILE_DEFINITIONS>
        EXPECTED_PRODUCT_VERSION="${PROJECT_VERSION}"
        EXPECTED_PRODUCT_VERSION_CODE=${product_version_code})
    add_test(NAME preverb_product_version_test COMMAND preverb_product_version_test)
    add_test(NAME preverb_version_contract_test COMMAND ${CMAKE_COMMAND}
        "-DTEST_ROOT=${CMAKE_BINARY_DIR}/version-contract-fixtures" -P "${CMAKE_SOURCE_DIR}/tests/product_version_test.cmake")
    add_test(NAME preverb_build_information_test COMMAND ${CMAKE_COMMAND}
        "-DTEST_ROOT=${CMAKE_BINARY_DIR}/build-information-fixtures" -P "${CMAKE_SOURCE_DIR}/tests/build_information_test.cmake")
    add_test(NAME preverb_user_guide_currency_test COMMAND "${PREVERB_UI_POWERSHELL}"
        -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/tests/user_guide_currency_test.ps1")
    add_test(NAME preverb_built_version_test COMMAND ${CMAKE_COMMAND}
        "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}" "-DBINARY_DIR=${CMAKE_BINARY_DIR}" "-DCONFIGURATION=$<CONFIG>"
        -P "${CMAKE_SOURCE_DIR}/scripts/verify-version.cmake")
    add_executable(preverb_workflow_processor_test tests/workflow_processor_test.cpp)
    target_link_libraries(preverb_workflow_processor_test PRIVATE preverb_plugin)
    add_test(NAME preverb_workflow_processor_test COMMAND preverb_workflow_processor_test)
    add_executable(preverb_capture_transaction_test tests/capture_transaction_test.cpp)
    target_link_libraries(preverb_capture_transaction_test PRIVATE preverb_capture)
    add_test(NAME preverb_capture_transaction_test COMMAND preverb_capture_transaction_test)
    add_executable(preverb_capture_failure_test tests/capture_failure_test.cpp)
    target_link_libraries(preverb_capture_failure_test PRIVATE preverb_capture)
    add_test(NAME preverb_capture_failure_test COMMAND preverb_capture_failure_test)
    add_executable(preverb_capture_failure_mailbox_test tests/capture_failure_mailbox_test.cpp)
    target_include_directories(preverb_capture_failure_mailbox_test PRIVATE Source)
    add_test(NAME preverb_capture_failure_mailbox_test COMMAND preverb_capture_failure_mailbox_test)
    add_executable(preverb_capture_context_test tests/capture_context_test.cpp)
    target_link_libraries(preverb_capture_context_test PRIVATE preverb_capture)
    add_test(NAME preverb_capture_context_test COMMAND preverb_capture_context_test)
    add_executable(preverb_workflow_state_test tests/workflow_state_test.cpp)
    target_link_libraries(preverb_workflow_state_test PRIVATE preverb_session)
    add_test(NAME preverb_workflow_state_test COMMAND preverb_workflow_state_test)
    add_executable(preverb_gain_staging_test tests/gain_staging_test.cpp)
    target_link_libraries(preverb_gain_staging_test PRIVATE preverb_assets)
    add_test(NAME preverb_gain_staging_test COMMAND preverb_gain_staging_test)
    add_executable(preverb_ir_review tests/ir_library_review.cpp)
    target_link_libraries(preverb_ir_review PRIVATE preverb_assets)
    add_executable(preverb_gain_review tests/gain_listening_review.cpp)
    target_link_libraries(preverb_gain_review PRIVATE preverb_assets)
    add_executable(preverb_ir_library_test tests/ir_library_test.cpp)
    target_link_libraries(preverb_ir_library_test PRIVATE preverb_assets psapi)
    add_test(NAME preverb_ir_library_test COMMAND preverb_ir_library_test "${CMAKE_BINARY_DIR}/ir-library-metrics-$<CONFIG>.json")
    add_executable(preverb_ir_handover_test tests/ir_handover_test.cpp)
    target_link_libraries(preverb_ir_handover_test PRIVATE preverb_preview)
    add_test(NAME preverb_ir_handover_test COMMAND preverb_ir_handover_test)
    add_executable(preverb_ir_choices_test tests/ir_choices_test.cpp)
    target_link_libraries(preverb_ir_choices_test PRIVATE preverb_preview)
    add_test(NAME preverb_ir_choices_test COMMAND preverb_ir_choices_test)
    add_executable(preverb_generation_configuration_test tests/generation_configuration_test.cpp)
    target_link_libraries(preverb_generation_configuration_test PRIVATE preverb_preview)
    add_test(NAME preverb_generation_configuration_test COMMAND preverb_generation_configuration_test)
    add_executable(preverb_learn_generation_test tests/learn_generation_test.cpp)
    target_link_libraries(preverb_learn_generation_test PRIVATE preverb_plugin juce::juce_audio_utils)
    add_test(NAME preverb_learn_generation_test COMMAND preverb_learn_generation_test)
    add_executable(preverb_learn_generation_ui_test tests/learn_generation_ui_test.cpp)
    target_compile_definitions(preverb_learn_generation_ui_test PRIVATE PREVERB_BUILD_INFO_FILE="${CMAKE_BINARY_DIR}/build-information.json")
    target_compile_definitions(preverb_learn_generation_ui_test PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_learn_generation_ui_test PRIVATE preverb_plugin juce::juce_audio_utils)
    add_test(NAME preverb_learn_generation_ui_test COMMAND preverb_learn_generation_ui_test)
    add_executable(preverb_ir_streaming_test tests/ir_streaming_test.cpp)
    target_link_libraries(preverb_ir_streaming_test PRIVATE preverb_preview psapi)
    add_test(NAME preverb_ir_streaming_test COMMAND preverb_ir_streaming_test "${CMAKE_BINARY_DIR}/ir-streaming-metrics-$<CONFIG>.json")
    add_executable(preverb_export_voice_test tests/export_voice_test.cpp)
    target_link_libraries(preverb_export_voice_test PRIVATE preverb_preview)
    add_test(NAME preverb_export_voice_test COMMAND preverb_export_voice_test)
    add_executable(preverb_song_export_engine_test tests/song_export_engine_test.cpp)
    target_link_libraries(preverb_song_export_engine_test PRIVATE preverb_preview)
    add_test(NAME preverb_song_export_engine_test COMMAND preverb_song_export_engine_test)
    add_executable(preverb_song_export_test tests/song_export_test.cpp)
    target_link_libraries(preverb_song_export_test PRIVATE preverb_assets)
    add_test(NAME preverb_song_export_test COMMAND preverb_song_export_test)
    add_executable(preverb_audition_mix_test tests/audition_mix_test.cpp)
    target_link_libraries(preverb_audition_mix_test PRIVATE preverb_preview)
    add_test(NAME preverb_audition_mix_test COMMAND preverb_audition_mix_test)
    add_executable(preverb_transition_feedback_test tests/transition_feedback_test.cpp)
    target_link_libraries(preverb_transition_feedback_test PRIVATE preverb_preview)
    add_test(NAME preverb_transition_feedback_test COMMAND preverb_transition_feedback_test)
    add_executable(preverb_automatic_coordinator_test tests/automatic_coordinator_test.cpp)
    target_link_libraries(preverb_automatic_coordinator_test PRIVATE preverb_preview)
    add_test(NAME preverb_automatic_coordinator_test COMMAND preverb_automatic_coordinator_test)
    add_executable(preverb_automatic_targets_test tests/automatic_targets_test.cpp)
    target_link_libraries(preverb_automatic_targets_test PRIVATE preverb_analysis)
    add_test(NAME preverb_automatic_targets_test COMMAND preverb_automatic_targets_test)
    add_executable(preverb_automatic_generation_test tests/automatic_generation_test.cpp)
    target_link_libraries(preverb_automatic_generation_test PRIVATE preverb_analysis)
    add_test(NAME preverb_automatic_generation_test COMMAND preverb_automatic_generation_test)
    add_executable(preverb_learn_candidates_test tests/learn_candidates_test.cpp)
    target_link_libraries(preverb_learn_candidates_test PRIVATE preverb_analysis)
    add_test(NAME preverb_learn_candidates_test COMMAND preverb_learn_candidates_test)
    add_executable(preverb_swell_viewport_test tests/swell_viewport_test.cpp)
    target_link_libraries(preverb_swell_viewport_test PRIVATE preverb_session)
    add_test(NAME preverb_swell_viewport_test COMMAND preverb_swell_viewport_test)
    add_executable(preverb_initial_perform_view_test tests/initial_perform_view_test.cpp)
    target_link_libraries(preverb_initial_perform_view_test PRIVATE preverb_session)
    add_test(NAME preverb_initial_perform_view_test COMMAND preverb_initial_perform_view_test)
    add_executable(preverb_swell_ui_test tests/swell_ui_test.cpp)
    target_compile_definitions(preverb_swell_ui_test PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_swell_ui_test PRIVATE preverb_plugin juce::juce_audio_utils)
    if(MSVC)
        # The broad Debug fixture alone uses ~746 KiB before nested native UI
        # and graphics calls. Keep headroom beyond Windows' default 1 MiB.
        target_link_options(preverb_swell_ui_test PRIVATE /STACK:8388608)
    endif()
    add_test(NAME preverb_swell_ui_test COMMAND preverb_swell_ui_test)
    add_executable(preverb_transport_readout_test tests/transport_readout_test.cpp)
    target_compile_definitions(preverb_transport_readout_test PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_transport_readout_test PRIVATE preverb_plugin juce::juce_audio_utils)
    add_test(NAME preverb_transport_readout_test COMMAND preverb_transport_readout_test)
    add_executable(preverb_waveform_display_test tests/waveform_display_test.cpp)
    target_link_libraries(preverb_waveform_display_test PRIVATE preverb_plugin juce::juce_audio_utils)
    add_test(NAME preverb_waveform_display_test COMMAND preverb_waveform_display_test)
    add_executable(preverb_waveform_paint_test tests/waveform_paint_test.cpp)
    target_link_libraries(preverb_waveform_paint_test PRIVATE preverb_plugin juce::juce_audio_utils)
    add_test(NAME preverb_waveform_paint_test COMMAND preverb_waveform_paint_test)
    # Optional measurements, kept separate from correctness/CTest timings.
    add_executable(preverb_waveform_paint_benchmark tests/waveform_paint_benchmark.cpp)
    target_link_libraries(preverb_waveform_paint_benchmark PRIVATE preverb_plugin juce::juce_audio_utils)
    add_executable(preverb_waveform_editor_benchmark tests/waveform_editor_benchmark.cpp)
    target_link_libraries(preverb_waveform_editor_benchmark PRIVATE preverb_plugin juce::juce_audio_utils)
    add_executable(preverb_manual_preparation_test tests/manual_preparation_test.cpp)
    target_compile_definitions(preverb_manual_preparation_test PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_manual_preparation_test PRIVATE preverb_plugin juce::juce_audio_utils WavPack::WavPack)
    add_test(NAME preverb_manual_preparation_test COMMAND preverb_manual_preparation_test)
    # Native composition checkpoints; #58's explicit review seam is not the production default.
    add_executable(preverb_perform_composition_preview tests/perform_composition_preview.cpp)
    target_compile_definitions(preverb_perform_composition_preview PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_perform_composition_preview PRIVATE preverb_plugin juce::juce_audio_utils)
    add_test(NAME preverb_perform_composition_preview COMMAND preverb_perform_composition_preview)
    add_test(NAME preverb_perform_navigation_test COMMAND preverb_perform_composition_preview --test-navigation)
    add_test(NAME preverb_perform_gesture_test COMMAND preverb_perform_composition_preview --test-gestures)
    add_test(NAME preverb_perform_binding_test COMMAND preverb_perform_composition_preview --test-editor)
    add_test(NAME preverb_perform_fit_test COMMAND preverb_perform_composition_preview --test-fit)
    add_test(NAME preverb_perform_knob_test COMMAND preverb_perform_composition_preview --test-knobs)
    add_test(NAME preverb_perform_knob_reset_test COMMAND preverb_perform_composition_preview --test-knob-resets)
    add_test(NAME preverb_perform_audition_test COMMAND preverb_perform_composition_preview --test-audition)
    add_test(NAME preverb_perform_audition_strip_test COMMAND preverb_perform_composition_preview --test-audition-strip)
    add_test(NAME preverb_perform_readability_test COMMAND preverb_perform_composition_preview --test-readability)
    add_test(NAME preverb_perform_hover_help_test COMMAND preverb_perform_composition_preview --test-hover-help)
    add_test(NAME preverb_responsive_editor_test COMMAND preverb_perform_composition_preview --test-responsive)
    add_test(NAME preverb_responsive_scale_test COMMAND preverb_perform_composition_preview --test-responsive-scale)
    add_executable(preverb_scoped_recapture_test tests/scoped_recapture_test.cpp)
    target_link_libraries(preverb_scoped_recapture_test PRIVATE preverb_capture)
    add_test(NAME preverb_scoped_recapture_test COMMAND preverb_scoped_recapture_test)
    add_executable(preverb_swell_engine_test tests/swell_engine_test.cpp)
    target_link_libraries(preverb_swell_engine_test PRIVATE preverb_preview WavPack::WavPack)
    add_test(NAME preverb_swell_engine_test COMMAND preverb_swell_engine_test)
    add_executable(preverb_swell_analysis_test tests/swell_analysis_test.cpp)
    target_link_libraries(preverb_swell_analysis_test PRIVATE preverb_analysis)
    add_test(NAME preverb_swell_analysis_test COMMAND preverb_swell_analysis_test)
    add_executable(preverb_swell_shape_test tests/swell_shape_test.cpp)
    target_link_libraries(preverb_swell_shape_test PRIVATE preverb_assets)
    add_test(NAME preverb_swell_shape_test COMMAND preverb_swell_shape_test)
    add_executable(preverb_swell_asset_test tests/swell_asset_test.cpp)
    target_link_libraries(preverb_swell_asset_test PRIVATE preverb_assets WavPack::WavPack)
    add_test(NAME preverb_swell_asset_test COMMAND preverb_swell_asset_test)
    add_executable(preverb_swell_session_test tests/swell_session_test.cpp)
    target_link_libraries(preverb_swell_session_test PRIVATE preverb_session)
    add_test(NAME preverb_swell_session_test COMMAND preverb_swell_session_test)
    add_executable(preverb_transport_time_test tests/transport_time_test.cpp)
    target_link_libraries(preverb_transport_time_test PRIVATE preverb_session)
    add_test(NAME preverb_transport_time_test COMMAND preverb_transport_time_test)
    add_executable(preverb_transport_playback_test tests/transport_playback_test.cpp)
    target_link_libraries(preverb_transport_playback_test PRIVATE preverb_preview)
    add_test(NAME preverb_transport_playback_test COMMAND preverb_transport_playback_test)
    add_executable(preverb_wavpack_ir_test tests/wavpack_ir_test.cpp)
    target_link_libraries(preverb_wavpack_ir_test PRIVATE preverb_dsp WavPack::WavPack)
    add_test(NAME preverb_wavpack_ir_test COMMAND preverb_wavpack_ir_test)
    add_executable(preverb_create_m2_test_media tests/create_m2_test_media.cpp)
    target_link_libraries(preverb_create_m2_test_media PRIVATE preverb_dsp)
    add_executable(preverb_audition_voice_test tests/audition_voice_test.cpp)
    target_link_libraries(preverb_audition_voice_test PRIVATE preverb_preview)
    add_test(NAME preverb_audition_voice_test COMMAND preverb_audition_voice_test)
    add_executable(preverb_preview_engine_test tests/preview_engine_test.cpp)
    target_link_libraries(preverb_preview_engine_test PRIVATE preverb_preview)
    add_test(NAME preverb_preview_engine_test COMMAND preverb_preview_engine_test)
    add_executable(preverb_reverse_convolution_test tests/reverse_convolution_test.cpp)
    target_link_libraries(preverb_reverse_convolution_test PRIVATE preverb_dsp)
    add_test(NAME preverb_reverse_convolution_test COMMAND preverb_reverse_convolution_test)
    add_executable(preverb_source_range_test tests/source_range_test.cpp)
    target_link_libraries(preverb_source_range_test PRIVATE preverb_capture)
    add_test(NAME preverb_source_range_test COMMAND preverb_source_range_test)
    add_test(NAME preverb_build_smoke COMMAND preverb_build_smoke)

    add_executable(preverb_processor_test tests/processor_test.cpp)
    target_link_libraries(preverb_processor_test PRIVATE
        preverb_plugin
        juce::juce_audio_utils
    )
    add_test(NAME preverb_processor_test COMMAND preverb_processor_test)

    add_executable(preverb_capture_test tests/capture_map_test.cpp)
    target_link_libraries(preverb_capture_test PRIVATE preverb_capture)
    add_test(NAME preverb_capture_test COMMAND preverb_capture_test)

    add_executable(preverb_persistent_capture_test tests/persistent_capture_test.cpp)
    target_link_libraries(preverb_persistent_capture_test PRIVATE preverb_capture)
    add_test(NAME preverb_persistent_capture_test COMMAND preverb_persistent_capture_test)

    add_executable(preverb_scheduled_tone_test tests/scheduled_tone_test.cpp)
    target_link_libraries(preverb_scheduled_tone_test PRIVATE preverb_playback)
    add_test(NAME preverb_scheduled_tone_test COMMAND preverb_scheduled_tone_test)

    add_executable(preverb_song_bar_ruler_test tests/song_bar_ruler_test.cpp)
    target_include_directories(preverb_song_bar_ruler_test PRIVATE Source)
    add_test(NAME preverb_song_bar_ruler_test COMMAND preverb_song_bar_ruler_test)
endif()

if(BUILD_TESTING)
    # Opt-in disk-growth audit; CTest runs only its small compatibility fixtures.
    add_executable(preverb_storage_audit tests/storage_audit.cpp)
    target_link_libraries(preverb_storage_audit PRIVATE preverb_assets Psapi)
    add_test(NAME preverb_storage_audit_compatibility COMMAND preverb_storage_audit)
endif()

add_library(preverb_storage STATIC Source/Storage/AssetInventory.cpp Source/Storage/PortableAssets.cpp)
target_include_directories(preverb_storage PUBLIC Source)
target_link_libraries(preverb_storage PUBLIC preverb_assets)
target_link_libraries(preverb_plugin PUBLIC preverb_storage)
if(BUILD_TESTING)
    add_executable(preverb_asset_inventory_test tests/asset_inventory_test.cpp)
    target_link_libraries(preverb_asset_inventory_test PRIVATE preverb_storage)
    add_test(NAME preverb_asset_inventory_test COMMAND preverb_asset_inventory_test)
    add_executable(preverb_storage_inventory_preview tests/storage_inventory_preview.cpp)
    target_compile_definitions(preverb_storage_inventory_preview PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_storage_inventory_preview PRIVATE preverb_storage preverb_ui_resources juce::juce_audio_utils)
    add_test(NAME preverb_storage_inventory_panel_test COMMAND preverb_storage_inventory_preview)
endif()

if(BUILD_TESTING)
    add_executable(preverb_storage_settings_test tests/storage_settings_test.cpp)
    target_compile_definitions(preverb_storage_settings_test PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_storage_settings_test PRIVATE preverb_plugin juce::juce_audio_utils)
    add_test(NAME preverb_storage_settings_test COMMAND preverb_storage_settings_test)
endif()

# Explicitly opt-in; default product builds do not observe or log host metadata.
option(PREVERB_PROJECT_FOLDER_PROBE "Enable diagnostic task 1.4 VST3 state-stream metadata logging" OFF)
if(PREVERB_PROJECT_FOLDER_PROBE)
    include(cmake/ProjectFolderProbe.cmake)
endif()

if(BUILD_TESTING)
    add_executable(preverb_portable_assets_test tests/portable_assets_test.cpp)
    target_link_libraries(preverb_portable_assets_test PRIVATE preverb_storage)
    add_test(NAME preverb_portable_assets_test COMMAND preverb_portable_assets_test)
    add_test(NAME preverb_working_folder_test COMMAND preverb_portable_assets_test --working-folder)
endif()

if(BUILD_TESTING)
    add_executable(preverb_portable_snapshot_test tests/portable_snapshot_test.cpp)
    target_link_libraries(preverb_portable_snapshot_test PRIVATE preverb_plugin juce::juce_audio_utils)
    add_test(NAME preverb_portable_snapshot_test COMMAND preverb_portable_snapshot_test)
endif()

if(BUILD_TESTING)
    add_executable(preverb_portable_storage_preview tests/portable_storage_preview.cpp)
    target_compile_definitions(preverb_portable_storage_preview PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_portable_storage_preview PRIVATE preverb_storage preverb_ui_resources juce::juce_audio_utils)
    add_test(NAME preverb_portable_storage_panel_test COMMAND preverb_portable_storage_preview)
    add_test(NAME preverb_working_folder_panel_test COMMAND preverb_portable_storage_preview --working-folder)
endif()

if(BUILD_TESTING)
    add_executable(preverb_portable_recovery_test tests/portable_recovery_test.cpp)
    target_compile_definitions(preverb_portable_recovery_test PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(preverb_portable_recovery_test PRIVATE preverb_plugin juce::juce_audio_utils Userenv)
    add_test(NAME preverb_portable_recovery_test COMMAND preverb_portable_recovery_test)
endif()

if(BUILD_TESTING)
    add_test(NAME preverb_recovery_panel_test COMMAND preverb_portable_recovery_test --ui)
endif()

if(BUILD_TESTING)
    add_test(NAME preverb_shared_swell_engine_test COMMAND preverb_workflow_processor_test --shared-engine)
endif()
