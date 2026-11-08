#include <DSP/ReverseConvolution.h>
#include <DSP/SourceEdgeConditioning.h>
#include <DSP/SwellShape.h>
#include <algorithm>
#include <cmath>
#include <iostream>

int main(int argc, char** argv)
{
    if (argc!=2 && argc!=3)
        return 2;
    juce::WavAudioFormat wav;
    auto stream=juce::File(juce::String::fromUTF8(argv[1])).createInputStream();
    if (!stream)
        return 3;
    std::unique_ptr<juce::AudioFormatReader> reader(wav.createReaderFor(stream.release(),true));
    constexpr int length=2048;
    if (!reader || reader->lengthInSamples!=49153 || reader->sampleRate!=48000 || reader->numChannels!=2)
        return 4;
    juce::AudioBuffer<float> dry(2,length);
    if (!reader->read(&dry,0,length,0,true,true))
        return 5;
    std::vector<float> owned(static_cast<size_t>(2*length));
    for (int frame=0;frame<length;++frame)
        for (int channel=0;channel<2;++channel)
            owned[static_cast<size_t>(frame*2+channel)]=dry.getSample(channel,frame);

    ImpulseResponse ir;
    ir.rate=reader->sampleRate;
    ir.channels=1;
    ir.samples.resize(length+1);
    ir.samples[0]=0.2f;
    ir.samples[960]=0.1f;
    ir.samples[length]=0.5f;
    const auto before=renderWorkCounts();
    juce::String error;
    WetRender raw;
    m3::Swell edit;
    edit.sourceEnd=length;
    if (!m3::conditionWetSourceEdges(owned,ir.rate,error)
        || !renderReverseConvolution(owned,0,ir.rate,ir,0,raw,error)
        || !m3::fullShape(edit,raw.start,raw.sourceEnd,error))
    {
        std::cerr << error << '\n';
        return 6;
    }
    if (raw.start!=-length || raw.frames()!=2*length)
        return 7;
    edit.shape.fadeIn=48;
    edit.shape.fadeOut=48;
    WetRender shaped, translated;
    if (!m3::prepareShape(edit,raw,previewMemoryBudget,shaped,error))
        return 8;
    edit.placement=32;
    if (!m3::prepareShape(edit,raw,previewMemoryBudget,translated,error))
        return 9;
    const auto after=renderWorkCounts();
    if (translated.start!=shaped.start+32 || translated.samples!=shaped.samples
        || after.convolutions-before.convolutions!=1)
        return 10;

    float leadPeak=0;
    for (int i=0;i<2*length;++i)
        leadPeak=std::max(leadPeak,std::abs(shaped.samples[static_cast<size_t>(i)]));
    if (leadPeak<=0.001f)
        return 11;
    if (argc==3 && !juce::File(juce::String::fromUTF8(argv[2])).replaceWithData(
        shaped.samples.data(),shaped.samples.size()*sizeof(float)))
        return 12;
    std::cout << "Source frames: " << reader->lengthInSamples << "\n"
              << "Negative start: " << shaped.start << ", prepared frames: " << shaped.frames() << "\n"
              << "Audible leading wet peak: " << leadPeak << "\n"
              << "Translation preserved every shaped sample and reused one convolution.\n";
}
