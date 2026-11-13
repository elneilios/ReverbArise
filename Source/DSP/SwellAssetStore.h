#pragma once
#include <Core/SwellSession.h>
#include <DSP/ReverseConvolution.h>

namespace m3
{
class SwellAssetStore
{
public:
    explicit SwellAssetStore(juce::File root) : root(std::move(root)) {}
    bool importIR(const juce::File&, uint64_t budget, Asset&, juce::String& error);
    bool writeIR(const ImpulseResponse&, Asset&, juce::String& error);
    bool reuseOrWriteIR(const ImpulseResponse&, const juce::String& content, uint64_t budget,
                        Asset&, juce::String& error);
    bool writeRender(const WetRender&, const Asset& ir, const juce::String& dependency,
                     const SongTime&, Asset&, juce::String& error);
    bool readIR(const Asset&, uint64_t budget, ImpulseResponse&, juce::String& error) const;
    bool readRender(const Asset&, uint64_t budget, WetRender&, juce::String& error) const;
    bool verify(const Asset&, juce::String& error) const;
    const juce::File& directory() const noexcept { return root; }
private:
    bool write(const std::vector<float>&, Asset&, juce::String& error);
    bool resolve(const Asset&, juce::File&, juce::String& error) const;
    juce::File root;
};
}
