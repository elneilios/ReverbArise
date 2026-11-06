// Native compile fixture: normal builds expose neither telemetry nor a companion.
#define PREVERB_COMPATIBILITY_TEST 0
#define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1
#include <Plugin/PluginEditor.h>
template<class T> concept HasTelemetry=requires(T& v){v.compatibilityTelemetry;v.compatibilityReport();};
template<class T> concept HasCompanion=requires(T& v){v.showCompatibility();};
template<class T> concept HasDiagnosticHeader=requires(T& v){v.onCompatibility;};
static_assert(!HasTelemetry<PreverbAudioProcessor>);
static_assert(!HasCompanion<PreverbAudioProcessorEditor>);
static_assert(!HasDiagnosticHeader<m3::ui::WorkflowHeader>);
