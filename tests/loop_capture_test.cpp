#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Plugin/PluginProcessor.h>
#include <limits>

namespace
{
using Phase = LoopCapture::Phase;
struct Host : juce::AudioPlayHead
{
    PositionInfo position;
    juce::Optional<PositionInfo> getPosition() const override { return position; }
};

struct Fixture
{
    TestDirectory root;
    std::unique_ptr<PreverbAudioProcessor> owner=std::make_unique<PreverbAudioProcessor>(root.file);
    PreverbAudioProcessor& processor=*owner;
    juce::AudioBuffer<float> audio{2, 64};
    juce::MidiBuffer midi;

    Fixture()
    {
        processor.prepareToPlay(48000, 64);
        midi.ensureSize(128);
    }
    void edge(int at, bool high, int note=LoopCapture::gateNote)
    {
        const uint8_t event[]{0x90, static_cast<uint8_t>(note), static_cast<uint8_t>(high ? 100 : 0)};
        midi.addEvent(event, 3, at);
    }
    void process(float value = .25f)
    {
        for (int i=0; i<audio.getNumSamples(); ++i)
        {
            audio.setSample(0,i,value);
            audio.setSample(1,i,-value);
        }
        {
            AudioAllocationGuard guard;
            processor.processBlock(audio,midi);
        }
        REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
    }
    LoopCapture::View view() const { return processor.getLoopCaptureView(); }
    void expect(Phase phase) const
    {
        const auto deadline = juce::Time::getMillisecondCounter() + 5000;
        while (view().phase == Phase::settling && juce::Time::getMillisecondCounter() < deadline)
            juce::Thread::sleep(2);
        REQUIRE(view().phase == phase);
    }
    void verifyWav(int count, int changeAt, float first, float second) const
    {
        juce::WavAudioFormat wav;
        auto stream = view().wav.createInputStream();
        REQUIRE(stream != nullptr);
        const std::unique_ptr<juce::AudioFormatReader> reader(wav.createReaderFor(stream.release(),true));
        REQUIRE(reader && reader->sampleRate == 48000 && reader->numChannels == 2 && reader->lengthInSamples == count);
        juce::AudioBuffer<float> decoded(2,count);
        REQUIRE(reader->read(&decoded,0,count,0,true,true));
        for (int i=0; i<count; ++i)
        {
            const auto expected = i < changeAt ? first : second;
            REQUIRE(decoded.getSample(0,i) == expected && decoded.getSample(1,i) == -expected);
        }
    }
};

void restoreDuringSettlement()
{
    TestDirectory root;
    const auto location=std::make_shared<const CaptureStorageLocation>(CaptureStorageLocation{root.file,root.file,{}});
    const LoopCapture::Session target{location,juce::Uuid().toDashedString(),juce::Uuid().toDashedString(),1};
    std::atomic<uint64_t> generation{1};
    juce::WaitableEvent publishing, resume;
    LoopCapture loop([&]() -> std::optional<LoopCapture::Session> {
        publishing.signal();
        resume.wait();
        return target;
    },[&]{return generation.load();},[]{});
    loop.prepare(48000);
    REQUIRE(loop.setEnabled(true) && loop.arm(target));
    juce::AudioBuffer<float> audio(2,64);
    audio.clear();
    audio.setSample(0,3,.25f);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOff(1,LoopCapture::gateNote),0);
    midi.addEvent(juce::MidiMessage::noteOn(1,LoopCapture::gateNote,static_cast<juce::uint8>(100)),2);
    midi.addEvent(juce::MidiMessage::noteOff(1,LoopCapture::gateNote),8);
    loop.process(audio,midi,true,1);
    const auto ready=publishing.wait(5000);
    if(ready)
    {
        generation.store(2);
        loop.restore(juce::XmlElement("PreverbState"),2);
    }
    resume.signal();
    loop.shutdown();
    REQUIRE(ready && !loop.hasAcceptedLoop() && loop.view().wav==juce::File{});
}
}

int main()
{
    return runTest([]{
        juce::ScopedJuceInitialiser_GUI gui;
        Fixture f;
        auto& p=f.processor;
        REQUIRE(p.acceptsMidi() && !p.producesMidi() && p.getParameters().isEmpty());
        REQUIRE(p.getTotalNumInputChannels()==2 && p.getTotalNumOutputChannels()==2);
        juce::MemoryBlock songState;
        p.getStateInformation(songState);
        REQUIRE(!f.view().enabled && !f.view().acceptedLoop && !p.armLoopCapture());
        REQUIRE(p.setLoopCaptureEnabled(true));
        p.setNonRealtime(true);
        REQUIRE(!p.armLoopCapture());
        p.setNonRealtime(false);
        f.edge(0,true);
        f.process();
        REQUIRE(p.armLoopCapture());
        f.edge(3,true);
        f.edge(11,false,12);
        f.process();
        f.expect(Phase::waitForLow);
        f.edge(7,false);
        f.process();
        f.expect(Phase::waitForRise);
        REQUIRE(!p.setLoopCaptureEnabled(false) && !p.setRecaptureExisting(true));
        REQUIRE(!p.requestWorkflowMode(workflow::Mode::live));
        f.edge(5,true);
        f.edge(7,true);
        f.edge(14,false);
        f.edge(26,false);
        f.process();
        f.expect(Phase::ready);
        f.verifyWav(9,9,.25f,.25f);
        REQUIRE(f.view().waveform.size==9 && f.view().waveform.peaks[0].minimum==-.25f
                && f.view().waveform.peaks[0].maximum==.25f);
        const auto first = f.view().wav;
        REQUIRE(f.view().acceptedLoop && !p.getSongTime().valid && p.getCaptureSnapshot().manifest.revision==0);

        REQUIRE(p.armLoopCapture());
        f.edge(61,true);
        f.process(.25f);
        f.expect(Phase::recording);
        f.edge(2,false);
        f.process(.5f);
        f.expect(Phase::ready);
        f.verifyWav(5,3,.25f,.5f);
        const auto accepted = f.view().wav;
        REQUIRE(accepted != first && first.existsAsFile());
        juce::MemoryBlock loopState;
        p.getStateInformation(loopState);

        REQUIRE(p.armLoopCapture());
        f.edge(1,true);
        f.process();
        p.cancelLoopCapture();
        f.edge(8,false);
        f.process();
        f.expect(Phase::cancelled);
        REQUIRE(f.view().wav == accepted);
        REQUIRE(p.armLoopCapture());
        f.edge(4,true);
        f.edge(8,false);
        f.process(0);
        f.expect(Phase::silent);
        REQUIRE(p.armLoopCapture());
        f.edge(4,true);
        f.edge(8,false);
        f.process(std::numeric_limits<float>::quiet_NaN());
        f.expect(Phase::invalidAudio);
        REQUIRE(p.armLoopCapture());
        f.edge(4,true);
        f.process();
        p.reset();
        REQUIRE(f.view().phase==Phase::interrupted && f.view().busy());
        f.process();
        f.expect(Phase::interrupted);
        REQUIRE(p.armLoopCapture());
        f.edge(0,false);
        f.edge(4,true);
        f.process();
        p.prepareToPlay(44100,64);
        f.process();
        f.expect(Phase::interrupted);
        p.prepareToPlay(8000,64);
        REQUIRE(p.armLoopCapture());
        f.edge(0,false);
        f.edge(1,true);
        f.process();
        for (int i=0;i<1875;++i)
            f.process();
        f.expect(Phase::tooLong);
        REQUIRE(f.view().wav == accepted);
        REQUIRE(p.armLoopCapture());
        f.edge(64,false);
        f.process();
        f.expect(Phase::invalidAudio);
        REQUIRE(p.armLoopCapture());
        juce::AudioBuffer<float> mono(1,64);
        p.processBlock(mono,f.midi);
        f.expect(Phase::invalidAudio);
        REQUIRE(f.view().wav == accepted);

        const auto restore = [&](const juce::MemoryBlock& state)
        {
            p.setStateInformation(state.getData(),static_cast<int>(state.getSize()));
            REQUIRE(p.flushCaptureForTesting());
            const auto deadline=juce::Time::getMillisecondCounter()+5000;
            while(f.view().checking && juce::Time::getMillisecondCounter()<deadline)
                juce::Thread::sleep(2);
            REQUIRE(!f.view().checking);
        };
        restore(loopState);
        REQUIRE(f.view().enabled && f.view().acceptedLoop && f.view().wav == accepted && f.view().waveform.size==5);
        REQUIRE(accepted.appendText("invalid checksum"));
        restore(loopState);
        REQUIRE(f.view().acceptedLoop && f.view().wav == juce::File{});
        REQUIRE(accepted.deleteFile());
        restore(loopState);
        REQUIRE(f.view().acceptedLoop && f.view().wav == juce::File{});

        restore(songState);
        REQUIRE(!f.view().enabled && !f.view().acceptedLoop);
        Host host;
        p.setPlayHead(&host);
        p.prepareToPlay(48000,64);
        host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
        host.position.setBpm(120);
        host.position.setPpqPosition(0);
        host.position.setTimeInSamples(0);
        host.position.setIsPlaying(true);
        f.edge(1,true);
        f.edge(9,false);
        f.process();
        REQUIRE(p.getWorkflowSnapshot().phase==workflow::Phase::recording);
        REQUIRE(!p.setLoopCaptureEnabled(true) && !p.armLoopCapture());
        host.position.setTimeInSamples(64);
        host.position.setPpqPosition(64.0/24000);
        host.position.setIsPlaying(false);
        f.process();
        REQUIRE(p.flushCaptureForTesting() && !p.getCaptureSnapshot().manifest.ranges.empty());
        REQUIRE(!f.view().acceptedLoop);
        const auto songCapture=p.getCaptureSnapshot();
        REQUIRE(p.setLoopCaptureEnabled(true) && p.armLoopCapture());
        f.edge(1,true);
        f.edge(9,false);
        f.process(0);
        f.expect(Phase::silent);
        REQUIRE(!f.view().acceptedLoop && p.getCaptureSnapshot().reference==songCapture.reference);
        REQUIRE(p.requestWorkflowMode(workflow::Mode::live));
        REQUIRE(!p.armLoopCapture() && p.requestWorkflowMode(workflow::Mode::capture));
        REQUIRE(p.armLoopCapture());
        f.edge(1,true);
        f.edge(9,false);
        f.process();
        f.expect(Phase::ready);
        REQUIRE(f.view().acceptedLoop && p.getCaptureSnapshot().reference==songCapture.reference);
        REQUIRE(p.setLoopCaptureEnabled(false));
        REQUIRE(!p.requestWorkflowMode(workflow::Mode::live));
        REQUIRE(!p.requestWorkflowMode(workflow::Mode::finalise));
        host.position.setTimeInSamples(64);
        host.position.setPpqPosition(64.0/24000);
        host.position.setIsPlaying(true);
        f.process();
        host.position.setTimeInSamples(128);
        host.position.setPpqPosition(128.0/24000);
        host.position.setIsPlaying(false);
        f.process();
        REQUIRE(p.flushCaptureForTesting());
        REQUIRE(!f.view().acceptedLoop && p.getCaptureSnapshot().manifest.revision>songCapture.manifest.revision);
        p.setPlayHead(nullptr);

        Fixture blocked;
        const auto instance=blocked.processor.getCaptureSnapshot().manifest;
        const auto obstruction=blocked.root.file.getChildFile(instance.projectId).getChildFile(instance.instanceId).getChildFile("loop-takes");
        REQUIRE(obstruction.getParentDirectory().createDirectory().wasOk());
        REQUIRE(obstruction.replaceWithText("Storage failure fixture"));
        REQUIRE(blocked.processor.setLoopCaptureEnabled(true) && blocked.processor.armLoopCapture());
        blocked.edge(0,false);
        blocked.edge(1,true);
        blocked.edge(9,false);
        blocked.process();
        blocked.expect(Phase::storageError);
        REQUIRE(!blocked.view().acceptedLoop && blocked.view().wav==juce::File{});
        restoreDuringSettlement();
    });
}
