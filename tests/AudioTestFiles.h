#pragma once
#include "TestSupport.h"
#include <juce_audio_formats/juce_audio_formats.h>
inline void writeTestWav(const juce::File& file, int channels, double rate,
                         const std::vector<float>& samples, int bits = 32)
{
    REQUIRE(file.getParentDirectory().createDirectory().wasOk());
    juce::AudioBuffer<float> buffer(channels, static_cast<int>(samples.size() / channels));
    for (int frame = 0; frame < buffer.getNumSamples(); ++frame)
        for (int channel = 0; channel < channels; ++channel)
            buffer.setSample(channel, frame, samples[static_cast<size_t>(frame) * channels + channel]);
    juce::WavAudioFormat format;
    auto options = juce::AudioFormatWriterOptions().withSampleRate(rate).withNumChannels(channels).withBitsPerSample(bits);
    if (bits == 32) options = options.withSampleFormat(juce::AudioFormatWriterOptions::SampleFormat::floatingPoint);
    std::unique_ptr<juce::OutputStream> stream(file.createOutputStream());
    auto writer = format.createWriterFor(stream, options);
    REQUIRE(writer && writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples()));
}
