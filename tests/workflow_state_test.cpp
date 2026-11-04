#include "TestSupport.h"
#include "AllocationGuard.h"
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
                REQUIRE(shown.selected==mode);
                REQUIRE(std::string_view(shown.status).empty()==(mode==Mode::live && phase==Phase::idle));
                for(const auto& choice:shown.choices)
                {
                    REQUIRE(choice.available==(entryBlock(snapshot,choice.mode,readiness)==EntryBlock::none));
                    REQUIRE(choice.available==std::string_view(choice.reason).empty());
                }
            }
    REQUIRE(entryBlock({},Mode::live,{})==EntryBlock::source);
    REQUIRE(entryBlock({},Mode::live,{true,false,true})==EntryBlock::timing);
    REQUIRE(entryBlock({},Mode::finalise,{true,true,false})==EntryBlock::wet);
    REQUIRE(entryBlock({},Mode::live,{false,true,true,true})==EntryBlock::none);
    REQUIRE(entryBlock({},Mode::finalise,{false,true,true,true})==EntryBlock::none);
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
            const bool expected=target==Mode::capture || (timing && (target==Mode::finalise?wet:source));
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
    REQUIRE(state.complete(token,true,true,true));REQUIRE(state.snapshot().mode==Mode::capture && !state.snapshot().recapture);
    REQUIRE(state.requestMode(Mode::live,{true,true,true}));
    REQUIRE(!state.complete(token,true,true,true));REQUIRE(state.requestMode(Mode::finalise,{true,true,true}));
    REQUIRE(!state.begin(true));REQUIRE(!capabilities(state.snapshot(),{true,true,true}).scheduledWet);
    // Restored saved mode can remain selected while readiness is unavailable.
    state.reset(Mode::live);REQUIRE(!capabilities(state.snapshot(),{}).edit && state.snapshot().mode==Mode::live);
    state.reset();REQUIRE(state.requestMode(Mode::live,{false,true,true,true}));
    REQUIRE(capabilities(state.snapshot(),{false,true,true,true}).edit);
    state.reset();REQUIRE(!state.requestMode(Mode::live,{false,false,true,true}));
    REQUIRE(state.requestMode(Mode::finalise,{false,true,true,true}));
    REQUIRE(capabilities(state.snapshot(),{false,true,true,true}).preview && !capabilities(state.snapshot(),{false,true,true,true}).capture);
    for(auto outcome:{Result::empty,Result::failed}){
        state.reset();token=state.begin(true);REQUIRE(state.stop(token));
        REQUIRE(state.complete(token,outcome!=Result::failed,false,false));
        REQUIRE(state.snapshot().mode==Mode::capture && state.snapshot().result==outcome);
    }
    state.reset();token=state.begin(true);REQUIRE(state.stop(token));state.reset();
    REQUIRE(!state.complete(token,true,true,true));REQUIRE(state.snapshot().mode==Mode::capture);
    State neighbour;state.arm(true);token=state.begin(true);state.cancel();
    REQUIRE(!state.complete(token,true,true,true));REQUIRE(!state.snapshot().recapture && neighbour.snapshot().mode==Mode::capture);
    // Reservation is the definition/mode linearization point. Losing admission
    // never invokes the publisher, including a settings-slot or pass change.
    state.reset();auto generation=state.snapshot();bool published=false;
    REQUIRE(state.publishConfiguration(1,generation.pass));
    REQUIRE(!state.publishGenerated(generation,[&]()noexcept{published=true;}) && !published);
    generation=state.snapshot();const auto recording=state.begin(true);
    REQUIRE(recording && !state.publishGenerated(generation,[&]()noexcept{published=true;}) && !published);
    state.cancel();generation=state.snapshot();
    bool locked=false,playRejected=false,navigationRejected=false,settingsRejected=false;
    REQUIRE(state.publishGenerated(generation,[&]()noexcept
    {
        locked=state.snapshot().generationCommit && state.snapshot().locked();
        {AudioAllocationGuard guard;playRejected=!state.begin(true);}
        navigationRejected=!state.requestMode(Mode::finalise,{true,true,true});
        settingsRejected=!state.publishConfiguration(1,generation.pass);
        published=true;
    }));
    REQUIRE(published && locked && playRejected && navigationRejected && settingsRejected);
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && state.snapshot().mode==Mode::live && !state.snapshot().generationCommit);
    REQUIRE(!state.publishGenerated(generation,[]()noexcept{}));
    for(int iteration=0;iteration<200;++iteration)
    {
        state.reset();generation=state.snapshot();std::atomic<bool> start{false},committed{false};uint64_t admitted=0;bool generated=false;
        std::thread audio([&]{while(!start.load())std::this_thread::yield();admitted=state.begin(true);});
        std::thread worker([&]{while(!start.load())std::this_thread::yield();generated=state.publishGenerated(generation,[&]()noexcept{committed=true;});});
        start=true;audio.join();worker.join();
        REQUIRE(generated!=(admitted!=0) && committed.load()==generated);
        REQUIRE(state.snapshot().mode==(generated?Mode::live:Mode::capture));
        REQUIRE(state.snapshot().phase==(generated?Phase::idle:Phase::recording));
    }
    state.reset();generation=state.snapshot();std::atomic<bool> entered{false},release{false},resetStarted{false},resetDone{false};
    bool generated=false;
    std::thread worker([&]{generated=state.publishGenerated(generation,[&]()noexcept
        {entered=true;while(!release.load())std::this_thread::yield();});});
    const auto deadline=juce::Time::getMillisecondCounter()+5000;
    while(!entered.load() && juce::Time::getMillisecondCounter()<deadline)std::this_thread::yield();
    std::thread restore([&]{resetStarted=true;state.reset(Mode::finalise);resetDone=true;});
    while(!resetStarted.load())std::this_thread::yield();
    const auto held=entered.load() && !resetDone.load();release=true;worker.join();restore.join();
    REQUIRE(held && generated && resetDone && state.snapshot().mode==Mode::finalise && !state.snapshot().generationCommit);
});}
