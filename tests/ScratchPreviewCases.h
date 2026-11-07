#pragma once

#include "ScratchPreviewFixture.h"

static void scratchPreviewCases()
{
    using namespace m3::storage::portable;
    ScratchPreviewFixture f;
    const auto& store=f.store.file;
    const auto candidate=store.getChildFile(f.path);
    const auto verify=[&](const auto& r){
        REQUIRE(r.complete && !r.cancelled && r.error.isEmpty() && r.store==f.storeId);
        REQUIRE(r.eligible.size()==1 && r.eligible[0].operation==f.operation && r.eligible[0].path==f.path);
        REQUIRE(r.eligible[0].bytes==f.bytes && r.bytes==f.bytes && r.eligible[0].reason.contains("intact original"));
    };
    juce::String error;
    std::shared_ptr<Prepared> prepared;
    REQUIRE(prepare({f.original.root,f.original.xml,f.original.manifest},prepared,error));
    // The same staging pattern is left by an interruption with a valid v1
    // plan. It has no terminal producer proof, even if Cancel follows the fault.
    bool lateCancel=false;
    Hooks fault;
    fault.cancelled=[&]{return lateCancel;};
    fault.checkpoint=[&](const auto& phase,const auto&){if(phase=="sealed-payload"){lateCancel=true;return false;}return true;};
    const auto interrupted=backup(prepared,store,fault);
    REQUIRE(!interrupted.success && !store.getChildFile("transactions/"+interrupted.operation+"/scratch-cancelled.json").exists());
    const auto oldParts=store.getChildFile("transactions/"+interrupted.operation+"/staging").findChildFiles(juce::File::findFiles,false);
    REQUIRE(oldParts.size()==1 && oldParts[0].getSize()==static_cast<juce::int64>(f.bytes) && m3::fileHash(oldParts[0])==f.sha);
    bool duringCopy=false;
    Hooks unsealed;
    unsealed.cancelled=[&]{return duringCopy;};
    unsealed.checkpoint=[&](const auto& phase,const auto&){if(phase=="copy")duringCopy=true;return true;};
    const auto partial=backup(prepared,store,unsealed);
    REQUIRE(partial.cancelled && !partial.success && !store.getChildFile("transactions/"+partial.operation+"/scratch-cancelled.json").exists());
    Input separate{f.original.root,f.original.xml,f.original.manifest};
    separate.producer="Distinct completed pack in mixed fixture";
    REQUIRE(prepare(separate,prepared,error));
    const auto committed=backup(prepared,store);
    REQUIRE(committed.success);
    prepared.reset();
    for(const auto* name:{"working/segments/sole-recording.wav","working/staging/abandoned.wav","working/irs/context.wav",
                          "working/renders/older-flp.wav","exports/playlist.wav","working/legacy.tmp"}){
        const auto file=store.getChildFile(name);
        REQUIRE(file.getParentDirectory().createDirectory().wasOk() && file.replaceWithText("protected fixture"));
    }
    const auto inventory=[&]{
        std::map<juce::String,juce::String> hashes;
        for(const auto& root:{store,f.original.root})for(const auto& file:root.findChildFiles(juce::File::findFiles,true))
            hashes.emplace(file.getFullPathName(),m3::fileHash(file));
        return hashes;
    };
    const auto before=inventory();
    const auto mixed=previewScratch(store,f.original.root);
    verify(mixed);
    REQUIRE(mixed.excluded.size()==3);
    REQUIRE(inventory()==before);

    const auto excluded=[&](const auto& r){REQUIRE(r.complete && r.eligible.empty() && r.bytes==0 && !r.excluded.empty());};
    Fixture other;
    excluded(previewScratch(store,other.root));
    const auto plan=store.getChildFile("transactions/"+f.operation+"/plan.json");
    const auto busy=CreateFileW(plan.getFullPathName().toWideCharPointer(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
    REQUIRE(busy!=INVALID_HANDLE_VALUE);
    const auto busyReport=previewScratch(store,f.original.root);
    CloseHandle(busy);
    excluded(busyReport);

    const auto journal=f.journal().loadFileAsString();
    auto unknown=parse(journal);property(unknown,"version",999);
    REQUIRE(f.journal().replaceWithText(encodeJson(unknown)));
    excluded(previewScratch(store,f.original.root));
    REQUIRE(f.journal().replaceWithText(journal));

    const auto alias=parse(journal)["alias"].toString();
    const auto original=f.original.root.getChildFile(alias),offline=original.getSiblingFile("temporarily-unavailable");
    REQUIRE(original.moveFileTo(offline));
    excluded(previewScratch(store,f.original.root));
    REQUIRE(offline.moveFileTo(original));

    const auto link=candidate.getSiblingFile("test-hard-link");
    REQUIRE(CreateHardLinkW(link.getFullPathName().toWideCharPointer(),candidate.getFullPathName().toWideCharPointer(),nullptr));
    excluded(previewScratch(store,f.original.root));
    REQUIRE(link.deleteFile());
    verify(previewScratch(store,f.original.root));

    bool stop=false;
    Hooks cancel;
    cancel.cancelled=[&]{return stop;};
    cancel.checkpoint=[&](const auto& phase,const auto&){if(phase=="read")stop=true;return true;};
    const auto cancelled=previewScratch(store,f.original.root,cancel);
    REQUIRE(cancelled.cancelled && !cancelled.complete && cancelled.eligible.empty() && cancelled.bytes==0);
    REQUIRE(inventory()==before);

    const auto moved=candidate.getSiblingFile("retained-original-part");
    REQUIRE(candidate.moveFileTo(moved) && moved.copyFileTo(candidate));
    REQUIRE(m3::fileHash(candidate)==f.sha);
    excluded(previewScratch(store,f.original.root)); // Same bytes, different file identity.
    const auto transactions=store.getChildFile("transactions");
    for(int i=0;i<257;++i)REQUIRE(transactions.getChildFile("unrecognized-"+juce::String(i)).createDirectory().wasOk());
    const auto limited=previewScratch(store,f.original.root);
    REQUIRE(!limited.complete && limited.eligible.empty() && limited.bytes==0 && limited.error.contains("limit"));
    std::cout<<"Scratch preview: real cancelled Backup, protected categories and old plans, exact totals, busy plan, wrong/missing source, unsupported journal, hard link, replaced identity and read-only cancellation passed.\n";
}
