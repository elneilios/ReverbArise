#pragma once

#include "ScratchPreviewFixture.h"

static void holdScratchLease(const juce::File& bridge)
{
    REQUIRE(bridge.getChildFile("ready").replaceWithText("ready"));
    const auto deadline=juce::Time::getMillisecondCounterHiRes()+20000;
    while(!bridge.getChildFile("release").existsAsFile()){
        REQUIRE(juce::Time::getMillisecondCounterHiRes()<deadline);
        juce::Thread::sleep(5);
    }
}

static void scratchLeaseChild(const juce::String& mode,const juce::File& store,const juce::File& source,
                              const juce::String& selected,const juce::File& bridge)
{
    using namespace m3::storage::portable;
    juce::String error;
    if(mode=="reader"){
        std::shared_ptr<Verified> pack;
        REQUIRE(locateReceipt(juce::File(selected),pack,error));
        holdScratchLease(bridge);
    }else if(mode=="resume"){
        bool entered=false;
        Hooks hooks;
        hooks.checkpoint=[&](const auto& point,const auto&){
            if(point!="journal")return true;
            entered=true;
            holdScratchLease(bridge);
            return false; // Stop this disposable resume before copying any object.
        };
        REQUIRE(!resume(store,selected,source,hooks).success && entered);
    }else{
        REQUIRE(mode=="maintenance");
        const auto preview=previewScratch(store,source);
        REQUIRE(preview.complete && preview.eligible.size()==1);
        REQUIRE(withRevalidatedScratch(store,source,preview,selected,[&](const auto&){holdScratchLease(bridge);},error));
    }
}

struct ScratchLeaseProcess
{
    TestDirectory bridge;
    juce::ChildProcess child;

    ~ScratchLeaseProcess()
    {
        if(child.isRunning()){
            bridge.file.getChildFile("release").replaceWithText("release");
            child.waitForProcessToFinish(5000);
        }
    }

    void begin(const ScratchPreviewFixture& fixture,const juce::String& mode,const juce::String& selected)
    {
        REQUIRE(bridge.file.createDirectory().wasOk());
        const auto executable=juce::File::getSpecialLocation(juce::File::currentExecutableFile);
        REQUIRE(child.start(juce::StringArray{executable.getFullPathName(),"--scratch-lease-child",mode,
            fixture.store.file.getFullPathName(),fixture.original.root.getFullPathName(),selected,bridge.file.getFullPathName()}));
        const auto deadline=juce::Time::getMillisecondCounterHiRes()+12000;
        while(!bridge.file.getChildFile("ready").existsAsFile()){
            if(!child.isRunning())throw std::runtime_error(child.readAllProcessOutput().toStdString());
            REQUIRE(juce::Time::getMillisecondCounterHiRes()<deadline);
            juce::Thread::sleep(5);
        }
    }

    void end()
    {
        REQUIRE(bridge.file.getChildFile("release").replaceWithText("release"));
        REQUIRE(child.waitForProcessToFinish(5000));
        if(child.getExitCode()!=0)throw std::runtime_error(child.readAllProcessOutput().toStdString());
    }
};

static void scratchMaintenanceCases()
{
    using namespace m3::storage::portable;
    ScratchPreviewFixture f;
    const auto& store=f.store.file;
    const auto& source=f.original.root;
    const auto stage=store.getChildFile(f.path);
    juce::String error;
    std::shared_ptr<Prepared> prepared;
    Input otherPack{source,f.original.xml,f.original.manifest};
    otherPack.producer="Separate completed pack for a concurrent reader";
    REQUIRE(prepare(otherPack,prepared,error));
    const auto committed=backup(prepared,store);
    REQUIRE(committed.success);
    const auto receiptPath=store.getChildFile("receipts/"+committed.receipt.locator.snapshot+".json");
    prepared.reset();
    const auto preview=previewScratch(store,source);
    REQUIRE(preview.complete && preview.eligible.size()==1 && preview.eligible[0].path==f.path);
    int admissions=0;
    const auto ready=[&](const auto&){++admissions;};
    const auto admit=[&](const Hooks& hooks=Hooks{}){return withRevalidatedScratch(store,source,preview,f.path,ready,error,hooks);};
    REQUIRE(admit() && admissions==1);
    REQUIRE(!withRevalidatedScratch(store,source,preview,"exports/playlist.wav",ready,error));
    auto incomplete=preview;incomplete.complete=false;
    REQUIRE(!withRevalidatedScratch(store,source,incomplete,f.path,ready,error));

    {
        ScratchLeaseProcess reader;
        reader.begin(f,"reader",receiptPath.getFullPathName());
        REQUIRE(!admit() && error.contains("reserve"));
        reader.end();
    }
    {
        ScratchLeaseProcess writer;
        writer.begin(f,"resume",f.operation);
        REQUIRE(!admit() && error.contains("reserve"));
        writer.end();
    }
    {
        ScratchLeaseProcess maintainer;
        maintainer.begin(f,"maintenance",f.path);
        REQUIRE(!admit() && error.contains("reserve"));
        std::shared_ptr<Verified> read;
        REQUIRE(!locateReceipt(receiptPath,read,error));
        REQUIRE(!resume(store,f.operation,source).success);
        REQUIRE(prepare(otherPack,prepared,error));
        REQUIRE(!backup(prepared,store).success);
        prepared.reset();
        const auto moved=stage.getSiblingFile("must-stay-pinned");
        REQUIRE(!MoveFileExW(stage.getFullPathName().toWideCharPointer(),moved.getFullPathName().toWideCharPointer(),0));
        REQUIRE(!moved.exists());

        ScratchPreviewFixture independent;
        const auto other=previewScratch(independent.store.file,independent.original.root);
        REQUIRE(withRevalidatedScratch(independent.store.file,independent.original.root,other,independent.path,[](const auto&){},error));
        maintainer.end();
    }
    REQUIRE(m3::fileHash(stage)==f.sha && admit() && admissions==2);
    const auto journal=parse(f.journal().loadFileAsString());
    const auto original=source.getChildFile(journal["alias"].toString());
    for(const auto& locked:{stage,original,store.getChildFile("transactions/"+f.operation+"/plan.json")}){
        const auto handle=CreateFileW(locked.getFullPathName().toWideCharPointer(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
        REQUIRE(handle!=INVALID_HANDLE_VALUE);
        const auto accepted=admit();
        CloseHandle(handle);
        REQUIRE(accepted==(locked==original)); // Shared immutable counterpart readers remain safe.
    }
    REQUIRE(admissions==3);
    const auto originalHeld=CreateFileW(original.getFullPathName().toWideCharPointer(),GENERIC_READ,0,nullptr,OPEN_EXISTING,0,nullptr);
    REQUIRE(originalHeld!=INVALID_HANDLE_VALUE);
    const auto originalBusy=admit();
    CloseHandle(originalHeld);
    REQUIRE(!originalBusy);

    const auto retained=original.getSiblingFile("retained-counterpart");
    REQUIRE(original.moveFileTo(retained) && retained.copyFileTo(original));
    REQUIRE(!admit() && error.contains("surviving copy changed"));
    REQUIRE(original.deleteFile() && retained.moveFileTo(original));

    TestDirectory outside;
    REQUIRE(outside.file.createDirectory().wasOk());
    const auto staging=stage.getParentDirectory(),oldStaging=staging.getSiblingFile("retained-staging");
    const auto outsideFile=outside.file.getChildFile(stage.getFileName());
    REQUIRE(stage.copyFileTo(outsideFile));
    Hooks swap;
    swap.checkpoint=[&](const auto& point,const auto&){
        if(point=="maintenance-locked"){
            REQUIRE(staging.moveFileTo(oldStaging));
            junction(staging,outside.file);
        }
        return true;
    };
    const auto escaped=admit(swap);
    REQUIRE(RemoveDirectoryW(staging.getFullPathName().toWideCharPointer()) && oldStaging.moveFileTo(staging));
    REQUIRE(!escaped && m3::fileHash(outsideFile)==f.sha && m3::fileHash(stage)==f.sha);

    bool stop=false;
    Hooks cancel;
    cancel.cancelled=[&]{return stop;};
    cancel.checkpoint=[&](const auto& point,const auto&){if(point=="scratch-evidence-checked")stop=true;return true;};
    REQUIRE(!admit(cancel) && error.containsIgnoreCase("cancelled"));

    const auto resumed=resume(store,f.operation,source);
    REQUIRE(resumed.success && resumed.receipt.locator.snapshot!=committed.receipt.locator.snapshot);
    REQUIRE(!admit() && error.contains("snapshot or receipt"));
    REQUIRE(previewScratch(store,source).eligible.empty() && m3::fileHash(stage)==f.sha);
    REQUIRE(admissions==3);

    // A metadata edge arriving after the first reference check must also veto
    // admission. This direct fixture write bypasses the cooperative store lease.
    ScratchPreviewFixture late;
    const auto old=previewScratch(late.store.file,late.original.root);
    Snapshot plan;
    REQUIRE(inspectInterrupted(late.store.file,late.operation,plan,error));
    const auto newSnapshot=late.store.file.getChildFile("snapshots/"+plan.id+".json");
    Hooks publishReference;
    publishReference.checkpoint=[&](const auto& point,const auto&){
        if(point=="scratch-evidence-checked")
            REQUIRE(newSnapshot.getParentDirectory().createDirectory().wasOk() && newSnapshot.replaceWithText(encodeSnapshot(plan)));
        return true;
    };
    REQUIRE(!withRevalidatedScratch(late.store.file,late.original.root,old,late.path,ready,error,publishReference));
    REQUIRE(error.contains("snapshot or receipt") && newSnapshot.existsAsFile() && admissions==3);
    REQUIRE(m3::fileHash(late.store.file.getChildFile(late.path))==late.sha);
    std::cout<<"Scratch maintenance: independent reader, writer and exclusive lease; stale selection, locks, replaced counterpart, junction race, cancellation and new recovery references passed.\n";
}
