#pragma once
static void renderPolicyMatrix()
{
    for(bool replace:{false,true})for(bool detect:{false,true})for(bool stop:{false,true}){
        GenerationFixture f;auto& p=*f.processor;auto& e=f.engine();e.setLearningAutomatic(false);e.setAutoMarkers(false);e.setAutoGenerateOnStop(false);
        const auto ir=f.root.file.getChildFile("matrix.wav");writeTestWav(ir,1,48000,{1,.1f});e.assignDefaultIR(ir);await([&]{return e.savedSession().defaultIRName=="matrix.wav";});f.record();
        REQUIRE(p.requestWorkflowMode(workflow::Mode::live));REQUIRE(e.addTarget(4800));const auto id=e.savedSession().primary;
        await([&]{return find(e.savedSession(),id)->phase==Phase::ready;});const auto previous=e.savedSession();const auto count=e.snapshot().renderCount;
        REQUIRE(p.requestWorkflowMode(workflow::Mode::capture));e.setLearningAutomatic(detect);e.setAutoMarkers(detect);REQUIRE(e.setAutoGenerateOnStop(stop));REQUIRE(p.setRecaptureExisting(replace) && f.arm());
        const auto analysis=e.snapshot().analysisCount;f.tick(0,false);for(int i=0;i<8;++i){f.tick(i*4096,true,.4f);juce::Thread::sleep(1);}f.tick(32768,false);REQUIRE(p.flushCaptureForTesting());
        const bool expectAutomaticBatch=replace && detect && stop;
        try{await([&]{
            const auto snapshot=e.snapshot();
            const auto* retained=find(snapshot.session,id);
            const auto automaticCount=std::count_if(snapshot.session.swells.begin(),snapshot.session.swells.end(),[](const auto& swell){
                return swell.provenance==Provenance::automatic && swell.phase==Phase::ready && swell.render.present();
            });
            return retained && retained->phase==Phase::ready
                && (automaticCount>0)==expectAutomaticBatch
                && snapshot.session.swells.size()==1+static_cast<size_t>(automaticCount)
                && snapshot.renderCount==count+(replace?1:0)+automaticCount;
        });}
        catch(...){const auto s=e.snapshot();const auto w=find(s.session,id);
            std::cerr<<"Recapture="<<replace<<" Detect="<<detect<<" Generate on Stop="<<stop
                <<" | "<<(w?w->status:juce::String("Missing swell"))<<" | renders "<<count<<" -> "<<s.renderCount<<" | "<<s.message<<"\n";throw;}
        const auto s=e.savedSession();const auto w=find(s,id);REQUIRE(w->shape==find(previous,id)->shape && w->name==find(previous,id)->name && w->ir==find(previous,id)->ir);
        REQUIRE(s.automaticProcessing==detect && s.autoMarkersEnabled==detect && s.autoGenerateOnStop==stop);
        REQUIRE(replace?w->render!=find(previous,id)->render:w->render==find(previous,id)->render);
        if(!detect)REQUIRE(s.swells.size()==1 && e.snapshot().analysisCount==analysis);
    }
    std::cout<<"All eight Recapture/Detect/Generate-on-Stop combinations preserve capture permission, existing-render maintenance and saved policy.\n";
}
