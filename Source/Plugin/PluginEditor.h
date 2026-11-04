#pragma once
#include "PluginProcessor.h"
#include <UI/SwellTimeline.h>
#include <UI/WorkspaceTheme.h>
#include <UI/WorkspaceLayout.h>
#include <UI/KnobValue.h>
#include <UI/Controls/IrSelector.h>
#include <UI/WorkflowHeader.h>
#include <UI/WorkspaceNotice.h>
#include <UI/Controls/InvocationSlider.h>
#include <UI/LearnWorkspace.h>
#include <UI/ExportWorkspace.h>
#include <UI/Controls/SwellList.h>

class PreverbAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer
{
public:
    explicit PreverbAudioProcessorEditor(PreverbAudioProcessor&);
    ~PreverbAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void refresh();
    m3::SwellTimeline& timelineComponent() noexcept {return timeline;}
    juce::Component* diagnosticsPanel() noexcept {return &diagnosticsViewport;}
    juce::Component* settingsPanel() noexcept {return &settingsTabs;}
    juce::Rectangle<int> swellMenuAnchor(const juce::MouseEvent& event)const;
private:
    m3::ui::WorkspaceTheme workspaceTheme;
    m3::ui::WorkflowHeader workflowHeader;
    m3::ui::controls::NoticeBar noticeBar;
    m3::ui::IRPreparationNotice irPreparationNotice;
    juce::Label inspectorHeading,hostInfo,listEmpty,inspectorEmpty,auditionHeading,editorReadout;
    enum Field {name,arrival,sourceA,sourceB,audibleA,audibleB,fadeIn,fadeOut,fieldCount};
    void timerCallback() override {refresh();}
    void setDiagnosticsOpen(bool);
    void setSettingsOpen(bool);
    void initialiseSettings();
    void layoutSettings();
    void showSwellMenu(const m3::ui::controls::SwellListRow&,const juce::MouseEvent&);
    void curveChanged(bool);
    void setPreferences();
    void applyBarRange();
    void armSelected();
    void shapeReset(bool full);
    enum InspectorValue {targetValue,sourceStartValue,sourceEndValue,preRollValue,endOffsetValue,fadeInValue,fadeOutValue,levelValue,inCurveValue,outCurveValue,placementValue,valueCount};
    void beginValueEdit(int);
    bool commitValueEdit(int,const juce::String&,std::optional<double> delta={});
    void refreshValues(const m3::Swell*,const SongTime&);
    std::array<m3::ui::KnobValue,valueCount> valueControls;
    std::array<std::optional<m3::Swell>,valueCount> valueAnchors;
    std::array<double,valueCount> valueSpans{};
    std::array<uint64_t,valueCount> valueTokens{};
    std::array<uint64_t,valueCount> valueWorkflowTokens{};
    std::array<int64_t,valueCount> valueRawStarts{},valueRawEnds{};
    SongTime valueTime;
    m3::ui::controls::IrSelector irBox;
    struct IRInvocation {juce::String id;uint64_t input=0,epoch=0;std::vector<m3::IRChoice> choices;};
    std::optional<IRInvocation> irInvocation;
    std::vector<m3::IRChoice> irEntries;
    juce::TextButton resetMenu{"v"};
    juce::String irOwner,irChoice,irCatalogue;
    PreverbAudioProcessor& processor;
    m3::ui::LearnWorkspace learnWorkspace;
    m3::ui::ExportWorkspace exportWorkspace;
    bool learningLayout=false,exportLayout=false,hasPerformWorkspace=false;
    std::map<juce::Component*,bool> performVisibility;
    std::optional<m3::SwellViewport> performViewport;
    m3::EditorSnapshot state;
    m3::SwellTimeline timeline;
    m3::SwellOverview songOverview{timeline};
    m3::ui::controls::SwellList list;
    juce::Label status,detail,listLabel,auditionLabel,separationLabel,spanLabel,barStartLabel,barEndLabel,recaptureStatus,toneStartLabel,toneEndLabel;
    juce::TextButton addButton{"Add at playhead"},removeButton{"Delete"},loadIRButton{"Load IR..."},generateButton{"Regenerate selected"},allButton{"Regenerate all"},auditionButton{"Audition wet"},stopButton{"Stop"};
    juce::TextButton analyzeButton{"Suggest onsets"},confirmButton{"Confirm"},previousButton{"Prev"},nextButton{"Next"},zoomInButton{"Zoom +"},zoomOutButton{"Zoom -"},fitButton{"Fit"};
    juce::TextButton recaptureButton{"Recapture & regenerate"},cancelButton{"Cancel recapture"},fullButton{"Full response"},resetButton{"Reset shape"},setBarsButton{"Set source from bars"},clearButton{"Clear map"};
    juce::TextButton assetInfoButton{"Copy asset info"};
    juce::ToggleButton snapButton{"Snap"},enabledButton{"Enabled"},muteButton{"Mute"},soloButton{"Solo"},mappingButton{"Map while playing"},toneButton{"440 Hz test tone"};
    m3::ui::controls::InvocationToggle autoButton{"Auto-refresh after recapture"};
    juce::Slider auditionSlider,toneStartSlider,toneEndSlider;
    m3::ui::controls::InvocationSlider separationSlider;
    std::optional<uint64_t> markerPass,separationPass,automaticPass,markingPass,refreshPass;
    juce::ComboBox candidateBox,spanBox,inCurveBox,outCurveBox;
    std::array<juce::Label,fieldCount> fieldLabels;
    std::array<juce::TextEditor,fieldCount> fields;
    juce::TextEditor barStartEditor,barEndEditor;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::String fittedKey;
    uint64_t presentationSession=0;
    std::optional<int64_t> chosenCandidate;
    bool updating=false,destroying=false;
    m3::ui::FlatViewport inspectorViewport,diagnosticsViewport;
    juce::Component inspectorContent;
    m3::ui::FlatSurface diagnosticsContent;
    void layoutLiveDiagnostics();
    juce::Label captureDiagnostic,playbackDiagnostic,policyDiagnostic,liveDiagnostic;
    std::array<juce::Component,5> settingsPages;
    juce::TabbedComponent settingsTabs{juce::TabbedButtonBar::TabsAtTop};
    std::array<juce::Label,12> settingsLabels;
    std::array<juce::Label,5> settingsHelp;
    juce::TextButton openDiagnosticsButton{"Open diagnostics..."};
    std::unique_ptr<juce::DocumentWindow> settingsWindow;
    std::unique_ptr<juce::DocumentWindow> diagnosticsWindow;
    juce::Label headerStatus,selectedStatus,irStatus,monitorStatus,helper;
    juce::ToggleButton recaptureExisting{"Recapture existing audio"};
    juce::TextButton trimResetButton{"Trim 0 dB"};
    juce::TextButton defaultIRButton{"Default IR for future..."},suppressionButton{"Reset deleted targets"};
    m3::ui::controls::InvocationToggle automaticButton{"Automatic processing"};juce::ToggleButton sampleUnitsButton{"Exact sample positions"};
    juce::Slider regenerationKnob;juce::Label regenerationLabel;juce::ToggleButton freezeButton{"Freeze render"};
    m3::ui::controls::InvocationSlider markerKnob;m3::ui::controls::InvocationToggle markerButton{"Auto markers"};juce::ToggleButton isolateButton{"Isolate selected"},isolateDryButton{"Isolate dry source"};
    juce::Label markerLabel,markerState,placementLabel;juce::TextEditor placementEditor;
    juce::ToggleButton liveButton{"Live playback"};juce::ComboBox liveRoutingBox;juce::Slider liveLevel;
    juce::Label liveLevelLabel,gainPolicyLabel;juce::ComboBox gainPolicyBox;
    juce::Slider blendSlider,tailSlider;
    juce::Label tailLabel;
    bool diagnosticsOpen=false;
    double diagnosticsRefreshedAt=-1;
    juce::TextButton timingButton{"Source and audible timing..."};
    bool advancedTiming=false;
};
