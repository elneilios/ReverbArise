#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_processors_headless/format_types/VST3_SDK/pluginterfaces/vst/ivstattributes.h>
#include <deque>

// Opt-in native investigation only. Never read state bytes, seek, infer a project
// path or touch assets. Calls originate at VST3 state control boundaries, not DSP.
namespace issue8::probe
{
class Log final:private juce::Thread
{
public:
    Log():Thread("Issue 8 folder discovery") { startThread(); }
    ~Log()override { signalThreadShouldExit();notify();stopThread(-1); }
    void post(const juce::var& event)
    {
        const juce::ScopedLock guard(lock);
        if(accepted++>=512 || queue.size()>=128)return;
        queue.push_back(juce::JSON::toString(event,true)+"\n");notify();
    }
private:
    void run()override
    {
        const auto chosen=juce::SystemStats::getEnvironmentVariable("PREVERB_FOLDER_PROBE_LOG_DIR",{});
        const auto directory=chosen.isEmpty()?juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("ReverbAriseFolderProbe"):juce::File(chosen);
        if(directory.createDirectory().failed())return;
        const auto file=directory.getChildFile("host-"+juce::Uuid().toString()+".jsonl");
        auto output=file.createOutputStream();if(!output)return;
        size_t written=0;
        for(;;){std::deque<juce::String> batch;{const juce::ScopedLock guard(lock);batch.swap(queue);}
            for(const auto& row:batch){if(written+row.getNumBytesAsUTF8()>1024*1024)return;*output<<row;written+=row.getNumBytesAsUTF8();}output->flush();
            if(threadShouldExit())break;wait(250);
        }
    }
    juce::CriticalSection lock;std::deque<juce::String> queue;size_t accepted=0;
};
inline juce::String text(const Steinberg::Vst::TChar* value)
{return juce::String(juce::CharPointer_UTF16(reinterpret_cast<const juce::CharPointer_UTF16::CharType*>(value)));}
inline void observe(const char* operation,Steinberg::IBStream* state)noexcept
{
    try {
        auto* data=new juce::DynamicObject;juce::var event(data);
        data->setProperty("schema",1);data->setProperty("probe","issue8-stream-metadata-only-1");
        data->setProperty("juce",juce::SystemStats::getJUCEVersion());data->setProperty("operation",operation);
        data->setProperty("time",juce::Time::getCurrentTime().toISO8601(true));
        // currentExecutableFile identifies the loaded DLL in this plugin context, not the host/project.
        data->setProperty("hostExecutable",juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName());
        data->setProperty("thread",juce::MessageManager::getInstance()->isThisTheMessageThread()?"message":"other-control");
        data->setProperty("streamPresent",state!=nullptr);
        Steinberg::FUnknownPtr<Steinberg::Vst::IStreamAttributes> attributes(state);
        data->setProperty("streamAttributes",attributes!=nullptr);
        if(attributes){Steinberg::Vst::String128 name{};const auto code=attributes->getFileName(name);name[127]=0;
            data->setProperty("fileNameResult",static_cast<int>(code));if(code==Steinberg::kResultOk)data->setProperty("streamFileName",text(name));
            auto* list=attributes->getAttributes();data->setProperty("attributeList",list!=nullptr);
            if(list){for(const auto* key:{"StateType","FilePathString"}){Steinberg::Vst::TChar buffer[1024]{};
                const auto result=list->getString(key,buffer,sizeof(buffer));buffer[1023]=0;
                data->setProperty(juce::String(key)+"Result",static_cast<int>(result));if(result==Steinberg::kResultOk)data->setProperty(key,text(buffer));}}
        }
        static Log log;log.post(event);
    }catch(...){/* Diagnostic failure must not prevent normal save/restore. */}
}
}
