#pragma once

#include "ScratchPreviewFixture.h"

static void scratchQuarantineChild(const juce::String& action,const juce::File& store,const juce::File& source,
                                  const juce::String& operation,const juce::String& boundary)
{
    using namespace m3::storage::portable;
    Hooks hooks;
    hooks.checkpoint=[&](const auto& point,const auto&){
        if(point==boundary)ExitProcess(73); // No C++ stack unwinding or compensating actions.
        return true;
    };
    juce::String error;
    if(action=="quarantine"){
        const auto preview=previewScratch(store,source);
        REQUIRE(preview.complete && preview.eligible.size()==1 && preview.eligible.front().operation==operation);
        quarantineScratch(store,source,preview,preview.eligible.front().path,error,hooks);
    }else{
        REQUIRE(action=="undo");
        undoScratchQuarantine(store,operation,error,hooks);
    }
    throw std::runtime_error(("Process did not exit at "+boundary+": "+error).toStdString());
}

static void crashQuarantineAt(const ScratchPreviewFixture& fixture,const char* action,const char* boundary)
{
    juce::ChildProcess child;
    const auto exe=juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    REQUIRE(child.start(juce::StringArray{exe.getFullPathName(),"--scratch-quarantine-child",action,
        fixture.store.file.getFullPathName(),fixture.original.root.getFullPathName(),fixture.operation,boundary}));
    REQUIRE(child.waitForProcessToFinish(12000));
    if(child.getExitCode()!=73)throw std::runtime_error(child.readAllProcessOutput().toStdString());
}

static void scratchQuarantineCases()
{
    using namespace m3::storage::portable;
    using State=ScratchQuarantineState;
    juce::String error;
    const auto original=[](const auto& f){return f.store.file.getChildFile(f.path);};
    const auto payload=[](const auto& f){return f.store.file.getChildFile("quarantine/"+f.operation+"/payload");};
    const auto inspect=[](const auto& f){return inspectScratchQuarantine(f.store.file,f.operation);};
    const auto undo=[&](const auto& f,const Hooks& hooks=Hooks{}){return undoScratchQuarantine(f.store.file,f.operation,error,hooks);};
    const auto quarantine=[&](const auto& f,const Hooks& hooks=Hooks{}){
        const auto preview=previewScratch(f.store.file,f.original.root);
        REQUIRE(preview.complete && preview.eligible.size()==1);
        return quarantineScratch(f.store.file,f.original.root,preview,f.path,error,hooks);
    };
    const auto hashes=[](const juce::File& root){
        std::map<juce::String,juce::String> found;
        for(const auto& file:root.findChildFiles(juce::File::findFiles,true))
            found.emplace(file.getFullPathName(),m3::fileHash(file));
        return found;
    };
    const auto expect=[&](const auto& f,State state,bool pending=false){
        const auto before=hashes(f.store.file);
        const auto result=inspect(f);
        if(result.state!=state)throw std::runtime_error(result.error.toStdString());
        REQUIRE(result.undoPending==pending && result.operation==f.operation);
        REQUIRE(hashes(f.store.file)==before);
        if(state==State::quarantined){
            REQUIRE(result.originalPath==f.path && result.bytes==f.bytes);
            REQUIRE(!original(f).exists() && payload(f).existsAsFile() && m3::fileHash(payload(f))==f.sha);
        }else if(state!=State::ambiguous){
            REQUIRE(original(f).existsAsFile() && m3::fileHash(original(f))==f.sha && !payload(f).exists());
        }
    };

    struct Case {const char* point;State state;bool pending=false;};
    for(const auto& test:{Case{"quarantine-before-directory",State::unavailable},
                         Case{"quarantine-intent-before",State::unavailable},
                         Case{"quarantine-intent-sealed",State::unavailable},
                         Case{"quarantine-intent-published",State::staged},
                         Case{"quarantine-intent-after",State::staged},
                         Case{"quarantine-before-move",State::staged},
                         Case{"quarantine-after-move",State::quarantined},
                         Case{"quarantine-complete-before",State::quarantined},
                         Case{"quarantine-complete-sealed",State::quarantined},
                         Case{"quarantine-complete-published",State::quarantined},
                         Case{"quarantine-complete-after",State::quarantined}}){
        ScratchPreviewFixture f;
        const auto durable=hashes(f.original.root);
        crashQuarantineAt(f,"quarantine",test.point);
        expect(f,test.state);
        if(test.state!=State::quarantined)REQUIRE(quarantine(f)); // Explicit retry only.
        REQUIRE(undo(f));
        expect(f,State::restored);
        REQUIRE(hashes(f.original.root)==durable);
    }

    for(const auto& test:{Case{"undo-intent-before",State::quarantined},
                         Case{"undo-intent-sealed",State::quarantined},
                         Case{"undo-intent-published",State::quarantined,true},
                         Case{"undo-intent-after",State::quarantined,true},
                         Case{"undo-before-move",State::quarantined,true},
                         Case{"undo-after-move",State::restored},
                         Case{"undo-complete-before",State::restored},
                         Case{"undo-complete-sealed",State::restored},
                         Case{"undo-complete-published",State::restored},
                         Case{"undo-complete-after",State::restored}}){
        ScratchPreviewFixture f;
        REQUIRE(quarantine(f));
        crashQuarantineAt(f,"undo",test.point);
        expect(f,test.state,test.pending);
        REQUIRE(undo(f) && undo(f)); // Finish interrupted undo and verify idempotence.
        expect(f,State::restored);
        REQUIRE(previewScratch(f.store.file,f.original.root).eligible.empty());
    }

    {
        ScratchPreviewFixture f;
        auto incomplete=previewScratch(f.store.file,f.original.root);
        incomplete.complete=false;
        REQUIRE(!quarantineScratch(f.store.file,f.original.root,incomplete,f.path,error));
        REQUIRE(!f.store.file.getChildFile("quarantine").exists());
        Hooks conflict;
        conflict.checkpoint=[&](const auto& point,const auto&){
            if(point=="quarantine-before-move")REQUIRE(payload(f).replaceWithText("occupied quarantine target"));
            return true;
        };
        REQUIRE(!quarantine(f,conflict));
        REQUIRE(original(f).existsAsFile() && m3::fileHash(original(f))==f.sha);
        REQUIRE(payload(f).loadFileAsString()=="occupied quarantine target");
        expect(f,State::ambiguous);
        REQUIRE(!undo(f));
    }
    {
        ScratchPreviewFixture f;
        REQUIRE(quarantine(f));
        Hooks conflict;
        conflict.checkpoint=[&](const auto& point,const auto&){
            if(point=="undo-before-move")REQUIRE(original(f).replaceWithText("occupied original target"));
            return true;
        };
        REQUIRE(!undo(f,conflict));
        REQUIRE(m3::fileHash(payload(f))==f.sha && original(f).loadFileAsString()=="occupied original target");
        expect(f,State::ambiguous);
        REQUIRE(!undo(f));
    }
    {
        ScratchPreviewFixture f;
        REQUIRE(quarantine(f));
        const auto handle=CreateFileW(payload(f).getFullPathName().toWideCharPointer(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_EXISTING,0,nullptr);
        REQUIRE(handle!=INVALID_HANDLE_VALUE);
        char byte=0;
        DWORD count=0;
        const auto read=ReadFile(handle,&byte,1,&count,nullptr)!=0 && count==1;
        byte^=1;
        const auto wrote=read && SetFilePointer(handle,0,nullptr,FILE_BEGIN)==0 && WriteFile(handle,&byte,1,&count,nullptr)!=0 && count==1;
        CloseHandle(handle);
        REQUIRE(wrote && payload(f).getSize()==static_cast<juce::int64>(f.bytes));
        const auto damaged=m3::fileHash(payload(f));
        REQUIRE(damaged!=f.sha);
        expect(f,State::ambiguous);
        REQUIRE(!undo(f) && !original(f).exists() && m3::fileHash(payload(f))==damaged);
    }
    {
        ScratchPreviewFixture f;
        REQUIRE(quarantine(f));
        const auto retained=payload(f).getSiblingFile("retained-payload");
        REQUIRE(payload(f).moveFileTo(retained));
        expect(f,State::ambiguous);
        REQUIRE(!undo(f) && m3::fileHash(retained)==f.sha);
        REQUIRE(retained.moveFileTo(payload(f)));
        const auto intent=payload(f).getSiblingFile("move.json");
        REQUIRE(intent.replaceWithText("conflicting record"));
        expect(f,State::ambiguous);
        REQUIRE(!undo(f) && m3::fileHash(payload(f))==f.sha);
    }
    {
        ScratchPreviewFixture f;
        REQUIRE(quarantine(f));
        const auto staging=original(f).getParentDirectory(),retained=staging.getSiblingFile("retained-staging");
        REQUIRE(staging.moveFileTo(retained) && staging.createDirectory().wasOk());
        expect(f,State::ambiguous);
        REQUIRE(!undo(f) && m3::fileHash(payload(f))==f.sha && !original(f).exists());
    }
    for(const auto* failure:{"quarantine-intent-before","quarantine-complete-before"}){
        ScratchPreviewFixture f;
        bool full=false;
        Hooks disk;
        disk.checkpoint=[&](const auto& point,const auto&){if(point==failure)full=true;return true;};
        disk.injectWriteError=[&](uint64_t,uint32_t){return full?ERROR_DISK_FULL:0;};
        REQUIRE(!quarantine(f,disk));
        const auto beforeMove=juce::String(failure).startsWith("quarantine-intent");
        expect(f,beforeMove?State::unavailable:State::quarantined);
        if(beforeMove)REQUIRE(quarantine(f));
        REQUIRE(undo(f));
        expect(f,State::restored);
    }
    {
        ScratchPreviewFixture f;
        Hooks lateJournal;
        lateJournal.checkpoint=[&](const auto& point,const auto&){
            if(point=="quarantine-before-move")REQUIRE(payload(f).getSiblingFile("undo.json").replaceWithText("conflicting undo"));
            return true;
        };
        REQUIRE(!quarantine(f,lateJournal));
        REQUIRE(!payload(f).exists() && m3::fileHash(original(f))==f.sha);
        expect(f,State::ambiguous);
    }
    std::cout<<"Scratch quarantine: 21 process exits at journal/move boundaries, verified restart and undo, occupied paths, changed directory, corrupt payload and ambiguous state passed.\n";
}
