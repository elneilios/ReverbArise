#pragma once
#include <limits>

struct CaptureFailureHost:juce::AudioPlayHead
{
    PositionInfo position;bool available=true;
    juce::Optional<PositionInfo> getPosition()const override{return available?juce::Optional<PositionInfo>{position}:juce::Optional<PositionInfo>{};}
    void at(int64_t sample,bool playing,double rate=1000)
    {available=true;position.setTimeInSamples(sample);position.setPpqPosition(double(sample)*2/rate);position.setBpm(120);position.setTimeSignature(TimeSignature{4,4});position.setIsPlaying(playing);}
};
inline void processorCaptureFailureCases()
{
    for(bool prior:{false,true})for(int kind=0;kind<9;++kind)
    {
        TestDirectory root;auto processor=std::make_unique<PreverbAudioProcessor>(root.file);CaptureFailureHost host;
        processor->prepareToPlay(1000,64);processor->setPlayHead(&host);processor->swellEngine().setLearningAutomatic(false);
        juce::MidiBuffer midi;juce::AudioBuffer<float> input(2,64);
        const auto process=[&]
        {
            for(int c=0;c<2;++c)std::fill_n(input.getWritePointer(c),input.getNumSamples(),.125f);
            {AudioAllocationGuard guard;processor->processBlock(input,midi);}
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
            REQUIRE(input.getSample(0,0)==.125f && input.getSample(1,input.getNumSamples()-1)==.125f);
        };
        if(prior)
        {host.at(0,true);process();host.at(64,false);process();REQUIRE(processor->flushCaptureForTesting());REQUIRE(processor->requestWorkflowMode(workflow::Mode::capture));}
        const auto before=processor->getCaptureSnapshot();
        juce::MemoryBlock savedBefore;processor->getStateInformation(savedBefore);
        REQUIRE(processor->setRecaptureExisting(prior));host.at(10000,true);process();REQUIRE(processor->flushCaptureForTesting());
        const auto failedPass=processor->getWorkflowSnapshot().pass;host.at(10064,true);
        auto expected=CaptureFailureReason::timingUnavailable;
        switch(kind)
        {
            case 0:host.position.setPpqPosition({});break;
            case 1:host.position.setBpm({});break;
            case 2:
#if PREVERB_COMPATIBILITY_METER_4_4
                // Missing meter is the explicit experimental exception. A host-
                // supplied invalid meter must still fail without replacement.
                host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{0,0});
#else
                host.position.setTimeSignature({});
#endif
                break;
            case 3:host.position.setTimeInSamples({});break;
            case 4:host.position.setBpm(121);expected=CaptureFailureReason::timingChanged;break;
            case 5:input.setSize(2,8193);expected=CaptureFailureReason::blockSize;break;
            case 6:host.at(INT64_MAX-32,true);expected=CaptureFailureReason::range;break;
            case 7:processor->prepareToPlay(2000,64);expected=CaptureFailureReason::sampleRate;break;
            case 8:host.available=false;break;
        }
        process();input.setSize(2,64);host.at(10128,false);process();REQUIRE(processor->flushCaptureForTesting());
        const auto failure=processor->getCaptureSnapshot();
        REQUIRE(processor->getWorkflowSnapshot().phase==workflow::Phase::failed && failure.reference==before.reference);
        REQUIRE(failure.failure && failure.failure.event.reason==expected && failure.failure.event.pass==failedPass);
        REQUIRE(failure.failure.event.queued==1 && failure.failure.event.dropped==1 && failure.failure.finalDropped==1);
        REQUIRE(processor->getCaptureProgress().ranges.empty());
        processor->prepareToPlay(1000,64);
        // Existing incompatible-rate policy requires explicit restore or Clear.
        // Restore preserves source and retires diagnostics from the prior epoch.
        if(kind==7 && prior){processor->setStateInformation(savedBefore.getData(),static_cast<int>(savedBefore.getSize()));REQUIRE(processor->flushCaptureForTesting());}
        host.at(20000,true);process();host.at(20064,false);process();
        REQUIRE(processor->flushCaptureForTesting() && processor->getWorkflowSnapshot().result==workflow::Result::accepted);
        if(kind==7 && prior)REQUIRE(!processor->getCaptureSnapshot().failure);
        else REQUIRE(processor->getCaptureSnapshot().failure.event.pass==failedPass);
        if(prior)
        {CaptureSession restored(root.file);REQUIRE(restored.restore(before.manifest.projectId,before.manifest.instanceId,before.reference,before.manifest.revision));float l=0,r=0;REQUIRE(restored.readSample(0,l,r) && l==.125f);}
        std::cout<<"processor fault prior="<<prior<<" kind="<<kind<<" callback-allocations=0 retry=passed\n";
    }
}
