#pragma once
#include <Windows.h>
#include <Sddl.h>
#include <UserEnv.h>
#undef small
// Credential launches inherit the caller environment. Load the actual token
// profile environment before JUCE resolves any special folders.
static void initialiseProfileEnvironment()
{
    HANDLE token=nullptr;REQUIRE(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY|TOKEN_DUPLICATE,&token));
    void* environment=nullptr;REQUIRE(CreateEnvironmentBlock(&environment,token,FALSE));CloseHandle(token);
    for(auto* entry=static_cast<wchar_t*>(environment);*entry;entry+=wcslen(entry)+1){
        auto* equals=wcschr(entry,L'=');if(!equals || equals==entry)continue;
        const std::wstring name(entry,equals);REQUIRE(SetEnvironmentVariableW(name.c_str(),equals+1));
    }
    DestroyEnvironmentBlock(environment);
}
static juce::String validationSid()
{
    HANDLE token=nullptr;REQUIRE(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token));DWORD count=0;
    GetTokenInformation(token,TokenUser,nullptr,0,&count);std::vector<char> data(count);REQUIRE(GetTokenInformation(token,TokenUser,data.data(),count,&count));CloseHandle(token);
    LPWSTR sid=nullptr;REQUIRE(ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER*>(data.data())->User.Sid,&sid));juce::String value(sid);LocalFree(sid);return value;
}
static juce::String validationProfile()
{HANDLE token=nullptr;REQUIRE(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token));wchar_t path[32768]{};DWORD count=32768;REQUIRE(GetUserProfileDirectoryW(token,path,&count));CloseHandle(token);return juce::String(path);}
struct ProfileHost:juce::AudioPlayHead
{PositionInfo position;juce::Optional<PositionInfo> getPosition()const override{return position;}};
static void profilePrepare(const juce::File& bridge)
{
    auto bootstrap=juce::var(new juce::DynamicObject);bootstrap.getDynamicObject()->setProperty("ownerSid",validationSid());juce::Array<juce::var> cases;
    for(const auto name:{"full","context","unfrozen"}){
        Fixture fixture;
        if(juce::String(name)=="context"){fixture.session.swells.clear();fixture.session.defaultIR={};fixture.session.importedIRs.clear();}
        else{
            fixture.session.primary=fixture.session.swells[0].id;fixture.session.selected={fixture.session.primary};fixture.session.swells[0].name="Preserved recovery edit";
            fixture.session.swells[0].shape.gainDb=-3;fixture.session.swells[0].shape.endOffset=2;fixture.session.autoGenerateOnStop=true;
            auto history=fixture.session.exportState.history.front();history.path="exports/Swells-current.wav";history.gainLaw=2;history.dry=0;history.wet=1;
            REQUIRE(fixture.root.getChildFile("exports/Swells-old.wav").copyFileTo(fixture.root.getChildFile(history.path)));
            fixture.session.exportState.history.push_back(history);fixture.session.exportState.completedPath=history.path;
        }
        fixture.refresh();
        if(juce::String(name)=="unfrozen"){
            auto& w=fixture.session.swells[0];w.frozen=false;w.shape.fadeIn=w.shape.fadeOut=0;
            fixture.session.automaticProcessing=false;fixture.session.autoMarkersEnabled=false;fixture.session.autoGenerateOnStop=false;fixture.refresh();
            auto owner=std::make_unique<PreverbAudioProcessor>(fixture.directory.file);load(*owner,fixture.xml);
            ProfileHost host;host.position.setTimeInSamples(0);host.position.setPpqPosition(0.0);host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});host.position.setIsPlaying(false);
            owner->setPlayHead(&host);owner->prepareToPlay(48000,1);juce::AudioBuffer<float> audio(2,1);audio.clear();juce::MidiBuffer midi;owner->processBlock(audio,midi);
            waitFor([&]{return owner->swellEngine().savedSession().swells[0].phase==m3::Phase::ready;});
            REQUIRE(owner->swellEngine().snapshot().renderCount==1);fixture.session=owner->swellEngine().savedSession();owner->setPlayHead(nullptr);owner.reset();fixture.refresh();
        }
        const auto folder=bridge.getChildFile(name);REQUIRE(folder.createDirectory().wasOk());const auto assets=folder.getChildFile("assets");REQUIRE(assets.createDirectory().wasOk());
        std::shared_ptr<m3::storage::portable::Prepared> prepared;juce::String error;REQUIRE(m3::storage::portable::prepare({fixture.root,fixture.xml,fixture.manifest},prepared,error));
        const auto result=m3::storage::portable::backup(prepared,assets);if(!result.success)throw std::runtime_error(result.error.toStdString());
        REQUIRE(folder.getChildFile("state.xml").replaceWithText(fixture.xml));auto record=juce::var(new juce::DynamicObject);
        record.getDynamicObject()->setProperty("name",name);record.getDynamicObject()->setProperty("originalRoot",fixture.root.getFullPathName());
        record.getDynamicObject()->setProperty("receipt",result.receipt.locator.snapshot+".json");
        if(!fixture.session.swells.empty()){m3::SwellAssetStore store(fixture.root);WetRender wet;REQUIRE(store.readRender(fixture.session.swells[0].render,previewMemoryBudget,wet,error));record.getDynamicObject()->setProperty("peak",wet.peak);record.getDynamicObject()->setProperty("start",static_cast<juce::int64>(wet.start));}
        cases.add(record);
    }
    bootstrap.getDynamicObject()->setProperty("cases",cases);REQUIRE(bridge.getChildFile("bootstrap.json").replaceWithText(juce::JSON::toString(bootstrap,true)));
}
static void profileValidate(const juce::File& bridge,const juce::String& expectedUser)
{
    wchar_t username[256]{};DWORD chars=256;REQUIRE(GetUserNameW(username,&chars));REQUIRE(juce::String(username)==expectedUser);
    const auto bootstrap=juce::JSON::parse(bridge.getChildFile("bootstrap.json").loadFileAsString());const auto sid=validationSid();REQUIRE(sid!=bootstrap["ownerSid"].toString());
    const auto profile=validationProfile();const auto root=PreverbAudioProcessor::defaultProjectsRoot();std::cerr<<"Token profile: "<<profile<<"; projects: "<<root.getFullPathName()<<"\n";REQUIRE(root.isAChildOf(juce::File(profile)) && root.createDirectory().wasOk());
    juce::Array<juce::var> reports;
    for(const auto& record:*bootstrap["cases"].getArray()){
        REQUIRE(!juce::File(record["originalRoot"].toString()).exists());const auto folder=bridge.getChildFile(record["name"].toString());juce::String error;
        std::shared_ptr<m3::storage::portable::Verified> verified;REQUIRE(m3::storage::portable::locateReceipt(folder.getChildFile("assets/receipts/"+record["receipt"].toString()),verified,error));
        const auto& saved=m3::storage::portable::snapshot(*verified);const auto work=renderWorkCounts();
        auto owner=std::make_unique<PreverbAudioProcessor>();auto& processor=*owner;load(processor,folder.getChildFile("state.xml").loadFileAsString());
        REQUIRE(processor.getCaptureSnapshot().unavailable);m3::storage::portable::Input scope;REQUIRE(processor.pinPortableAssets(scope,error,true));
        juce::String before;REQUIRE(m3::encode(processor.swellEngine().savedSession(),before,error));
        REQUIRE(m3::storage::portable::matchesRecoveryTarget(*verified,scope,error));REQUIRE(m3::storage::portable::restoreCopy(verified,root).success);
        REQUIRE(processor.reloadRecoveredAssets(scope,error));REQUIRE(processor.flushCaptureForTesting());REQUIRE(!processor.getCaptureSnapshot().unavailable);
        ProfileHost host;host.position.setTimeInSamples(0);host.position.setPpqPosition(0.0);host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});host.position.setIsPlaying(false);
        processor.setPlayHead(&host);processor.prepareToPlay(48000,1);juce::AudioBuffer<float> stopped(2,1);stopped.clear();juce::MidiBuffer midi;processor.processBlock(stopped,midi);
        if(!saved.observed.session.swells.empty()){const auto end=juce::Time::getMillisecondCounterHiRes()+15000;while(processor.swellEngine().savedSession().swells[0].phase!=m3::Phase::ready){if(juce::Time::getMillisecondCounterHiRes()>=end)throw std::runtime_error(processor.swellEngine().savedSession().swells[0].status.toStdString());juce::Thread::sleep(2);}}
        const auto instance=root.getChildFile(saved.observed.project).getChildFile(saved.observed.instance);
        for(const auto& entry:saved.files)REQUIRE(m3::fileHash(instance.getChildFile(entry.path))==entry.sha256);
        CaptureSession capture(root);REQUIRE(capture.restore(saved.observed.project.toStdString(),saved.observed.instance.toStdString(),saved.observed.reference,saved.observed.capture.revision));
        float a=0,b=0;REQUIRE(capture.readSample(0,a,b) && a==.1f && b==-.2f);REQUIRE(!capture.readSample(3,a,b));REQUIRE(capture.readSample(4,a,b) && a==0 && b==0);
        REQUIRE(processor.getCaptureSnapshot().manifest.contextSpans==saved.observed.capture.contextSpans);
        juce::String after;REQUIRE(m3::encode(processor.swellEngine().savedSession(),after,error));REQUIRE(after==before);
        if(!saved.observed.session.swells.empty()){m3::SwellAssetStore store(instance);WetRender wet;REQUIRE(store.readRender(saved.observed.session.swells[0].render,previewMemoryBudget,wet,error));REQUIRE(wet.peak==static_cast<float>(static_cast<double>(record["peak"])) && wet.start==static_cast<juce::int64>(record["start"]) && wet.sourceStart==0 && wet.sourceEnd==2);}
        const auto done=renderWorkCounts();REQUIRE(done.convolutions==work.convolutions && done.resamplings==work.resamplings && processor.captureStageCountForTesting()==0 && processor.swellEngine().snapshot().renderCount==0);
        auto result=juce::var(new juce::DynamicObject);auto* o=result.getDynamicObject();o->setProperty("case",record["name"]);o->setProperty("files",static_cast<int>(saved.files.size()));o->setProperty("hashesExact",true);o->setProperty("metadataExact",true);o->setProperty("convolutions",0);o->setProperty("resamplings",0);o->setProperty("captureStages",0);reports.add(result);
        processor.setPlayHead(nullptr);
    }
    auto result=juce::var(new juce::DynamicObject);result.getDynamicObject()->setProperty("sid",sid);result.getDynamicObject()->setProperty("profile",profile);result.getDynamicObject()->setProperty("projectsRoot",root.getFullPathName());result.getDynamicObject()->setProperty("cases",reports);
    REQUIRE(bridge.getChildFile("profile-results.json").replaceWithText(juce::JSON::toString(result,true)));std::cout<<"Fresh Windows profile: exact source/wet/export/context hashes, metadata, peaks and zero capture/convolution/resampling verified.\n";
}
