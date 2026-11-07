#include "WorkingFolder.h"
#include <Core/ContentHash.h>
#define NOMINMAX
#include <Windows.h>
#include <winternl.h>
#include <array>
#include <cwchar>
#include <cstddef>
#include <stdexcept>

namespace m3::storage::portable
{
namespace
{
struct Failure:std::runtime_error {using std::runtime_error::runtime_error;NTSTATUS status=0;Failure(const std::string& message,NTSTATUS code):std::runtime_error(message),status(code){}};
struct Cancelled final:Failure {using Failure::Failure;};
void require(bool good,const juce::String& message){if(!good)throw Failure(message.toStdString());}
bool fail(juce::String& e,const juce::String& m){e=m;return false;}
bool hex(const juce::String& s){return s.length()==64 && s.containsOnly("0123456789abcdef");}
bool uuid(const juce::String& s){return s.length()==36 && !juce::Uuid(s).isNull() && juce::Uuid(s).toDashedString()==s;}
juce::String digest(const void* bytes,size_t n){ContentHash h;h.update(bytes,n);return h.finish();}
juce::String digest(const juce::String& s){return digest(s.toRawUTF8(),s.getNumBytesAsUTF8());}
juce::var object(){return new juce::DynamicObject;}
void set(juce::var& v,const juce::Identifier& key,const juce::var& value){v.getDynamicObject()->setProperty(key,value);}
juce::String json(const juce::var& v){return juce::JSON::toString(v,true);}
bool version(const juce::var& v,int expected){return v.isObject() && v["version"].isInt() && static_cast<int>(v["version"])==expected;}
bool number(const juce::var& v,uint64_t& n){const auto s=v.toString();const auto value=s.getLargeIntValue();if(!v.isString() || value<0 || juce::String(value)!=s)return false;n=static_cast<uint64_t>(value);return true;}
juce::String objectPath(const Entry& e){return e.path.endsWith(".wav")?"objects/sha256/"+e.sha256.substring(0,2)+"/"+e.sha256+".wav":"descriptors/"+e.sha256+".json";}

bool jsonShape(const juce::String& text)
{
    std::array<juce::juce_wchar,32> stack{};std::array<size_t,32> members{},items{};size_t depth=0,nodes=0,separators=0;bool quoted=false,escaped=false;auto p=text.getCharPointer();
    while(!p.isEmpty()){const auto c=p.getAndAdvance();if(quoted){if(escaped)escaped=false;else if(c=='\\')escaped=true;else if(c=='"')quoted=false;continue;}
        if(c=='"')quoted=true;else if(c=='{' || c=='['){if(depth==stack.size() || ++nodes>200000)return false;members[depth]=0;items[depth]=0;stack[depth++]=c;}
        else if(c=='}' || c==']'){if(!depth || stack[--depth]!=(c=='}'?'{':'['))return false;}
        else if(c==':' || c==','){if(++separators>1000000 || !depth)return false;
            if(c==':' && (stack[depth-1]!='{' || ++members[depth-1]>128))return false;
            if(c==',' && stack[depth-1]=='[' && ++items[depth-1]>=65536)return false;}}
    return !depth && !quoted && !escaped;
}
juce::var boundedJson(const juce::String& text)
{require(static_cast<size_t>(text.getNumBytesAsUTF8())<=maxDocumentBytes && jsonShape(text),"Malformed or excessively nested JSON metadata");return juce::JSON::parse(text);}
bool xmlShape(const juce::String& text)
{
    int depth=0,tags=0,attributes=0;bool inTag=false,closing=false;juce::juce_wchar quote=0,previous=0;auto p=text.getCharPointer();
    while(!p.isEmpty()){const auto c=p.getAndAdvance();if(!inTag){if(c!='<')continue;if(*p=='!')return false;
            if(*p=='?'){bool ended=false;while(!p.isEmpty()){const auto q=p.getAndAdvance();if(q=='?' && *p=='>'){++p;ended=true;break;}}if(!ended)return false;continue;}
            inTag=true;closing=*p=='/';previous=0;continue;}
        if(quote){if(c==quote)quote=0;continue;}if(c=='"' || c=='\''){quote=c;continue;}
        if(c=='=' && ++attributes>256)return false;if(c=='<')return false;if(c=='>'){if(++tags>128)return false;if(closing){if(--depth<0)return false;}else{if(++depth>16)return false;if(previous=='/')--depth;}inTag=false;}
        else if(!juce::CharacterFunctions::isWhitespace(c))previous=c;}
    return !inTag && !quote && depth==0;
}
bool legacy(const juce::String& xml,const juce::String& manifest,ObservedSnapshot& out,juce::String& error)
{
    if(static_cast<size_t>(xml.getNumBytesAsUTF8())>97*1024*1024 || static_cast<size_t>(manifest.getNumBytesAsUTF8())>maximumMetadataBytes
        || !xmlShape(xml) || (!manifest.isEmpty() && !jsonShape(manifest)))return fail(error,"Oversized, declared or excessively nested accepted metadata");
    const auto doc=juce::XmlDocument::parse(xml);if(!doc)return fail(error,"Malformed accepted processor state");
    if(const auto* creative=doc->getChildByName("SwellSession"))if(!jsonShape(creative->getAllSubText()))return fail(error,"Excessively nested creative metadata");
    return readLegacySnapshot(xml,manifest,out,error);
}
bool only(const juce::var& v,const juce::StringArray& names)
{const auto* value=v.getDynamicObject();if(!value)return false;const auto& properties=value->getProperties();for(int i=0;i<properties.size();++i)if(!names.contains(properties.getName(i).toString()))return false;return true;}

struct Handle
{
    HANDLE h=INVALID_HANDLE_VALUE;
    explicit Handle(HANDLE value):h(value){}~Handle(){if(h!=INVALID_HANDLE_VALUE)CloseHandle(h);}
    Handle(const Handle&)=delete;Handle& operator=(const Handle&)=delete;
};
using Held=std::shared_ptr<Handle>;
Held hold(HANDLE h,const juce::String& message){require(h!=INVALID_HANDLE_VALUE,message+" (Windows "+juce::String(GetLastError())+")");return std::make_shared<Handle>(h);}
juce::String resolved(HANDLE h)
{
    std::array<wchar_t,32768> b{};const auto n=GetFinalPathNameByHandleW(h,b.data(),static_cast<DWORD>(b.size()),FILE_NAME_NORMALIZED|VOLUME_NAME_GUID);
    require(n>0 && n<b.size(),"Could not resolve pinned file identity");return juce::String(b.data()).replaceCharacter('\\','/');
}
void ordinary(HANDLE h,bool directory)
{
    BY_HANDLE_FILE_INFORMATION info{};require(GetFileInformationByHandle(h,&info)!=0,"Could not inspect pinned entry");
    require(!(info.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && ((info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0)==directory,"Reparse point or unexpected entry type");
}


bool component(const juce::String& name)
{
    if(name.isEmpty() || name=="." || name==".." || name.endsWithChar('.') || name.endsWithChar(' ') || name.containsAnyOf("<>:\"/\\|?*"))return false;
    auto p=name.getCharPointer();while(!p.isEmpty()){const auto c=p.getAndAdvance();if(c<32 || (c>=0xd800 && c<=0xdfff) || c>0x10ffff)return false;}
    const auto base=name.upToFirstOccurrenceOf(".",false,false).toUpperCase();return !(base=="CON" || base=="PRN" || base=="AUX" || base=="NUL" || (base.length()==4 && (base.startsWith("COM") || base.startsWith("LPT")) && base[3]>='0' && base[3]<='9'));
}

Held relativeOpen(HANDLE parent,const juce::String& leaf,DWORD access,bool directory,ULONG disposition,DWORD sharing,bool writeThrough=false)
{
    // A single component, anchored to an already opened directory object. No
    // global path walk can follow a directory changed to a junction mid-operation.
    require(component(leaf),"Invalid handle-relative component");
    using CreateFunction=NTSTATUS(NTAPI*)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES,PIO_STATUS_BLOCK,PLARGE_INTEGER,ULONG,ULONG,ULONG,ULONG,PVOID,ULONG);
    const auto createFile=reinterpret_cast<CreateFunction>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtCreateFile"));
    require(createFile!=nullptr,"Handle-relative file operations are unavailable");
    const auto utf16=leaf.toWideCharPointer();UNICODE_STRING name{};name.Buffer=const_cast<PWSTR>(utf16);const auto units=std::wcslen(utf16);require(units<32768,"Oversized UTF-16 component");name.Length=static_cast<USHORT>(units*sizeof(wchar_t));name.MaximumLength=name.Length;
    OBJECT_ATTRIBUTES attributes{};attributes.Length=sizeof(attributes);attributes.RootDirectory=parent;attributes.ObjectName=&name;
    constexpr ULONG dontReparse=0x1000;attributes.Attributes=OBJ_CASE_INSENSITIVE|dontReparse;
    IO_STATUS_BLOCK io{};HANDLE file=INVALID_HANDLE_VALUE;
    constexpr ULONG synchronous=0x20,openReparsePoint=0x00200000,directoryFile=1,nonDirectoryFile=0x40,writeThroughOption=2;
    const auto code=createFile(&file,access|SYNCHRONIZE,&attributes,&io,nullptr,FILE_ATTRIBUTE_NORMAL,sharing,disposition,
                              synchronous|openReparsePoint|(directory?directoryFile:nonDirectoryFile)|(writeThrough?writeThroughOption:0u),nullptr,0);
    if(code<0)throw Failure(("Handle-relative open/create failed for "+leaf+" (NTSTATUS "+juce::String::toHexString(static_cast<int>(code))+")").toStdString(),code);
    auto held=std::make_shared<Handle>(file);ordinary(file,directory);return held;
}
struct Root
{
    juce::File directory;juce::String canonical,operation;
    std::map<juce::String,Held> directories;std::vector<Held> files;
    explicit Root(juce::File selected):directory(std::move(selected))
    {
        const auto full=directory.getFullPathName();
        require(full.length()>=3 && std::wcslen(full.toWideCharPointer())<=240 && juce::CharacterFunctions::isLetter(full[0]) && full[1]==':' && (full[2]=='\\' || full[2]=='/'),"Select a local absolute Windows folder; UNC/device/long paths are unsupported");
        std::vector<juce::File> ancestors;auto p=directory;for(;;){ancestors.push_back(p);const auto parent=p.getParentDirectory();if(parent==p)break;p=parent;}
        for(auto i=ancestors.rbegin();i!=ancestors.rend();++i)pinDirectory(*i,false);
        canonical=resolved(directories.at(full)->h);require(!canonical.startsWithIgnoreCase("//?/UNC/"),"Network-mapped stores are unsupported");
    }
    Held pinDirectory(const juce::File& file,bool create)
    {
        const auto key=file.getFullPathName();if(const auto found=directories.find(key);found!=directories.end()){ordinary(found->second->h,true);return found->second;}
        require(std::wcslen(key.toWideCharPointer())<=240,"Selected path exceeds the supported Windows path length");Held handle;
        const auto parent=file.getParentDirectory();
        if(parent==file){const auto drive=key.endsWithChar(':')?key+"\\":key;handle=hold(CreateFileW(drive.toWideCharPointer(),FILE_READ_ATTRIBUTES|FILE_TRAVERSE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS,nullptr),"Drive root unavailable");ordinary(handle->h,true);}
        else{const auto owner=directories.at(parent.getFullPathName());ordinary(owner->h,true);
            handle=relativeOpen(owner->h,file.getFileName(),FILE_READ_ATTRIBUTES|FILE_TRAVERSE,true,create?3:1,FILE_SHARE_READ|FILE_SHARE_WRITE);}
        directories.emplace(key,handle);return handle;
    }
    juce::File path(const juce::String& relative,bool createParents=false)
    {
        require(safeRelativePath(relative),"Unsafe relative asset path: "+relative);ordinary(directories.at(directory.getFullPathName())->h,true);
        const auto parts=juce::StringArray::fromTokens(relative,"/",{});auto parent=directory;
        for(int i=0;i<parts.size()-1;++i){parent=parent.getChildFile(parts[i]);const auto h=pinDirectory(parent,createParents);
            const auto actual=resolved(h->h);require(actual.startsWithIgnoreCase(canonical+"/"),"Folder escaped selected root");}
        auto result=parent.getChildFile(parts[parts.size()-1]);require(std::wcslen(result.getFullPathName().toWideCharPointer())<=240,"Selected path exceeds the supported Windows path length");return result;
    }
    Held parentOf(const juce::File& file){const auto parent=directories.at(file.getParentDirectory().getFullPathName());ordinary(parent->h,true);return parent;}
    Held openRead(const juce::String& relative,DWORD sharing=FILE_SHARE_READ,DWORD access=GENERIC_READ,bool writeThrough=false)
    {
        const auto file=path(relative);const auto owner=parentOf(file);
        auto held=relativeOpen(owner->h,file.getFileName(),access,false,1,sharing,writeThrough);
        require(resolved(held->h).startsWithIgnoreCase(canonical+"/"),"File escaped selected root");
        return held;
    }
    Held read(const juce::String& relative)
    {
        auto held=openRead(relative);
        files.push_back(held);
        return held;
    }
};

uint64_t size(HANDLE h){LARGE_INTEGER n{};require(GetFileSizeEx(h,&n)!=0 && n.QuadPart>=0,"Could not read file size");return static_cast<uint64_t>(n.QuadPart);}
void rewind(HANDLE h){LARGE_INTEGER n{};require(SetFilePointerEx(h,n,nullptr,FILE_BEGIN)!=0,"Could not seek pinned file");}
void checkpoint(const Hooks& hooks,const juce::String& event,const juce::String& path={})
{
    if(hooks.cancelled && hooks.cancelled())throw Cancelled("Operation cancelled; originals and previous backups remain intact");
    require(!hooks.checkpoint || hooks.checkpoint(event,path),"Injected interruption at "+event+": "+path);
}
juce::String hash(HANDLE h,const Hooks& hooks={})
{
    rewind(h);ContentHash sum;std::array<char,65536> block{};for(;;){checkpoint(hooks,"read");DWORD n=0;require(ReadFile(h,block.data(),static_cast<DWORD>(block.size()),&n,nullptr)!=0,"Could not read pinned file");if(!n)break;sum.update(block.data(),n);}return sum.finish();
}
juce::String readText(const Held& file,size_t limit=maxDocumentBytes)
{
    const auto n=size(file->h);require(n<=limit,"Oversized portable metadata");std::vector<char> bytes(static_cast<size_t>(n)+1,0);DWORD got=0;
    require(ReadFile(file->h,bytes.data(),static_cast<DWORD>(n),&got,nullptr)!=0 && got==n,"Unreadable portable metadata");const auto text=juce::String::fromUTF8(bytes.data(),static_cast<int>(n));require(text.getNumBytesAsUTF8()==n && std::memcmp(text.toRawUTF8(),bytes.data(),static_cast<size_t>(n))==0,"Invalid or NUL-containing UTF-8 metadata");return text;
}
juce::String readText(Root& root,const juce::String& relative,size_t limit=maxDocumentBytes)
{return readText(root.read(relative),limit);}

juce::String identity(HANDLE file)
{
    FILE_ID_INFO info{};
    require(GetFileInformationByHandleEx(file,FileIdInfo,&info,sizeof(info))!=0,"Cannot verify physical file identity");
    ContentHash hash;
    hash.update(&info.VolumeSerialNumber,sizeof(info.VolumeSerialNumber));
    hash.update(&info.FileId,sizeof(info.FileId));
    return hash.finish();
}
juce::String rootIdentity(const Root& root)
{return identity(root.directories.at(root.directory.getFullPathName())->h);}

void requirePrivateDataFile(HANDLE file)
{
    ordinary(file,false);
    BY_HANDLE_FILE_INFORMATION info{};
    require(GetFileInformationByHandle(file,&info)!=0 && info.nNumberOfLinks==1,"Linked or uninspectable staged file");
    alignas(FILE_STREAM_INFO) std::array<std::byte,2048> streams{};
    require(GetFileInformationByHandleEx(file,FileStreamInfo,streams.data(),static_cast<DWORD>(streams.size()))!=0,"Cannot verify staged data streams");
    const auto* stream=reinterpret_cast<const FILE_STREAM_INFO*>(streams.data());
    constexpr auto unnamed=L"::$DATA";
    require(stream->NextEntryOffset==0 && stream->StreamNameLength==7*sizeof(wchar_t)
        && std::wmemcmp(stream->StreamName,unnamed,7)==0,"Alternate or unknown staged data streams");
}
struct SealedPart {juce::String leaf,fileId;};
constexpr auto scratchRecord="scratch-cancelled.json";
constexpr int scratchVersion=1;
void verify(Root& root,const juce::String& path,const juce::String& expected,uint64_t bytes,const Hooks& hooks)
{const auto h=root.read(path);require(size(h->h)==bytes && hash(h->h,hooks)==expected,"Asset byte identity mismatch: "+path);}
NTSTATUS renameNoReplace(Root& root,HANDLE file,const juce::File& destination)
{
    const auto name=destination.getFileName();
    const auto length=static_cast<DWORD>(std::wcslen(name.toWideCharPointer())*sizeof(wchar_t));
    std::vector<char> memory(sizeof(FILE_RENAME_INFO)+length,0);
    auto* rename=reinterpret_cast<FILE_RENAME_INFO*>(memory.data());
    rename->ReplaceIfExists=FALSE;
    rename->RootDirectory=root.parentOf(destination)->h;
    rename->FileNameLength=length;
    std::memcpy(rename->FileName,name.toWideCharPointer(),length);
    using RenameFunction=NTSTATUS(NTAPI*)(HANDLE,PIO_STATUS_BLOCK,PVOID,ULONG,FILE_INFORMATION_CLASS);
    const auto renameFile=reinterpret_cast<RenameFunction>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtSetInformationFile"));
    require(renameFile!=nullptr,"Handle-relative publication is unavailable on this Windows version");
    IO_STATUS_BLOCK status{};
    return renameFile(file,&status,rename,static_cast<ULONG>(memory.size()),static_cast<FILE_INFORMATION_CLASS>(10)); // FileRenameInformation
}
void publish(Root& root,const juce::String& relative,const juce::String& expected,uint64_t bytes,
             const std::function<void(HANDLE)>& write,const Hooks& hooks,SealedPart* cancelledPart=nullptr)
{
    checkpoint(hooks,"before-file",relative);const auto destination=root.path(relative,true);
    const auto attributes=GetFileAttributesW(destination.getFullPathName().toWideCharPointer());
    if(attributes!=INVALID_FILE_ATTRIBUTES){verify(root,relative,expected,bytes,hooks);return;}
    require(GetLastError()==ERROR_FILE_NOT_FOUND,"Destination cannot be inspected");
    const auto temporary=root.operation.isNotEmpty()?root.path("transactions/"+root.operation+"/staging/part-"+juce::Uuid().toDashedString(),true):destination.getSiblingFile("part-"+juce::Uuid().toDashedString());
    auto h=relativeOpen(root.parentOf(temporary)->h,temporary.getFileName(),GENERIC_READ|GENERIC_WRITE|DELETE,false,2,FILE_SHARE_READ);
    ordinary(h->h,false);require(resolved(h->h).startsWithIgnoreCase(root.canonical+"/"),"Staged file escaped root");write(h->h);
    require(size(h->h)==bytes && hash(h->h,hooks)==expected,"Destination-only hash verification failed");
    require(FlushFileBuffers(h->h)!=0,"Could not flush destination bytes");
    try {
        if(cancelledPart)checkpoint(hooks,"sealed-payload",relative);
        checkpoint(hooks,"before-publish",relative);
    }catch(const Cancelled&){
        if(cancelledPart){
            // This catch precedes the first attempt to publish these bytes.
            // Unsupported identity evidence leaves the private file protected.
            try{requirePrivateDataFile(h->h);*cancelledPart={temporary.getFileName(),identity(h->h)};}
            catch(const Failure&){}
        }
        throw;
    }
    const auto renamed=renameNoReplace(root,h->h,destination);
    h.reset();
    // Another verified writer may have won the no-replace publication. Reuse
    // only exact bytes, with a bounded wait for its write handle to close.
    for(int attempt=0;;++attempt){try{verify(root,relative,expected,bytes,hooks);break;}catch(const Failure& e){
        if(renamed>=0 || static_cast<uint32_t>(e.status)!=0xc0000043u || attempt>=24)throw;
        checkpoint(hooks,"publication-retry",relative);juce::Thread::sleep(1);}}
    checkpoint(hooks,"after-publish",relative);
}
void writeBytes(HANDLE file,const void* data,DWORD count,uint64_t written,const Hooks& hooks)
{
    const auto injected=hooks.injectWriteError?hooks.injectWriteError(written,count):0;DWORD actual=0;
    const bool ok=injected?false:WriteFile(file,data,count,&actual,nullptr)!=0;
    const auto error=injected?injected:ok?ERROR_WRITE_FAULT:GetLastError();
    require(ok && actual==count,"Destination write failed (Windows "+juce::String(error)+"). Originals remain intact.");
}
void writeText(Root& root,const juce::String& path,const juce::String& text,const Hooks& hooks)
{
    const auto n=static_cast<uint64_t>(text.getNumBytesAsUTF8());require(n<=maxDocumentBytes,"Oversized metadata output");
    publish(root,path,digest(text),n,[&](HANDLE h){writeBytes(h,text.toRawUTF8(),static_cast<DWORD>(n),0,hooks);},hooks);
}
void copy(Root& destination,const juce::String& relative,HANDLE source,const Entry& entry,const Hooks& hooks,SealedPart* cancelledPart=nullptr)
{
    publish(destination,relative,entry.sha256,entry.bytes,[&](HANDLE h){rewind(source);std::array<char,65536> block{};uint64_t copied=0;
        while(copied<entry.bytes){checkpoint(hooks,"copy",relative);DWORD n=0;require(ReadFile(source,block.data(),static_cast<DWORD>(std::min<uint64_t>(block.size(),entry.bytes-copied)),&n,nullptr)!=0 && n>0,"Source became unreadable");
            writeBytes(h,block.data(),n,copied,hooks);copied+=n;}require(size(source)==entry.bytes,"Source changed during copy");},hooks,cancelledPart);
}
bool sameMetadata(const juce::String& bytes,const juce::String& descriptor)
{return json(boundedJson(bytes))==json(boundedJson(descriptor));}
void checkClosure(const Snapshot& s)
{
    require(s.files.size()==s.observed.dependencies.size(),"Portable dependency closure is incomplete or has extra aliases");
    for(size_t i=0;i<s.files.size();++i){const auto& file=s.files[i];const auto& d=s.observed.dependencies[i];
        require(file.path==d.path && (d.checksum.isEmpty() || d.checksum==file.sha256),"Portable alias/checksum mismatch");}
}
Receipt receiptFor(const Locator& locator,const Snapshot& s)
{
    Receipt r;r.locator=locator;r.project=s.observed.project;r.instance=s.observed.instance;r.exclusions=s.exclusions;
    r.unknownCaptureHistory=s.observed.unknownCaptureHistory;r.producer=s.producer;r.files=s.files.size();for(const auto& file:s.files)r.bytes+=file.bytes;return r;
}
juce::String decodeStoreId(const Held& metadata)
{
    const auto value=boundedJson(readText(metadata,8192));
    require(version(value,storeVersion) && only(value,{"version","storeId","reader"})
        && value["reader"].isInt() && static_cast<int>(value["reader"])==readerVersion
        && uuid(value["storeId"].toString()),"Unknown or invalid portable store version/identity");
    return value["storeId"].toString();
}
juce::String storeId(Root& root,bool create,const Hooks& hooks)
{
    auto path=root.directory.getChildFile("store.json");
    if(!path.exists() && create){juce::Array<juce::File> entries;root.directory.findChildFiles(entries,juce::File::findFilesAndDirectories,false);
        require(entries.isEmpty(),"Choose an empty folder or an existing supported asset backup");auto v=object();set(v,"version",storeVersion);set(v,"storeId",juce::Uuid().toDashedString());set(v,"reader",readerVersion);writeText(root,"store.json",json(v),hooks);}
    // All store users retain this shared open for the lifetime of their Root.
    return decodeStoreId(root.read("store.json"));
}
}
bool safeRelativePath(const juce::String& path)
{
    if(path.isEmpty() || path.length()>192 || path.containsChar('\\') || path.containsChar(':') || path.startsWithChar('/') || path.endsWithChar('/'))return false;
    const auto parts=juce::StringArray::fromTokens(path,"/",{});if(parts.size()>8)return false;
    for(const auto& part:parts){if(part.isEmpty() || part=="." || part==".." || part.endsWithChar('.') || part.endsWithChar(' ') || !part.containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_. "))return false;
        const auto base=part.upToFirstOccurrenceOf(".",false,false).toUpperCase();if(base=="CON" || base=="PRN" || base=="AUX" || base=="NUL" || (base.length()==4 && (base.startsWith("COM") || base.startsWith("LPT")) && base[3]>='0' && base[3]<='9'))return false;}
    return true;
}
juce::String encodeSnapshot(const Snapshot& s)
{
    auto v=object();set(v,"version",snapshotVersion);set(v,"processorEnvelope",s.processorXml);set(v,"captureManifest",s.manifestJson);set(v,"exclusions",s.exclusions);set(v,"producer",s.producer);
    juce::Array<juce::var> entries;for(const auto& e:s.files){auto row=object();set(row,"version",descriptorVersion);set(row,"path",e.path);set(row,"object",e.object);set(row,"sha256",e.sha256);set(row,"bytes",juce::String(e.bytes));entries.add(row);}set(v,"files",entries);return json(v);
}
bool decodeSnapshot(const juce::String& text,Snapshot& out,juce::String& error)
{
    try{require(static_cast<size_t>(text.getNumBytesAsUTF8())<=maxDocumentBytes,"Oversized portable snapshot");const auto v=boundedJson(text);require(version(v,snapshotVersion) && only(v,{"version","processorEnvelope","captureManifest","exclusions","producer","files"}),"Unsupported portable snapshot version");Snapshot s;
        require(v["processorEnvelope"].isString() && v["captureManifest"].isString() && v["exclusions"].isString(),"Missing snapshot metadata");
        s.processorXml=v["processorEnvelope"].toString();s.manifestJson=v["captureManifest"].toString();s.exclusions=v["exclusions"].toString();s.producer=v["producer"].toString();require(v["producer"].isString() && s.producer.length()<=4096 && s.exclusions==scopeExclusions,"Oversized scope note");
        require(legacy(s.processorXml,s.manifestJson,s.observed,error),error);require(uuid(s.observed.project) && uuid(s.observed.instance),"Portable snapshots require stable accepted instance IDs");
        const auto* rows=v["files"].getArray();require(rows && rows->size()<=static_cast<int>(maxFiles),"Invalid portable file list");std::set<juce::String> paths;uint64_t total=0;
        for(const auto& row:*rows){Entry e;e.path=row["path"].toString();e.object=row["object"].toString();e.sha256=row["sha256"].toString();
            require(version(row,descriptorVersion) && only(row,{"version","path","object","sha256","bytes"}) && safeRelativePath(e.path) && (e.path.endsWith(".json") || e.path.endsWith(".wav")) && hex(e.sha256) && number(row["bytes"],e.bytes) && e.bytes<=16ull*1024*1024*1024 && (!e.path.endsWith(".json") || e.bytes<=maximumMetadataBytes) && paths.insert(e.path.toLowerCase()).second,"Invalid, duplicate or unsupported portable descriptor");
            require(e.object==objectPath(e),"Invalid content-addressed object path");total+=e.bytes;require(total<=64ull*1024*1024*1024,"Portable byte limit exceeded");s.files.push_back(e);}
        checkClosure(s);s.id=digest(text);out=std::move(s);error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}
juce::String encodeReceipt(const Receipt& r)
{auto v=object();set(v,"version",receiptVersion);set(v,"storeId",r.locator.store);set(v,"snapshot",r.locator.snapshot);set(v,"projectId",r.project);set(v,"instanceId",r.instance);set(v,"files",juce::String(r.files));set(v,"bytes",juce::String(r.bytes));set(v,"exclusions",r.exclusions);set(v,"unknownCaptureHistory",r.unknownCaptureHistory);set(v,"producer",r.producer);return json(v);}
bool decodeReceipt(const juce::String& text,Receipt& out,juce::String& error)
{
    if(text.getNumBytesAsUTF8()>8192 || !jsonShape(text))return fail(error,"Oversized completion receipt");
    const auto v=juce::JSON::parse(text);Receipt r;r.locator={v["storeId"].toString(),v["snapshot"].toString()};r.project=v["projectId"].toString();r.instance=v["instanceId"].toString();r.exclusions=v["exclusions"].toString();r.producer=v["producer"].toString();r.unknownCaptureHistory=static_cast<bool>(v["unknownCaptureHistory"]);
    if(text.getNumBytesAsUTF8()>8192 || !version(v,receiptVersion) || !only(v,{"version","storeId","snapshot","projectId","instanceId","files","bytes","exclusions","unknownCaptureHistory","producer"}) || !uuid(r.locator.store) || !hex(r.locator.snapshot) || !uuid(r.project) || !uuid(r.instance) || !number(v["files"],r.files) || r.files>maxFiles || !number(v["bytes"],r.bytes) || r.bytes>64ull*1024*1024*1024 || r.exclusions!=scopeExclusions || !v["unknownCaptureHistory"].isBool() || !v["producer"].isString() || r.producer.length()>4096)return fail(error,"Invalid or unsupported completion receipt");
    out=std::move(r);error.clear();return true;
}
class Prepared
{
public:
    Snapshot state;std::shared_ptr<Root> root;std::vector<Held> files;
};
class Verified
{
public:
    Snapshot state;Receipt receipt;std::shared_ptr<Root> root;std::vector<Held> files;
};
const Snapshot& snapshot(const Prepared& p){return p.state;}
const Snapshot& snapshot(const Verified& p){return p.state;}
const Receipt& receipt(const Verified& p){return p.receipt;}
bool prepare(const Input& in,std::shared_ptr<Prepared>& out,juce::String& error,const Hooks& hooks)
{
    try{require(!in.sourceUnavailable,"Source assets are unavailable; locate or restore them before creating a complete backup");auto p=std::make_shared<Prepared>();p->state.processorXml=in.processorXml;p->state.manifestJson=in.manifestJson;p->state.exclusions=in.exclusions;p->state.producer=in.producer;
        require(legacy(in.processorXml,in.manifestJson,p->state.observed,error),error);const auto& s=p->state.observed;
        require(uuid(s.project) && uuid(s.instance) && in.instanceRoot.getFileName()==s.instance && in.instanceRoot.getParentDirectory().getFileName()==s.project,"Selected source root does not match accepted identities");
        require(s.dependencies.size()<=maxFiles,"Too many accepted dependencies");p->root=std::make_shared<Root>(in.instanceRoot);
        uint64_t total=0;for(const auto& d:s.dependencies){checkpoint(hooks,"prepare",d.path);const auto h=p->root->read(d.path);Entry e;e.path=d.path;e.bytes=size(h->h);require(e.bytes<=16ull*1024*1024*1024,"Asset exceeds portable file limit");total+=e.bytes;require(total<=64ull*1024*1024*1024 && (!e.path.endsWith(".json") || e.bytes<=maximumMetadataBytes),"Portable byte/metadata budget exceeded");e.sha256=hash(h->h,hooks);e.object=objectPath(e);
            require(d.checksum.isEmpty() || d.checksum==e.sha256,"Accepted checksum differs: "+d.path);if(d.descriptor.isNotEmpty())require(sameMetadata(readText(*p->root,d.path),d.descriptor),"Accepted descriptor differs: "+d.path);
            p->state.files.push_back(e);p->files.push_back(h);}
        Snapshot checked;require(decodeSnapshot(encodeSnapshot(p->state),checked,error),error);p->state=std::move(checked);out=std::move(p);error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}
bool prepareLegacyRecovery(const juce::File& selected,const Input& target,std::shared_ptr<Prepared>& out,juce::String& error,const Hooks& hooks)
{
    try{require(xmlShape(target.processorXml) && target.processorXml.getNumBytesAsUTF8()<=maxDocumentBytes,"Invalid saved recovery metadata");
        const auto xml=juce::XmlDocument::parse(target.processorXml);require(xml!=nullptr,"Saved recovery metadata could not be decoded");
        Input candidate=target;candidate.instanceRoot=selected;candidate.sourceUnavailable=false;
        const auto reference=xml->getStringAttribute("manifestReference");
        if(reference.isNotEmpty()){Root root(selected);candidate.manifestJson=readText(root,reference,maximumMetadataBytes);}
        return prepare(candidate,out,error,hooks);
    }catch(const std::exception& e){return fail(error,e.what());}
}
namespace
{
void recordCancelledCopy(Root& destination,const Root& source,const juce::String& store,
                         const Snapshot& snapshot,const Entry& entry,const SealedPart& part,const Hooks& hooks)
{
    auto record=object();
    set(record,"version",scratchVersion);set(record,"producer","backup-payload");set(record,"state","cancelled");
    set(record,"store",store);set(record,"operation",destination.operation);set(record,"snapshot",snapshot.id);
    set(record,"destinationRoot",rootIdentity(destination));set(record,"sourceRoot",rootIdentity(source));
    set(record,"part",part.leaf);set(record,"fileId",part.fileId);set(record,"alias",entry.path);
    set(record,"bytes",juce::String(static_cast<juce::int64>(entry.bytes)));set(record,"sha256",entry.sha256);
    auto terminalHooks=hooks;
    terminalHooks.cancelled={};
    writeText(destination,"transactions/"+destination.operation+"/"+scratchRecord,json(record),terminalHooks);
}
Result execute(const Snapshot& s,Root* source,const std::vector<Held>* pinned,Root& destination,const juce::String& store,
               const juce::String& operation,const Hooks& hooks,bool recordScratch=false)
{
    Result result;
    result.operation=operation;
    SealedPart part;
    const Entry* copying=nullptr;
    try{
        destination.operation=operation;checkClosure(s);const auto text=encodeSnapshot(s);require(digest(text)==s.id,"Snapshot changed after preparation");
        auto plan=object();set(plan,"version",journalVersion);set(plan,"operation",operation);set(plan,"storeId",store);set(plan,"snapshot",text);
        writeText(destination,"transactions/"+operation+"/plan.json",json(plan),hooks);checkpoint(hooks,"journal");
        for(size_t i=0;i<s.files.size();++i){const auto& e=s.files[i];const auto target=destination.path(e.object,true);
            if(target.exists()){verify(destination,e.object,e.sha256,e.bytes,hooks);continue;}
            require(source!=nullptr,"Originals are unavailable and a backup object is still missing");
            const auto h=pinned?(*pinned)[i]:source->read(e.path);
            copying=&e;
            copy(destination,e.object,h->h,e,hooks,recordScratch?&part:nullptr);
            checkpoint(hooks,"object",e.path);}
        // Validate destination-only closure and saved descriptor bytes before any
        // snapshot or completion receipt makes this operation visible as complete.
        for(const auto& e:s.files)verify(destination,e.object,e.sha256,e.bytes,hooks);
        for(size_t i=0;i<s.files.size();++i){const auto& d=s.observed.dependencies[i];if(d.descriptor.isNotEmpty())require(sameMetadata(readText(destination,s.files[i].object),d.descriptor),"Destination descriptor does not match saved metadata");}
        checkpoint(hooks,"closure");writeText(destination,"snapshots/"+s.id+".json",text,hooks);checkpoint(hooks,"snapshot");
        result.receipt=receiptFor({store,s.id},s);writeText(destination,"receipts/"+s.id+".json",encodeReceipt(result.receipt),hooks);result.success=true;
    }catch(const Cancelled& e){
        result.error=e.what();result.cancelled=true;
        if(recordScratch && source && copying && part.leaf.isNotEmpty()){
            // The staging handle has closed. The plan remains pinned until
            // the cancelled Backup returns, so inspection cannot race it.
            try{recordCancelledCopy(destination,*source,store,s,*copying,part,hooks);}
            catch(const std::exception&){result.error+=". Scratch ownership could not be recorded; leftovers remain protected.";}
        }
    }catch(const std::exception& e){result.error=e.what();result.cancelled=hooks.cancelled && hooks.cancelled();}
    return result;
}
Snapshot decodePlan(const juce::String& text,const juce::String& operation,const juce::String& store)
{
    const auto plan=boundedJson(text);
    require(version(plan,journalVersion) && only(plan,{"version","operation","storeId","snapshot"}) && plan["operation"].toString()==operation && plan["storeId"].toString()==store,"Invalid or conflicting interrupted operation");Snapshot s;juce::String error;require(decodeSnapshot(plan["snapshot"].toString(),s,error),error);return s;
}
Snapshot interrupted(Root& root,const juce::String& operation,juce::String& store)
{
    require(uuid(operation),"Invalid operation identity");
    store=storeId(root,false,{});
    return decodePlan(readText(root,"transactions/"+operation+"/plan.json"),operation,store);
}
Result backupPack(const std::shared_ptr<Prepared>& p,const juce::File& selected,const Hooks& hooks,bool recordScratch)
{
    Result result;try{require(p!=nullptr,"No accepted snapshot was prepared");checkpoint(hooks,"start");Root destination(selected);
        require(!destination.canonical.equalsIgnoreCase(p->root->canonical) && !destination.canonical.startsWithIgnoreCase(p->root->canonical+"/") && !p->root->canonical.startsWithIgnoreCase(destination.canonical+"/"),"Backup destination must be separate from the working instance tree");
        ULARGE_INTEGER free{};require(GetDiskFreeSpaceExW(selected.getFullPathName().toWideCharPointer(),&free,nullptr,nullptr)!=0,"Could not measure destination free space");uint64_t total=0;for(const auto& e:p->state.files)total+=e.bytes;
        require(free.QuadPart>=total*2+4*encodeSnapshot(p->state).getNumBytesAsUTF8()+8192,"Insufficient free space for verified backup and staging");
        const auto store=storeId(destination,true,hooks);Root source(p->root->directory);return execute(p->state,&source,nullptr,destination,store,juce::Uuid().toDashedString(),hooks,recordScratch);
    }catch(const std::exception& e){result.error=e.what();result.cancelled=hooks.cancelled && hooks.cancelled();return result;}
}
}
Result backup(const std::shared_ptr<Prepared>& p,const juce::File& selected,const Hooks& hooks)
{return backupPack(p,selected,hooks,true);}
bool inspectInterrupted(const juce::File& selected,const juce::String& operation,Snapshot& out,juce::String& error)
{try{Root root(selected);juce::String store;auto s=interrupted(root,operation,store);out=std::move(s);error.clear();return true;}catch(const std::exception& e){return fail(error,e.what());}}
Result resume(const juce::File& selected,const juce::String& operation,const juce::File& explicitSource,const Hooks& hooks)
{
    Result result;result.operation=operation;try{Root destination(selected);juce::String store;const auto s=interrupted(destination,operation,store);std::unique_ptr<Root> source;
        if(explicitSource!=juce::File{}){require(explicitSource.getFileName()==s.observed.instance && explicitSource.getParentDirectory().getFileName()==s.observed.project,"Resume source identity differs");source=std::make_unique<Root>(explicitSource);}
        return execute(s,source.get(),nullptr,destination,store,operation,hooks);
    }catch(const std::exception& e){result.error=e.what();return result;}
}
namespace
{
constexpr auto missingName=0xc0000034u;
constexpr uint64_t previewByteLimit=64ull*1024*1024*1024;
constexpr size_t previewOperationLimit=256;
struct PreviewLimit final:Failure {using Failure::Failure;};

juce::StringArray previewOperations(Root& root,const Hooks& hooks)
{
    Held directory;
    try{
        const auto parent=root.directories.at(root.directory.getFullPathName());
        directory=relativeOpen(parent->h,"transactions",FILE_LIST_DIRECTORY|FILE_READ_ATTRIBUTES,true,1,FILE_SHARE_READ|FILE_SHARE_WRITE);
    }catch(const Failure& e){
        if(static_cast<uint32_t>(e.status)==missingName)return {};
        throw;
    }
    require(resolved(directory->h).equalsIgnoreCase(root.canonical+"/transactions"),"Transaction directory escaped selected store");
    juce::StringArray operations;
    size_t visited=0;
    alignas(FILE_ID_BOTH_DIR_INFO) std::array<std::byte,16384> buffer{};
    auto infoClass=FileIdBothDirectoryRestartInfo;
    for(;;){
        checkpoint(hooks,"preview-directory");
        if(!GetFileInformationByHandleEx(directory->h,infoClass,buffer.data(),static_cast<DWORD>(buffer.size()))){
            require(GetLastError()==ERROR_NO_MORE_FILES,"Cannot enumerate pinned transaction directory");
            break;
        }
        infoClass=FileIdBothDirectoryInfo;
        size_t offset=0;
        for(;;){
            constexpr auto nameOffset=offsetof(FILE_ID_BOTH_DIR_INFO,FileName);
            require(offset<=buffer.size()-nameOffset,"Invalid directory record");
            const auto* item=reinterpret_cast<const FILE_ID_BOTH_DIR_INFO*>(buffer.data()+offset);
            require(item->FileNameLength%sizeof(wchar_t)==0 && item->FileNameLength<=buffer.size()-offset-nameOffset,"Invalid directory entry name");
            const juce::String name(item->FileName,static_cast<size_t>(item->FileNameLength/sizeof(wchar_t)));
            if(name!="." && name!=".."){
                require(++visited<=previewOperationLimit,"Transaction preview limit reached; results are incomplete");
                if(uuid(name))operations.add(name);
            }
            if(!item->NextEntryOffset)break;
            require(item->NextEntryOffset>=nameOffset+item->FileNameLength && item->NextEntryOffset<=buffer.size()-offset
                && item->NextEntryOffset%alignof(FILE_ID_BOTH_DIR_INFO)==0,"Invalid next directory record");
            offset+=item->NextEntryOffset;
        }
    }
    operations.sort(false);
    return operations;
}

void requireSeparateRoots(const Root& destination,const Root& source)
{
    require(!destination.canonical.equalsIgnoreCase(source.canonical)
        && !destination.canonical.startsWithIgnoreCase(source.canonical+"/")
        && !source.canonical.startsWithIgnoreCase(destination.canonical+"/"),"Backup and original instance folders must be separate");
}

void requireNoRecoveryReference(Root& destination,const juce::String& snapshotId)
{
    // Valid pack objects can only live in objects/ or descriptors/. Working
    // and recovery aliases have separate instance roots. A snapshot or receipt
    // for this exact plan is still a conservative veto, including a snapshot
    // published before its receipt. Inaccessible metadata is also a veto.
    for(const auto* name:{"snapshots/","receipts/"}){
        try{destination.openRead(juce::String(name)+snapshotId+".json");}
        catch(const Failure& e){if(static_cast<uint32_t>(e.status)==missingName)continue;throw;}
        throw Failure("The Backup plan has a snapshot or receipt. Keep its recovery data.");
    }
}

struct ScratchProof
{
    juce::String path,alias,sourceRoot,project,instance,snapshot,fileId,sha,ownership;
    uint64_t bytes=0;
    Held record,plan;
};

ScratchProof readScratchProof(Root& destination,const juce::String& store,const juce::String& operation)
{
    const auto prefix="transactions/"+operation+"/";
    juce::var record;
    Held proof;
    juce::String recordText;
    try{
        proof=destination.openRead(prefix+scratchRecord);
        recordText=readText(proof,8192);
        record=boundedJson(recordText);
    }
    catch(const Failure& e){
        if(static_cast<uint32_t>(e.status)==missingName)throw Failure("No terminal scratch record. Existing plans and unknown leftovers are protected.");
        throw;
    }
    require(version(record,scratchVersion) && only(record,{"version","producer","state","store","operation","snapshot",
        "destinationRoot","sourceRoot","part","fileId","alias","bytes","sha256"})
        && record["producer"].toString()=="backup-payload" && record["state"].toString()=="cancelled"
        && record["store"].toString()==store && record["operation"].toString()==operation
        && record["destinationRoot"].toString()==rootIdentity(destination),"Unsupported or conflicting scratch ownership record");
    const auto sourceRoot=record["sourceRoot"].toString();
    require(hex(sourceRoot),"Invalid original folder identity");
    const auto part=record["part"].toString(),fileId=record["fileId"].toString(),sha=record["sha256"].toString();
    uint64_t bytes=0;
    require(part.startsWith("part-") && uuid(part.substring(5)) && hex(fileId) && hex(sha) && number(record["bytes"],bytes),"Invalid staged identity or byte proof");

    // Resume pins the existing plan for its full execution, including with the
    // legacy reader. Exclusive read access detects a busy operation without
    // creating a lease file or enabling any maintenance action.
    const auto plan=destination.openRead(prefix+"plan.json",0);
    const auto planText=readText(plan);
    const auto accepted=decodePlan(planText,operation,store);
    require(accepted.id==record["snapshot"].toString(),"Scratch snapshot does not match the Backup plan");
    const auto alias=record["alias"].toString();
    const auto entry=std::find_if(accepted.files.begin(),accepted.files.end(),[&](const auto& file){return file.path==alias;});
    require(entry!=accepted.files.end() && entry->sha256==sha && entry->bytes==bytes,"Staged bytes do not match an accepted Backup dependency");
    const auto ownership=digest(identity(proof->h)+digest(recordText)+identity(plan->h)+digest(planText));
    return {prefix+"staging/"+part,alias,sourceRoot,accepted.observed.project,accepted.observed.instance,
            accepted.id,fileId,sha,ownership,bytes,proof,plan};
}

Held existingFile(Root& root,const juce::String& path,DWORD access=GENERIC_READ,bool durable=false)
{
    try{return root.openRead(path,0,access,durable);}
    catch(const Failure& e){if(static_cast<uint32_t>(e.status)==missingName)return {};throw;}
}

void requireNoQuarantineHistory(Root& destination,const juce::String& operation)
{
    const auto prefix="quarantine/"+operation+"/";
    require(!existingFile(destination,prefix+"undo.json") && !existingFile(destination,prefix+"restored.json")
        && !existingFile(destination,prefix+"moved.json") && !existingFile(destination,prefix+"remove.json")
        && !existingFile(destination,prefix+"removed.json"),"This copy already has a quarantine, undo or removal history");
}

struct ScratchInspection
{
    ScratchPreviewEntry entry;
    ScratchProof proof;
    Held staged,original;
};

ScratchInspection inspectCancelledCopy(Root& destination,Root& source,const juce::String& store,
                                       const juce::String& operation,uint64_t& budget,const Hooks& hooks,
                                       const ScratchPreviewEntry* reviewed=nullptr)
{
    requireNoQuarantineHistory(destination,operation);
    auto proof=readScratchProof(destination,store,operation);
    require(proof.sourceRoot==rootIdentity(source) && source.directory.getFileName()==proof.instance
        && source.directory.getParentDirectory().getFileName()==proof.project,"Select the original instance folder for this Backup to verify another copy");
    requireNoRecoveryReference(destination,proof.snapshot);
    const auto bytes=proof.bytes;
    if(bytes>budget/2)throw PreviewLimit("Preview byte verification limit reached; results are incomplete");
    budget-=bytes*2;
    const auto& path=proof.path;
    require(!reviewed || (reviewed->path==path && reviewed->bytes==bytes),"Reviewed path or byte count changed. Preview again.");
    const auto staged=destination.openRead(path,0,reviewed?GENERIC_READ|DELETE:GENERIC_READ,reviewed!=nullptr);
    requirePrivateDataFile(staged->h);
    require(identity(staged->h)==proof.fileId && size(staged->h)==bytes && hash(staged->h,hooks)==proof.sha,"Staged file changed since the cancelled copy");
    const auto original=source.openRead(proof.alias);
    const auto originalId=identity(original->h);
    require(originalId!=proof.fileId && size(original->h)==bytes && hash(original->h,hooks)==proof.sha,"No distinct verified surviving copy");

    checkpoint(hooks,"scratch-evidence-checked",path);
    requireNoRecoveryReference(destination,proof.snapshot);
    require(resolved(staged->h).equalsIgnoreCase(destination.canonical+"/"+path)
        && resolved(original->h).equalsIgnoreCase(source.canonical+"/"+proof.alias),"Pinned staged or original path changed");
    const auto evidence=digest(digest(destination.canonical)+digest(source.canonical)+proof.ownership+proof.fileId+originalId);
    require(!reviewed || reviewed->evidence==evidence,"Reviewed ownership or surviving copy changed. Preview again.");
    return {{operation,path,"Verified cancelled Backup copy with an intact original",bytes,evidence},std::move(proof),staged,original};
}
}

ScratchPreview previewScratch(const juce::File& selectedStore,const juce::File& originalInstance,const Hooks& hooks)
{
    ScratchPreview report;
    try{
        checkpoint(hooks,"preview-start");
        Root destination(selectedStore),source(originalInstance);
        requireSeparateRoots(destination,source);
        report.store=storeId(destination,false,hooks);
        auto budget=previewByteLimit;
        for(const auto& operation:previewOperations(destination,hooks)){
            checkpoint(hooks,"preview-operation",operation);
            try{
                auto entry=inspectCancelledCopy(destination,source,report.store,operation,budget,hooks).entry;
                report.bytes+=entry.bytes;
                report.eligible.push_back(std::move(entry));
            }catch(const Cancelled&){throw;}
            catch(const PreviewLimit&){throw;}
            catch(const std::exception& e){report.excluded.push_back({operation,"transactions/"+operation,e.what(),0});}
        }
        checkpoint(hooks,"preview-complete");
        report.complete=true;
    }catch(const Cancelled& e){report.cancelled=true;report.error=e.what();}
    catch(const std::exception& e){report.error=e.what();}
    if(!report.complete){report.eligible.clear();report.bytes=0;}
    return report;
}

// The scope keeps the exact handles alive through the synchronous caller.
// A later quarantine implementation must use the staged handle and pinned
// parent, rather than reconstructing and reopening a path from the preview.
namespace
{
Held reserveScratchStore(Root& destination)
{
    Held lease;
    try{lease=destination.openRead("store.json",0);}
    catch(const Failure& e){throw Failure("Cannot reserve the backup store for maintenance. Keep its files and retry when idle. "+std::string(e.what()),e.status);}
    requirePrivateDataFile(lease->h);
    return lease;
}
}

class ScratchMaintenance
{
public:
    Root& destination;
    Root& source;
    Held lease;
    ScratchInspection pinned;
};

bool withRevalidatedScratch(const juce::File& selectedStore,const juce::File& originalInstance,
                           const ScratchPreview& reviewed,const juce::String& selectedPath,
                           const std::function<void(const ScratchMaintenance&)>& ready,
                           juce::String& error,const Hooks& hooks)
{
    try{
        require(reviewed.complete && !reviewed.cancelled && uuid(reviewed.store) && ready,"A complete reviewed preview is required");
        const auto entry=std::find_if(reviewed.eligible.begin(),reviewed.eligible.end(),[&](const auto& item){return item.path==selectedPath;});
        require(entry!=reviewed.eligible.end() && uuid(entry->operation) && hex(entry->evidence),"The file was not in the reviewed preview");
        checkpoint(hooks,"maintenance-start");
        Root destination(selectedStore),source(originalInstance);
        requireSeparateRoots(destination,source);

        const auto lease=reserveScratchStore(destination);
        require(decodeStoreId(lease)==reviewed.store,"Reviewed backup store changed");
        checkpoint(hooks,"maintenance-locked");
        auto budget=previewByteLimit;
        ScratchMaintenance scope{destination,source,lease,
            inspectCancelledCopy(destination,source,reviewed.store,entry->operation,budget,hooks,&*entry)};
        if(hooks.cancelled && hooks.cancelled())throw Cancelled("Maintenance revalidation cancelled");
        ready(scope);
        error.clear();
        return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}

namespace
{
struct QuarantinePaths
{
    explicit QuarantinePaths(const juce::String& operation):prefix("quarantine/"+operation+"/"){}
    juce::String prefix;
    juce::String payload()const{return prefix+"payload";}
    juce::String move()const{return prefix+"move.json";}
    juce::String moved()const{return prefix+"moved.json";}
    juce::String undo()const{return prefix+"undo.json";}
    juce::String restored()const{return prefix+"restored.json";}
    juce::String remove()const{return prefix+"remove.json";}
    juce::String removed()const{return prefix+"removed.json";}
};

void requireNtfs(HANDLE lease)
{
    wchar_t system[16]{};
    require(GetVolumeInformationByHandleW(lease,nullptr,0,nullptr,nullptr,nullptr,system,16)!=0
        && juce::String(system).equalsIgnoreCase("NTFS"),"Quarantine requires a local NTFS store");
}

juce::String moveIntent(Root& root,const juce::String& store,const juce::String& operation,
                        const ScratchProof& proof,const QuarantinePaths& paths)
{
    const auto originalParent=root.parentOf(root.path(proof.path));
    const auto quarantineParent=root.parentOf(root.path(paths.payload()));
    auto value=object();
    set(value,"version",1);
    set(value,"store",store);
    set(value,"operation",operation);
    set(value,"rootId",rootIdentity(root));
    set(value,"ownership",proof.ownership);
    set(value,"original",proof.path);
    set(value,"originalParent",identity(originalParent->h));
    set(value,"quarantine",paths.payload());
    set(value,"quarantineParent",identity(quarantineParent->h));
    set(value,"fileId",proof.fileId);
    set(value,"sha256",proof.sha);
    set(value,"bytes",juce::String(static_cast<juce::int64>(proof.bytes)));
    return json(value);
}

juce::String phaseRecord(const juce::String& intent,const juce::String& phase)
{
    auto value=object();
    set(value,"version",1);set(value,"intent",digest(intent));set(value,"phase",phase);
    return json(value);
}

void checkRecord(const Held& file,const juce::String& expected)
{
    requirePrivateDataFile(file->h);
    require(readText(file,8192)==expected,"Quarantine journal conflicts with the exact operation. Files retained.");
}

Held writeJournalRecord(Root& root,const juce::String& relative,const juce::String& text,
                        const juce::String& boundary,const Hooks& hooks)
{
    checkpoint(hooks,boundary+"-before",relative);
    const auto target=root.path(relative);
    if(auto existing=existingFile(root,relative,GENERIC_READ|GENERIC_WRITE,true)){
        checkRecord(existing,text);
        require(FlushFileBuffers(existing->h)!=0,"Cannot flush existing quarantine record");
        checkpoint(hooks,boundary+"-after",relative);
        return existing;
    }
    const auto parent=root.parentOf(target);
    const auto temporary="journal-"+juce::Uuid().toDashedString();
    auto staged=relativeOpen(parent->h,temporary,GENERIC_READ|GENERIC_WRITE|DELETE,false,2,0,true);
    writeBytes(staged->h,text.toRawUTF8(),static_cast<DWORD>(text.getNumBytesAsUTF8()),0,hooks);
    require(FlushFileBuffers(staged->h)!=0,"Cannot flush quarantine record bytes");
    rewind(staged->h);
    checkRecord(staged,text);
    checkpoint(hooks,boundary+"-sealed",relative);
    require(renameNoReplace(root,staged->h,target)>=0,"Cannot publish quarantine record without overwrite");
    checkpoint(hooks,boundary+"-published",relative);
    require(FlushFileBuffers(staged->h)!=0,"Cannot flush published quarantine record");
    checkpoint(hooks,boundary+"-after",relative);
    return staged;
}

void verifyQuarantineFile(Root& root,const Held& file,const juce::String& path,const ScratchProof& proof,const Hooks& hooks)
{
    requirePrivateDataFile(file->h);
    require(resolved(file->h).equalsIgnoreCase(root.canonical+"/"+path)
        && identity(file->h)==proof.fileId && size(file->h)==proof.bytes && hash(file->h,hooks)==proof.sha,
        "Quarantine path, file identity or hash differs. Files retained.");
}

void moveQuarantineFile(Root& root,const Held& file,const juce::String& to)
{
    const auto target=root.path(to);
    const auto parent=root.parentOf(target);
    const auto parentPath=resolved(parent->h);
    require(parentPath.startsWithIgnoreCase(root.canonical+"/"),"Quarantine target escaped the pinned store");
    // All ancestors are opened without reparse traversal. The volume GUID
    // also has to agree. The native rename has no replace or copy fallback.
    const auto current=resolved(file->h);
    require(current.startsWithIgnoreCase(root.canonical+"/")
        && current.upToFirstOccurrenceOf("}/",true,false).equalsIgnoreCase(parentPath.upToFirstOccurrenceOf("}/",true,false)),
        "Quarantine move must stay on the pinned volume");
    require(renameNoReplace(root,file->h,target)>=0,"Quarantine destination is occupied or unavailable. Files retained.");
}

struct QuarantineObservation
{
    ScratchQuarantineInfo info;
    ScratchProof proof;
    juce::String intent;
    Held record,moved,undo,restored,remove,removed,original,payload;
};

QuarantineObservation observeQuarantine(Root& root,const juce::String& store,const juce::String& operation,
                                        const QuarantinePaths& paths,bool forMutation,const Hooks& hooks)
{
    QuarantineObservation observed;
    auto& info=observed.info;
    info.operation=operation;
    info.quarantinePath=paths.payload();
    observed.record=existingFile(root,paths.move());
    if(!observed.record){
        require(!existingFile(root,paths.payload()) && !existingFile(root,paths.moved())
            && !existingFile(root,paths.undo()) && !existingFile(root,paths.restored())
            && !existingFile(root,paths.remove()) && !existingFile(root,paths.removed()),
            "Quarantine data has no verified move journal. Files retained.");
        info.error="No committed quarantine journal for this operation";
        return observed;
    }
    observed.proof=readScratchProof(root,store,operation);
    const auto& proof=observed.proof;
    info.originalPath=proof.path;
    info.bytes=proof.bytes;
    observed.intent=moveIntent(root,store,operation,proof,paths);
    checkRecord(observed.record,observed.intent);
    observed.moved=existingFile(root,paths.moved());
    observed.undo=existingFile(root,paths.undo());
    observed.restored=existingFile(root,paths.restored());
    if(observed.moved)checkRecord(observed.moved,phaseRecord(observed.intent,"moved"));
    if(observed.undo)checkRecord(observed.undo,phaseRecord(observed.intent,"undo"));
    if(observed.restored)checkRecord(observed.restored,phaseRecord(observed.intent,"restored"));

    observed.original=existingFile(root,proof.path);
    observed.payload=existingFile(root,paths.payload(),forMutation?GENERIC_READ|DELETE:GENERIC_READ,forMutation);
    observed.remove=existingFile(root,paths.remove());
    observed.removed=existingFile(root,paths.removed());
    if(observed.remove)checkRecord(observed.remove,phaseRecord(observed.intent,"remove"));
    if(observed.removed){
        checkRecord(observed.removed,phaseRecord(observed.intent,"removed"));
        require(observed.remove && observed.moved && !observed.original && !observed.payload
            && !observed.undo && !observed.restored,"Removal journal conflicts with retained paths. Files retained.");
        info.state=ScratchQuarantineState::removed;
        return observed;
    }
    require(static_cast<bool>(observed.original)!=static_cast<bool>(observed.payload),"Both or neither quarantine locations exist. Inspect the journal before taking action.");
    if(observed.original){
        verifyQuarantineFile(root,observed.original,proof.path,proof,hooks);
        require((!observed.moved && !observed.restored) || observed.undo,"Original staging file conflicts with quarantine journal. Files retained.");
        info.state=observed.undo?ScratchQuarantineState::restored:ScratchQuarantineState::staged;
    }else{
        verifyQuarantineFile(root,observed.payload,paths.payload(),proof,hooks);
        require(!observed.restored,"Quarantine payload conflicts with restored journal. Files retained.");
        info.state=ScratchQuarantineState::quarantined;
        info.undoPending=static_cast<bool>(observed.undo);
    }
    return observed;
}
}

bool quarantineScratch(const juce::File& selectedStore,const juce::File& originalInstance,
                       const ScratchPreview& reviewed,const juce::String& selectedPath,
                       juce::String& error,const Hooks& hooks)
{
    return withRevalidatedScratch(selectedStore,originalInstance,reviewed,selectedPath,[&](const ScratchMaintenance& scope){
        auto& root=scope.destination;
        const auto& proof=scope.pinned.proof;
        const auto& operation=scope.pinned.entry.operation;
        const QuarantinePaths paths(operation);
        requireNtfs(scope.lease->h);
        checkpoint(hooks,"quarantine-before-directory");
        root.path(paths.move(),true);
        require(!existingFile(root,paths.payload()),"Quarantine destination already exists. Files retained.");
        const auto intent=moveIntent(root,reviewed.store,operation,proof,paths);
        const auto record=writeJournalRecord(root,paths.move(),intent,"quarantine-intent",hooks);
        checkpoint(hooks,"quarantine-before-move",proof.path);
        verifyQuarantineFile(root,scope.pinned.staged,proof.path,proof,hooks);
        requireNoRecoveryReference(root,proof.snapshot);
        requireNoQuarantineHistory(root,operation);
        moveQuarantineFile(root,scope.pinned.staged,paths.payload());
        checkpoint(hooks,"quarantine-after-move",paths.payload());
        verifyQuarantineFile(root,scope.pinned.staged,paths.payload(),proof,hooks);
        writeJournalRecord(root,paths.moved(),phaseRecord(intent,"moved"),"quarantine-complete",hooks);
    },error,hooks);
}

ScratchQuarantineInfo inspectScratchQuarantine(const juce::File& selectedStore,const juce::String& operation,const Hooks& hooks)
{
    try{
        require(uuid(operation),"Select an exact quarantine operation");
        checkpoint(hooks,"quarantine-inspect");
        Root root(selectedStore);
        const auto lease=reserveScratchStore(root);
        return observeQuarantine(root,decodeStoreId(lease),operation,QuarantinePaths(operation),false,hooks).info;
    }catch(const std::exception& e){
        ScratchQuarantineInfo result;
        result.state=ScratchQuarantineState::ambiguous;
        result.operation=operation;
        result.error=e.what();
        return result;
    }
}

bool undoScratchQuarantine(const juce::File& selectedStore,const juce::String& operation,juce::String& error,const Hooks& hooks)
{
    try{
        require(uuid(operation),"Select an exact quarantine operation to undo");
        checkpoint(hooks,"undo-start");
        Root root(selectedStore);
        const auto lease=reserveScratchStore(root);
        requireNtfs(lease->h);
        const QuarantinePaths paths(operation);
        const auto observed=observeQuarantine(root,decodeStoreId(lease),operation,paths,true,hooks);
        const auto& proof=observed.proof;
        const auto& intent=observed.intent;
        if(observed.info.state==ScratchQuarantineState::restored){
            if(!observed.restored)writeJournalRecord(root,paths.restored(),phaseRecord(intent,"restored"),"undo-complete",hooks);
            error.clear();
            return true;
        }
        require(observed.info.state==ScratchQuarantineState::quarantined,"No verified quarantined file to undo");
        const auto undo=observed.undo?observed.undo:writeJournalRecord(root,paths.undo(),phaseRecord(intent,"undo"),"undo-intent",hooks);
        checkpoint(hooks,"undo-before-move",paths.payload());
        verifyQuarantineFile(root,observed.payload,paths.payload(),proof,hooks);
        require(!existingFile(root,paths.restored()) && !existingFile(root,paths.removed()),
            "Undo or removal history changed. Files retained.");
        moveQuarantineFile(root,observed.payload,proof.path);
        checkpoint(hooks,"undo-after-move",proof.path);
        verifyQuarantineFile(root,observed.payload,proof.path,proof,hooks);
        writeJournalRecord(root,paths.restored(),phaseRecord(intent,"restored"),"undo-complete",hooks);
        error.clear();
        return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}

namespace
{
juce::int64 removalDeadline(const Held& moved)
{
    constexpr juce::int64 graceMs=30LL*24*60*60*1000;
    constexpr uint64_t windowsEpochMs=11644473600000ULL;
    FILETIME creation{},lastWrite{};
    require(moved && GetFileTime(moved->h,&creation,nullptr,&lastWrite)!=0,
        "Verified quarantine completion time is unavailable. Keep this file or undo quarantine.");
    const auto ticks=[](FILETIME time){return (uint64_t{time.dwHighDateTime}<<32)|time.dwLowDateTime;};
    // Round up so a sub-millisecond timestamp cannot shorten the grace period.
    const auto time=ticks(creation)>ticks(lastWrite)?ticks(creation):ticks(lastWrite);
    const auto ms=time/10000+(time%10000!=0);
    require(ms>windowsEpochMs,"Invalid quarantine completion time. Files retained.");
    return static_cast<juce::int64>(ms-windowsEpochMs)+graceMs;
}

ScratchRemovalReview verifyRemoval(Root& root,Root& source,const QuarantinePaths& paths,
                                   const QuarantineObservation& observed,const Hooks& hooks,Held& surviving)
{
    require(observed.info.state==ScratchQuarantineState::quarantined && !observed.undo,
        "Only verified quarantine without pending undo can be reviewed for removal.");
    const auto deadline=removalDeadline(observed.moved);
    require(juce::Time::currentTimeMillis()>=deadline,"The 30-day quarantine grace period has not elapsed.");
    const auto& proof=observed.proof;
    require(proof.sourceRoot==rootIdentity(source) && source.directory.getFileName()==proof.instance
        && source.directory.getParentDirectory().getFileName()==proof.project,
        "Select the original instance folder for this Backup to verify the surviving copy.");
    requireNoRecoveryReference(root,proof.snapshot);
    surviving=source.openRead(proof.alias);
    const auto survivingId=identity(surviving->h);
    require(survivingId!=proof.fileId && size(surviving->h)==proof.bytes && hash(surviving->h,hooks)==proof.sha
        && resolved(surviving->h).equalsIgnoreCase(source.canonical+"/"+proof.alias),"No distinct verified surviving copy.");
    const auto evidence=digest(observed.intent+identity(observed.moved->h)+survivingId);
    return {observed.info.operation,paths.payload(),evidence,proof.bytes,deadline};
}
}

bool reviewScratchRemoval(const juce::File& selectedStore,const juce::File& originalInstance,
                          const juce::String& operation,ScratchRemovalReview& review,juce::String& error,const Hooks& hooks)
{
    review={};
    try{
        require(uuid(operation),"Select an exact quarantine operation to review.");
        Root root(selectedStore),source(originalInstance);
        requireSeparateRoots(root,source);
        const auto lease=reserveScratchStore(root);
        requireNtfs(lease->h);
        const QuarantinePaths paths(operation);
        const auto observed=observeQuarantine(root,decodeStoreId(lease),operation,paths,false,hooks);
        Held surviving;
        review=verifyRemoval(root,source,paths,observed,hooks,surviving);
        error.clear();
        return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}

bool removeReviewedScratch(const juce::File& selectedStore,const juce::File& originalInstance,
                           const ScratchRemovalReview& review,juce::String& error,const Hooks& hooks)
{
    try{
        require(uuid(review.operation) && hex(review.evidence),"Review an exact quarantined file before approving removal.");
        Root root(selectedStore),source(originalInstance);
        requireSeparateRoots(root,source);
        const auto lease=reserveScratchStore(root);
        requireNtfs(lease->h);
        const QuarantinePaths paths(review.operation);
        auto observed=observeQuarantine(root,decodeStoreId(lease),review.operation,paths,true,hooks);
        Held surviving;
        const auto fresh=verifyRemoval(root,source,paths,observed,hooks,surviving);
        require(fresh==review,"Removal evidence changed. Review the exact file again.");

        const auto intent=observed.remove?observed.remove:writeJournalRecord(
            root,paths.remove(),phaseRecord(observed.intent,"remove"),"removal-intent",hooks);
        checkpoint(hooks,"removal-before-delete",paths.payload());
        verifyQuarantineFile(root,observed.payload,paths.payload(),observed.proof,hooks);
        requireNoRecoveryReference(root,observed.proof.snapshot);
        require(!existingFile(root,observed.proof.path) && !existingFile(root,paths.undo())
            && !existingFile(root,paths.restored()) && !existingFile(root,paths.removed()),
            "Quarantine state changed after review. Files retained.");
        require(removalDeadline(observed.moved)==review.eligibleAfterUtcMs
            && juce::Time::currentTimeMillis()>=review.eligibleAfterUtcMs,"Quarantine time changed. Files retained.");

        FILE_DISPOSITION_INFO disposition{TRUE};
        require(SetFileInformationByHandle(observed.payload->h,FileDispositionInfo,&disposition,sizeof(disposition))!=0,
            "Could not remove the pinned quarantine payload. Files retained.");
        observed.payload.reset();
        checkpoint(hooks,"removal-after-delete",paths.payload());
        require(!existingFile(root,paths.payload()) && !existingFile(root,observed.proof.path),
            "Quarantine paths changed during removal. Inspect the journal.");
        writeJournalRecord(root,paths.removed(),phaseRecord(observed.intent,"removed"),"removal-complete",hooks);
        error.clear();
        return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}

Result recoverCaptureSnapshotCopy(const juce::File& originalInstance,const CaptureRecoveryCandidate& candidate,
                                  const juce::File& selectedProjectsRoot,const Hooks& hooks)
{
    Result result;
    try{
        require(candidate.unavailable.isEmpty() && hex(candidate.sha256)
            && candidate.reference.startsWith("manifests/") && candidate.reference.endsWith(".json")
            && !candidate.reference.substring(10).containsChar('/'),"Select an exact reviewed capture snapshot.");
        Root source(originalInstance),target(selectedProjectsRoot);
        requireSeparateRoots(target,source);
        ObservedSnapshot observed;
        observed.reference=candidate.reference;
        const auto text=readText(source,observed.reference,maximumMetadataBytes);
        juce::String error;
        require(digest(text)==candidate.sha256 && jsonShape(text) && parseCaptureManifest(text,observed.capture,error),
            "Capture snapshot changed or is unreadable: "+error);
        const juce::String project(observed.capture.projectId),instance(observed.capture.instanceId);
        require(uuid(project) && uuid(instance) && originalInstance.getFileName()==instance
            && originalInstance.getParentDirectory().getFileName()==project
            && observed.capture.revision==candidate.revision,"Capture snapshot does not match the reviewed instance or revision.");
        require(visitDependencies(observed,error) && observed.dependencies.size()<=maxFiles,error);

        std::vector<std::pair<Entry,Held>> dependencies;
        uint64_t total=0;
        for(const auto& dependency:observed.dependencies){
            if(dependency.path==candidate.reference)continue;
            const auto file=source.openRead(dependency.path);
            const auto bytes=size(file->h);
            require(bytes<=16ull*1024*1024*1024 && (total+=bytes)<=64ull*1024*1024*1024,
                "Capture recovery byte limit exceeded.");
            const auto sha=hash(file->h,hooks);
            require(dependency.checksum.isEmpty() || dependency.checksum==sha,
                "Capture dependency changed: "+dependency.path);
            if(dependency.descriptor.isNotEmpty()){
                rewind(file->h);
                require(sameMetadata(readText(file,maximumMetadataBytes),dependency.descriptor),
                    "Capture descriptor changed: "+dependency.path);
            }
            dependencies.emplace_back(Entry{dependency.path,{},sha,bytes},file);
        }
        const auto prefix=project+"/"+instance+"/";
        for(const auto& [entry,file]:dependencies)copy(target,prefix+entry.path,file->h,entry,hooks);
        for(const auto& [entry,file]:dependencies)verify(target,prefix+entry.path,entry.sha256,entry.bytes,hooks);
        writeText(target,prefix+observed.reference,text,hooks);
        result.success=true;
    }catch(const Cancelled& e){result.cancelled=true;result.error=e.what();}
    catch(const std::exception& e){result.error=e.what();}
    return result;
}

bool locate(const juce::File& selected,const Locator& expected,std::shared_ptr<Verified>& out,juce::String& error,const Hooks& hooks)
{
    try{require(uuid(expected.store) && hex(expected.snapshot),"Select an exact store and immutable snapshot; latest-head inference is unsupported");auto v=std::make_shared<Verified>();v->root=std::make_shared<Root>(selected);
        require(storeId(*v->root,false,hooks)==expected.store,"Selected folder has a different store identity");
        require(decodeReceipt(readText(*v->root,"receipts/"+expected.snapshot+".json",8192),v->receipt,error),error);
        const auto text=readText(*v->root,"snapshots/"+expected.snapshot+".json");require(digest(text)==expected.snapshot && decodeSnapshot(text,v->state,error),"Snapshot identity/version mismatch: "+error);
        require(encodeReceipt(v->receipt)==encodeReceipt(receiptFor(expected,v->state)),"Completion receipt does not match accepted snapshot");
        for(size_t i=0;i<v->state.files.size();++i){const auto& e=v->state.files[i];const auto h=v->root->read(e.object);require(size(h->h)==e.bytes && hash(h->h,hooks)==e.sha256,"Missing or corrupt backup object: "+e.path);v->files.push_back(h);
            const auto& d=v->state.observed.dependencies[i];if(d.descriptor.isNotEmpty())require(sameMetadata(readText(*v->root,e.object),d.descriptor),"Backup descriptor differs from saved dependency");}
        out=std::move(v);error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}
bool matchesRecoveryTarget(const Verified& verified,const Input& target,juce::String& error)
{
    try{
        ObservedSnapshot current;
        require(legacy(target.processorXml,verified.state.manifestJson,current,error),"Backup source does not match the project reference: "+error);
        require(current.project==verified.state.observed.project && current.instance==verified.state.observed.instance
            && current.reference==verified.state.observed.reference && current.capture.revision==verified.state.observed.capture.revision
            && target.instanceRoot.getFileName()==current.instance && target.instanceRoot.getParentDirectory().getFileName()==current.project,
            "Backup belongs to another instance or source revision");
        for(const auto& dependency:current.dependencies){const auto file=std::find_if(verified.state.files.begin(),verified.state.files.end(),[&](const auto& e){return e.path==dependency.path;});
            require(file!=verified.state.files.end() && (dependency.checksum.isEmpty() || file->sha256==dependency.checksum),"Backup lacks a current dependency: "+dependency.path);
            if(dependency.descriptor.isNotEmpty()){const auto saved=std::find_if(verified.state.observed.dependencies.begin(),verified.state.observed.dependencies.end(),[&](const auto& d){return d.path==dependency.path;});
                require(saved!=verified.state.observed.dependencies.end() && sameMetadata(saved->descriptor,dependency.descriptor),"Backup descriptor does not match the current arrangement: "+dependency.path);}}
        error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}
bool locateReceipt(const juce::File& selectedReceipt,std::shared_ptr<Verified>& out,juce::String& error,const Hooks& hooks)
{
    try{require(selectedReceipt.getParentDirectory().getFileName()=="receipts","Select a completion receipt inside the backup receipts folder");
        const auto folder=selectedReceipt.getParentDirectory().getParentDirectory();Root root(folder);Receipt receipt;
        require(decodeReceipt(readText(root,"receipts/"+selectedReceipt.getFileName(),8192),receipt,error),error);
        require(selectedReceipt.getFileName()==receipt.locator.snapshot+".json","Completion receipt filename differs from its immutable snapshot");
        return locate(folder,receipt.locator,out,error,hooks);
    }catch(const std::exception& e){return fail(error,e.what());}
}
Result restoreCopy(const std::shared_ptr<Verified>& v,const juce::File& selected,const Hooks& hooks)
{
    Result result;try{require(v!=nullptr,"Select and verify a complete backup first");Root target(selected);require(!target.canonical.equalsIgnoreCase(v->root->canonical) && !target.canonical.startsWithIgnoreCase(v->root->canonical+"/") && !v->root->canonical.startsWithIgnoreCase(target.canonical+"/"),"Restore destination must be separate from the backup tree");const auto prefix=v->state.observed.project+"/"+v->state.observed.instance+"/";
        for(size_t i=0;i<v->state.files.size();++i){const auto& e=v->state.files[i];copy(target,prefix+e.path,v->files[i]->h,e,hooks);}
        for(const auto& e:v->state.files)verify(target,prefix+e.path,e.sha256,e.bytes,hooks);
        result.receipt=v->receipt;result.success=true;
    }catch(const std::exception& e){result.error=e.what();result.cancelled=hooks.cancelled && hooks.cancelled();}return result;
}
Result restoreLegacyCopy(const std::shared_ptr<Prepared>& prepared,const juce::File& destination,const Hooks& hooks)
{
    Result failed;if(hooks.cancelled && hooks.cancelled()){failed.cancelled=true;failed.error="Recovery cancelled";return failed;}if(!prepared){failed.error="Select and verify the original assets first";return failed;}
    auto verified=std::make_shared<Verified>();verified->state=prepared->state;verified->root=prepared->root;verified->files=prepared->files;
    // A legacy report deliberately has no portable store ID or completion receipt.
    verified->receipt=receiptFor({{},prepared->state.id},prepared->state);
    const auto expected=destination.getChildFile(prepared->state.observed.project).getChildFile(prepared->state.observed.instance);
    if(expected==prepared->root->directory){failed.success=true;failed.receipt=verified->receipt;return failed;}
    return restoreCopy(verified,destination,hooks);
}



class WorkingFolder
{
public:
    WorkingFolderInfo info;
    std::shared_ptr<Root> pinned;
    Held lease;
    Snapshot closure;uint64_t recoveryRecordBytes=0;
};
const WorkingFolderInfo& workingFolderInfo(const WorkingFolder& folder){return folder.info;}
namespace
{
void pinWorkingDirectories(WorkingFolder& folder,const ObservedSnapshot& observed)
{
    const auto prefix="working/"+observed.project+"/"+observed.instance+"/";
    for(const auto* category:{"segments","manifests","irs","renders","exports"})writeText(*folder.pinned,prefix+category+"/.directory-pin","ReverbArise directory guard v1\n",{});
    folder.info.projectsRoot=folder.info.folder.getChildFile("working");
    folder.info.association.writer=juce::Uuid().toDashedString();
    const auto branch="instances/"+observed.instance+"/branches/"+folder.info.association.writer+"/lease";
    const auto leaseFile=folder.pinned->path(branch,true);
    folder.lease=relativeOpen(folder.pinned->parentOf(leaseFile)->h,leaseFile.getFileName(),GENERIC_READ|GENERIC_WRITE,false,2,0);
    ordinary(folder.lease->h,false);
    // Pinned marker files keep managed categories nonempty. Together with
    // pinned directory ancestry this prevents directory replacement and
    // in-place junction conversion without blocking ordinary child publication.
    // Immutable data aliases are published with UUID/no-replace semantics by
    // the existing stores. Each runtime has its own exclusive publication lease;
    // no mutable common head is introduced or consulted on restore.
}
void registerWorkingHint(WorkingFolder& folder,const juce::File& registryRoot,const Hooks& hooks)
{
    if(!registryRoot.isDirectory()){
        auto ancestor=registryRoot;juce::String relative;
        for(int depth=0;depth<64 && !ancestor.isDirectory();++depth){relative=ancestor.getFileName()+"/"+relative;ancestor=ancestor.getParentDirectory();}
        require(ancestor.isDirectory() && relative.isNotEmpty(),"Could not resolve the local registry directory");
        Root owned(ancestor);writeText(owned,relative+".registry-pin","ReverbArise location registry v1\n",hooks);
    }
    Root registry(registryRoot);folder.info.association.registration=juce::Uuid().toDashedString();
    auto record=object();set(record,"version",1);set(record,"store",folder.info.association.store);set(record,"folder",folder.info.folder.getFullPathName());
    writeText(registry,"locations/"+folder.info.association.registration+".json",json(record),hooks);
}
void verifyWorkingAliases(WorkingFolder& folder,const Input& expected,const Hooks& hooks)
{
    std::shared_ptr<Prepared> exact;juce::String error;
    require(prepare(expected,exact,error,hooks),error);
    folder.closure=exact->state;folder.info.files=folder.closure.files.size();folder.info.bytes=0;
    for(const auto& entry:folder.closure.files)folder.info.bytes+=entry.bytes;
    // All exact byte hashes/descriptors are verified before writer admission.
}
ObservedSnapshot recoveryMetadata(const Input&,juce::String&,juce::String&,Association&);
void requireEmptyRecoveryRoot(Root&);
std::shared_ptr<WorkingFolder> openWorking(const juce::File& selected,const Association& saved,const juce::String& xml,
                                         const juce::String& manifest,const Hooks& hooks,bool allowMissing=false)
{
    require(associationValid(saved),"Invalid saved working-folder locator");
    require(associationSnapshot(xml,manifest)==saved.snapshot,"Saved working metadata snapshot mismatch");
    auto folder=std::make_shared<WorkingFolder>();folder->info.folder=selected;folder->pinned=std::make_shared<Root>(selected);
    require(storeId(*folder->pinned,false,hooks)==saved.store,"Selected folder has a different store identity");
    folder->info.association=saved;ObservedSnapshot observed;juce::String error;
    if(!legacy(xml,manifest,observed,error)){
        require(allowMissing && manifest.isEmpty(),error);juce::String base=xml,captured;Association ignored;
        const auto tree=juce::XmlDocument::parse(xml);require(tree!=nullptr,"Missing recovery metadata");
        Input partial{selected.getChildFile("working").getChildFile(tree->getStringAttribute("projectId")).getChildFile(tree->getStringAttribute("instanceId")),xml,{}};
        observed=recoveryMetadata(partial,base,captured,ignored);
    }
    require(observed.dependencies.size()<=maxFiles,"Too many associated dependencies");
    const auto prefix="working/"+observed.project+"/"+observed.instance+"/";
    for(const auto& dependency:observed.dependencies)require(std::wcslen(selected.getChildFile(prefix+dependency.path).getFullPathName().toWideCharPointer())<=240,
        "Selected path exceeds the supported Windows path length; choose a shorter working-folder path");
    auto association=saved;juce::String associated;require(encodeAssociatedState(xml,manifest,association,associated,error),error);
    Input input{selected.getChildFile("working").getChildFile(observed.project).getChildFile(observed.instance),associated,manifest};
    try{verifyWorkingAliases(*folder,input,hooks);}catch(const Failure& failure){
        require(allowMissing,"Associated dependencies could not be verified: "+juce::String(failure.what()));folder->info.incompleteRecovery=true;folder->closure.observed=observed;
        folder->info.captureMetadataKnown=manifest.isNotEmpty() || observed.reference.isEmpty();
    }
    folder->info.restoreManifest=manifest;folder->info.expected=std::move(input);return folder;
}
}
bool prepareWorkingFolder(const Input& input,const juce::File& selected,const juce::File& registryRoot,
                          std::shared_ptr<WorkingFolder>& output,juce::String& error,const Hooks& hooks)
{
    try{
        std::shared_ptr<Prepared> source;require(prepare(input,source,error,hooks),error);
        const auto copied=backupPack(source,selected,hooks,false);require(copied.success,copied.error);
        std::shared_ptr<Verified> verified;require(locate(selected,copied.receipt.locator,verified,error,hooks),error);
        auto folder=std::make_shared<WorkingFolder>();folder->info.folder=selected;folder->pinned=std::make_shared<Root>(selected);
        folder->info.association.store=copied.receipt.locator.store;folder->info.expected=input;
        require(storeId(*folder->pinned,false,hooks)==folder->info.association.store,"Working store identity changed during preparation");
        const auto prefix="working/"+source->state.observed.project+"/"+source->state.observed.instance+"/";
        for(size_t i=0;i<verified->state.files.size();++i)copy(*folder->pinned,prefix+verified->state.files[i].path,verified->files[i]->h,verified->state.files[i],hooks);
        Input destination=input;destination.instanceRoot=selected.getChildFile("working").getChildFile(source->state.observed.project).getChildFile(source->state.observed.instance);
        verifyWorkingAliases(*folder,destination,hooks);
        pinWorkingDirectories(*folder,folder->closure.observed);
        // Association metadata is distinct from a Pack receipt: the exact host
        // state remains authority for later accepted revisions and their aliases.
        juce::String xml=input.processorXml,manifest=input.manifestJson;
        if(auto tree=juce::XmlDocument::parse(xml);tree && tree->getIntAttribute("stateVersion")==associatedEnvelopeVersion){Association previous;require(decodeAssociatedState(xml,previous,xml,manifest,error),error);folder->info.association.recoveryRecord=previous.recoveryRecord;folder->info.association.recoveryOrigins=previous.recoveryOrigins;}
        folder->info.association.snapshot=associationSnapshot(xml,manifest);registerWorkingHint(*folder,registryRoot,hooks);
        checkpoint(hooks,"working-folder-ready");output=std::move(folder);error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}
bool revalidateWorkingFolder(const WorkingFolder& folder,const Input& current,juce::String& error,const Hooks& hooks)
{
    try{
        require(!folder.info.incompleteRecovery,"Incomplete recovery is not verified folder migration");
        const auto& expected=folder.info.expected;
        require(storeId(*folder.pinned,false,hooks)==folder.info.association.store,"Working store identity changed");
        require(current.instanceRoot==expected.instanceRoot && current.controlEpoch==expected.controlEpoch
            && current.processorXml==expected.processorXml && current.manifestJson==expected.manifestJson && current.sourceUnavailable==expected.sourceUnavailable && (!current.sourceUnavailable || folder.info.located),
            "Session changed after folder preparation. Prepare the folder again; originals are retained.");
        const auto prefix="working/"+folder.closure.observed.project+"/"+folder.closure.observed.instance+"/";
        for(const auto& entry:folder.closure.files)verify(*folder.pinned,prefix+entry.path,entry.sha256,entry.bytes,hooks);
        checkpoint(hooks,"working-folder-revalidated");error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}
bool locateWorkingFolder(const juce::File& selected,const Association& saved,const juce::String& xml,const juce::String& manifest,
                         const juce::File& registryRoot,std::shared_ptr<WorkingFolder>& output,juce::String& error,const Hooks& hooks)
{
    try{auto folder=openWorking(selected,saved,xml,manifest,hooks);pinWorkingDirectories(*folder,folder->closure.observed);registerWorkingHint(*folder,registryRoot,hooks);output=std::move(folder);error.clear();return true;}
    catch(const std::exception& e){return fail(error,e.what());}
}
bool prepareRestoredWorkingFolder(const Input& input,const juce::File& receiptFile,const juce::File& selected,
                                 const juce::File& registryRoot,std::shared_ptr<WorkingFolder>& output,juce::String& error,const Hooks& hooks)
{
    try{
        std::shared_ptr<Verified> verified;require(locateReceipt(receiptFile,verified,error,hooks),error);
        require(matchesRecoveryTarget(*verified,input,error),error);
        auto folder=std::make_shared<WorkingFolder>();folder->info.folder=selected;folder->info.expected=input;folder->info.located=true;
        folder->pinned=std::make_shared<Root>(selected);requireEmptyRecoveryRoot(*folder->pinned);
        folder->info.association.store=storeId(*folder->pinned,true,hooks);
        writeText(*folder->pinned,"working/.directory-pin","ReverbArise directory guard v1\n",hooks);
        const auto restored=restoreCopy(verified,selected.getChildFile("working"),hooks);require(restored.success,restored.error);
        auto base=input.processorXml,manifest=input.manifestJson;Association previous;
        if(const auto state=juce::XmlDocument::parse(base);state && state->getIntAttribute("stateVersion")==associatedEnvelopeVersion)
            require(decodeAssociatedState(base,previous,base,manifest,error),error);
        Input restoredInput=input;restoredInput.instanceRoot=selected.getChildFile("working").getChildFile(verified->state.observed.project).getChildFile(verified->state.observed.instance);
        restoredInput.sourceUnavailable=false;restoredInput.folderUnavailable=false;verifyWorkingAliases(*folder,restoredInput,hooks);
        folder->info.restoreManifest=manifest;folder->info.association.snapshot=associationSnapshot(base,manifest);
        folder->info.association.recoveryRecord=previous.recoveryRecord;folder->info.association.recoveryOrigins=previous.recoveryOrigins;
        pinWorkingDirectories(*folder,folder->closure.observed);registerWorkingHint(*folder,registryRoot,hooks);
        output=std::move(folder);error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}
bool prepareAssociatedLocation(const Input& input,const juce::File& selected,const juce::File& registryRoot,
                              std::shared_ptr<WorkingFolder>& output,juce::String& error,const Hooks& hooks)
{
    try{
        Association saved;juce::String base,manifest;require(decodeAssociatedState(input.processorXml,saved,base,manifest,error),error);
        require(input.manifestJson==manifest,"Conflicting current source metadata");
        auto folder=openWorking(selected,saved,base,manifest,hooks);folder->info.expected=input;folder->info.located=true;
        pinWorkingDirectories(*folder,folder->closure.observed);registerWorkingHint(*folder,registryRoot,hooks);
        output=std::move(folder);error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}
bool resolveWorkingFolder(const Association& saved,const juce::String& xml,const juce::String& manifest,
                          const juce::File& registryRoot,std::shared_ptr<WorkingFolder>& output,juce::String& error,const Hooks& hooks)
{
    try{
        require(associationValid(saved),"Invalid saved folder locator");Root registry(registryRoot);
        const auto readHint=[&](const juce::String& name)->juce::File{
            const auto value=boundedJson(readText(registry,"locations/"+name,8192));
            require(version(value,1) && only(value,{"version","store","folder"}) && value["store"].toString()==saved.store,"Invalid or mismatched local folder hint");
            const auto path=value["folder"].toString();
            require(path.length()>=3 && juce::CharacterFunctions::isLetter(path[0]) && path[1]==':' && (path[2]=='\\' || path[2]=='/'),"Local folder hints must be absolute; no working-directory inference is permitted");
            return juce::File(path);};
        try{auto folder=openWorking(readHint(saved.registration+".json"),saved,xml,manifest,hooks,true);pinWorkingDirectories(*folder,folder->closure.observed);output=std::move(folder);error.clear();return true;}catch(const Failure&){}
        registry.path("locations/placeholder",false);
        const auto records=registryRoot.getChildFile("locations").findChildFiles(juce::File::findFiles,false,"*.json");
        require(records.size()<=128,"Too many local folder hints; explicitly locate the folder");
        std::shared_ptr<WorkingFolder> match;
        for(const auto& record:records){checkpoint(hooks,"folder-hint");try{
            const auto path=readHint(record.getFileName());if(match && path==match->info.folder)continue;
            auto candidate=openWorking(path,saved,xml,manifest,hooks,true);
            require(!match,"Several verified store copies match; explicitly select the working folder");match=std::move(candidate);
        }catch(const Failure& e){if(juce::String(e.what()).startsWith("Several verified"))throw;}}
        require(match!=nullptr,"Associated assets are unavailable. Locate their working folder or restore a backup.");
        pinWorkingDirectories(*match,match->closure.observed);
        output=std::move(match);error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}


namespace
{
ObservedSnapshot recoveryMetadata(const Input& input,juce::String& legacyXml,juce::String& manifest,Association& origin)
{
    require(associationXmlShape(input.processorXml),"Malformed or oversized recovery metadata");
    auto xml=juce::XmlDocument::parse(input.processorXml);require(xml && xml->hasTagName("PreverbState"),"Missing recovery processor metadata");
    legacyXml=input.processorXml;manifest=input.manifestJson;juce::String error;
    if(xml->getStringAttribute("stateVersion")==juce::String(associatedEnvelopeVersion)){
        juce::String embedded;require(decodeAssociatedState(input.processorXml,origin,legacyXml,embedded,error),error);
        require(manifest.isEmpty() || manifest==embedded,"Conflicting saved recovery capture metadata");manifest=embedded;xml=juce::XmlDocument::parse(legacyXml);
    }
    require(xml->getStringAttribute("stateVersion")=="8","Recovery requires a supported accepted envelope");
    const auto project=xml->getStringAttribute("projectId"),instance=xml->getStringAttribute("instanceId");
    const auto reference=xml->getStringAttribute("manifestReference"),revision=xml->getStringAttribute("manifestRevision");
    require(uuid(project) && uuid(instance) && revision.getLargeIntValue()>=0 && juce::String(revision.getLargeIntValue())==revision
        && (reference.isEmpty()?revision=="0":safeRelativePath(reference) && reference.startsWith("manifests/") && reference.endsWith(".json")),"Invalid saved recovery identity");
    require(input.instanceRoot.getFileName()==instance && input.instanceRoot.getParentDirectory().getFileName()==project,"Recovery source namespace does not match saved identities");
    ObservedSnapshot observed;
    if(manifest.isNotEmpty() || reference.isEmpty())require(legacy(legacyXml,manifest,observed,error),error);
    else{
        // Decode only known creative references. No synthetic source manifest,
        // invented chunk list or inferred capture coverage is published.
        auto known=std::make_unique<juce::XmlElement>(*xml);known->setAttribute("manifestReference",juce::String{});known->setAttribute("manifestRevision","0");
        require(legacy(known->toString(),{},observed,error),error);
        observed.reference=reference;observed.capture.revision=static_cast<uint64_t>(revision.getLargeIntValue());
        observed.dependencies.push_back({reference,{},{},{}});
    }
    legacyXml=xml->toString();return observed;
}
void requireEmptyRecoveryRoot(Root& target)
{
    WIN32_FIND_DATAW data{};const auto search=target.directory.getChildFile("*").getFullPathName();
    const auto handle=FindFirstFileW(search.toWideCharPointer(),&data);
    if(handle==INVALID_HANDLE_VALUE){require(GetLastError()==ERROR_FILE_NOT_FOUND,"Could not inspect recovery destination");return;}
    struct FindGuard{HANDLE h;~FindGuard(){FindClose(h);}} guard{handle};
    do{const juce::String name(data.cFileName);require(name=="." || name=="..","Choose an empty, separate recovery workspace folder");}while(FindNextFileW(handle,&data));
    require(GetLastError()==ERROR_NO_MORE_FILES,"Could not finish recovery destination inspection");
}
}
bool prepareRecoveryWorkspace(const Input& input,const juce::File& selected,const juce::File& registryRoot,
                              std::shared_ptr<WorkingFolder>& output,juce::String& error,const Hooks& hooks)
{
    try{
        checkpoint(hooks,"recovery-workspace-start");juce::String xml,manifest;Association origin;
        auto observed=recoveryMetadata(input,xml,manifest,origin);
        auto folder=std::make_shared<WorkingFolder>();folder->info.folder=selected;folder->info.expected=input;
        folder->info.incompleteRecovery=true;folder->info.originalAssociation=origin;
        folder->info.captureMetadataKnown=manifest.isNotEmpty() || observed.reference.isEmpty();
        require(selected!=input.instanceRoot && !selected.isAChildOf(input.instanceRoot) && !input.instanceRoot.isAChildOf(selected),"Recovery workspace must be separate from the original source tree");
        folder->pinned=std::make_shared<Root>(selected);requireEmptyRecoveryRoot(*folder->pinned);
        std::shared_ptr<Root> source;
        try{source=std::make_shared<Root>(input.instanceRoot);}catch(const Failure&){folder->info.originalFolderUnavailable=true;}
        // A readable original manifest can recover known coverage even when the
        // live pin had no available-source JSON. It must match the saved identity.
        if(source && !folder->info.captureMetadataKnown){try{
            const auto captured=readText(*source,observed.reference,maximumMetadataBytes);ObservedSnapshot full;juce::String detail;
            require(legacy(xml,captured,full,detail),detail);observed=std::move(full);manifest=captured;folder->info.captureMetadataKnown=true;
        }catch(const Failure&){} }
        require(observed.dependencies.size()<=maxFiles,"Too many recovery dependencies");
        folder->info.association.store=storeId(*folder->pinned,true,hooks);
        folder->info.association.snapshot=associationSnapshot(xml,manifest);folder->closure.observed=observed;folder->info.restoreManifest=manifest;
        const auto revision=juce::XmlDocument::parse(xml)->getStringAttribute("manifestRevision").getLargeIntValue();
        RecoveryLocator previous{origin.store,origin.snapshot,origin.writer,origin.registration,observed.project,observed.instance,observed.reference,static_cast<uint64_t>(revision)};
        require(recoveryLocatorValid(previous),"Invalid original recovery locator");folder->info.association.recoveryOrigins.push_back(previous);
        for(const auto& older:origin.recoveryOrigins)if(std::find(folder->info.association.recoveryOrigins.begin(),folder->info.association.recoveryOrigins.end(),older)==folder->info.association.recoveryOrigins.end())folder->info.association.recoveryOrigins.push_back(older);
        require(folder->info.association.recoveryOrigins.size()<=16,"Too many recovery origins; history cannot be discarded");
        const auto prefix="working/"+observed.project+"/"+observed.instance+"/";uint64_t total=0;
        for(const auto& dependency:observed.dependencies){
            checkpoint(hooks,"recovery-survivor",dependency.path);Held file;Entry entry;entry.path=dependency.path;
            if(source){try{
                file=source->read(entry.path);entry.bytes=size(file->h);
                require(entry.bytes<=16ull*1024*1024*1024 && (!entry.path.endsWith(".json") || entry.bytes<=maximumMetadataBytes),"Oversized surviving dependency");
                entry.sha256=hash(file->h,hooks);entry.object=objectPath(entry);
                require(dependency.checksum.isEmpty() || dependency.checksum==entry.sha256,"Surviving dependency checksum differs");
                if(dependency.descriptor.isNotEmpty())require(sameMetadata(readText(*source,entry.path),dependency.descriptor),"Surviving descriptor differs");
            }catch(const Failure&){file.reset();}}
            if(!file){folder->info.missing.push_back(entry.path);continue;}
            total+=entry.bytes;require(total<=64ull*1024*1024*1024,"Recovery byte limit exceeded");
            // Destination failures are not mistaken for missing source files.
            copy(*folder->pinned,prefix+entry.path,file->h,entry,hooks);folder->closure.files.push_back(entry);
        }
        if(folder->info.captureMetadataKnown && observed.reference.isNotEmpty()
            && std::find(folder->info.missing.begin(),folder->info.missing.end(),observed.reference)!=folder->info.missing.end()){
            writeText(*folder->pinned,prefix+observed.reference,manifest,hooks);
            Entry entry{observed.reference,{},digest(manifest),static_cast<uint64_t>(manifest.getNumBytesAsUTF8())};entry.object=objectPath(entry);folder->closure.files.push_back(entry);total+=entry.bytes;
            require(total<=64ull*1024*1024*1024,"Recovery byte limit exceeded");folder->info.recreatedCaptureMetadata=true;
        }
        require(input.sourceUnavailable || folder->info.originalFolderUnavailable || !folder->info.missing.empty(),"All declared assets are available; prepare a verified folder copy instead");
        folder->info.files=folder->closure.files.size();folder->info.bytes=total;
        auto record=object();set(record,"version",1);set(record,"kind","incomplete-recovery-workspace");set(record,"complete",false);
        set(record,"originalProcessorXml",input.processorXml);set(record,"originalCaptureManifest",manifest);set(record,"captureMetadataKnown",folder->info.captureMetadataKnown);
        juce::Array<juce::var> missing;for(const auto& path:folder->info.missing)missing.add(path);set(record,"missing",missing);
        const auto metadata=json(record);folder->info.recoveryRecord=digest(metadata);
        folder->info.association.recoveryRecord=folder->info.recoveryRecord;
        writeText(*folder->pinned,prefix+"recovery/"+folder->info.recoveryRecord+".json",metadata,hooks);
        folder->recoveryRecordBytes=static_cast<uint64_t>(metadata.getNumBytesAsUTF8());
        pinWorkingDirectories(*folder,observed);registerWorkingHint(*folder,registryRoot,hooks);
        checkpoint(hooks,"recovery-workspace-ready");output=std::move(folder);error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}
bool revalidateRecoveryWorkspace(const WorkingFolder& folder,const Input& current,juce::String& error,const Hooks& hooks)
{
    try{
        const auto& expected=folder.info.expected;require(folder.info.incompleteRecovery,"Not an incomplete recovery workspace");
        require(current.instanceRoot==expected.instanceRoot && current.controlEpoch==expected.controlEpoch && current.processorXml==expected.processorXml
            && current.manifestJson==expected.manifestJson && current.sourceUnavailable==expected.sourceUnavailable,"Recovery session changed; keep the current association and prepare again");
        require(storeId(*folder.pinned,false,hooks)==folder.info.association.store,"Recovery store identity changed");
        verify(*folder.pinned,"working/"+folder.closure.observed.project+"/"+folder.closure.observed.instance+"/recovery/"+folder.info.recoveryRecord+".json",folder.info.recoveryRecord,folder.recoveryRecordBytes,hooks);
        const auto prefix="working/"+folder.closure.observed.project+"/"+folder.closure.observed.instance+"/";
        for(const auto& entry:folder.closure.files)verify(*folder.pinned,prefix+entry.path,entry.sha256,entry.bytes,hooks);
        checkpoint(hooks,"recovery-workspace-revalidated");error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}

}
