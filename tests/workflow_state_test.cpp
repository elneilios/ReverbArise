#include "TestSupport.h"
#include <Core/WorkflowState.h>
#include <Core/SwellSession.h>
#include <UI/WorkflowPresentation.h>
#include <string_view>

int main(){return runTest([]{
    using namespace workflow;
    REQUIRE(std::string_view(m3::ui::modeLabel(Mode::capture))=="Learn");
    REQUIRE(std::string_view(m3::ui::modeLabel(Mode::live))=="Perform");
    REQUIRE(std::string_view(m3::ui::modeLabel(Mode::finalise))=="Export");
    REQUIRE(std::string_view(modeName(Mode::capture))=="Capture" && std::string_view(modeName(Mode::live))=="Live" && std::string_view(modeName(Mode::finalise))=="Finalise");
    for(const auto mode:{Mode::capture,Mode::live,Mode::finalise})
        for(const auto phase:{Phase::idle,Phase::recording,Phase::draining,Phase::validating,Phase::failed})
            for(unsigned bits=0;bits<16;++bits)
            {
                const Snapshot snapshot{mode,phase};const Readiness readiness{bool(bits&1),bool(bits&2),bool(bits&4),bool(bits&8)};
                const auto shown=m3::ui::presentWorkflow(snapshot,readiness);
                REQUIRE(shown.selected==mode && std::string_view(shown.status).size()>0);
                for(const auto& choice:shown.choices)
                {
                    REQUIRE(choice.available==(entryError(snapshot,choice.mode,readiness)==nullptr));
                    REQUIRE(choice.available==std::string_view(choice.reason).empty());
                }
            }
    REQUIRE(entryBlock({},Mode::live,{})==EntryBlock::source);
    REQUIRE(entryBlock({},Mode::live,{true,false,true})==EntryBlock::timing);
    REQUIRE(entryBlock({},Mode::finalise,{true,true,false})==EntryBlock::wet);
    REQUIRE(entryBlock({},Mode::live,{false,true,true,true})==EntryBlock::none);
    REQUIRE(entryBlock({},Mode::finalise,{false,true,true,true})==EntryBlock::source);
    REQUIRE(std::string_view(m3::ui::workflowStatus({Mode::capture,Phase::recording})).find("Recording")!=std::string_view::npos);
    REQUIRE(std::string_view(m3::ui::workflowStatus({Mode::capture,Phase::draining})).find("Committing")!=std::string_view::npos);
    REQUIRE(std::string_view(m3::ui::workflowStatus({Mode::capture,Phase::validating})).find("Validating")!=std::string_view::npos);
    REQUIRE(std::string_view(m3::ui::workflowStatus({Mode::capture,Phase::failed})).find("retained")!=std::string_view::npos);
    m3::Swell rendered;rendered.phase=m3::Phase::ready;rendered.render.path="renders/verified.wav";
    rendered.render.frames=100;rendered.render.rate=48000;rendered.render.bpm=120;
    SongTime current{true,0,0,48000,120,4,4};
    REQUIRE(m3::hasCompatibleReadyRender(rendered,current));
    for(int mismatch=0;mismatch<7;++mismatch){auto asset=rendered;auto time=current;
        if(mismatch==0)asset.render.rate=44100;if(mismatch==1)asset.render.bpm=121;
        if(mismatch==2)asset.render.numerator=3;if(mismatch==3)asset.render.denominator=8;
        if(mismatch==4)asset.render.frames=0;if(mismatch==5)asset.phase=m3::Phase::checking;if(mismatch==6)time.valid=false;
        REQUIRE(!m3::hasCompatibleReadyRender(asset,time));}
    State state;REQUIRE(state.snapshot().mode==Mode::capture && !state.snapshot().recapture);
    // Independent eligibility table: source, timing and wet readiness vary
    // independently; queued wet never blocks Live.
    for(int source=0;source<2;++source)for(int timing=0;timing<2;++timing)for(int wet=0;wet<2;++wet)
        for(auto target:{Mode::capture,Mode::live,Mode::finalise})
        {
            state.reset();Readiness r{source!=0,timing!=0,wet!=0};
            const bool expected=target==Mode::capture || (source && timing && (target!=Mode::finalise || wet));
            REQUIRE(state.requestMode(target,r)==expected);
            const auto c=capabilities(state.snapshot(),r);
            REQUIRE(c.edit==(expected && target==Mode::live && source && timing));
            REQUIRE(c.scheduledWet==c.edit);
        }
    state.reset();REQUIRE(!state.begin(false));REQUIRE(state.arm(true));
    auto token=state.begin(true);REQUIRE(token && state.snapshot().locked());
    for(auto target:{Mode::capture,Mode::live,Mode::finalise})REQUIRE(!state.requestMode(target,{true,true,true}));
    REQUIRE(!state.arm(false) && !state.begin(true));REQUIRE(state.stop(token));REQUIRE(!state.stop(token));
    REQUIRE(state.validating(token));REQUIRE(!state.requestMode(Mode::live,{true,true,true}));
    REQUIRE(state.complete(token,true,true,true));REQUIRE(state.snapshot().mode==Mode::live && !state.snapshot().recapture);
    REQUIRE(!state.complete(token,true,true,true));REQUIRE(state.requestMode(Mode::finalise,{true,true,true}));
    REQUIRE(!state.begin(true));REQUIRE(!capabilities(state.snapshot(),{true,true,true}).scheduledWet);
    // Restored saved mode can remain selected while readiness is unavailable.
    state.reset(Mode::live);REQUIRE(!capabilities(state.snapshot(),{}).edit && state.snapshot().mode==Mode::live);
    state.reset();REQUIRE(state.requestMode(Mode::live,{false,true,true,true}));
    REQUIRE(capabilities(state.snapshot(),{false,true,true,true}).edit);
    state.reset();REQUIRE(!state.requestMode(Mode::live,{false,false,true,true}));
    REQUIRE(!state.requestMode(Mode::finalise,{false,true,true,true}));
    for(auto outcome:{Result::empty,Result::failed}){
        state.reset();token=state.begin(true);REQUIRE(state.stop(token));
        REQUIRE(state.complete(token,outcome!=Result::failed,false,false));
        REQUIRE(state.snapshot().mode==Mode::capture && state.snapshot().result==outcome);
    }
    state.reset();token=state.begin(true);REQUIRE(state.stop(token));state.reset();
    REQUIRE(!state.complete(token,true,true,true));REQUIRE(state.snapshot().mode==Mode::capture);
    State neighbour;state.arm(true);token=state.begin(true);state.cancel();
    REQUIRE(!state.complete(token,true,true,true));REQUIRE(!state.snapshot().recapture && neighbour.snapshot().mode==Mode::capture);
});}
