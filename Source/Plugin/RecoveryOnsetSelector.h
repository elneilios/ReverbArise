#pragma once

#include <Analysis/OnsetAnalysis.h>
#include <UI/Controls/ChoiceSelector.h>
#include <algorithm>
#include <optional>

class RecoveryOnsetSelector final : public m3::ui::controls::ChoiceSelector
{
public:
    RecoveryOnsetSelector()
    {
        setName("onsetSuggestions");
        setTitle("Onset suggestions");
        setTextWhenNothingSelected(emptyGuidance);
        setDescription(emptyGuidance);
        setEnabled(false);
    }

    std::optional<int64_t> selectedSample() const
    {
        const auto id = getSelectedId();
        if (id <= 0 || static_cast<size_t>(id) > samples.size())
            return std::nullopt;
        return samples[static_cast<size_t>(id - 1)];
    }

    void display(const std::vector<m3::Candidate>& candidates, double first, double last,
                 std::optional<int64_t> preferred)
    {
        auto next = visibleSamples(candidates, first, last, preferred);
        if (samples != next)
        {
            clear(juce::dontSendNotification);
            samples = std::move(next);
            for (size_t i = 0; i < samples.size(); ++i)
                addItem("Sample " + juce::String(samples[i]), static_cast<int>(i + 1));
            setDescription(samples.empty() ? emptyGuidance : "Choose an onset suggestion, then use Confirm to apply it.");
        }
        const auto selected = preferred ? std::find(samples.begin(), samples.end(), *preferred) : samples.end();
        const auto id = selected != samples.end() ? static_cast<int>(selected - samples.begin()) + 1
                                                  : samples.empty() ? 0 : 1;
        setSelectedId(id, juce::dontSendNotification);
        setEnabled(!samples.empty());
    }

    bool matches(const std::vector<m3::Candidate>& candidates, double first, double last) const
    {
        return samples == visibleSamples(candidates, first, last, selectedSample());
    }

private:
    static std::vector<int64_t> visibleSamples(const std::vector<m3::Candidate>& candidates,
                                                double first, double last,
                                                std::optional<int64_t> preferred)
    {
        constexpr size_t visibleLimit = 128;
        std::vector<int64_t> result;
        result.reserve(std::min(candidates.size(), visibleLimit + 1));
        for (const auto& candidate : candidates)
        {
            const auto sample = candidate.sample;
            const auto position = static_cast<double>(sample);
            if (preferred == sample || (result.size() < visibleLimit && position >= first && position <= last))
                result.push_back(sample);
        }
        return result;
    }

    static constexpr auto emptyGuidance = "Click a suggestion in the source lane";
    std::vector<int64_t> samples;
};
