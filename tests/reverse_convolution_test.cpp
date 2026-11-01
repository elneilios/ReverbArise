#include "AudioTestFiles.h"
#include <DSP/ReverseConvolution.h>
#include <random>
#include <numbers>

static void compare(const std::vector<float>& x, const ImpulseResponse& ir, int64_t origin)
{
    WetRender output;
    juce::String error;
    REQUIRE(renderReverseConvolution(x, origin, ir.rate, ir, 0, output, error));
    const auto n = x.size() / 2, m = ir.frames();
    REQUIRE(output.frames() == n + m - 1);
    REQUIRE(output.start == origin - static_cast<int64_t>(m - 1));
    REQUIRE(output.sourceEnd == origin + static_cast<int64_t>(n));
    for (size_t frame = 0; frame < output.frames(); ++frame)
        for (int channel = 0; channel < 2; ++channel)
        {
            double reference = 0, l1 = 0;
            const auto begin = frame >= m ? frame - m + 1 : 0;
            for (size_t k = begin; k < n && k <= frame; ++k)
            {
                const auto product = static_cast<double>(x[k * 2 + channel])
                    * ir.samples[(m - 1 - (frame - k)) * ir.channels + (ir.channels == 1 ? 0 : channel)];
                reference += product; l1 += std::abs(product);
            }
            REQUIRE(std::abs(output.samples[frame * 2 + channel] - reference) <= 2.0e-5 * std::max(1.0, l1));
        }
    // Independent reverse-source -> ordinary convolution -> reverse-output identity.
    for (size_t frame = 0; frame < output.frames(); ++frame)
    {
        const auto reversedFrame = output.frames() - 1 - frame;
        double reference = 0;
        for (size_t k = 0; k < n; ++k)
            if (reversedFrame >= k && reversedFrame - k < m)
                reference += static_cast<double>(x[(n - 1 - k) * 2]) * ir.samples[(reversedFrame - k) * ir.channels];
        REQUIRE(std::abs(output.samples[frame * 2] - reference) < 1.0e-4);
    }
}

int main() { return runTest([]
{
    juce::String error;
    ImpulseResponse mono{48000, 1, {0, 1, 0.5f, -0.25f}, "synthetic", "mono"};
    compare({1,0, 0,0, 0,1}, mono, 0);
    compare({2,-3}, ImpulseResponse{48000, 1, {0.5f}, {}, {}}, -10);
    compare(std::vector<float>(14, 0), mono, 20);
    ImpulseResponse stereo{48000, 2, {1,0, 0,1, -0.5f,0.25f}, {}, {}};
    compare({1,0,2,0,0,0,0,0}, stereo, 100);
    std::mt19937 generator(42);
    std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);
    std::vector<float> randomSource(74);
    ImpulseResponse randomIr{48000, 2, std::vector<float>(38), {}, {}};
    for (auto& v : randomSource) v = distribution(generator);
    for (auto& v : randomIr.samples) v = distribution(generator);
    compare(randomSource, randomIr, 12345);
    ImpulseResponse longIr{48000, 1, std::vector<float>(65537), {}, {}};
    longIr.samples.front() = 0.75f; longIr.samples.back() = -0.25f;
    compare({1,-1}, longIr, 0);
    WetRender unchanged; unchanged.samples = {3,4};
    REQUIRE(! renderReverseConvolution(randomSource, 0, 48000, mono, 0, unchanged, error, [] { return true; }));
    REQUIRE(unchanged.samples == std::vector<float>({3,4}));
    uint64_t required = 0;
    REQUIRE(! estimateRenderMemory(UINT64_MAX, 3, 0, required, error));
    REQUIRE(! estimateRenderMemory(1000, 1000, previewMemoryBudget, required, error));
    REQUIRE(! renderReverseConvolution({}, 0, 48000, mono, 0, unchanged, error));
    REQUIRE(! renderReverseConvolution({1,1}, INT64_MIN, 48000, mono, 0, unchanged, error));
    TestDirectory root;
    writeTestWav(root.file.getChildFile("mono.wav"), 1, 44100, {0,0,0.25f,-0.5f}, 16);
    ImpulseResponse decoded, prepared;
    REQUIRE(loadImpulseResponse(root.file.getChildFile("mono.wav"), previewMemoryBudget, decoded, error));
    REQUIRE(decoded.channels == 1 && decoded.frames() == 4 && decoded.samples[0] == 0);
    REQUIRE(prepareImpulseResponse(decoded, 44100, previewMemoryBudget, prepared, error));
    REQUIRE(prepared.samples == decoded.samples);
    writeTestWav(root.file.getChildFile("stereo.wav"), 2, 48000, {1,0,0,-0.5f});
    REQUIRE(loadImpulseResponse(root.file.getChildFile("stereo.wav"), previewMemoryBudget, decoded, error));
    REQUIRE(decoded.samples == std::vector<float>({1,0,0,-0.5f}));
    writeTestWav(root.file.getChildFile("four.wav"), 4, 48000, std::vector<float>(16, 0));
    REQUIRE(! loadImpulseResponse(root.file.getChildFile("four.wav"), previewMemoryBudget, decoded, error));
    REQUIRE(! loadImpulseResponse(root.file.getChildFile("missing.wav"), previewMemoryBudget, decoded, error));
    REQUIRE(root.file.getChildFile("compressed-disguised.wav").replaceWithText("wvpk compressed test header"));
    const auto retainedIR = decoded.samples;
    REQUIRE(! loadImpulseResponse(root.file.getChildFile("compressed-disguised.wav"), previewMemoryBudget, decoded, error));
    REQUIRE(error.contains("WavPack") && decoded.samples == retainedIR);
    writeTestWav(root.file.getChildFile("empty.wav"), 1, 48000, {});
    REQUIRE(! loadImpulseResponse(root.file.getChildFile("empty.wav"), previewMemoryBudget, decoded, error));
    REQUIRE(! loadImpulseResponse(root.file.getChildFile("stereo.wav"), 1, decoded, error));
    writeTestWav(root.file.getChildFile("nan.wav"), 1, 48000, {std::numeric_limits<float>::quiet_NaN()});
    REQUIRE(! loadImpulseResponse(root.file.getChildFile("nan.wav"), previewMemoryBudget, decoded, error));
    ImpulseResponse impulse{48000, 1, std::vector<float>(400, 0), {}, {}};
    impulse.samples[160] = 1;
    for (const auto rate : {44100.0, 48000.0, 96000.0})
    {
        REQUIRE(prepareImpulseResponse(impulse, rate, previewMemoryBudget, prepared, error));
        REQUIRE(prepared.frames() == static_cast<size_t>(std::round(400 * rate / 48000)));
        const auto peak = std::max_element(prepared.samples.begin(), prepared.samples.end()) - prepared.samples.begin();
        REQUIRE(std::abs(static_cast<double>(peak) - 160 * rate / 48000) <= 1);
        REQUIRE(std::abs(prepared.samples.front()) < 1.0e-6);
    }
    ImpulseResponse high{96000, 1, std::vector<float>(4096), {}, {}};
    for (size_t i = 0; i < high.frames(); ++i) high.samples[i] = static_cast<float>(std::sin(2 * std::numbers::pi * 40000 * i / 96000));
    REQUIRE(prepareImpulseResponse(high, 48000, previewMemoryBudget, prepared, error));
    double energy = 0;
    for (size_t i = 100; i < prepared.frames() - 100; ++i) energy += prepared.samples[i] * prepared.samples[i];
    REQUIRE(std::sqrt(energy / (prepared.frames() - 200)) < 0.01);
}); }
