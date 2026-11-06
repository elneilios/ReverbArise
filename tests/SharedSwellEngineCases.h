#pragma once
#include "LearnGenerationSupport.h"
#include "AudioTestFiles.h"
#include <DSP/ReverseConvolution.h>
#include <Core/ContentHash.h>

#include "RenderPolicyMatrix.h"

static void commonEngineCases()
{
    GenerationFixture f;auto& p=*f.processor;auto& e=f.engine();
    e.setLearningAutomatic(false);e.setAutoMarkers(false);e.setAutoGenerateOnStop(false);e.setPreferences(0,.5f,.08,false);
    const auto ir=f.root.file.getChildFile("short.wav");writeTestWav(ir,1,48000,{1,.2f});e.assignDefaultIR(ir);
    await([&]{return e.savedSession().defaultIRName=="short.wav";});
    f.record();REQUIRE(e.savedSession().swells.empty() && e.snapshot().renderCount==0);
    REQUIRE(p.requestWorkflowMode(workflow::Mode::live));REQUIRE(e.addTarget(4800));const auto first=e.savedSession().primary;
    REQUIRE(e.addTarget(148800));const auto second=e.savedSession().primary;
    await([&]{const auto s=e.savedSession();return find(s,first)->phase==Phase::ready && find(s,second)->phase==Phase::ready;});
    const auto initial=e.savedSession();const auto oldFirst=find(initial,first)->render,oldSecond=find(initial,second)->render;const auto count=e.snapshot().renderCount;
    const auto sourceHash=fileHash(e.assetDirectory().getChildFile(oldFirst.path));
    REQUIRE(p.requestWorkflowMode(workflow::Mode::capture));
    const auto pass=[&](float scale,bool replace){REQUIRE(p.setRecaptureExisting(replace));f.tick(0,false);for(int i=0;i<8;++i){f.tick(i*4096,true,scale);juce::Thread::sleep(1);}f.tick(32768,false);REQUIRE(p.flushCaptureForTesting());};
    const auto acceptedReference=p.getCaptureSnapshot().reference;p.holdCaptureValidationForTesting(true);
    REQUIRE(p.setRecaptureExisting(true));f.tick(0,false);for(int i=0;i<8;++i){f.tick(i*4096,true,.4f);juce::Thread::sleep(1);}f.tick(32768,false);
    await([&]{return p.getWorkflowSnapshot().phase==workflow::Phase::validating;});juce::Thread::sleep(30);
    REQUIRE(p.getCaptureSnapshot().reference==acceptedReference && e.snapshot().renderCount==count && find(e.savedSession(),first)->render==oldFirst);
    p.holdCaptureValidationForTesting(false);REQUIRE(p.flushCaptureForTesting());
    await([&]{const auto s=e.savedSession();return find(s,first)->phase==Phase::ready && e.snapshot().renderCount==count+1;});
    auto changed=e.savedSession();REQUIRE(p.getWorkflowSnapshot().mode==workflow::Mode::capture);
    REQUIRE(changed.swells.size()==initial.swells.size() && find(changed,first)->render!=oldFirst && find(changed,second)->render==oldSecond);
    REQUIRE(!changed.automaticProcessing && !changed.autoMarkersEnabled && !changed.autoGenerateOnStop && !changed.autoRegenerate);
    REQUIRE(fileHash(e.assetDirectory().getChildFile(oldFirst.path))==sourceHash);
    const auto accepted=find(changed,first)->render;const auto work=renderWorkCounts();
    pass(.4f,true);juce::Thread::sleep(150);
    REQUIRE(e.snapshot().renderCount==count+1 && find(e.savedSession(),first)->render==accepted);
    REQUIRE(renderWorkCounts().convolutions==work.convolutions && renderWorkCounts().resamplings==work.resamplings);
    REQUIRE(p.requestWorkflowMode(workflow::Mode::live));REQUIRE(e.selectTargets({first},first));
    const auto shapeWork=renderWorkCounts();
    REQUIRE(e.edit(first,[](auto& w,auto&){w.name="Preserved edit";w.shape.gainDb=-3;w.shape.fadeIn=16;w.placement+=100;return true;}));
    juce::Thread::sleep(100);REQUIRE(find(e.savedSession(),first)->render==accepted);
    REQUIRE(renderWorkCounts().convolutions==shapeWork.convolutions && renderWorkCounts().resamplings==shapeWork.resamplings);
    const auto replacement=f.root.file.getChildFile("replacement.wav");writeTestWav(replacement,1,48000,{.5f,.1f});
    auto before=e.savedSession();REQUIRE(e.assignIR(first,find(before,first)->inputGeneration,replacement,false,e.assignmentEpoch()));
    await([&]{const auto s=e.savedSession();return find(s,first)->phase==Phase::ready && e.snapshot().renderCount==count+2;});
    REQUIRE(find(e.savedSession(),second)->render==oldSecond);
    const auto future=f.root.file.getChildFile("future.wav");writeTestWav(future,1,48000,{.25f,.05f});
    const auto material=find(e.savedSession(),first)->render;e.assignDefaultIR(future);
    await([&]{return e.savedSession().defaultIRName=="future.wav";});juce::Thread::sleep(50);
    REQUIRE(find(e.savedSession(),first)->render==material && e.snapshot().renderCount==count+2);
    REQUIRE(e.setFrozen(second,true));const auto frozenHash=fileHash(e.assetDirectory().getChildFile(oldSecond.path));
    f.host.position.setBpm(100);f.tick(0,false);
    await([&]{const auto s=e.savedSession();return find(s,first)->phase==Phase::ready && find(s,first)->render.bpm==100;});
    REQUIRE(find(e.savedSession(),second)->render==oldSecond && find(e.savedSession(),second)->frozen && fileHash(e.assetDirectory().getChildFile(oldSecond.path))==frozenHash);
    const auto saved=e.savedSession();const auto renders=e.snapshot().renderCount;
    e.restore(saved,p.getCaptureSnapshot().reference);
    await([&]{return find(e.savedSession(),first)->phase==Phase::ready;});REQUIRE(e.snapshot().renderCount==renders);
    const auto path=e.assetDirectory().getChildFile(find(saved,first)->render.path),offline=path.getSiblingFile("saved-wet-offline.wav");
    await([&]{return path.moveFileTo(offline);});e.restore(saved,p.getCaptureSnapshot().reference);
    await([&]{return e.snapshot().renderCount==renders+1 && find(e.savedSession(),first)->phase==Phase::ready;});
    REQUIRE(offline.existsAsFile() && fileHash(e.assetDirectory().getChildFile(oldSecond.path))==frozenHash);
    renderPolicyMatrix();
    std::cout<<"Shared engine: no-marker/no-Stop-generation source refresh in Learn, selective/identical inputs, shape reuse, assigned/future IR isolation, timing, Freeze, exact reuse and ordinary cache recovery passed.\n";
}
