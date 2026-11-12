#include "AudioTestFiles.h"
#include <Storage/AssetInventory.h>
#include <Capture/CaptureSession.h>
#include <DSP/SwellAssetStore.h>
#include <Core/ContentHash.h>
#include <windows.h>
#include <winioctl.h>
using namespace m3;
using namespace m3::storage;
struct Fixture
{
    TestDirectory directory;CaptureSession capture{directory.file};juce::File root;
    Session session;std::vector<Asset> irs;ObservedSnapshot observed;juce::String xml,manifest,error;
    Fixture()
    {
        const auto ids=capture.snapshot().manifest;root=directory.file.getChildFile(ids.projectId).getChildFile(ids.instanceId);SwellAssetStore store(root);
        for(int i=0;i<4;++i){ImpulseResponse ir;ir.channels=2;ir.rate=48000;ir.samples={.1f+i*.01f,-.1f,0,0};Asset a;REQUIRE(store.writeIR(ir,a,error));irs.push_back(a);}
        CaptureGenerationContext context;context.ir=irs[0];context.irName="Historical IR";context.automatic=false;context.id=captureGenerationContextId(context);
        const float audio[]{.1f,-.2f,.3f,-.4f},silence[4]{};
        REQUIRE(capture.commit(0,48000,audio,2,0,&context));REQUIRE(capture.commit(4,48000,silence,2,0,&context));
        session.defaultIR=irs[1];session.importedIRs.push_back({irs[2],"Unused retained library entry"});session.timing={true,0,0,48000,120,4,4};
        Swell w;w.name="Retained disabled/frozen";w.arrival=0;w.sourceStart=0;w.sourceEnd=2;w.shape=initialShape(48000);w.enabled=false;w.frozen=true;w.ir=irs[3];w.generationSource=juce::String::repeatedString("a",64);
        WetRender wet;wet.rate=48000;wet.start=-1;wet.sourceStart=0;wet.sourceEnd=2;wet.samples={0,0,.1f,-.2f,.3f,-.4f};
        REQUIRE(store.writeRender(wet,w.ir,"source",session.timing,w.render,error));session.swells.push_back(w);
        REQUIRE(root.getChildFile("exports").createDirectory().wasOk());const auto exported=root.getChildFile("exports/Swells-old.wav");writeTestWav(exported,2,48000,audioVector());
        ExportFile file;file.path="exports/Swells-old.wav";file.checksum=fileHash(exported);file.composition=juce::String::repeatedString("b",64);file.rate=48000;file.frames=2;file.dry=.35f;file.wet=.65f;file.gainLaw=1;
        session.exportState.initialized=true;session.exportState.history.push_back(file);session.exportState.completedPath=file.path;
        refresh();
    }
    static std::vector<float> audioVector(){return {.1f,-.2f,.3f,-.4f};}
    void refresh(int envelope=8)
    {
        const auto snapshot=capture.snapshot();REQUIRE(serializeCaptureManifest(snapshot.manifest,manifest,error));juce::String creative;REQUIRE(encode(session,creative,error));
        juce::XmlElement state("PreverbState");state.setAttribute("stateVersion",envelope);state.setAttribute("projectId",juce::String(snapshot.manifest.projectId));state.setAttribute("instanceId",juce::String(snapshot.manifest.instanceId));
        state.setAttribute("manifestReference",snapshot.reference);state.setAttribute("manifestRevision",juce::String(static_cast<juce::int64>(snapshot.manifest.revision)));state.createNewChildElement("SwellSession")->addTextElement(creative);xml=state.toString();
        observed.label="Saved fixture";REQUIRE(readLegacySnapshot(xml,manifest,observed,error));
    }
    Request request(){Request r;r.roots={root};r.snapshots={observed};return r;}
};
static void waitFor(InventoryService& service){const auto end=juce::Time::getMillisecondCounterHiRes()+30000;while(service.busy()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<end);juce::Thread::sleep(2);}}
static void junction(const juce::File& link,const juce::File& target)
{
    REQUIRE(link.createDirectory().wasOk());
    struct Mount {DWORD tag;USHORT size,reserved,subOffset,subLength,printOffset,printLength;wchar_t path[2048];} data{};
    const auto text=juce::String("\\??\\")+target.getFullPathName();const auto length=static_cast<USHORT>(text.length()*sizeof(wchar_t));REQUIRE(length+2<sizeof(data.path));
    data.tag=IO_REPARSE_TAG_MOUNT_POINT;data.size=static_cast<USHORT>(8+length+4);data.subLength=length;data.printOffset=static_cast<USHORT>(length+2);
    std::memcpy(data.path,text.toWideCharPointer(),length);const auto handle=CreateFileW(link.getFullPathName().toWideCharPointer(),GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS,nullptr);REQUIRE(handle!=INVALID_HANDLE_VALUE);
    DWORD written=0;const auto success=DeviceIoControl(handle,FSCTL_SET_REPARSE_POINT,&data,data.size+8,nullptr,0,&written,nullptr);CloseHandle(handle);REQUIRE(success);
}
int main(){return runTest([]{
    Fixture f;const auto contains=[&](const juce::String& path){return std::any_of(f.observed.dependencies.begin(),f.observed.dependencies.end(),[&](const auto& d){return d.path==path;});};
    for(const auto& a:f.irs)REQUIRE(contains(a.path));REQUIRE(contains(f.session.swells[0].render.path) && contains("exports/Swells-old.wav"));
    REQUIRE(f.observed.capture.schemaVersion==2 && f.observed.capture.contextSpans.size()==2 && f.observed.session.swells[0].frozen && !f.observed.session.swells[0].enabled);
    auto noSwells=f.observed;noSwells.session.swells.clear();noSwells.session.defaultIR={};noSwells.session.defaultSoundPreset={};REQUIRE(visitDependencies(noSwells,f.error));REQUIRE(noSwells.emptyIRReferences==1);
    REQUIRE(std::any_of(noSwells.dependencies.begin(),noSwells.dependencies.end(),[&](const auto& d){return d.path==f.irs[0].path;}));
    auto defaultOnly=f.observed;defaultOnly.session.swells.clear();REQUIRE(visitDependencies(defaultOnly,f.error));REQUIRE(std::any_of(defaultOnly.dependencies.begin(),defaultOnly.dependencies.end(),[&](const auto& d){return d.path==f.irs[1].path;}));
    REQUIRE(!readLegacySnapshot(f.xml,juce::String::repeatedString(" ",static_cast<int>(maximumMetadataBytes+1)),f.observed,f.error));
    REQUIRE(!readLegacySnapshot("<!DOCTYPE PreverbState>"+f.xml,f.manifest,f.observed,f.error));
    const auto original=f.observed.dependencies.size();auto invalid=juce::XmlDocument::parse(f.xml);invalid->setAttribute("stateVersion",9);REQUIRE(!readLegacySnapshot(invalid->toString(),f.manifest,f.observed,f.error) && f.observed.dependencies.size()==original);
    auto badManifest=juce::JSON::parse(f.manifest);badManifest.getDynamicObject()->setProperty("schemaVersion",3);REQUIRE(!readLegacySnapshot(f.xml,juce::JSON::toString(badManifest),f.observed,f.error));
    auto future=f.observed;future.session.defaultIR.engine=2;REQUIRE(!visitDependencies(future,f.error) && future.dependencies.size()==original);
    auto badExport=juce::XmlDocument::parse(f.xml);auto creative=juce::JSON::parse(badExport->getChildByName("SwellSession")->getAllSubText());creative["export"].getDynamicObject()->setProperty("version",3);badExport->getChildByName("SwellSession")->deleteAllChildElements();badExport->getChildByName("SwellSession")->addTextElement(juce::JSON::toString(creative));REQUIRE(!readLegacySnapshot(badExport->toString(),f.manifest,f.observed,f.error));
    auto report=inspect(f.request());if(!report.inventoryComplete)for(const auto& issue:report.issues)std::cerr<<issue<<'\n';
    REQUIRE(report.inventoryComplete && report.dependencyVerificationComplete && report.missingReferences==0 && report.observedCaptureSnapshots==2);
    uint64_t total=0,count=0;std::map<juce::String,juce::String> hashes;for(const auto& file:report.files){total+=file.bytes;++count;hashes[file.path]=file.sha256;}
    REQUIRE(report.observedReferenceBytes>0 && report.observedReferenceBytes+report.protectedHistoryOrUnknownBytes==report.physicalBytes);
    REQUIRE(total==report.logicalBytes && count==report.physicalFiles && report.reclaimableBytes==0 && !report.catalog.uncertainty.empty());
    REQUIRE(report.catalog.protectedByDefault("anything") && report.catalog.aliases.contains(f.irs[0].content));
    // Same file reached through overlapping roots and hardlink aliases counts once physically.
    auto repeated=f.request();repeated.roots.push_back(f.root);auto twice=inspect(repeated);REQUIRE(twice.physicalBytes==report.physicalBytes && twice.logicalBytes==report.logicalBytes);
    const auto originalFile=f.root.getChildFile(f.irs[0].path),hard=f.root.getChildFile("hard.wav");REQUIRE(CreateHardLinkW(hard.getFullPathName().toWideCharPointer(),originalFile.getFullPathName().toWideCharPointer(),nullptr));
    const auto copied=f.root.getChildFile("copy.wav");REQUIRE(originalFile.copyFileTo(copied));auto duplicates=inspect(f.request());REQUIRE(duplicates.physicalFiles==report.physicalFiles+1 && duplicates.exactDuplicateBytes>=static_cast<uint64_t>(copied.getSize()) && duplicates.decodedEquivalentFiles>=2);
    const auto signedFile=f.root.getChildFile("signed-zero.wav");writeTestWav(signedFile,2,48000,{.1f,-.1f,0.f,-0.f});REQUIRE(fileHash(signedFile)!=fileHash(originalFile));auto equivalent=inspect(f.request());REQUIRE(equivalent.decodedEquivalentFiles>=3 && equivalent.reclaimableBytes==0);
    const auto outside=f.directory.file.getChildFile("outside");REQUIRE(outside.createDirectory().wasOk());REQUIRE(outside.getChildFile("secret.txt").replaceWithText("not in selected roots"));const auto link=f.root.getChildFile("linked");junction(link,outside);const auto linked=inspect(f.request());REQUIRE(!linked.inventoryComplete);for(const auto& file:linked.files)REQUIRE(!file.path.contains("secret.txt"));REQUIRE(RemoveDirectoryW(link.getFullPathName().toWideCharPointer()));
    auto changing=f.request();changing.snapshots.clear();changing.observationError="Accepted references changed during observation";const auto uncertain=inspect(changing);REQUIRE(!uncertain.inventoryComplete && !uncertain.dependencyVerificationComplete && uncertain.reclaimableBytes==0);
    auto missing=f.request();missing.roots={f.root.getChildFile("offline")};auto offline=inspect(missing);REQUIRE(!offline.inventoryComplete && !offline.dependencyVerificationComplete && offline.missingReferences>0 && offline.reclaimableBytes==0);
    auto limited=f.request();limited.limits.entries=1;REQUIRE(!inspect(limited).inventoryComplete);limited=f.request();limited.limits.hashBytes=1;REQUIRE(!inspect(limited).dependencyVerificationComplete);
    auto cancelled=inspect(f.request(),[]{return true;});REQUIRE(cancelled.cancelled && !cancelled.inventoryComplete && cancelled.reclaimableBytes==0);
    const auto unknown=f.root.getChildFile("store.json");REQUIRE(unknown.replaceWithText("{\"version\":999}"));REQUIRE(!inspect(f.request()).inventoryComplete);REQUIRE(unknown.deleteFile());
    const auto unicode=f.root.getChildFile(juce::String::fromUTF8("unicode-\xc3\xa9"));REQUIRE(unicode.createDirectory().wasOk());writeTestWav(unicode.getChildFile("audio.wav"),2,48000,{.2f,-.2f});REQUIRE(inspect(f.request()).inventoryComplete);
    // Missing/corrupt index rebuilds only observations; it never frees unknown durable data.
    Catalog catalog;catalog.rebuild({f.observed},{{"Active export","exports/Swells-old.wav"}},false);REQUIRE(catalog.references.contains("exports/swells-old.wav"));catalog.rebuild({}, {},false);REQUIRE(catalog.protectedByDefault(f.irs[0].path) && !catalog.uncertainty.empty());
    const auto locked=CreateFileW(originalFile.getFullPathName().toWideCharPointer(),GENERIC_READ,0,nullptr,OPEN_EXISTING,0,nullptr);REQUIRE(locked!=INVALID_HANDLE_VALUE);const auto lockedReport=inspect(f.request());CloseHandle(locked);REQUIRE(!lockedReport.inventoryComplete && !lockedReport.dependencyVerificationComplete);
    REQUIRE(SetFileAttributesW(originalFile.getFullPathName().toWideCharPointer(),FILE_ATTRIBUTE_READONLY));REQUIRE(inspect(f.request()).inventoryComplete);REQUIRE(SetFileAttributesW(originalFile.getFullPathName().toWideCharPointer(),FILE_ATTRIBUTE_NORMAL));
    InventoryService service;service.request(missing);service.request(f.request());waitFor(service);REQUIRE(service.snapshot() && service.snapshot()->inventoryComplete);service.cancel();REQUIRE(!service.snapshot());
    for(const auto& [path,hash]:hashes)REQUIRE(fileHash(f.root.getChildFile(path))==hash);
    // Legacy/unversioned adapters remain read-only and do not fabricate contexts.
    auto legacy=f.observed;legacy.capture.schemaVersion=1;legacy.capture.generationContexts.clear();legacy.capture.contextSpans.clear();REQUIRE(visitDependencies(legacy,f.error) && legacy.unknownCaptureHistory);
    for(int envelope=1;envelope<=8;++envelope)f.refresh(envelope);
    std::cout<<"Inventory: dependency closure, version refusal, immutable hashes, counts, aliases, hardlinks, limits, cancellation, offline roots, catalog rebuild and stale-request publication passed.\n";
});}
