#include "Probe.h"
#include <iostream>
#include <cstdlib>
#include <source_location>
static thread_local bool watch=false;static thread_local int allocations=0,deallocations=0;
void* operator new(std::size_t n){if(watch)++allocations;if(auto* p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept {if(watch)++deallocations;std::free(p);} void operator delete(void* p,std::size_t) noexcept {if(watch)++deallocations;std::free(p);}
static void require(bool v,const std::source_location at=std::source_location::current()){if(!v)throw std::runtime_error("native callback assertion failed at line "+std::to_string(at.line()));}
int main(int argc,char** argv){try {
    juce::ScopedJuceInitialiser_GUI initialise;
    auto p=std::make_unique<probe::Receiver>();p->prepareToPlay(48000,512);
    const auto nativeLog=p->evidence.getSiblingFile("native-test-"+p->instance+".csv");
    require(p->evidence.moveFileTo(nativeLog));p->evidence=nativeLog;
    if(argc==3 && std::string(argv[1])=="--snapshot"){
        std::unique_ptr<juce::AudioProcessorEditor> editor(p->createEditor());
        auto image=editor->createComponentSnapshot(editor->getLocalBounds());
        juce::File target{juce::String(argv[2])};auto output=target.createOutputStream();
        require(output!=nullptr && juce::PNGImageFormat{}.writeImageToStream(image,*output));return 0;
    }
    juce::AudioBuffer<float> b(2,512);juce::MidiBuffer midi;
    const auto now=probe::qpcSeconds();
    auto send=[&](uint32_t seq,int mode,double at){probe::setRaw(p->query,probe::normalized(probe::clockWord(at)));probe::setRaw(p->written,probe::normalized(probe::clockWord(at)));probe::setRaw(p->context,probe::normalized((seq<<2)|mode));};
    b.clear();watch=true;p->processBlock(b,midi);watch=false;require(probe::word(p->ack->get())==0);
    send(1,2,now);b.clear();watch=true;p->processAt(b,now+.001,true,false);watch=false;
    require(probe::word(p->ack->get())==6 && b.getMagnitude(0,512)==0);
    p->toneEnabled=true;b.clear();watch=true;p->processAt(b,now+.002,true,false);watch=false;require(b.getMagnitude(0,512)>.06f);
    send(2,1,now+.003);b.clear();watch=true;p->processAt(b,now+.004,true,false);watch=false;
    require(b.getMagnitude(0,240,272)==0 && probe::word(p->ack->get())==9);
    send(3,2,now+.005);b.clear();p->processAt(b,now+.006,true,true);require(b.getMagnitude(0,512)>.06f); // No unconditional offline mute.
    b.clear();p->processAt(b,now+1,true,true);require(b.getMagnitude(0,240,272)==0); // Actual lease expiry model in native path.
    p->toneEnabled=false;for(int c=0;c<2;++c)for(int i=0;i<512;++i)b.setSample(c,i,.125f);
    watch=true;p->processAt(b,now+1.1,true,false);watch=false;require(b.getSample(0,511)==.125f && allocations==0);
    juce::MemoryBlock state;p->getStateInformation(state);p->setStateInformation(state.getData(),static_cast<int>(state.getSize()));b.clear();p->processAt(b,now+2,false,false);
    require(state.getSize()==1 && probe::word(p->ack->get())==0);require(allocations==0 && deallocations==0);
    std::cout<<"PASS native parameter consumption, ack, dry, tone, Pattern ramp, offline fresh/expiry, transient state; watched callback allocations=0 deallocations=0\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
