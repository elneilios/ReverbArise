#include "Probe.h"
#include <cstdlib>
#include <iostream>
#include <source_location>
#include <thread>
static thread_local bool watch=false;static thread_local int allocations=0,deallocations=0;
void* operator new(std::size_t n){if(watch)++allocations;if(auto* p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept {if(watch)++deallocations;std::free(p);}void operator delete(void* p,std::size_t) noexcept {if(watch)++deallocations;std::free(p);}
static void require(bool b,const std::source_location at=std::source_location::current()){if(!b)throw std::runtime_error("assertion failed line "+std::to_string(at.line()));}
struct Head:juce::AudioPlayHead {bool play=true;juce::Optional<PositionInfo> getPosition() const override {PositionInfo p;p.setIsPlaying(play);p.setTimeInSamples(100);return p;}};
int main(int argc,char** argv){try{juce::ScopedJuceInitialiser_GUI init;auto p=std::make_unique<hostprobe::Probe>();p->prepareToPlay(48000,512);
    auto native=p->evidence.getSiblingFile("native-test-"+p->instance+".csv");require(p->evidence.moveFileTo(native));p->evidence=native;
    if(argc==3 && std::string(argv[1])=="--snapshot"){std::unique_ptr<juce::AudioProcessorEditor> editor(p->createEditor());auto out=juce::File{juce::String(argv[2])}.createOutputStream();require(out && juce::PNGImageFormat{}.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds()),*out));return 0;}
    Head h;p->setPlayHead(&h);juce::AudioBuffer<float> b(2,512);juce::MidiBuffer m;
    b.clear();for(int i=0;i<512;++i){b.setSample(0,i,.125f);b.setSample(1,i,-.25f);}
    watch=true;p->processBlock(b,m);p->reset();p->processorLayoutsChanged();p->numChannelsChanged();p->setNonRealtime(true);watch=false;
    require(b.getSample(0,511)==.125f && b.getSample(1,0)==-.25f);p->drain();require(p->latest.playing && p->latest.sample==100 && p->latest.input==.25f);
    p->tone=true;b.clear();watch=true;p->processBlock(b,m);watch=false;require(b.getMagnitude(0,512)>.015f);
    b.clear();b.applyGain(1.f);watch=true;p->processBlockBypassed(b,m);watch=false;require(b.getMagnitude(0,512)==0 && p->counts[hostprobe::bypass]==1);
    h.play=false;b.clear();watch=true;p->processBlock(b,m);watch=false;require(b.getMagnitude(0,512)==0);
    juce::MemoryBlock state;p->getStateInformation(state);p->setStateInformation(state.getData(),static_cast<int>(state.getSize()));require(!p->tone && state.getSize()==1);
    p->setMarker(3);p->releaseResources();p->drain();require(p->counts[hostprobe::reset]==1 && p->counts[hostprobe::marker]==1 && p->counts[hostprobe::restore]==1);
    require(allocations==0 && deallocations==0);require(p->queue.dropped==0);p->setPlayHead(nullptr);
    auto q=std::make_unique<hostprobe::Queue>();hostprobe::Event e;for(int i=0;i<9000;++i)q->push(e);require(q->dropped==809);int read=0;while(q->pop(e))++read;require(read==8191);
    // Deliberate contention drops immediately rather than spinning or blocking.
    q->busy.test_and_set();q->push(e);q->busy.clear();require(q->dropped==810);
    auto concurrent=std::make_unique<hostprobe::Queue>();std::array<std::thread,3> writers;
    for(auto& t:writers)t=std::thread([&]{for(int i=0;i<1000;++i)concurrent->push({});});for(auto& t:writers)t.join();read=0;while(concurrent->pop(e))++read;require(read+concurrent->dropped==3000);
    std::cout<<"PASS native dry/bypass/tone/transport/lifecycle/transient state; bounded queue overflow/contention/concurrent accounting; watched callback allocations=0 deallocations=0\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
