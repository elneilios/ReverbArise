#include "Probe.h"
#define NOMINMAX
#include <windows.h>
#include <sstream>
#include <iomanip>

namespace probe {
double qpcSeconds() noexcept {static const double frequency=[] {LARGE_INTEGER f;QueryPerformanceFrequency(&f);return double(f.QuadPart);}();LARGE_INTEGER n;QueryPerformanceCounter(&n);return n.QuadPart/frequency;}
Receiver::Receiver():AudioProcessor(BusesProperties().withInput("Dry input",juce::AudioChannelSet::stereo(),true).withOutput("Dry plus probe",juce::AudioChannelSet::stereo(),true)) {
    qpcSeconds(); // Initialise clock outside the callback.
    const auto add=[this](const char* id,const char* name){auto* p=new juce::AudioParameterFloat(juce::ParameterID{id,1},name,juce::NormalisableRange<float>{0.f,1.f,0.f},0.f);addParameter(p);return p;};
    context=add("ra_probe_context_v1","RA Probe Context v1");
    ack=add("ra_probe_ack_v1","RA Probe Audio Ack v1");
    query=add("ra_probe_query_v1","RA Probe Query Clock v1");
    written=add("ra_probe_write_v1","RA Probe Write Clock v1");
    auto dir=juce::File(PROBE_EVIDENCE_DIR);
    if(dir.createDirectory().wasOk()) {
        evidence=dir.getChildFile("receiver-"+instance+".csv");
        logStatus=evidence.replaceWithText("block,qpc_seconds,packet,sequence,mode,fresh,playing,offline,tone_enabled,frames,rate,host_sample,query_to_callback_ms,write_to_callback_ms,input_peak,output_peak,probe_signal_peak\n")?"Logging":"LOG WRITE FAILED";
    } else logStatus="LOG DIRECTORY FAILED";
    startTimer(100);
}
void Receiver::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {
    auto pos=getPlayHead()!=nullptr?getPlayHead()->getPosition():juce::Optional<juce::AudioPlayHead::PositionInfo>{};
    processAt(b,qpcSeconds(),pos && pos->getIsPlaying(),isNonRealtime(),pos && pos->getTimeInSamples()?*pos->getTimeInSamples():-1);
}
void Receiver::processAt(juce::AudioBuffer<float>& b,double now,bool playing,bool offline,int64_t hostSample) noexcept {
    if(resetPending.exchange(false)){lastPacket=0;lastReceipt=-1;envelope=0;setRaw(ack,0);}
    const auto packet=word(context->get());const auto sequence=packet>>2;const auto mode=packet&3;
    const auto q=word(query->get()),w=word(written->get());
    if(packet!=lastPacket && sequence!=0 && mode<3) {lastPacket=packet;lastReceipt=now;setRaw(ack,normalized(packet));}
    const auto queryAge=ageMs(now,q),writeAge=ageMs(now,w);
    const auto fresh=lastReceipt>=0 && (now-lastReceipt)*1000<750 && queryAge<750;
    const auto on=toneEnabled.load(std::memory_order_relaxed);
    const auto gate=on && fresh && mode==2 && (playing || offline);
    Event e;e.block=++blocks;e.seconds=now;e.packet=lastPacket;e.mode=sequence!=0 && mode<3?static_cast<int>(mode):0;
    e.fresh=fresh;e.playing=playing;e.offline=offline;e.tone=on;e.frames=b.getNumSamples();e.rate=rate;e.hostSample=hostSample;
    if(sequence!=0){e.queryMs=queryAge;e.writeMs=writeAge;}
    const auto step=static_cast<float>(1.0/std::max(1.0,rate*.005));
    const auto channels=std::min(2,b.getNumChannels());
    for(int i=0;i<b.getNumSamples();++i) {
        envelope=std::clamp(envelope+(gate?step:-step),0.f,1.f);
        const auto signal=static_cast<float>(std::sin(phase)*.0630957344*envelope);
        phase+=juce::MathConstants<double>::twoPi*440/rate;if(phase>=juce::MathConstants<double>::twoPi)phase-=juce::MathConstants<double>::twoPi;
        e.signalPeak=std::max(e.signalPeak,std::abs(signal));
        for(int c=0;c<channels;++c){auto* samples=b.getWritePointer(c);e.inputPeak=std::max(e.inputPeak,std::abs(samples[i]));samples[i]+=signal;e.outputPeak=std::max(e.outputPeak,std::abs(samples[i]));}
    }
    ring.push(e);
}
void Receiver::timerCallback() {
    std::ostringstream rows;rows<<std::setprecision(12);Event e;bool any=false;
    while(ring.pop(e)){latest=e;any=true;rows<<e.block<<','<<e.seconds<<','<<e.packet<<','<<(e.packet>>2)<<','<<e.mode<<','<<e.fresh<<','<<e.playing<<','<<e.offline<<','<<e.tone<<','<<e.frames<<','<<e.rate<<','<<e.hostSample<<','<<e.queryMs<<','<<e.writeMs<<','<<e.inputPeak<<','<<e.outputPeak<<','<<e.signalPeak<<'\n';}
    if(any && evidence!=juce::File{} && !evidence.appendText(rows.str()))logStatus="LOG WRITE FAILED";
}
class Editor final: public juce::AudioProcessorEditor,private juce::Timer {
    Receiver& receiver;juce::Label info;juce::ToggleButton tone{"Enable -24 dBFS probe tone (Song only)"};
    juce::TextButton folder{"Open evidence folder"},copy{"Copy current diagnostics"};
public:
    explicit Editor(Receiver& p):AudioProcessorEditor(p),receiver(p) {
        setSize(760,410);info.setFont(juce::FontOptions(16));info.setJustificationType(juce::Justification::topLeft);
        addAndMakeVisible(info);addAndMakeVisible(tone);addAndMakeVisible(folder);addAndMakeVisible(copy);
        tone.setToggleState(p.toneEnabled.load(),juce::dontSendNotification);tone.onClick=[this]{receiver.toneEnabled=tone.getToggleState();};
        folder.onClick=[this]{receiver.evidence.getParentDirectory().startAsProcess();};copy.onClick=[this]{juce::SystemClipboard::copyTextToClipboard(info.getText());};
        startTimer(100);timerCallback();
    }
    void paint(juce::Graphics& g) override {g.fillAll(juce::Colour(0xff20242a));}
    void resized() override {info.setBounds(18,18,724,290);tone.setBounds(18,310,700,28);folder.setBounds(18,358,220,30);copy.setBounds(252,358,240,30);}
    void timerCallback() override {
        const auto& e=receiver.latest;const bool callbackRecent=e.block!=0 && (qpcSeconds()-e.seconds)<.75;
        const bool fresh=callbackRecent && e.fresh;const char* names[]={"Unknown","Pattern","Song"};
        juce::String text="EXPLORE ONLY - ReverbArise FL Context Probe v1\nInstance: "+receiver.instance+"\n";
        text<<"Received: "<<names[std::clamp(e.mode,0,2)]<<" | "<<(fresh?"FRESH":"STALE / UNKNOWN")<<" | effective: "<<(fresh?names[std::clamp(e.mode,0,2)]:"Unknown")<<"\n";
        text<<"Consumed seq: "<<static_cast<int64_t>(e.packet>>2)<<" | block: "<<static_cast<int64_t>(e.block)<<" | dropped rows: "<<static_cast<int>(receiver.ring.dropped.load())<<"\n";
        text<<"Play: "<<int(e.playing)<<" | offline: "<<int(e.offline)<<" | frames: "<<e.frames<<" | signal peak: "<<juce::String(e.signalPeak,6)<<"\n";
        text<<"Query -> callback: "<<juce::String(e.queryMs,3)<<" ms | write -> callback: "<<juce::String(e.writeMs,3)<<" ms\n";
        text<<"QPC stamps: 0.125 ms resolution, modulo 2097.152 s; inspect CSV for coherence.\n";
        text<<receiver.logStatus<<": "<<receiver.evidence.getFullPathName()<<"\nDry input always passes unchanged; probe tone fades within 5 ms.\nDisable Smart disable for the probe during this experiment.";
        info.setText(text,juce::dontSendNotification);
    }
};
juce::AudioProcessorEditor* Receiver::createEditor(){return new Editor(*this);}
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new probe::Receiver();}
