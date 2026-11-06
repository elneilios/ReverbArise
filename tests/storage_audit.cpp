// Issue #8: opt-in measurement harness. Uses only caller-owned disposable roots.
#include "AudioTestFiles.h"
#include <Capture/CaptureSession.h>
#include <Capture/SourceRangeReader.h>
#include <DSP/SwellAssetStore.h>
#include <DSP/SwellShape.h>
#include <DSP/ExportFiles.h>
#include <Core/ContentHash.h>
#include <windows.h>
#include <psapi.h>
#include <map>
#include <set>
#include <iostream>
using namespace m3;
static constexpr int rate=48000,chunk=5*rate;
static juce::var obj(){return new juce::DynamicObject;}
static void put(juce::var& v,const char* k,const juce::var& x){v.getDynamicObject()->setProperty(k,x);}
static juce::String digest(const juce::File& f){return fileHash(f);}
static void json(const juce::File& f,const juce::var& v){REQUIRE(f.replaceWithText(juce::JSON::toString(v,true)));}
static std::vector<float> signal(int64_t first,int frames,int seed,bool silence=false)
{
    std::vector<float> v(static_cast<size_t>(frames)*2);
    if(!silence)for(int n=0;n<frames;++n)for(int c=0;c<2;++c){
        // Integer-derived finite floats: deterministic, channel-distinct, nonzero.
        uint32_t x=static_cast<uint32_t>(first+n)*1664525u+1013904223u+static_cast<uint32_t>(seed*7919+c*104729);
        x^=x>>13;v[static_cast<size_t>(n)*2+c]=(static_cast<int>(x%20001)-10000)/65536.f+.000001f;
    }return v;
}
static void restore(CaptureSession& c,const CaptureSessionSnapshot& s)
{REQUIRE(c.restore(s.manifest.projectId,s.manifest.instanceId,s.reference,s.manifest.revision));}
static juce::var locator(const CaptureSessionSnapshot& s)
{auto v=obj();put(v,"project",juce::String(s.manifest.projectId));put(v,"instance",juce::String(s.manifest.instanceId));put(v,"reference",s.reference);put(v,"revision",static_cast<juce::int64>(s.manifest.revision));return v;}
static CaptureSessionSnapshot fromLocator(CaptureSession& c,const juce::var& v)
{REQUIRE(c.restore(v["project"].toString().toStdString(),v["instance"].toString().toStdString(),v["reference"].toString(),static_cast<uint64_t>(static_cast<juce::int64>(v["revision"]))));return c.snapshot();}
static double cpuSeconds()
{FILETIME a,b,k,u;REQUIRE(GetProcessTimes(GetCurrentProcess(),&a,&b,&k,&u));ULARGE_INTEGER x,y;x.LowPart=k.dwLowDateTime;x.HighPart=k.dwHighDateTime;y.LowPart=u.dwLowDateTime;y.HighPart=u.dwHighDateTime;return (x.QuadPart+y.QuadPart)/1e7;}
struct Inventory
{
    juce::File root;juce::Array<juce::var> steps;std::map<juce::String,juce::String> immutable;std::set<juce::String> selected;
    double started=juce::Time::getMillisecondCounterHiRes(),cpu=cpuSeconds();
    juce::var scan(const juce::String& label,bool final=false)
    {
        auto row=obj();put(row,"step",label);auto categories=obj();int64_t total=0,allocated=0,duplicates=0,selectedBytes=0;
        std::map<juce::String,std::pair<int64_t,int64_t>> groups;std::map<juce::String,int64_t> hashes;
        juce::Array<juce::var> files;std::set<juce::String> seen;
        for(const auto& f:root.findChildFiles(juce::File::findFiles,true)){
            const auto path=f.getRelativePathFrom(root).replaceCharacter('\\','/');if(path=="audit.json" || path.startsWith("writer-") || path=="seed.json" || path==".issue8-disposable")continue;
            const auto bytes=f.getSize();const auto hash=digest(f);seen.insert(path);
            if(const auto old=immutable.find(path);old!=immutable.end())REQUIRE(old->second==hash);
            else immutable[path]=hash;
            juce::String category="other";
            if(path.contains("/segments/"))category="capture_wav";
            else if(path.contains("/manifests/"))category="manifests";
            else if(path.contains("/irs/"))category=f.hasFileExtension("wav")?"ir_wav":"ir_json";
            else if(path.contains("/renders/"))category=f.hasFileExtension("wav")?"render_wav":"render_json";
            else if(path.contains("/exports/"))category="exports";
            else if(path.startsWith("inputs/"))category="fixture_external_ir";
            else if(path.startsWith("states/"))category="fixture_host_state";
            if(path.contains(".tmp") || path.contains(".part") || path.contains(".unpublished-"))category="scratch";
            auto& g=groups[category];g.first++;g.second+=bytes;total+=bytes;
            const auto handle=CreateFileW(f.getFullPathName().toWideCharPointer(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);REQUIRE(handle!=INVALID_HANDLE_VALUE);
            FILE_STANDARD_INFO standard{};const auto gotAllocation=GetFileInformationByHandleEx(handle,FileStandardInfo,&standard,sizeof(standard));CloseHandle(handle);REQUIRE(gotAllocation);
            const int64_t allocation=standard.AllocationSize.QuadPart;allocated+=allocation;
            if(hashes.contains(hash))duplicates+=bytes;else hashes[hash]=bytes;
            if(selected.contains(path))selectedBytes+=bytes;
            if(final){auto item=obj();put(item,"path",path);put(item,"category",category);put(item,"bytes",bytes);put(item,"allocatedBytes",allocation);put(item,"sha256",hash);put(item,"selectedCurrentDependency",selected.contains(path));files.add(item);}
        }
        for(const auto& [path,hash]:immutable)REQUIRE(seen.contains(path));
        for(const auto& [name,g]:groups){auto v=obj();put(v,"files",g.first);put(v,"bytes",g.second);categories.getDynamicObject()->setProperty(name,v);}
        put(row,"categories",categories);put(row,"files",static_cast<juce::int64>(seen.size()));put(row,"bytes",total);put(row,"allocatedBytes",allocated);put(row,"exactDuplicateBytes",duplicates);
        if(final){put(row,"selectedCurrentDependencyBytes",selectedBytes);put(row,"outsideSelectedCurrentDependenciesBytes",total-selectedBytes);}
        put(row,"wallSeconds",(juce::Time::getMillisecondCounterHiRes()-started)/1000);put(row,"cpuSeconds",cpuSeconds()-cpu);
        PROCESS_MEMORY_COUNTERS memory{};REQUIRE(GetProcessMemoryInfo(GetCurrentProcess(),&memory,sizeof(memory)));put(row,"peakWorkingSetBytes",static_cast<juce::int64>(memory.PeakWorkingSetSize));
        if(final)put(row,"inventory",files);steps.add(row);std::cout<<label<<": "<<total<<" bytes, "<<seen.size()<<" files\n"<<std::flush;return row;
    }
};
static void capturePass(CaptureSession& c,int token,int seed,bool sparse=false,const CaptureGenerationContext* context=nullptr)
{
    CaptureGenerationContext accepted;accepted.id=captureGenerationContextId(accepted);if(!context)context=&accepted; // Match workflow schema-2 publication, including dry-only passes.
    REQUIRE(c.beginPass(token,1,true,context));
    for(int second=0;second<300;second+=5){if(sparse && second%20>=15)continue;const auto input=signal(static_cast<int64_t>(second)*rate,chunk,seed,sparse && second%20>=10);
        if(!c.stagePass(token,static_cast<int64_t>(second)*rate,rate,input.data(),chunk))throw std::runtime_error("Capture at second "+std::to_string(second)+": "+c.passFailure().toStdString());}
    const auto result=c.finishPass(token,1);REQUIRE(result.success);
}
static void compatibility()
{
    juce::String error,encoded;Session original;Swell swell;swell.name="Retained";swell.shape=initialShape(rate);swell.sourceEnd=rate;swell.generationSource=juce::String::repeatedString("a",64);original.swells.push_back(swell);
    REQUIRE(encode(original,encoded,error));
    for(int version=1;version<=11;++version){auto v=juce::JSON::parse(encoded);auto* p=v.getDynamicObject();p->setProperty("version",version);
        const auto remove=[&](int introduced,std::initializer_list<const char*> fields){if(version<introduced)for(auto key:fields)p->removeProperty(key);};
        remove(11,{"autoGenerateOnStop"});remove(10,{"export"});remove(9,{"importedIRs"});remove(8,{"gainPolicy","gainRevision"});remove(7,{"isolateDry"});remove(6,{"regenerationSensitivity"});remove(5,{"liveEnabled","liveRouting","liveWetDb"});remove(4,{"autoMarkersEnabled","isolateSelected"});remove(3,{"postRollSeconds"});remove(2,{"mappingEnabled","automaticProcessing","auditionMode","wetBlend","defaultIR","defaultIRName","suppressed"});
        auto* w=v["swells"][0].getDynamicObject();if(version<11)w->removeProperty("generationSource");if(version<8)w->removeProperty("calibration");if(version<6)w->removeProperty("frozen");if(version<4)w->removeProperty("placement");if(version<3){w->removeProperty("supportOffset");v["swells"][0]["shape"].getDynamicObject()->removeProperty("inTension");v["swells"][0]["shape"].getDynamicObject()->removeProperty("outTension");}if(version<2){w->removeProperty("provenance");w->removeProperty("detectorAnchor");}
        Session decoded;REQUIRE(decode(juce::JSON::toString(v),decoded,error));REQUIRE(decoded.swells[0].id==swell.id && decoded.autoGenerateOnStop==(version<11));REQUIRE(decoded.swells[0].generationSource==(version==11?swell.generationSource:juce::String{}));
    }
    for(int version:{0,12}){auto bad=juce::JSON::parse(encoded);bad.getDynamicObject()->setProperty("version",version);Session kept=original;REQUIRE(!decode(juce::JSON::toString(bad),kept,error));juce::String after;REQUIRE(encode(kept,after,error) && after==encoded);}
    TestDirectory temp;CaptureSession c(temp.file);auto samples=signal(0,100,7);REQUIRE(c.commit(0,rate,samples.data(),100));auto v1=c.snapshot().manifest;CaptureGenerationContext context;context.origin=CaptureContextOrigin::legacy;context.id=captureGenerationContextId(context);CaptureManifest v2;REQUIRE(captureWithLegacyContext(v1,context,v2,error));
    for(const auto& m:{v1,v2}){juce::String text;CaptureManifest loaded;REQUIRE(serializeCaptureManifest(m,text,error) && parseCaptureManifest(text,loaded,error));REQUIRE(loaded.schemaVersion==m.schemaVersion && loaded.ranges.size()==1);auto bad=juce::JSON::parse(text);bad.getDynamicObject()->setProperty("schemaVersion",3);REQUIRE(!parseCaptureManifest(juce::JSON::toString(bad),loaded,error) && loaded.schemaVersion==m.schemaVersion);}
    SwellAssetStore assets(temp.file.getChildFile("managed"));ImpulseResponse ir;ir.rate=rate;ir.channels=2;ir.samples=samples;Asset known;REQUIRE(assets.writeIR(ir,known,error));auto newer=known;newer.engine=2;Asset structural;REQUIRE(parseAssetMetadata(assetMetadata(newer),structural,error) && structural.engine==2);REQUIRE(!assets.verify(newer,error) && assets.verify(known,error));
    std::cout<<"Compatibility: creative 1-11 and unknown 0/12; capture 1/2 and unknown 3 passed. Export and envelope versions covered by existing native tests.\n";
}
static void writer(const juce::File& root,int identity)
{
    REQUIRE(root.getChildFile(".issue8-disposable").existsAsFile());CaptureSession c(root.getChildFile("Projects"));fromLocator(c,juce::JSON::parse(root.getChildFile("seed.json")));
    REQUIRE(root.getChildFile("writer-ready-"+juce::String(identity)).replaceWithText("loaded"));const auto deadline=juce::Time::getMillisecondCounterHiRes()+30000;
    while(!root.getChildFile("writer-go").existsAsFile()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<deadline);juce::Thread::sleep(2);}auto input=signal(identity*rate,rate,100+identity);
    REQUIRE(!c.snapshot().manifest.generationContexts.empty());const auto pinned=c.snapshot().manifest.generationContexts.front();REQUIRE(c.beginPass(identity,7,true,&pinned));REQUIRE(c.stagePass(identity,identity*rate,rate,input.data(),rate));REQUIRE(c.finishPass(identity,7).success);json(root.getChildFile("writer-"+juce::String(identity)+".json"),locator(c.snapshot()));
}
static void growth(const juce::File& root)
{
    REQUIRE(!root.exists() && root.getFileName().startsWith("Issue8Audit-"));REQUIRE(root.createDirectory().wasOk());REQUIRE(root.getChildFile(".issue8-disposable").replaceWithText("Issue #8 synthetic fixture only; retained for inspection"));Inventory inventory{root};const auto projects=root.getChildFile("Projects");CaptureSession capture(projects);inventory.scan("empty");
    capturePass(capture,1,17);const auto first=capture.snapshot();REQUIRE(first.manifest.ranges.size()==60);for(const auto& r:first.manifest.ranges)REQUIRE(r.frameCount==chunk);const auto instance=projects.getChildFile(first.manifest.projectId).getChildFile(first.manifest.instanceId);inventory.scan("dense_300s_first");
    capturePass(capture,2,17);REQUIRE(capture.snapshot().reference==first.reference);inventory.scan("identical_dense_pass");
    capturePass(capture,3,18);inventory.scan("changed_dense_pass_2");capturePass(capture,4,19);inventory.scan("changed_dense_pass_3");
    const auto accepted=capture.snapshot();
    CaptureSession sparse(projects);CaptureGenerationContext context;context.automatic=false;context.id=captureGenerationContextId(context);capturePass(sparse,5,17,true,&context);REQUIRE(sparse.snapshot().manifest.schemaVersion==2);float left=0,right=0;REQUIRE(sparse.readSample(12*rate,left,right) && left==0 && right==0 && !sparse.readSample(17*rate,left,right));inventory.scan("sparse_300s_150audio_75silence_75gap");
    SwellAssetStore store(instance);std::vector<Asset> impulses;std::vector<ImpulseResponse> decoded;REQUIRE(root.getChildFile("inputs").createDirectory().wasOk());
    for(int i=0;i<6;++i){ImpulseResponse ir;ir.channels=2;ir.rate=rate;ir.samples=signal(0,4*rate,30+i);for(auto& sample:ir.samples)sample*=.002f;const auto file=root.getChildFile("inputs/ir-"+juce::String(i)+".wav");writeTestWav(file,2,rate,ir.samples);Asset asset;juce::String error;REQUIRE(store.importIR(file,previewMemoryBudget,asset,error));impulses.push_back(asset);decoded.push_back(std::move(ir));}inventory.scan("six_4s_ir_imports");
    for(int repeat=0;repeat<9;++repeat)for(int i=0;i<6;++i){Asset asset;juce::String error;REQUIRE(store.importIR(root.getChildFile("inputs/ir-"+juce::String(i)+".wav"),previewMemoryBudget,asset,error));REQUIRE(asset.content==impulses[i].content && asset.path!=impulses[i].path);}inventory.scan("ten_equal_sets_of_six_ir_imports");
    Session session;session.timing={true,0,0,rate,120,4,4};session.defaultIR=impulses[0];for(int i=0;i<6;++i)session.importedIRs.push_back({impulses[i],"Fixture "+juce::String(i)});
    CaptureAssetStore sourceStore(instance);juce::String error;WetRender last;
    for(int generation=0;generation<4;++generation){for(int i=0;i<40;++i){const auto at=static_cast<int64_t>(i*7+4)*rate;SourceInterval source;REQUIRE(readSourceInterval(sourceStore,accepted.manifest,at,at+4*rate,4*rate,source,error));WetRender wet;const auto ir=(i+generation)%6;REQUIRE(renderReverseConvolution(source.samples,at,rate,decoded[ir],0,wet,error));Asset asset;REQUIRE(store.writeRender(wet,impulses[ir],source.dependency,session.timing,asset,error));
            if(generation==0){Swell w;w.name="Fixture "+juce::String(i);w.arrival=at;w.sourceStart=at;w.sourceEnd=at+4*rate;w.shape=initialShape(rate);session.swells.push_back(w);}auto& w=session.swells[static_cast<size_t>(i)];w.ir=impulses[ir];w.render=asset;w.phase=Phase::ready;last=std::move(wet);
        }inventory.scan("forty_swell_generation_"+juce::String(generation+1));}
    REQUIRE(root.getChildFile("states").createDirectory().wasOk());juce::String state;REQUIRE(encode(session,state,error));REQUIRE(root.getChildFile("states/session-before-edits.json").replaceWithText(state));const auto beforeShape=inventory.scan("saved_session");
    for(auto& w:session.swells){WetRender wet,shaped;REQUIRE(store.readRender(w.render,previewMemoryBudget,wet,error));w.shape.gainDb=-6;w.name="Renamed "+w.name;REQUIRE(fullShape(w,wet.start,w.sourceEnd,error));w.shape.gainDb=-6;REQUIRE(prepareShape(w,wet,previewMemoryBudget,shaped,error));}const auto afterShape=inventory.scan("forty_name_gain_trim_shape_edits");REQUIRE(beforeShape["bytes"]==afterShape["bytes"] && beforeShape["files"]==afterShape["files"]);
    Session restoredSession;REQUIRE(decode(state,restoredSession,error));CaptureSession reopened(projects);restore(reopened,first);REQUIRE(reopened.readSample(0,left,right));const auto expected=signal(0,1,17);REQUIRE(left==expected[0]);restore(reopened,accepted);for(const auto& w:restoredSession.swells)REQUIRE(store.verify(w.ir,error) && store.verify(w.render,error));inventory.scan("old_snapshot_and_reopen");
    REQUIRE(reopened.clear(rate));restore(reopened,accepted);inventory.scan("clear_then_restore");
    // A failed manifest publication and aborted capture may leave sole source.
    CaptureSession failed(projects);restore(failed,accepted);const auto blocker=instance.getChildFile("manifests");const auto held=instance.getChildFile("held-manifests");REQUIRE(blocker.moveFileTo(held) && blocker.replaceWithText("injected obstruction"));auto changed=signal(0,rate,99);REQUIRE(!failed.commit(0,rate,changed.data(),rate,0,&context));REQUIRE(failed.snapshot().reference==accepted.reference);REQUIRE(blocker.deleteFile() && held.moveFileTo(blocker));inventory.scan("failed_snapshot_publication");
    REQUIRE(reopened.beginPass(50,1,true,&context));REQUIRE(reopened.stagePass(50,0,rate,changed.data(),rate));reopened.abortPass();REQUIRE(reopened.snapshot().reference==accepted.reference);inventory.scan("aborted_pass_retained_source");
    std::shared_ptr<const SongExportPlan> plan;REQUIRE(prepareSongExportPlan(session,accepted.manifest,accepted.reference,session.timing,.65f,.5f,plan,error));SongMix mix(instance);REQUIRE(mix.prepare(plan,previewMemoryBudget,error));REQUIRE(mix.measure(error));ExportFile exported;if(!publishSongExport(mix,instance,exported,error))throw std::runtime_error("Export publication: "+error.toStdString());REQUIRE(verifyExportFile(instance,exported,error));inventory.scan("managed_export");
    session.exportState.initialized=true;session.exportState.dry=.65f;session.exportState.wet=.5f;session.exportState.history.push_back(exported);session.exportState.completedPath=exported.path;session.exportState.desiredIdentity=exported.composition;
    REQUIRE(encode(session,state,error));REQUIRE(root.getChildFile("states/session-after-export.json").replaceWithText(state));
    const auto selectFile=[&](const juce::File& f){inventory.selected.insert(f.getRelativePathFrom(root).replaceCharacter('\\','/'));};
    const auto selectCapture=[&](const CaptureSessionSnapshot& s){const auto dir=projects.getChildFile(s.manifest.projectId).getChildFile(s.manifest.instanceId);selectFile(dir.getChildFile(s.reference));for(const auto& range:s.manifest.ranges)if(range.kind==CaptureCoverageKind::audio)selectFile(dir.getChildFile(range.assetPath));};
    selectCapture(accepted);selectCapture(sparse.snapshot());for(const auto& ir:impulses){selectFile(instance.getChildFile(ir.path));selectFile(instance.getChildFile(ir.path).withFileExtension(".json"));}for(const auto& w:session.swells){selectFile(instance.getChildFile(w.render.path));selectFile(instance.getChildFile(w.render.path).withFileExtension(".json"));}selectFile(instance.getChildFile(exported.path));
    json(root.getChildFile("seed.json"),locator(accepted));
    auto report=obj();put(report,"format",1);put(report,"sourceBaseline","df765d95170caaaab10a646e43088ea3be66ce53");put(report,"rate",rate);put(report,"durationSeconds",300);put(report,"seed",17);put(report,"swellCount",40);put(report,"renderGenerations",4);put(report,"renderSourceSeconds",4);put(report,"irSeconds",4);put(report,"original",locator(first));put(report,"accepted",locator(accepted));put(report,"sparse",locator(sparse.snapshot()));put(report,"steps",inventory.steps);put(report,"final",inventory.scan("final_before_concurrent_writers",true));put(report,"steps",inventory.steps);json(root.getChildFile("audit.json"),report);
}
static void verifyWriters(const juce::File& root)
{
    auto report=juce::JSON::parse(root.getChildFile("audit.json"));const auto projects=root.getChildFile("Projects");CaptureSession original(projects);const auto seed=fromLocator(original,report["accepted"]);float l,r;REQUIRE(original.readSample(0,l,r));const auto before=signal(0,1,19);REQUIRE(l==before[0]);juce::String refs[2];
    for(int n=0;n<2;++n){const int id=n+1;CaptureSession clone(projects);const auto next=fromLocator(clone,juce::JSON::parse(root.getChildFile("writer-"+juce::String(id)+".json")));REQUIRE(next.manifest.projectId==seed.manifest.projectId && next.manifest.instanceId==seed.manifest.instanceId);refs[n]=next.reference;REQUIRE(next.reference!=seed.reference);REQUIRE(clone.readSample(id*rate,l,r));const auto changed=signal(id*rate,1,100+id);REQUIRE(l==changed[0]);const auto other=(id==1?2:1)*rate;REQUIRE(clone.readSample(other,l,r));const auto unchanged=signal(other,1,19);REQUIRE(l==unchanged[0]);}
    REQUIRE(refs[0]!=refs[1]);const auto final=report["final"]["inventory"];for(const auto& item:*final.getArray()){const auto file=root.getChildFile(item["path"].toString());REQUIRE(file.existsAsFile() && digest(file)==item["sha256"].toString());}
    put(report,"concurrentProcesses",2);put(report,"concurrentSharedIdsIndependentMaps",true);put(report,"priorFilesUnchangedAfterConcurrentWrites",true);Inventory inventory{root};for(const auto& item:*final.getArray())if(static_cast<bool>(item["selectedCurrentDependency"]))inventory.selected.insert(item["path"].toString());put(report,"afterConcurrentWriters",inventory.scan("after_concurrent_writers",true));json(root.getChildFile("audit.json"),report);std::cout<<"Two processes: distinct accepted maps, shared IDs; every prior file hash unchanged.\n";
}
int main(int argc,char** argv){return runTest([&]{if(argc==1){compatibility();return;}REQUIRE(argc>=3);const juce::String mode(argv[1]);const juce::File root(argv[2]);if(mode=="--growth")growth(root);else if(mode=="--verify-writers")verifyWriters(root);else if(mode=="--writer"){REQUIRE(argc==4);writer(root,juce::String(argv[3]).getIntValue());}else REQUIRE(false);});}
