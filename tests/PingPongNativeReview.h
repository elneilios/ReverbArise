#pragma once

#include <DSP/PingPongResponse.h>
#include <DSP/SoundShaping.h>
#include <numbers>

namespace preview46
{
inline void sharedSoundPreparationSmoke()
{
    using namespace m3;
    for(const auto rate:{8000.,44100.,48000.,96000.,192000.,384000.})
    {
        std::vector<float> source(static_cast<size_t>(rate*.1)*2);
        for(size_t i=0;i<source.size()/2;++i)
            source[2*i]=static_cast<float>(.5*std::sin(2*std::numbers::pi*500*i/rate));
        auto filtered=source;
        REQUIRE(applySourceCut(filtered,rate,0,true,{}) && applySourceCut(filtered,rate,0,false,{}) && filtered==source);
        for(const auto highPass:{false,true})
            for(const auto hz:{SoundShaping::minimumCutoff,SoundShaping::maximumCutoff})
            {
                filtered=source;
                REQUIRE(applySourceCut(filtered,rate,hz,highPass,{}));
                for(size_t i=0;i<filtered.size();i+=2)
                    REQUIRE(std::isfinite(filtered[i]) && std::abs(filtered[i])<1 && filtered[i+1]==0);
            }
        REQUIRE(applySourceCut(filtered,rate,20,false,{}));
        REQUIRE(std::ranges::all_of(filtered,[](float value){return std::isfinite(value);}));
    }
    ImpulseResponse silence{48000,2,std::vector<float>(128),{},{}};
    REQUIRE(dampFullResponse(silence,100,{}) && std::ranges::all_of(silence.samples,[](float v){return v==0;}));
    ImpulseResponse single{48000,1,{1.f},{},{}};
    REQUIRE(dampFullResponse(single,100,{}) && single.samples.front()==1.f);
    std::cout<<"Shared Sound: Off bypass, crossed cuts, cutoff endpoints, channel isolation, and silent and single-frame IRs passed.\n";
}

inline void chooseSoundPreset(controls::SoundPresetEditor& panel, const juce::String& name)
{
    auto& picker=panel.presetPicker();
    for(int i=0;i<picker.getNumItems();++i)
        if(picker.getItemText(i)==name)
        {
            picker.beforeInvocation();
            if(picker.getSelectedId()==i+1 && picker.onReselect)
                picker.onReselect();
            else
                picker.setSelectedId(i+1,juce::sendNotificationSync);
            return;
        }
    throw std::runtime_error(("Missing from the native Sound picker: " + name).toStdString());
}

inline void choosePingPong(controls::SoundPresetEditor& panel)
{
    chooseSoundPreset(panel, m3::pingPongPresetName);
}

inline std::vector<int64_t> pingPongPeaks(const ImpulseResponse& ir)
{
    std::vector<int64_t> peaks;
    int64_t active=-static_cast<int64_t>(ir.frames());
    float largest=0;
    for(int64_t i=0;i<static_cast<int64_t>(ir.frames());++i)
    {
        const auto stored=ir.frames()-1-static_cast<size_t>(i);
        const auto amplitude=std::max(std::abs(ir.samples[2*stored]),std::abs(ir.samples[2*stored+1]));
        if(amplitude<0.00001f)continue;
        if(i-active>static_cast<int64_t>(ir.rate*.015)){peaks.push_back(i);largest=0;}
        if(amplitude>largest){peaks.back()=i;largest=amplitude;}
        active=i;
    }
    return peaks;
}

inline void pingPongStereoSmoke(const juce::File& output)
{
    using namespace m3;
    constexpr auto rate=48000;
    constexpr auto arrival=144000;
    const PingPongTiming timing{rate,arrival,0,{true,0,0,rate,82,4,4}};
    ImpulseResponse ir;
    SoundPresetIdentity preset;
    juce::String error;
    REQUIRE(preparePingPong({},timing,previewMemoryBudget,ir,preset,error));
    // A sustained source biased strongly to one side. The approved pluck was
    // much shorter and used the same dry signal in both channels.
    std::vector<float> source(rate*2);
    for(size_t i=0;i<source.size()/2;++i)
    {
        const auto time=static_cast<double>(i)/rate;
        const auto attack=std::min(1.0,time/.008);
        const auto chord=std::sin(time*2*std::numbers::pi*130.81)+.6*std::sin(time*2*std::numbers::pi*196);
        const auto sample=static_cast<float>(.18*chord*attack*std::exp(-2*time));
        source[i*2]=sample;
        source[i*2+1]=sample*.15f;
    }
    WetRender previous,updated;
    REQUIRE(renderReverseConvolution(source,arrival,rate,ir,0,previous,error));
    auto gated=source;
    REQUIRE(preparePingPongExcitation(gated,rate,0,preset,82,{}) && gated==source);
    preset.pingPong->gateAmount=100;
    REQUIRE(preparePingPongExcitation(gated,rate,0,preset,82,{}));
    REQUIRE(renderReverseConvolution(gated,arrival,rate,ir,0,updated,error));
    auto midpoint=source;
    preset.pingPong->gateAmount=50;
    REQUIRE(preparePingPongExcitation(midpoint,rate,0,preset,82,{}));
    for(size_t i=0;i<midpoint.size();++i)
        REQUIRE(std::abs(midpoint[i]-.5f*(source[i]+gated[i]))<0.000001f);
    const auto peaks=pingPongPeaks(ir);
    double weakestBefore=120,weakestAfter=120;
    for(const auto peak:peaks)
    {
        const auto response=(ir.frames()-1-static_cast<size_t>(peak))*2;
        const auto side=std::abs(ir.samples[response])>std::abs(ir.samples[response+1])?0:1;
        const auto separation=[&](const WetRender& wet)
        {
            double energy[2]{};
            for(auto frame=peak+rate/50;frame<peak+rate/8;++frame)
                for(int channel=0;channel<2;++channel)
                {
                    const auto value=wet.samples[static_cast<size_t>(frame)*2+channel];
                    energy[channel]+=value*value;
                }
            return 10*std::log10((energy[side]+1e-20)/(energy[1-side]+1e-20));
        };
        weakestBefore=std::min(weakestBefore,separation(previous));
        weakestAfter=std::min(weakestAfter,separation(updated));
    }
    REQUIRE(weakestAfter>12 && weakestAfter>weakestBefore+10);
    std::cout<<"Biased sustained stereo: weakest echo "<<weakestBefore<<" -> "<<weakestAfter<<" dB toward its intended side.\n";
    if(output==juce::File{})return;
    REQUIRE(output.createDirectory().wasOk());
    std::vector<float> comparison;
    for(const auto* wet:{&previous,&updated})
    {
        if(!comparison.empty())comparison.resize(comparison.size()+rate);
        const auto count=static_cast<size_t>(arrival+rate/50-wet->start)*2;
        comparison.insert(comparison.end(),wet->samples.begin(),wet->samples.begin()+count);
    }
    float peak=0;
    for(const auto sample:comparison)peak=std::max(peak,std::abs(sample));
    REQUIRE(peak>0);
    for(auto& sample:comparison)sample*=.8f/peak;
    const auto file=output.getChildFile("ping-pong-stereo-before-after.wav");
    REQUIRE(!file.exists() || file.deleteFile());
    writeTestWav(file,2,rate,comparison,24);
}

inline void pingPongPreparationSmoke()
{
    using namespace m3;
    juce::String error;
    const std::array<double,6> fractions{1,.5,.75,.25,.375,.125};
    for(const auto rate:{44100.,48000.,96000.})
    {
        for(size_t i=0;i<fractions.size();++i)
        {
            const auto arrival=static_cast<int64_t>(rate*4+.031*rate);
            const PingPongTiming timing{rate,arrival,0,{true,0,0,rate,82,4,4}};
            const PingPongSettings settings{true,static_cast<EchoRate>(i),100,75};
            ImpulseResponse ir,repeat;
            SoundPresetIdentity preset,again;
            REQUIRE(preparePingPong(settings,timing,previewMemoryBudget,ir,preset,error));
            REQUIRE(preparePingPong(settings,timing,previewMemoryBudget,repeat,again,error));
            REQUIRE(preset==again && ir.samples==repeat.samples && preset.rulerAligned);
            const auto peaks=pingPongPeaks(ir);
            REQUIRE(peaks.size()>=3 && peaks.size()<=11);
            if(settings.rate==EchoRate::quarter)REQUIRE(peaks.size()<11);
            const auto step=rate*60*fractions[i]/timing.ruler.bpm;
            for(const auto peak:peaks)
            {
                const auto absolute=arrival-static_cast<int64_t>(ir.frames()-1)+peak;
                const auto snapped=std::llround(std::round(absolute/step)*step);
                REQUIRE(std::abs(absolute-snapped)<=1);
            }
        }
    }
    ImpulseResponse baseline,mono,flat,stronger,spreadDefault,moderateDamping,strongDamping;
    SoundPresetIdentity preset;
    REQUIRE(preparePingPong({false,EchoRate::eighth,100,75},{},previewMemoryBudget,baseline,preset,error));
    REQUIRE(!preset.rulerAligned);
    const auto original=pingPongPeaks(baseline);
    REQUIRE(original.size()==11 && original.back()-original[original.size()-2]==10560);
    REQUIRE(preparePingPong({false,EchoRate::eighth,0,75},{},previewMemoryBudget,mono,preset,error));
    REQUIRE(preparePingPong({false,EchoRate::eighth,100,0},{},previewMemoryBudget,flat,preset,error));
    REQUIRE(preparePingPong({false,EchoRate::eighth,100,100},{},previewMemoryBudget,stronger,preset,error));
    REQUIRE(preparePingPong({false,EchoRate::eighth,85,75},{},previewMemoryBudget,spreadDefault,preset,error));
    for(size_t i=0;i<mono.frames();++i)REQUIRE(mono.samples[2*i]==mono.samples[2*i+1]);
    REQUIRE(baseline.samples!=flat.samples);
    const auto energy=[](const auto& response)
    {
        double total=0;
        for(const auto sample:response.samples)total+=sample*sample;
        return total;
    };
    REQUIRE(energy(flat)<energy(baseline) && energy(baseline)<energy(stronger));
    REQUIRE(preparePingPong({false,EchoRate::eighth,100,75,0,50},{},previewMemoryBudget,moderateDamping,preset,error));
    REQUIRE(preparePingPong({false,EchoRate::eighth,100,75,0,100},{},previewMemoryBudget,strongDamping,preset,error));
    REQUIRE(energy(strongDamping)<energy(moderateDamping) && energy(moderateDamping)<energy(baseline));
    REQUIRE(pingPongPeaks(moderateDamping)==original && pingPongPeaks(strongDamping)==original);
    for(const auto peak:original)
    {
        const auto at=(baseline.frames()-1-static_cast<size_t>(peak))*2;
        for(size_t channel=0;channel<2;++channel)
            REQUIRE(moderateDamping.samples[at+channel]==baseline.samples[at+channel]
                && strongDamping.samples[at+channel]==baseline.samples[at+channel]);
    }
    const auto tail=(spreadDefault.frames()-1-static_cast<size_t>(original.back()))*2;
    REQUIRE(std::abs(spreadDefault.samples[tail])>8*std::abs(spreadDefault.samples[tail+1]));
    REQUIRE(std::ranges::all_of(flat.samples,[](float sample){return std::isfinite(sample) && std::abs(sample)<1;}));
    ImpulseResponse tight, tightRepeat;
    SoundPresetIdentity repeated;
    const PingPongTiming fast{48000,193488,0,{true,0,0,48000,300,4,4}};
    const PingPongSettings fastest{true,EchoRate::thirtySecond,85,100};
    REQUIRE(preparePingPong(fastest,fast,previewMemoryBudget,tight,preset,error));
    REQUIRE(preparePingPong(fastest,fast,previewMemoryBudget,tightRepeat,repeated,error));
    REQUIRE(preset==repeated && tight.samples==tightRepeat.samples);
    REQUIRE(std::ranges::all_of(tight.samples,[](float sample){return std::isfinite(sample) && std::abs(sample)<1;}));
    std::cout<<"Preparation: all six rates at 82 BPM, 44.1/48/96 kHz, tight 1/32 at 300 BPM, exact repeat, Sync off, spread and build passed.\n";
}

inline void writePingPongListeningClips(const juce::File& directory)
{
    using namespace m3;
    constexpr int rate=48000;
    constexpr double bpm=82;
    constexpr auto quarter=rate*60.0/bpm;
    const auto arrival=static_cast<int64_t>(std::llround(quarter*4+rate*.12));
    const PingPongTiming timing{rate,arrival,0,{true,0,0,rate,bpm,4,4}};
    std::vector<float> kick(rate*2/10);
    for(size_t i=0;i<kick.size()/2;++i)
    {
        const auto seconds=static_cast<double>(i)/rate;
        const auto phase=2*std::numbers::pi*(51*seconds+129*.012*(1-std::exp(-seconds/.012)));
        const auto value=static_cast<float>(.7*std::sin(phase)*std::min(1.0,seconds/.001)
            *std::exp(-55*seconds)*std::clamp((.1-seconds)*100,0.0,1.0));
        kick[i*2]=kick[i*2+1]=value;
    }
    struct Clip { const char* file; EchoRate rate; int build; };
    const std::array<Clip,6> clips{{
        {"eighth-build-0.wav",EchoRate::eighth,0},{"eighth-build-100.wav",EchoRate::eighth,100},
        {"dotted-eighth-kick.wav",EchoRate::dottedEighth,75},{"sixteenth-kick.wav",EchoRate::sixteenth,75},
        {"dotted-sixteenth-kick.wav",EchoRate::dottedSixteenth,75},{"thirty-second-kick.wav",EchoRate::thirtySecond,75}}};
    std::array<WetRender,6> wet;
    juce::String error;
    float peak=0;
    for(size_t i=0;i<clips.size();++i)
    {
        ImpulseResponse ir;
        SoundPresetIdentity preset;
        REQUIRE(preparePingPong({true,clips[i].rate,85,clips[i].build},timing,previewMemoryBudget,ir,preset,error));
        REQUIRE(renderReverseConvolution(kick,arrival,rate,ir,0,wet[i],error));
        peak=std::max(peak,wet[i].peak);
    }
    REQUIRE(peak>0);
    const auto gain=.44f/peak;
    for(size_t i=0;i<clips.size();++i)
    {
        std::vector<float> mix(rate*4*2);
        for(int64_t at=0;at<rate*4;++at)
        {
            const auto phase=(at-std::llround(std::floor(at/quarter)*quarter))/static_cast<double>(rate);
            const auto click=phase>=0 && phase<.02
                ? static_cast<float>(.045*std::sin(2*std::numbers::pi*1450*phase)*std::exp(-240*phase)) : 0.0f;
            for(int c=0;c<2;++c)
            {
                const auto index=(at-wet[i].start)*2+c;
                const auto dry=(at-arrival)*2+c;
                auto& output=mix[static_cast<size_t>(at)*2+c];
                output=click;
                if(index>=0 && index<static_cast<int64_t>(wet[i].samples.size()))
                    output+=wet[i].samples[static_cast<size_t>(index)]*gain;
                if(dry>=0 && dry<static_cast<int64_t>(kick.size()))
                    output+=kick[static_cast<size_t>(dry)]*.62f;
                REQUIRE(std::abs(output)<1.f);
            }
        }
        const auto file=directory.getChildFile(clips[i].file);
        REQUIRE(!file.exists() || file.deleteFile());
        writeTestWav(file,2,rate,mix,24);
    }
}

inline void pingPongNative(std::unique_ptr<Review> fixture,const juce::File& output,bool show)
{
    using namespace m3;
    pingPongPreparationSmoke();
    sharedSoundPreparationSmoke();
    pingPongStereoSmoke(output);
    auto editor=fixture->composition.productionEditor();
    editor->enableResponsiveReview(std::make_shared<TransientPresentationStorage>());
    editor->setPresentationSize(900,600);
    editor->setVisible(true);
    auto& engine=static_cast<PreverbAudioProcessor&>(*editor->getAudioProcessor()).swellEngine();
    auto& mode=require<controls::SegmentedSelector>(*editor,"workflowMode");
    require<controls::SegmentedSelector>(*editor,"learnSections").onRequest(2);
    require<controls::SegmentedSelector>(*editor,"inspectorSections").onRequest(3);
    const auto capture=[&](const juce::String& name)
    {
        if(output==juce::File{})return;
        REQUIRE(output.createDirectory().wasOk());
        const auto image=editor->createComponentSnapshot(editor->getLocalBounds());
        juce::MemoryOutputStream bytes;
        REQUIRE(juce::PNGImageFormat().writeImageToStream(image,bytes));
        REQUIRE(output.getChildFile(name+".png").replaceWithData(bytes.getData(),bytes.getDataSize()));
    };
    const auto before=engine.savedSession();
    const auto id=before.primary;
    const auto checkCustomFileChoice=[](controls::SoundPresetEditor& panel)
    {
        chooseSoundPreset(panel, "Custom IR");
        REQUIRE(panel.presetPicker().displayedText()=="Custom IR (choose file)");
        REQUIRE(panel.importAction().isVisible());
        REQUIRE(!require<juce::Component>(panel,"soundTempoSync").isVisible());
        REQUIRE(require<KnobValue>(panel,"dampingCell").isVisible());
        REQUIRE(find(panel,"echo-rateCell")==nullptr);
        choosePingPong(panel);
        REQUIRE(require<KnobValue>(panel,"echo-rateCell").isVisible());
    };
    mode.onRequest(1);
    editor->refresh();
    auto& defaults=require<controls::SoundPresetEditor>(*editor,"learnIrEditor");
    REQUIRE(defaults.isVisible());
    choosePingPong(defaults);
    wait([&]{editor->refresh();return defaults.presetPicker().getText()==pingPongPresetName && defaults.hasResponsePreview();});
    REQUIRE(engine.savedSession().defaultSoundPreset.pingPong==PingPongSettings{});
    checkCustomFileChoice(defaults);
    REQUIRE(engine.savedSession().defaultSoundPreset.pingPong==PingPongSettings{});
    defaults.onValueBegin();
    defaults.onValue("build-intensity",45);
    defaults.onValueCancel();
    REQUIRE(find(engine.savedSession(),id)->ir==find(before,id)->ir);
    capture("learn-ping-pong-900");
    defaults.onValueBegin();
    const auto beforeGesture=engine.snapshot().defaultIRRequest;
    defaults.onValue("stereo-spread",76);
    defaults.onValue("stereo-spread",72);
    REQUIRE(require<KnobValue>(defaults,"stereo-spreadCell").valueField().getText()=="72%");
    REQUIRE(engine.snapshot().defaultIRRequest==beforeGesture);
    defaults.onValueEnd();
    wait([&]{const auto saved=engine.savedSession();return saved.defaultSoundPreset.pingPong->spread==72;});
    REQUIRE(engine.snapshot().defaultIRRequest==beforeGesture+1);
    REQUIRE(engine.savedSession().defaultSoundPreset.pingPong->build==75);

    mode.onRequest(2);
    editor->refresh();
    auto& selected=require<controls::SoundPresetEditor>(*editor,"selectedIrEditor");
    REQUIRE(selected.isVisible());
    choosePingPong(selected);
    wait([&]
    {
        editor->refresh();
        const auto session=engine.savedSession();
        const auto* swell=find(session,id);
        return selected.presetPicker().getText()==pingPongPresetName && selected.hasResponsePreview()
            && swell->phase==Phase::ready && swell->soundPreset.rulerAligned;
    });
    REQUIRE(require<juce::Component>(selected,"soundTempoSync").isVisible());
    REQUIRE(require<KnobValue>(selected,"echo-rateCell").isVisible());
    const auto beforeCustomChoice=engine.savedSession();
    checkCustomFileChoice(selected);
    const auto afterCustomChoice=engine.savedSession();
    REQUIRE(find(afterCustomChoice,id)->soundPreset==find(beforeCustomChoice,id)->soundPreset);
    REQUIRE(find(afterCustomChoice,id)->render==find(beforeCustomChoice,id)->render);
    REQUIRE(find(engine.savedSession(),id)->soundPreset.pingPong->spread==85);
    REQUIRE(engine.addTarget(72000));
    const auto created=engine.savedSession().primary;
    REQUIRE(created!=id);
    wait([&]{const auto session=engine.savedSession();return find(session,created)->phase==Phase::ready;});
    {
        const auto session=engine.savedSession();
        const auto* newSwell=find(session,created);
        REQUIRE(newSwell->soundPreset.rulerAligned && newSwell->soundPreset.pingPong->spread==72);
    }
    REQUIRE(engine.removeTarget(created,engine.assignmentEpoch()));
    REQUIRE(engine.selectTargets({id},id));

    const auto verifyRuler=[&]
    {
        const auto session=engine.savedSession();
        const auto* swell=find(session,id);
        ImpulseResponse ir;
        juce::String error;
        REQUIRE(SwellAssetStore(engine.assetDirectory()).readIR(swell->ir,previewMemoryBudget,ir,error));
        const auto peaks=pingPongPeaks(ir);
        WetRender impulse;
        REQUIRE(renderReverseConvolution({1,1},swell->arrival,ir.rate,ir,0,impulse,error));
        REQUIRE(peaks.size()>2 && swell->soundPreset.rulerAligned);
        long double origin=0;
        REQUIRE(songOrigin(session.timing,origin));
        const auto step=ir.rate*30/session.timing.bpm;
        for(const auto peak:peaks)
        {
            const auto absolute=swell->arrival+swell->placement-static_cast<int64_t>(ir.frames()-1)+peak;
            const auto grid=std::llround(origin+std::round((absolute-origin)/step)*step);
            REQUIRE(std::abs(absolute-grid)<=1);
            const auto stored=ir.frames()-1-static_cast<size_t>(peak);
            REQUIRE(std::abs(impulse.samples[static_cast<size_t>(peak)*2]-ir.samples[stored*2])<0.00002f);
            REQUIRE(std::abs(impulse.samples[static_cast<size_t>(peak)*2+1]-ir.samples[stored*2+1])<0.00002f);
        }
    };
    verifyRuler();
    const auto initial=engine.savedSession();
    auto& timeline=require<SwellTimeline>(*editor,"swellTimeline");
    editor->refresh();
    timeline.fitSelection();
    timeline.setSnap(false);
    const auto lane=timeline.sourceLane();
    const auto viewport=timeline.viewport();
    const auto originalArrival=find(initial,id)->arrival;
    const juce::Point<float> start{static_cast<float>(viewport.pixel(originalArrival,lane.getX(),lane.getWidth())),lane.getY()+10};
    const auto finish=start.translated(1800.f*lane.getWidth()/static_cast<float>(viewport.span()),0);
    const auto offGrid=originalArrival+std::llround(static_cast<long double>(finish.x-start.x)*viewport.span()/lane.getWidth());
    const auto event=[&](juce::Point<float> position,bool down)
    {
        const auto now=juce::Time::getCurrentTime();
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),position,
            juce::ModifierKeys(down?juce::ModifierKeys::leftButtonModifier:0),0,0,0,0,0,
            &timeline,&timeline,now,start,now,1,position!=start);
    };
    engine.holdRenderPublicationForTesting(true);
    timeline.mouseDown(event(start,true));
    timeline.mouseDrag(event(start+(finish-start)*.4f,true));
    timeline.mouseDrag(event(finish,true));
    REQUIRE(!engine.snapshot().irRequests.contains(id));
    REQUIRE(find(engine.savedSession(),id)->arrival==originalArrival);
    timeline.mouseUp(event(finish,false));
    wait([&]{return engine.renderPublicationWaitingForTesting();});
    REQUIRE(engine.snapshot().irRequests.at(id).timing->arrival==offGrid);
    REQUIRE(find(engine.savedSession(),id)->arrival!=offGrid && find(engine.savedSession(),id)->ir==find(initial,id)->ir);
    engine.holdRenderPublicationForTesting(false);
    wait([&]{return find(engine.savedSession(),id)->arrival==offGrid;});
    verifyRuler();
    REQUIRE(engine.edit(id,[](auto& swell,auto& error){return moveWet(swell,-1000,error);}));
    wait([&]{return find(engine.savedSession(),id)->placement==-1000;});
    verifyRuler();
    const auto assetAtFirstPlacement=find(engine.savedSession(),id)->ir;
    REQUIRE(engine.edit(id,[](auto& swell,auto& error){return moveWet(swell,-13000,error);}));
    wait([&]{return find(engine.savedSession(),id)->placement==-13000;});
    REQUIRE(find(engine.savedSession(),id)->ir==assetAtFirstPlacement);
    verifyRuler();
    REQUIRE(engine.edit(id,[](auto& swell,auto& error){return moveWet(swell,-1000,error);}));
    wait([&]{return find(engine.savedSession(),id)->placement==-1000;});
    const auto accepted=engine.savedSession();
    const auto* swell=find(accepted,id);
    REQUIRE(engine.setFrozen(id,true));
    REQUIRE(!engine.edit(id,[](auto& entry,auto& error){return moveWet(entry,-2000,error);}));
    REQUIRE(engine.setFrozen(id,false));
    juce::String json,error;
    Session decoded;
    REQUIRE(encode(engine.savedSession(),json,error) && decode(json,decoded,error));
    REQUIRE(decoded.defaultSoundPreset==accepted.defaultSoundPreset
        && find(decoded,id)->soundPreset==swell->soundPreset && find(decoded,id)->ir==swell->ir);
    for(const auto& other:before.swells)
        if(other.id!=id)REQUIRE(find(decoded,other.id)->ir==other.ir);
    engine.restore(decoded);
    wait([&]{const auto session=engine.savedSession();return find(session,id)->phase==Phase::ready && engine.snapshot().auditionReady;});
    REQUIRE(find(engine.savedSession(),id)->render==swell->render);
    verifyRuler();
    editor->refresh();
    auto& rateControl=require<KnobValue>(selected,"echo-rateCell");
    const auto chooseRate=[&](EchoRate rate,const juce::String& label)
    {
        editor->refresh();
        selected.onValueBegin();
        selected.onValue("echo-rate",static_cast<double>(rate));
        REQUIRE(rateControl.valueField().getText()==label);
        selected.onValueEnd();
        wait([&]
        {
            editor->refresh();
            const auto session=engine.savedSession();
            return find(session,id)->soundPreset.pingPong->rate==rate && !engine.snapshot().irRequests.contains(id);
        });
    };
    chooseRate(EchoRate::dottedSixteenth,"1/16.");
    chooseRate(EchoRate::thirtySecond,"1/32");
    Session fastSaved;
    REQUIRE(encode(engine.savedSession(),json,error) && decode(json,fastSaved,error));
    REQUIRE(find(fastSaved,id)->soundPreset.pingPong->rate==EchoRate::thirtySecond);
    chooseRate(EchoRate::eighth,"1/8");

    REQUIRE(engine.savedSession().gainPolicy==GainPolicy::musical);
    engine.setMonitoring(AuditionMode::wetOnly,1.0f);
    wait([&]{return engine.snapshot().auditionReady;});
    const auto recordAudition=[&](const juce::String& name)
    {
        const auto session=engine.savedSession();
        const auto& swell=*find(session,id);
        int64_t start,end;
        REQUIRE(shapeBounds(swell,swell.render.start,swell.render.sourceEnd,start,end,error));
        bool started=false;
        wait([&]{return started || (started=engine.audition());});
        std::vector<float> audio;
        audio.reserve(static_cast<size_t>(end-start)*2);
        double lateEnergy=0;
        for(auto at=start;at<end;)
        {
            float left[1024]{},right[1024]{};
            const auto count=static_cast<int>(std::min<int64_t>(1024,end-at));
            engine.processAudio(left,right,count,session.timing);
            for(int i=0;i<count;++i)
            {
                audio.push_back(left[i]);
                audio.push_back(right[i]);
                if(at+i>=end-24000)
                    lateEnergy+=left[i]*left[i]+right[i]*right[i];
            }
            at+=count;
        }
        REQUIRE(engine.snapshot().playing && lateEnergy>0);
        engine.stop();
        float left[1024]{},right[1024]{};
        engine.processAudio(left,right,1024,session.timing);
        if(output!=juce::File{})
        {
            const auto file=output.getChildFile(name+".wav");
            REQUIRE(!file.exists() || file.deleteFile());
            writeTestWav(file,2,swell.render.rate,audio,24);
        }
        return std::pair{10*std::log10(lateEnergy),swell.calibration.baselineDb};
    };
    const auto auditionValue=[&](const juce::String& control,int PingPongSettings::*field,int percent)
    {
        editor->refresh();
        auto& knob=require<KnobValue>(selected,control+"Cell");
        if((*find(engine.savedSession(),id)->soundPreset.pingPong).*field!=percent)
        {
            {
                juce::Slider::ScopedDragNotification gesture(knob.valueKnob());
                knob.valueKnob().setValue(percent/100.0,juce::sendNotificationSync);
                REQUIRE(knob.valueField().getText()==juce::String(percent)+"%");
                REQUIRE(!engine.snapshot().irRequests.contains(id));
            }
            wait([&]
            {
                editor->refresh();
                const auto saved=engine.savedSession();
                return (*find(saved,id)->soundPreset.pingPong).*field==percent && engine.snapshot().auditionReady;
            });
        }
        const auto session=engine.savedSession();
        REQUIRE(find(session,id)->soundPreset.version==pingPongPresetVersion);
        REQUIRE(find(session,id)->calibration.state!=CalibrationState::unavailable);
        return recordAudition("audition-"+control+"-"+juce::String(percent));
    };
    const auto flat=auditionValue("build-intensity",&PingPongSettings::build,0).first;
    const auto reference=auditionValue("build-intensity",&PingPongSettings::build,75).first;
    const auto full=auditionValue("build-intensity",&PingPongSettings::build,100).first;
    std::cout<<"Wet Audition late energy changes: 0 -> 75 = "<<reference-flat
             <<" dB, 75 -> 100 = "<<full-reference<<" dB.\n";
    REQUIRE(reference>flat+4 && full>reference+1);
    const auto reset=[&](controls::SoundPresetEditor& panel,const juce::String& name,const juce::String& expected)
    {
        editor->refresh();
        auto action=require<KnobValue>(panel,name+"Cell").prepareResetAction();
        REQUIRE(action.label=="Reset to default ("+expected+")" && action.apply);
        action.apply();
    };
    reset(selected,"build-intensity","75%");
    wait([&]{return find(engine.savedSession(),id)->soundPreset.pingPong->build==75;});
    const auto dryDamping=auditionValue("damping",&PingPongSettings::damping,0);
    const auto wetDamping=auditionValue("damping",&PingPongSettings::damping,100);
    std::cout<<"Wet Audition Damping 0 -> 100: "<<wetDamping.first-dryDamping.first
             <<" dB late energy; "<<wetDamping.second-dryDamping.second<<" dB matching change.\n";
    REQUIRE(wetDamping.first<dryDamping.first-2.0);
    auto previousDamping=engine.savedSession();
    auto* savedDamping=find(previousDamping,id);
    savedDamping->soundPreset.version=pingPongDampingVersion;
    REQUIRE(encode(previousDamping,json,error) && decode(json,decoded,error));
    engine.restore(decoded);
    wait([&]{return engine.snapshot().auditionReady;});
    REQUIRE(find(engine.savedSession(),id)->render==savedDamping->render);
    editor->refresh();
    reset(selected,"damping","0%");
    wait([&]{return find(engine.savedSession(),id)->soundPreset.pingPong->damping==0;});
    editor->refresh();
    chooseRate(EchoRate::sixteenth,"1/16");
    reset(selected,"echo-rate","1/8");
    wait([&]{return find(engine.savedSession(),id)->soundPreset.pingPong->rate==EchoRate::eighth;});
    editor->refresh();
    const auto openSound=engine.savedSession();
    REQUIRE(find(openSound,id)->soundPreset.pingPong->damping==0);
    selected.onValueBegin();
    selected.onValue("gate-amount",100);
    selected.onValueEnd();
    wait([&]{return find(engine.savedSession(),id)->soundPreset.pingPong->gateAmount==100;});
    auto oldGated=engine.savedSession();
    auto* oldSwell=find(oldGated,id);
    REQUIRE(oldSwell->render.content!=find(openSound,id)->render.content && oldSwell->ir==find(openSound,id)->ir);
    REQUIRE(oldGated.defaultSoundPreset==openSound.defaultSoundPreset);
    oldSwell->soundPreset.version=pingPongExcitationVersion;
    if(!encode(oldGated,json,error) || !decode(json,decoded,error))
        throw std::runtime_error("Legacy gated state: "+error.toStdString()
            +" damping="+std::to_string(oldSwell->soundPreset.pingPong->damping));
    REQUIRE(find(decoded,id)->soundPreset.pingPong->gateAmount==100 && find(decoded,id)->render==oldSwell->render);
    engine.restore(decoded);
    wait([&]{return engine.snapshot().auditionReady;});
    REQUIRE(find(engine.savedSession(),id)->render==oldSwell->render);
    reset(selected,"gate-amount","0%");
    wait([&]{return find(engine.savedSession(),id)->soundPreset.pingPong->gateAmount==0;});
    REQUIRE(find(engine.savedSession(),id)->render.content==find(openSound,id)->render.content);
    editor->refresh();
    auto previousRevision=engine.savedSession();
    auto* previousSwell=find(previousRevision,id);
    previousSwell->soundPreset.version=pingPongGateVersion;
    REQUIRE(encode(previousRevision,json,error) && decode(json,decoded,error));
    REQUIRE(find(decoded,id)->soundPreset.pingPong->damping==0 && find(decoded,id)->render==previousSwell->render);
    selected.onValueBegin();
    selected.onValue("damping",50);
    selected.onValueEnd();
    wait([&]{return find(engine.savedSession(),id)->soundPreset.pingPong->damping==50;});
    const auto damped=engine.savedSession();
    REQUIRE(find(damped,id)->render.content!=previousSwell->render.content);
    REQUIRE(find(damped,id)->soundPreset.pingPong->gateAmount==previousSwell->soundPreset.pingPong->gateAmount);
    verifyRuler();
    REQUIRE(encode(damped,json,error) && decode(json,decoded,error)
        && find(decoded,id)->soundPreset.pingPong->damping==50);
    reset(selected,"damping","0%");
    wait([&]{return find(engine.savedSession(),id)->soundPreset.pingPong->damping==0;});
    REQUIRE(find(engine.savedSession(),id)->render.content==previousSwell->render.content);
    const auto selectedSound=find(engine.savedSession(),id)->soundPreset;
    engine.setMonitoring(AuditionMode::dryWet,1.0f);
    editor->refresh();
    require<SwellTimeline>(*editor,"swellTimeline").fitSelection();
    capture("perform-ping-pong-900");
    editor->setPresentationSize(1100,760);
    capture("perform-ping-pong-expanded");
    mode.onRequest(1);
    editor->refresh();
    reset(defaults,"stereo-spread","85%");
    wait([&]{return engine.savedSession().defaultSoundPreset.pingPong->spread==85;});
    editor->refresh();
    defaults.onValueBegin();
    defaults.onValue("gate-amount",50);
    defaults.onValueEnd();
    wait([&]{return engine.savedSession().defaultSoundPreset.pingPong->gateAmount==50;});
    REQUIRE(encode(engine.savedSession(),json,error) && decode(json,decoded,error)
        && decoded.defaultSoundPreset.pingPong->gateAmount==50);
    reset(defaults,"gate-amount","0%");
    wait([&]{return engine.savedSession().defaultSoundPreset.pingPong->gateAmount==0;});
    editor->refresh();
    defaults.onValueBegin();
    defaults.onValue("damping",50);
    defaults.onValueEnd();
    wait([&]{return engine.savedSession().defaultSoundPreset.pingPong->damping==50;});
    reset(defaults,"damping","0%");
    wait([&]{return engine.savedSession().defaultSoundPreset.pingPong->damping==0;});
    editor->refresh();
    defaults.onValueBegin();
    defaults.onValue("level",3.);
    REQUIRE(engine.savedSession().defaultLevelDb==3.f);
    reset(defaults,"level","0 dB");
    REQUIRE(engine.savedSession().defaultLevelDb==0.f);
    REQUIRE(find(engine.savedSession(),id)->soundPreset==selectedSound);
    capture("learn-ping-pong-expanded");
    const auto setSoundValue=[](controls::SoundPresetEditor& panel,const juce::String& control,double value)
    {
        panel.onValueBegin();
        panel.onValue(control,value);
        panel.onValueEnd();
    };
    const auto defaultBefore=engine.savedSession();
    setSoundValue(defaults,"low-cut",SoundShaping::minimumCutoff);
    wait([&]{editor->refresh();return engine.savedSession().defaultSoundPreset.shaping.lowCut==20;});
    setSoundValue(defaults,"high-cut",SoundShaping::maximumCutoff);
    wait([&]{editor->refresh();return engine.savedSession().defaultSoundPreset.shaping.highCut==20000;});
    REQUIRE(encode(engine.savedSession(),json,error) && decode(json,decoded,error));
    REQUIRE(decoded.defaultSoundPreset.shaping.lowCut==20 && decoded.defaultSoundPreset.shaping.highCut==20000);
    REQUIRE(find(decoded,id)->soundPreset==find(defaultBefore,id)->soundPreset);
    capture("learn-shared-shaping-expanded");
    editor->setPresentationSize(900,600);
    capture("learn-shared-shaping-900");
    mode.onRequest(2);
    editor->setPresentationSize(900,600);
    editor->refresh();
    const auto selectedBefore=engine.savedSession();
    REQUIRE(engine.addTarget(72000));
    const auto shapedNew=engine.savedSession().primary;
    REQUIRE(shapedNew!=id);
    wait([&]{const auto saved=engine.savedSession();return find(saved,shapedNew)->phase==Phase::ready;});
    {
        const auto saved=engine.savedSession();
        REQUIRE(find(saved,shapedNew)->soundPreset.shaping==decoded.defaultSoundPreset.shaping);
        REQUIRE(find(saved,id)->soundPreset==find(selectedBefore,id)->soundPreset);
    }
    REQUIRE(engine.removeTarget(shapedNew,engine.assignmentEpoch()));
    REQUIRE(engine.selectTargets({id},id));
    editor->refresh();
    REQUIRE(engine.assignBundledIR(id,find(selectedBefore,id)->inputGeneration,defaultImpulseId));
    wait([&]{editor->refresh();return find(engine.savedSession(),id)->soundPreset.id==defaultImpulseId
        && engine.snapshot().auditionReady;});
    const auto neutral=engine.savedSession();
    engine.setMonitoring(AuditionMode::wetOnly,1.0f);
    wait([&]{return engine.snapshot().auditionReady;});
    if(output!=juce::File{})recordAudition("hall-damping-0");
    setSoundValue(selected,"damping",100);
    wait([&]{
        editor->refresh();
        const auto state=engine.snapshot();
        if(const auto request=state.irRequests.find(id);request!=state.irRequests.end() && request->second.error.isNotEmpty())
            throw std::runtime_error(request->second.error.toStdString());
        if(find(state.session,id)->soundPreset.shaping.damping==100)return state.auditionReady;
        if(!state.irRequests.contains(id))throw std::runtime_error("Hall shaping did not enter the render queue: "+state.message.toStdString());
        return false;
    });
    REQUIRE(find(engine.savedSession(),id)->ir==find(neutral,id)->ir);
    REQUIRE(find(engine.savedSession(),id)->render.content!=find(neutral,id)->render.content);
    REQUIRE(encode(engine.savedSession(),json,error) && decode(json,decoded,error));
    REQUIRE(find(decoded,id)->soundPreset.shaping.damping==100
        && find(decoded,id)->render==find(engine.savedSession(),id)->render);
    if(output!=juce::File{})recordAudition("hall-damping-100");
    capture("perform-shared-shaping-900");
    editor->setPresentationSize(1100,760);
    capture("perform-shared-shaping-expanded");
    editor->setPresentationSize(900,600);
    reset(selected,"damping","0%");
    wait([&]{editor->refresh();return find(engine.savedSession(),id)->soundPreset.shaping.damping==0
        && engine.snapshot().auditionReady;});
    REQUIRE(find(engine.savedSession(),id)->render.content==find(neutral,id)->render.content);
    auto shapedPlate=engine.savedSession();
    REQUIRE(engine.assignBundledIR(id,find(shapedPlate,id)->inputGeneration,"intimate-plate"));
    wait([&]{editor->refresh();return find(engine.savedSession(),id)->soundPreset.id=="intimate-plate"
        && engine.snapshot().auditionReady;});
    const auto plate=engine.savedSession();
    if(output!=juce::File{})recordAudition("plate-damping-0");
    setSoundValue(selected,"damping",100);
    wait([&]{editor->refresh();return find(engine.savedSession(),id)->soundPreset.shaping.damping==100
        && engine.snapshot().auditionReady;});
    REQUIRE(find(engine.savedSession(),id)->render.content!=find(plate,id)->render.content);
    if(output!=juce::File{})recordAudition("plate-damping-100");
    const auto retained=engine.savedSession();
    REQUIRE(engine.assignManagedIR(id,find(retained,id)->inputGeneration,find(retained,id)->ir,"Custom fixture",engine.assignmentEpoch()));
    wait([&]{editor->refresh();return find(engine.savedSession(),id)->soundPreset.id==customImpulseId
        && engine.snapshot().auditionReady;});
    REQUIRE(find(engine.savedSession(),id)->soundPreset.shaping.damping==100);
    capture("perform-custom-shaping-900");
    REQUIRE(engine.setFrozen(id,true));
    editor->refresh();
    REQUIRE(!require<KnobValue>(selected,"low-cutCell").isEnabled());
    capture("perform-custom-shaping-frozen-900");
    REQUIRE(engine.setFrozen(id,false));
    editor->refresh();
    auto& highPass=require<KnobValue>(selected,"low-cutCell");
    selected.onValueBegin();
    selected.onValue("low-cut",110);
    REQUIRE(highPass.valueField().getText().contains("110"));
    selected.onValue("low-cut",180);
    REQUIRE(highPass.valueField().getText().contains("180"));
    REQUIRE(!engine.snapshot().irRequests.contains(id));
    selected.onValueEnd();
    wait([&]{editor->refresh();return find(engine.savedSession(),id)->soundPreset.shaping.lowCut==180
        && engine.snapshot().auditionReady;});
    reset(selected,"low-cut","Off");
    wait([&]{editor->refresh();return find(engine.savedSession(),id)->soundPreset.shaping.lowCut==0
        && engine.snapshot().auditionReady;});
    const auto acceptedCustom=engine.savedSession();
    const auto acceptedRender=find(acceptedCustom,id)->render;
    const auto acceptedPreset=find(acceptedCustom,id)->soundPreset;
    const auto clock=acceptedCustom.timing;
    const auto setCut=[&](int hz)
    {
        const auto session=engine.savedSession();
        auto shaping=find(session,id)->soundPreset.effectiveShaping();
        shaping.lowCut=hz;
        return engine.assignSoundShaping(id,find(session,id)->inputGeneration,shaping,engine.assignmentEpoch());
    };
    engine.setLivePlayback(true,LiveRouting::wetOnly,0);
    const auto offlineBlock=[&](int64_t sample)
    {
        auto time=clock;
        time.sample=sample;
        time.ppq=clock.ppq+(sample-clock.sample)*clock.bpm/(60*clock.rate);
        std::array<float,2048> left{},right{};
        engine.processOfflineAudio(left.data(),right.data(),static_cast<int>(left.size()),{time});
        REQUIRE(!engine.snapshot().offlineStatus.startsWith("Export incomplete"));
        return std::pair{left,right};
    };
    const auto comparisonSample=find(acceptedCustom,id)->arrival-2048;
    offlineBlock(comparisonSample-2048);
    const auto expectedBounce=offlineBlock(comparisonSample);
    REQUIRE(std::ranges::any_of(expectedBounce.first,[](float sample){return sample!=0;}));
    engine.endOffline();

    REQUIRE(engine.assignIR(id,find(acceptedCustom,id)->inputGeneration,
        engine.assetDirectory().getChildFile("missing-shaping-test.wav"),false,engine.assignmentEpoch()));
    wait([&]{
        const auto state=engine.snapshot();
        const auto request=state.irRequests.find(id);
        return request!=state.irRequests.end() && request->second.error.isNotEmpty();
    });
    REQUIRE(find(engine.savedSession(),id)->render==acceptedRender);
    REQUIRE(find(engine.savedSession(),id)->soundPreset==acceptedPreset);
    offlineBlock(comparisonSample-2048);
    REQUIRE(offlineBlock(comparisonSample)==expectedBounce);
    engine.endOffline();

    engine.holdRenderPublicationForTesting(true);
    REQUIRE(setCut(150));
    wait([&]{return engine.renderPublicationWaitingForTesting();});
    REQUIRE(setCut(480));
    REQUIRE(find(engine.savedSession(),id)->render==acceptedRender);
    REQUIRE(engine.setFrozen(id,true));
    REQUIRE(!setCut(960));
    engine.holdRenderPublicationForTesting(false);
    REQUIRE(find(engine.savedSession(),id)->soundPreset==acceptedPreset);
    offlineBlock(comparisonSample-2048);
    REQUIRE(offlineBlock(comparisonSample)==expectedBounce);
    engine.endOffline();
    REQUIRE(engine.setFrozen(id,false));
    wait([&]{
        editor->refresh();
        const auto state=engine.snapshot();
        return state.auditionReady && find(state.session,id)->phase==Phase::ready;
    });
    engine.holdRenderPublicationForTesting(true);
    REQUIRE(setCut(230));
    wait([&]{return engine.renderPublicationWaitingForTesting();});
    REQUIRE(find(engine.savedSession(),id)->soundPreset.shaping.lowCut==0);
    REQUIRE(find(engine.savedSession(),id)->render==find(acceptedCustom,id)->render);
    editor->refresh();
    auto& lowPass=require<KnobValue>(selected,"high-cutCell");
    REQUIRE(!lowPass.isEnabled());
    REQUIRE(lowPass.valueField().getTooltip().contains("finish preparing"));
    selected.onValueBegin();
    selected.onValue("high-cut",1000);
    selected.onValueEnd();
    if(output!=juce::File{})
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil(2200);
        capture("perform-custom-shaping-preparing-900");
    }
    engine.holdRenderPublicationForTesting(false);
    wait([&]{editor->refresh();return find(engine.savedSession(),id)->soundPreset.shaping.lowCut==230
        && engine.snapshot().auditionReady;});
    REQUIRE(lowPass.isEnabled() && find(engine.savedSession(),id)->soundPreset.shaping.highCut==0);
    offlineBlock(comparisonSample-2048);
    REQUIRE(offlineBlock(comparisonSample)!=expectedBounce);
    engine.endOffline();
    if(output!=juce::File{})recordAudition("custom-high-pass-230");
    const auto acceptedFraction=highPass.displayedFraction();
    REQUIRE(acceptedFraction>.3 && acceptedFraction<.4);
    REQUIRE(static_cast<juce::Component&>(highPass.valueKnob()).keyPressed(juce::KeyPress(juce::KeyPress::upKey)));
    wait([&]{
        editor->refresh();
        const auto cutoff=find(engine.savedSession(),id)->soundPreset.shaping.lowCut;
        return cutoff>230 && cutoff<300 && engine.snapshot().auditionReady;
    });
    highPass.valueField().setText("230",false);
    highPass.valueField().onReturnKey();
    wait([&]{editor->refresh();return find(engine.savedSession(),id)->soundPreset.shaping.lowCut==230
        && engine.snapshot().auditionReady;});
    reset(selected,"low-cut","Off");
    wait([&]{editor->refresh();return find(engine.savedSession(),id)->soundPreset.shaping.lowCut==0
        && engine.snapshot().auditionReady;});
    choosePingPong(selected);
    wait([&]{
        editor->refresh();
        const auto state=engine.snapshot();
        if(const auto request=state.irRequests.find(id);request!=state.irRequests.end() && request->second.error.isNotEmpty())
            throw std::runtime_error(request->second.error.toStdString());
        const auto* swell=find(state.session,id);
        return swell->soundPreset.pingPong && swell->soundPreset.effectiveShaping().damping==100 && state.auditionReady;
    });
    if(output!=juce::File{})recordAudition("custom-high-pass-off");
    if(output!=juce::File{})writePingPongListeningClips(output);
    if(!show)
    {
        auto& processor=static_cast<PreverbAudioProcessor&>(*editor->getAudioProcessor());
        const auto original=engine.savedSession();
        const auto originalIr=find(original,id)->ir;
        const auto originalClock=original.timing;
        Head testHead;
        struct RestorePlayHead
        {
            PreverbAudioProcessor& processor;
            juce::AudioPlayHead* previous;
            ~RestorePlayHead() { processor.setPlayHead(previous); }
        } restore{processor,processor.getPlayHead()};
        processor.setPlayHead(&testHead);
        const auto observe=[&](int64_t sample,double bpm)
        {
            testHead.position.setBpm(bpm);
            testHead.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
            testHead.position.setTimeInSamples(sample);
            testHead.position.setPpqPosition(sample*bpm/(originalClock.rate*60.));
            testHead.position.setIsPlaying(false);
            juce::AudioBuffer<float> audio(2,256);
            juce::MidiBuffer midi;
            audio.clear();
            processor.processBlock(audio,midi);
        };
        observe(24000,originalClock.bpm);
        observe(12000,originalClock.bpm);
        REQUIRE(find(engine.savedSession(),id)->ir==originalIr);
        const auto newTempo=originalClock.bpm+10;
        observe(12000,newTempo);
        observe(12256,newTempo);
        wait([&]{
            const auto saved=engine.savedSession();
            const auto* changed=find(saved,id);
            return changed->phase==Phase::ready && changed->render.bpm==newTempo
                && changed->soundPreset.rulerAligned;
        });
        REQUIRE(find(engine.savedSession(),id)->ir!=originalIr);
        verifyRuler();
    }
    std::cout<<"Native Sound: Learn/Perform isolation and inheritance, pending audio retention, Target and Move Wet grid, response reuse, Freeze, and saved render restore passed.\n";
    if(!show)return;
    class Window final : public juce::DocumentWindow
    {
    public:
        Window():DocumentWindow("ReverbArise - Stereo Ping Pong Crescendo",background,closeButton)
        {setUsingNativeTitleBar(true);}
        void closeButtonPressed() override {juce::MessageManager::getInstance()->stopDispatchLoop();}
    } window;
    window.setContentOwned(editor.release(),true);
    window.centreWithSize(900,600);
    window.setVisible(true);
    juce::MessageManager::getInstance()->runDispatchLoop();
}
}
