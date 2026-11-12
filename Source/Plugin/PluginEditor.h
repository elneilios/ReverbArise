#pragma once
#include <UI/Controls/FieldTextEditor.h>
#include "PluginProcessor.h"
#include "RecoveryOnsetSelector.h"
#include <UI/SwellTimeline.h>
#include <UI/WorkspaceTheme.h>
#include <UI/WorkspaceLayout.h>
#include <UI/KnobValue.h>
#include <UI/Controls/EllipsisLabel.h>
#include <UI/Controls/WrappedLabel.h>
#include <UI/Controls/TransportReadout.h>
#include <UI/ConvolutionIrBinding.h>
#include <UI/WorkflowHeader.h>
#include <UI/WorkspaceNotice.h>
#include <UI/Controls/InvocationSlider.h>
#include <UI/LearnWorkspace.h>
#include <UI/ExportWorkspace.h>
#include <UI/Controls/SwellList.h>
#include <UI/Controls/ChoicePopup.h>
#include <UI/PerformLayout.h>
#include <UI/Controls/TimelineZoomControls.h>
#include <UI/Controls/LatchingActionButton.h>
#include <UI/Controls/ActionButton.h>
#include <UI/Controls/ActionButtonStateField.h>
#include <UI/Controls/CapturedActionButton.h>
#include <UI/Controls/PresentationSurface.h>
#include <UI/EditorPresentationState.h>
#include <UI/PortableStoragePanel.h>
#include <UI/SettingsTabs.h>
#include <UI/Controls/FocusViewport.h>

class PreverbAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer
{
public:
    explicit PreverbAudioProcessorEditor(PreverbAudioProcessor&,std::function<void(const juce::String&)> buildInformationWriter={});
    ~PreverbAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void refresh();
    bool assetRecoveryWarningVisible()const{return recoveryWarning && recoveryWarning->isVisible();}
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
    m3::ui::SettingsTabs* settingsPanel() noexcept {return &settingsTabs;}
    juce::Rectangle<int> swellMenuAnchor(const juce::MouseEvent& event)const;
    // The host factory supplies user-local storage; fixtures inject isolated storage.
    void enableResponsivePresentation(std::shared_ptr<m3::ui::PresentationStorage>,std::optional<m3::ui::PresentationFitContext> display={});
    // Native review defaults never write the user's real presentation profile.
    void enableResponsiveReview(std::shared_ptr<m3::ui::PresentationStorage> storage={},std::optional<m3::ui::PresentationFitContext> display={});
    bool responsiveReviewEnabled()const noexcept{return responsivePresentation;}
    bool setPresentationScale(int);
    bool setPresentationSize(int,int);
    void setPresentationDisplayForReview(m3::ui::PresentationFitContext);
    const m3::ui::EditorPresentationState* presentationState()const noexcept{return presentation.get();}
private:
    bool responsivePresentation=false;
    bool applyingPresentation=false,updatingPresentationControls=false;
    m3::ui::controls::PresentationSurface presentationSurface;
    std::shared_ptr<m3::ui::PresentationStorage> presentationStorage;
    std::unique_ptr<m3::ui::EditorPresentationState> presentation;
    std::optional<m3::ui::PresentationFitContext> reviewDisplay;
    std::optional<m3::ui::PresentationFitContext> appliedPresentationDisplay;
    juce::Point<int> appliedPresentationSize;
    m3::ui::controls::ChoiceSelector presentationSizeBox,presentationScaleBox;
    m3::ui::controls::EllipsisLabel presentationSizeLabel,presentationScaleLabel;
    m3::ui::controls::WrappedLabel presentationHelp;
    void initialisePresentation();
    void applyPresentation();
    void resizePresentation();
    void servicePresentation();
    void updatePresentationControls();
    void layoutPresentationSettings();
    void updateAuxiliaryPresentation();
    void paintWorkspace(juce::Graphics&);
    m3::ui::PresentationFitContext presentationDisplay()const;
    juce::Component& workspaceRoot()noexcept{return responsivePresentation?presentationSurface.contentRoot():*this;}
    juce::Rectangle<int> workspaceBounds()const noexcept{return responsivePresentation?presentationSurface.contentRoot().getLocalBounds():getLocalBounds();}
    int inspectorSection=1;
    m3::ui::controls::SegmentedSelector inspectorSections;
    void layoutResponsive();
    void layoutPerformPlayback(juce::Rectangle<int>);
    void openLoopPlacementEditor(PreverbAudioProcessor::LoopPlacementTarget);
    void openLoopPlacementMenu(bool keyboard);
    void refreshLoopPlayback(bool visible,bool liveEnabled,double capturedRate,const HostPositionDiagnostics&);
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
    m3::ui::controls::EllipsisLabel inspectorHeading,auditionHeading,editorReadout,listCount;
    m3::ui::controls::WrappedLabel listEmpty,inspectorEmpty;
    std::array<m3::ui::controls::EllipsisLabel,3> inspectorGroups;
    enum Field {name,arrival,sourceA,sourceB,audibleA,audibleB,fadeIn,fadeOut,fieldCount};
    void timerCallback() override {servicePresentation();modifierKeysChanged(hasKeyboardFocus(true)?juce::ModifierKeys::getCurrentModifiersRealtime():juce::ModifierKeys{});refresh();}
    void setDiagnosticsOpen(bool);
    void setSettingsOpen(bool);
    void initialiseSettings();
    void showRegenerationResult(const m3::RegenerationResult&);
    void layoutSettings();
    void refreshStorageScope(const CaptureSessionSnapshot&,uint64_t);
    void serviceAssetRecovery();
    void showAssetRecoveryWarning(const m3::storage::portable::Input&,bool metadataFailure=false);
    m3::storage::InventoryService recoveryInspection;
    juce::File recoveryInspectionRoot;
    juce::String recoveryInspectionReference;
    uint64_t recoveryInspectionEpoch=UINT64_MAX;
    bool recoveryInspected=false,recoveryWarned=false,metadataRecoveryWarned=false;
    std::unique_ptr<juce::AlertWindow> recoveryWarning;

    void showSwellMenu(m3::ui::controls::SwellList::MenuRequest);
    void requestDeleteSwells(std::vector<juce::String>,uint64_t epoch,uint64_t pass,
                             juce::String focusRow={},bool keyboard=false);
    void curveChanged(bool);
    void setPreferences();
    void applyBarRange();
    void validateBarRangeDraft();
    void refreshSourceRangeFeedback(const m3::Swell*);
    void layoutSourceRange();
    void armSelected();
    void refreshPerformNavigation();
    void publishPerformNavigation();
    void shapeReset(bool full);
    enum InspectorValue {targetValue,sourceStartValue,sourceEndValue,preRollValue,endOffsetValue,fadeInValue,fadeOutValue,levelValue,inCurveValue,outCurveValue,placementValue,valueCount};
    void beginValueEdit(int);
    void rebaseValueEdit(int);
    void stepValueEdit(int,double,bool fineWheel);
    m3::ui::KnobValue::ResetAction prepareValueReset(int);
    struct InitialValuePositions {int64_t arrival=0,start=0,end=1;};
    void resetValueEdit(int,InitialValuePositions);
    bool valueIntentCurrent(int)const;
    bool commitValueEdit(int,const juce::String&,std::optional<double> delta={});
    int layoutValueRow(juce::Rectangle<int>,int first,int second=-1,int gap=0);
    void layoutResponsiveValues();
    bool valueFeedbackChanged=false;
    uint64_t valueFeedbackGeneration=0;
    std::vector<juce::String> valueFeedbackSelection;
    void refreshValues(const m3::Swell*,const SongTime&);
    std::array<m3::ui::KnobValue,valueCount> valueControls;
    std::array<std::optional<m3::Swell>,valueCount> valueAnchors;
    std::array<double,valueCount> valueSpans{};
    std::array<uint64_t,valueCount> valueTokens{};
    std::array<uint64_t,valueCount> valueWorkflowTokens{};
    std::array<int64_t,valueCount> valueRawStarts{},valueRawEnds{};
    struct ValueInvocation {uint64_t epoch=0;std::vector<juce::String> selected;SongTime time,accepted;double rate=0;bool sampleAvailable=false;juce::String render;m3::GridClockContext clock;uint64_t loopRevision=0;};
    std::array<ValueInvocation,valueCount> valueInvocations;
    int valueModifierFlags=0;
    std::map<juce::String,InitialValuePositions> initialValuePositions;
    uint64_t initialValueEpoch=0,valueResetRevision=0;
    m3::ui::controls::ConvolutionIrEditor irEditor;
    m3::ui::ConvolutionIrBinding irBinding;
    m3::ui::controls::ActionButton resetMenu{"v",m3::ui::controls::CommandIcon::none,m3::ui::controls::CommandVariant::destructive};
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
    std::unique_ptr<m3::ui::controls::ChoicePopup> swellMenu;
    juce::Component::SafePointer<juce::AlertWindow> renameDialog;
    std::unique_ptr<juce::AlertWindow> deleteSwellConfirmation;
    m3::ui::controls::WrappedLabel status,detail,recaptureStatus;
    m3::ui::controls::EllipsisLabel listLabel,auditionLabel,separationLabel,spanLabel,barStartLabel,barEndLabel,toneStartLabel,toneEndLabel;
    m3::ui::controls::CapturedActionButton removeButton{"Delete"};
    m3::ui::controls::ActionButton addButton{"Add at playhead",m3::ui::controls::CommandIcon::add},generateButton{"Regenerate selected"},allButton{"Regenerate all"};
    m3::ui::controls::ActionButton analyzeButton{"Suggest onsets"},confirmButton{"Confirm"};
    m3::ui::controls::TimelineZoomControls performZoom;
    m3::ui::controls::LatchingActionButton snapButton{"Snap"};
    m3::ui::controls::ChoiceSelector gridBox;
    struct NavigationScope {uint64_t epoch=0,pass=0;juce::String primary;};
    std::optional<NavigationScope> navigationInvocation;
    bool navigationIntentCurrent()const;
    bool performFitted=false;
    bool navigationDirty=false;
    uint64_t navigationEpoch=0;
    m3::ui::controls::ActionButton recaptureButton{"Recapture & regenerate"},cancelButton{"Cancel recapture"},fullButton{"Full response"},resetButton{"Reset shape",m3::ui::controls::CommandIcon::none,m3::ui::controls::CommandVariant::destructive},setBarsButton{"Set source from bars"},clearButton{"Clear map"};
    m3::ui::controls::ActionButton assetInfoButton{"Copy asset info"};
    m3::ui::controls::ActionButton copyDiagnosticsButton{"Copy diagnostics"};
    m3::ui::controls::BooleanToggle enabledButton{"Enabled",m3::ui::controls::ToggleForm::checkbox};
    m3::ui::controls::BooleanToggle soloButton{"Solo",m3::ui::controls::ToggleForm::checkbox};
    m3::ui::controls::Switch muteButton{"Mute"},mappingButton{"Map while playing"},toneButton{"440 Hz test tone"};
    m3::ui::controls::Switch autoButton{"Auto-refresh after recapture"};
    m3::ui::controls::SliderControl auditionSlider{{"Audition trim (dB)", "Adjust the wet audition level.", false}};
    m3::ui::controls::SliderControl toneStartSlider{{"Test tone start bar", "First bar of the test tone.", false}};
    m3::ui::controls::SliderControl toneEndSlider{{"Test tone end bar (exclusive)", "The tone ends before this bar.", false}};
    m3::ui::controls::SliderControl separationSlider{{"Separation (ms)", "Minimum time between suggested events.", false}};
    std::optional<uint64_t> markerPass,separationPass,automaticPass,markingPass,refreshPass;
    struct FlagScope { uint64_t epoch,pass;juce::String primary; };
    std::optional<FlagScope> enabledScope,soloScope,freezeScope,muteScope;
    RecoveryOnsetSelector candidateBox;
    m3::ui::controls::ChoiceSelector inCurveBox,outCurveBox;
    std::array<m3::ui::controls::EllipsisLabel,fieldCount> fieldLabels;
    std::array<m3::ui::controls::FieldTextEditor,fieldCount> fields;
    m3::ui::controls::FieldTextEditor barStartEditor{"Bar start"},barEndEditor{"End excl."};
    m3::ui::controls::FieldFeedbackLabel sourceRangeFeedback;
    std::optional<PreverbAudioProcessor::SourceBarTarget> sourceRangeDraftTarget;
    PreverbAudioProcessor::SourceBarSubmission sourceRangeResult;
    juce::String sourceRangeMessage;
    bool sourceRangeHasError=false;
    uint64_t presentationSession=0;
    std::pair<juce::String, uint64_t> onsetSource;
    bool updating=false,destroying=false;
    m3::ui::controls::FocusViewport inspectorViewport,diagnosticsViewport;
    juce::Component inspectorContent;
    m3::ui::FlatSurface diagnosticsContent;
    void layoutLiveDiagnostics();
    m3::ui::controls::WrappedLabel captureDiagnostic,playbackDiagnostic,policyDiagnostic,liveDiagnostic;
    std::array<juce::Component,7> settingsPages;
    std::unique_ptr<m3::ui::PortableStoragePanel> storageInventory;
    m3::ui::controls::FocusViewport storageViewport;
    juce::String storageIdentity;
    m3::ui::SettingsTabs settingsTabs;
    std::array<m3::ui::controls::EllipsisLabel,12> settingsLabels;
    std::array<m3::ui::controls::WrappedLabel,6> settingsHelp;
    m3::ui::controls::EllipsisLabel regenerationFeedback;
    uint64_t regenerationFeedbackSession=0;
    m3::ui::controls::EllipsisLabel aboutHeading;
    std::array<m3::ui::controls::EllipsisLabel,3> buildInformationLabels;
    std::array<m3::ui::controls::EllipsisLabel,3> buildInformationValues;
    m3::ui::controls::ActionButton copyBuildInformation{"Copy build information",m3::ui::controls::CommandIcon::none,true};
    std::function<void(const juce::String&)> buildInformationWriter;
    m3::ui::controls::ActionButton openDiagnosticsButton{"Open diagnostics..."};
    std::unique_ptr<juce::DocumentWindow> settingsWindow;
    std::unique_ptr<juce::DocumentWindow> diagnosticsWindow;
    juce::Label headerStatus,helper;
    m3::ui::controls::EllipsisLabel loopHeading;
    m3::ui::controls::ActionButtonStateField audition{"Audition",m3::ui::controls::CommandIcon::play,
        m3::ui::controls::CommandVariant::primary,"Audition status",40,120};
    m3::ui::controls::ActionButtonStateField loopStart{"Set Loop Start",m3::ui::controls::CommandIcon::none,
        m3::ui::controls::CommandVariant::primary,"Loop start",130};
    m3::ui::controls::ActionButtonStateField loopCue{"Cue Loop",m3::ui::controls::CommandIcon::none,
        m3::ui::controls::CommandVariant::neutral,"Loop status",112};
    int loopDividerX = 0;
    juce::Component::SafePointer<juce::AlertWindow> loopPlacementDialog;
    std::unique_ptr<m3::ui::controls::ChoicePopup> loopPlacementMenu;
    std::optional<PreverbAudioProcessor::LoopPlacementTarget> loopPlacementInvocation;
    struct AuditionCommand {uint64_t session,pass;bool stopping;};
    std::optional<AuditionCommand> auditionCommand;
    bool loopPerformLayout=false;
    bool loopButtonStops=false;
    std::optional<std::pair<uint64_t,bool>> loopButtonInvocation;
    std::optional<std::pair<int64_t,m3::SwellEngine::TargetIdentity>> addTargetInvocation;
    m3::ui::controls::EllipsisLabel selectedStatus,performSoundScope;
    juce::ToggleButton recaptureExisting{"Recapture existing audio"};
    m3::ui::controls::LatchingActionButton auditionWetOnly{"Wet only"};
    std::optional<std::pair<uint64_t,uint64_t>> auditionInvocation;
    m3::ui::controls::ActionButton defaultIRButton{"Default IR for future..."},suppressionButton{"Reset deleted targets"};
    m3::ui::controls::Switch automaticButton{"Automatic processing"};juce::ToggleButton sampleUnitsButton{"Exact sample positions"};
    juce::Slider regenerationKnob;juce::Label regenerationLabel;
    m3::ui::controls::BooleanToggle freezeButton{"Freeze",m3::ui::controls::ToggleForm::checkbox};
    m3::ui::controls::InvocationSlider markerKnob;m3::ui::controls::InvocationToggle markerButton{"Auto markers"};m3::ui::controls::Switch isolateButton{"Isolate selected"},isolateDryButton{"Isolate dry source"};
    juce::Label markerLabel,markerState,placementLabel;
    m3::ui::controls::FieldTextEditor placementEditor{"Move wet (ms)", m3::ui::controls::FieldTextEditor::Role::readOnly};
    m3::ui::controls::Switch liveButton{"Live playback"};m3::ui::controls::ChoiceSelector liveRoutingBox;
    m3::ui::controls::SliderControl liveLevel{{"Live level (dB)", "Adjust the live playback level.", false}};
    juce::Label liveLevelLabel,gainPolicyLabel;m3::ui::controls::ChoiceSelector gainPolicyBox;
    m3::ui::controls::SliderControl tailSlider{{"Audition tail (s)", "Captured Dry + Wet context after the later of the original arrival and wet endpoint.", false}};
    juce::Label tailLabel;
    bool diagnosticsOpen=false;
    double diagnosticsRefreshedAt=-1;
    double auditionRetryAfter=0;
    m3::ui::controls::ActionButton timingButton{"Source and audible timing..."};
    bool advancedTiming=false;
};
