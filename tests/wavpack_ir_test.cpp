#include "AudioTestFiles.h"
#include <DSP/ReverseConvolution.h>
#include <wavpack.h>
#include <bit>

static std::vector<char> encode(const std::vector<int32_t>& input, int channels, int bits, bool floating, bool hybrid = false)
{
    std::vector<char> encoded;
    const auto write = [](void* destination, void* data, int32_t bytes) -> int
    {
        auto& output = *static_cast<std::vector<char>*>(destination);
        const auto begin = static_cast<const char*>(data);
        output.insert(output.end(), begin, begin + bytes); return 1;
    };
    std::unique_ptr<WavpackContext, decltype(&WavpackCloseFile)> encoder(
        WavpackOpenFileOutput(write, &encoded, nullptr), &WavpackCloseFile);
    REQUIRE(encoder != nullptr);
    WavpackConfig config{};
    config.num_channels = channels; config.sample_rate = 48000;
    config.channel_mask = channels == 1 ? 4 : channels == 2 ? 3 : 0;
    config.bits_per_sample = bits; config.bytes_per_sample = (bits + 7) / 8;
    config.float_norm_exp = floating ? 127 : 0;
    if (hybrid) { config.flags |= CONFIG_HYBRID_FLAG; config.bitrate = 2.0f; }
    REQUIRE(WavpackSetConfiguration64(encoder.get(), &config, static_cast<int64_t>(input.size() / channels), nullptr));
    REQUIRE(WavpackPackInit(encoder.get()));
    auto copy = input; // WavPack encoding may modify its input.
    REQUIRE(WavpackPackSamples(encoder.get(), copy.data(), static_cast<uint32_t>(input.size() / channels)));
    REQUIRE(WavpackFlushSamples(encoder.get()));
    return encoded;
}

int main(int argc, char** argv) { return runTest([&]
{
    TestDirectory root;
    REQUIRE(root.file.createDirectory().wasOk());
    juce::String error;
    ImpulseResponse decoded;
    for (const auto bits : {8,16,20,24,32})
        for (const auto channels : {1,2})
        {
            const auto bytes = (bits + 7) / 8;
            const auto half = static_cast<int32_t>(uint32_t{1} << (bytes * 8 - 2));
            std::vector<int32_t> input(512 * channels);
            for (size_t i = 0; i < input.size(); ++i)
                input[i] = i % 4 == 0 ? half : i % 4 == 1 ? -half : 0;
            const auto packed = encode(input, channels, bits, false);
            const auto file = root.file.getChildFile("pcm-" + juce::String(bits) + "-" + juce::String(channels) + ".wv");
            REQUIRE(file.replaceWithData(packed.data(), packed.size()));
            REQUIRE(loadImpulseResponse(file, previewMemoryBudget, decoded, error));
            REQUIRE(decoded.channels == channels && decoded.rate == 48000 && decoded.frames() == 512);
            for (size_t i = 0; i < input.size(); ++i)
                REQUIRE(decoded.samples[i] == static_cast<float>(std::ldexp(static_cast<double>(input[i]), 1 - bytes * 8)));
        }
    const std::vector<float> floats{0,0, 1.5f,-1.25f, 0.25f,-0.5f, 0,0};
    std::vector<int32_t> floatBits;
    for (auto v : floats) floatBits.push_back(std::bit_cast<int32_t>(v));
    const auto packed = encode(floatBits, 2, 32, true);
    const auto disguised = root.file.getChildFile(juce::String::fromUTF8("ir_\xc3\xa9_\xe9\x9b\xaa.wav"));
    REQUIRE(disguised.replaceWithData(packed.data(), packed.size()));
    REQUIRE(loadImpulseResponse(disguised, previewMemoryBudget, decoded, error));
    REQUIRE(decoded.samples == floats); // No normalization, clipping, or channel mixing.
    const auto native = root.file.getChildFile("same.wv");
    REQUIRE(native.replaceWithData(packed.data(), packed.size()));
    ImpulseResponse fromNative, fromWav;
    REQUIRE(loadImpulseResponse(native, previewMemoryBudget, fromNative, error));
    writeTestWav(root.file.getChildFile("same.wav"), 2, 48000, floats);
    REQUIRE(loadImpulseResponse(root.file.getChildFile("same.wav"), previewMemoryBudget, fromWav, error));
    REQUIRE(fromNative.samples == fromWav.samples && fromNative.samples == decoded.samples);
    const auto saved = decoded.samples;
    REQUIRE(! loadImpulseResponse(native, 1, decoded, error) && error.contains("256 MiB") && decoded.samples == saved);
    const auto damaged = root.file.getChildFile("truncated.wv");
    REQUIRE(damaged.replaceWithData(packed.data(), packed.size() / 2));
    REQUIRE(! loadImpulseResponse(damaged, previewMemoryBudget, decoded, error) && decoded.samples == saved);
    auto corrupt = packed; corrupt.back() ^= 1;
    REQUIRE(damaged.replaceWithData(corrupt.data(), corrupt.size()));
    REQUIRE(! loadImpulseResponse(damaged, previewMemoryBudget, decoded, error));
    const auto multichannel = encode({1,2,3,4}, 4, 16, false);
    REQUIRE(damaged.replaceWithData(multichannel.data(), multichannel.size()));
    REQUIRE(! loadImpulseResponse(damaged, previewMemoryBudget, decoded, error) && error.contains("mono or stereo"));
    const auto nonfinite = encode({std::bit_cast<int32_t>(std::numeric_limits<float>::infinity())}, 1, 32, true);
    REQUIRE(damaged.replaceWithData(nonfinite.data(), nonfinite.size()));
    REQUIRE(! loadImpulseResponse(damaged, previewMemoryBudget, decoded, error) && error.contains("non-finite"));
    std::vector<int32_t> hybridInput(4096);
    for (size_t i = 0; i < hybridInput.size(); ++i) hybridInput[i] = static_cast<int32_t>((i * 7919) % 65536) - 32768;
    const auto hybrid = encode(hybridInput, 1, 16, false, true);
    REQUIRE(damaged.replaceWithData(hybrid.data(), hybrid.size()));
    REQUIRE(! loadImpulseResponse(damaged, previewMemoryBudget, decoded, error) && error.contains("correction"));

    // Optional read-only check of user-supplied media; never part of the portable CTest fixture set.
    if (argc > 1)
    {
        const juce::File file(juce::String::fromUTF8(argv[1]));
        if (! loadImpulseResponse(file, previewMemoryBudget, decoded, error)) throw std::runtime_error(error.toStdString());
        WetRender result;
        REQUIRE(renderReverseConvolution({1,1}, 0, decoded.rate, decoded, 0, result, error));
        REQUIRE(result.frames() == decoded.frames() && result.peak > 0);
        std::cout << "User IR decoded and rendered: " << decoded.channels << " channels, " << decoded.rate
                  << " Hz, " << decoded.frames() << " frames, peak " << result.peak << '\n';
    }
}); }
