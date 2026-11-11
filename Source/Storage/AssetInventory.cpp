#include "AssetInventory.h"
#include <Capture/LoopCaptureState.h>
#include <Core/AssetAssociation.h>
#include <Core/ContentHash.h>
#include <juce_audio_formats/juce_audio_formats.h>
#define NOMINMAX
#include <Windows.h>
#include <array>
#include <cmath>

namespace m3::storage
{
namespace
{
bool fail(juce::String& error,const juce::String& message){error=message;return false;}
bool uuid(const juce::String& s){return s.length()==36 && !juce::Uuid(s).isNull() && juce::Uuid(s).toDashedString()==s;}
bool relative(const juce::String& path)
{
    const auto parts=juce::StringArray::fromTokens(path,"/",{});
    return path.length()<=192 && parts.size()==2 && !path.contains("..") && !path.containsChar(':') && !path.containsChar('\\')
        && parts[0].containsOnly("abcdefghijklmnopqrstuvwxyz") && parts[1].isNotEmpty()
        && parts[1].containsOnly("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_. ");
}
bool bounded(const juce::String& text,size_t bytes){return static_cast<size_t>(text.getNumBytesAsUTF8())<=bytes;}
struct Handle
{
    HANDLE value=INVALID_HANDLE_VALUE;
    explicit Handle(HANDLE h=INVALID_HANDLE_VALUE):value(h){}
    ~Handle(){if(value!=INVALID_HANDLE_VALUE)CloseHandle(value);}
    Handle(const Handle&)=delete;Handle& operator=(const Handle&)=delete;
    explicit operator bool()const{return value!=INVALID_HANDLE_VALUE;}
};
juce::String finalPath(HANDLE handle)
{
    std::array<wchar_t,32768> buffer{};const auto size=GetFinalPathNameByHandleW(handle,buffer.data(),static_cast<DWORD>(buffer.size()),FILE_NAME_NORMALIZED);
    return size>0 && size<buffer.size()?juce::String(buffer.data()).replaceCharacter('\\','/'):juce::String{};
}
bool below(const juce::String& path,const juce::String& root){return path.equalsIgnoreCase(root) || path.startsWithIgnoreCase(root+"/");}
Handle open(const juce::File& file,bool directory)
{
    return Handle(CreateFileW(file.getFullPathName().toWideCharPointer(),directory?FILE_READ_ATTRIBUTES:GENERIC_READ,
        FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT|(directory?FILE_FLAG_BACKUP_SEMANTICS:FILE_FLAG_SEQUENTIAL_SCAN),nullptr));
}
bool noAncestorLinks(juce::File path)
{
    for(;;){const auto a=GetFileAttributesW(path.getFullPathName().toWideCharPointer());if(a==INVALID_FILE_ATTRIBUTES || (a&FILE_ATTRIBUTE_REPARSE_POINT))return false;
        const auto parent=path.getParentDirectory();if(parent==path)return true;path=parent;}
}
juce::String category(const juce::String& p)
{
    const auto path="/"+p.toLowerCase();
    if(path.contains(".tmp") || path.endsWith(".part") || path.contains(".unpublished-"))return "Unknown temporary files";
    if(path.contains("/segments/") && path.endsWith(".wav"))return "Captured audio";
    if(path.contains("/manifests/") && path.endsWith(".json"))return "Capture history";
    if(path.contains("/irs/"))return "IRs and descriptors";
    if(path.contains("/renders/"))return "Wet renders and descriptors";
    if(path.contains("/exports/"))return "Protected exports";
    return "Other or unknown";
}
bool decodedHash(const juce::MemoryBlock& bytes,juce::String& output)
{
    juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(new juce::MemoryInputStream(bytes,false),true));
    if(!reader || !reader->usesFloatingPointData || reader->bitsPerSample!=32 || reader->numChannels<1 || reader->numChannels>2 || reader->lengthInSamples<0
        || static_cast<uint64_t>(reader->lengthInSamples)>bytes.getSize()/(4*reader->numChannels))return false;
    ContentHash hash;const auto rate=reader->sampleRate;const auto channels=static_cast<int>(reader->numChannels);const auto frames=static_cast<uint64_t>(reader->lengthInSamples);
    hash.update(&rate,sizeof(rate));hash.update(&channels,sizeof(channels));hash.update(&frames,sizeof(frames));
    juce::AudioBuffer<float> block(channels,4096);std::array<float,8192> interleaved{};
    for(int64_t at=0;at<reader->lengthInSamples;at+=4096){const auto n=static_cast<int>(std::min(int64_t{4096},reader->lengthInSamples-at));
        if(!reader->read(&block,0,n,at,true,true))return false;
        for(int i=0;i<n;++i)for(int c=0;c<channels;++c){const auto sample=block.getSample(c,i);if(!std::isfinite(sample))return false;interleaved[static_cast<size_t>(i*channels+c)]=sample;}
        hash.samples(interleaved.data(),static_cast<size_t>(n*channels));}
    output=hash.finish();return true;
}
}
bool visitDependencies(ObservedSnapshot& source,juce::String& error)
{
    if(!isValidCaptureManifest(source.capture,error) || !validate(source.session,error))return false;
    auto next=source;next.dependencies.clear();next.emptyIRReferences=0;next.unknownCaptureHistory=source.capture.schemaVersion==1;
    std::map<juce::String,Dependency> entries;
    const auto add=[&](Dependency d){if(!relative(d.path))return fail(error,"Unsafe dependency path");const auto key=d.path.toLowerCase();
        if(const auto found=entries.find(key);found!=entries.end()){
            if(found->second.path!=d.path || found->second.checksum!=d.checksum || found->second.descriptor!=d.descriptor)return fail(error,"Conflicting dependency aliases");
        }else entries.emplace(key,std::move(d));return true;};
    // Build relative descriptor paths without consulting process CWD.
    const auto managed=[&](const Asset& a){if(!a.present()){++next.emptyIRReferences;return true;}if(a.engine!=1)return fail(error,"Unsupported dependency engine");
        if(!add({a.path,a.checksum,a.content,{}}))return false;return add({a.path.upToLastOccurrenceOf(".",false,false)+".json",{},{},juce::JSON::toString(assetMetadata(a),true)});};
    juce::String manifest;
    if(source.reference.isNotEmpty() && (!serializeCaptureManifest(source.capture,manifest,error) || !add({source.reference,{},{},manifest})))return false;
    for(const auto& range:source.capture.ranges)if(range.kind==CaptureCoverageKind::audio && !add({range.assetPath,range.checksum,{},{}}))return false;
    for(const auto& context:source.capture.generationContexts)if(!managed(context.ir))return false;
    if(!managed(source.session.defaultIR))return false;
    for(const auto& entry:source.session.importedIRs)if(!managed(entry.asset))return false;
    for(const auto& swell:source.session.swells){if(!managed(swell.ir))return false;if(swell.render.present() && !managed(swell.render))return false;}
    for(const auto& file:source.session.exportState.history)if(!add({file.path,file.checksum,{},{}}))return false;
    for(auto& [key,d]:entries)next.dependencies.push_back(std::move(d));source=std::move(next);error.clear();return true;
}
bool readLegacySnapshot(const juce::String& xml,const juce::String& manifestJson,ObservedSnapshot& output,juce::String& error)
{
    if(!bounded(xml,97*1024*1024) || !bounded(manifestJson,maximumMetadataBytes))return fail(error,"Oversized observed state");
    if(xml.containsIgnoreCase("<!DOCTYPE") || xml.containsIgnoreCase("<!ENTITY"))return fail(error,"External/entity XML is not supported by the read-only adapter");
    const auto envelope=juce::XmlDocument::parse(xml);if(!envelope || !envelope->hasTagName("PreverbState"))return fail(error,"Malformed processor envelope");
    const auto version=envelope->getIntAttribute("stateVersion",0);
    if(version!=associatedEnvelopeVersion && !bounded(xml,maximumMetadataBytes*6+1024*1024))return fail(error,"Oversized legacy state");
    if(version==associatedEnvelopeVersion){
        Association association;juce::String legacy,captured;
        if(!decodeAssociatedState(xml,association,legacy,captured,error) || captured!=manifestJson)return fail(error,"Associated metadata does not match the supplied capture manifest");
        ObservedSnapshot next;next.label=output.label;if(!readLegacySnapshot(legacy,captured,next,error))return false;
        if(association.recoveryRecord.isNotEmpty())next.dependencies.push_back({"recovery/"+association.recoveryRecord+".json",association.recoveryRecord,{},{}});
        output=std::move(next);return true;
    }
    if(version<1 || version>8 || envelope->getStringAttribute("stateVersion")!=juce::String(version))return fail(error,"Unsupported processor envelope version");
    ObservedSnapshot next;next.label=output.label;
    if(version>=2){next.project=envelope->getStringAttribute("projectId");next.instance=envelope->getStringAttribute("instanceId");next.reference=envelope->getStringAttribute("manifestReference");
        const auto revision=envelope->getStringAttribute("manifestRevision");const auto n=revision.getLargeIntValue();
        if(!uuid(next.project) || !uuid(next.instance) || n<0 || juce::String(n)!=revision)return fail(error,"Invalid observed capture identity");
        next.capture.projectId=next.project.toStdString();next.capture.instanceId=next.instance.toStdString();next.capture.revision=static_cast<uint64_t>(n);
        if(next.reference.isNotEmpty()){
            if(!relative(next.reference) || !next.reference.startsWith("manifests/") || !next.reference.endsWith(".json") || !parseCaptureManifest(manifestJson,next.capture,error))return fail(error,"Invalid or missing accepted capture manifest");
            if(next.capture.projectId!=next.project.toStdString() || next.capture.instanceId!=next.instance.toStdString() || next.capture.revision!=static_cast<uint64_t>(n))return fail(error,"Observed source identity/revision mismatch");
        }else if(n!=0)return fail(error,"Missing accepted manifest reference");
    }else{next.capture.projectId=juce::Uuid().toDashedString().toStdString();next.capture.instanceId=juce::Uuid().toDashedString().toStdString();}
    auto kind = CaptureSourceKind::song;
    LoopCapturePreference preference;
    if (!readLoopState(*envelope, kind, preference) || kind != next.capture.kind())
        return fail(error, "Saved source kind or next-capture preference does not match the capture metadata");
    if(version>=3){const auto* session=envelope->getChildByName("SwellSession");if(!session || !decode(session->getAllSubText(),next.session,error))return fail(error,"Invalid or unsupported creative state: "+error);}
    else next.session.autoGenerateOnStop=true;
    if(version==3){next.session.mappingEnabled=false;next.session.automaticProcessing=false;for(auto& swell:next.session.swells)swell.provenance=Provenance::manualEdited;}
    if(!visitDependencies(next,error))return false;output=std::move(next);return true;
}
void Catalog::rebuild(const std::vector<ObservedSnapshot>& snapshots,const std::vector<Pin>& pins,bool priorIndexAvailable)
{
    references.clear();aliases.clear();uncertainty={"Offline FLPs and external Playlist copies are not enumerable. All assets remain protected."};
    if(!priorIndexAvailable)uncertainty.push_back("No prior observed index; absence never proves an asset unused.");
    size_t edges=0;
    for(const auto& snapshot:snapshots){for(const auto& d:snapshot.dependencies){if(++edges>200000){uncertainty.push_back("Observed reference capacity reached; protection remains unconditional.");return;}references[d.path.toLowerCase()].insert(snapshot.label);if(d.decodedIdentity.isNotEmpty())aliases[d.decodedIdentity].insert(d.path);}if(snapshot.unknownCaptureHistory)uncertainty.push_back("Legacy context history is unknown: "+snapshot.label);}
    for(const auto& pin:pins){if(relative(pin.path))references[pin.path.toLowerCase()].insert(pin.label);else uncertainty.push_back("Unknown active/recovery pin: "+pin.label);}
    uncertainty.push_back("Cross-process jobs/readers and unobserved history may exist; no reclamation is enabled.");
}
Report inspect(const Request& request,const std::function<bool()>& cancelled)
{
    Report report;report.scannedAt=juce::Time::getCurrentTime();try {
    if(request.roots.empty() && request.observationError.isNotEmpty()){
        report.inventoryComplete=false;report.dependencyVerificationComplete=false;report.issues.push_back(request.observationError.substring(0,512));return report;
    }
    if(request.roots.empty() || request.roots.size()>128 || request.snapshots.size()>128 || request.activePins.size()>4096 || request.limits.entries>20000 || request.limits.depth>12
        || request.limits.decodeBytes>32ull*1024*1024 || request.limits.milliseconds>30000 || request.limits.milliseconds<=0){report.inventoryComplete=false;report.dependencyVerificationComplete=false;report.issues.push_back("Invalid or excessive inventory limits.");return report;}
    report.catalog.rebuild(request.snapshots,request.activePins,request.priorIndexAvailable);
    const auto started=juce::Time::getMillisecondCounterHiRes();size_t entries=0;uint64_t remaining=request.limits.hashBytes;
    std::set<juce::String> physical;std::map<juce::String,juce::String> paths;uint64_t metadataBytes=0;size_t observedEdges=0;
    const auto issue=[&](const juce::String& text){report.inventoryComplete=false;if(report.issues.size()<128)report.issues.push_back(text);};
    if(request.observationError.isNotEmpty())issue(request.observationError.substring(0,512));
    const auto stop=[&]{if(cancelled && cancelled()){report.cancelled=true;return true;}return entries>=request.limits.entries || juce::Time::getMillisecondCounterHiRes()-started>request.limits.milliseconds;};
    for(const auto& root:request.roots){
        if(stop()){issue("Scan cancelled or reached its entry/time limit.");break;}
        if(root.getFullPathName().startsWith("\\\\") || !noAncestorLinks(root)){issue("Missing, inaccessible or linked root: "+root.getFullPathName());continue;}
        auto rootHandle=open(root,true);if(!rootHandle){issue("Cannot open selected root: "+root.getFullPathName());continue;}
        const auto resolvedRoot=finalPath(rootHandle.value);if(resolvedRoot.isEmpty()){issue("Cannot resolve selected root.");continue;}
        std::function<void(const juce::File&,size_t)> walk;
        walk=[&](const juce::File& directory,size_t depth){
            if(stop()){issue("Scan cancelled or reached its entry/time limit.");return;}
            if(depth>request.limits.depth){issue("Directory depth limit reached.");return;}
            auto directoryHandle=open(directory,true);BY_HANDLE_FILE_INFORMATION di{};
            if(!directoryHandle || !GetFileInformationByHandle(directoryHandle.value,&di) || (di.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) || !below(finalPath(directoryHandle.value),resolvedRoot)){issue("Skipped inaccessible, replaced or linked directory.");return;}
            WIN32_FIND_DATAW data{};const auto search=directory.getChildFile("*").getFullPathName();const auto find=FindFirstFileW(search.toWideCharPointer(),&data);
            struct FindGuard {HANDLE value;~FindGuard(){if(value!=INVALID_HANDLE_VALUE)FindClose(value);}} findGuard{find};
            if(find==INVALID_HANDLE_VALUE){if(GetLastError()!=ERROR_FILE_NOT_FOUND)issue("Cannot enumerate directory: "+directory.getFullPathName());return;}
            for(bool more=true;more;more=FindNextFileW(find,&data)!=FALSE){const juce::String name(data.cFileName);if(name=="." || name=="..")continue;
                if(stop()){issue("Scan cancelled or reached its entry/time limit.");break;}++entries;const auto file=directory.getChildFile(name);
                if(data.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT){issue("Skipped link: "+file.getFullPathName());continue;}
                if(data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){walk(file,depth+1);continue;}
                auto handle=open(file,false);BY_HANDLE_FILE_INFORMATION info{};FILE_STANDARD_INFO standard{};
                if(!handle || !GetFileInformationByHandle(handle.value,&info) || !GetFileInformationByHandleEx(handle.value,FileStandardInfo,&standard,sizeof(standard)) || (info.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)){issue("Cannot read file identity: "+file.getFullPathName());continue;}
                const auto resolved=finalPath(handle.value);if(resolved.isEmpty() || !below(resolved,resolvedRoot)){issue("File escaped selected root.");continue;}
                InventoryFile item;item.root=root.getFullPathName();item.path=file.getRelativePathFrom(root).replaceCharacter('\\','/');item.category=category(item.path);
                item.physicalId=juce::String(info.dwVolumeSerialNumber)+":"+juce::String(info.nFileIndexHigh)+":"+juce::String(info.nFileIndexLow);
                if(const auto found=paths.find(resolved.toLowerCase());found!=paths.end()){if(found->second!=item.physicalId)issue("Ambiguous case-colliding paths.");continue;}paths.emplace(resolved.toLowerCase(),item.physicalId);
                item.bytes=static_cast<uint64_t>(standard.EndOfFile.QuadPart);item.allocated=static_cast<uint64_t>(standard.AllocationSize.QuadPart);
                report.logicalBytes+=item.bytes;auto& count=report.categories[item.category];++count.files;count.bytes+=item.bytes;
                if(physical.insert(item.physicalId).second){++report.physicalFiles;report.physicalBytes+=item.bytes;report.allocatedBytes+=item.allocated;}
                const bool metadata=file.hasFileExtension("json");const bool audio=file.hasFileExtension("wav");
                juce::MemoryBlock bytes;const bool retain=(metadata && item.bytes<=maximumMetadataBytes && metadataBytes+item.bytes<=64ull*1024*1024) || (audio && item.bytes<=request.limits.decodeBytes);
                if(item.bytes<=remaining){ContentHash hash;std::vector<char> block(65536);uint64_t read=0;bool ok=true;
                    while(read<item.bytes){if(stop()){ok=false;break;}DWORD got=0;const auto n=static_cast<DWORD>(std::min<uint64_t>(block.size(),item.bytes-read));
                        if(!ReadFile(handle.value,block.data(),n,&got,nullptr) || got==0){ok=false;break;}hash.update(block.data(),got);if(retain)bytes.append(block.data(),got);read+=got;}
                    remaining-=read;BY_HANDLE_FILE_INFORMATION after{};
                    if(!ok || !GetFileInformationByHandle(handle.value,&after) || after.nFileSizeHigh!=info.nFileSizeHigh || after.nFileSizeLow!=info.nFileSizeLow || CompareFileTime(&after.ftLastWriteTime,&info.ftLastWriteTime)!=0)issue("File changed or became unreadable during scan: "+item.path);
                    else{item.sha256=hash.finish();if(metadata && retain){metadataBytes+=item.bytes;item.metadata=juce::String::fromUTF8(static_cast<const char*>(bytes.getData()),static_cast<int>(bytes.getSize()));}else if(metadata)issue("Metadata budget reached; history is uncertain.");
                        if(audio){if(!retain || !decodedHash(bytes,item.decodedIdentity))++report.decodedSkipped;}}
                }else{issue("Hash-byte limit reached; counts are retained but verification is incomplete.");if(audio)++report.decodedSkipped;}
                if(name.equalsIgnoreCase("store.json"))issue("Portable store format is not yet supported by this reader.");
                if(item.category=="Capture history"){
                    CaptureManifest observed;juce::String error;
                    if(!item.metadata.isEmpty() && parseCaptureManifest(item.metadata,observed,error)){++report.observedCaptureSnapshots;
                        const auto reference=[&](const juce::String& path){if(++observedEdges<=200000)report.catalog.references[path.toLowerCase()].insert("Observed capture revision "+juce::String(static_cast<juce::int64>(observed.revision))+" / "+item.path);else issue("Historical reference capacity reached.");};
                        for(const auto& range:observed.ranges)if(range.kind==CaptureCoverageKind::audio)reference(range.assetPath);
                        for(const auto& context:observed.generationContexts)if(context.ir.present()){reference(context.ir.path);reference(context.ir.path.upToLastOccurrenceOf(".",false,false)+".json");report.catalog.aliases[context.ir.content].insert(context.ir.path);}}
                    else issue("Unsupported or unreadable historical manifest: "+item.path);
                }
                if(metadata && (item.path.startsWithIgnoreCase("irs/") || item.path.startsWithIgnoreCase("renders/"))){Asset alias;juce::String error;
                    if(parseAssetMetadata(juce::JSON::parse(item.metadata),alias,error) && alias.engine==1)report.catalog.aliases[alias.content].insert(alias.path);
                    else issue("Unsupported or invalid managed descriptor: "+item.path);}
                report.files.push_back(std::move(item));
            }
            const auto enumerationError=GetLastError();if(enumerationError!=ERROR_NO_MORE_FILES && !stop())issue("Directory enumeration was interrupted.");
            BY_HANDLE_FILE_INFORMATION after{};if(!GetFileInformationByHandle(directoryHandle.value,&after) || CompareFileTime(&di.ftLastWriteTime,&after.ftLastWriteTime)!=0)issue("Directory changed during scan; results are observational.");
        };walk(root,0);
    }
    std::map<juce::String,std::set<juce::String>> exact,decoded;std::map<juce::String,uint64_t> sizes;
    for(const auto& file:report.files){if(file.sha256.isNotEmpty()){exact[file.sha256].insert(file.physicalId);sizes[file.sha256]=file.bytes;}if(file.decodedIdentity.isNotEmpty())decoded[file.decodedIdentity].insert(file.physicalId);}
    for(const auto& [hash,ids]:exact)if(ids.size()>1)report.exactDuplicateBytes+=(ids.size()-1)*sizes[hash];
    for(const auto& [hash,ids]:decoded)if(ids.size()>1)report.decodedEquivalentFiles+=ids.size();
    for(const auto& snapshot:request.snapshots)for(const auto& d:snapshot.dependencies){bool found=false;
        for(auto& file:report.files)if(file.path.equalsIgnoreCase(d.path)){
            if(!uuid(snapshot.project) || !uuid(snapshot.instance) || !juce::File(file.root).getFileName().equalsIgnoreCase(snapshot.instance)
                || !juce::File(file.root).getParentDirectory().getFileName().equalsIgnoreCase(snapshot.project))continue;
            const auto good=file.sha256.isNotEmpty() && (d.checksum.isEmpty() || d.checksum==file.sha256)
                && (d.descriptor.isEmpty() || juce::JSON::toString(juce::JSON::parse(file.metadata),true)==d.descriptor);
            if(good){file.observedBy.insert(snapshot.label);found=true;break;}}
        if(found)++report.referencedFiles;else{++report.missingReferences;report.dependencyVerificationComplete=false;}}
    std::set<juce::String> referencedPhysical;
    for(auto& file:report.files){if(const auto labels=report.catalog.references.find(file.path.toLowerCase());labels!=report.catalog.references.end())file.observedBy.insert(labels->second.begin(),labels->second.end());
        if(!file.observedBy.empty() && referencedPhysical.insert(file.physicalId).second)report.observedReferenceBytes+=file.bytes;}
    report.protectedHistoryOrUnknownBytes=report.physicalBytes-report.observedReferenceBytes;
    if(!report.inventoryComplete)report.dependencyVerificationComplete=false;
    } catch(const std::exception& e){report.inventoryComplete=false;report.dependencyVerificationComplete=false;report.issues.push_back("Inventory failed: "+juce::String(e.what()));}
    return report;
}
CaptureRecoveryReport captureRecoveryCandidates(const Report& report,const juce::File& root,
                                                 const ObservedSnapshot& accepted)
{
    CaptureRecoveryReport result;
    if(!uuid(accepted.project) || !uuid(accepted.instance) || root.getFileName()!=accepted.instance
        || root.getParentDirectory().getFileName()!=accepted.project)return result;
    std::map<juce::String,const InventoryFile*> files;
    std::set<juce::String> referenced;
    for(const auto& file:report.files)
        if(juce::File(file.root)==root)files.emplace(file.path.toLowerCase(),&file);
    result.complete=report.inventoryComplete && !report.cancelled && !files.empty();
    for(const auto& dependency:accepted.dependencies)referenced.insert(dependency.path.toLowerCase());
    for(const auto& [path,file]:files){
        if(!path.startsWith("manifests/") || !path.endsWith(".json") || !relative(file->path))continue;
        CaptureRecoveryCandidate candidate{file->path,file->sha256};
        ObservedSnapshot observed;
        observed.reference=file->path;
        juce::String error;
        if(file->sha256.isEmpty() || !parseCaptureManifest(file->metadata,observed.capture,error)
            || observed.capture.projectId!=accepted.project.toStdString()
            || observed.capture.instanceId!=accepted.instance.toStdString()
            || !visitDependencies(observed,error)){
            candidate.unavailable.add("Unsupported or unreadable capture snapshot.");
            result.complete=false;
        }else{
            candidate.revision=observed.capture.revision;
            for(const auto& dependency:observed.dependencies){
                const auto key=dependency.path.toLowerCase();
                referenced.insert(key);
                const auto found=files.find(key);
                if(found==files.end() || found->second->sha256.isEmpty()
                    || (dependency.checksum.isNotEmpty() && found->second->sha256!=dependency.checksum)
                    || (dependency.descriptor.isNotEmpty()
                        && juce::JSON::toString(juce::JSON::parse(found->second->metadata),true)!=dependency.descriptor))
                    candidate.unavailable.add(dependency.path);
            }
        }
        if(file->path!=accepted.reference)result.snapshots.push_back(std::move(candidate));
    }
    for(const auto& [path,file]:files)
        if(path.startsWith("segments/") && path.endsWith(".wav") && relative(file->path)
            && !referenced.contains(path) && !report.catalog.references.contains(path))result.unindexedAudio.add(file->path);
    return result;
}

InventoryService::InventoryService():Thread("Read-only asset inventory"){}
InventoryService::~InventoryService(){signalThreadShouldExit();++epoch;notify();stopThread(-1);}
uint64_t InventoryService::request(Request request)
{
    const juce::ScopedLock guard(lock);const auto id=++epoch;pending=std::move(request);result.store(nullptr);working=true;
    if(!isThreadRunning() && !startThread()){
        pending.reset();auto failed=std::make_shared<Report>();failed->inventoryComplete=false;failed->dependencyVerificationComplete=false;
        failed->issues.push_back("The storage scan worker could not start. Reopen Storage to retry.");result.store(std::move(failed));working=false;
    }else notify();return id;
}
void InventoryService::cancel(){const juce::ScopedLock guard(lock);++epoch;pending.reset();result.store(nullptr);working=false;notify();}
void InventoryService::run()
{
    while(!threadShouldExit()){std::optional<Request> next;uint64_t id=0;{const juce::ScopedLock guard(lock);next=std::move(pending);pending.reset();id=epoch.load();}
        if(!next){wait(-1);continue;}auto report=std::make_shared<Report>(inspect(*next,[&]{return threadShouldExit() || epoch.load()!=id;}));
        {const juce::ScopedLock guard(lock);if(epoch.load()==id && !threadShouldExit()){result.store(std::move(report));working=false;}}}
}
}
