#include "DefaultImpulse.h"
#include <PreverbResources.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <cmath>
#include <Core/ContentHash.h>

namespace m3
{
bool bundledImpulse(ImpulseResponse& output, juce::String& error)
{return decodeDefaultImpulse(PreverbResources::PreverbHall_wav,PreverbResources::PreverbHall_wavSize,output,error);}
bool decodeDefaultImpulse(const void* bytes,size_t size,ImpulseResponse& output,juce::String& error)
{
    if(!bytes || size==0){error="Bundled IR is missing; assign a replacement IR";return false;}
    ContentHash checksum;checksum.update(bytes,size);
    if(checksum.finish()!="b7275a27fc7629a9a1a7191abda4fbb3c8a46372dc9071e36dc1112e4839b458")
    {error="Bundled IR checksum mismatch; assign a replacement IR";return false;}
    juce::WavAudioFormat format;
    auto stream=std::make_unique<juce::MemoryInputStream>(bytes,size,false);
    std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(stream.release(),true));
    if(!reader || reader->numChannels!=2 || reader->sampleRate!=48000 || reader->lengthInSamples!=86400)
    {error="Bundled Preverb Hall IR is invalid; assign a replacement IR";return false;}
    juce::AudioBuffer<float> audio(2,86400);
    if(!reader->read(&audio,0,86400,0,true,true)){error="Cannot decode bundled IR";return false;}
    ImpulseResponse result;result.rate=reader->sampleRate;result.channels=2;result.samples.reserve(172800);
    for(int i=0;i<86400;++i)for(int c=0;c<2;++c)
    {const auto v=audio.getSample(c,i);if(!std::isfinite(v)){error="Non-finite bundled IR";return false;}result.samples.push_back(v);}
    output=std::move(result);error.clear();return true;
}
bool installBundledImpulse(SwellAssetStore& store,Asset& output,juce::String& error)
{ImpulseResponse ir;return bundledImpulse(ir,error) && store.writeIR(ir,output,error);}
}
