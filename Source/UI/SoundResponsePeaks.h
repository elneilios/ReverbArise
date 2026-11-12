#pragma once

#include "Controls/SoundPresetEditor.h"
#include <DSP/DefaultImpulse.h>
#include <map>

namespace m3::ui
{
// One bounded worker/cache for the presentation. Its keys distinguish an
// installed catalogue revision from the exact asset accepted by an instance.
class SoundResponsePeaks
{
    using Peaks = controls::SoundPresetEditor::Peaks;
public:
    ~SoundResponsePeaks() { worker.removeAllJobs(true, -1); }
    uint64_t revision() const noexcept { return published.load(); }

    std::shared_ptr<const Peaks> bundled(const juce::String& id)
    {
        const auto* impulse = findBundledImpulse(id);
        if (!impulse) return {};
        return get("bundled/" + juce::String(impulse->decodedHash), [impulse](ImpulseResponse& response)
        {
            juce::String error;
            return decodeBundledImpulse(*impulse, decodeBudget, response, error);
        });
    }

    std::shared_ptr<const Peaks> managed(const juce::File& root, const Asset& asset)
    {
        if (!asset.present()) return {};
        return get(root.getFullPathName() + "/" + asset.path + "/" + asset.checksum, [root, asset](ImpulseResponse& response)
        {
            juce::String error;
            return SwellAssetStore(root).readIR(asset, decodeBudget, response, error);
        });
    }

private:
    friend struct SoundResponsePeaksFixture;
    static constexpr size_t cacheLimit = 48, pendingLimit = 12, binLimit = 128;
    static constexpr uint64_t decodeBudget = 32ULL * 1024 * 1024;
    struct Entry { std::shared_ptr<const Peaks> peaks; uint64_t touch = 0; bool pending = true; };

    template<class Load>
    std::shared_ptr<const Peaks> get(const juce::String& key, Load load)
    {
        const juce::ScopedLock guard(lock);
        if (auto entry = cache.find(key); entry != cache.end())
        {
            entry->second.touch = ++sequence;
            return entry->second.peaks;
        }
        if (pending >= pendingLimit) return {};
        if (cache.size() == cacheLimit)
        {
            auto oldest = cache.end();
            for (auto entry = cache.begin(); entry != cache.end(); ++entry)
                if (!entry->second.pending && (oldest == cache.end() || entry->second.touch < oldest->second.touch))
                    oldest = entry;
            if (oldest == cache.end()) return {};
            cache.erase(oldest);
        }
        cache.emplace(key, Entry{{}, ++sequence, true});
        ++pending;
        worker.addJob(std::function<void()>([this, key, load = std::move(load)]
        {
            std::shared_ptr<const Peaks> peaks;
            try
            {
                ImpulseResponse response;
                if (load(response) && response.frames() > 0 && (response.channels == 1 || response.channels == 2))
                {
                    const auto frames = response.frames();
                    const auto bins = std::min(frames, binLimit);
                    const auto channels = static_cast<size_t>(response.channels);
                    auto summary = std::make_shared<Peaks>(channels, std::vector<float>(bins));
                    for (size_t frame = 0; frame < frames; ++frame)
                        for (size_t channel = 0; channel < channels; ++channel)
                        {
                            const auto sample = std::abs(response.samples[frame * channels + channel]);
                            auto& peak = (*summary)[channel][frame * bins / frames];
                            if (std::isfinite(sample)) peak = std::max(peak, sample);
                        }
                    peaks = std::move(summary);
                }
            }
            catch (const std::bad_alloc&) {}
            const juce::ScopedLock completed(lock);
            auto& entry = cache.at(key);
            entry.peaks = std::move(peaks);
            entry.pending = false;
            --pending;
            ++published;
        }));
        return {};
    }

    juce::CriticalSection lock;
    std::map<juce::String, Entry> cache;
    size_t pending = 0;
    uint64_t sequence = 0;
    std::atomic<uint64_t> published{0};
    juce::ThreadPool worker{juce::ThreadPoolOptions{}.withNumberOfThreads(1).withDesiredThreadPriority(juce::Thread::Priority::low)};
};
}
