#pragma once

// Actual host passes, sparse source admission, loop revisit and project restore.
static void swellListCaptureCases(bool review=false)
{
    TestDirectory root;auto p=std::make_unique<PreverbAudioProcessor>(root.file);auto& engine=p->swellEngine();
    UIPlayHead head;p->prepareToPlay(48000,4096);p->setPlayHead(&head);
    head.position.setBpm(120);head.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    waitUI([&]{return engine.savedSession().defaultIR.present();});
    auto editor=std::make_unique<PreverbAudioProcessorEditor>(*p);editor->setVisible(true);editor->refresh();
    snapshotEditor(*editor,"ordering-empty");
    const auto listFor=[](PreverbAudioProcessorEditor& e)
    {auto* list=dynamic_cast<m3::ui::controls::SwellList*>(named(e,"swellList"));REQUIRE(list);return list;};
    juce::AudioBuffer<float> buffer(2,4096);juce::MidiBuffer midi;
    const auto pass=[&](int64_t start,int loops,float amplitude,size_t expectedSwells)
    {
        const auto previousAnalysis=engine.snapshot().analysisCount;
        REQUIRE(p->requestWorkflowMode(workflow::Mode::capture));head.position.setIsPlaying(true);
        for(int repeat=0;repeat<loops;++repeat)for(int local=0;local<48000;local+=4096)
        {
            const auto count=std::min(4096,48000-local);buffer.setSize(2,count);buffer.clear();
            for(int n=0;n<count;++n){const auto at=local+n-12000;if(at>=0 && at<10000)
                {const auto value=static_cast<float>(amplitude*std::exp(-at/1800.)*std::sin(at*.17));buffer.setSample(0,n,value);buffer.setSample(1,n,value*.7f);}}
            head.position.setTimeInSamples(start+local);head.position.setPpqPosition((start+local)/24000.0);p->processBlock(buffer,midi);
        }
        finishCaptureFixture(*p,head.position);
        // The previous pass can already be ready before the engine observes the
        // newly committed source. Wait for this pass, not that old idle snapshot.
        waitUI([&]{const auto s=engine.snapshot();return s.analysisCount>previousAnalysis && s.session.swells.size()==expectedSwells && !s.analyzing && s.pendingIntervals==0
            && std::all_of(s.session.swells.begin(),s.session.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
        editor->refresh();
    };
    pass(192000,1,.3f,1);REQUIRE(engine.savedSession().swells.size()==1);const auto late=engine.savedSession().swells.front().id;
    REQUIRE(engine.edit(late,[](auto& w,auto&){w.name="AAA later section";w.enabled=false;w.muted=true;w.solo=true;w.shape.gainDb=-7;return true;}));
    REQUIRE(engine.setFrozen(late,true));const auto protectedLate=*m3::find(engine.savedSession(),late);
    pass(0,2,.3f,2);const auto learned=engine.savedSession();REQUIRE(learned.swells.size()==2 && learned.swells.front().id==late);
    const auto early=learned.swells.back().id;REQUIRE(m3::find(learned,early)->arrival<48000);
    REQUIRE(engine.edit(early,[](auto& w,auto&){w.name="ZZZ earlier section";return true;}));editor->refresh();
    auto* list=listFor(*editor);REQUIRE(list->rowAt(0)->id==early && list->rowAt(1)->id==late);
    REQUIRE(m3::sameSavedSwell(protectedLate,*m3::find(engine.savedSession(),late)));
    const auto capture=p->getCaptureSnapshot();REQUIRE(capture.manifest.ranges.size()>=2);
    REQUIRE(std::none_of(capture.manifest.ranges.begin(),capture.manifest.ranges.end(),[](const auto& r){return r.startSample>48000 && r.startSample<192000;}));
    armCaptureFixture(*p);pass(0,1,.2f,2);editor->refresh();
    REQUIRE(list->rowAt(0)->id==early && list->rowAt(1)->id==late);
    REQUIRE(m3::sameSavedSwell(protectedLate,*m3::find(engine.savedSession(),late)));

    // Shuffled serialized definitions and independent settings at equal arrival.
    auto saved=engine.savedSession();auto tied=*m3::find(saved,early);tied.id=juce::Uuid().toDashedString();tied.name="Custom tied target";
    tied.enabled=false;tied.muted=true;tied.solo=false;tied.placement=13;tied.shape.gainDb=-11;
    const auto tie=tied.id;saved.swells.push_back(tied);std::reverse(saved.swells.begin(),saved.swells.end());
    saved.selected={late,early};saved.primary=early;engine.restore(saved);editor->refresh();
    waitUI([&]{const auto s=engine.savedSession();return std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
    // Use one stable snapshot for iterator lifetime.
    saved=engine.savedSession();
    const auto renders=engine.snapshot().renderCount;for(int i=0;i<3;++i)editor->refresh();
    REQUIRE(engine.snapshot().renderCount==renders);
    const auto tieFirst=std::min(early,tie),tieSecond=std::max(early,tie);
    REQUIRE(list->rowAt(0)->id==tieFirst && list->rowAt(1)->id==tieSecond && list->rowAt(2)->id==late);
    juce::MemoryBlock state;p->getStateInformation(state);
    auto reopened=std::make_unique<PreverbAudioProcessor>(root.file);reopened->prepareToPlay(48000,4096);reopened->setPlayHead(&head);
    reopened->setStateInformation(state.getData(),static_cast<int>(state.getSize()));REQUIRE(reopened->flushCaptureForTesting());
    head.position.setIsPlaying(false);buffer.clear();reopened->processBlock(buffer,midi);
    waitUI([&]{const auto s=reopened->swellEngine().savedSession();return s.swells.size()==3 && std::all_of(s.swells.begin(),s.swells.end(),[](const auto& w){return w.phase==m3::Phase::ready;});});
    const auto restored=reopened->swellEngine().savedSession();REQUIRE(restored.primary==early && restored.selected==saved.selected);
    REQUIRE(restored.swells.size()==saved.swells.size());
    for(size_t i=0;i<saved.swells.size();++i){REQUIRE(restored.swells[i].id==saved.swells[i].id);REQUIRE(m3::sameSavedSwell(saved.swells[i],restored.swells[i]));}
    auto restoredEditor=std::make_unique<PreverbAudioProcessorEditor>(*reopened);restoredEditor->setVisible(true);restoredEditor->refresh();
    auto* restoredList=listFor(*restoredEditor);REQUIRE(restoredList->rowAt(0)->id==tieFirst && restoredList->rowAt(1)->id==tieSecond && restoredList->rowAt(2)->id==late);
    REQUIRE(restoredList->primaryId()==early && restoredList->isRowSelected(restoredList->rowFor(late)));
    for(const auto size:{juce::Point<int>{1100,850},juce::Point<int>{1400,900},juce::Point<int>{1400,1000},juce::Point<int>{1600,1000}})
    {restoredEditor->setSize(size.x,size.y);restoredEditor->refresh();snapshotEditor(*restoredEditor,"ordering-restored-"+juce::String(size.x)+"x"+juce::String(size.y));}
    if(review){restoredEditor->setSize(1100,850);controlReview::run(std::move(restoredEditor));}
}
