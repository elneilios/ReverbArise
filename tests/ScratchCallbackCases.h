#pragma once

#include "ScratchPreviewFixture.h"
#include "AllocationGuard.h"

static void scratchCallbackCases()
{
    ScratchPreviewFixture f;
    auto processor=std::make_unique<PreverbAudioProcessor>(f.original.directory.file);
    load(*processor,f.original.xml);
    ProfileHost host;
    host.position.setTimeInSamples(0);
    host.position.setPpqPosition(0.0);
    host.position.setBpm(120);
    host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    host.position.setIsPlaying(false);
    processor->setPlayHead(&host);
    processor->prepareToPlay(48000,64);
    juce::AudioBuffer<float> buffer(2,64);
    buffer.clear();
    juce::MidiBuffer midi;
    processor->processBlock(buffer,midi);

    const auto report=previewScratch(f.store.file,f.original.root);
    REQUIRE(report.complete && report.eligible.size()==1);
    juce::WaitableEvent entered,release;
    juce::String error;
    auto maintenance=std::async(std::launch::async,[&]{
        return withRevalidatedScratch(f.store.file,f.original.root,report,f.path,[&](const auto&){
            entered.signal();
            REQUIRE(release.wait(15000));
        },error);
    });
    if(!entered.wait(10000)){
        release.signal();
        REQUIRE(maintenance.get());
        throw std::runtime_error("Maintenance worker did not enter its held scope");
    }
    auto callback=std::async(std::launch::async,[&]{
        AudioAllocationGuard guard;
        for(int i=0;i<32;++i)processor->processBlock(buffer,midi);
        return watchedAllocations==0 && watchedDeallocations==0;
    });
    const auto returnedWhileHeld=callback.wait_for(std::chrono::seconds(5))==std::future_status::ready;
    release.signal();
    REQUIRE(maintenance.get() && returnedWhileHeld && callback.get());
    REQUIRE(m3::fileHash(f.store.file.getChildFile(f.path))==f.sha);
    processor->setPlayHead(nullptr);
    std::cout<<"Scratch maintenance held its lease while 32 real processor audio callbacks completed without allocations or waiting for release.\n";
}
