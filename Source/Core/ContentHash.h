#pragma once
#include <juce_core/juce_core.h>
#include <vector>

namespace m3
{
class ContentHash
{
public:
    ContentHash();
    ~ContentHash();
    ContentHash(const ContentHash&) = delete;
    ContentHash& operator=(const ContentHash&) = delete;
    void update(const void*, size_t);
    void samples(const float*, size_t); // Canonical finite float32, signed zero normalized.
    juce::String finish();
private:
    void* algorithm = nullptr;
    void* hash = nullptr;
    std::vector<unsigned char> scratch;
};
juce::String fileHash(const juce::File&);
juce::String sampleHash(const std::vector<float>&, double rate, int channels);
}
