#pragma once
#include <Capture/PerformanceComparison.h>
#include <Capture/CaptureSession.h>
inline void performanceExportCases()
{
    // Compare to a retained baseline, including phase/noise variation, drift,
    // changed spectral content, new coverage and an explicit exact override.
    TestDirectory root;CaptureSession capture(root.file);std::vector<float> original(4800*2),variation(original.size());
    for(int n=0;n<4800;++n)for(int c=0;c<2;++c)original[n*2+c]=.2f*std::sin(n*2*juce::MathConstants<double>::pi*440/48000);
    REQUIRE(capture.commit(0,48000,original.data(),4800,.15));const auto baseline=capture.snapshot().reference;
    for(int pass=0;pass<4;++pass){for(int n=0;n<4800;++n)for(int c=0;c<2;++c)variation[n*2+c]=static_cast<float>((1.02+pass*.025)*.2*std::sin(n*2*juce::MathConstants<double>::pi*440/48000+.05)+.0001*std::sin(n*1.3));
        REQUIRE(capture.commit(0,48000,variation.data(),4800,.15));REQUIRE(capture.snapshot().reference==baseline);}
    for(size_t n=0;n<variation.size();++n)variation[n]=original[n]*1.4f;
    REQUIRE(capture.commit(0,48000,variation.data(),4800,.15));REQUIRE(capture.snapshot().reference!=baseline);
    REQUIRE(capture.commit(0,48000,original.data(),4800));const auto exact=capture.snapshot().reference;
    variation=original;variation[12]+=.00001f;REQUIRE(capture.commit(0,48000,variation.data(),4800,0));REQUIRE(capture.snapshot().reference!=exact);
    for(int n=0;n<4800;++n)for(int c=0;c<2;++c)variation[n*2+c]=.2f*std::sin(n*2*juce::MathConstants<double>::pi*2000/48000);
    const auto before=capture.snapshot().reference;REQUIRE(capture.commit(0,48000,variation.data(),4800,.3));REQUIRE(capture.snapshot().reference!=before);
    REQUIRE(capture.commit(0,48000,original.data(),4800));const auto beforeTransient=capture.snapshot().reference;
    variation=original;for(int n=2400;n<3000;++n)for(int c=0;c<2;++c)variation[n*2+c]+=.8f;
    REQUIRE(capture.commit(0,48000,variation.data(),4800,.3));REQUIRE(capture.snapshot().reference!=beforeTransient);
    REQUIRE(capture.commit(10000,48000,variation.data(),4800,.3));REQUIRE(capture.snapshot().manifest.ranges.back().startSample==10000);

    auto p=std::make_unique<PreverbAudioProcessor>(root.file);auto& engine=p->swellEngine();engine.setAutomaticProcessing(false);
    p->prepareToPlay(1000,4096);TestPlayHead head;p->setPlayHead(&head);head.position.setBpm(240);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    const auto position=[&](int64_t at,bool playing){head.position.setIsPlaying(playing);head.position.setTimeInSamples(at);head.position.setPpqPosition(at/250.0);};
    juce::AudioBuffer<float> buffer(2,1000);juce::MidiBuffer midi;
    for(int at=0;at<24000;at+=1000){position(at,true);for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),1000,.125f);p->processBlock(buffer,midi);REQUIRE(p->flushCaptureForTesting());}
    p->setMappingEnabled(false);position(0,false);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(engine.addTarget(500));const auto first=engine.savedSession().primary;
    REQUIRE(engine.addTarget(520));const auto overlap=engine.savedSession().primary;
    REQUIRE(engine.addTarget(20000));const auto last=engine.savedSession().primary;
    REQUIRE(engine.selectTargets({first,overlap,last},first));const auto file=root.file.getChildFile("export-ir.wav");writeTestWav(file,1,1000,std::vector<float>(101,.001f));engine.loadIR(file);
    waitM3([&]{return engine.savedSession().swells.back().irName=="export-ir.wav";});
    // Long IR makes a single continuous wet interval cross the 16384-frame page seam.
    REQUIRE(engine.selectTargets({last},last));const auto longFile=root.file.getChildFile("long-export-ir.wav");writeTestWav(longFile,1,1000,std::vector<float>(20001,.000001f));engine.loadIR(longFile);
    waitM3([&]{return engine.savedSession().swells.back().irName=="long-export-ir.wav";});REQUIRE(engine.selectTargets({first,overlap,last},first));engine.generate(true);
    waitM3([&]{const auto s=engine.savedSession();return std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
    for(const auto& w:engine.savedSession().swells)REQUIRE(engine.edit(w.id,[](auto& s,auto&){s.shape.endOffset=0;s.shape.fadeIn=s.shape.fadeOut=0;return true;}));
    REQUIRE(engine.setFrozen(first,true));REQUIRE(!engine.edit(first,[](auto& s,auto&){++s.sourceEnd;return true;}));
    REQUIRE(engine.edit(first,[](auto& s,auto&){s.shape.gainDb=-6;s.name="Frozen accepted take";return true;}));
    engine.setIsolateDry(true);engine.setRegenerationSensitivity(.2f);engine.setLivePlayback(true,m3::LiveRouting::dryWet,0);
    const auto frozen=engine.savedSession();const auto captureRef=p->getCaptureSnapshot().reference;
    const auto renderCount=engine.snapshot().renderCount;
    // There are no worker sleeps during bounce. Host Playing is deliberately false.
    const auto bounce=[&](std::vector<int> sizes,int start,int end){p->prepareToPlay(1000,4096);p->setNonRealtime(true);std::vector<float> out;
        for(int at=start,index=0;at<end;++index){const auto count=std::min(sizes[index%sizes.size()],end-at);buffer.setSize(2,count);position(at,false);
            for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),count,.25f);p->processBlock(buffer,midi);
            REQUIRE(!engine.snapshot().liveStatus.startsWith("Export incomplete"));for(int n=0;n<count;++n)out.push_back(buffer.getSample(0,n));at+=count;}
        return out;};
    const auto a=bounce({17,4096,513,1,8192},0,23000);const auto b=bounce({20000,3000},0,23000);REQUIRE(a==b);
    // Independent positioned convolution: constant source * 101-sample box IR.
    for(int at=0;at<23000;++at){double wet=0;
        for(const auto& w:frozen.swells){const auto start=w.arrival-w.ir.frames+1,end=w.arrival;if(at<start || at>=end)continue;
            const auto age=at-start;const auto crop=std::min({1.0,age/2.0,(end-1-at)/2.0});const auto admission=std::min(1.0,(age+1)/5.0);
            wet+=std::min(age+1,w.render.sourceEnd-w.sourceStart)*.125*(w.id==last?.000001:.001)*std::pow(10.0,w.shape.gainDb/20.0)*crop*admission;}
        REQUIRE(std::abs(a[at]-(.25+wet))<1e-6);}
    REQUIRE(p->getCaptureSnapshot().reference==captureRef && engine.snapshot().renderCount==renderCount);
    engine.setLivePlayback(true,m3::LiveRouting::wetOnly,0);const auto cold=bounce({1023},19930,20020);
    REQUIRE(cold[0]>0 && std::abs(cold[10]-.00003125f)<1e-6f); // Cold export is immediately prepared; no leaked dry.
    // A frozen render survives actual changed source, state roundtrip and missing capture.
    p->setNonRealtime(false);p->setMappingEnabled(true);position(0,true);buffer.setSize(2,1000);for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),1000,.25f);
    {AudioAllocationGuard guard;p->processBlock(buffer,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);REQUIRE(p->flushCaptureForTesting());p->setMappingEnabled(false);
    waitM3([&]{const auto s=engine.savedSession();return s.swells[0].phase==m3::Phase::ready;});REQUIRE(engine.savedSession().swells[0].render==frozen.swells[0].render);
    juce::String json,error;m3::Session decoded;REQUIRE(m3::encode(engine.savedSession(),json,error) && m3::decode(json,decoded,error));REQUIRE(decoded.swells[0].frozen && decoded.regenerationSensitivity==.2f && decoded.isolateDry);
    auto legacy=juce::JSON::parse(json);legacy.getDynamicObject()->setProperty("version",5);legacy.getDynamicObject()->removeProperty("regenerationSensitivity");
    REQUIRE(m3::decode(juce::JSON::toString(legacy),decoded,error));REQUIRE(!decoded.swells[0].frozen && decoded.regenerationSensitivity==.5f);
    auto previousVersion=juce::JSON::parse(json);previousVersion.getDynamicObject()->setProperty("version",6);previousVersion.getDynamicObject()->removeProperty("isolateDry");
    REQUIRE(m3::decode(juce::JSON::toString(previousVersion),decoded,error) && !decoded.isolateDry && decoded.swells[0].frozen && decoded.regenerationSensitivity==.2f);
    auto invalid=juce::JSON::parse(json);invalid.getDynamicObject()->setProperty("regenerationSensitivity",1.1);REQUIRE(!m3::decode(juce::JSON::toString(invalid),decoded,error));
    {juce::MemoryBlock bytes;p->getStateInformation(bytes);auto reopened=std::make_unique<PreverbAudioProcessor>(root.file);
        reopened->prepareToPlay(1000,512);reopened->setPlayHead(&head);reopened->setStateInformation(bytes.getData(),static_cast<int>(bytes.getSize()));REQUIRE(reopened->flushCaptureForTesting());
        buffer.clear();reopened->processBlock(buffer,midi);REQUIRE(reopened->swellEngine().savedSession().regenerationSensitivity==.2f && reopened->swellEngine().savedSession().isolateDry);
        waitM3([&]{return reopened->swellEngine().savedSession().swells[0].phase==m3::Phase::ready;});REQUIRE(reopened->swellEngine().savedSession().swells[0].frozen);}
    REQUIRE(engine.setFrozen(first,false));waitM3([&]{const auto s=engine.savedSession();return s.swells[0].phase==m3::Phase::ready && s.swells[0].render!=frozen.swells[0].render;});
    REQUIRE(engine.setFrozen(first,true));auto saved=engine.savedSession();saved.swells.erase(saved.swells.begin()+1,saved.swells.end());saved.selected={first};saved.primary=first;
    REQUIRE(engine.assetDirectory().getChildFile(saved.swells[0].ir.path).deleteFile());
    p->requestClearCapture();REQUIRE(p->flushCaptureForTesting());engine.restore(saved);position(0,false);buffer.clear();p->processBlock(buffer,midi);
    waitM3([&]{return engine.savedSession().swells[0].phase==m3::Phase::ready && engine.snapshot().liveReady;});
    const auto recovered=bounce({13,4096},0,800);REQUIRE(recovered[450]>0);
    // Corrupt/missing render is explicit, stable failure, never a successful silent bounce.
    const auto asset=engine.savedSession().swells[0].render;REQUIRE(engine.assetDirectory().getChildFile(asset.path).deleteFile());
    p->prepareToPlay(1000,512);position(450,false);buffer.setSize(2,16);buffer.clear();p->processBlock(buffer,midi);
    REQUIRE(engine.snapshot().liveStatus.startsWith("Export warning"));const auto failure=engine.snapshot().liveStatus;
    position(466,false);buffer.clear();p->processBlock(buffer,midi);REQUIRE(engine.snapshot().liveStatus==failure);
    p->setNonRealtime(false);engine.setLivePlayback(false,m3::LiveRouting::dryWet,0);position(0,false);buffer.clear();
    {AudioAllocationGuard guard;p->processBlock(buffer,midi);}REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    REQUIRE(engine.residentBytes()<=previewMemoryBudget);
}
