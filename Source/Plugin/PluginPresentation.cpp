#include "PluginEditor.h"

namespace
{
constexpr std::array<juce::Point<int>,4> presentationPresets{{{900,600},{1100,850},{1400,900},{1600,1000}}};
}

m3::ui::PresentationFitContext PreverbAudioProcessorEditor::presentationDisplay()const
{
    if(reviewDisplay)return *reviewDisplay;
    const auto* display=juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());
    if(!display)return {};
    const auto work=display->userBounds.toNearestInt();
    m3::ui::PresentationExtent chrome;
    const auto origin=localPointToGlobal(juce::Point<float>{});
    const auto scaleX=std::max(.001f,origin.getDistanceFrom(localPointToGlobal(juce::Point<float>{1,0})));
    const auto scaleY=std::max(.001f,origin.getDistanceFrom(localPointToGlobal(juce::Point<float>{0,1})));
    if(const auto* top=getTopLevelComponent())
    {
        // Measure the JUCE embedding/review bar and native peer frame separately
        // from plugin content. External host-only chrome needs final host coverage.
        chrome.width=std::max(0,top->getScreenBounds().getWidth()-getScreenBounds().getWidth());
        chrome.height=std::max(0,top->getScreenBounds().getHeight()-getScreenBounds().getHeight());
        if(const auto* peer=top->getPeer())if(const auto frame=peer->getFrameSizeIfPresent())
        {chrome.width+=frame->getLeftAndRight();chrome.height+=frame->getTopAndBottom();}
    }
    // Convert desktop coordinates to this editor's external host units once.
    // User magnification lives below the editor and is absent from these ratios.
    return {{static_cast<int>(std::floor(work.getWidth()/scaleX)),static_cast<int>(std::floor(work.getHeight()/scaleY))},
        {static_cast<int>(std::ceil(chrome.width/scaleX)),static_cast<int>(std::ceil(chrome.height/scaleY))}};
}
void PreverbAudioProcessorEditor::initialisePresentation()
{
    using namespace m3::ui;
    auto& page=settingsPages[6];page.setName("settingsAppearance");settingsTabs.addPage(SettingsPage::appearance,page);
    for(auto* child:std::initializer_list<juce::Component*>{&presentationSizeLabel,&presentationScaleLabel,&presentationSizeBox,&presentationScaleBox,&presentationHelp})page.addAndMakeVisible(child);
    presentationSizeLabel.setText("Window size",juce::dontSendNotification);presentationScaleLabel.setText("UI scale",juce::dontSendNotification);
    applyTextRole(presentationSizeLabel,TextRole::controlLabel);applyTextRole(presentationScaleLabel,TextRole::controlLabel);
    applyTextRole(presentationHelp,TextRole::helper,juce::Justification::topLeft);
    presentationSizeBox.setName("presentationSize");presentationScaleBox.setName("presentationScale");presentationHelp.setName("presentationHelp");
    for(size_t i=0;i<presentationPresets.size();++i)
    {const auto p=presentationPresets[i];presentationSizeBox.addItem(juce::String(p.x)+" x "+juce::String(p.y),static_cast<int>(i+1));}
    presentationScaleBox.addItem("100%",100);presentationScaleBox.addItem("125%",125);
    presentationSizeBox.setTooltip("Add useful workspace, or freely resize the plugin window. Waveform zoom stays independent.");
    presentationScaleBox.setTooltip("Magnify controls and text. Choices that do not fit this display are unavailable.");
    presentationSizeBox.onChange=[this]
    {
        if(updatingPresentationControls)return;
        const auto index=presentationSizeBox.getSelectedId()-1;
        if(index>=0 && index<static_cast<int>(presentationPresets.size()))
        {const auto p=presentationPresets[static_cast<size_t>(index)];setPresentationSize(p.x,p.y);}
    };
    presentationScaleBox.onChange=[this]{if(!updatingPresentationControls)setPresentationScale(presentationScaleBox.getSelectedId());};
    layoutPresentationSettings();
}
void PreverbAudioProcessorEditor::layoutPresentationSettings()
{
    using namespace m3::ui;
    auto area=juce::Rectangle<int>(spacing::section,spacing::section,590,240);
    auto row=area.removeFromTop(controlGeometry::height);presentationSizeLabel.setBounds(row.removeFromLeft(175));presentationSizeBox.setRowBounds(row);
    area.removeFromTop(spacing::related);row=area.removeFromTop(controlGeometry::height);
    presentationScaleLabel.setBounds(row.removeFromLeft(175));presentationScaleBox.setRowBounds(row);
    area.removeFromTop(spacing::section);presentationHelp.setBounds(area);
}
void PreverbAudioProcessorEditor::updatePresentationControls()
{
    if(!presentation)return;
    const juce::ScopedValueSetter<bool> guard(updatingPresentationControls,true);
    const auto current=presentation->effective();int selected=0;
    for(size_t i=0;i<presentationPresets.size();++i)
    {
        const auto p=presentationPresets[i];presentationSizeBox.setItemEnabled(static_cast<int>(i+1),presentation->canSelectSize(p.x,p.y));
        if(current.effective.width==p.x && current.effective.height==p.y)selected=static_cast<int>(i+1);
    }
    presentationSizeBox.setSelectedId(selected,juce::dontSendNotification);
    if(!selected)presentationSizeBox.setText(juce::String(current.effective.width)+" x "+juce::String(current.effective.height)+" (custom)",juce::dontSendNotification);
    for(int scale:{100,125})presentationScaleBox.setItemEnabled(scale,presentation->canSelectScale(scale));
    presentationScaleBox.setSelectedId(current.effective.scalePercent,juce::dontSendNotification);
    juce::String help="Window size adds workspace. UI scale magnifies controls and text. Waveform zoom is separate.";
    if(!current.supported)help="This display cannot fit the 900 x 600 minimum. Move the plugin to a larger work area or adjust host/display scaling. Settings remains available.";
    else if(current.recovered)help="Temporarily fitted to this display. Your preferred size and scale are retained for a larger work area.";
    if(presentation->lastSave().failed())help+="\nSize preferences could not be saved; this window remains usable. Saving will retry.";
    presentationHelp.setText(help,juce::dontSendNotification);presentationHelp.setTooltip(help);presentationHelp.setDescription(help);
}
void PreverbAudioProcessorEditor::applyPresentation()
{
    if(!presentation)return;
    const juce::ScopedValueSetter<bool> guard(applyingPresentation,true);
    const auto effective=presentation->effective().effective;
    const auto display=presentationDisplay();appliedPresentationDisplay=display;
    const auto extent=m3::ui::presentationExtent(effective),available=display.available();
    const auto minimum=m3::ui::presentationExtent({900,600,effective.scalePercent});
    const auto maximum=m3::ui::presentationExtent({2000,1400,effective.scalePercent});
    setResizeLimits(minimum.width,minimum.height,std::max(minimum.width,std::min(maximum.width,available.width)),std::max(minimum.height,std::min(maximum.height,available.height)));
    appliedPresentationSize={extent.width,extent.height};setSize(extent.width,extent.height);
    presentationSurface.setBounds(getLocalBounds());presentationSurface.present(effective);
    layoutResponsive();updatePresentationControls();updateAuxiliaryPresentation();repaint();
}
void PreverbAudioProcessorEditor::resizePresentation()
{
    if(!presentation || applyingPresentation)return;
    if(juce::Point<int>(getWidth(),getHeight())!=appliedPresentationSize)
        presentation->manualResize({getWidth(),getHeight()},juce::Time::getMillisecondCounterHiRes());
    applyPresentation();
}
void PreverbAudioProcessorEditor::servicePresentation()
{
    if(!presentation || destroying)return;
    const bool restored=presentation->retryInitialRead();
    const auto display=presentationDisplay();
    const bool displayChanged=presentation->displayChanged(display);
    const bool constraintsChanged=!appliedPresentationDisplay || appliedPresentationDisplay->workArea!=display.workArea || appliedPresentationDisplay->chrome!=display.chrome;
    if(restored || displayChanged || constraintsChanged)applyPresentation();
    presentation->persistIfDue(juce::Time::getMillisecondCounterHiRes());updatePresentationControls();updateAuxiliaryPresentation();
}
bool PreverbAudioProcessorEditor::setPresentationScale(int percent)
{
    if(!presentation || !presentation->selectScale(percent,juce::Time::getMillisecondCounterHiRes()))
    {updatePresentationControls();return false;}
    applyPresentation();return true;
}
bool PreverbAudioProcessorEditor::setPresentationSize(int width,int height)
{
    if(!presentation || !presentation->selectSize(width,height,juce::Time::getMillisecondCounterHiRes()))
    {updatePresentationControls();return false;}
    applyPresentation();return true;
}
void PreverbAudioProcessorEditor::setPresentationDisplayForReview(m3::ui::PresentationFitContext context)
{
    reviewDisplay=context;if(presentation){presentation->displayChanged(context);applyPresentation();}
}
