#pragma once
#include <UI/Controls/WrappedLabel.h>
#include <UI/SettingsTabs.h>

inline void textPresentationCases()
{
    using namespace m3::ui;
    using namespace m3::ui::controls;
    WorkspaceTheme theme;
    WrappedLabel prose;
    prose.setLookAndFeel(&theme);
    applyTextRole(prose,TextRole::helper,juce::Justification::topLeft);
    const auto value=juce::String::repeatedString("A_very_long_unbroken_asset_filename_",12);
    prose.setText(value,juce::dontSendNotification);
    const auto font=prose.getFont();
    for(const auto width:{80,240,590})
    {
        const auto height=prose.preferredHeight(width);
        prose.setSize(width,height);
        const auto layout=prose.textLayout(width);
        REQUIRE(layout.getHeight()<=height);
        REQUIRE(layout.getNumLines()>1);
        for(int i=0;i<layout.getNumLines();++i)
            REQUIRE(layout.getLine(i).getLineBoundsX().getEnd()<=width+1);
        REQUIRE(prose.createComponentSnapshot(prose.getLocalBounds()).isValid());
        REQUIRE(prose.getFont()==font && prose.getDescription()==value && prose.getTooltip()==value);
    }
    prose.setText(value,juce::dontSendNotification);
    REQUIRE(prose.getDescription()==value);
    prose.setText({},juce::dontSendNotification);
    REQUIRE(prose.preferredHeight(240)==0 && prose.getDescription().isEmpty() && prose.getTooltip().isEmpty());
    EllipsisLabel label;
    label.setText("Old value",juce::dontSendNotification);
    label.setText("New value",juce::dontSendNotification);
    REQUIRE(label.getDescription()=="New value" && label.getTooltip()=="New value");
    label.setDescription("Binding-owned explanation");label.setTooltip("Binding-owned context");
    label.setText("Accepted value",juce::dontSendNotification);
    REQUIRE(label.getDescription()=="Binding-owned explanation" && label.getTooltip()=="Binding-owned context");
    SettingsTabs tabs;
    juce::Component first,second;
    tabs.setSize(640,400);
    tabs.addPage(SettingsPage::playback,first);
    tabs.addPage(SettingsPage::processing,second);
    tabs.setPageContentHeight(SettingsPage::playback,900);
    auto* viewport=first.findParentComponentOfClass<FocusViewport>();
    REQUIRE(viewport && viewport->getViewedComponent()==&first);
    REQUIRE(!viewport->getWantsKeyboardFocus());
    REQUIRE(first.getHeight()==900 && viewport->getViewedComponent()->getHeight()>viewport->getHeight());
    viewport->setViewPosition(0,500);
    REQUIRE(viewport->getViewPositionY()==500);
    // Reflowing an inactive page must leave the current reading position alone.
    tabs.setPageContentHeight(SettingsPage::processing,800);
    REQUIRE(viewport->getViewPositionY()==500);
    tabs.setPageContentHeight(SettingsPage::playback,1000);
    REQUIRE(viewport->getViewPositionY()==500);
    tabs.setSize(600,360);
    REQUIRE(viewport->getViewPositionY()==500);
    tabs.setPageContentHeight(SettingsPage::playback,450);
    REQUIRE(viewport->getViewPositionY()==first.getHeight()-viewport->getViewHeight());
    REQUIRE(viewport->getViewPositionY()>0);
    tabs.setPageContentHeight(SettingsPage::playback,0);
    REQUIRE(viewport->getViewPositionY()==0);
    tabs.selectPage(SettingsPage::processing);
    REQUIRE(viewport->getViewedComponent()==&second && !first.isVisible());
    prose.setLookAndFeel(nullptr);
}
