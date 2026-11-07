#pragma once

#include "PortableFixture.h"

static void captureRecoveryCases()
{
    using namespace m3::storage::portable;
    Fixture f;
    const auto previous=f.capture.snapshot();
    const auto acceptedXml=f.xml,acceptedManifest=f.manifest;
    const auto previousFile=f.root.getChildFile(previous.reference);
    const auto previousHash=m3::fileHash(previousFile);
    const auto source=f.root.getChildFile(previous.manifest.ranges.front().assetPath);
    const auto unindexed=f.root.getChildFile("segments/"+juce::Uuid().toDashedString()+".wav");
    REQUIRE(source.copyFileTo(unindexed));
    const auto unindexedHash=m3::fileHash(unindexed);
    const float audio[]{.5f,-.2f,.1f,-.3f};
    REQUIRE(f.capture.commit(8,48000,audio,2,0,&previous.manifest.generationContexts.front()));
    const auto newer=f.capture.snapshot();
    REQUIRE(newer.reference!=previous.reference && newer.manifest.revision>previous.manifest.revision);
    const auto newFile=f.root.getChildFile(newer.reference),newSource=f.root.getChildFile(newer.manifest.ranges.back().assetPath);
    const auto newHash=m3::fileHash(newFile),newSourceHash=m3::fileHash(newSource);

    const auto scan=[&]{
        return captureRecoveryCandidates(inspect(f.request()),f.root,f.observed);
    };
    const auto candidate=[&](const CaptureRecoveryReport& report){
        const auto found=std::find_if(report.snapshots.begin(),report.snapshots.end(),
            [&](const auto& row){return row.reference==newer.reference;});
        REQUIRE(found!=report.snapshots.end());
        return *found;
    };
    const auto report=scan();
    REQUIRE(report.complete && report.unindexedAudio.contains(unindexed.getRelativePathFrom(f.root).replaceCharacter('\\','/')));
    REQUIRE(std::none_of(report.snapshots.begin(),report.snapshots.end(),
        [&](const auto& row){return row.reference==previous.reference;}));
    const auto selected=candidate(report);
    REQUIRE(selected.unavailable.isEmpty() && selected.sha256==newHash && selected.revision==newer.manifest.revision);

    TestDirectory destination;
    REQUIRE(destination.file.createDirectory().wasOk());
    const auto recoveredRoot=destination.file.getChildFile(f.observed.project).getChildFile(f.observed.instance);
    const auto recoveredManifest=recoveredRoot.getChildFile(newer.reference);
    Hooks failed;
    failed.injectWriteError=[](uint64_t,uint32_t){return ERROR_DISK_FULL;};
    REQUIRE(!recoverCaptureSnapshotCopy(f.root,selected,destination.file,failed).success);
    REQUIRE(!recoveredManifest.exists());

    const auto held=newSource.getSiblingFile("held-source.wav");
    REQUIRE(newSource.moveFileTo(held));
    REQUIRE(candidate(scan()).unavailable.contains(newer.manifest.ranges.back().assetPath));
    REQUIRE(!recoverCaptureSnapshotCopy(f.root,selected,destination.file).success);
    REQUIRE(held.moveFileTo(newSource) && !recoveredManifest.exists());

    REQUIRE(!recoverCaptureSnapshotCopy(f.root,selected,f.directory.file).success);
    const auto wrongHash=CaptureRecoveryCandidate{selected.reference,juce::String::repeatedString("f",64),selected.revision};
    REQUIRE(!recoverCaptureSnapshotCopy(f.root,wrongHash,destination.file).success);

    const auto recovered=recoverCaptureSnapshotCopy(f.root,selected,destination.file);
    if(!recovered.success)throw std::runtime_error(recovered.error.toStdString());
    REQUIRE(m3::fileHash(recoveredManifest)==newHash);
    REQUIRE(m3::fileHash(recoveredRoot.getChildFile(newer.manifest.ranges.back().assetPath))==newSourceHash);
    REQUIRE(m3::fileHash(recoveredRoot.getChildFile(f.irs[0].path))==m3::fileHash(f.root.getChildFile(f.irs[0].path)));
    REQUIRE(!recoveredRoot.getChildFile(previous.reference).exists());
    REQUIRE(m3::fileHash(previousFile)==previousHash && m3::fileHash(newFile)==newHash
        && m3::fileHash(newSource)==newSourceHash && m3::fileHash(unindexed)==unindexedHash);
    REQUIRE(f.xml==acceptedXml && f.manifest==acceptedManifest && f.observed.reference==previous.reference);
    std::cout<<"Capture recovery: observed history, unindexed source, missing dependency, destination failure and isolated verified copy passed.\n";
}
