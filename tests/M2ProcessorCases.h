#pragma once
#include "AudioTestFiles.h"
#include "AllocationGuard.h"

inline void m2ProcessorCases()
{
    juce::ScopedJuceInitialiser_GUI gui;
    TestDirectory root;
    auto processor = std::make_unique<PreverbAudioProcessor>(root.file);
    REQUIRE(processor->swellEngine().savedSession().auditionFollowsLive && processor->swellEngine().savedSession().auditionMode==m3::AuditionMode::dryWet);
    processor->swellEngine().setMonitoring(m3::AuditionMode::wetOnly,.5f); // This historical oracle explicitly exercises wet-only monitoring.
    REQUIRE(processor->swellEngine().setGainPolicy(m3::GainPolicy::legacy)); // Original preview sample oracle.
    processor->swellEngine().setLivePlayback(false,m3::LiveRouting::dryWet,0); // Exercise independent manual preview.
    processor->swellEngine().setAutomaticProcessing(false);
    processor->prepareToPlay(1000, 250);
    TestPlayHead head;
    head.position.setBpm(240); head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    REQUIRE(setFixtureCaptureArmed(*processor));head.position.setIsPlaying(true);
    processor->setPlayHead(&head);
    juce::AudioBuffer<float> buffer(2, 250);
    juce::MidiBuffer midi;
    processor->setMappingEnabled(true);
    for (int block = 0; block < 4; ++block)
    {
        head.position.setTimeInSamples(block * 250); head.position.setPpqPosition(block);
        buffer.clear();
        if (block == 0) { buffer.setSample(0, 0, 0.25f); buffer.setSample(1, 0, -0.5f); }
        processor->processBlock(buffer, midi);
    }
    finishCaptureFixture(*processor,head.position); REQUIRE(processor->flushCaptureForTesting());
    writeTestWav(root.file.getChildFile("ir.wav"), 1, 1000, {1,0.5f,0.25f});
    const auto ir=root.file.getChildFile("ir.wav");
    processor->loadPreviewIR(ir);
    const auto deadline=juce::Time::getMillisecondCounter()+5000;
    for (;;)
    {
        const auto preview=processor->getPreviewSnapshot();
        if(preview.irName==ir.getFileName() && preview.phase==PreviewPhase::ready)break;
        if(juce::Time::getMillisecondCounter()>=deadline)
            throw std::runtime_error("Imported IR did not become audition-ready: "+preview.status.toStdString());
        juce::Thread::sleep(1);
    }
    REQUIRE(processor->getPreviewSnapshot().waveform->start == -2);
    {
        std::unique_ptr<juce::AudioProcessorEditor> editor(processor->createEditor());
        editor->setVisible(true); // Offscreen component only: no desktop peer or native window.
        bool auditionEnabled = false;
        for (auto* child : editor->getChildren())
        {
            REQUIRE(editor->getLocalBounds().contains(child->getBounds()));
        }
        const auto inspect=[&](auto&& self,juce::Component& parent)->void
        {
            if(parent.getName()=="audition")auditionEnabled=parent.isEnabled();
            for(auto* child:parent.getChildren())self(self,*child);
        };
        inspect(inspect,*editor);
        REQUIRE(auditionEnabled);
        const auto imagePath = juce::SystemStats::getEnvironmentVariable("PREVERB_UI_PREVIEW_OUTPUT", {});
        if (imagePath.isNotEmpty())
        {
            juce::Image image(juce::Image::ARGB, editor->getWidth(), editor->getHeight(), true, juce::SoftwareImageType{});
            juce::Graphics graphics(image);
            editor->paintEntireComponent(graphics, true);
            REQUIRE(image.getPixelAt(20, 20).getARGB() == 0xff172126);
            const juce::File file(imagePath);
            REQUIRE(file.getParentDirectory().createDirectory().wasOk());
            auto stream = file.createOutputStream();
            REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk());
            REQUIRE(juce::PNGImageFormat().writeImageToStream(image, *stream));
        }
    }
    REQUIRE(processor->auditionPreview());
    head.position.setIsPlaying(false);
    for (int block = 0; block < 5; ++block)
    {
        // Seeking/looping changes the absolute timeline but not the manual preview cursor.
        head.position.setTimeInSamples(block % 2 ? 90000 : 0);
        head.position.setPpqPosition(block % 2 ? 360 : 0);
        buffer.clear();
        { AudioAllocationGuard guard; processor->processBlock(buffer, midi); }
        REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
        if (block == 0)
        { const auto gain = 1.0f;
          REQUIRE(buffer.getSample(0,0)==0); REQUIRE(std::abs(buffer.getSample(1,2) + .5f * (.95f*.95f*(3-2*.95f)) * .4f * gain)<1e-5); }
        if (block == 4) REQUIRE(processor->swellEngine().snapshot().playing); // The finite window now loops until Stop.
    }
    REQUIRE(processor->auditionPreview());
    head.position.setIsPlaying(true); head.position.setTimeInSamples(4000); head.position.setPpqPosition(16);
    REQUIRE(processor->requestWorkflowMode(workflow::Mode::capture));
    REQUIRE(setFixtureCaptureArmed(*processor));
    buffer.clear(); processor->processBlock(buffer, midi);
    finishCaptureFixture(*processor,head.position); REQUIRE(processor->flushCaptureForTesting());
    auto captured = processor->getCaptureSnapshot().manifest;
    CaptureAssetStore store(root.file.getChildFile(captured.projectId).getChildFile(captured.instanceId));
    SourceInterval dryCapture;
    juce::String error;
    REQUIRE(readSourceInterval(store, captured, 4000, 4250, 1000, dryCapture, error));
    REQUIRE(std::all_of(dryCapture.samples.begin(), dryCapture.samples.end(), [](float v) { return v == 0; }));
    processor->stopPreview(); buffer.clear(); processor->processBlock(buffer, midi);
    juce::MemoryBlock saved;
    processor->getStateInformation(saved);
    REQUIRE(saved.getSize() < 8192);
    processor->setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    // M3 supersedes the transient M2 preview contract; saved definitions survive.
    REQUIRE(!processor->swellEngine().savedSession().swells.empty());
    REQUIRE(!processor->swellEngine().snapshot().playing && !processor->isMappingEnabled());
    {
        std::unique_ptr<juce::AudioProcessorEditor> editor(processor->createEditor());
        const auto find=[](const auto& self,juce::Component& component,const juce::String& name) -> juce::Component*
        {
            if(component.getName()==name)return &component;
            for(auto* child:component.getChildren())
                if(auto* match=self(self,*child,name))return match;
            return nullptr;
        };
        auto* pair=find(find,*editor,"auditionPair");
        REQUIRE(pair);
        const auto* action=find(find,*pair,"audition");
        REQUIRE(action && action->getTitle()=="Play audition");
    }
    buffer.clear();
    { AudioAllocationGuard guard; processor->processBlock(buffer, midi); }
    REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0 && buffer.getMagnitude(0, 250) == 0);
    REQUIRE(processor->flushCaptureForTesting());
    REQUIRE(processor->getCaptureSnapshot().manifest.instanceId == captured.instanceId);
    processor->setPlayHead(nullptr);
}
