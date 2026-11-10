#pragma once
#include "Controls/SegmentedSelector.h"
#include "Controls/FocusViewport.h"

namespace m3::ui
{
// Display order is independent of the editor's storage for the page components.
enum class SettingsPage { playback, processing, swell, audition, recovery, appearance, storage, about };

class SettingsTabs final : public juce::Component
{
public:
    SettingsTabs()
    {
        setName("settingsTabs");
        strip.setTitle("Settings pages");
        strip.setTextHeight(bodySize);
        strip.onRequest = [this](int id) { selectPage(static_cast<SettingsPage>(id)); };
        addAndMakeVisible(strip);
        addAndMakeVisible(viewport);
        viewport.setFocusRevealEnabled(true);
        viewport.setWantsKeyboardFocus(false);
    }

    void addPage(SettingsPage id, juce::Component& content)
    {
        pages[static_cast<size_t>(id)] = &content;
        updateStrip();
        if (id == selected)
            showPage(content);
    }

    void selectPage(SettingsPage id)
    {
        auto* content = page(id);
        if (content == nullptr || active == content)
            return;
        selected = id;
        updateStrip();
        showPage(*content);
    }

    juce::Component* page(SettingsPage id) const noexcept { return pages[static_cast<size_t>(id)]; }
    SettingsPage selectedPage() const noexcept { return selected; }
    const auto& pageComponents() const noexcept { return pages; }
    controls::SegmentedSelector& selector() noexcept { return strip; }
    void setPageContentHeight(SettingsPage id,int height)
    {
        heights[static_cast<size_t>(id)]=std::max(0,height);
        resized();
    }

    void paint(juce::Graphics& g) override { g.fillAll(background); }
    void resized() override
    {
        auto area = getLocalBounds();
        strip.setBounds(area.removeFromTop(tabDepth));
        viewport.setBounds(area);
        // Inactive pages stay detached. Showing Storage still starts its scan.
        for (size_t i=0;i<pages.size();++i)
            if (auto* content=pages[i])
            {
                const auto height=std::max(area.getHeight(),heights[i]);
                const auto width=area.getWidth()-(height>area.getHeight()?viewport.getScrollBarThickness():0);
                content->setBounds(0,0,width,height);
            }
    }

private:
    void updateStrip()
    {
        std::vector<controls::SegmentChoice> choices;
        for (size_t i = 0; i < pages.size(); ++i)
            if (pages[i] != nullptr)
                choices.push_back({static_cast<int>(i), titles[i], "Open " + juce::String(titles[i]) + " settings"});
        strip.display(std::move(choices), static_cast<int>(selected));
    }

    void showPage(juce::Component& content)
    {
        if (active != nullptr)
        {
            active->setVisible(false);
            viewport.setViewedComponent(nullptr,false);
        }
        active = &content;
        viewport.setViewedComponent(&content,false);
        resized();
        content.setVisible(true);
    }

    static constexpr int tabDepth = 36;
    static constexpr std::array titles{"Playback", "Processing", "Swell", "Audition", "Recovery",
        "Appearance", "Storage", "About"};
    std::array<juce::Component*, titles.size()> pages{};
    std::array<int,titles.size()> heights{};
    controls::FocusViewport viewport;
    controls::SegmentedSelector strip;
    SettingsPage selected = SettingsPage::playback;
    juce::Component* active = nullptr;
};
}
