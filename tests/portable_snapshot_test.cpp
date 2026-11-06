#include "PortableFixture.h"
#include <Plugin/PluginProcessor.h>
#include <Storage/PortableAssets.h>
#include <thread>
#include "WorkflowTestSupport.h"
using namespace m3::storage::portable;
struct StorageHost:juce::AudioPlayHead
{PositionInfo position;juce::Optional<PositionInfo> getPosition()const override{return position;}};
static void load(PreverbAudioProcessor& processor,Fixture& fixture)
{const auto xml=juce::XmlDocument::parse(fixture.xml);REQUIRE(xml);juce::MemoryBlock bytes;juce::AudioProcessor::copyXmlToBinary(*xml,bytes);processor.setStateInformation(bytes.getData(),static_cast<int>(bytes.getSize()));REQUIRE(processor.flushCaptureForTesting());}
int main(){juce::ScopedJuceInitialiser_GUI gui;return runTest([]{
    Fixture a,b;auto owner=std::make_unique<PreverbAudioProcessor>(a.directory.file);auto& processor=*owner;
    // Both independent namespaces live under the injected user-data fixture root.
    REQUIRE(b.root.copyDirectoryTo(a.directory.file.getChildFile(b.observed.project).getChildFile(b.observed.instance)));
    load(processor,a);Input pinned;juce::String error;
    const auto waitPin=[&]{const auto end=juce::Time::getMillisecondCounterHiRes()+10000;while(!processor.pinPortableAssets(pinned,error)){REQUIRE(juce::Time::getMillisecondCounterHiRes()<end);juce::Thread::sleep(2);}};
    waitPin();std::shared_ptr<Prepared> prepared;REQUIRE(prepare(pinned,prepared,error));const auto original=snapshot(*prepared);REQUIRE(original.observed.project==a.observed.project && original.observed.reference==a.observed.reference);
    juce::String saved;REQUIRE(encode(original.observed.session,saved,error));REQUIRE(original.observed.session.swells[0].frozen && original.observed.session.swells[0].generationSource==a.session.swells[0].generationSource);
    const auto before=processor.swellEngine().snapshot().renderCount;const auto capture=processor.getCaptureSnapshot();
    for(int i=0;i<20;++i){REQUIRE(processor.pinPortableAssets(pinned,error));ObservedSnapshot check;REQUIRE(readLegacySnapshot(pinned.processorXml,pinned.manifestJson,check,error));REQUIRE(check.reference==capture.reference && check.project==a.observed.project);}
    REQUIRE(processor.swellEngine().snapshot().renderCount==before && processor.getCaptureSnapshot().reference==capture.reference);
    // Change accepted sessions while another control reader takes snapshots. Each
    // success must be wholly A or wholly B; failed attempts preserve prior output.
    std::atomic<bool> bad=false;std::jthread reader([&](std::stop_token token){while(!token.stop_requested()){Input input;juce::String why;if(processor.pinPortableAssets(input,why)){ObservedSnapshot observed;if(!readLegacySnapshot(input.processorXml,input.manifestJson,observed,why)
        || !((observed.project==a.observed.project && observed.reference==a.observed.reference) || (observed.project==b.observed.project && observed.reference==b.observed.reference)))bad=true;}}});
    for(int i=0;i<8;++i){load(processor,i%2?a:b);juce::Thread::sleep(2);}reader.request_stop();reader.join();REQUIRE(!bad.load());
    waitPin();ObservedSnapshot last;REQUIRE(readLegacySnapshot(pinned.processorXml,pinned.manifestJson,last,error));REQUIRE(last.project==a.observed.project);

    // Pin dependency closure while actual capture/import/render workers publish.
    StorageHost host;host.position.setTimeInSamples(128);host.position.setPpqPosition(128.0/24000);host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});host.position.setIsPlaying(false);
    processor.setPlayHead(&host);processor.prepareToPlay(48000,512);juce::AudioBuffer<float> block(2,512);block.clear();juce::MidiBuffer midi;processor.processBlock(block,midi);
    std::atomic<bool> closureBad=false;std::atomic<int> closures=0;std::jthread auditor([&](std::stop_token token){while(!token.stop_requested()){Input input;juce::String why;if(processor.pinPortableAssets(input,why)){std::shared_ptr<Prepared> check;if(!prepare(input,check,why)){std::cerr<<why<<'\n';closureBad=true;}else ++closures;}juce::Thread::sleep(1);}});
    for(int channel=0;channel<2;++channel)for(int frame=0;frame<512;++frame)block.setSample(channel,frame,.125f);
    host.position.setIsPlaying(true);processor.processBlock(block,midi);finishCaptureFixture(processor,host.position);
    const auto end=juce::Time::getMillisecondCounterHiRes()+10000;
    for(int i=0;i<4;++i){REQUIRE(processor.swellEngine().assignDefaultIR(a.root.getChildFile(a.irs[i].path),processor.swellEngine().assignmentEpoch(),processor.getWorkflowSnapshot().pass));
        while(processor.swellEngine().snapshot().defaultIRPending){REQUIRE(juce::Time::getMillisecondCounterHiRes()<end);juce::Thread::sleep(2);}}
    REQUIRE(processor.swellEngine().setFrozen(a.session.swells[0].id,false));REQUIRE(processor.swellEngine().edit(a.session.swells[0].id,[](auto& swell,auto&){swell.shape.endOffset=2;swell.supportOffset=0;return true;}));const auto renders=processor.swellEngine().snapshot().renderCount;processor.swellEngine().generate(true);
    while(processor.swellEngine().snapshot().renderCount==renders){if(juce::Time::getMillisecondCounterHiRes()>=end)throw std::runtime_error(processor.swellEngine().snapshot().session.swells[0].status.toStdString());juce::Thread::sleep(2);}
    auditor.request_stop();auditor.join();REQUIRE(!closureBad.load() && closures.load()>0);processor.setPlayHead(nullptr);
    REQUIRE(processor.getCaptureSnapshot().reference!=a.observed.reference);

    std::cout<<"Portable accepted state: stopped-host restore, exact references/edits, unchanged render/source concurrent session replacement and actual capture/import/render publication passed.\n";
});}
