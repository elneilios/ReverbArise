#include "ExportFiles.h"
#include <Core/ContentHash.h>
#include <Core/ManagedDirectory.h>
#define NOMINMAX
#include <Windows.h>
namespace m3
{
namespace
{
class CheckedOutput final:public juce::OutputStream
{
public:
    explicit CheckedOutput(std::unique_ptr<juce::FileOutputStream> output):file(std::move(output)){}
    void flush()override{file->flush();}
    int64_t getPosition()override{return file->getPosition();}
    bool setPosition(int64_t at)override{return !failed && file->setPosition(at);}
    bool write(const void* data,size_t count)override{
        if(failed)return false;
        if(shortWrite){shortWrite=false;failed=true;if(count>1)file->write(data,count/2);return false;}
        const auto ok=file->write(data,count);failed=!ok;return ok;
    }
    std::unique_ptr<juce::FileOutputStream> file;bool shortWrite=false,failed=false;
};
bool noLinks(juce::File file,const juce::File& root)
{
    for(;;){const auto attributes=GetFileAttributesW(file.getFullPathName().toWideCharPointer());
        if(attributes!=INVALID_FILE_ATTRIBUTES && (attributes&FILE_ATTRIBUTE_REPARSE_POINT))return false;
        if(file==root)return true;if(!file.isAChildOf(root))return false;file=file.getParentDirectory();}
}
bool interrupted(const RenderCancellation& cancel,juce::String& error)
{if(cancel && cancel()){error="Export superseded or cancelled";return true;}return false;}
bool injected(const ExportFileFault& fault,ExportFileStage stage,juce::String& error)
{if(fault && fault(stage)){error="Export file operation failed";return true;}return false;}
juce::String checksum(const juce::File& file,const RenderCancellation& cancel)
{
    auto input=file.createInputStream();if(!input)throw std::runtime_error("Cannot open Export WAV for checksum");
    ContentHash hash;std::array<char,65536> bytes;
    while(!input->isExhausted()){if(cancel && cancel())throw std::runtime_error("Export checksum cancelled");
        const auto n=input->read(bytes.data(),static_cast<int>(bytes.size()));if(n<=0)throw std::runtime_error("Export checksum read failed");hash.update(bytes.data(),static_cast<size_t>(n));}
    return hash.finish();
}
bool verify(const juce::File& file,const ExportFile& meta,juce::String& error,const RenderCancellation& cancel)
{
    if(interrupted(cancel,error))return false;
    auto input=file.createInputStream();if(!input){error="Export WAV is missing or unreadable";return false;}
    juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(input.release(),true));
    if(!reader || reader->numChannels!=2 || reader->bitsPerSample!=32 || !reader->usesFloatingPointData
        || reader->sampleRate!=meta.rate || reader->lengthInSamples!=meta.frames){error="Export WAV header/frame count is invalid";return false;}
    juce::AudioBuffer<float> block(2,exportPageFrames);
    for(int64_t at=0;at<meta.frames;){if(interrupted(cancel,error))return false;
        const auto frames=static_cast<int>(std::min(int64_t{exportPageFrames},meta.frames-at));
        if(!reader->read(&block,0,frames,at,true,true)){error="Export WAV verification read failed";return false;}
        for(int c=0;c<2;++c)for(int n=0;n<frames;++n)if(!std::isfinite(block.getSample(c,n))){error="Export WAV contains non-finite audio";return false;}
        at+=frames;
    }
    if(checksum(file,cancel)!=meta.checksum){error="Export WAV checksum differs from its immutable reference";return false;}
    error.clear();return true;
}
struct ScratchFile {juce::File file;~ScratchFile(){if(file!=juce::File{})file.deleteFile();}};
bool metadataValid(const ExportFile& meta,juce::String& error)
{ExportState state;state.initialized=true;state.wet=meta.wet;state.history.push_back(meta);state.completedPath=meta.path;return validExportState(state,error);}
}
bool verifyExportFile(const juce::File& instance,const ExportFile& meta,juce::String& error,const RenderCancellation& cancel)
{
    try{if(!metadataValid(meta,error))return false;const auto file=instance.getChildFile(meta.path);
        if(!noLinks(file,instance)){error="Export reference crosses a managed link";return false;}
        return verify(file,meta,error,cancel);
    }catch(const std::exception& e){error=e.what();return false;}
}
bool publishSongExport(SongMix& mix,const juce::File& instance,ExportFile& output,juce::String& error,const RenderCancellation& cancel,const ExportFileFault& fault)
{
    try{
        const auto plan=mix.plan();if(!plan){error="Export composition is not prepared";return false;}
        const auto dir=instance.getChildFile("exports");if(!noLinks(dir,instance) || ensureManagedDirectory(dir).failed()){error="Cannot create safe Export directory";return false;}
        ScratchFile scratch{dir.getChildFile(".unpublished-"+juce::Uuid().toString()+".wav")};
        juce::WavAudioFormat format;auto staged=scratch.file.createOutputStream();
        if(!staged || staged->failedToOpen()){error="Cannot open staged Export WAV";return false;}
        auto checked=std::make_unique<CheckedOutput>(std::move(staged));auto* raw=checked.get();
        std::unique_ptr<juce::OutputStream> stream(std::move(checked));
        const auto options=juce::AudioFormatWriterOptions().withSampleRate(plan->timing.rate).withNumChannels(2).withBitsPerSample(32)
            .withSampleFormat(juce::AudioFormatWriterOptions::SampleFormat::floatingPoint);
        auto writer=format.createWriterFor(stream,options);if(!writer){error="Cannot create float32 Export WAV";return false;}
        raw->shortWrite=fault && fault(ExportFileStage::shortWrite);
        juce::AudioBuffer<float> block(2,exportPageFrames);std::array<float,exportPageFrames*2> samples;
        for(auto at=plan->origin;at<plan->end;){if(interrupted(cancel,error) || injected(fault,ExportFileStage::write,error))return false;
            const auto frames=static_cast<int>(std::min(int64_t{exportPageFrames},plan->end-at));
            if(!mix.read(at,{samples.data(),static_cast<size_t>(frames)*2},error))return false;
            for(int c=0;c<2;++c)for(int n=0;n<frames;++n)block.setSample(c,n,samples[n*2+c]);
            if(!writer->writeFromAudioSampleBuffer(block,0,frames)){error="Export WAV write failed";return false;}at+=frames;
        }
        if(!writer->flush() || raw->failed || raw->file->getStatus().failed() || injected(fault,ExportFileStage::close,error)){error="Export WAV flush/close failed";return false;}writer.reset();
        ExportFile candidate;candidate.composition=plan->identity;candidate.rate=plan->timing.rate;candidate.origin=plan->origin;candidate.frames=plan->end-plan->origin;candidate.wet=plan->wet;
        auto percent=juce::String(static_cast<double>(plan->wet)*100,3);while(percent.endsWithChar('0'))percent=percent.dropLastCharacters(1);if(percent.endsWithChar('.'))percent=percent.dropLastCharacters(1);
        candidate.dry=plan->dry;candidate.gainLaw=2;
        const auto dryPercent=juce::String(static_cast<double>(plan->dry)*100,3);
        candidate.path="exports/Swells-v2-"+dryPercent+"pct-dry-"+percent+"pct-wet-"+juce::Uuid().toString()+".wav";candidate.checksum=checksum(scratch.file,cancel);
        if(!metadataValid(candidate,error) || injected(fault,ExportFileStage::verify,error) || !verify(scratch.file,candidate,error,cancel))return false;
        const auto destination=instance.getChildFile(candidate.path);
        if(interrupted(cancel,error) || injected(fault,ExportFileStage::publish,error))return false;
        if(!noLinks(destination,instance) || !MoveFileExW(scratch.file.getFullPathName().toWideCharPointer(),destination.getFullPathName().toWideCharPointer(),MOVEFILE_WRITE_THROUGH))
        {error="Cannot atomically publish Export WAV";return false;}
        scratch.file={};if(!verifyExportFile(instance,candidate,error,cancel))return false;
        output=std::move(candidate);error.clear();return true;
    }catch(const std::exception& e){error=e.what();return false;}
}
bool saveExportFile(const juce::File& instance,const ExportFile& meta,const juce::File& destination,bool overwrite,juce::String& error,const RenderCancellation& cancel,const ExportFileFault& fault,const ExportCommit& commit)
{
    try{
        if(!verifyExportFile(instance,meta,error,cancel))return false;
        const auto source=instance.getChildFile(meta.path);if(destination==source)return true;
        if(destination==instance || destination.isAChildOf(instance)){error="Managed source, render and Export storage cannot be overwritten";return false;}
        if(destination.exists() && !overwrite){error="Save destination exists; overwrite confirmation required";return false;}
        if(ensureManagedDirectory(destination.getParentDirectory()).failed()){error="Cannot create Save destination";return false;}
        ScratchFile scratch{destination.getSiblingFile(destination.getFileName()+".unpublished-"+juce::Uuid().toString())};
        auto input=source.createInputStream();auto output=scratch.file.createOutputStream();if(!input || !output || output->failedToOpen()){error="Cannot open Save streams";return false;}
        std::array<char,65536> bytes;
        while(!input->isExhausted()){if(interrupted(cancel,error) || injected(fault,ExportFileStage::write,error))return false;
            const auto n=input->read(bytes.data(),static_cast<int>(bytes.size()));if(n<=0 || !output->write(bytes.data(),static_cast<size_t>(n))){error="Save WAV write failed";return false;}}
        output->flush();if(output->getStatus().failed() || injected(fault,ExportFileStage::close,error)){error="Save WAV close failed";return false;}output.reset();input.reset();
        if(injected(fault,ExportFileStage::verify,error) || !verify(scratch.file,meta,error,cancel))return false;
        if(interrupted(cancel,error) || injected(fault,ExportFileStage::publish,error))return false;
        const auto publish=[&]{return MoveFileExW(scratch.file.getFullPathName().toWideCharPointer(),destination.getFullPathName().toWideCharPointer(),MOVEFILE_WRITE_THROUGH|(overwrite?MOVEFILE_REPLACE_EXISTING:0))!=0;};
        if(!(commit?commit(publish):publish())){error="Cannot publish saved WAV or Save request superseded";return false;}
        scratch.file={};error.clear();return true;
    }catch(const std::exception& e){error=e.what();return false;}
}
}
