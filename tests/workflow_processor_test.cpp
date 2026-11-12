#include "TestSupport.h"
#include "AudioTestFiles.h"
#include "AllocationGuard.h"
#include <Capture/LoopMidiObservations.h>
#include <Capture/LoopBeatClock.h>
#include <Capture/LoopPhraseClock.h>
#include <Capture/SourceRangeReader.h>
#include "LoopCaptureFixture.h"
#include <Plugin/PluginProcessor.h>
#include <UI/WorkflowPresentation.h>
#include <Core/MonitoringSafety.h>
#include "ProcessorCaptureFailureCases.h"
#include "ProcessorReactivationCases.h"

struct WorkflowHost:juce::AudioPlayHead
{PositionInfo position;juce::Optional<PositionInfo> getPosition()const override{return position;}};
struct SavedStateListener final:juce::AudioProcessorListener
{
    unsigned changes=0;bool wrongThread=false;
    void audioProcessorParameterChanged(juce::AudioProcessor*,int,float)override{}
    void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails& details)override
    {if(details.nonParameterStateChanged){++changes;wrongThread|=!juce::MessageManager::getInstance()->isThisTheMessageThread();}}
};

#include "SharedSwellEngineCases.h"

static void loopMidiCases()
{
    using namespace loop_capture;
    juce::MidiBuffer midi;
    MidiObservations observations;
    const auto read = [&](MidiProtocol protocol = MidiProtocol::rack)
    {
        { AudioAllocationGuard guard; readLoopObservations(midi, 128, observations, true, protocol); }
        REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
    };
    const auto on = [&](int note, int offset, int channel = 1)
    { midi.addEvent(juce::MidiMessage::noteOn(channel, note, static_cast<juce::uint8>(100)), offset); };
    on(19, 0); on(21, 0); on(12, 0); on(22, 0);
    on(16, 17); on(17, 17); on(18, 17);
    on(21, 22, 2);
    midi.addEvent(juce::MidiMessage::noteOff(1, 14), 127);
    read();
    REQUIRE(observations.error == MidiObservations::Error::none && observations.size == 3);
    REQUIRE(observations.samples[0].sampleOffset == 0 && observations.samples[0].roles == (bar | beat | phrase | reset));
    REQUIRE(observations.samples[1].sampleOffset == 17 && observations.samples[1].roles == (cue | finish | stop));
    REQUIRE(observations.samples[2].sampleOffset == 127 && observations.samples[2].roles == gateLow);

    midi.clear();
    on(23, 3); on(24, 3); on(25, 39); on(17, 39);
    midi.addEvent(juce::MidiMessage::noteOff(1, 25), 50);
    read();
    REQUIRE(observations.size == 2);
    REQUIRE(observations.samples[0].sampleOffset == 3 && observations.samples[0].roles == (captureClock | captureArm));
    REQUIRE(observations.samples[1].sampleOffset == 39 && observations.samples[1].roles == (captureStop | finish));

    on(14, 128);
    read();
    REQUIRE(observations.error == MidiObservations::Error::invalidOffset && observations.size == 0);
    midi.clear(); on(21, -1); read();
    REQUIRE(observations.error == MidiObservations::Error::invalidOffset && observations.size == 0);
    midi.clear();
    for (unsigned i = 0; i < MidiObservations::maximumEvents; ++i) on(21, 8, 2);
    read();
    REQUIRE(observations.error == MidiObservations::Error::none && observations.size == 0);
    on(14, 10); read();
    REQUIRE(observations.error == MidiObservations::Error::overflow && observations.size == 0);
    midi.clear(); on(14, 10);
    midi.addEvent(juce::MidiMessage::noteOff(1, 14), 10);
    read();
    REQUIRE(observations.error == MidiObservations::Error::ambiguousGate && observations.size == 0);

    midi.clear();
    on(23, 0); on(60, 1); on(62, 1);
    on(12, 4); on(16, 8); on(17, 12); on(18, 16); on(24, 20); on(25, 24);
    midi.addEvent(juce::MidiMessage::noteOff(1, 18), 28);
    for (const auto protocol : {MidiProtocol::flStudio, MidiProtocol::rack})
    {
        read(protocol);
        const auto first = protocol == MidiProtocol::rack ? 1u : 0u;
        REQUIRE(observations.error == MidiObservations::Error::none && observations.size == first + 6);
        constexpr std::array<uint16_t,6> shared{phrase,cue,finish,stop,captureArm,captureStop};
        for (unsigned i = 0; i < shared.size(); ++i)
            REQUIRE(observations.samples[first+i].sampleOffset == 4 * static_cast<int>(i+1)
                    && observations.samples[first+i].roles == shared[i]);
    }
}

static void loopPhraseClockCases()
{
    using namespace loop_capture;
    for (const auto [frames, bpm] : { std::pair{128, 137}, std::pair{256, 280} })
    {
        PhraseClock clock, other;
        const auto sample = [&](int beatIndex) {return 100 + (2880000LL * beatIndex + bpm / 2) / bpm;};
        const auto roles = [](int index) -> uint16_t
        {return beat | (index % 4 == 0 ? bar : 0) | (index % 16 == 0 ? phrase : 0);};
        REQUIRE(!clock.observe(sample(0), roles(0)).verifiedPhrase);
        REQUIRE(!measuredProfile(44100,frames).valid());
        clock.configure(measuredProfile(48000,frames));
        REQUIRE(clock.tempo() == bpm && clock.state() == PhraseClock::State::notVerified);
        REQUIRE(clock.observe(sample(0), roles(0) | reset).revoked);
        for (int i = 1; i < 16; ++i)
            REQUIRE(!clock.observe(sample(i), roles(i)).verifiedBeat);
        for (int i = 16; i <= 48; ++i)
        {
            const auto boundary = clock.observe(sample(i), roles(i));
            REQUIRE(!boundary.revoked);
            REQUIRE(boundary.verifiedBeat == (i >= 18));
            REQUIRE(boundary.verifiedPhrase == (i == 32 || i == 48));
            REQUIRE(clock.phaseReady() == (i == 48));
        }
        REQUIRE(other.state() == PhraseClock::State::notConfigured);
        REQUIRE(!clock.advance(sample(48) + 1));
        REQUIRE(clock.observe(sample(50), cue).revoked);
        REQUIRE(clock.state() == PhraseClock::State::syncLost && !clock.phaseReady());

        for (int i = 64; i <= 67; ++i)
            REQUIRE(!clock.observe(sample(i), roles(i)).revoked);
        REQUIRE(clock.state() == PhraseClock::State::ready);
        REQUIRE(clock.observe(sample(68), beat).revoked); // Missing coincident bar.
        REQUIRE(clock.state() == PhraseClock::State::syncLost);
        clock.configure(measuredProfile(96000,frames));
        REQUIRE(!clock.observe(sample(80), roles(80)).verifiedPhrase);
        REQUIRE(clock.state() == PhraseClock::State::notConfigured);
    }
    ClockAdmission admission;
    admission.configure({48000,120},measuredProfile(48000,128));
    admission.observe(0,phrase|bar|beat);
    admission.observe(21022,beat);
    admission.observe(42044,beat);
    REQUIRE(admission.state()==ClockAdmission::State::ready && admission.tempo()==137);
}

static void loopBeatClockCases()
{
    loop_capture::BeatClock clock;
    clock.begin(48000);
    REQUIRE(clock.observe(0) && !clock.verified());
    REQUIRE(clock.observe(21022) && !clock.verified());
    REQUIRE(clock.observe(42044) && clock.verified());
    REQUIRE(clock.observe(63066));
    REQUIRE(clock.observe(84088));
    REQUIRE(std::abs(clock.tempo(84088, 4) - 137) < 0.01);
    REQUIRE(!clock.expired(105110));
    REQUIRE(clock.expired(105112));

    clock.begin(44100);
    REQUIRE(clock.observe(19));
    REQUIRE(clock.observe(22069));
    REQUIRE(!clock.observe(45000));

    clock.begin(48000);
    REQUIRE(clock.observe(0));
    REQUIRE(clock.observe(24000));
    REQUIRE(!clock.observe(24020));

    for (const auto firstInterval : {24000, 24002})
    {
        clock.begin(48000);
        REQUIRE(clock.observe(0));
        REQUIRE(clock.observe(firstInterval));
        REQUIRE(!clock.observe(48002) && !clock.verified());
    }
    for (const auto firstInterval : {24000, 24001})
    {
        clock.begin(48000);
        REQUIRE(clock.observe(0));
        REQUIRE(clock.observe(firstInterval));
        REQUIRE(clock.observe(48001) && clock.verified());
        REQUIRE(clock.tempo(48001, 2) > 0);
    }
    for (const auto period : {9600, 96000})
    {
        clock.begin(48000);
        REQUIRE(clock.observe(0) && clock.observe(period) && clock.observe(2 * period));
        REQUIRE(clock.verified() && clock.tempo(2 * period, 2) > 0);
    }
}

static void loopTakeCases()
{
    using namespace loop_capture;
    using Edge=Take::Edge;
    const LoopCapturePreference endSignal{true,LoopCapturePreference::Method::endSignal};
    for (const bool coincident : {false,true})
    {
        Take take;
        take.arm(endSignal);
        REQUIRE(take.observe(0,gateRise,{true,true,true})==Edge::none);
        REQUIRE(take.observe(1,gateLow,{})==Edge::none);
        REQUIRE(take.observe(10,gateRise,{true,true,true})==Edge::start);
        REQUIRE(take.observe(20,gateRise,{true,false,false})==Edge::none);
        REQUIRE(take.observe(30,gateLow,{true,true,false})==Edge::none); // Unqualified Rack note-off.
        if (!coincident) REQUIRE(take.observe(35,0,{},true)==Edge::none);
        REQUIRE(take.observe(40,bar,{true,true,false},coincident)==Edge::end);
        REQUIRE(take.timing(137,40).endSample==30);
        REQUIRE(take.observe(50,bar,{true,true,false},true)==Edge::none);
    }
    Take take;
    take.arm(endSignal);
    REQUIRE(take.observe(0,gateLow,{})==Edge::none);
    REQUIRE(take.observe(1,gateRise,{true,true,true})==Edge::start);
    REQUIRE(take.observe(2,0,{},true)==Edge::none);
    REQUIRE(take.observe(3,bar,{true,true,false,true})==Edge::invalid);
    REQUIRE(take.observe(4,bar,{true,true,false},true)==Edge::none);
    take.arm({true});
    REQUIRE(take.observe(0,phrase,{true,true,true})==Edge::start);
    REQUIRE(take.observe(Take::maximumFrames+1,beat,{true,false,false})==Edge::invalid);
}

static void loopStateCases()
{
    TestDirectory root;
    auto processor = std::make_unique<PreverbAudioProcessor>(root.file);
    auto other = std::make_unique<PreverbAudioProcessor>(root.file);
    juce::MemoryBlock oldSong;
    processor->getStateInformation(oldSong);
    const auto state = juce::AudioProcessor::getXmlFromBinary(oldSong.getData(), static_cast<int>(oldSong.getSize()));
    REQUIRE(state && state->getChildByName("AcceptedLoopSource") == nullptr && state->getChildByName("NextCapture") == nullptr);

    CaptureSession source(root.file);
    CaptureGenerationContext context;
    context.id = captureGenerationContextId(context);
    constexpr uint32_t frames = 84088;
    const std::vector<float> audio(frames * 2, 0.25f);
    REQUIRE(source.commit(0, 48000, audio.data(), frames, 0, &context));
    auto loop = source.snapshot().manifest;
    loop.schemaVersion = CaptureManifest::loopSchemaVersion;
    loop.loop = LoopPhraseTiming{137, 4, 4, 4, 42, frames};
    auto sourceRoot = root.file.getChildFile(loop.projectId).getChildFile(loop.instanceId);
    juce::String reference, error;
    REQUIRE(CaptureAssetStore(sourceRoot).publishSnapshot(loop, reference, error));
    state->setAttribute("projectId", juce::String(loop.projectId));
    state->setAttribute("instanceId", juce::String(loop.instanceId));
    state->setAttribute("manifestRevision", juce::String(static_cast<juce::int64>(loop.revision)));
    state->setAttribute("manifestReference", reference);
    state->createNewChildElement("AcceptedLoopSource")->setAttribute("version", 1);
    juce::MemoryBlock fixture;
    juce::AudioProcessor::copyXmlToBinary(*state, fixture);
    processor->setStateInformation(fixture.getData(), static_cast<int>(fixture.getSize()));
    REQUIRE(processor->flushCaptureForTesting());
    auto accepted = processor->getCaptureSnapshot();
    REQUIRE(!accepted.unavailable && accepted.reference == reference && accepted.manifest.loop == loop.loop);
    REQUIRE(accepted.manifest.revision == loop.revision && accepted.sourceKind() == CaptureSourceKind::loop);
    REQUIRE(processor->getLoopCapturePreference() == LoopCapturePreference{});

    LoopCapturePreference next{true, LoopCapturePreference::Method::duration, LoopCapturePreference::Unit::beats, 3};
    REQUIRE(processor->setLoopCapturePreference(next));
    REQUIRE(other->getLoopCapturePreference() == LoopCapturePreference{});
    REQUIRE(other->getCaptureSnapshot().sourceKind() == CaptureSourceKind::song);
    juce::MemoryBlock saved;
    processor->getStateInformation(saved);
    other->setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    REQUIRE(other->flushCaptureForTesting());
    accepted = other->getCaptureSnapshot();
    REQUIRE(!accepted.unavailable && accepted.manifest.loop == loop.loop && accepted.reference == reference);
    REQUIRE(other->getLoopCapturePreference() == next && !other->getWorkflowSnapshot().locked());

    auto invalid = juce::AudioProcessor::getXmlFromBinary(saved.getData(), static_cast<int>(saved.getSize()));
    REQUIRE(invalid);
    invalid->getChildByName("NextCapture")->setAttribute("count", 0);
    juce::AudioProcessor::copyXmlToBinary(*invalid, fixture);
    other->setStateInformation(fixture.getData(), static_cast<int>(fixture.getSize()));
    REQUIRE(other->getLoopCapturePreference() == next && other->getCaptureSnapshot().reference == reference);

    auto repeat = next;
    repeat.method = LoopCapturePreference::Method::endOnRepeat;
    repeat.count = LoopCapturePreference::maximumCount;
    REQUIRE(processor->setLoopCapturePreference(repeat));
    juce::MemoryBlock repeatState;
    processor->getStateInformation(repeatState);
    other->setStateInformation(repeatState.getData(), static_cast<int>(repeatState.getSize()));
    REQUIRE(other->flushCaptureForTesting() && other->getLoopCapturePreference() == repeat);
    REQUIRE(other->getCaptureSnapshot().reference == reference);

    REQUIRE(sourceRoot.getChildFile(loop.ranges.front().assetPath).deleteFile());
    other->setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    REQUIRE(other->flushCaptureForTesting());
    accepted = other->getCaptureSnapshot();
    REQUIRE(accepted.unavailable && accepted.sourceKind() == CaptureSourceKind::loop && accepted.reference == reference);
    REQUIRE(sourceRoot.getChildFile(reference).deleteFile());
    other->setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    REQUIRE(other->flushCaptureForTesting());
    accepted = other->getCaptureSnapshot();
    REQUIRE(accepted.unavailable && !accepted.manifestKnown && accepted.sourceKind() == CaptureSourceKind::loop);

    other->setStateInformation(oldSong.getData(), static_cast<int>(oldSong.getSize()));
    REQUIRE(other->flushCaptureForTesting());
    REQUIRE(other->getCaptureSnapshot().sourceKind() == CaptureSourceKind::song);
    REQUIRE(other->getLoopCapturePreference() == LoopCapturePreference{});
}

static void loopProcessorCases()
{
    struct Profile {int frames;double rate,declaredTempo;};
    for (const auto [frames,rate,declaredTempo] : {Profile{128,48000,0},Profile{256,48000,0},Profile{512,44100,121.5}})
    {
        LoopCaptureFixture fixture(frames,rate,declaredTempo);
        auto& processor=*fixture.processor;
        auto preference=processor.getLoopCapturePreference();
        if (frames==256)
        {
            preference.unit=LoopCapturePreference::Unit::beats;
            preference.count=3;
            REQUIRE(processor.setLoopCapturePreference(preference));
        }
        REQUIRE(processor.canArmLoopCapture());
        fixture.through(fixture.marker(2));
        if (declaredTempo>0)
        {
            auto tooLong=preference;
            tooLong.count=LoopCapturePreference::maximumCount;
            REQUIRE(processor.setLoopCapturePreference(tooLong));
            REQUIRE(!processor.canArmLoopCapture() && processor.loopCaptureStatus().contains("capture limit"));
            REQUIRE(processor.setLoopCapturePreference(preference));
        }
        REQUIRE(processor.canArmLoopCapture() && processor.armLoopCapture());
        REQUIRE(!processor.armLoopCapture());
        REQUIRE(processor.getWorkflowSnapshot().phase==workflow::Phase::armed);
        const auto start=fixture.marker(16), end=fixture.marker(frames==256?19:20);
        fixture.through(start);
        REQUIRE(processor.getWorkflowSnapshot().phase==workflow::Phase::recording);
        { AudioAllocationGuard guard; fixture.tick(); }
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        fixture.through(end,true);
        const auto accepted=processor.getCaptureSnapshot();
        REQUIRE(!processor.loopCaptureArmed() && accepted.sourceKind()==CaptureSourceKind::loop);
        REQUIRE(accepted.manifest.loop->originSample==start && accepted.manifest.loop->endSample==end-start);
        REQUIRE(accepted.manifest.loop->beats==(frames==256?3:4));
        REQUIRE(accepted.manifest.sampleRate==rate);
        REQUIRE(accepted.manifest.loop->tempoBpm==fixture.tempo);
        const auto grid=processor.getSourceEditTime(accepted);
        REQUIRE(grid.valid && grid.sample==0 && grid.ppq==0 && grid.bpm==fixture.tempo);
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::live) && !processor.requestLoopCue());
        REQUIRE(!processor.requestWorkflowMode(workflow::Mode::finalise));
        REQUIRE(processor.requestWorkflowMode(workflow::Mode::capture));
        const auto storeRoot=accepted.location->projectsRoot.getChildFile(accepted.manifest.projectId).getChildFile(accepted.manifest.instanceId);
        CaptureAssetStore store(storeRoot);
        SourceInterval pcm;juce::String error;
        REQUIRE(readSourceInterval(store,accepted.manifest,0,end-start,static_cast<size_t>(end-start),pcm,error));
        REQUIRE(pcm.samples.front()==fixture.signal(start) && pcm.samples[1]==-fixture.signal(start));
        REQUIRE(pcm.samples[pcm.samples.size()-2]==fixture.signal(end-1));
        REQUIRE(pcm.samples.back()==-fixture.signal(end-1));

        REQUIRE(!processor.armLoopCapture());
        REQUIRE(processor.setLoopRecapture(true) && processor.armLoopCapture());
        juce::MemoryBlock bytes;processor.getStateInformation(bytes);
        auto reopened=std::make_unique<PreverbAudioProcessor>(fixture.root.file);
        reopened->setStateInformation(bytes.getData(),static_cast<int>(bytes.getSize()));
        REQUIRE(reopened->flushCaptureForTesting());
        REQUIRE(!reopened->loopCaptureArmed() && !reopened->loopRecaptureEnabled() && !reopened->getWorkflowSnapshot().locked());
        const auto restored = reopened->getCaptureSnapshot();
        REQUIRE(restored.reference == accepted.reference && reopened->getLoopCapturePreference() == preference);
        REQUIRE(restored.sourceKind() == CaptureSourceKind::loop && !restored.unavailable);
        REQUIRE(restored.manifest.projectId == accepted.manifest.projectId
            && restored.manifest.instanceId == accepted.manifest.instanceId
            && restored.manifest.revision == accepted.manifest.revision
            && restored.manifest.sampleRate == rate && restored.manifest.loop == accepted.manifest.loop);
        SourceInterval restoredPcm;
        REQUIRE(readSourceInterval(store, restored.manifest, 0, end-start, static_cast<size_t>(end-start), restoredPcm, error));
        REQUIRE(restoredPcm.samples == pcm.samples && !reopened->armLoopCapture());
        if (frames==128)
        {
            fixture.tick(false,true);
            REQUIRE(!processor.loopCaptureArmed());
            REQUIRE(processor.getCaptureSnapshot().reference==accepted.reference);
            fixture.through(fixture.marker(34));
            REQUIRE(processor.canArmLoopCapture() && processor.armLoopCapture());
            processor.releaseResources();
            processor.prepareToPlay(44100,frames);
            fixture.tick();
            REQUIRE(!processor.loopCaptureArmed() && !processor.canArmLoopCapture());
            REQUIRE(processor.getCaptureSnapshot().reference==accepted.reference);
        }
        else if (frames==256)
        {
            processor.cancelCapturePass();
            REQUIRE(!processor.loopCaptureArmed());
            fixture.through(fixture.marker(32));
            REQUIRE(processor.getCaptureSnapshot().reference==accepted.reference);
            REQUIRE(processor.armLoopCapture());
            processor.holdCaptureWorkerForTesting(true);
            const auto deadline=juce::Time::getMillisecondCounter()+2000;
            while (!processor.captureWorkerHeldForTesting() && juce::Time::getMillisecondCounter()<deadline)
                juce::Thread::sleep(1);
            REQUIRE(processor.captureWorkerHeldForTesting());
            fixture.through(fixture.marker(48)+static_cast<int64_t>(frames)*PluginCaptureQueue::capacity);
            processor.holdCaptureWorkerForTesting(false);
            REQUIRE(processor.flushCaptureForTesting());
            REQUIRE(!processor.loopCaptureArmed() && processor.getWorkflowSnapshot().result==workflow::Result::failed);
            REQUIRE(processor.getCaptureSnapshot().reference==accepted.reference);
        }
        else
        {
            fixture.through(fixture.marker(32));
            REQUIRE(processor.getWorkflowSnapshot().phase==workflow::Phase::recording);
            fixture.host.position.setBpm(122);
            fixture.tick();
            REQUIRE(processor.flushCaptureForTesting());
            REQUIRE(processor.getWorkflowSnapshot().result==workflow::Result::cancelled);
            REQUIRE(processor.getCaptureSnapshot().reference==accepted.reference);
        }
    }
}

static void simpleLoopProcessorCases()
{
    for (const int steps : {4, 8, 16})
    {
        LoopCaptureFixture sequence;
        sequence.simpleClock = sequence.simplePhrase = true;
        sequence.phraseBeats = steps;
        sequence.armBeat = 0;
        sequence.stopBeat = steps - 1;
        sequence.commandSample = sequence.marker(steps); // The next Step 1 also sends Arm.
        auto& plugin = *sequence.processor;
        auto settings = plugin.getLoopCapturePreference();
        settings.method = LoopCapturePreference::Method::endSignal;
        REQUIRE(plugin.setLoopCapturePreference(settings));
        sequence.through(sequence.marker(steps - 1), true);
        REQUIRE(plugin.loopFinishing());
        sequence.through(sequence.marker(steps), true);
        const auto take = plugin.getCaptureSnapshot();
        REQUIRE(take.manifest.loop && take.manifest.loop->beats == steps);
        REQUIRE(take.manifest.loop->originSample == sequence.marker(0));
        REQUIRE(take.manifest.loop->endSample == sequence.marker(steps) - sequence.marker(0));
        REQUIRE(!plugin.loopCaptureArmed());
    }

    LoopCaptureFixture fixture(512, 44100, 121.5);
    fixture.simpleClock = true;
    fixture.simplePhrase = true;
    fixture.commandNote = 12;
    fixture.commandSample = fixture.marker(2) + 17;
    auto& processor = *fixture.processor;
    auto preference = processor.getLoopCapturePreference();
    preference.method = LoopCapturePreference::Method::endSignal;
    REQUIRE(processor.setLoopCapturePreference(preference));
    REQUIRE(processor.armLoopCapture());
    fixture.through(fixture.marker(0));
    REQUIRE(processor.getWorkflowSnapshot().phase == workflow::Phase::recording);
    fixture.frames = 768;
    fixture.audio.setSize(2, fixture.frames);
    { AudioAllocationGuard guard; fixture.tick(); }
    REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
    fixture.through(fixture.marker(3), true);
    REQUIRE(processor.requestLoopStop(processor.getWorkflowSnapshot().pass));
    fixture.tick();
    REQUIRE(processor.loopFinishing());
    fixture.through(fixture.marker(4), true);
    const auto first = processor.getCaptureSnapshot();
    REQUIRE(first.sourceKind() == CaptureSourceKind::loop && first.manifest.loop);
    REQUIRE(first.manifest.loop->originSample == fixture.marker(0));
    REQUIRE(first.manifest.loop->endSample == fixture.marker(4) - fixture.marker(0));
    REQUIRE(first.manifest.loop->beats == 4);
    REQUIRE(!processor.armLoopCapture());

    fixture.armBeat = 5;
    fixture.stopBeat = 8;
    fixture.through(fixture.marker(9), true);
    REQUIRE(!processor.loopCaptureArmed() && processor.getCaptureSnapshot().reference == first.reference);

    REQUIRE(processor.setLoopRecapture(true));
    fixture.armBeat = 10;
    fixture.stopBeat = 13;
    fixture.through(fixture.marker(14), true);
    const auto replacement = processor.getCaptureSnapshot();
    REQUIRE(replacement.reference != first.reference && replacement.manifest.loop);
    REQUIRE(replacement.manifest.loop->originSample == fixture.marker(10));
    REQUIRE(replacement.manifest.loop->endSample == fixture.marker(14) - fixture.marker(10));

    fixture.armBeat = 15;
    fixture.stopBeat = 18;
    fixture.through(fixture.marker(18), true);
    REQUIRE(processor.loopFinishing());
    fixture.clockEnabled = false;
    fixture.through(fixture.marker(19) + 1024, true);
    REQUIRE(processor.getWorkflowSnapshot().result == workflow::Result::cancelled);
    REQUIRE(processor.getCaptureSnapshot().reference == replacement.reference);

    preference.method = LoopCapturePreference::Method::duration;
    REQUIRE(processor.setLoopCapturePreference(preference));
    fixture.clockEnabled = true;
    fixture.armBeat = 20;
    fixture.stopBeat = 24;
    fixture.through(fixture.marker(24), true);
    REQUIRE(processor.getWorkflowSnapshot().result == workflow::Result::cancelled);
    REQUIRE(processor.getCaptureSnapshot().reference == replacement.reference);

    preference.method = LoopCapturePreference::Method::endSignal;
    REQUIRE(processor.setLoopCapturePreference(preference));
    fixture.armBeat = 25;
    fixture.stopBeat = 29;
    fixture.through(fixture.marker(29), true);
    REQUIRE(processor.loopFinishing());
    REQUIRE(processor.getCaptureSnapshot().reference == replacement.reference);
    fixture.through(fixture.marker(33), true);
    const auto eightBeats = processor.getCaptureSnapshot();
    REQUIRE(eightBeats.manifest.loop->beats == 8);
    REQUIRE(eightBeats.manifest.loop->endSample == fixture.marker(33) - fixture.marker(25));
    juce::MemoryBlock state;
    processor.getStateInformation(state);
    auto reopened = std::make_unique<PreverbAudioProcessor>(fixture.root.file);
    reopened->setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    REQUIRE(reopened->flushCaptureForTesting());
    REQUIRE(reopened->getCaptureSnapshot().reference == eightBeats.reference);
    REQUIRE(!reopened->loopCaptureArmed() && !reopened->loopRecaptureEnabled());
    const auto replacementStart = fixture.nextBeat;
    fixture.armBeat = replacementStart;
    fixture.stopBeat = replacementStart + 3;
    fixture.through(fixture.marker(replacementStart + 3), true);
    REQUIRE(processor.getWorkflowSnapshot().phase == workflow::Phase::recording && processor.loopFinishing());
    processor.holdCaptureValidationForTesting(true);
    fixture.through(fixture.marker(replacementStart + 4) - 2 * fixture.frames, true);
    fixture.through(fixture.marker(replacementStart + 4));
    const auto restoreDeadline = juce::Time::getMillisecondCounterHiRes() + 5000;
    while (processor.getWorkflowSnapshot().phase != workflow::Phase::validating
        && juce::Time::getMillisecondCounterHiRes() < restoreDeadline)
        juce::Thread::sleep(2);
    REQUIRE(processor.getWorkflowSnapshot().phase == workflow::Phase::validating);
    juce::MemoryBlock settling;
    processor.getStateInformation(settling);
    processor.setStateInformation(settling.getData(), static_cast<int>(settling.getSize()));
    processor.holdCaptureValidationForTesting(false);
    REQUIRE(processor.flushCaptureForTesting());
    REQUIRE(processor.getCaptureSnapshot().reference == eightBeats.reference);
    REQUIRE(!processor.loopCaptureArmed() && !processor.loopRecaptureEnabled() && !processor.loopFinishing());
    REQUIRE(!processor.armLoopCapture());
    fixture.armBeat = fixture.nextBeat;
    fixture.through(fixture.marker(fixture.armBeat), true);
    REQUIRE(!processor.loopCaptureArmed() && processor.getCaptureSnapshot().reference == eightBeats.reference);

    LoopCaptureFixture shortTake(512, 48000);
    shortTake.simpleClock = true;
    auto& cold = *shortTake.processor;
    auto oneBeat = cold.getLoopCapturePreference();
    oneBeat.unit = LoopCapturePreference::Unit::beats;
    REQUIRE(cold.setLoopCapturePreference(oneBeat) && cold.armLoopCapture());
    shortTake.through(shortTake.marker(1), true);
    REQUIRE(cold.getCaptureSnapshot().sourceKind() == CaptureSourceKind::song);
    shortTake.through(shortTake.marker(2), true);
    REQUIRE(cold.getCaptureSnapshot().manifest.loop->endSample == shortTake.marker(1) - shortTake.marker(0));
    const auto acceptedShortTake = cold.getCaptureSnapshot().reference;
    REQUIRE(cold.setLoopRecapture(true));
    for (const bool midiStop : {false, true})
    {
        const int startBeat = (shortTake.nextBeat + 3) / 4 * 4;
        REQUIRE(cold.armLoopCapture());
        shortTake.through(shortTake.marker(startBeat), true);
        const auto pending = cold.getWorkflowSnapshot();
        REQUIRE(pending.phase == workflow::Phase::recording && cold.loopCaptureArmed());
        if (midiStop)
        {
            shortTake.commandNote = 25;
            shortTake.commandSample = shortTake.marker(startBeat) + 1024;
            shortTake.through(shortTake.commandSample, true);
        }
        else
        {
            REQUIRE(cold.requestLoopStop(pending.pass));
            REQUIRE(!cold.loopCaptureArmed() && cold.getWorkflowSnapshot().result==workflow::Result::cancelled);
            shortTake.tick();
        }
        REQUIRE(!cold.loopCaptureArmed());
        shortTake.through(shortTake.marker(startBeat + 2), true);
        REQUIRE(cold.getWorkflowSnapshot().result == workflow::Result::cancelled);
        REQUIRE(cold.getCaptureSnapshot().reference == acceptedShortTake);
    }

    LoopCaptureFixture between(512, 44100);
    between.simpleClock = true;
    auto& midBeat = *between.processor;
    REQUIRE(midBeat.setLoopCapturePreference(preference));
    between.commandSample = between.marker(0) + 1000;
    between.through(between.commandSample);
    REQUIRE(midBeat.getWorkflowSnapshot().phase == workflow::Phase::armed);
    REQUIRE(!midBeat.requestLoopStop(midBeat.getWorkflowSnapshot().pass + 1));
    between.stopBeat = 4;
    between.through(between.marker(5), true);
    const auto observed = midBeat.getCaptureSnapshot();
    REQUIRE(observed.manifest.loop && observed.manifest.loop->originSample == between.marker(1));
    REQUIRE(observed.manifest.loop->endSample == between.marker(5) - between.marker(1));

    REQUIRE(midBeat.setLoopRecapture(true));
    REQUIRE(midBeat.armLoopCapture());
    between.commandSample = between.cursor + 7;
    between.commandNote = 25;
    between.tick();
    REQUIRE(midBeat.getWorkflowSnapshot().result == workflow::Result::cancelled);
    REQUIRE(midBeat.getCaptureSnapshot().reference == observed.reference);

    between.armBeat = 6;
    between.stopBeat = 7;
    between.through(between.marker(7), true);
    REQUIRE(midBeat.loopFinishing());
    juce::Thread::sleep(550);
    between.tick();
    REQUIRE(midBeat.getWorkflowSnapshot().result == workflow::Result::cancelled);
    REQUIRE(midBeat.getCaptureSnapshot().reference == observed.reference);
}

static void rackDurationAlignmentCases()
{
    LoopCaptureFixture fixture;
    fixture.simpleClock = true;
    auto& plugin = *fixture.processor;
    REQUIRE(plugin.armLoopCapture());
    fixture.through(fixture.marker(0));
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    fixture.through(fixture.marker(4),true);
    const auto original = plugin.getCaptureSnapshot();
    REQUIRE(original.manifest.loop && original.manifest.loop->originSample == fixture.marker(0));

    REQUIRE(plugin.enterLoopStandby());
    fixture.through(fixture.marker(6));
    REQUIRE(plugin.requestWorkflowMode(workflow::Mode::capture));
    REQUIRE(plugin.setLoopRecapture(true) && plugin.armLoopCapture());
    fixture.through(fixture.marker(7));
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::armed);
    REQUIRE(plugin.getCaptureSnapshot().reference == original.reference);
    fixture.through(fixture.marker(8));
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    fixture.through(fixture.marker(12),true);
    const auto retake = plugin.getCaptureSnapshot();
    REQUIRE(retake.reference != original.reference && retake.manifest.loop);
    REQUIRE(retake.manifest.loop->endSample == fixture.marker(12) - fixture.marker(8));

    fixture.clockEnabled = false;
    fixture.through(fixture.marker(14) + 1024);
    REQUIRE(plugin.armLoopCapture());
    fixture.clockEnabled = true;
    fixture.through(fixture.marker(15));
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    fixture.through(fixture.marker(19),true);
    const auto restarted = plugin.getCaptureSnapshot();
    REQUIRE(restarted.reference != retake.reference && restarted.manifest.loop);
    REQUIRE(restarted.manifest.loop->endSample == fixture.marker(19) - fixture.marker(15));
}

static void rackRepeatCaptureCases()
{
    LoopCaptureFixture fixture;
    fixture.simpleClock = true;
    fixture.phraseBeats = 4;
    auto& plugin = *fixture.processor;
    auto preference = plugin.getLoopCapturePreference();
    preference.method = LoopCapturePreference::Method::endOnRepeat;
    REQUIRE(plugin.setLoopCapturePreference(preference) && plugin.armLoopCapture());
    fixture.through(fixture.marker(2));
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::armed);
    fixture.simplePhrase = true;
    fixture.through(fixture.marker(4));
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    fixture.commandNote = 12;
    fixture.commandSample = fixture.marker(7) + 1;
    fixture.through(fixture.commandSample,true);
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    fixture.through(fixture.marker(8),true);
    const auto accepted = plugin.getCaptureSnapshot();
    REQUIRE(accepted.manifest.loop && accepted.manifest.loop->beats == 4);
    REQUIRE(accepted.manifest.loop->originSample == fixture.marker(4));
    REQUIRE(accepted.manifest.loop->endSample == fixture.marker(8) - fixture.marker(4));

    REQUIRE(plugin.setLoopRecapture(true) && plugin.armLoopCapture());
    fixture.through(fixture.marker(12),true);
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    fixture.tick(false,true);
    REQUIRE(plugin.flushCaptureForTesting());
    REQUIRE(plugin.getWorkflowSnapshot().result == workflow::Result::cancelled);
    REQUIRE(plugin.getCaptureSnapshot().reference == accepted.reference);
}

static void rackLoopWithPartialHostTransport()
{
    using Phase = m3::LoopLiveReference::State;
    using Prep = m3::LoopPlaybackPublication::Preparation;
    using Playing = m3::LoopCueScheduler::State;
    LoopCaptureFixture fixture(128,44100);
    fixture.simpleClock = true;
    fixture.armBeat = 0;
    fixture.stopBeat = 6;
    auto& plugin = *fixture.processor;
    auto& engine = plugin.swellEngine();
    engine.setAutomaticProcessing(false);
    fixture.host.position.setIsPlaying(true);
    fixture.host.position.setTimeInSamples(0);
    plugin.setPlayHead(&fixture.host);
    auto preference = plugin.getLoopCapturePreference();
    preference.method = LoopCapturePreference::Method::endSignal;
    REQUIRE(plugin.setLoopCapturePreference(preference));
    fixture.through(fixture.marker(8),true);
    const auto accepted = plugin.getCaptureSnapshot();
    REQUIRE(accepted.manifest.loop && accepted.manifest.loop->beats == 8);
    REQUIRE(accepted.manifest.sampleRate == 44100);
    REQUIRE(plugin.enterLoopStandby());

    REQUIRE(engine.addTarget(512));
    const auto ir = fixture.root.file.getChildFile("rack-clock-ir.wav");
    writeTestWav(ir,1,44100,{.5f,0,0,0,.25f});
    engine.loadIR(ir);
    const auto waitFor = [&](const auto& predicate)
    {
        const auto deadline = juce::Time::getMillisecondCounter() + 8000;
        while (!predicate() && juce::Time::getMillisecondCounter() < deadline)
        {
            fixture.tick();
            juce::Thread::sleep(2);
        }
        REQUIRE(predicate());
    };
    waitFor([&]{return engine.savedSession().swells.front().irName == ir.getFileName();});
    waitFor([&]{return plugin.loopPlaybackStatus().preparation == Prep::ready;});
    REQUIRE(plugin.loopPlaybackStatus().phase == Phase::ready);
    REQUIRE(plugin.requestLoopCue());
    fixture.through(fixture.marker(fixture.nextBeat+8));
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating);
    REQUIRE(plugin.getCaptureSnapshot().reference == accepted.reference);
}

static void flStudioLoopProcessorCases()
{
    {
        constexpr int64_t beat=24000;
        m3::LoopClockGrid rack;
        rack.reset(48000);
        for (int i=0;i<=6;++i)
            rack.observe(i*beat,loop_capture::captureClock | (i==0 ? loop_capture::phrase : 0),48000,true);
        REQUIRE(rack.ready());
        REQUIRE(rack.nextDownbeat(6*beat,beat,16)==16*beat);
        REQUIRE(rack.nextDownbeat(6*beat,11*beat,16)==32*beat);
        rack.reset(48000);
        for (int i=0;i<=6;++i) rack.observe(i*beat,loop_capture::captureClock,48000,true);
        REQUIRE(rack.nextDownbeat(6*beat,beat,16)==8*beat);
    }
    using Phase = m3::LoopLiveReference::State;
    TestDirectory root;
    LoopCaptureFixture::TimingHost host;
    auto processor = std::make_unique<PreverbAudioProcessor>(root.file, true);
    auto& plugin = *processor;
    host.position.setBpm(120);
    host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4, 4});
    host.position.setIsPlaying(false);
    plugin.setPlayHead(&host);
    plugin.prepareToPlay(48000, 250);
    plugin.swellEngine().setAutomaticProcessing(false);
    auto settings = plugin.getLoopCapturePreference();
    settings.enabled = true;
    settings.count = 2;
    REQUIRE(plugin.setLoopCapturePreference(settings));

    constexpr int64_t quarter = 24000;
    constexpr int64_t bar = 4 * quarter;
    int64_t cursor = 0;
    juce::AudioBuffer<float> audio(2, 250);
    juce::MidiBuffer midi;
    midi.ensureSize(128);
    unsigned blocks = 0;
    const auto tick = [&](int64_t noteAt = -1, int note = 0)
    {
        constexpr int sizes[]{250, 104, 146};
        const int frames = sizes[blocks % 3];
        audio.setSize(2, frames, false, false, true);
        for (int channel = 0; channel < 2; ++channel)
            std::fill_n(audio.getWritePointer(channel), frames, .25f);
        host.position.setTimeInSamples(cursor);
        host.position.setPpqPosition(static_cast<double>(cursor) / quarter);
        midi.clear();
        if (noteAt >= cursor && noteAt < cursor + frames)
        {
            const int offset = static_cast<int>(noteAt - cursor);
            midi.addEvent(juce::MidiMessage::noteOn(1, note, juce::uint8{100}), offset);
            midi.addEvent(juce::MidiMessage::noteOn(1, note, juce::uint8{100}), offset);
        }
        { AudioAllocationGuard guard; plugin.processBlock(audio, midi); }
        REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
        cursor += frames;
        if (++blocks % 16 == 0) REQUIRE(plugin.flushCaptureForTesting());
    };
    const auto through = [&](int64_t sample, int64_t noteAt = -1, int note = 0)
    {
        while (cursor <= sample) tick(noteAt, note);
        REQUIRE(plugin.flushCaptureForTesting());
    };

    REQUIRE(plugin.armLoopCapture());
    tick();
    cursor = 0;
    plugin.reset(); // FL Studio resets the effect as transport starts.
    host.position.setIsPlaying(true);
    tick();
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    through(2 * bar);
    const auto duration = plugin.getCaptureSnapshot();
    REQUIRE(duration.manifest.loop && duration.manifest.loop->endSample == 2 * bar);
    REQUIRE(duration.manifest.loop->originSample == 0);
    REQUIRE(duration.manifest.loop->beats == 8 && duration.manifest.loop->tempoBpm == 120);
    REQUIRE(!plugin.requestLoopCue()); // Learn never accepts Perform commands.

    REQUIRE(plugin.setLoopRecapture(true));
    host.position.setIsPlaying(false);
    tick();
    REQUIRE(plugin.armLoopCapture());
    cursor = 0;
    host.position.setIsPlaying(true);
    tick();
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    cursor += 2 * bar;
    tick();
    REQUIRE(plugin.flushCaptureForTesting());
    REQUIRE(plugin.getWorkflowSnapshot().result == workflow::Result::cancelled);
    REQUIRE(plugin.getCaptureSnapshot().reference == duration.reference);

    host.position.setIsPlaying(false);
    tick();
    REQUIRE(plugin.armLoopCapture());
    cursor = 2 * quarter; // Bar 1 beat 3.
    host.position.setIsPlaying(true);
    through(bar - 500);
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::armed);
    through(bar);
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    through(3 * bar);
    const auto midBar = plugin.getCaptureSnapshot();
    REQUIRE(midBar.reference != duration.reference && midBar.manifest.loop);
    REQUIRE(midBar.manifest.loop->endSample == 2 * bar);

    // FL Studio Pattern 2 declares the one-bar cycle and splits its callback
    // at the endpoint, then reports sample and PPQ zero while still playing.
    host.position.setIsLooping(true);
    host.position.setLoopPoints(juce::AudioPlayHead::LoopPoints{0,4});
    const auto startPattern = [&]{
        host.position.setIsPlaying(false);
        tick();
        REQUIRE(plugin.setLoopCapturePreference(settings) && plugin.armLoopCapture());
        cursor = 0;
        host.position.setIsPlaying(true);
        tick();
        REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    };
    const auto wrapPattern = [&]{
        through(bar - 1);
        REQUIRE(cursor == bar);
        cursor = 0;
        through(0);
    };
    settings.count = 1;
    startPattern();
    wrapPattern();
    const auto oneBarPattern = plugin.getCaptureSnapshot();
    REQUIRE(oneBarPattern.reference != midBar.reference && oneBarPattern.manifest.loop);
    REQUIRE(oneBarPattern.manifest.loop->endSample == bar && oneBarPattern.manifest.loop->beats == 4);

    settings.count = 2;
    startPattern();
    wrapPattern();
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    wrapPattern();
    const auto twoBarPattern = plugin.getCaptureSnapshot();
    REQUIRE(twoBarPattern.reference != oneBarPattern.reference && twoBarPattern.manifest.loop);
    REQUIRE(twoBarPattern.manifest.loop->endSample == 2 * bar && twoBarPattern.manifest.loop->beats == 8);

    startPattern();
    through(quarter);
    cursor = 0; // A backward seek short of the declared endpoint is still a discontinuity.
    through(0);
    REQUIRE(plugin.getWorkflowSnapshot().result == workflow::Result::cancelled);
    REQUIRE(plugin.getCaptureSnapshot().reference == twoBarPattern.reference);

    settings.method = LoopCapturePreference::Method::endSignal;
    startPattern();
    through(2 * quarter, 2 * quarter, 25);
    REQUIRE(plugin.loopFinishing());
    wrapPattern();
    const auto endedPattern = plugin.getCaptureSnapshot();
    REQUIRE(endedPattern.reference != twoBarPattern.reference && endedPattern.manifest.loop);
    REQUIRE(endedPattern.manifest.loop->endSample == bar && endedPattern.manifest.loop->beats == 4);

    settings.method = LoopCapturePreference::Method::endOnRepeat;
    startPattern();
    wrapPattern();
    const auto repeated = plugin.getCaptureSnapshot();
    REQUIRE(repeated.reference != endedPattern.reference && repeated.manifest.loop);
    REQUIRE(repeated.manifest.loop->endSample == bar && repeated.manifest.loop->beats == 4);
    REQUIRE(plugin.armLoopCapture());
    through(quarter);
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::armed);
    wrapPattern();
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    wrapPattern();
    const auto nextRepeat = plugin.getCaptureSnapshot();
    REQUIRE(nextRepeat.reference != repeated.reference && nextRepeat.manifest.loop);
    REQUIRE(nextRepeat.manifest.loop->endSample == bar && nextRepeat.manifest.loop->beats == 4);

    startPattern();
    host.position.setLoopPoints(juce::AudioPlayHead::LoopPoints{0,8});
    tick();
    REQUIRE(plugin.flushCaptureForTesting());
    REQUIRE(plugin.getWorkflowSnapshot().result == workflow::Result::cancelled);
    REQUIRE(plugin.getCaptureSnapshot().reference == nextRepeat.reference);
    host.position.setIsLooping(false);

    settings.method = LoopCapturePreference::Method::endSignal;
    REQUIRE(plugin.setLoopCapturePreference(settings));
    host.position.setIsPlaying(false);
    tick();
    cursor = 18 * bar;
    host.position.setIsPlaying(true);
    through(18 * bar, 18 * bar, 24);
    REQUIRE(plugin.getWorkflowSnapshot().phase == workflow::Phase::recording);
    through(18 * bar + quarter, 18 * bar + quarter, 17);
    REQUIRE(!plugin.loopFinishing());
    through(18 * bar + 2 * quarter, 18 * bar + 2 * quarter, 25);
    REQUIRE(plugin.loopFinishing());
    through(19 * bar);
    const auto ended = plugin.getCaptureSnapshot();
    REQUIRE(ended.reference != midBar.reference && ended.manifest.loop);
    REQUIRE(ended.manifest.loop->endSample == bar && ended.manifest.loop->beats == 4);

    host.position.setIsPlaying(false);
    tick();
    REQUIRE(plugin.armLoopCapture());
    plugin.reset();
    host.position.setIsPlaying(true);
    through(20 * bar + quarter);
    REQUIRE(plugin.requestLoopStop(plugin.getWorkflowSnapshot().pass));
    tick();
    REQUIRE(plugin.loopFinishing());
    host.position.setIsPlaying(false);
    tick();
    REQUIRE(plugin.flushCaptureForTesting());
    REQUIRE(plugin.getWorkflowSnapshot().result == workflow::Result::cancelled);
    REQUIRE(plugin.getCaptureSnapshot().reference == ended.reference);

    auto& engine = plugin.swellEngine();
    host.position.setIsPlaying(true);
    REQUIRE(plugin.enterLoopStandby());
    REQUIRE(engine.addTarget(quarter / 2));
    const auto ir = root.file.getChildFile("FL Studio Loop IR.wav");
    writeTestWav(ir, 1, 48000, {.5f, 0, 0, .25f});
    engine.loadIR(ir);
    const auto waitFor = [&](const auto& predicate)
    {
        const auto deadline = juce::Time::getMillisecondCounter() + 8000;
        while (!predicate() && juce::Time::getMillisecondCounter() < deadline)
        {
            tick();
            juce::Thread::sleep(2);
        }
        REQUIRE(predicate());
    };
    waitFor([&]{return engine.savedSession().swells.front().irName == ir.getFileName();});
    waitFor([&]{return plugin.loopPlaybackStatus().preparation == m3::LoopPlaybackPublication::Preparation::ready;});
    using Playing = m3::LoopCueScheduler::State;
    REQUIRE(plugin.loopPlaybackStatus().phase == Phase::ready);
    host.position.setIsPlaying(false);
    tick(cursor,16);
    REQUIRE(!plugin.requestLoopCue());
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::standby);
    host.position.setIsPlaying(true);
    cursor = 24 * bar + 100;
    tick();
    const auto cue = cursor + 23;
    through(cue, cue, 16);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::queued);
    const auto firstBar = ((cursor + bar - 1) / bar) * bar;
    through(firstBar);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating);
    const auto stop = firstBar + quarter;
    through(stop, stop, 25);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating);
    through(stop + quarter, stop + quarter, 17);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::finishing);
    through(firstBar + bar + 1000);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::standby);

    const auto marker = firstBar + bar + quarter + 57;
    through(marker, marker, 12);
    const auto laterCue = marker + quarter;
    through(laterCue, laterCue, 16);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::queued);
    through(marker + bar);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating);
    REQUIRE(plugin.loopPlaybackStatus().playhead && *plugin.loopPlaybackStatus().playhead < 512);
    const auto stopNow = marker + bar + 1000;
    through(stopNow, stopNow, 18);
    through(stopNow + 1000);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::standby);

    REQUIRE(plugin.requestLoopCue());
    tick();
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::queued);
    cursor += 3 * bar;
    through(cursor + 1000);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::standby && plugin.requestLoopCue());

    // Pattern playback keeps playing at the declared boundary, even though
    // FL Studio resets the absolute transport sample and PPQ to zero.
    host.position.setIsPlaying(false);
    tick();
    host.position.setIsLooping(true);
    host.position.setLoopPoints(juce::AudioPlayHead::LoopPoints{0,4});
    host.position.setIsPlaying(true);
    cursor = 0;
    through(quarter);
    using Route=m3::LoopPlayback::Route;
    REQUIRE(plugin.loopPlaybackStatus().route == Route::hostRepeat && !plugin.requestLoopCue());
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating);
    wrapPattern();
    REQUIRE(plugin.loopPlaybackStatus().playback != Playing::standby);
    wrapPattern();
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating);
    wrapPattern();
    const auto patternPlayhead = plugin.loopPlaybackStatus().playhead;
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating && patternPlayhead && *patternPlayhead < 250);

    through(quarter);
    cursor = 0; // Same destination as a repeat, but the previous block did not reach the endpoint.
    through(1000);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating);
    REQUIRE(plugin.loopPlaybackStatus().playhead && *plugin.loopPlaybackStatus().playhead < 1300);
    const auto stopRepeat=cursor+10;
    through(stopRepeat,stopRepeat,18);
    through(stopRepeat+1000);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::standby);
    wrapPattern();
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::standby);

    host.position.setIsPlaying(false);
    tick();
    host.position.setLoopPoints(juce::AudioPlayHead::LoopPoints{0,8});
    host.position.setIsPlaying(true);
    cursor=0;
    through(quarter);
    REQUIRE(plugin.loopPlaybackStatus().route == Route::manual && plugin.loopPlaybackStatus().playback == Playing::standby);

    const auto target=plugin.loopPlacementTarget();
    host.position.setIsPlaying(false);
    cursor=2*bar;
    tick();
    REQUIRE(plugin.setLoopPlacementFromPlayhead(target));
    REQUIRE((plugin.getLoopPlacement()==m3::LoopPlacement{8.0}));
    cursor=3*bar;
    tick();
    REQUIRE(plugin.setLoopPlacementFromPlayhead(target));
    REQUIRE((plugin.getLoopPlacement()==m3::LoopPlacement{12.0}));
    host.position.setLoopPoints(juce::AudioPlayHead::LoopPoints{0,4});
    host.position.setIsPlaying(true);
    cursor=0;
    tick();
    REQUIRE(plugin.loopPlaybackStatus().route == Route::placement && !plugin.requestLoopCue());
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::queued);
    plugin.requestLoopPlaybackStop();
    tick();
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::standby);

    host.position.setIsPlaying(false);
    tick();
    host.position.setIsLooping(false);
    host.position.setIsPlaying(true);
    cursor=3*bar-quarter;
    through(3*bar+1000);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating);
    REQUIRE(plugin.loopPlaybackStatus().playhead && *plugin.loopPlaybackStatus().playhead < 1300);
    cursor=4*bar+quarter;
    through(cursor+1000);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating);
    through(6*bar);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::repeating);
    plugin.requestLoopPlaybackStop();
    tick();
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::finishing);
    through(8*bar);
    REQUIRE(plugin.loopPlaybackStatus().playback == Playing::standby);

    juce::MemoryBlock state;
    plugin.getStateInformation(state);
    auto restored=std::make_unique<PreverbAudioProcessor>(root.file,true);
    REQUIRE(!restored->getLoopPlacement());
    restored->setStateInformation(state.getData(),static_cast<int>(state.getSize()));
    REQUIRE(restored->flushCaptureForTesting());
    REQUIRE((restored->getLoopPlacement() == m3::LoopPlacement{12.0}));
    REQUIRE(plugin.setLoopPlacement(target,{}));
    REQUIRE(!plugin.getLoopPlacement() && restored->getLoopPlacement());
}

static void loopPlaybackProcessorCases()
{
    using Phase = m3::LoopLiveReference::State;
    using Prep = m3::LoopPlaybackPublication::Preparation;
    using Playing = m3::LoopCueScheduler::State;
    LoopCaptureFixture fixture;
    fixture.simpleClock = true;
    auto& p = *fixture.processor;
    p.setLoopPlaybackReference({{48000,137},4,4,4,true},true);
    auto& engine = p.swellEngine();
    engine.setAutomaticProcessing(false);
    REQUIRE(p.armLoopCapture());
    fixture.through(fixture.marker(4),true);
    const auto accepted = p.getCaptureSnapshot();
    REQUIRE(accepted.manifest.loop && accepted.manifest.loop->endSample == 84088);
    REQUIRE(p.enterLoopStandby());
    REQUIRE(!p.requestLoopCue());
    auto pref = p.getLoopCapturePreference(); pref.enabled = false;
    REQUIRE(p.setLoopCapturePreference(pref));

    const auto until = [&](const char* purpose, const auto& predicate){
        const auto deadline = juce::Time::getMillisecondCounter() + 8000;
        while (!predicate() && juce::Time::getMillisecondCounter() < deadline)
        {
            fixture.midi.clear();fixture.audio.clear();p.processBlock(fixture.audio,fixture.midi);
            juce::Thread::sleep(2);
        }
        if (!predicate())
        {
            const auto snapshot = engine.snapshot();
            throw std::runtime_error(std::string(purpose) + ": " + snapshot.message.toStdString()
                + (snapshot.session.swells.empty() ? "" : " / " + snapshot.session.swells.front().status.toStdString())
                + " / preparation " + std::to_string(static_cast<int>(p.loopPlaybackStatus().preparation)));
        }
    };
    REQUIRE(engine.addTarget(512));
    const auto swell = engine.savedSession().primary;
    const auto ir = fixture.root.file.getChildFile("loop-fixture.wav");
    writeTestWav(ir,1,48000,{.5f,0,0,0,.25f});
    engine.loadIR(ir);
    until("Loop IR import",[&]{return engine.savedSession().swells.front().irName == "loop-fixture.wav";});
    until("Loop wet preparation",[&]{return p.loopPlaybackStatus().preparation == Prep::ready;});
    REQUIRE(p.loopPlaybackStatus().phase != Phase::ready && !p.requestLoopCue());
    REQUIRE(p.requestWorkflowMode(workflow::Mode::capture));

    int64_t cursor = 0;
    int beat = 0;
    const auto marker = [](int n){return 37 + std::llround(48000.L * 60 * n / 137);};
    const auto note = [&](int value,int offset){fixture.midi.addEvent(juce::MidiMessage::noteOn(1,value,juce::uint8{100}),offset);};
    fixture.midi.ensureSize(2048);
    const auto through = [&](int64_t sample, int64_t command=-1, int commandNote=0){
        bool heardWet = false;
        while (cursor <= sample)
        {
            fixture.midi.clear();
            if (marker(beat) < cursor + fixture.frames)
            {
                const auto offset = static_cast<int>(marker(beat)-cursor);
                note(21,offset);
                if (beat % 4 == 0) { note(19,offset); note(12,offset); }
                ++beat;
            }
            if (command >= cursor && command < cursor + fixture.frames)
                note(commandNote,static_cast<int>(command-cursor));
            for (int channel = 0; channel < 2; ++channel)
                std::fill_n(fixture.audio.getWritePointer(channel),fixture.frames,.125f);
            {AudioAllocationGuard guard;p.processBlock(fixture.audio,fixture.midi);}
            REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
            for (int n = 0; n < fixture.frames; ++n) heardWet |= fixture.audio.getSample(0,n) != .125f;
            cursor += fixture.frames;
        }
        return heardWet;
    };
    REQUIRE(!through(marker(4)));
    REQUIRE(p.loopPlaybackStatus().phase == Phase::waiting && !p.requestLoopCue());
    REQUIRE(!through(marker(8),marker(8),16)); // External Cue in Learn cannot queue wet.
    REQUIRE(p.loopPlaybackStatus().phase == Phase::ready && !p.requestLoopCue());
    REQUIRE(p.loopPlaybackStatus().playback == Playing::standby && !p.loopPlaybackStatus().playhead);
    REQUIRE(p.enterLoopStandby());
    REQUIRE(p.loopPlaybackStatus().phase == Phase::ready && p.requestLoopCue());
    REQUIRE(through(marker(12)+5000));
    REQUIRE(p.loopPlaybackStatus().playback == Playing::repeating);
    through(marker(13)+1000,marker(13)+512,25);
    REQUIRE(p.loopPlaybackStatus().playback == Playing::repeating);
    through(marker(14)+1000,marker(14)+512,17);
    REQUIRE(p.loopPlaybackStatus().playback == Playing::finishing);
    through(marker(14)+2000,marker(14)+1512,18);
    REQUIRE(p.loopPlaybackStatus().playback == Playing::standby);
    REQUIRE(!through(marker(15)));

    REQUIRE(p.requestLoopCue());
    REQUIRE(through(marker(16)+5000));
    const auto replacement = fixture.root.file.getChildFile("loop-fixture-replacement.wav");
    writeTestWav(replacement,1,48000,{.25f,0,0,0,.5f});
    engine.holdRenderPublicationForTesting(true);
    engine.loadIR(replacement);
    const auto waitOnClock = [&](const auto& predicate){
        const auto deadline = juce::Time::getMillisecondCounter() + 8000;
        while (!predicate() && juce::Time::getMillisecondCounter() < deadline)
        {
            through(cursor);
            juce::Thread::sleep(2);
        }
        REQUIRE(predicate());
    };
    waitOnClock([&]{return engine.renderPublicationWaitingForTesting();});
    REQUIRE(p.loopPlaybackStatus().playback == Playing::repeating);
    engine.holdRenderPublicationForTesting(false);
    waitOnClock([&]{return engine.savedSession().swells.front().irName == replacement.getFileName()
        && p.loopPlaybackStatus().preparation == Prep::ready;});
    REQUIRE(p.loopPlaybackStatus().playback == Playing::repeating && p.loopPlaybackStatus().playhead);
    REQUIRE(engine.edit(swell,[](auto& w,auto&){w.placement += 128;return true;}));
    through(cursor);
    REQUIRE(p.loopPlaybackStatus().playback == Playing::repeating);
    waitOnClock([&]{return p.loopPlaybackStatus().preparation == Prep::ready;});
    REQUIRE(p.loopPlaybackStatus().playback == Playing::repeating);
    engine.holdRenderPublicationForTesting(true);
    REQUIRE(engine.edit(swell,[](auto& w,auto&){w.sourceEnd -= 128;return true;}));
    waitOnClock([&]{return engine.renderPublicationWaitingForTesting()
        && p.loopPlaybackStatus().preparation == Prep::waiting;});
    REQUIRE(p.loopPlaybackStatus().playback == Playing::repeating);
    engine.holdRenderPublicationForTesting(false);
    waitOnClock([&]{return p.loopPlaybackStatus().preparation == Prep::ready;});
    REQUIRE(p.loopPlaybackStatus().playback == Playing::repeating);
    p.requestLoopPlaybackStop();
    through(cursor);
    REQUIRE(p.loopPlaybackStatus().playback == Playing::finishing);
    p.requestLoopPlaybackStop();
    through(cursor+512);
    REQUIRE(p.loopPlaybackStatus().playback == Playing::standby);
    REQUIRE(p.requestLoopCue());
    through(cursor);
    REQUIRE(p.loopPlaybackStatus().playback == Playing::queued);
    juce::MemoryBlock queuedState;
    p.getStateInformation(queuedState);
    auto restoredQueue = std::make_unique<PreverbAudioProcessor>(fixture.root.file);
    restoredQueue->setStateInformation(queuedState.getData(), static_cast<int>(queuedState.getSize()));
    REQUIRE(restoredQueue->flushCaptureForTesting());
    REQUIRE(restoredQueue->getWorkflowSnapshot().mode == workflow::Mode::live);
    REQUIRE(restoredQueue->loopPlaybackStatus().playback == Playing::standby && !restoredQueue->requestLoopCue());
    REQUIRE(p.loopPlaybackStatus().playback == Playing::queued);
    const auto closeEditor = [&p](juce::AudioProcessorEditor* editor){
        p.editorBeingDeleted(editor);
        delete editor;
    };
    for (int open = 0; open < 2; ++open)
    {
        const std::unique_ptr<juce::AudioProcessorEditor, decltype(closeEditor)> editor(
            p.createEditorAndMakeActive(), closeEditor);
        REQUIRE(editor && p.getCaptureSnapshot().reference == accepted.reference);
        REQUIRE(p.loopPlaybackStatus().playback == Playing::queued);
    }
    REQUIRE(engine.edit(swell,[](auto& w,auto&){w.placement += 128;return true;}));
    through(cursor);
    REQUIRE(p.loopPlaybackStatus().playback == Playing::standby);

    REQUIRE(engine.addTarget(24000));
    const auto other = engine.savedSession().primary;
    REQUIRE(engine.edit(other,[](auto& w,auto&){w.muted=true;w.solo=true;return true;}));
    until("Loop solo exclusion",[&]{return p.loopPlaybackStatus().preparation == Prep::noWet;});
    REQUIRE(engine.edit(other,[](auto& w,auto&){w.solo=false;return true;}));
    until("Loop solo release",[&]{return p.loopPlaybackStatus().preparation == Prep::ready;});
    REQUIRE(engine.edit(swell,[](auto& w,auto&){w.muted=true;return true;}));
    until("Loop mute",[&]{return p.loopPlaybackStatus().preparation == Prep::noWet;});
    REQUIRE(!p.requestLoopCue());
    REQUIRE(p.requestWorkflowMode(workflow::Mode::capture));
    REQUIRE(p.getCaptureSnapshot().reference == accepted.reference);
    REQUIRE(!p.requestLoopCue());
    juce::MemoryBlock state;
    p.getStateInformation(state);
    auto restored = std::make_unique<PreverbAudioProcessor>(fixture.root.file);
    restored->setStateInformation(state.getData(),static_cast<int>(state.getSize()));
    REQUIRE(restored->flushCaptureForTesting() && restored->getCaptureSnapshot().reference == accepted.reference);
    REQUIRE(restored->enterLoopStandby() && !restored->requestLoopCue());
    REQUIRE(restored->loopPlaybackStatus().phase == Phase::unconfigured);
    const auto restoredSwells = restored->swellEngine().savedSession().swells;
    const auto savedSwells = engine.savedSession().swells;
    REQUIRE(restoredSwells.size() == savedSwells.size());
    REQUIRE(restoredSwells.front().id == savedSwells.front().id
        && restoredSwells.front().placement == savedSwells.front().placement
        && restoredSwells.front().ir == savedSwells.front().ir
        && restoredSwells.front().render == savedSwells.front().render);
    p.processBlockBypassed(fixture.audio, fixture.midi);
    REQUIRE(p.loopPlaybackStatus().phase == Phase::unconfigured && !p.requestLoopCue());
    p.releaseResources();
    p.prepareToPlay(48000, fixture.frames);
    REQUIRE(p.getCaptureSnapshot().reference == accepted.reference && !p.requestLoopCue());
}

int main(int argc,char* argv[]){return runTest([&]{
    if(argc==2 && std::string_view(argv[1])=="--rack-loop-transport"){
        juce::ScopedJuceInitialiser_GUI gui;rackLoopWithPartialHostTransport();return;}
    if(argc==2 && std::string_view(argv[1])=="--fl-studio-loop"){
        juce::ScopedJuceInitialiser_GUI gui;flStudioLoopProcessorCases();return;}
    if(argc>1 && juce::String(argv[1])=="--loop-playback"){
        juce::ScopedJuceInitialiser_GUI gui;loopPlaybackProcessorCases();return;}
    if(argc==2 && juce::String(argv[1])=="--shared-engine"){juce::ScopedJuceInitialiser_GUI gui;commonEngineCases();return;}
    if(argc>1 && juce::String(argv[1])=="--test-reactivation"){
        juce::ScopedJuceInitialiser_GUI gui;processorReactivationCases();return;
    }
    {
        CaptureProgress progress;CaptureManifest committed;
        committed.ranges.push_back({16,16,CaptureCoverageKind::silence});
        std::array<float,128> input;input.fill(.25f);
        progress.add(0,64,input.data(),false,committed);
        REQUIRE(progress.ranges.size()==2 && progress.ranges[0].range.frameCount==16 && progress.ranges[1].range.startSample==32);
        input.fill(.75f);progress.add(0,64,input.data(),false,committed);
        REQUIRE(progress.ranges[0].peaks.front().maximum==.25f); // First received wins; protected silence stays protected.
        progress.add(8,32,input.data(),true,committed);
        REQUIRE(progress.ranges.size()==1 && progress.ranges[0].range.frameCount==64);
        REQUIRE(std::any_of(progress.ranges[0].peaks.begin(),progress.ranges[0].peaks.end(),[](const auto& p){return p.startSample==8 && p.maximum==.75f;}));
        REQUIRE(std::none_of(progress.ranges[0].peaks.begin(),progress.ranges[0].peaks.end(),[](const auto& p){return p.startSample<8 && p.startSample+p.frameCount>8;}));
        REQUIRE(progress.bytes()<4*1024*1024);
        CaptureProgress sparse;for(size_t i=0;i<CaptureProgress::maxRanges;++i){CaptureOverviewRange r;r.range.startSample=static_cast<int64_t>(i*2);r.range.frameCount=1;sparse.ranges.push_back(r);}
        sparse.add(10000,1,input.data(),false,{});
        REQUIRE(sparse.limited && sparse.ranges.empty()); // Telemetry loss cannot invent coverage.
        CaptureProgress dense;CaptureOverviewRange full;full.range={0,65536*512,CaptureCoverageKind::audio};full.peaks.resize(CaptureProgress::maxPeaks);
        dense.ranges.push_back(std::move(full));REQUIRE(dense.bytes()<4*1024*1024);
        dense.add(40000000,1,input.data(),false,{});REQUIRE(dense.limited && dense.ranges.empty());
        std::cerr<<"Progress metadata ceiling: "<<sizeof(CaptureProgress)+CaptureProgress::maxRanges*sizeof(CaptureOverviewRange)+CaptureProgress::maxPeaks*sizeof(CapturePeak)<<" bytes (no retained PCM)\n";
    }
    juce::ScopedJuceInitialiser_GUI gui;loopMidiCases();loopPhraseClockCases();loopBeatClockCases();loopTakeCases();loopStateCases();loopProcessorCases();simpleLoopProcessorCases();rackDurationAlignmentCases();rackRepeatCaptureCases();rackLoopWithPartialHostTransport();flStudioLoopProcessorCases();loopPlaybackProcessorCases();processorReactivationCases();processorCaptureFailureCases();TestDirectory root;SavedStateListener listener;
    auto p=std::make_unique<PreverbAudioProcessor>(root.file);
    {
        REQUIRE(!p->swellEngine().savedSession().autoGenerateOnStop);
        juce::MemoryBlock fresh;p->getStateInformation(fresh);
        const auto xml=juce::AudioProcessor::getXmlFromBinary(fresh.getData(),static_cast<int>(fresh.getSize()));
        REQUIRE(xml && xml->getIntAttribute("stateVersion")==8 && xml->getStringAttribute("workflowMode")=="Capture");
        auto copy=std::make_unique<PreverbAudioProcessor>(root.file);
        for(int variant=0;variant<6;++variant)
        {
            auto fixture=std::make_unique<juce::XmlElement>(*xml);
            auto* child=fixture->getChildByName("SwellSession");auto json=juce::JSON::parse(child->getAllSubText());
            auto* object=json.getDynamicObject();object->setProperty("automaticProcessing",false);
            if(variant<2)object->setProperty("autoGenerateOnStop",variant==1);
            if(variant==2){object->setProperty("version",10);object->removeProperty("autoGenerateOnStop");}
            if(variant==3){fixture->setAttribute("stateVersion",2);fixture->deleteAllChildElements();}
            else {if(variant==4)object->removeProperty("autoGenerateOnStop");
                if(variant==5)object->setProperty("autoGenerateOnStop",1);
                child->deleteAllChildElements();child->addTextElement(juce::JSON::toString(json));}
            const auto prior=copy->swellEngine().savedSession();juce::MemoryBlock bytes;
            juce::AudioProcessor::copyXmlToBinary(*fixture,bytes);
            copy->setStateInformation(bytes.getData(),static_cast<int>(bytes.getSize()));REQUIRE(copy->flushCaptureForTesting());
            const auto restored=copy->swellEngine().savedSession();
            REQUIRE(restored.autoGenerateOnStop==(variant>=4?prior.autoGenerateOnStop:variant!=0));
            REQUIRE(copy->getWorkflowSnapshot().mode==workflow::Mode::capture && !copy->getWorkflowSnapshot().recapture);
            juce::MemoryBlock saved;copy->getStateInformation(saved);
            const auto result=juce::AudioProcessor::getXmlFromBinary(saved.getData(),static_cast<int>(saved.getSize()));
            REQUIRE(result && result->getIntAttribute("stateVersion")==8);
            REQUIRE(juce::JSON::parse(result->getChildByName("SwellSession")->getAllSubText())["autoGenerateOnStop"].isBool());
        }
    }
    WorkflowHost host;p->prepareToPlay(1000,64);p->setPlayHead(&host);juce::AudioBuffer<float> block(2,64);juce::MidiBuffer midi;
    double playingOrigin=0;
    const auto process=[&](int64_t at,bool playing,float value){host.position.setIsPlaying(playing);host.position.setTimeInSamples(at);
        host.position.setPpqPosition((at-(playing?playingOrigin:0))/500.0);host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
        for(int c=0;c<2;++c)std::fill_n(block.getWritePointer(c),block.getNumSamples(),value);
        if(p->isNonRealtime())p->processBlock(block,midi);
        else {{AudioAllocationGuard guard;p->processBlock(block,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);}};
    auto& e=p->swellEngine();e.setAutomaticProcessing(false);
    const auto defaultDeadline=juce::Time::getMillisecondCounter()+10000;
    while(!e.savedSession().defaultIR.present() && juce::Time::getMillisecondCounter()<defaultDeadline)juce::Thread::sleep(2);
    REQUIRE(e.savedSession().defaultIR.present());p->flushHostStateChangesForTesting();p->addListener(&listener);
    e.setLearningAutomatic(true);p->flushHostStateChangesForTesting();REQUIRE(listener.changes==1);
    e.setLearningAutomatic(false);p->flushHostStateChangesForTesting();REQUIRE(listener.changes==2);
    e.setLearningAutomatic(false);p->flushHostStateChangesForTesting();REQUIRE(listener.changes==2 && !listener.wrongThread);
    listener.changes=0;
    REQUIRE(p->requestWorkflowMode(workflow::Mode::capture));
    REQUIRE(p->setRecaptureExisting(true) && p->setRecaptureExisting(false));p->flushHostStateChangesForTesting();
    REQUIRE(listener.changes==0); // Same mode and temporary arm are not saved edits.
    REQUIRE(p->getName()=="ReverbArise" && p->getParameters().isEmpty());
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::capture);
    REQUIRE(!e.addTarget(0) && !e.audition());process(0,true,.125f);
    e.setLearningAutomatic(true);p->flushHostStateChangesForTesting();REQUIRE(!e.savedSession().automaticProcessing && listener.changes==0);
    REQUIRE(!p->requestWorkflowMode(workflow::Mode::live));REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getCaptureSnapshot().manifest.ranges.empty());
    const auto progressDeadline=juce::Time::getMillisecondCounter()+2000;
    while(p->getCaptureProgress().ranges.empty() && juce::Time::getMillisecondCounter()<progressDeadline)juce::Thread::sleep(2);
    const auto recording=p->getCaptureProgress();
    REQUIRE(recording.pass==p->getWorkflowSnapshot().pass && recording.ranges.size()==1);
    REQUIRE(recording.ranges[0].range.startSample==0 && recording.ranges[0].range.frameCount==64 && recording.ranges[0].peaks[0].maximum==.125f);
    REQUIRE(p->getCaptureSnapshot().manifest.ranges.empty()); // Before storage batching; save/readers remain committed-only.
    juce::MemoryBlock midPass;p->getStateInformation(midPass);
    process(64,false,0);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::capture && p->getWorkflowReadiness().usefulSource);
    p->flushHostStateChangesForTesting();REQUIRE(listener.changes==1 && !listener.wrongThread);
    auto accepted=p->getCaptureSnapshot();REQUIRE(!accepted.manifest.ranges.empty());
    REQUIRE(accepted.manifest.schemaVersion==2 && captureContextAt(accepted.manifest,0));
    REQUIRE(!captureContextAt(accepted.manifest,0)->automatic && captureContextAt(accepted.manifest,0)->ir==e.savedSession().defaultIR);
    REQUIRE(p->getCaptureProgress().ranges.empty());
    REQUIRE(p->requestWorkflowMode(workflow::Mode::live));
    juce::MemoryBlock liveBytes;p->getStateInformation(liveBytes);
    auto liveRestore=std::make_unique<PreverbAudioProcessor>(root.file);liveRestore->setStateInformation(liveBytes.getData(),static_cast<int>(liveBytes.getSize()));
    REQUIRE(liveRestore->flushCaptureForTesting() && liveRestore->getWorkflowSnapshot().mode==workflow::Mode::live);liveRestore.reset();
    REQUIRE(block.getMagnitude(0,64)==0);process(0,true,.5f);process(64,false,0);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getCaptureSnapshot().reference==accepted.reference); // Live never captures.
    const auto beforeMode=listener.changes;
    REQUIRE(p->requestWorkflowMode(workflow::Mode::capture));p->flushHostStateChangesForTesting();REQUIRE(listener.changes==beforeMode+1);
    REQUIRE(p->setRecaptureExisting(true) && p->setRecaptureExisting(false));p->flushHostStateChangesForTesting();REQUIRE(listener.changes==beforeMode+1);
    process(0,true,.5f);process(64,false,0);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::capture && p->getCaptureSnapshot().reference==accepted.reference);
    REQUIRE(p->setRecaptureExisting(true));process(0,true,.5f);process(64,false,0);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::capture && !p->getWorkflowSnapshot().recapture);
    REQUIRE(p->requestWorkflowMode(workflow::Mode::live));
    REQUIRE(p->getCaptureSnapshot().reference!=accepted.reference);
    const auto replaced=p->getCaptureSnapshot();
    REQUIRE(e.addTarget(0));const auto id=e.savedSession().primary;
    REQUIRE(e.edit(id,[](auto& w,auto&){w.sourceStart=0;w.sourceEnd=64;w.shape.endOffset=64;w.shape.fadeOut=0;return true;}));
    const auto file=root.file.getChildFile("unit.wav");writeTestWav(file,1,1000,{1,0,0,0});e.loadIR(file);
    const auto until=juce::Time::getMillisecondCounter()+10000;
    while(juce::Time::getMillisecondCounter()<until && e.snapshot().irReplacementPending)juce::Thread::sleep(5);
    e.generate();while(juce::Time::getMillisecondCounter()<until && e.savedSession().swells[0].phase!=m3::Phase::ready)juce::Thread::sleep(5);
    REQUIRE(e.savedSession().swells[0].phase==m3::Phase::ready);
    const auto compatibleRender=e.savedSession().swells[0].render;
    REQUIRE(p->getWorkflowReadiness().readySwell);
    REQUIRE(e.edit(id,[](auto& w,auto&){w.render.bpm=121;return true;}));
    REQUIRE(p->getWorkflowReadiness().usefulSource && p->getWorkflowReadiness().compatible);
    REQUIRE(!p->getWorkflowReadiness().readySwell && !p->requestWorkflowMode(workflow::Mode::finalise));
    REQUIRE(e.edit(id,[&](auto& w,auto&){w.render=compatibleRender;return true;}));
    e.generate();const auto compatibleDeadline=juce::Time::getMillisecondCounter()+10000;
    while(juce::Time::getMillisecondCounter()<compatibleDeadline && e.savedSession().swells[0].phase!=m3::Phase::ready)juce::Thread::sleep(5);
    REQUIRE(p->getWorkflowReadiness().readySwell);
    {
        // This clock/preview identity oracle has only 64 captured dry samples.
        // Pin Wet only explicitly; #46's default mixed audition correctly needs
        // dry coverage for the whole listening window, independent of live readiness.
        const auto initialMonitoring=e.savedSession();
        REQUIRE(e.setAuditionWetOnly(true,e.assignmentEpoch()));
        process(0,false,0);const auto readyDeadline=juce::Time::getMillisecondCounter()+10000;
        while((!e.snapshot().liveReady || !e.snapshot().auditionReady) && juce::Time::getMillisecondCounter()<readyDeadline)juce::Thread::sleep(2);
        REQUIRE(e.snapshot().liveReady && e.snapshot().auditionReady);
        REQUIRE(e.audition());process(0,false,0);juce::AudioBuffer<float> previewBefore(block);
        e.stop();process(64,false,0);process(0,false,0);
        juce::String beforeClock,afterClock,clockError;REQUIRE(m3::encode(e.savedSession(),beforeClock,clockError));
        const auto sourceBefore=p->getCaptureSnapshot().reference;
        playingOrigin=16;process(0,true,.125f);
        REQUIRE(e.snapshot().liveActive && !e.snapshot().liveIncompatible && block.getMagnitude(0,64)>.125f);
        REQUIRE(std::abs(e.snapshot().liveObservation->originDelta-16)<.00001);
        REQUIRE(p->getCaptureSnapshot().reference==sourceBefore);
        process(64,false,0);playingOrigin=0;
        REQUIRE(e.audition());process(0,false,0);
        for(int channel=0;channel<2;++channel)for(int n=0;n<64;++n)
            REQUIRE(std::abs(block.getSample(channel,n)-previewBefore.getSample(channel,n))<1e-6f);
        e.stop();process(64,false,0);
        REQUIRE(m3::encode(e.savedSession(),afterClock,clockError) && beforeClock==afterClock);
        REQUIRE(p->getCaptureSnapshot().reference==sourceBefore);
        REQUIRE(e.setAuditionWetOnly(initialMonitoring.auditionMode==m3::AuditionMode::wetOnly,e.assignmentEpoch()));
    }
    p->flushHostStateChangesForTesting();const auto beforeSettings=listener.changes;
    REQUIRE(e.edit(id,[](auto& w,auto&){w.shape.gainDb=3;return true;}));
    e.setPostRoll(1);e.setIsolateDry(true);e.setIsolateSelected(true);e.setMonitoring(m3::AuditionMode::dryWet,.7f);
    e.setPreferences(-3,.25f,.1,false);e.setAutoMarkers(false);e.setRegenerationSensitivity(.2f);
    p->flushHostStateChangesForTesting();REQUIRE(listener.changes==beforeSettings+1); // Coalesces continuous/grouped edits.
    const auto beforeNoop=listener.changes;
    REQUIRE(e.edit(id,[](auto&,auto&){return true;}));e.setPostRoll(1);e.setIsolateDry(true);e.setIsolateSelected(true);
    e.setMonitoring(m3::AuditionMode::dryWet,.7f);e.setPreferences(-3,.25f,.1,false);e.setAutoMarkers(false);e.setRegenerationSensitivity(.2f);
    REQUIRE(e.selectTargets({},{}));REQUIRE(e.selectTargets({id},id));e.stop();
    e.setPostRoll(6);e.setLivePlayback(true,m3::LiveRouting::dryWet,100);
    REQUIRE(!e.edit(id,[](auto&,auto&){return false;}));
    p->flushHostStateChangesForTesting();REQUIRE(listener.changes==beforeNoop && !listener.wrongThread);
    e.setLivePlayback(true,m3::LiveRouting::wetOnly,0);
    REQUIRE(p->requestWorkflowMode(workflow::Mode::finalise));
    REQUIRE(e.savedSession().exportState.initialized && e.savedSession().exportState.dry==0 && e.savedSession().exportState.wet==1);
    REQUIRE(e.setExportMix(0,e.assignmentEpoch()) && e.savedSession().wetBlend==.7f);
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::finalise);
    REQUIRE(!e.setExportMix(.8f,e.assignmentEpoch()+1));
    REQUIRE(e.setExportMix(.65f,e.assignmentEpoch()));

    {
        const auto deadline=juce::Time::getMillisecondCounter()+10000;
        while(e.snapshot().songExport.phase!=m3::ExportPhase::ready && juce::Time::getMillisecondCounter()<deadline)juce::Thread::sleep(2);
        const auto prepared=e.snapshot().songExport.prepared;REQUIRE(prepared && prepared->metrics.audibleStart && prepared->metrics.audibleEnd);
        const auto start=*prepared->metrics.audibleStart,total=*prepared->metrics.audibleEnd-start;const auto quantum=static_cast<int>(std::min(int64_t{16},total/2));REQUIRE(quantum>0);block.setSize(2,quantum);
        juce::WavAudioFormat format;auto stream=e.assetDirectory().getChildFile(prepared->file.path).createInputStream();REQUIRE(stream);
        std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(stream.release(),true));juce::AudioBuffer<float> expected(2,quantum*2);
        REQUIRE(reader && reader->read(&expected,0,quantum*2,start-prepared->file.origin,true,true));
        REQUIRE(e.playExport(e.assignmentEpoch()));
        while(!e.snapshot().songExport.cursor && juce::Time::getMillisecondCounter()<deadline){process(12000,false,.25f);juce::Thread::sleep(2);}
        REQUIRE(e.snapshot().songExport.cursor==start+quantum);
        for(int c=0;c<2;++c)for(int n=0;n<quantum;++n)REQUIRE(std::abs(block.getSample(c,n)-(.25f+expected.getSample(c,n)*m3::monitorEdgeGain(n,static_cast<size_t>(total),1000,.005)))<1e-6);
        playingOrigin=16;process(32000,true,.25f);playingOrigin=0;
        for(int c=0;c<2;++c)for(int n=0;n<quantum;++n)REQUIRE(std::abs(block.getSample(c,n)-(.25f+expected.getSample(c,n+quantum)*m3::monitorEdgeGain(n+quantum,static_cast<size_t>(total),1000,.005)))<1e-6);
        REQUIRE(p->getCaptureSnapshot().reference==replaced.reference);e.stop();process(0,false,.25f);block.setSize(2,64);
    }
    REQUIRE(!e.edit(id,[](auto& w,auto&){w.shape.gainDb=6;return true;}));
    process(0,true,.25f);process(64,true,.25f); // Let the <=5 ms transition settle.
    for(int c=0;c<2;++c)for(int n=0;n<64;++n)REQUIRE(block.getSample(c,n)==.25f);
    p->setNonRealtime(true);process(0,true,.375f);
    for(int c=0;c<2;++c)for(int n=0;n<64;++n)REQUIRE(block.getSample(c,n)==.375f);
    REQUIRE(p->getCaptureSnapshot().reference==replaced.reference);
    juce::MemoryBlock state;p->getStateInformation(state);
    {
        // Two inserts in one bounce retain independent mode snapshots.
        auto finalised=std::make_unique<PreverbAudioProcessor>(root.file);
        finalised->prepareToPlay(1000,64);finalised->setPlayHead(&host);
        finalised->setStateInformation(state.getData(),static_cast<int>(state.getSize()));
        REQUIRE(finalised->flushCaptureForTesting());
        p->setNonRealtime(false);process(0,false,0);
        REQUIRE(p->requestWorkflowMode(workflow::Mode::live));
        p->setNonRealtime(true);process(0,true,.375f);
        bool containsWet=false;for(int n=0;n<64;++n)containsWet|=std::abs(block.getSample(0,n))>1e-8f && std::abs(block.getSample(0,n)-.375f)>1e-6f;
        REQUIRE(containsWet);
        finalised->setNonRealtime(true);juce::AudioBuffer<float> dry(2,64);
        for(int c=0;c<2;++c)std::fill_n(dry.getWritePointer(c),64,.375f);
        const auto finaliseSource=finalised->getCaptureSnapshot().reference;
        finalised->processBlock(dry,midi);
        for(int c=0;c<2;++c)for(int n=0;n<64;++n)REQUIRE(dry.getSample(c,n)==.375f);
        REQUIRE(finalised->getCaptureSnapshot().reference==finaliseSource);
    }
    auto restored=std::make_unique<PreverbAudioProcessor>(root.file);restored->prepareToPlay(1000,64);
    restored->setStateInformation(state.getData(),static_cast<int>(state.getSize()));REQUIRE(restored->flushCaptureForTesting());
    REQUIRE(restored->getWorkflowSnapshot().mode==workflow::Mode::finalise && !restored->getWorkflowSnapshot().recapture);
    REQUIRE(restored->swellEngine().savedSession().exportState.wet==.65f && restored->swellEngine().savedSession().wetBlend==.7f);
    // Display labels never replace the established XML workflow identifiers.
    const auto durableFixture=juce::AudioProcessor::getXmlFromBinary(state.getData(),static_cast<int>(state.getSize()));
    REQUIRE(durableFixture && durableFixture->getStringAttribute("workflowMode")=="Finalise");
    const auto savedVersion=durableFixture->getIntAttribute("stateVersion");
    for(const auto mode:{workflow::Mode::capture,workflow::Mode::live,workflow::Mode::finalise})
    {
        auto fixture=std::make_unique<juce::XmlElement>(*durableFixture);
        fixture->setAttribute("workflowMode",workflow::modeName(mode));juce::MemoryBlock encoded;
        juce::AudioProcessor::copyXmlToBinary(*fixture,encoded);
        restored->setStateInformation(encoded.getData(),static_cast<int>(encoded.getSize()));REQUIRE(restored->flushCaptureForTesting());
        REQUIRE(restored->getWorkflowSnapshot().mode==mode && !restored->getWorkflowSnapshot().recapture);
        REQUIRE(restored->getCaptureSnapshot().reference==replaced.reference && restored->getParameters().isEmpty());
        REQUIRE(juce::String(m3::ui::presentWorkflow(restored->getWorkflowSnapshot(),restored->getWorkflowReadiness()).choices[static_cast<size_t>(mode)].label)
            ==(mode==workflow::Mode::capture?"Learn":mode==workflow::Mode::live?"Perform":"Export"));
        juce::MemoryBlock roundTrip;restored->getStateInformation(roundTrip);
        const auto saved=juce::AudioProcessor::getXmlFromBinary(roundTrip.getData(),static_cast<int>(roundTrip.getSize()));
        REQUIRE(saved && saved->getStringAttribute("workflowMode")==workflow::modeName(mode) && saved->getIntAttribute("stateVersion")==savedVersion);
    }
    auto legacy=juce::AudioProcessor::getXmlFromBinary(state.getData(),static_cast<int>(state.getSize()));
    REQUIRE(legacy);legacy->setAttribute("stateVersion",7);legacy->removeAttribute("workflowMode");juce::MemoryBlock old;
    juce::AudioProcessor::copyXmlToBinary(*legacy,old);restored->setStateInformation(old.getData(),static_cast<int>(old.getSize()));
    REQUIRE(restored->flushCaptureForTesting() && restored->getWorkflowSnapshot().mode==workflow::Mode::capture);
    REQUIRE(juce::String(m3::ui::modeLabel(restored->getWorkflowSnapshot().mode))=="Learn");
    REQUIRE(restored->getCaptureSnapshot().reference==replaced.reference);
    for(int version=1;version<=7;++version)
    {legacy->setAttribute("stateVersion",version);juce::MemoryBlock fixture;juce::AudioProcessor::copyXmlToBinary(*legacy,fixture);
        restored->setStateInformation(fixture.getData(),static_cast<int>(fixture.getSize()));REQUIRE(restored->flushCaptureForTesting());
        REQUIRE(restored->getWorkflowSnapshot().mode==workflow::Mode::capture && !restored->getWorkflowSnapshot().recapture);}
    restored->setStateInformation(midPass.getData(),static_cast<int>(midPass.getSize()));REQUIRE(restored->flushCaptureForTesting());
    REQUIRE(restored->getWorkflowSnapshot().mode==workflow::Mode::capture && restored->getCaptureSnapshot().manifest.ranges.empty());
    // Bad mode metadata cannot replace a valid session.
    auto invalid=juce::AudioProcessor::getXmlFromBinary(state.getData(),static_cast<int>(state.getSize()));
    invalid->setAttribute("workflowMode","Unknown");juce::MemoryBlock invalidBytes;juce::AudioProcessor::copyXmlToBinary(*invalid,invalidBytes);
    restored->setStateInformation(invalidBytes.getData(),static_cast<int>(invalidBytes.getSize()));
    REQUIRE(restored->getWorkflowSnapshot().mode==workflow::Mode::capture && restored->getCaptureSnapshot().manifest.ranges.empty());
    p->setNonRealtime(false);process(64,false,0);REQUIRE(p->requestWorkflowMode(workflow::Mode::capture));
    p->holdCaptureValidationForTesting(true);process(200,true,.25f);process(264,false,0);
    const auto deadline=juce::Time::getMillisecondCounter()+2000;
    while(p->getWorkflowSnapshot().phase!=workflow::Phase::validating && juce::Time::getMillisecondCounter()<deadline)juce::Thread::sleep(1);
    REQUIRE(p->getWorkflowSnapshot().phase==workflow::Phase::validating && !p->requestWorkflowMode(workflow::Mode::live));
    // Restore supersedes the validating pass; late completion cannot change it.
    p->setStateInformation(old.getData(),static_cast<int>(old.getSize()));p->holdCaptureValidationForTesting(false);
    REQUIRE(p->getCaptureProgress().ranges.empty()); // Restore immediately fences the obsolete pass overlay.
    REQUIRE(p->flushCaptureForTesting() && p->getWorkflowSnapshot().mode==workflow::Mode::capture && p->getCaptureSnapshot().reference==replaced.reference);
    // Saturated capture queue aborts the whole pass, retaining accepted source.
    p->holdCaptureWorkerForTesting(true);juce::Thread::sleep(5);process(0,false,0);
    for(int i=0;i<40;++i)process(1000+i*64,true,.2f);process(3560,false,0);
    p->holdCaptureWorkerForTesting(false);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getCaptureProgress().ranges.empty()); // Failed pass overlay cannot survive settlement.
    REQUIRE(p->getDroppedCaptureBlockCount()>0 && p->getWorkflowSnapshot().phase==workflow::Phase::failed && p->getCaptureSnapshot().reference==replaced.reference);
    p->holdCaptureValidationForTesting(true);process(4000,false,0);process(4000,true,.1f);process(4064,false,0);
    const auto validationDeadline=juce::Time::getMillisecondCounter()+2000;
    while(p->getWorkflowSnapshot().phase!=workflow::Phase::validating && juce::Time::getMillisecondCounter()<validationDeadline)juce::Thread::sleep(1);
    REQUIRE(p->getWorkflowSnapshot().phase==workflow::Phase::validating);
    p->setNonRealtime(true);process(0,false,.375f);p->holdCaptureValidationForTesting(false);REQUIRE(p->flushCaptureForTesting());
    REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::capture);process(32,false,.375f);
    REQUIRE(block.getSample(0,63)==.375f); // A Capture-entered bounce cannot acquire wet mid-export.
    {
        auto cancelled=std::make_unique<PreverbAudioProcessor>(root.file.getChildFile("cancel-progress"));WorkflowHost cancelHost;
        cancelled->prepareToPlay(1000,64);cancelled->setPlayHead(&cancelHost);cancelled->swellEngine().setLearningAutomatic(false);
        cancelHost.position.setBpm(120);cancelHost.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
        cancelHost.position.setTimeInSamples(0);cancelHost.position.setPpqPosition(0);cancelHost.position.setIsPlaying(true);
        juce::AudioBuffer<float> input(2,64);for(int c=0;c<2;++c)std::fill_n(input.getWritePointer(c),64,.25f);
        {AudioAllocationGuard guard;cancelled->processBlock(input,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        const auto until=juce::Time::getMillisecondCounter()+2000;
        while(cancelled->getCaptureProgress().ranges.empty() && juce::Time::getMillisecondCounter()<until)juce::Thread::sleep(2);
        REQUIRE(!cancelled->getCaptureProgress().ranges.empty());cancelled->cancelCapturePass();
        REQUIRE(cancelled->getCaptureProgress().ranges.empty());REQUIRE(cancelled->flushCaptureForTesting());
        REQUIRE(cancelled->getCaptureSnapshot().manifest.ranges.empty());
    }
    {
        auto quiet=std::make_unique<PreverbAudioProcessor>(root.file.getChildFile("quiet"));WorkflowHost quietHost;
        quiet->prepareToPlay(1000,64);quiet->setPlayHead(&quietHost);juce::AudioBuffer<float> silent(2,64);silent.clear();
        quietHost.position.setTimeInSamples(0);quietHost.position.setIsPlaying(true);quiet->processBlock(silent,midi);
        REQUIRE(quiet->getWorkflowSnapshot().phase==workflow::Phase::idle); // Missing musical timing is not fabricated.
        quietHost.position.setIsPlaying(false);quiet->processBlock(silent,midi);
        quietHost.position.setBpm(120);quietHost.position.setPpqPosition(0);quietHost.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
        quietHost.position.setIsPlaying(true);quiet->processBlock(silent,midi);quietHost.position.setIsPlaying(false);quiet->processBlock(silent,midi);
        REQUIRE(quiet->flushCaptureForTesting() && quiet->getWorkflowSnapshot().mode==workflow::Mode::capture);
        REQUIRE(!quiet->requestWorkflowMode(workflow::Mode::live) && quiet->getCaptureSnapshot().manifest.ranges.front().kind==CaptureCoverageKind::silence);
    }
});}
