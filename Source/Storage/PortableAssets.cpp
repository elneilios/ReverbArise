#include "PortableAssets.h"
#include <Core/ContentHash.h>
#define NOMINMAX
#include <Windows.h>
#include <winternl.h>
#include <array>
#include <cwchar>
#include <stdexcept>

namespace m3::storage::portable
{
namespace
{
struct Failure:std::runtime_error {using std::runtime_error::runtime_error;NTSTATUS status=0;Failure(const std::string& message,NTSTATUS code):std::runtime_error(message),status(code){}};
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
        if(c=='=' && ++attributes>128)return false;if(c=='<')return false;if(c=='>'){if(++tags>128)return false;if(closing){if(--depth<0)return false;}else{if(++depth>16)return false;if(previous=='/')--depth;}inTag=false;}
        else if(!juce::CharacterFunctions::isWhitespace(c))previous=c;}
    return !inTag && !quote && depth==0;
}
bool legacy(const juce::String& xml,const juce::String& manifest,ObservedSnapshot& out,juce::String& error)
{
    if(static_cast<size_t>(xml.getNumBytesAsUTF8())>maximumMetadataBytes*6+1024*1024 || static_cast<size_t>(manifest.getNumBytesAsUTF8())>maximumMetadataBytes
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

Held relativeOpen(HANDLE parent,const juce::String& leaf,DWORD access,bool directory,ULONG disposition,DWORD sharing)
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
    constexpr ULONG synchronous=0x20,openReparsePoint=0x00200000,directoryFile=1,nonDirectoryFile=0x40;
    const auto code=createFile(&file,access|SYNCHRONIZE,&attributes,&io,nullptr,FILE_ATTRIBUTE_NORMAL,sharing,disposition,
                              synchronous|openReparsePoint|(directory?directoryFile:nonDirectoryFile),nullptr,0);
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
    Held read(const juce::String& relative)
    {
        const auto file=path(relative);const auto owner=parentOf(file);
        auto held=relativeOpen(owner->h,file.getFileName(),GENERIC_READ,false,1,FILE_SHARE_READ);
        require(resolved(held->h).startsWithIgnoreCase(canonical+"/"),"File escaped selected root");files.push_back(held);return held;
    }
};

uint64_t size(HANDLE h){LARGE_INTEGER n{};require(GetFileSizeEx(h,&n)!=0 && n.QuadPart>=0,"Could not read file size");return static_cast<uint64_t>(n.QuadPart);}
void rewind(HANDLE h){LARGE_INTEGER n{};require(SetFilePointerEx(h,n,nullptr,FILE_BEGIN)!=0,"Could not seek pinned file");}
void checkpoint(const Hooks& hooks,const juce::String& event,const juce::String& path={})
{require(!hooks.cancelled || !hooks.cancelled(),"Operation cancelled; originals and previous backups remain intact");require(!hooks.checkpoint || hooks.checkpoint(event,path),"Injected interruption at "+event+": "+path);}
juce::String hash(HANDLE h,const Hooks& hooks={})
{
    rewind(h);ContentHash sum;std::array<char,65536> block{};for(;;){checkpoint(hooks,"read");DWORD n=0;require(ReadFile(h,block.data(),static_cast<DWORD>(block.size()),&n,nullptr)!=0,"Could not read pinned file");if(!n)break;sum.update(block.data(),n);}return sum.finish();
}
juce::String readText(Root& root,const juce::String& relative,size_t limit=maxDocumentBytes)
{
    const auto file=root.read(relative);const auto n=size(file->h);require(n<=limit,"Oversized portable metadata");std::vector<char> bytes(static_cast<size_t>(n)+1,0);DWORD got=0;
    require(ReadFile(file->h,bytes.data(),static_cast<DWORD>(n),&got,nullptr)!=0 && got==n,"Unreadable portable metadata");const auto text=juce::String::fromUTF8(bytes.data(),static_cast<int>(n));require(text.getNumBytesAsUTF8()==n && std::memcmp(text.toRawUTF8(),bytes.data(),static_cast<size_t>(n))==0,"Invalid or NUL-containing UTF-8 metadata");return text;
}
void verify(Root& root,const juce::String& path,const juce::String& expected,uint64_t bytes,const Hooks& hooks)
{const auto h=root.read(path);require(size(h->h)==bytes && hash(h->h,hooks)==expected,"Asset byte identity mismatch: "+path);}
void publish(Root& root,const juce::String& relative,const juce::String& expected,uint64_t bytes,
             const std::function<void(HANDLE)>& write,const Hooks& hooks)
{
    checkpoint(hooks,"before-file",relative);const auto destination=root.path(relative,true);
    const auto attributes=GetFileAttributesW(destination.getFullPathName().toWideCharPointer());
    if(attributes!=INVALID_FILE_ATTRIBUTES){verify(root,relative,expected,bytes,hooks);return;}
    require(GetLastError()==ERROR_FILE_NOT_FOUND,"Destination cannot be inspected");
    const auto temporary=root.operation.isNotEmpty()?root.path("transactions/"+root.operation+"/staging/part-"+juce::Uuid().toDashedString(),true):destination.getSiblingFile("part-"+juce::Uuid().toDashedString());
    auto h=relativeOpen(root.parentOf(temporary)->h,temporary.getFileName(),GENERIC_READ|GENERIC_WRITE|DELETE,false,2,FILE_SHARE_READ);
    ordinary(h->h,false);require(resolved(h->h).startsWithIgnoreCase(root.canonical+"/"),"Staged file escaped root");write(h->h);
    require(size(h->h)==bytes && hash(h->h,hooks)==expected,"Destination-only hash verification failed");require(FlushFileBuffers(h->h)!=0,"Could not flush destination bytes");checkpoint(hooks,"before-publish",relative);
    const auto name=destination.getFileName();std::vector<char> memory(sizeof(FILE_RENAME_INFO)+(static_cast<size_t>(name.length())+1)*sizeof(wchar_t),0);auto* rename=reinterpret_cast<FILE_RENAME_INFO*>(memory.data());
    rename->ReplaceIfExists=FALSE;rename->RootDirectory=root.parentOf(destination)->h;rename->FileNameLength=static_cast<DWORD>(name.length()*sizeof(wchar_t));std::memcpy(rename->FileName,name.toWideCharPointer(),rename->FileNameLength);
    using RenameFunction=NTSTATUS(NTAPI*)(HANDLE,PIO_STATUS_BLOCK,PVOID,ULONG,FILE_INFORMATION_CLASS);
    const auto renameFile=reinterpret_cast<RenameFunction>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtSetInformationFile"));
    require(renameFile!=nullptr,"Handle-relative publication is unavailable on this Windows version");IO_STATUS_BLOCK status{};
    const auto renamed=renameFile(h->h,&status,rename,static_cast<ULONG>(memory.size()),static_cast<FILE_INFORMATION_CLASS>(10));
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
void copy(Root& destination,const juce::String& relative,HANDLE source,const Entry& entry,const Hooks& hooks)
{
    publish(destination,relative,entry.sha256,entry.bytes,[&](HANDLE h){rewind(source);std::array<char,65536> block{};uint64_t copied=0;
        while(copied<entry.bytes){checkpoint(hooks,"copy",relative);DWORD n=0;require(ReadFile(source,block.data(),static_cast<DWORD>(std::min<uint64_t>(block.size(),entry.bytes-copied)),&n,nullptr)!=0 && n>0,"Source became unreadable");
            writeBytes(h,block.data(),n,copied,hooks);copied+=n;}require(size(source)==entry.bytes,"Source changed during copy");},hooks);
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
juce::String storeId(Root& root,bool create,const Hooks& hooks)
{
    auto path=root.directory.getChildFile("store.json");
    if(!path.exists() && create){juce::Array<juce::File> entries;root.directory.findChildFiles(entries,juce::File::findFilesAndDirectories,false);
        require(entries.isEmpty(),"Choose an empty folder or an existing supported asset backup");auto v=object();set(v,"version",storeVersion);set(v,"storeId",juce::Uuid().toDashedString());set(v,"reader",readerVersion);writeText(root,"store.json",json(v),hooks);}
    const auto v=boundedJson(readText(root,"store.json",8192));require(version(v,storeVersion) && only(v,{"version","storeId","reader"}) && v["reader"].isInt() && static_cast<int>(v["reader"])==readerVersion && uuid(v["storeId"].toString()),"Unknown or invalid portable store version/identity");return v["storeId"].toString();
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
    try{auto p=std::make_shared<Prepared>();p->state.processorXml=in.processorXml;p->state.manifestJson=in.manifestJson;p->state.exclusions=in.exclusions;p->state.producer=in.producer;
        require(legacy(in.processorXml,in.manifestJson,p->state.observed,error),error);const auto& s=p->state.observed;
        require(uuid(s.project) && uuid(s.instance) && in.instanceRoot.getFileName()==s.instance && in.instanceRoot.getParentDirectory().getFileName()==s.project,"Selected source root does not match accepted identities");
        require(s.dependencies.size()<=maxFiles,"Too many accepted dependencies");p->root=std::make_shared<Root>(in.instanceRoot);
        uint64_t total=0;for(const auto& d:s.dependencies){checkpoint(hooks,"prepare",d.path);const auto h=p->root->read(d.path);Entry e;e.path=d.path;e.bytes=size(h->h);require(e.bytes<=16ull*1024*1024*1024,"Asset exceeds portable file limit");total+=e.bytes;require(total<=64ull*1024*1024*1024 && (!e.path.endsWith(".json") || e.bytes<=maximumMetadataBytes),"Portable byte/metadata budget exceeded");e.sha256=hash(h->h,hooks);e.object=objectPath(e);
            require(d.checksum.isEmpty() || d.checksum==e.sha256,"Accepted checksum differs: "+d.path);if(d.descriptor.isNotEmpty())require(sameMetadata(readText(*p->root,d.path),d.descriptor),"Accepted descriptor differs: "+d.path);
            p->state.files.push_back(e);p->files.push_back(h);}
        Snapshot checked;require(decodeSnapshot(encodeSnapshot(p->state),checked,error),error);p->state=std::move(checked);out=std::move(p);error.clear();return true;
    }catch(const std::exception& e){return fail(error,e.what());}
}
namespace
{
Result execute(const Snapshot& s,Root* source,const std::vector<Held>* pinned,Root& destination,const juce::String& store,const juce::String& operation,const Hooks& hooks)
{
    Result result;result.operation=operation;try{
        destination.operation=operation;checkClosure(s);const auto text=encodeSnapshot(s);require(digest(text)==s.id,"Snapshot changed after preparation");
        auto plan=object();set(plan,"version",journalVersion);set(plan,"operation",operation);set(plan,"storeId",store);set(plan,"snapshot",text);
        writeText(destination,"transactions/"+operation+"/plan.json",json(plan),hooks);checkpoint(hooks,"journal");
        for(size_t i=0;i<s.files.size();++i){const auto& e=s.files[i];const auto target=destination.path(e.object,true);
            if(target.exists()){verify(destination,e.object,e.sha256,e.bytes,hooks);continue;}
            require(source!=nullptr,"Originals are unavailable and a backup object is still missing");const auto h=pinned?(*pinned)[i]:source->read(e.path);copy(destination,e.object,h->h,e,hooks);checkpoint(hooks,"object",e.path);}
        // Validate destination-only closure and saved descriptor bytes before any
        // snapshot or completion receipt makes this operation visible as complete.
        for(const auto& e:s.files)verify(destination,e.object,e.sha256,e.bytes,hooks);
        for(size_t i=0;i<s.files.size();++i){const auto& d=s.observed.dependencies[i];if(d.descriptor.isNotEmpty())require(sameMetadata(readText(destination,s.files[i].object),d.descriptor),"Destination descriptor does not match saved metadata");}
        checkpoint(hooks,"closure");writeText(destination,"snapshots/"+s.id+".json",text,hooks);checkpoint(hooks,"snapshot");
        result.receipt=receiptFor({store,s.id},s);writeText(destination,"receipts/"+s.id+".json",encodeReceipt(result.receipt),hooks);result.success=true;
    }catch(const std::exception& e){result.error=e.what();result.cancelled=hooks.cancelled && hooks.cancelled();}return result;
}
Snapshot interrupted(Root& root,const juce::String& operation,juce::String& store)
{
    require(uuid(operation),"Invalid operation identity");store=storeId(root,false,{});const auto plan=boundedJson(readText(root,"transactions/"+operation+"/plan.json"));
    require(version(plan,journalVersion) && only(plan,{"version","operation","storeId","snapshot"}) && plan["operation"].toString()==operation && plan["storeId"].toString()==store,"Invalid or conflicting interrupted operation");Snapshot s;juce::String error;require(decodeSnapshot(plan["snapshot"].toString(),s,error),error);return s;
}
}
Result backup(const std::shared_ptr<Prepared>& p,const juce::File& selected,const Hooks& hooks)
{
    Result result;try{require(p!=nullptr,"No accepted snapshot was prepared");checkpoint(hooks,"start");Root destination(selected);
        require(!destination.canonical.equalsIgnoreCase(p->root->canonical) && !destination.canonical.startsWithIgnoreCase(p->root->canonical+"/") && !p->root->canonical.startsWithIgnoreCase(destination.canonical+"/"),"Backup destination must be separate from the working instance tree");
        ULARGE_INTEGER free{};require(GetDiskFreeSpaceExW(selected.getFullPathName().toWideCharPointer(),&free,nullptr,nullptr)!=0,"Could not measure destination free space");uint64_t total=0;for(const auto& e:p->state.files)total+=e.bytes;
        require(free.QuadPart>=total*2+4*encodeSnapshot(p->state).getNumBytesAsUTF8()+8192,"Insufficient free space for verified backup and staging");
        const auto store=storeId(destination,true,hooks);Root source(p->root->directory);return execute(p->state,&source,nullptr,destination,store,juce::Uuid().toDashedString(),hooks);
    }catch(const std::exception& e){result.error=e.what();result.cancelled=hooks.cancelled && hooks.cancelled();return result;}
}
bool inspectInterrupted(const juce::File& selected,const juce::String& operation,Snapshot& out,juce::String& error)
{try{Root root(selected);juce::String store;auto s=interrupted(root,operation,store);out=std::move(s);error.clear();return true;}catch(const std::exception& e){return fail(error,e.what());}}
Result resume(const juce::File& selected,const juce::String& operation,const juce::File& explicitSource,const Hooks& hooks)
{
    Result result;result.operation=operation;try{Root destination(selected);juce::String store;const auto s=interrupted(destination,operation,store);std::unique_ptr<Root> source;
        if(explicitSource!=juce::File{}){require(explicitSource.getFileName()==s.observed.instance && explicitSource.getParentDirectory().getFileName()==s.observed.project,"Resume source identity differs");source=std::make_unique<Root>(explicitSource);}
        return execute(s,source.get(),nullptr,destination,store,operation,hooks);
    }catch(const std::exception& e){result.error=e.what();return result;}
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
}
