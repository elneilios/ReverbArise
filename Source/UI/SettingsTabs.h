#pragma once
#include "Controls/SegmentedSelector.h"

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

    void paint(juce::Graphics& g) override { g.fillAll(background); }
    void resized() override
    {
        auto area = getLocalBounds();
        strip.setBounds(area.removeFromTop(tabDepth));
        // Inactive pages stay detached. Showing Storage still starts its scan.
        for (auto* content : pages)
            if (content != nullptr)
                content->setBounds(area);
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
            removeChildComponent(active);
        }
        active = &content;
        addChildComponent(content);
        resized();
        content.setVisible(true);
    }

    static constexpr int tabDepth = 36;
    static constexpr std::array titles{"Playback", "Processing", "Swell", "Audition", "Recovery",
        "Appearance", "Storage", "About"};
    std::array<juce::Component*, titles.size()> pages{};
    controls::SegmentedSelector strip;
    SettingsPage selected = SettingsPage::playback;
    juce::Component* active = nullptr;
};
}
