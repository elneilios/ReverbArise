#pragma once
#include "PluginProcessor.h"
#include <UI/SwellTimeline.h>
#include <UI/WorkspaceTheme.h>
#include <UI/WorkspaceLayout.h>
#include <UI/KnobValue.h>
#include <UI/WorkflowHeader.h>

class PreverbAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer,private juce::ListBoxModel
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
    juce::Label inspectorHeading,hostInfo,listEmpty,inspectorEmpty,auditionHeading,editorReadout;
    enum Field {name,arrival,sourceA,sourceB,audibleA,audibleB,fadeIn,fadeOut,fieldCount};
    void timerCallback() override {refresh();}
    void setDiagnosticsOpen(bool);
    void setSettingsOpen(bool);
    void initialiseSettings();
    void layoutSettings();
    int getNumRows() override;
    void paintListBoxItem(int,juce::Graphics&,int,int,bool) override;
    void selectedRowsChanged(int) override;
    void listBoxItemClicked(int,const juce::MouseEvent&) override;
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
    m3::ui::InvocationComboBox irBox;
    struct IRInvocation {juce::String id;uint64_t input=0,epoch=0;};
    std::optional<IRInvocation> irInvocation;
    juce::TextButton resetMenu{"v"};
    juce::String irOwner,irChoice;
    PreverbAudioProcessor& processor;
    m3::EditorSnapshot state;
    m3::SwellTimeline timeline;
    m3::SwellOverview songOverview{timeline};
    juce::ListBox list{"Swells",this};
    juce::Label status,detail,listLabel,auditionLabel,separationLabel,spanLabel,barStartLabel,barEndLabel,recaptureStatus,toneStartLabel,toneEndLabel;
    juce::TextButton addButton{"Add at playhead"},removeButton{"Delete"},loadIRButton{"Load IR..."},generateButton{"Regenerate selected"},allButton{"Regenerate all"},auditionButton{"Audition wet"},stopButton{"Stop"};
    juce::TextButton analyzeButton{"Suggest onsets"},confirmButton{"Confirm"},previousButton{"Prev"},nextButton{"Next"},zoomInButton{"Zoom +"},zoomOutButton{"Zoom -"},fitButton{"Fit"};
    juce::TextButton recaptureButton{"Recapture & regenerate"},cancelButton{"Cancel recapture"},fullButton{"Full response"},resetButton{"Reset shape"},setBarsButton{"Set source from bars"},clearButton{"Clear map"};
    juce::TextButton assetInfoButton{"Copy asset info"};
    juce::ToggleButton snapButton{"Snap"},autoButton{"Auto-refresh after recapture"},enabledButton{"Enabled"},muteButton{"Mute"},soloButton{"Solo"},mappingButton{"Map while playing"},toneButton{"440 Hz test tone"};
    juce::Slider auditionSlider,separationSlider,toneStartSlider,toneEndSlider;
    juce::ComboBox candidateBox,spanBox,inCurveBox,outCurveBox;
    std::array<juce::Label,fieldCount> fieldLabels;
    std::array<juce::TextEditor,fieldCount> fields;
    juce::TextEditor barStartEditor,barEndEditor;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::String fittedKey;
    std::optional<int64_t> chosenCandidate;
    bool updating=false,destroying=false;
    m3::ui::FlatViewport inspectorViewport,diagnosticsViewport;
    juce::Component inspectorContent;
    m3::ui::FlatSurface diagnosticsContent;
    juce::Label captureDiagnostic,playbackDiagnostic,policyDiagnostic;
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
    juce::ToggleButton automaticButton{"Automatic processing"},sampleUnitsButton{"Exact sample positions"};
    juce::Slider regenerationKnob;juce::Label regenerationLabel;juce::ToggleButton freezeButton{"Freeze render"};
    juce::Slider markerKnob;juce::ToggleButton markerButton{"Auto markers"},isolateButton{"Isolate selected"},isolateDryButton{"Isolate dry source"};
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
