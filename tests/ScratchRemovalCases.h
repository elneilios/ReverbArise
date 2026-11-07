#pragma once

#include "ScratchPreviewFixture.h"

static void scratchRemovalCases()
{
    using namespace m3::storage::portable;
    using State=ScratchQuarantineState;
    juce::String error;
    const auto payload=[](const auto& f){return f.store.file.getChildFile("quarantine/"+f.operation+"/payload");};
    const auto inspect=[](const auto& f){return inspectScratchQuarantine(f.store.file,f.operation);};
    const auto quarantine=[&](const auto& f){
        const auto preview=previewScratch(f.store.file,f.original.root);
        REQUIRE(preview.complete && preview.eligible.size()==1);
        REQUIRE(quarantineScratch(f.store.file,f.original.root,preview,f.path,error));
    };
    const auto review=[&](const auto& f,ScratchRemovalReview& approved){
        return reviewScratchRemoval(f.store.file,f.original.root,f.operation,approved,error);
    };
    const auto remove=[&](const auto& f,const ScratchRemovalReview& approved,const Hooks& hooks=Hooks{}){
        return removeReviewedScratch(f.store.file,f.original.root,approved,error,hooks);
    };
    const auto age=[&](const auto& f,int days){
        const auto journal=payload(f).getSiblingFile("moved.json");
        const auto time=juce::Time::getCurrentTime()-juce::RelativeTime::days(days);
        REQUIRE(journal.setCreationTime(time) && journal.setLastModificationTime(time));
    };
    const auto retained=[&](const auto& f){REQUIRE(payload(f).existsAsFile() && m3::fileHash(payload(f))==f.sha);};

    {
        ScratchPreviewFixture f;
        quarantine(f);
        ScratchRemovalReview approved;
        REQUIRE(!review(f,approved) && approved.operation.isEmpty());
        REQUIRE(!remove(f,approved));
        age(f,29);
        REQUIRE(!review(f,approved));
        age(f,31);
        REQUIRE(inspect(f).state==State::quarantined);
        retained(f); // Expiry and passive inspection cannot remove bytes.
        REQUIRE(review(f,approved));
        REQUIRE(approved.path.endsWith("/payload") && approved.bytes==f.bytes);
        retained(f); // Review is read-only and performs no deletion.
        auto changed=approved;
        changed.bytes++;
        REQUIRE(!remove(f,changed));
        age(f,32);
        REQUIRE(!remove(f,approved)); // An altered deadline retires the review.
        REQUIRE(review(f,approved));

        for(const auto& path:{payload(f),f.store.file.getChildFile("store.json")}){
            const auto handle=CreateFileW(path.getFullPathName().toWideCharPointer(),GENERIC_READ,
                FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
            REQUIRE(handle!=INVALID_HANDLE_VALUE);
            const auto blocked=!remove(f,approved);
            CloseHandle(handle);
            REQUIRE(blocked);
            retained(f);
        }

        const auto proof=juce::JSON::parse(f.journal().loadFileAsString());
        const auto source=f.original.root.getChildFile(proof["alias"].toString());
        REQUIRE(source.existsAsFile() && m3::fileHash(source)==f.sha);
        const auto heldSource=source.getSiblingFile("retained-original");
        REQUIRE(source.moveFileTo(heldSource));
        REQUIRE(!remove(f,approved));
        retained(f);
        REQUIRE(heldSource.moveFileTo(source));
        const auto snapshot=f.store.file.getChildFile("snapshots/"+proof["snapshot"].toString()+".json");
        Hooks lateReference;
        lateReference.checkpoint=[&](const auto& point,const auto&){
            if(point=="removal-before-delete")REQUIRE(snapshot.replaceWithText("late snapshot"));
            return true;
        };
        REQUIRE(!remove(f,approved,lateReference));
        retained(f);
        REQUIRE(snapshot.deleteFile());
        REQUIRE(review(f,approved) && remove(f,approved));
        REQUIRE(inspect(f).state==State::removed && !payload(f).exists());
        REQUIRE(!remove(f,approved) && !undoScratchQuarantine(f.store.file,f.operation,error));
        REQUIRE(m3::fileHash(source)==f.sha && f.journal().existsAsFile());
    }

    for(const auto* point:{"removal-intent-before","removal-before-delete","removal-complete-before"}){
        ScratchPreviewFixture f;
        quarantine(f);
        age(f,31);
        ScratchRemovalReview approved;
        REQUIRE(review(f,approved));
        Hooks fault;
        fault.checkpoint=[&](const auto& boundary,const auto&){return boundary!=point;};
        REQUIRE(!remove(f,approved,fault));
        if(juce::String(point)=="removal-complete-before"){
            REQUIRE(!payload(f).exists() && inspect(f).state==State::ambiguous);
            REQUIRE(!review(f,approved));
        }else{
            retained(f);
            REQUIRE(undoScratchQuarantine(f.store.file,f.operation,error));
            REQUIRE(inspect(f).state==State::restored);
        }
    }

    {
        ScratchPreviewFixture f;
        quarantine(f);
        const auto marker=payload(f).getSiblingFile("moved.json");
        REQUIRE(marker.deleteFile());
        ScratchRemovalReview approved;
        REQUIRE(!review(f,approved));
        retained(f);
        REQUIRE(undoScratchQuarantine(f.store.file,f.operation,error));
        REQUIRE(inspect(f).state==State::restored);
    }
    std::cout<<"Scratch removal: grace, separate review, stale evidence, active handles, missing survivor, new reference, interruption and undo passed.\n";
}
