#include "PortableFixture.h"
#include <windows.h>
#include <winioctl.h>
#include <aclapi.h>
#include <future>
#include <Storage/PortableAssets.h>
using namespace m3::storage::portable;
static juce::var parse(const juce::String& s){return juce::JSON::parse(s);}
static juce::String encodeJson(const juce::var& v){return juce::JSON::toString(v,true);}
static void property(juce::var& v,const juce::Identifier& name,const juce::var& value){v.getDynamicObject()->setProperty(name,value);}
static void junction(const juce::File& link,const juce::File& target)
{
    REQUIRE(link.createDirectory().wasOk());
    struct Mount {DWORD tag;USHORT size,reserved,subOffset,subLength,printOffset,printLength;wchar_t path[2048];} data{};
    const auto text=juce::String("\\??\\")+target.getFullPathName();const auto length=static_cast<USHORT>(text.length()*sizeof(wchar_t));REQUIRE(length+2<sizeof(data.path));
    data.tag=IO_REPARSE_TAG_MOUNT_POINT;data.size=static_cast<USHORT>(8+length+4);data.subLength=length;data.printOffset=static_cast<USHORT>(length+2);
    std::memcpy(data.path,text.toWideCharPointer(),length);const auto handle=CreateFileW(link.getFullPathName().toWideCharPointer(),GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS,nullptr);REQUIRE(handle!=INVALID_HANDLE_VALUE);
    DWORD written=0;const auto success=DeviceIoControl(handle,FSCTL_SET_REPARSE_POINT,&data,data.size+8,nullptr,0,&written,nullptr);CloseHandle(handle);REQUIRE(success);
}

// Only the disposable fixture's DACL changes, and is restored before cleanup.
struct ReadOnlyFixture
{
    juce::File folder;PSECURITY_DESCRIPTOR original=nullptr;PACL oldAcl=nullptr,newAcl=nullptr;
    explicit ReadOnlyFixture(juce::File file):folder(std::move(file))
    {
        REQUIRE(GetNamedSecurityInfoW(const_cast<wchar_t*>(folder.getFullPathName().toWideCharPointer()),SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,nullptr,nullptr,&oldAcl,nullptr,&original)==ERROR_SUCCESS);
        HANDLE token=nullptr;REQUIRE(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token));DWORD bytes=0;GetTokenInformation(token,TokenUser,nullptr,0,&bytes);std::vector<char> data(bytes);
        const bool got=GetTokenInformation(token,TokenUser,data.data(),bytes,&bytes)!=0;CloseHandle(token);REQUIRE(got);
        EXPLICIT_ACCESSW deny{};deny.grfAccessPermissions=FILE_ADD_FILE|FILE_ADD_SUBDIRECTORY;deny.grfAccessMode=DENY_ACCESS;deny.grfInheritance=NO_INHERITANCE;
        deny.Trustee.TrusteeForm=TRUSTEE_IS_SID;deny.Trustee.TrusteeType=TRUSTEE_IS_USER;deny.Trustee.ptstrName=static_cast<LPWSTR>(reinterpret_cast<TOKEN_USER*>(data.data())->User.Sid);
        REQUIRE(SetEntriesInAclW(1,&deny,oldAcl,&newAcl)==ERROR_SUCCESS);
        REQUIRE(SetNamedSecurityInfoW(const_cast<wchar_t*>(folder.getFullPathName().toWideCharPointer()),SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,nullptr,nullptr,newAcl,nullptr)==ERROR_SUCCESS);
    }
    ~ReadOnlyFixture(){if(original){SetNamedSecurityInfoW(const_cast<wchar_t*>(folder.getFullPathName().toWideCharPointer()),SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,nullptr,nullptr,oldAcl,nullptr);LocalFree(original);}if(newAcl)LocalFree(newAcl);}
};

#include "working_folder_test.h"
#include "ScratchPreviewCases.h"
int main(int argc,char** argv){if(argc>1 && juce::String(argv[1])=="--scratch")return runTest(scratchPreviewCases);if(argc>1)return workingFolderTest(argc,argv);return runTest([]{
    Fixture f;juce::String error;Input input{f.root,f.xml,f.manifest};std::shared_ptr<Prepared> prepared;
    if(!prepare(input,prepared,error))throw std::runtime_error(error.toStdString());const auto saved=snapshot(*prepared);REQUIRE(saved.files.size()==f.observed.dependencies.size());
    REQUIRE(saved.observed.session.swells[0].frozen && !saved.observed.session.swells[0].enabled && saved.observed.capture.schemaVersion==2);
    Snapshot decoded;REQUIRE(decodeSnapshot(encodeSnapshot(saved),decoded,error));REQUIRE(decoded.id==saved.id && decoded.processorXml==f.xml && decoded.manifestJson==f.manifest);
    for(const auto& path:juce::StringArray{"../bad.wav","/absolute","C:/drive","x/CON.wav","x/NUL.json","x/a.","x/a ","x/a:stream","x\\a.wav","a//b","x/COM1.wav"})REQUIRE(!safeRelativePath(path));
    REQUIRE(safeRelativePath("irs/normal-name_1.wav"));
    const auto original=decoded.id;auto invalid=parse(encodeSnapshot(saved));property(invalid,"version",2);REQUIRE(!decodeSnapshot(encodeJson(invalid),decoded,error) && decoded.id==original);
    invalid=parse(encodeSnapshot(saved));auto rows=*invalid["files"].getArray();rows.removeLast();property(invalid,"files",rows);REQUIRE(!decodeSnapshot(encodeJson(invalid),decoded,error));
    invalid=parse(encodeSnapshot(saved));rows=*invalid["files"].getArray();property(rows.getReference(0),"object","../escape.wav");property(invalid,"files",rows);REQUIRE(!decodeSnapshot(encodeJson(invalid),decoded,error));
    invalid=parse(encodeSnapshot(saved));rows=*invalid["files"].getArray();rows.add(rows[0]);property(invalid,"files",rows);REQUIRE(!decodeSnapshot(encodeJson(invalid),decoded,error));

    // Unsupported per-file descriptor versions and case-conflicting aliases close safely.
    invalid=parse(encodeSnapshot(saved));rows=*invalid["files"].getArray();property(rows.getReference(0),"version",2);property(invalid,"files",rows);REQUIRE(!decodeSnapshot(encodeJson(invalid),decoded,error));
    invalid=parse(encodeSnapshot(saved));rows=*invalid["files"].getArray();property(rows.getReference(0),"path",rows[0]["path"].toString().toUpperCase());property(invalid,"files",rows);REQUIRE(!decodeSnapshot(encodeJson(invalid),decoded,error));
    auto oldPrepared=prepared;Hooks stopPreparation;stopPreparation.cancelled=[]{return true;};REQUIRE(!prepare(input,prepared,error,stopPreparation) && prepared==oldPrepared);
    // Real NTFS junctions cannot redirect either source reads or destination writes.
    TestDirectory outside;REQUIRE(outside.file.createDirectory().wasOk());REQUIRE(outside.file.getChildFile("keep.txt").replaceWithText("outside unchanged"));
    TestDirectory linked;REQUIRE(linked.file.createDirectory().wasOk());bool linkedOnce=false;Hooks redirect;
    redirect.checkpoint=[&](const auto& event,const auto&){if(event=="journal" && !linkedOnce){junction(linked.file.getChildFile("objects"),outside.file);linkedOnce=true;}return true;};
    const auto linkedResult=backup(prepared,linked.file,redirect);if(!linkedOnce)std::cerr<<linkedResult.error<<"\\n";REQUIRE(!linkedResult.success && linkedOnce);REQUIRE(outside.file.getChildFile("keep.txt").loadFileAsString()=="outside unchanged");
    REQUIRE(RemoveDirectoryW(linked.file.getChildFile("objects").getFullPathName().toWideCharPointer()));
    TestDirectory linkParent;REQUIRE(linkParent.file.createDirectory().wasOk());const auto rootLink=linkParent.file.getChildFile("linked-root");junction(rootLink,outside.file);
    REQUIRE(!backup(prepared,rootLink).success);REQUIRE(RemoveDirectoryW(rootLink.getFullPathName().toWideCharPointer()));
    // A pinned destination directory cannot be replaced between validation/publication.
    TestDirectory racing;REQUIRE(racing.file.createDirectory().wasOk());bool raced=false;Hooks race;
    race.checkpoint=[&](const auto& event,const auto& relative){if(event=="before-publish" && relative.startsWith("objects/")){
        const auto parent=racing.file.getChildFile(relative).getParentDirectory();const auto moved=parent.getSiblingFile("replaced-directory");
        REQUIRE(!MoveFileExW(parent.getFullPathName().toWideCharPointer(),moved.getFullPathName().toWideCharPointer(),0));raced=true;}return true;};
    const auto raceResult=backup(prepared,racing.file,race);if(!raceResult.success)std::cerr<<raceResult.error<<"\\n";REQUIRE(raceResult.success && raced);

    TestDirectory inPlace;REQUIRE(inPlace.file.createDirectory().wasOk());juce::File converted;Hooks mutate;
    mutate.checkpoint=[&](const auto& event,const auto& relative){if(event=="before-publish" && relative.startsWith("objects/") && converted==juce::File{}){converted=inPlace.file.getChildFile(relative).getParentDirectory();junction(converted,outside.file);}return true;};
    const auto mutation=backup(prepared,inPlace.file,mutate);REQUIRE(!mutation.success && converted!=juce::File{});REQUIRE(outside.file.getChildFile("keep.txt").loadFileAsString()=="outside unchanged");REQUIRE(RemoveDirectoryW(converted.getFullPathName().toWideCharPointer()));

    TestDirectory blocked;REQUIRE(blocked.file.createDirectory().wasOk());REQUIRE(backup(prepared,blocked.file).success);const auto exclusive=CreateFileW(blocked.file.getChildFile("store.json").getFullPathName().toWideCharPointer(),GENERIC_READ,0,nullptr,OPEN_EXISTING,0,nullptr);
    REQUIRE(exclusive!=INVALID_HANDLE_VALUE);REQUIRE(!backup(prepared,blocked.file).success);CloseHandle(exclusive);
    REQUIRE(!backup(prepared,juce::File::createFileWithoutCheckingPath("C:relative")).success);
    REQUIRE(!backup(prepared,juce::File::createFileWithoutCheckingPath("\\\\server\\share")).success);

    {TestDirectory readOnly;REQUIRE(readOnly.file.createDirectory().wasOk());ReadOnlyFixture deny(readOnly.file);const auto refused=backup(prepared,readOnly.file);REQUIRE(!refused.success && !readOnly.file.getChildFile("receipts").exists());}
    // A file larger than the copy page leaves partial owned staging on a write
    // interruption; no completion is published and resume verifies fresh bytes.
    {Fixture large;std::vector<float> samples(96000,.125f);const auto context=large.capture.snapshot().manifest.generationContexts.front();REQUIRE(large.capture.commit(100,48000,samples.data(),48000,0,&context));large.refresh();Input accepted{large.root,large.xml,large.manifest};std::shared_ptr<Prepared> largePin;REQUIRE(prepare(accepted,largePin,error));
        TestDirectory diskFault;REQUIRE(diskFault.file.createDirectory().wasOk());Hooks fault;fault.injectWriteError=[](uint64_t written,uint32_t){return written>=65536?static_cast<uint32_t>(ERROR_DISK_FULL):0u;};
        const auto failed=backup(largePin,diskFault.file,fault);REQUIRE(!failed.success && failed.operation.isNotEmpty() && failed.error.contains("112"));REQUIRE(!diskFault.file.getChildFile("receipts/"+snapshot(*largePin).id+".json").exists());
        juce::Array<juce::File> partials;diskFault.file.getChildFile("transactions/"+failed.operation+"/staging").findChildFiles(partials,juce::File::findFiles,false);REQUIRE(std::any_of(partials.begin(),partials.end(),[](const auto& file){return file.getSize()==65536;}));REQUIRE(resume(diskFault.file,failed.operation,large.root).success);}
    // A real second volume is opt-in; no network/removable path is inferred.
    const auto cross=juce::SystemStats::getEnvironmentVariable("PREVERB_PORTABLE_CROSS_VOLUME_ROOT",{});
    if(cross.isNotEmpty()){TestDirectory volume;volume.file=juce::File(cross).getChildFile("portable-"+juce::Uuid().toString());REQUIRE(volume.file.createDirectory().wasOk());
        REQUIRE(volume.file.getFullPathName().substring(0,2).toLowerCase()!=f.root.getFullPathName().substring(0,2).toLowerCase());
        wchar_t sourceVolume[MAX_PATH]{},targetVolume[MAX_PATH]{};DWORD sourceSerial=0,targetSerial=0;REQUIRE(GetVolumePathNameW(f.root.getFullPathName().toWideCharPointer(),sourceVolume,MAX_PATH));REQUIRE(GetVolumePathNameW(volume.file.getFullPathName().toWideCharPointer(),targetVolume,MAX_PATH));
        REQUIRE(GetVolumeInformationW(sourceVolume,nullptr,0,&sourceSerial,nullptr,nullptr,nullptr,0));REQUIRE(GetVolumeInformationW(targetVolume,nullptr,0,&targetSerial,nullptr,nullptr,nullptr,0));REQUIRE(sourceSerial!=targetSerial);auto result=backup(prepared,volume.file);if(!result.success)std::cerr<<result.error<<'\n';REQUIRE(result.success);std::shared_ptr<Verified> check;REQUIRE(locate(volume.file,result.receipt.locator,check,error));std::cout<<"Physical cross-volume backup verified.\n";}


    Snapshot beforeBad=decoded;const auto nested=juce::String::repeatedString("[",1000)+"0"+juce::String::repeatedString("]",1000);REQUIRE(!decodeSnapshot(nested,decoded,error) && decoded.id==beforeBad.id);
    invalid=parse(encodeSnapshot(saved));property(invalid,"futureDependencies",juce::Array<juce::var>{"unknown.wav"});REQUIRE(!decodeSnapshot(encodeJson(invalid),decoded,error));
    Input tooDeep=input;tooDeep.manifestJson=nested;REQUIRE(!prepare(tooDeep,prepared,error) && prepared==oldPrepared);
    tooDeep=input;tooDeep.processorXml="<PreverbState>"+juce::String::repeatedString("<nested>",64)+juce::String::repeatedString("</nested>",64)+"</PreverbState>";REQUIRE(!prepare(tooDeep,prepared,error));
    auto wide=juce::var(new juce::DynamicObject);for(int i=0;i<129;++i)property(wide,juce::Identifier("field"+juce::String(i)),i);REQUIRE(!decodeSnapshot(encodeJson(wide),decoded,error));
    const auto hugeArray="["+juce::String::repeatedString("0,",65536)+"0]";REQUIRE(!decodeSnapshot(hugeArray,decoded,error));
    Receipt untouched;untouched.project="preserve";REQUIRE(!decodeReceipt(juce::String::repeatedString(" ",8193),untouched,error) && untouched.project=="preserve");

    TestDirectory destination;REQUIRE(destination.file.createDirectory().wasOk());
    auto packed=backup(prepared,destination.file);if(!packed.success)std::cerr<<packed.error<<'\n';REQUIRE(packed.success);
    REQUIRE(packed.receipt.files==saved.files.size() && packed.receipt.instance==f.observed.instance);
    Receipt receipt;REQUIRE(decodeReceipt(encodeReceipt(packed.receipt),receipt,error));REQUIRE(receipt.locator.snapshot==saved.id);
    auto badReceipt=parse(encodeReceipt(receipt));property(badReceipt,"version",2);REQUIRE(!decodeReceipt(encodeJson(badReceipt),receipt,error));
    std::shared_ptr<Verified> verified;REQUIRE(locate(destination.file,packed.receipt.locator,verified,error));
    auto wrong=packed.receipt.locator;wrong.store=juce::Uuid().toDashedString();auto previous=verified;REQUIRE(!locate(destination.file,wrong,verified,error) && verified==previous);
    // Repeat backups reuse verified immutable bytes without changing earlier receipts.
    auto repeated=backup(prepared,destination.file);REQUIRE(repeated.success && repeated.receipt.locator.snapshot==saved.id);
    auto writerA=std::async(std::launch::async,[&]{return backup(prepared,destination.file);});auto writerB=std::async(std::launch::async,[&]{return backup(prepared,destination.file);});
    const auto resultA=writerA.get(),resultB=writerB.get();if(!resultA.success)std::cerr<<resultA.error<<'\n';if(!resultB.success)std::cerr<<resultB.error<<'\n';REQUIRE(resultA.success && resultB.success);
    // User cancellation never publishes a completion receipt; owned staging can resume.
    TestDirectory interrupted;REQUIRE(interrupted.file.createDirectory().wasOk());bool stop=false;Hooks cancellation;
    cancellation.cancelled=[&]{return stop;};cancellation.checkpoint=[&](const auto& phase,const auto&){if(phase=="object")stop=true;return true;};
    auto cancelled=backup(prepared,interrupted.file,cancellation);REQUIRE(!cancelled.success && cancelled.cancelled && cancelled.operation.isNotEmpty());
    Snapshot interruptedState;REQUIRE(inspectInterrupted(interrupted.file,cancelled.operation,interruptedState,error));REQUIRE(interruptedState.id==saved.id);
    REQUIRE(!interrupted.file.getChildFile("receipts/"+saved.id+".json").exists());
    auto resumed=resume(interrupted.file,cancelled.operation,f.root);if(!resumed.success)std::cerr<<resumed.error<<'\n';REQUIRE(resumed.success);
    // Fault each publication boundary; originals and any prior completed pack survive.
    for(const auto& phase:juce::StringArray{"journal","object","closure","snapshot"}){TestDirectory fault;REQUIRE(fault.file.createDirectory().wasOk());Hooks hooks;
        hooks.checkpoint=[&](const auto& point,const auto&){return point!=phase;};const auto failed=backup(prepared,fault.file,hooks);REQUIRE(!failed.success);
        REQUIRE(!fault.file.getChildFile("receipts/"+saved.id+".json").exists());if(failed.operation.isNotEmpty())REQUIRE(resume(fault.file,failed.operation,f.root).success);
        REQUIRE(locate(destination.file,packed.receipt.locator,verified,error));}

    for(const auto& entry:saved.files){TestDirectory fault;REQUIRE(fault.file.createDirectory().wasOk());Hooks hooks;hooks.checkpoint=[&](const auto& event,const auto& path){return event!="before-publish" || path!=entry.object;};
        const auto failed=backup(prepared,fault.file,hooks);REQUIRE(!failed.success && !fault.file.getChildFile("receipts/"+saved.id+".json").exists());REQUIRE(resume(fault.file,failed.operation,f.root).success);}
    {TestDirectory marker;REQUIRE(marker.file.createDirectory().wasOk());Hooks hooks;hooks.checkpoint=[](const auto& event,const auto& path){return event!="before-file" || !path.startsWith("receipts/");};const auto failed=backup(prepared,marker.file,hooks);REQUIRE(!failed.success && !marker.file.getChildFile("receipts/"+saved.id+".json").exists());REQUIRE(resume(marker.file,failed.operation,f.root).success);}
    {TestDirectory moved;const auto initial=moved.file.getChildFile("before"),after=moved.file.getChildFile("after");REQUIRE(destination.file.copyDirectoryTo(initial));REQUIRE(initial.moveFileTo(after) && !initial.exists());std::shared_ptr<Verified> movedPin;REQUIRE(locate(after,packed.receipt.locator,movedPin,error));}
    {TestDirectory unicode;Fixture source;const auto accented=juce::String::fromUTF8("\xc3\xa9\xf0\x9f\x8e\xb5");const auto sourceRoot=unicode.file.getChildFile(accented).getChildFile(source.observed.project).getChildFile(source.observed.instance);REQUIRE(sourceRoot.getParentDirectory().createDirectory().wasOk());REQUIRE(source.root.moveFileTo(sourceRoot));source.root=sourceRoot;
        const auto target=unicode.file.getChildFile(accented+"-backup"),recovery=unicode.file.getChildFile(accented+"r");REQUIRE(target.createDirectory().wasOk() && recovery.createDirectory().wasOk());std::shared_ptr<Prepared> sourcePin;if(!prepare({source.root,source.xml,source.manifest},sourcePin,error))throw std::runtime_error(error.toStdString());auto backed=backup(sourcePin,target);REQUIRE(backed.success);std::shared_ptr<Verified> found;REQUIRE(locate(target,backed.receipt.locator,found,error));const auto recoveredUnicode=restoreCopy(found,recovery);if(!recoveredUnicode.success)throw std::runtime_error(recoveredUnicode.error.toStdString());}

    // Existing user files cannot be claimed as a store.
    TestDirectory unrelated;REQUIRE(unrelated.file.createDirectory().wasOk());REQUIRE(unrelated.file.getChildFile("keep.txt").replaceWithText("keep"));REQUIRE(!backup(prepared,unrelated.file).success);REQUIRE(unrelated.file.getChildFile("keep.txt").loadFileAsString()=="keep");
    // Destination identity is rechecked; a conflicting byte object is never overwritten.
    TestDirectory corrupt;REQUIRE(corrupt.file.createDirectory().wasOk());REQUIRE(destination.file.copyDirectoryTo(corrupt.file));auto corruptPack=packed;
    const auto first=corrupt.file.getChildFile(saved.files[0].object);REQUIRE(first.replaceWithText("corrupt"));auto held=verified;REQUIRE(!locate(corrupt.file,corruptPack.receipt.locator,verified,error) && verified==held);
    REQUIRE(!backup(prepared,corrupt.file).success && first.loadFileAsString()=="corrupt");
    // Explicit receipt selection resolves an exact backup after process-local hints are lost.
    REQUIRE(locateReceipt(destination.file.getChildFile("receipts/"+packed.receipt.locator.snapshot+".json"),verified,error));
    const auto receiptHeld=verified;
    REQUIRE(!locateReceipt(destination.file.getChildFile("snapshots/"+packed.receipt.locator.snapshot+".json"),verified,error) && verified==receiptHeld);
    REQUIRE(!locateReceipt(corrupt.file.getChildFile("receipts/"+packed.receipt.locator.snapshot+".json"),verified,error) && verified==receiptHeld);
    // Pinning blocks replacing source dependencies while copy/verification proceeds.
    const auto source=f.root.getChildFile(saved.files[0].path);const auto before=fileHash(source);REQUIRE(!source.replaceWithText("must fail"));REQUIRE(fileHash(source)==before);
    // Exact destination-only legacy restore with source tree unavailable.
    REQUIRE(locate(destination.file,packed.receipt.locator,verified,error));prepared.reset();oldPrepared.reset();
    const auto offline=f.root.getSiblingFile("offline-"+f.root.getFileName());REQUIRE(f.root.moveFileTo(offline));std::shared_ptr<Prepared> absent;REQUIRE(!prepare(input,absent,error));
    TestDirectory restored;REQUIRE(restored.file.createDirectory().wasOk());const auto recovered=restoreCopy(verified,restored.file);if(!recovered.success)std::cerr<<recovered.error<<'\n';REQUIRE(recovered.success);
    const auto restoredRoot=restored.file.getChildFile(f.observed.project).getChildFile(f.observed.instance);
    for(const auto& e:saved.files)REQUIRE(fileHash(restoredRoot.getChildFile(e.path))==e.sha256);
    CaptureSession restoredCapture(restored.file);REQUIRE(restoredCapture.restore(f.observed.project.toStdString(),f.observed.instance.toStdString(),f.observed.reference,f.observed.capture.revision));
    float left=0,right=0;REQUIRE(restoredCapture.readSample(0,left,right) && left==.1f && right==-.2f);
    SwellAssetStore restoredAssets(restoredRoot);for(const auto& ir:f.irs)REQUIRE(restoredAssets.verify(ir,error));REQUIRE(restoredAssets.verify(f.session.swells[0].render,error));
    // A repeated explicit restore verifies existing bytes and refuses conflicts.
    REQUIRE(restoreCopy(verified,restored.file).success);const auto conflict=restoredRoot.getChildFile(saved.files[0].path);REQUIRE(conflict.replaceWithText("keep conflicting file"));
    REQUIRE(!restoreCopy(verified,restored.file).success && conflict.loadFileAsString()=="keep conflicting file");
    // Missing saved export history prevents preparing a complete pack.
    f.root=offline;f.refresh();input={f.root,f.xml,f.manifest};REQUIRE(!prepare(input,prepared,error)); // No silent reassociation.
    Fixture missingExport;REQUIRE(missingExport.root.getChildFile("exports/Swells-old.wav").deleteFile());Input missing{missingExport.root,missingExport.xml,missingExport.manifest};REQUIRE(!prepare(missing,prepared,error));
    Fixture contextOnly;contextOnly.session.swells.clear();contextOnly.session.defaultIR={};contextOnly.session.importedIRs.clear();contextOnly.refresh();Input only{contextOnly.root,contextOnly.xml,contextOnly.manifest};REQUIRE(prepare(only,prepared,error));
    REQUIRE(snapshot(*prepared).observed.emptyIRReferences>0 && snapshot(*prepared).observed.capture.generationContexts.size()>0);
    TestDirectory contextDestination;REQUIRE(contextDestination.file.createDirectory().wasOk());auto contextPack=backup(prepared,contextDestination.file);REQUIRE(contextPack.success);REQUIRE(locate(contextDestination.file,contextPack.receipt.locator,verified,error));
    {Fixture legacy;auto manifest=legacy.capture.snapshot().manifest;manifest.schemaVersion=1;manifest.generationContexts.clear();manifest.contextSpans.clear();juce::String legacyJson;REQUIRE(serializeCaptureManifest(manifest,legacyJson,error));REQUIRE(legacy.root.getChildFile(legacy.observed.reference).replaceWithText(legacyJson));Input legacyInput{legacy.root,legacy.xml,legacyJson};std::shared_ptr<Prepared> legacyPin;REQUIRE(prepare(legacyInput,legacyPin,error));TestDirectory legacyBackup;REQUIRE(legacyBackup.file.createDirectory().wasOk());auto legacyResult=backup(legacyPin,legacyBackup.file);REQUIRE(legacyResult.success && legacyResult.receipt.unknownCaptureHistory);}
    {Fixture zeros;SwellAssetStore store(zeros.root);ImpulseResponse positive;positive.channels=2;positive.rate=48000;positive.samples={1.0f,0.0f};auto negative=positive;negative.samples[1]=-0.0f;Asset a,b;REQUIRE(store.writeIR(positive,a,error) && store.writeIR(negative,b,error));REQUIRE(a.content==b.content && a.checksum!=b.checksum);zeros.session.defaultIR=a;zeros.session.importedIRs.push_back({b,"Signed-zero distinct bytes"});zeros.refresh();std::shared_ptr<Prepared> zeroPin;REQUIRE(prepare({zeros.root,zeros.xml,zeros.manifest},zeroPin,error));bool sawA=false,sawB=false;for(const auto& e:snapshot(*zeroPin).files){sawA|=e.sha256==a.checksum;sawB|=e.sha256==b.checksum;}REQUIRE(sawA && sawB);}
    // A completed object set can finish a previously interrupted operation with originals offline.
    prepared.reset();TestDirectory completion;REQUIRE(completion.file.createDirectory().wasOk());REQUIRE(prepare(only,prepared,error));Hooks beforeCompletion;beforeCompletion.checkpoint=[](const auto& event,const auto&){return event!="closure";};
    auto pending=backup(prepared,completion.file,beforeCompletion);REQUIRE(!pending.success);prepared.reset();REQUIRE(resume(completion.file,pending.operation,{}).success);

    std::cout<<"Portable assets: version/alias bounds, exact closure, backup, immutable reuse, cancel/resume, faults, corruption, source pins and destination-only restore passed.\n";
});}
