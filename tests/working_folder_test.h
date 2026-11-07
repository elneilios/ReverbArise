#include <Storage/WorkingFolder.h>
static void checked(bool ok,const juce::String& error){if(!ok)throw std::runtime_error(error.toStdString());}
#include "recovery_workspace_cases.h"
#include "directory_reparse_probe.h"
static int workingFolderTest(int argc,char** argv)
{
    return runTest([&]{
        // Child processes resolve only exact host metadata and registered hints.
        if(juce::String(argv[1])=="--working-child"){
            REQUIRE(argc==6);const juce::File stateFile(argv[2]),registry(argv[3]),output(argv[4]);
            Association association;juce::String xml,manifest,error;
            checked(decodeAssociatedState(stateFile.loadFileAsString(),association,xml,manifest,error),error);
            std::shared_ptr<WorkingFolder> folder;checked(resolveWorkingFolder(association,xml,manifest,registry,folder,error),error);
            REQUIRE(workingFolderInfo(*folder).association.writer!=association.writer);
            REQUIRE(output.replaceWithText(workingFolderInfo(*folder).association.writer));
            const juce::File release(argv[5]);const auto until=juce::Time::getMillisecondCounterHiRes()+30000;
            while(!release.exists()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<until);juce::Thread::sleep(10);}return;
        }
        directoryReparseProbe();recoveryWorkspaceCases();
        Fixture fixture;TestDirectory target,registry;REQUIRE(target.file.createDirectory().wasOk() && registry.file.createDirectory().wasOk());
        Input input{fixture.root,fixture.xml,fixture.manifest};input.controlEpoch=42;juce::String error;
        std::shared_ptr<Prepared> source;checked(prepare(input,source,error),error);const auto original=snapshot(*source);
        std::shared_ptr<WorkingFolder> prepared;checked(prepareWorkingFolder(input,target.file,registry.file,prepared,error),error);
        const auto first=workingFolderInfo(*prepared);REQUIRE(first.projectsRoot==target.file.getChildFile("working"));
        const auto instance=first.projectsRoot.getChildFile(fixture.observed.project).getChildFile(fixture.observed.instance);
        for(const auto& entry:original.files){REQUIRE(fileHash(fixture.root.getChildFile(entry.path))==entry.sha256);REQUIRE(fileHash(instance.getChildFile(entry.path))==entry.sha256);}
        REQUIRE(first.files==original.files.size());checked(revalidateWorkingFolder(*prepared,input,error),error);
        auto stale=input;++stale.controlEpoch;REQUIRE(!revalidateWorkingFolder(*prepared,stale,error));
        stale=input;stale.manifestJson+=" ";REQUIRE(!revalidateWorkingFolder(*prepared,stale,error));
        stale=input;stale.sourceUnavailable=true;REQUIRE(!revalidateWorkingFolder(*prepared,stale,error));
        // Exact metadata is embedded; future edits cannot silently select an old pack.
        auto association=first.association;juce::String saved,legacy,manifest;
        checked(encodeAssociatedState(input.processorXml,input.manifestJson,association,saved,error),error);
        Association decoded;checked(decodeAssociatedState(saved,decoded,legacy,manifest,error),error);
        REQUIRE(decoded==association && legacy==input.processorXml && manifest==input.manifestJson);
        ObservedSnapshot observed;checked(readLegacySnapshot(saved,manifest,observed,error),error);
        REQUIRE(observed.dependencies.size()==fixture.observed.dependencies.size());
        auto bad=juce::XmlDocument::parse(saved);bad->getChildByName("WorkingFolder")->setAttribute("snapshot",juce::String::repeatedString("0",64));
        REQUIRE(!decodeAssociatedState(bad->toString(),decoded,legacy,manifest,error) && decoded==association);
        REQUIRE(!readLegacySnapshot(saved,manifest+" ",observed,error));
        bad=juce::XmlDocument::parse(saved);bad->setAttribute("stateVersion",10);REQUIRE(!decodeAssociatedState(bad->toString(),decoded,legacy,manifest,error));
        bad->setAttribute("stateVersion","09");REQUIRE(!decodeAssociatedState(bad->toString(),decoded,legacy,manifest,error));
        REQUIRE(!decodeAssociatedState("<!DOCTYPE data [<!ENTITY x 'bad'>]>"+saved,decoded,legacy,manifest,error));
        REQUIRE(!decodeAssociatedState(juce::String::repeatedString("<x>",64)+juce::String::repeatedString("</x>",64),decoded,legacy,manifest,error));
        bad=juce::XmlDocument::parse(saved);bad->getChildByName("WorkingFolder")->getChildByName("CaptureManifest")->setAttribute("future","unknown");
        REQUIRE(!decodeAssociatedState(bad->toString(),decoded,legacy,manifest,error));
        // Reopen and duplicate receive independent exclusive leases, never saved authority.
        std::shared_ptr<WorkingFolder> reopened;checked(resolveWorkingFolder(association,input.processorXml,input.manifestJson,registry.file,reopened,error),error);
        REQUIRE(workingFolderInfo(*reopened).association.writer!=association.writer);
        const auto lease=target.file.getChildFile("instances/"+fixture.observed.instance+"/branches/"+workingFolderInfo(*reopened).association.writer+"/lease");
        const auto denied=CreateFileW(lease.getFullPathName().toWideCharPointer(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
        REQUIRE(denied==INVALID_HANDLE_VALUE);
        // Two real processes own distinct leases simultaneously.
        const auto state=registry.file.getChildFile("state.xml"),release=registry.file.getChildFile("release");REQUIRE(state.replaceWithText(saved));
        juce::ChildProcess children[2];juce::File results[2]{registry.file.getChildFile("writer-a"),registry.file.getChildFile("writer-b")};
        for(int i=0;i<2;++i){juce::StringArray command{juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName(),"--working-child",state.getFullPathName(),registry.file.getFullPathName(),results[i].getFullPathName(),release.getFullPathName()};REQUIRE(children[i].start(command));}
        const auto until=juce::Time::getMillisecondCounterHiRes()+30000;
        while(!results[0].existsAsFile() || !results[1].existsAsFile()){REQUIRE(juce::Time::getMillisecondCounterHiRes()<until);juce::Thread::sleep(10);}
        REQUIRE(results[0].loadFileAsString()!=results[1].loadFileAsString());REQUIRE(release.replaceWithText("release"));
        for(auto& child:children){REQUIRE(child.waitForProcessToFinish(10000));const auto log=child.readAllProcessOutput();if(child.getExitCode()!=0)std::cerr<<log;REQUIRE(child.getExitCode()==0);}
        // Physical fork: separate store identity and independent complete bytes.
        TestDirectory fork;REQUIRE(fork.file.createDirectory().wasOk());std::shared_ptr<WorkingFolder> forked;
        Input associated{instance,saved,input.manifestJson};checked(prepareWorkingFolder(associated,fork.file,registry.file,forked,error),error);
        REQUIRE(workingFolderInfo(*forked).association.store!=association.store);
        for(const auto& entry:original.files)REQUIRE(fileHash(instance.getChildFile(entry.path))==entry.sha256);
        // Explicit relocation works without any local hint. Wrong store is rejected.
        TestDirectory fresh;REQUIRE(fresh.file.createDirectory().wasOk());std::shared_ptr<WorkingFolder> located;
        REQUIRE(!resolveWorkingFolder(association,input.processorXml,input.manifestJson,fresh.file,located,error));
        REQUIRE(!locateWorkingFolder(fork.file,association,input.processorXml,input.manifestJson,fresh.file,located,error));
        checked(locateWorkingFolder(target.file,association,input.processorXml,input.manifestJson,fresh.file,located,error),error);
        checked(resolveWorkingFolder(workingFolderInfo(*located).association,input.processorXml,input.manifestJson,fresh.file,located,error),error);
        // Cancel/read-only/faults preserve caller output and all original hashes.
        auto unchanged=prepared;TestDirectory cancelled;REQUIRE(cancelled.file.createDirectory().wasOk());Hooks cancel;cancel.cancelled=[]{return true;};
        REQUIRE(!prepareWorkingFolder(input,cancelled.file,registry.file,prepared,error,cancel) && prepared==unchanged);
        {TestDirectory deniedFolder;REQUIRE(deniedFolder.file.createDirectory().wasOk());ReadOnlyFixture deny(deniedFolder.file);REQUIRE(!prepareWorkingFolder(input,deniedFolder.file,registry.file,prepared,error) && prepared==unchanged);}
        for(const auto& phase:juce::StringArray{"working-folder-ready","working-folder-revalidated"}){Hooks fault;fault.checkpoint=[&](const auto& event,const auto&){return event!=phase;};
            if(phase=="working-folder-ready"){TestDirectory failed;REQUIRE(failed.file.createDirectory().wasOk());REQUIRE(!prepareWorkingFolder(input,failed.file,registry.file,prepared,error,fault) && prepared==unchanged);}
            else REQUIRE(!revalidateWorkingFolder(*prepared,input,error,fault));}
        for(const auto& entry:original.files)REQUIRE(fileHash(fixture.root.getChildFile(entry.path))==entry.sha256);
        std::cout<<"Working-folder preparation, exact metadata, stale/cancel/fault guards, leases in two processes, physical fork and explicit relocation passed.\n";
    });
}
