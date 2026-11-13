#include "DefaultImpulse.h"
#include <PreverbResources.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <cmath>
#include <Core/ContentHash.h>

namespace m3
{
std::span<const BundledImpulse> bundledImpulses()
{
    static const BundledImpulse catalogue[]{
        {defaultImpulseId,1,"Preverb Hall","Original compact diffuse stereo hall.","CC0-1.0","generate_default_ir.py/v1",
         48000,2,16,86400,PreverbResources::PreverbHall_wav,static_cast<size_t>(PreverbResources::PreverbHall_wavSize),
         "b7275a27fc7629a9a1a7191abda4fbb3c8a46372dc9071e36dc1112e4839b458",
         defaultImpulseContent},
        #define BUNDLED_IMPULSE(...) {__VA_ARGS__},
        #include "BundledImpulseEntries.inc"
        #undef BUNDLED_IMPULSE
    };
    return catalogue;
}
const BundledImpulse* findBundledImpulse(const juce::String& id)
{for(const auto& entry:bundledImpulses())if(id==entry.id)return &entry;return nullptr;}
bool bundledImpulse(ImpulseResponse& output, juce::String& error)
{return decodeBundledImpulse(*findBundledImpulse(defaultImpulseId),previewMemoryBudget,output,error);}
bool decodeDefaultImpulse(const void* bytes,size_t size,ImpulseResponse& output,juce::String& error)
{
    auto entry=*findBundledImpulse(defaultImpulseId);entry.bytes=bytes;entry.size=size;
    return decodeBundledImpulse(entry,previewMemoryBudget,output,error);
}
bool decodeBundledImpulse(const BundledImpulse& entry,uint64_t budget,ImpulseResponse& output,juce::String& error)
{
    const auto bytes=entry.bytes;const auto size=entry.size;
    if(!bytes || size==0){error="Bundled IR is missing; assign a replacement IR";return false;}
    if(entry.channels<1 || entry.channels>2 || entry.frames<=0 || entry.frames>INT_MAX || !std::isfinite(entry.rate) || entry.rate<=0
        || static_cast<uint64_t>(entry.frames)>std::min(budget,previewMemoryBudget)/(entry.channels*sizeof(float)*2))
    {error="Bundled IR exceeds available working budget or has invalid metadata";return false;}
    ContentHash checksum;checksum.update(bytes,size);
    if(checksum.finish()!=entry.wavHash)
    {error="Bundled IR checksum mismatch; assign a replacement IR";return false;}
    juce::WavAudioFormat format;
    auto stream=std::make_unique<juce::MemoryInputStream>(bytes,size,false);
    std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(stream.release(),true));
    if(!reader || reader->numChannels!=entry.channels || reader->sampleRate!=entry.rate || reader->lengthInSamples!=entry.frames || reader->bitsPerSample!=entry.bits)
    {error="Bundled IR format does not match catalogue";return false;}
    const auto frames=static_cast<int>(entry.frames);
    juce::AudioBuffer<float> audio(entry.channels,frames);
    if(!reader->read(&audio,0,frames,0,true,entry.channels==2)){error="Cannot decode bundled IR";return false;}
    ImpulseResponse result;result.rate=reader->sampleRate;result.channels=entry.channels;
    result.name=entry.name;result.identity=juce::String(entry.id)+"@"+juce::String(entry.revision);
    result.samples.reserve(static_cast<size_t>(frames)*entry.channels);
    for(int i=0;i<frames;++i)for(int c=0;c<entry.channels;++c)
    {const auto v=audio.getSample(c,i);if(!std::isfinite(v)){error="Non-finite bundled IR";return false;}result.samples.push_back(v);}
    if(sampleHash(result.samples,result.rate,result.channels)!=entry.decodedHash)
    {error="Bundled decoded IR checksum mismatch";return false;}
    output=std::move(result);error.clear();return true;
}
bool installBundledImpulse(SwellAssetStore& store,Asset& output,juce::String& error)
{return installBundledImpulse(store,defaultImpulseId,previewMemoryBudget,output,error);}
bool installBundledImpulse(SwellAssetStore& store,const juce::String& id,uint64_t budget,Asset& output,juce::String& error)
{
    const auto* entry=findBundledImpulse(id);if(!entry){error="Unknown bundled IR identity";return false;}
    ImpulseResponse ir;if(!decodeBundledImpulse(*entry,budget,ir,error))return false;
    return store.reuseOrWriteIR(ir,entry->decodedHash,budget,output,error);
}
}
