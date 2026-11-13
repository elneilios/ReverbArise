#include "SwellAssetStore.h"
#include <Core/ContentHash.h>
#include <Core/ManagedDirectory.h>
#define NOMINMAX
#include <Windows.h>
#include <cmath>

namespace m3
{
static bool noLinks(juce::File file, const juce::File& root)
{
    for (;;)
    {
        const auto attributes=GetFileAttributesW(file.getFullPathName().toWideCharPointer());
        if(attributes!=INVALID_FILE_ATTRIBUTES && (attributes&FILE_ATTRIBUTE_REPARSE_POINT)) return false;
        if(file==root) return true;
        if(!file.isAChildOf(root)) return false;
        file=file.getParentDirectory();
    }
}
static bool publish(const juce::File& from,const juce::File& to,juce::String& error)
{
    if(!MoveFileExW(from.getFullPathName().toWideCharPointer(),to.getFullPathName().toWideCharPointer(),MOVEFILE_WRITE_THROUGH))
    { error="Cannot atomically publish managed asset"; return false; } return true;
}
bool SwellAssetStore::resolve(const Asset& a,juce::File& file,juce::String& error) const
{
    if(!a.present() || !safeAssetPath(a.path)) { error="Unsafe or missing managed asset reference"; return false; }
    file=root.getChildFile(a.path);
    if(!noLinks(file,root)) { error="Managed asset path crosses a link"; return false; }
    return true;
}
bool SwellAssetStore::verify(const Asset& a,juce::String& error) const
{
    try
    {
        juce::File file;
        if(!resolve(a,file,error)) return false;
        const auto metadata=file.withFileExtension(".json");
        if(!noLinks(metadata,root) || !metadata.existsAsFile() || metadata.getSize()>16384)
        { error="Missing or invalid managed asset metadata: "+a.path; return false; }
        Asset actual; juce::var v;
        if(juce::JSON::parse(metadata.loadFileAsString(),v).failed() || !parseAssetMetadata(v,actual,error)
            || juce::JSON::toString(assetMetadata(actual),true)!=juce::JSON::toString(assetMetadata(a),true))
        { error="Managed metadata differs from saved reference: "+a.path; return false; }
        if(a.engine!=1 || fileHash(file)!=a.checksum) { error="Managed asset version/checksum mismatch: "+a.path; return false; }
        return true;
    }
    catch(const std::exception& e) { error=e.what(); return false; }
}
bool SwellAssetStore::write(const std::vector<float>& samples,Asset& asset,juce::String& error)
{
    try
    {
        if(asset.frames<=0 || asset.channels<1 || asset.channels>2 || !std::isfinite(asset.rate) || asset.rate<=0
            || asset.frames>INT64_MAX/8 || static_cast<uint64_t>(asset.frames)*asset.channels!=samples.size())
        { error="Invalid managed audio dimensions"; return false; }
        auto candidate=asset;
        const auto kind=candidate.path; // Caller passes only the directory category.
        if(kind!="irs" && kind!="renders") { error="Invalid managed audio kind"; return false; }
        auto key=sampleHash(samples,candidate.rate,candidate.channels);
        if(kind=="renders")
        {
            const auto identity=key+"|"+candidate.dependency+"|"+candidate.irContent+"|"+juce::String(candidate.start)
                +"|"+juce::String(candidate.sourceStart)+"|"+juce::String(candidate.sourceEnd)+"|"+juce::String(candidate.bpm,12)
                +"|"+juce::String(candidate.numerator)+"/"+juce::String(candidate.denominator)+"|engine1";
            ContentHash hash; const auto utf=identity.toUTF8(); hash.update(utf.getAddress(),static_cast<size_t>(utf.sizeInBytes()-1)); key=hash.finish();
        }
        candidate.content=key;
        candidate.path=kind+"/"+key+"-"+juce::Uuid().toString().substring(0,8)+".wav";
        juce::File destination;
        if(!resolve(candidate,destination,error) || ensureManagedDirectory(destination.getParentDirectory()).failed())
        { if(error.isEmpty()) error="Cannot create managed asset directory"; return false; }
        const auto temporary=destination.getSiblingFile(juce::Uuid().toString()+".part");
        juce::WavAudioFormat format;
        std::unique_ptr<juce::OutputStream> output(temporary.createOutputStream());
        const auto options=juce::AudioFormatWriterOptions().withSampleRate(candidate.rate).withNumChannels(candidate.channels)
            .withBitsPerSample(32).withSampleFormat(juce::AudioFormatWriterOptions::SampleFormat::floatingPoint);
        auto writer=format.createWriterFor(output,options);
        if(!writer) { error="Cannot create managed float WAV"; temporary.deleteFile(); return false; }
        juce::AudioBuffer<float> block(candidate.channels,4096);
        for(int64_t at=0;at<candidate.frames;at+=4096)
        {
            const auto count=static_cast<int>(std::min(int64_t{4096},candidate.frames-at));
            for(int c=0;c<candidate.channels;++c) for(int n=0;n<count;++n)
                block.setSample(c,n,samples[static_cast<size_t>(at+n)*candidate.channels+c]);
            if(!writer->writeFromAudioSampleBuffer(block,0,count)) { writer.reset(); temporary.deleteFile(); error="Managed audio write failed"; return false; }
        }
        writer->flush(); writer.reset();
        candidate.checksum=fileHash(temporary);
        // Verify header/complete length without a second full-size decode allocation.
        std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(temporary.createInputStream().release(),true));
        if(!reader || reader->lengthInSamples!=candidate.frames || reader->numChannels!=static_cast<unsigned>(candidate.channels)
            || reader->sampleRate!=candidate.rate || !reader->usesFloatingPointData)
        { reader.reset(); temporary.deleteFile(); error="Managed WAV verification failed"; return false; }
        reader.reset();
        const auto metadata=destination.withFileExtension(".json");
        const auto temporaryMetadata=temporary.withFileExtension(".json.part");
        if(!temporaryMetadata.replaceWithText(juce::JSON::toString(assetMetadata(candidate),true))
            || !publish(temporary,destination,error) || !publish(temporaryMetadata,metadata,error))
        { temporary.deleteFile(); temporaryMetadata.deleteFile(); if(error.isEmpty()) error="Managed metadata write failed"; return false; }
        if(!verify(candidate,error)) return false;
        asset=std::move(candidate); error.clear(); return true;
    }
    catch(const std::exception& e) { error=e.what(); return false; }
}
bool SwellAssetStore::importIR(const juce::File& file,uint64_t budget,Asset& a,juce::String& error)
{ ImpulseResponse ir; return loadImpulseResponse(file,budget,ir,error) && writeIR(ir,a,error); }
bool SwellAssetStore::writeIR(const ImpulseResponse& ir,Asset& output,juce::String& error)
{
    Asset a; a.path="irs"; a.rate=ir.rate; a.channels=ir.channels; a.frames=static_cast<int64_t>(ir.frames()); a.sourceEnd=a.frames;
    if(!write(ir.samples,a,error)) return false; output=std::move(a); return true;
}
bool SwellAssetStore::reuseOrWriteIR(const ImpulseResponse& ir,const juce::String& content,uint64_t budget,
                                    Asset& output,juce::String& error)
{
    const auto directory=root.getChildFile("irs");
    const auto bytes=ir.samples.size()*sizeof(float);
    if(bytes<=budget/3 && noLinks(directory,root))
    {
        int examined=0;
        for(const auto& child:juce::RangedDirectoryIterator(directory,false,content+"-*.json"))
        {
            if(++examined>32)break;
            const auto metadata=child.getFile();
            if(!noLinks(metadata,root) || metadata.getSize()>16384)continue;
            Asset candidate;juce::var value;juce::String ignored;
            if(juce::JSON::parse(metadata.loadFileAsString(),value).failed() || !parseAssetMetadata(value,candidate,ignored)
                || candidate.content!=content || candidate.rate!=ir.rate || candidate.channels!=ir.channels
                || candidate.frames!=static_cast<int64_t>(ir.frames()))continue;
            ImpulseResponse verified;
            if(readIR(candidate,budget-bytes,verified,ignored) && verified.samples==ir.samples)
            {output=std::move(candidate);error.clear();return true;}
        }
    }
    return writeIR(ir,output,error);
}
bool SwellAssetStore::writeRender(const WetRender& wet,const Asset& ir,const juce::String& dependency,
                                 const SongTime& time,Asset& output,juce::String& error)
{
    int64_t end;
    if(wet.frames()>INT64_MAX || !checkedAdd(wet.start,static_cast<int64_t>(wet.frames()),end)
        || end!=wet.sourceEnd || wet.sourceStart<wet.start || wet.sourceStart>=wet.sourceEnd)
    {error="Invalid full render bounds";return false;}
    Asset a; a.path="renders"; a.rate=wet.rate; a.channels=2; a.frames=static_cast<int64_t>(wet.frames());
    a.start=wet.start; a.sourceStart=wet.sourceStart; a.sourceEnd=wet.sourceEnd;
    a.irContent=ir.content; a.dependency=dependency; a.bpm=time.bpm; a.numerator=time.numerator; a.denominator=time.denominator;
    if(!write(wet.samples,a,error)) return false; output=std::move(a); return true;
}
bool SwellAssetStore::readIR(const Asset& a,uint64_t budget,ImpulseResponse& output,juce::String& error) const
{
    if(!verify(a,error)) return false;
    ImpulseResponse ir;
    if(!loadImpulseResponse(root.getChildFile(a.path),budget,ir,error)) return false;
    if(ir.rate!=a.rate || ir.channels!=a.channels || static_cast<int64_t>(ir.frames())!=a.frames
        || (a.path.startsWith("irs/") && sampleHash(ir.samples,ir.rate,ir.channels)!=a.content))
    { error="Managed audio content/format mismatch"; return false; }
    ir.identity=a.content; output=std::move(ir); return true;
}
bool SwellAssetStore::readRender(const Asset& a,uint64_t budget,WetRender& output,juce::String& error) const
{
    ImpulseResponse ir; if(!a.path.startsWith("renders/") || !readIR(a,budget,ir,error)) return false;
    int64_t end;
    if(a.channels!=2 || !checkedAdd(a.start,a.frames,end) || end!=a.sourceEnd || a.sourceStart<a.start || a.sourceStart>=a.sourceEnd)
    { error="Managed render bounds mismatch"; return false; }
    WetRender wet; wet.start=a.start; wet.sourceStart=a.sourceStart; wet.sourceEnd=a.sourceEnd; wet.rate=a.rate;
    wet.samples=std::move(ir.samples);
    for(auto sample:wet.samples) wet.peak=std::max(wet.peak,std::abs(sample));
    output=std::move(wet); return true;
}
}
