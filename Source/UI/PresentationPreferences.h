#pragma once
#include <juce_core/juce_core.h>
#include <functional>
#include <limits>
#include <mutex>
#include <optional>

namespace m3::ui
{
// UI-local preferences only. Never include this store in processor/session state
// or call its synchronous filesystem operations from the audio callback.
struct PresentationPreferences
{
    int width=900,height=600,scalePercent=100;
    static bool validWidth(int value)noexcept{return value>=900 && value<=2000;}
    static bool validHeight(int value)noexcept{return value>=600 && value<=1400;}
    static bool validScale(int value)noexcept{return value==100 || value==125;}
    bool operator==(const PresentationPreferences&)const=default;
};
enum class PresentationReadStatus {missing,valid,recovered,unsupportedVersion,failed,busy};
struct LoadedPresentation
{
    PresentationPreferences value;
    PresentationReadStatus status=PresentationReadStatus::missing;
    juce::Result error=juce::Result::ok();
};
struct PresentationPatch
{
    std::optional<int> width,height,scalePercent;
    bool empty()const noexcept{return !width && !height && !scalePercent;}
    bool valid()const noexcept
    {return (!width || PresentationPreferences::validWidth(*width))
        && (!height || PresentationPreferences::validHeight(*height))
        && (!scalePercent || PresentationPreferences::validScale(*scalePercent));}
    void apply(PresentationPreferences& value)const noexcept
    {if(width)value.width=*width;if(height)value.height=*height;if(scalePercent)value.scalePercent=*scalePercent;}
};
inline LoadedPresentation decodePresentation(const juce::String& text)
{
    LoadedPresentation result;juce::var json;
    if(juce::JSON::parse(text,json).failed() || !json.isObject())
    {result.status=PresentationReadStatus::recovered;return result;}
    const auto integer=[](const juce::var& value)->std::optional<int>
    {
        if(!value.isInt() && !value.isInt64())return {};
        const auto number=static_cast<juce::int64>(value);
        if(number<std::numeric_limits<int>::min() || number>std::numeric_limits<int>::max())return {};
        return static_cast<int>(number);
    };
    const auto version=integer(json.getProperty("version",{}));
    if(!version || *version!=1)
    {result.status=PresentationReadStatus::unsupportedVersion;return result;}
    result.status=PresentationReadStatus::valid;
    const auto field=[&](const char* key,int& output,auto valid)
    {
        const auto value=integer(json.getProperty(key,{}));
        if(value && valid(*value))output=*value;else result.status=PresentationReadStatus::recovered;
    };
    field("contentWidth",result.value.width,PresentationPreferences::validWidth);
    field("contentHeight",result.value.height,PresentationPreferences::validHeight);
    field("scalePercent",result.value.scalePercent,PresentationPreferences::validScale);
    return result;
}
inline juce::String encodePresentation(PresentationPreferences value)
{
    auto* object=new juce::DynamicObject;
    object->setProperty("version",1);object->setProperty("contentWidth",value.width);
    object->setProperty("contentHeight",value.height);object->setProperty("scalePercent",value.scalePercent);
    return juce::JSON::toString(juce::var(object));
}
class PresentationStorage
{
public:
    virtual ~PresentationStorage()=default;
    virtual LoadedPresentation load()=0;
    // Implementations merge only changed fields against the latest disk value.
    virtual juce::Result save(PresentationPatch)=0;
};
class FilePresentationStorage final:public PresentationStorage
{
public:
    using Publish=std::function<bool(const juce::File&,const juce::File&)>;
    explicit FilePresentationStorage(juce::File destination,Publish publish={})
        :file(std::move(destination)),publisher(publish?std::move(publish):Publish{[](const auto& scratch,const auto& target){return scratch.replaceFileIn(target);}}),
         lock("ReverbArise-presentation-"+juce::String::toHexString(file.getFullPathName().toLowerCase().hashCode64())){}
    static juce::File userFile()
    {
        const auto local=juce::SystemStats::getEnvironmentVariable("LOCALAPPDATA",{});
        const auto parent=local.isNotEmpty()?juce::File(local):juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
        return parent.getChildFile("ReverbArise").getChildFile("presentation-v1.json");
    }
    LoadedPresentation load()override
    {
        const std::unique_lock guard(threadGate,std::try_to_lock);
        if(!guard.owns_lock() || !lock.enter(0))
            return {{},PresentationReadStatus::busy,juce::Result::fail("Presentation preferences are busy")};
        Unlock unlock{lock};return loadLocked();
    }
    juce::Result save(PresentationPatch patch)override
    {
        if(!patch.valid())return juce::Result::fail("Invalid presentation preference");
        if(patch.empty())return juce::Result::ok();
        // Do not wait for another editor/process. The controller retains and
        // retries the pending patch after its coalescing interval.
        const std::unique_lock guard(threadGate,std::try_to_lock);
        if(!guard.owns_lock() || !lock.enter(0))return juce::Result::fail("Presentation preferences are busy");
        Unlock unlock{lock};auto latest=loadLocked();
        if(latest.status==PresentationReadStatus::failed)return latest.error;
        if(latest.status==PresentationReadStatus::unsupportedVersion)
            return juce::Result::fail("Presentation preference version is not supported");
        patch.apply(latest.value);
        if(const auto directory=file.getParentDirectory().createDirectory();directory.failed())return directory;
        juce::TemporaryFile transaction(file);
        {
            juce::FileOutputStream output(transaction.getFile());
            if(output.failedToOpen())return output.getStatus();
            if(!output.writeText(encodePresentation(latest.value),false,false,"\n"))
                return juce::Result::fail("Could not write presentation preferences");
            output.flush();if(output.getStatus().failed())return output.getStatus();
        }
        // One same-directory atomic replacement; never truncate/remove the old
        // target first. Keep the previous record if publication fails.
        if(!publisher(transaction.getFile(),file))return juce::Result::fail("Could not publish presentation preferences");
        return juce::Result::ok();
    }
private:
    struct Unlock{juce::InterProcessLock& value;~Unlock(){value.exit();}};
    LoadedPresentation loadLocked()
    {
        // Windows replacement can briefly make the name unavailable to an
        // uncoordinated reader. Readers participate in the same transaction:
        // contention is explicit, never mistaken for a missing/corrupt profile.
        if(!file.exists())return {};
        juce::FileInputStream input(file);
        if(input.failedToOpen())return {{},PresentationReadStatus::failed,input.getStatus()};
        juce::MemoryBlock bytes;input.readIntoMemoryBlock(bytes,maximumBytes+1);
        if(bytes.getSize()>maximumBytes)return {{},PresentationReadStatus::failed,juce::Result::fail("Presentation preference record is too large")};
        if(input.getStatus().failed())return {{},PresentationReadStatus::failed,input.getStatus()};
        return decodePresentation(juce::String::fromUTF8(static_cast<const char*>(bytes.getData()),static_cast<int>(bytes.getSize())));
    }
    static constexpr size_t maximumBytes=16384;
    juce::File file;Publish publisher;juce::InterProcessLock lock;std::mutex threadGate;
};

// One controller per open editor. An editor loads once and never observes another
// editor's live size. A subsequent opening reads the latest successful record.
class PresentationPreferenceSession
{
public:
    explicit PresentationPreferenceSession(PresentationStorage& destination):storage(destination),initial(storage.load()),desired(initial.value){}
    const PresentationPreferences& preferences()const noexcept{return desired;}
    const LoadedPresentation& initialRead()const noexcept{return initial;}
    bool retryInitialRead()
    {
        if(initial.status!=PresentationReadStatus::busy || userIntent)return false;
        auto next=storage.load();if(next.status==PresentationReadStatus::busy)return false;
        initial=std::move(next);desired=initial.value;return true;
    }
    bool hasPendingSave()const noexcept{return !pending.empty();}
    const juce::Result& lastSave()const noexcept{return lastResult;}
    bool setSize(int width,int height,double now)
    {
        if(!PresentationPreferences::validWidth(width) || !PresentationPreferences::validHeight(height))return false;
        const bool unresolved=initial.status==PresentationReadStatus::busy;
        userIntent=true;bool changed=false;
        if(width!=desired.width || unresolved){desired.width=width;pending.width=width;changed=true;}
        if(height!=desired.height || unresolved){desired.height=height;pending.height=height;changed=true;}
        if(changed)saveAfter=now+coalescingMilliseconds;return true;
    }
    bool setScale(int percent,double now)
    {
        if(!PresentationPreferences::validScale(percent))return false;
        userIntent=true;
        if(percent!=desired.scalePercent || initial.status==PresentationReadStatus::busy)
        {desired.scalePercent=percent;pending.scalePercent=percent;saveAfter=now+coalescingMilliseconds;}
        return true;
    }
    bool saveIfDue(double now,bool force=false)
    {
        if(pending.empty() || (!force && now<saveAfter))return true;
        lastResult=storage.save(pending);
        if(lastResult.wasOk())pending={};else saveAfter=now+coalescingMilliseconds;
        return lastResult.wasOk();
    }
private:
    static constexpr double coalescingMilliseconds=750;
    PresentationStorage& storage;LoadedPresentation initial;PresentationPreferences desired;
    PresentationPatch pending;double saveAfter=0;bool userIntent=false;juce::Result lastResult=juce::Result::ok();
    JUCE_DECLARE_NON_COPYABLE(PresentationPreferenceSession)
};
}
