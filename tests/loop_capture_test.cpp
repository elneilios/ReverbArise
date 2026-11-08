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

void restoreLegacyGate(PreverbAudioProcessor& processor)
{
    juce::MemoryBlock bytes;
    processor.getStateInformation(bytes);
    const auto xml=juce::AudioProcessor::getXmlFromBinary(bytes.getData(),static_cast<int>(bytes.getSize()));
    REQUIRE(xml);
    xml->removeAttribute("learnLoopMethod");
    xml->setAttribute("learnLoopEnabled",true);
    juce::AudioProcessor::copyXmlToBinary(*xml,bytes);
    processor.setStateInformation(bytes.getData(),static_cast<int>(bytes.getSize()));
    REQUIRE(processor.flushCaptureForTesting());
    REQUIRE(processor.getLoopCaptureView().settings.method==LoopCapture::Method::legacyGate);
    REQUIRE(processor.setLoopCaptureEnabled(false));
}

void musicalCapture()
{
    Fixture f;
    auto& processor=f.processor;
    constexpr int beatFrames=6000, offset=5;
    processor.prepareToPlay(48000,beatFrames);
    f.audio.setSize(2,beatFrames);
    REQUIRE(processor.setLoopCaptureEnabled(true));
    REQUIRE(f.view().settings.method==LoopCapture::Method::followGate);
    auto beat=[&](bool bar, bool phrase=false, float value=.25f){
        // Clock notes arrive in any MIDI order at a shared callback offset.
        if(phrase)f.edge(offset,true,LoopCapture::phraseNote);
        if(bar)f.edge(offset,true,LoopCapture::barNote);
        f.edge(offset,true,LoopCapture::beatNote);
        f.process(value);
    };
    f.edge(0,false);
    beat(true);beat(false);beat(false);beat(false);
    REQUIRE(processor.armLoopCapture());
    f.edge(offset,true);
    beat(true,true);
    f.expect(Phase::recording);
    beat(false);
    f.edge(offset+20,false);
    beat(false);
    f.expect(Phase::waitForBar);
    beat(false);beat(true,false,.5f);
    f.expect(Phase::ready);
    f.verifyWav(4*beatFrames,4*beatFrames-offset,.25f,.5f);
    REQUIRE((f.view().musicalLength==LoopCapture::MusicalLength{4,4}));
    const auto accepted=f.view().wav;

    REQUIRE(processor.armLoopCapture());
    f.edge(offset,true);
    beat(false);
    f.expect(Phase::invalidStart);
    REQUIRE(f.view().wav==accepted);
    f.edge(offset,false);
    beat(false);beat(false);
    REQUIRE(processor.armLoopCapture());
    f.edge(offset,true);
    beat(true,true);
    beat(false);beat(false);beat(false);
    f.edge(offset,false);
    beat(true);
    f.expect(Phase::ready);
    f.verifyWav(4*beatFrames,4*beatFrames,.25f,.25f);

    REQUIRE(processor.setLoopCaptureSettings({LoopCapture::Method::fixedLength,1,2}));
    REQUIRE(processor.armLoopCapture());
    beat(false);beat(false);beat(false);
    f.edge(offset,true);
    beat(true,true);
    f.edge(offset+20,false);
    beat(false);
    f.expect(Phase::recording);
    beat(false);beat(false);beat(true);beat(false);
    beat(false,false,.5f);
    f.expect(Phase::ready);
    f.verifyWav(6*beatFrames,6*beatFrames-offset,.25f,.5f);
    REQUIRE((f.view().musicalLength==LoopCapture::MusicalLength{6,4}));
    const auto fixed=f.view().wav;
    juce::MemoryBlock state;
    processor.getStateInformation(state);

    REQUIRE(processor.setLoopCaptureSettings({LoopCapture::Method::followGate,1,0}));
    REQUIRE(processor.armLoopCapture());
    beat(false);
    f.edge(offset,true);
    beat(true,true);
    f.edge(offset+20,false);
    beat(false);
    f.expect(Phase::waitForBar);
    f.process();
    f.expect(Phase::interrupted);
    REQUIRE(f.view().wav==fixed);

    processor.setStateInformation(state.getData(),static_cast<int>(state.getSize()));
    REQUIRE(processor.flushCaptureForTesting());
    const auto deadline=juce::Time::getMillisecondCounter()+5000;
    while(f.view().checking && juce::Time::getMillisecondCounter()<deadline)
        juce::Thread::sleep(2);
    REQUIRE(f.view().wav==fixed && (f.view().musicalLength==LoopCapture::MusicalLength{6,4})
            && (f.view().settings==LoopCapture::Settings{LoopCapture::Method::fixedLength,1,2}));
}

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
    juce::XmlElement legacy("PreverbState");
    legacy.setAttribute("learnLoopEnabled",true);
    loop.restore(legacy,1);
    REQUIRE(loop.arm(target));
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

void phaseAndMix()
{
    using Status=LoopPlayback::Status;
    constexpr int period=512, lead=4;
    WetRender wet;
    wet.start=-lead;
    wet.samples.resize(2*(period+lead),.0625f);
    wet.peak=.0625f;
    REQUIRE(!LoopPlayback::prepare(wet,period,10,1));
    auto mix=LoopPlayback::prepare(wet,period,8192,1);
    REQUIRE(mix && mix->lead==lead && mix->repeat.size()==2*period);
    for (int offset=0;offset<period;++offset)
        REQUIRE(mix->repeat[2*offset]==(offset>=period-lead?.125f:.0625f));
    LoopPlayback playback;
    playback.configure(48000,period);
    playback.publish(std::move(mix));
    juce::MidiBuffer midi;
    std::array<float,64> left{},right{};
    int position=0;
    const auto process=[&](int target,const std::vector<std::pair<int,int>>& events,auto expected,double rate=48000.0,bool reset=false)
    {
        for (;position<target;)
        {
            const auto count=std::min(64,target-position);
            midi.clear();
            for (const auto& [at,note]:events)
                if (at>=position && at<position+count)
                    midi.addEvent(juce::MidiMessage::noteOn(1,note,static_cast<juce::uint8>(100)),at-position);
            left.fill(.25f);
            right.fill(-.25f);
            {
                AudioAllocationGuard guard;
                playback.process(left.data(),right.data(),count,rate,midi,true,reset);
            }
            REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
            for (int i=0;i<count;++i)
            {
                const auto value=expected(position+i);
                REQUIRE(std::abs(left[i]-(.25f+value))<1e-7f && std::abs(right[i]-(-.25f+value))<1e-7f);
            }
            position+=count;
            reset=false;
        }
    };
    constexpr int start=4*period;
    constexpr int marker=LoopPlayback::phraseNote,cue=LoopPlayback::cueNote,stop=LoopPlayback::stopNote;
    const auto dry=[](int){return 0.f;};
    REQUIRE(!playback.cue());
    process(3*period,{{0,marker},{period,marker},{period,marker},{2*period,marker},
        {2*period,cue},{2*period,stop},{3*period-3,cue}},dry);
    REQUIRE(playback.status()==Status::queued && !playback.cue());
    auto next=wet;
    std::fill(next.samples.begin(),next.samples.end(),.03125f);
    playback.publish(LoopPlayback::prepare(next,period,8192,2));
    const auto wetAt=[=](int at)
    {
        if(at<start-lead)return 0.f;
        if(at<start)return .0625f;
        return (at-start)%period>=period-lead?.125f:.0625f;
    };
    process(start,{{3*period,marker}},wetAt);
    REQUIRE(playback.status()==Status::leadIn);
    process(start+period,{{start,marker}},wetAt);
    REQUIRE(playback.status()==Status::repeating);
    const auto fade=[&](int at){return wetAt(at)*std::max(0,240-(at-start-period))/240.f;};
    process(start+2*period,{},fade);
    REQUIRE(playback.status()==Status::syncLost);
    process(start+4*period+1,{{start+2*period,marker},{start+3*period,marker},{start+4*period,marker}},dry);
    REQUIRE(playback.status()==Status::standby && playback.cue());
    process(start+4*period+2,{},dry);
    REQUIRE(playback.status()==Status::queued);
    playback.stop();
    process(start+4*period+3,{},dry);
    REQUIRE(playback.status()==Status::standby);
    process(start+4*period+4,{},dry,44100.0);
    REQUIRE(playback.status()==Status::waiting);
    const auto restart=position;
    process(restart+period+1,{{restart,marker},{restart+period-1,marker},{restart+period,cue}},dry);
    REQUIRE(playback.status()==Status::referenceMismatch && !playback.cue());
    process(restart+period+2,{},dry,48000.0,true);
    REQUIRE(playback.status()==Status::waiting);
    playback.collect();
    REQUIRE(playback.pinnedBytes()==(period+lead)*2*sizeof(float));
}
}

int main()
{
    return runTest([]{
        juce::ScopedJuceInitialiser_GUI gui;
        phaseAndMix();
        musicalCapture();
        Fixture f;
        auto& p=f.processor;
        REQUIRE(p.acceptsMidi() && !p.producesMidi() && p.getParameters().isEmpty());
        REQUIRE(p.getTotalNumInputChannels()==2 && p.getTotalNumOutputChannels()==2);
        juce::MemoryBlock songState;
        p.getStateInformation(songState);
        REQUIRE(!f.view().enabled && !f.view().acceptedLoop && !p.armLoopCapture());
        restoreLegacyGate(p);
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
        REQUIRE(!p.requestWorkflowMode(workflow::Mode::live));
        p.prepareToPlay(48000,64);
        REQUIRE(p.requestWorkflowMode(workflow::Mode::live));
        const auto prepared=[&](bool expected=true)
        {
            const auto deadline=juce::Time::getMillisecondCounter()+5000;
            auto view=p.getLoopPerformanceView();
            while(view.status=="Preparing Loop swell..." && juce::Time::getMillisecondCounter()<deadline)
            {
                juce::Thread::sleep(2);
                view=p.getLoopPerformanceView();
            }
            if(view.status=="Preparing Loop swell..." || static_cast<bool>(view.wet)!=expected)
                throw std::runtime_error(view.status.toStdString());
            return view;
        };
        auto loop=prepared();
        REQUIRE(loop.frames==5 && loop.rate==48000 && loop.wet->front().startSample<0);
        const auto binding=loop.binding;
        auto edit=loop.edit;
        edit.placement=-2;
        edit.lead=3;
        edit.fadeOut=0;
        auto superseded=edit;
        superseded.first=1;
        superseded.placement=5;
        REQUIRE(p.updateLoopSwell(superseded,binding));
        REQUIRE(p.updateLoopSwell(edit,binding));
        loop=prepared();
        REQUIRE(loop.edit==edit && loop.wet->front().startSample==-5);
        f.process();
        REQUIRE(p.auditionLoopSwell(binding));
        f.process();
        REQUIRE(f.audio.getMagnitude(0,64)>.25f);
        REQUIRE(p.requestWorkflowMode(workflow::Mode::capture) && p.armLoopCapture());
        f.edge(0,false);
        f.edge(4,true);
        f.edge(8,false);
        f.process(0);
        f.expect(Phase::silent);
        REQUIRE(p.requestWorkflowMode(workflow::Mode::live));
        REQUIRE(prepared().edit==edit && f.view().wav==accepted);
        p.getStateInformation(loopState);
        restore(loopState);
        REQUIRE(p.requestWorkflowMode(workflow::Mode::live));
        REQUIRE(prepared().edit==edit && !p.updateLoopSwell(edit,binding));
        REQUIRE(p.requestWorkflowMode(workflow::Mode::capture));
        REQUIRE(accepted.appendText("invalid checksum"));
        REQUIRE(p.requestWorkflowMode(workflow::Mode::live));
        REQUIRE(prepared(false).status=="The Loop WAV no longer matches the accepted take.");
        restore(loopState);
        REQUIRE(f.view().acceptedLoop && f.view().wav == juce::File{});
        REQUIRE(accepted.deleteFile());
        restore(loopState);
        REQUIRE(f.view().acceptedLoop && f.view().wav == juce::File{});

        restore(songState);
        restoreLegacyGate(p);
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
        REQUIRE(p.requestWorkflowMode(workflow::Mode::live));
        REQUIRE(p.requestWorkflowMode(workflow::Mode::capture));
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
        restoreLegacyGate(blocked.processor);
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
