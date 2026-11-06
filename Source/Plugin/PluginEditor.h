#pragma once
#include "PluginProcessor.h"
#include <UI/SwellTimeline.h>
#include <UI/WorkspaceTheme.h>
#include <UI/WorkspaceLayout.h>
#include <UI/KnobValue.h>
#include <UI/Controls/EllipsisLabel.h>
#include <UI/Controls/TransportReadout.h>
#include <UI/Controls/IrSelector.h>
#include <UI/WorkflowHeader.h>
#include <UI/WorkspaceNotice.h>
#include <UI/Controls/InvocationSlider.h>
#include <UI/LearnWorkspace.h>
#include <UI/ExportWorkspace.h>
#include <UI/Controls/SwellList.h>
#include <UI/PerformLayout.h>
#include <UI/Controls/TimelineZoomControls.h>
#include <UI/Controls/LatchingActionButton.h>
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/PresentationSurface.h>
#include <UI/EditorPresentationState.h>
#include <UI/Controls/FocusViewport.h>

class PreverbAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer
{
public:
    explicit PreverbAudioProcessorEditor(PreverbAudioProcessor&,std::function<void(const juce::String&)> buildInformationWriter={});
    ~PreverbAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void refresh();
    void modifierKeysChanged(const juce::ModifierKeys&)override;
    void mouseEnter(const juce::MouseEvent& event)override{updateHoverHelp(event);}
    void mouseMove(const juce::MouseEvent& event)override{updateHoverHelp(event);}
    void mouseDrag(const juce::MouseEvent& event)override{updateHoverHelp(event);}
    void mouseUp(const juce::MouseEvent& event)override{updateHoverHelp(event);}
    void mouseExit(const juce::MouseEvent&)override;
    // Native review harnesses may own polling after borrowing these components.
    // Normal plugin construction retains its existing automatic refresh.
    void stopAutomaticRefreshForReview() {stopTimer();}
    m3::SwellTimeline& timelineComponent() noexcept {return timeline;}
    juce::Component* diagnosticsPanel() noexcept {return &diagnosticsViewport;}
    juce::Component* settingsPanel() noexcept {return &settingsTabs;}
    juce::Rectangle<int> swellMenuAnchor(const juce::MouseEvent& event)const;
    // Explicit review seam; production adoption awaits combined integration/acceptance.
    void enableResponsiveReview(std::shared_ptr<m3::ui::PresentationStorage> storage={},std::optional<m3::ui::PresentationFitContext> display={});
    bool responsiveReviewEnabled()const noexcept{return responsiveReview;}
    bool setPresentationScale(int);
    bool setPresentationSize(int,int);
    void setPresentationDisplayForReview(m3::ui::PresentationFitContext);
    const m3::ui::EditorPresentationState* presentationState()const noexcept{return presentation.get();}
private:
    bool responsiveReview=false;
    bool applyingPresentation=false,updatingPresentationControls=false;
    m3::ui::controls::PresentationSurface presentationSurface;
    std::shared_ptr<m3::ui::PresentationStorage> presentationStorage;
    std::unique_ptr<m3::ui::EditorPresentationState> presentation;
    std::optional<m3::ui::PresentationFitContext> reviewDisplay;
    std::optional<m3::ui::PresentationFitContext> appliedPresentationDisplay;
    juce::Point<int> appliedPresentationSize;
    m3::ui::controls::DeliberateSelector presentationSizeBox,presentationScaleBox;
    juce::Label presentationSizeLabel,presentationScaleLabel,presentationHelp;
    void initialisePresentation();
    void applyPresentation();
    void resizePresentation();
    void servicePresentation();
    void updatePresentationControls();
    void layoutPresentationSettings();
    void updateAuxiliaryPresentation();
    void paintWorkspace(juce::Graphics&);
    m3::ui::PresentationFitContext presentationDisplay()const;
    juce::Component& workspaceRoot()noexcept{return responsiveReview?presentationSurface.contentRoot():*this;}
    juce::Rectangle<int> workspaceBounds()const noexcept{return responsiveReview?presentationSurface.contentRoot().getLocalBounds():getLocalBounds();}
    int inspectorSection=1;
    m3::ui::controls::SegmentedSelector inspectorSections;
    void layoutResponsive();
    void refreshResponsiveSections();
    m3::ui::WorkspaceTheme workspaceTheme;
    m3::ui::controls::EllipsisLabel hoverHelp;
    juce::Component::SafePointer<juce::Component> hoverHelpSource;
    void updateHoverHelp(const juce::MouseEvent&);
    void refreshHoverHelp();
    m3::ui::WorkflowHeader workflowHeader;
    m3::ui::controls::NoticeBar noticeBar;
    m3::ui::IRPreparationNotice irPreparationNotice;
    m3::ui::controls::TransportReadout hostInfo;
    juce::Label inspectorHeading,listEmpty,inspectorEmpty,auditionHeading,editorReadout,listCount;
    std::array<juce::Label,3> inspectorGroups;
    enum Field {name,arrival,sourceA,sourceB,audibleA,audibleB,fadeIn,fadeOut,fieldCount};
    void timerCallback() override {servicePresentation();modifierKeysChanged(hasKeyboardFocus(true)?juce::ModifierKeys::getCurrentModifiersRealtime():juce::ModifierKeys{});refresh();}
    void setDiagnosticsOpen(bool);
    void setSettingsOpen(bool);
    void initialiseSettings();
    void layoutSettings();
    void showSwellMenu(const m3::ui::controls::SwellListRow&,const juce::MouseEvent&);
    void curveChanged(bool);
    void setPreferences();
    void applyBarRange();
    void armSelected();
    void refreshPerformNavigation();
    void publishPerformNavigation();
    void shapeReset(bool full);
    enum InspectorValue {targetValue,sourceStartValue,sourceEndValue,preRollValue,endOffsetValue,fadeInValue,fadeOutValue,levelValue,inCurveValue,outCurveValue,placementValue,valueCount};
    void beginValueEdit(int);
    void rebaseValueEdit(int);
    void stepValueEdit(int,double);
    m3::ui::KnobValue::ResetAction prepareValueReset(int);
    struct InitialValuePositions {int64_t arrival=0,start=0,end=1;};
    void resetValueEdit(int,InitialValuePositions);
    bool valueIntentCurrent(int)const;
    bool commitValueEdit(int,const juce::String&,std::optional<double> delta={});
    void refreshValues(const m3::Swell*,const SongTime&);
    std::array<m3::ui::KnobValue,valueCount> valueControls;
    std::array<std::optional<m3::Swell>,valueCount> valueAnchors;
    std::array<double,valueCount> valueSpans{};
    std::array<uint64_t,valueCount> valueTokens{};
    std::array<uint64_t,valueCount> valueWorkflowTokens{};
    std::array<int64_t,valueCount> valueRawStarts{},valueRawEnds{};
    struct ValueInvocation {uint64_t epoch=0;std::vector<juce::String> selected;SongTime time,accepted;double rate=0;bool sampleAvailable=false;juce::String render;m3::GridClockContext clock;};
    std::array<ValueInvocation,valueCount> valueInvocations;
    int valueModifierFlags=0;
    std::map<juce::String,InitialValuePositions> initialValuePositions;
    uint64_t initialValueEpoch=0,valueResetRevision=0;
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
    juce::TextButton analyzeButton{"Suggest onsets"},confirmButton{"Confirm"};
    m3::ui::controls::TimelineZoomControls performZoom;
    m3::ui::controls::LatchingActionButton snapButton{"Snap"};
    m3::ui::controls::DeliberateSelector gridBox;
    struct NavigationScope {uint64_t epoch=0,pass=0;juce::String primary;};
    std::optional<NavigationScope> navigationInvocation;
    bool navigationIntentCurrent()const;
    bool performFitted=false;
    bool navigationDirty=false;
    uint64_t navigationEpoch=0;
    juce::TextButton recaptureButton{"Recapture & regenerate"},cancelButton{"Cancel recapture"},fullButton{"Full response"},resetButton{"Reset shape"},setBarsButton{"Set source from bars"},clearButton{"Clear map"};
    juce::TextButton assetInfoButton{"Copy asset info"};
    m3::ui::controls::ActionButton copyDiagnosticsButton{"Copy diagnostics"};
    juce::ToggleButton enabledButton{"Enabled"},muteButton{"Mute"},soloButton{"Solo"},mappingButton{"Map while playing"},toneButton{"440 Hz test tone"};
    m3::ui::controls::InvocationToggle autoButton{"Auto-refresh after recapture"};
    juce::Slider auditionSlider,toneStartSlider,toneEndSlider;
    m3::ui::controls::InvocationSlider separationSlider;
    std::optional<uint64_t> markerPass,separationPass,automaticPass,markingPass,refreshPass;
    juce::ComboBox candidateBox,inCurveBox,outCurveBox;
    std::array<juce::Label,fieldCount> fieldLabels;
    std::array<juce::TextEditor,fieldCount> fields;
    juce::TextEditor barStartEditor,barEndEditor;
    std::unique_ptr<juce::FileChooser> chooser;
    uint64_t presentationSession=0;
    std::optional<int64_t> chosenCandidate;
    bool updating=false,destroying=false;
    m3::ui::controls::FocusViewport inspectorViewport,diagnosticsViewport;
    juce::Component inspectorContent;
    m3::ui::FlatSurface diagnosticsContent;
    void layoutLiveDiagnostics();
    juce::Label captureDiagnostic,playbackDiagnostic,policyDiagnostic,liveDiagnostic;
    std::array<juce::Component,7> settingsPages;
    juce::TabbedComponent settingsTabs{juce::TabbedButtonBar::TabsAtTop};
    std::array<juce::Label,12> settingsLabels;
    std::array<juce::Label,6> settingsHelp;
    juce::Label aboutHeading;
    std::array<juce::Label,3> buildInformationLabels;
    std::array<m3::ui::controls::EllipsisLabel,3> buildInformationValues;
    m3::ui::controls::ActionButton copyBuildInformation{"Copy build information",m3::ui::controls::CommandIcon::none,true};
    std::function<void(const juce::String&)> buildInformationWriter;
    juce::TextButton openDiagnosticsButton{"Open diagnostics..."};
    std::unique_ptr<juce::DocumentWindow> settingsWindow;
    std::unique_ptr<juce::DocumentWindow> diagnosticsWindow;
    juce::Label headerStatus,helper;
    m3::ui::controls::EllipsisLabel monitorStatus;
    m3::ui::controls::EllipsisLabel selectedStatus,irStatus;
    juce::ToggleButton recaptureExisting{"Recapture existing audio"};
    m3::ui::controls::LatchingActionButton auditionWetOnly{"Wet only"};
    std::optional<std::pair<uint64_t,uint64_t>> auditionInvocation;
    juce::TextButton defaultIRButton{"Default IR for future..."},suppressionButton{"Reset deleted targets"};
    m3::ui::controls::InvocationToggle automaticButton{"Automatic processing"};juce::ToggleButton sampleUnitsButton{"Exact sample positions"};
    juce::Slider regenerationKnob;juce::Label regenerationLabel;juce::ToggleButton freezeButton{"Freeze render"};
    m3::ui::controls::InvocationSlider markerKnob;m3::ui::controls::InvocationToggle markerButton{"Auto markers"};juce::ToggleButton isolateButton{"Isolate selected"},isolateDryButton{"Isolate dry source"};
    juce::Label markerLabel,markerState,placementLabel;juce::TextEditor placementEditor;
    juce::ToggleButton liveButton{"Live playback"};juce::ComboBox liveRoutingBox;juce::Slider liveLevel;
    juce::Label liveLevelLabel,gainPolicyLabel;juce::ComboBox gainPolicyBox;
    juce::Slider tailSlider;
    juce::Label tailLabel;
    bool diagnosticsOpen=false;
    double diagnosticsRefreshedAt=-1;
    juce::TextButton timingButton{"Source and audible timing..."};
    bool advancedTiming=false;
};
