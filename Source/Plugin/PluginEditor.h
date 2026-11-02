#pragma once
#include "PluginProcessor.h"
#include <UI/SwellTimeline.h>
#include <UI/WorkspaceTheme.h>
#include <UI/WorkspaceLayout.h>
#include <UI/KnobValue.h>

class PreverbAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer,private juce::ListBoxModel
{
public:
    explicit PreverbAudioProcessorEditor(PreverbAudioProcessor&);
    ~PreverbAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void refresh();
    m3::SwellTimeline& timelineComponent() noexcept {return timeline;}
private:
    m3::ui::WorkspaceTheme workspaceTheme;
    m3::ui::Wordmark wordmark;
    juce::Label tagline,inspectorHeading,hostInfo,listEmpty,inspectorEmpty,auditionHeading,editorReadout;
    enum Field {name,arrival,sourceA,sourceB,audibleA,audibleB,fadeIn,fadeOut,fieldCount};
    void timerCallback() override {refresh();}
    int getNumRows() override;
    void paintListBoxItem(int,juce::Graphics&,int,int,bool) override;
    void selectedRowsChanged(int) override;
    void listBoxItemClicked(int,const juce::MouseEvent&) override;
    void commitField(int);
    void curveChanged(bool);
    void setPreferences();
    void applyBarRange();
    void armSelected();
    void shapeReset(bool full);
    void fieldBounds(juce::Rectangle<int>&,int field,int labelWidth,int editorWidth);
    enum InspectorValue {targetValue,sourceStartValue,sourceEndValue,preRollValue,endOffsetValue,fadeInValue,fadeOutValue,levelValue,inCurveValue,outCurveValue,placementValue,valueCount};
    void beginValueEdit(int);
    bool commitValueEdit(int,const juce::String&,std::optional<double> delta={});
    void refreshValues(const m3::Swell*,const SongTime&);
    std::array<m3::ui::KnobValue,valueCount> valueControls;
    std::array<std::optional<m3::Swell>,valueCount> valueAnchors;
    std::array<double,valueCount> valueSpans{};
    std::array<uint64_t,valueCount> valueTokens{};
    std::array<int64_t,valueCount> valueRawStarts{},valueRawEnds{};
    SongTime valueTime;
    juce::ComboBox irBox;
    juce::TextButton resetMenu{"v"};
    juce::String irOwner,irChoice;
    PreverbAudioProcessor& processor;
    m3::EditorSnapshot state;
    m3::SwellTimeline timeline;
    m3::SwellOverview songOverview{timeline};
    juce::ListBox list{"Swells",this};
    juce::Label title,status,detail,listLabel,auditionLabel,sensitivityLabel,separationLabel,spanLabel,curveInLabel,curveOutLabel,gainLabel,barStartLabel,barEndLabel,recaptureStatus,toneStartLabel,toneEndLabel;
    juce::TextButton addButton{"Add at playhead"},removeButton{"Delete"},loadIRButton{"Load IR..."},generateButton{"Regenerate selected"},allButton{"Regenerate all"},auditionButton{"Audition wet"},stopButton{"Stop"};
    juce::TextButton analyzeButton{"Suggest onsets"},confirmButton{"Confirm"},previousButton{"Prev"},nextButton{"Next"},zoomInButton{"Zoom +"},zoomOutButton{"Zoom -"},fitButton{"Fit"};
    juce::TextButton recaptureButton{"Recapture & regenerate"},cancelButton{"Cancel recapture"},fullButton{"Full response"},resetButton{"Reset shape"},setBarsButton{"Set source from bars"},clearButton{"Clear map"};
    juce::TextButton assetInfoButton{"Copy asset info"};
    juce::ToggleButton snapButton{"Snap"},autoButton{"Auto-refresh after recapture"},enabledButton{"Enabled"},muteButton{"Mute"},soloButton{"Solo"},mappingButton{"Map while playing"},toneButton{"440 Hz test tone"};
    juce::Slider auditionSlider,sensitivitySlider,separationSlider,gainSlider,toneStartSlider,toneEndSlider;
    juce::ComboBox candidateBox,spanBox,inCurveBox,outCurveBox;
    std::array<juce::Label,fieldCount> fieldLabels;
    std::array<juce::TextEditor,fieldCount> fields;
    juce::TextEditor barStartEditor,barEndEditor;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::String fittedKey;
    std::optional<int64_t> chosenCandidate;
    bool updating=false,destroying=false;
    juce::Viewport inspectorViewport,diagnosticsViewport;
    juce::Component inspectorContent,diagnosticsContent;
    juce::Label headerStatus,selectedStatus,irStatus,monitorStatus,helper;
    juce::TextButton diagnosticsButton{"Diagnostics"},wetOnlyButton{"Wet Only"},dryWetButton{"Dry + Wet"},trimResetButton{"Trim 0 dB"};
    juce::TextButton defaultIRButton{"Default IR for future..."},suppressionButton{"Reset deleted targets"};
    juce::ToggleButton automaticButton{"Automatic processing"},sampleUnitsButton{"Exact sample positions"};
    juce::Slider regenerationKnob;juce::Label regenerationLabel;juce::ToggleButton freezeButton{"Freeze render"};
    juce::Slider markerKnob;juce::ToggleButton markerButton{"Auto markers"},isolateButton{"Isolate selected"},isolateDryButton{"Isolate dry source"};
    juce::Label markerLabel,markerState,placementLabel;juce::TextEditor placementEditor;
    juce::ToggleButton liveButton{"Live playback"};juce::ComboBox liveRoutingBox;juce::Slider liveLevel;
    juce::Label liveLevelLabel;
    juce::TextButton nudgeLeft{"<"},nudgeRight{">"};
    juce::Slider blendSlider,fadeInKnob,fadeOutKnob,inTensionSlider,outTensionSlider,tailSlider;
    juce::Label inTensionLabel,outTensionLabel,tailLabel;
    bool diagnosticsOpen=false;
    juce::TextButton timingButton{"Source and audible timing..."};
    bool advancedTiming=false;
};
