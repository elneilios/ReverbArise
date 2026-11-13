#pragma once

#include "Controls/SoundPresetEditor.h"
#include <DSP/DefaultImpulse.h>
#include <DSP/PingPongResponse.h>
#include <map>

namespace m3::ui
{
// One bounded worker/cache for the presentation. Its keys distinguish an
// installed catalogue revision from the exact asset accepted by an instance.
class SoundResponsePeaks
{
    using Peaks = controls::SoundPresetEditor::Peaks;
public:
    static constexpr double managedRetryMs = 2000.;
    ~SoundResponsePeaks() { worker.removeAllJobs(true, -1); }
    uint64_t revision() const noexcept { return published.load(); }

    std::shared_ptr<const Peaks> bundled(const juce::String& id)
    {
        if (id == pingPongPresetId)
            return get("generated/" + id + "/" + juce::String(pingPongPresetVersion), [](ImpulseResponse& response)
            {
                SoundPresetIdentity identity;
                juce::String error;
                return preparePingPong({}, {}, decodeBudget, response, identity, error);
            });
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
        }, true);
    }

private:
    friend struct SoundResponsePeaksFixture;
    static constexpr size_t cacheLimit = 48, pendingLimit = 12, binLimit = 128;
    static constexpr uint64_t decodeBudget = 32ULL * 1024 * 1024;
    struct Entry
    {
        std::shared_ptr<const Peaks> peaks;
        uint64_t touch = 0;
        bool pending = true;
        double retryAt = 0.;
    };

    template<class Load>
    std::shared_ptr<const Peaks> get(const juce::String& key, Load load, bool retryFailure = false)
    {
        const juce::ScopedLock guard(lock);
        auto entry = cache.find(key);
        if (entry != cache.end())
        {
            entry->second.touch = ++sequence;
            if (entry->second.pending || entry->second.peaks || !retryFailure
                || juce::Time::getMillisecondCounterHiRes() < entry->second.retryAt)
                return entry->second.peaks;
        }
        if (pending >= pendingLimit) return {};
        if (entry == cache.end() && cache.size() == cacheLimit)
        {
            auto oldest = cache.end();
            for (auto candidate = cache.begin(); candidate != cache.end(); ++candidate)
                if (!candidate->second.pending && (oldest == cache.end() || candidate->second.touch < oldest->second.touch))
                    oldest = candidate;
            if (oldest == cache.end()) return {};
            cache.erase(oldest);
        }
        if (entry == cache.end()) cache.emplace(key, Entry{{}, ++sequence, true});
        else entry->second.pending = true;
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
            auto& result = cache.at(key);
            result.peaks = std::move(peaks);
            result.pending = false;
            result.retryAt = result.peaks ? 0. : juce::Time::getMillisecondCounterHiRes() + managedRetryMs;
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
