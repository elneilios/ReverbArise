#include "AudioTestFiles.h"
#include <random>
#include <numbers>
int main(int argc, char** argv) { return runTest([&]
{
    const auto root = argc > 1 ? juce::File::getCurrentWorkingDirectory().getChildFile(juce::String::fromUTF8(argv[1]))
                              : juce::File::getCurrentWorkingDirectory().getChildFile("out/test-media/m2");
    REQUIRE(root.createDirectory().wasOk());
    std::mt19937 random(1234);
    std::uniform_real_distribution<float> noise(-1, 1);
    std::vector<float> mono(24000), stereo(48000), source(192000, 0);
    for (size_t frame = 0; frame < mono.size(); ++frame)
    {
        const auto envelope = static_cast<float>(std::exp(-6.0 * frame / mono.size()));
        mono[frame] = 0.01f * noise(random) * envelope;
        stereo[frame * 2] = mono[frame];
        stereo[frame * 2 + 1] = 0.01f * noise(random) * envelope;
    }
    mono[0] += 0.1f; stereo[0] += 0.1f; stereo[1] += 0.1f;
    for (size_t frame = 0; frame < 48000; ++frame)
    {
        const auto envelope = std::min(1.0, frame / 480.0) * std::exp(-6.0 * frame / 48000);
        const auto sample = static_cast<float>(0.2 * envelope * std::sin(2 * std::numbers::pi * 220 * frame / 48000));
        source[frame * 2] = source[frame * 2 + 1] = sample;
    }
    for (const auto& name : {"m2-ir-mono.wav", "m2-ir-stereo.wav", "m2-source.wav"})
        REQUIRE(! root.getChildFile(name).exists());
    writeTestWav(root.getChildFile("m2-ir-mono.wav"), 1, 48000, mono);
    writeTestWav(root.getChildFile("m2-ir-stereo.wav"), 2, 48000, stereo);
    writeTestWav(root.getChildFile("m2-source.wav"), 2, 48000, source);
    std::cout << root.getFullPathName() << '\n';
}); }
