#include "ReverseConvolution.h"
#include "WavPackDecoder.h"
#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstring>
#include <limits>
#include <numbers>

namespace
{
bool fail(juce::String& error, const juce::String& reason) { error = reason; return false; }
bool finiteSamples(const std::vector<float>& data)
{ return std::all_of(data.begin(), data.end(), [](float v) { return std::isfinite(v); }); }
}

bool loadImpulseResponse(const juce::File& file, uint64_t available, ImpulseResponse& output, juce::String& error)
{
    juce::WavAudioFormat format;
    auto input = file.createInputStream();
    if (! input) return fail(error, "Cannot read WAV/WavPack IR");
    char signature[4]{};
    const auto signatureBytes = input->read(signature, 4);
    if (! input->setPosition(0)) return fail(error, "Cannot seek IR file");
    if (signatureBytes == 4 && std::memcmp(signature, "wvpk", 4) == 0)
    {
        input.reset();
        return loadWavPackImpulseResponse(file, available, output, error);
    }
    std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(input.release(), true));
    if (! reader) return fail(error, "Cannot decode this IR. Use a mono/stereo PCM/float WAV or lossless WavPack file.");
    if (reader->numChannels < 1 || reader->numChannels > 2)
        return fail(error, "IR has " + juce::String(reader->numChannels) + " channels; only mono/stereo WAV is supported.");
    if (reader->lengthInSamples <= 0) return fail(error, "IR WAV contains no audio frames");
    if (! std::isfinite(reader->sampleRate) || reader->sampleRate <= 0)
        return fail(error, "IR WAV has an invalid sample rate");
    const auto frames = static_cast<uint64_t>(reader->lengthInSamples);
    if (frames > static_cast<uint64_t>(std::numeric_limits<int>::max())
        || frames > std::min(available, previewMemoryBudget) / (reader->numChannels * sizeof(float) * 2))
        return fail(error, "IR exceeds 256 MiB preview memory budget");
    juce::AudioBuffer<float> buffer(static_cast<int>(reader->numChannels), static_cast<int>(frames));
    if (! reader->read(&buffer, 0, static_cast<int>(frames), 0, true, reader->numChannels == 2))
        return fail(error, "Cannot decode complete IR");
    ImpulseResponse result;
    result.rate = reader->sampleRate; result.channels = static_cast<int>(reader->numChannels);
    result.name = file.getFileName(); result.identity = juce::Uuid().toDashedString();
    result.samples.resize(static_cast<size_t>(frames) * result.channels);
    for (size_t frame = 0; frame < frames; ++frame)
        for (int channel = 0; channel < result.channels; ++channel)
            result.samples[frame * result.channels + channel] = buffer.getSample(channel, static_cast<int>(frame));
    if (! finiteSamples(result.samples)) return fail(error, "IR contains non-finite samples");
    output = std::move(result); error.clear(); return true;
}

bool prepareImpulseResponse(const ImpulseResponse& input, double rate, uint64_t available,
                            ImpulseResponse& output, juce::String& error, const RenderCancellation& cancelled)
{
    if (input.channels < 1 || input.channels > 2 || input.frames() == 0
        || ! std::isfinite(rate) || rate <= 0 || ! std::isfinite(input.rate) || input.rate <= 0
        || ! finiteSamples(input.samples)) return fail(error, "Invalid IR preparation input");
    const auto length = std::max(1.0, std::round(static_cast<double>(input.frames()) * rate / input.rate));
    if (! std::isfinite(length) || length > static_cast<double>(std::min(available, previewMemoryBudget) / (input.channels * sizeof(float))))
        return fail(error, "Prepared IR exceeds 256 MiB preview memory budget");
    ImpulseResponse result;
    result.rate = rate; result.channels = input.channels; result.name = input.name; result.identity = input.identity;
    result.samples.resize(static_cast<size_t>(length) * input.channels);
    if (rate == input.rate) result.samples = input.samples;
    else
    {
        // Symmetric 64-lobe Hann-windowed sinc, evaluated at the exact input origin.
        // Zero extension preserves the origin without adding a causal filter delay.
        const auto cutoff = std::min(1.0, rate / input.rate);
        const auto radius = 32.0 / cutoff;
        if (radius > 4096) return fail(error, "Unsupported IR sample-rate ratio");
        for (size_t frame = 0; frame < static_cast<size_t>(length); ++frame)
        {
            if ((frame & 1023) == 0 && cancelled && cancelled()) return fail(error, "Cancelled");
            const auto position = static_cast<double>(frame) * input.rate / rate;
            double sums[2]{}, weight = 0;
            const auto first = static_cast<int64_t>(std::ceil(position - radius));
            const auto last = static_cast<int64_t>(std::floor(position + radius));
            for (auto sample = first; sample <= last; ++sample)
            {
                const auto distance = position - static_cast<double>(sample);
                const auto argument = std::numbers::pi * distance * cutoff;
                const auto sinc = std::abs(argument) < 1.0e-12 ? 1.0 : std::sin(argument) / argument;
                const auto coefficient = cutoff * sinc * (0.5 + 0.5 * std::cos(std::numbers::pi * distance / radius));
                weight += coefficient;
                if (sample < 0 || static_cast<uint64_t>(sample) >= input.frames()) continue;
                for (int channel = 0; channel < input.channels; ++channel)
                    sums[channel] += coefficient * input.samples[static_cast<size_t>(sample) * input.channels + channel];
            }
            for (int channel = 0; channel < input.channels; ++channel)
                result.samples[frame * input.channels + channel] = static_cast<float>(sums[channel] / weight);
        }
    }
    if (! finiteSamples(result.samples)) return fail(error, "IR resampling produced non-finite samples");
    output = std::move(result); error.clear(); return true;
}

bool estimateRenderMemory(uint64_t n, uint64_t m, uint64_t occupied, uint64_t& required, juce::String& error)
{
    if (n == 0 || m == 0 || n > previewMemoryBudget / 8 || m > previewMemoryBudget / 8
        || occupied > previewMemoryBudget) return fail(error, "Request exceeds 256 MiB preview memory budget");
    const auto length = n + m - 1;
    uint64_t fft = 1;
    while (fft < length) fft *= 2;
    // Source, stereo prepared IR and output, three complex FFT arrays, FFT plan/scratch,
    // plus a bounded capture-chunk decoding allowance (four copies at the M1 chunk cap).
    required = occupied + 8 * (n + m + length) + fft * 48 + 64ULL * 1024 * 1024;
    if (required > previewMemoryBudget) return fail(error, "Request exceeds 256 MiB preview memory budget; select a shorter region or IR");
    error.clear(); return true;
}

bool renderReverseConvolution(const std::vector<float>& source, int64_t start, double rate,
    const ImpulseResponse& ir, uint64_t occupied, WetRender& output, juce::String& error,
    const RenderCancellation& cancelled, const RenderProgress& progress)
{
    const auto n = source.size() / 2, m = ir.frames();
    uint64_t required = 0;
    if (source.size() % 2 || ! finiteSamples(source) || ! finiteSamples(ir.samples)
        || ! std::isfinite(rate) || rate <= 0 || rate != ir.rate || ir.channels < 1 || ir.channels > 2)
        return fail(error, "Invalid source/IR format or non-finite samples");
    if (! estimateRenderMemory(n, m, occupied, required, error)) return false;
    if (start < std::numeric_limits<int64_t>::min() + static_cast<int64_t>(m - 1)
        || start > std::numeric_limits<int64_t>::max() - static_cast<int64_t>(n))
        return fail(error, "Render timeline position overflows");
    int order = 0;
    size_t fftSize = 1;
    while (fftSize < n + m - 1) { fftSize *= 2; ++order; }
    // JUCE supports order zero inconsistently across backends; pad a singleton to 2.
    if (order == 0) { order = 1; fftSize = 2; }
    juce::dsp::FFT fft(order);
    std::vector<std::complex<float>> a(fftSize), b(fftSize), scratch(fftSize);
    WetRender result;
    result.start = start - static_cast<int64_t>(m - 1); result.sourceStart = start;
    result.sourceEnd = start + static_cast<int64_t>(n); result.rate = rate;
    result.samples.resize((n + m - 1) * 2);
    for (int channel = 0; channel < 2; ++channel)
    {
        if (cancelled && cancelled()) return fail(error, "Cancelled");
        std::fill(a.begin(), a.end(), std::complex<float>{});
        std::fill(b.begin(), b.end(), std::complex<float>{});
        for (size_t frame = 0; frame < n; ++frame) a[frame] = source[frame * 2 + channel];
        for (size_t frame = 0; frame < m; ++frame) b[frame] = ir.samples[(m - 1 - frame) * ir.channels + (ir.channels == 1 ? 0 : channel)];
        fft.perform(a.data(), scratch.data(), false); a.swap(scratch);
        if (cancelled && cancelled()) return fail(error, "Cancelled");
        fft.perform(b.data(), scratch.data(), false);
        for (size_t bin = 0; bin < fftSize; ++bin) a[bin] *= scratch[bin];
        if (cancelled && cancelled()) return fail(error, "Cancelled");
        fft.perform(a.data(), b.data(), true);
        for (size_t frame = 0; frame < n + m - 1; ++frame)
        {
            const auto value = b[frame].real();
            if (! std::isfinite(value)) return fail(error, "Convolution produced non-finite samples");
            result.samples[frame * 2 + channel] = value;
            result.peak = std::max(result.peak, std::abs(value));
        }
        if (progress) progress(static_cast<float>(channel + 1) / 2);
    }
    if (cancelled && cancelled()) return fail(error, "Cancelled");
    output = std::move(result); error.clear(); return true;
}
