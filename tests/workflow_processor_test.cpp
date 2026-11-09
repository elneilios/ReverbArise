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
    const auto read = [&]
    {
        { AudioAllocationGuard guard; readLoopObservations(midi, 128, observations); }
        REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
    };
    const auto on = [&](int note, int offset, int channel = 1)
    { midi.addEvent(juce::MidiMessage::noteOn(channel, note, static_cast<juce::uint8>(100)), offset); };
    on(19, 0); on(21, 0); on(12, 0); on(22, 0);
    on(16, 17); on(17, 17);
    on(21, 22, 2);
    midi.addEvent(juce::MidiMessage::noteOff(1, 14), 127);
    read();
    REQUIRE(observations.error == MidiObservations::Error::none && observations.size == 3);
    REQUIRE(observations.samples[0].sampleOffset == 0 && observations.samples[0].roles == (bar | beat | phrase | reset));
    REQUIRE(observations.samples[1].sampleOffset == 17 && observations.samples[1].roles == (cue | stop));
    REQUIRE(observations.samples[2].sampleOffset == 127 && observations.samples[2].roles == gateLow);

    midi.clear();
    on(23, 3); on(24, 3); on(25, 39); on(17, 39);
    midi.addEvent(juce::MidiMessage::noteOff(1, 25), 50);
    read();
    REQUIRE(observations.size == 2);
    REQUIRE(observations.samples[0].sampleOffset == 3 && observations.samples[0].roles == (captureClock | captureArm));
    REQUIRE(observations.samples[1].sampleOffset == 39 && observations.samples[1].roles == (captureStop | stop));

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
        REQUIRE(!processor.requestWorkflowMode(workflow::Mode::live));
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
        REQUIRE(reopened->getCaptureSnapshot().reference==accepted.reference && reopened->getLoopCapturePreference()==preference);
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
    LoopCaptureFixture fixture(512, 44100, 121.5);
    fixture.simpleClock = true;
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

int main(int argc,char* argv[]){return runTest([&]{
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
    juce::ScopedJuceInitialiser_GUI gui;loopMidiCases();loopPhraseClockCases();loopBeatClockCases();loopTakeCases();loopStateCases();loopProcessorCases();simpleLoopProcessorCases();processorReactivationCases();processorCaptureFailureCases();TestDirectory root;SavedStateListener listener;
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
    while(juce::Time::getMillisecondCounter()<until && e.savedSession().swells[0].status=="Importing IR")juce::Thread::sleep(5);
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
