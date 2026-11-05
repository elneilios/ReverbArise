#include "Probe.h"
#define NOMINMAX
#include <windows.h>
#include <sstream>
#include <iomanip>

namespace hostprobe {
double now() noexcept {static const double frequency=[] {LARGE_INTEGER f;QueryPerformanceFrequency(&f);return double(f.QuadPart);}();LARGE_INTEGER n;QueryPerformanceCounter(&n);return n.QuadPart/frequency;}
Probe::Probe():AudioProcessor(BusesProperties().withInput("Source",juce::AudioChannelSet::stereo(),true).withOutput("Dry plus diagnostic tone",juce::AudioChannelSet::stereo(),true)) {
    now();auto dir=juce::File(PROBE_EVIDENCE_DIR);
    if(dir.createDirectory().wasOk()){
        evidence=dir.getChildFile("host-"+instance+".csv");
        logStatus=evidence.replaceWithText("event,qpc_seconds,block,marker,frames,rate,host_sample,position,playing,looping,offline,input_channels,output_channels,input_enabled,output_enabled,tone_enabled,input_peak,output_peak,dropped_total_at_flush\n")?"Logging":"WRITE FAILED";
    }else logStatus="DIRECTORY FAILED";
    startTimer(100);
}
Probe::~Probe(){stopTimer();drain();}
Event Probe::event(Kind k) noexcept {
    Event e;e.kind=k;e.time=now();e.block=blocks.load(std::memory_order_relaxed);e.mark=mark.load(std::memory_order_relaxed);e.rate=rate.load(std::memory_order_relaxed);e.tone=tone.load(std::memory_order_relaxed);return e;
}
void Probe::record(Kind k) noexcept {counts[k].fetch_add(1,std::memory_order_relaxed);queue.push(event(k));}
void Probe::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){render(b,false);}
void Probe::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer& m){AudioProcessor::processBlockBypassed(b,m);render(b,true);}
void Probe::render(juce::AudioBuffer<float>& b,bool bypassed){
    blocks.fetch_add(1,std::memory_order_relaxed);const auto k=bypassed?bypass:audio;counts[k].fetch_add(1,std::memory_order_relaxed);auto e=event(k);
    e.frames=b.getNumSamples();e.inChannels=getTotalNumInputChannels();e.outChannels=getTotalNumOutputChannels();
    e.inputEnabled=getBusCount(true)>0 && getBus(true,0)->isEnabled();e.outputEnabled=getBusCount(false)>0 && getBus(false,0)->isEnabled();e.offline=isNonRealtime();
    if(auto* head=getPlayHead())if(auto pos=head->getPosition()){e.position=true;e.playing=pos->getIsPlaying();e.looping=pos->getIsLooping();if(auto s=pos->getTimeInSamples())e.sample=*s;}
    const auto n=std::min(2,b.getNumChannels());const auto step=static_cast<float>(1.0/std::max(1.0,e.rate*.005));
    for(int i=0;i<e.frames;++i){
        const auto gate=e.tone && e.playing && !bypassed;
        envelope=bypassed?0.f:std::clamp(envelope+(gate?step:-step),0.f,1.f);
        const auto signal=static_cast<float>(std::sin(phase)*.0158489319*envelope); // -36 dBFS, not a wet-policy model.
        phase+=juce::MathConstants<double>::twoPi*440/e.rate;if(phase>=juce::MathConstants<double>::twoPi)phase-=juce::MathConstants<double>::twoPi;
        for(int c=0;c<n;++c){auto* p=b.getWritePointer(c);e.input=std::max(e.input,std::abs(p[i]));p[i]+=signal;e.output=std::max(e.output,std::abs(p[i]));}
    }
    queue.push(e);
}
// Include loss totals in the CSV itself, so absence claims can be audited later.
void Probe::drain(){
    std::ostringstream out;out<<std::setprecision(12);Event e;bool any=false;
    for(size_t i=0;i<8192 && queue.pop(e);++i){any=true;if(e.kind==audio || e.kind==bypass)latest=e;
        out<<names[e.kind]<<','<<e.time<<','<<e.block<<','<<e.mark<<','<<e.frames<<','<<e.rate<<','<<e.sample<<','<<e.position<<','<<e.playing<<','<<e.looping<<','<<e.offline<<','<<e.inChannels<<','<<e.outChannels<<','<<e.inputEnabled<<','<<e.outputEnabled<<','<<e.tone<<','<<e.input<<','<<e.output<<','<<queue.dropped.load()<<'\n';}
    if(any && evidence!=juce::File{} && !evidence.appendText(out.str()))logStatus="WRITE FAILED";
}
class Editor final:public juce::AudioProcessorEditor,private juce::Timer {
    Probe& p;juce::Label info;juce::ComboBox steps;juce::TextButton markButton{"Mark step"},folder{"Open evidence folder"},copy{"Copy diagnostics"};
    juce::ToggleButton toneButton{"Enable -36 dBFS diagnostic tone during Play"};
public:
    explicit Editor(Probe& x):AudioProcessorEditor(x),p(x){setSize(820,490);info.setFont(juce::FontOptions(15));info.setJustificationType(juce::Justification::topLeft);
        for(auto* c:std::array<juce::Component*,6>{&info,&steps,&markButton,&folder,&copy,&toneButton})addAndMakeVisible(c);
        const char* labels[]={"01 Enabled source / ordinary silence","02 Stop / Play control","03 Playlist mute / unmute","04 Channel Rack mute / unmute","05 Mixer mute / unmute","06 Effect slot off / on","07 Smart Disable control (optional)","08 Seek / loop control (optional)","09 Editor closed / reopen (optional)","10 End"};
        for(int i=0;i<10;++i)steps.addItem(labels[i],i+1);steps.setSelectedId(1);
        markButton.onClick=[this]{p.setMarker(steps.getSelectedId());};toneButton.onClick=[this]{p.tone=toneButton.getToggleState();p.setMarker(p.mark.load());};
        folder.onClick=[this]{p.evidence.getParentDirectory().startAsProcess();};copy.onClick=[this]{juce::SystemClipboard::copyTextToClipboard(info.getText());};startTimer(100);timerCallback();}
    void paint(juce::Graphics& g) override {g.fillAll(juce::Colour(0xff20242a));}
    void resized() override {info.setBounds(18,16,784,302);steps.setBounds(18,326,590,30);markButton.setBounds(624,326,172,30);toneButton.setBounds(18,370,770,28);folder.setBounds(18,434,230,30);copy.setBounds(266,434,230,30);}
    void timerCallback() override {const auto& e=p.latest;juce::String s="EXPLORE ONLY - Host Signal Probe v1 (no connector)\nInstance: "+p.instance+"\n";
        s<<"Callbacks: "<<static_cast<int64_t>(p.blocks.load())<<" | last callback age: "<<juce::String(e.block?(now()-e.time)*1000:-1,1)<<" ms | dropped rows: "<<static_cast<int>(p.queue.dropped.load())<<"\n";
        s<<"Play: "<<int(e.playing)<<" | offline: "<<int(e.offline)<<" | input peak: "<<juce::String(e.input,8)<<" | output: "<<juce::String(e.output,8)<<"\n";
        s<<"Input/output buses: "<<int(e.inputEnabled)<<"/"<<int(e.outputEnabled)<<" | marker: "<<p.mark.load()<<"\n";
        for(int k=bypass;k<kindCount;++k){s<<names[k]<<": "<<static_cast<int64_t>(p.counts[k].load())<<((k-bypass)%4==3?"\n":"   ");}
        s<<"\n"<<p.logStatus<<": "<<p.evidence.getFullPathName()<<"\nDry passes unchanged. Tone ignores source silence and host Song/Pattern.\nChoose step, click Mark step, then toggle the FL control twice slowly.\nBaseline: connector detached; Smart Disable OFF for this probe.";
        info.setText(s,juce::dontSendNotification);toneButton.setToggleState(p.tone.load(),juce::dontSendNotification);}
};
juce::AudioProcessorEditor* Probe::createEditor(){return new Editor(*this);}
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new hostprobe::Probe();}
