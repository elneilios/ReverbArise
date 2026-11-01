#include "AudioTestFiles.h"
#include <Core/ContentHash.h>
#include <DSP/SwellAssetStore.h>
#include <DSP/DefaultImpulse.h>
#include <Capture/CaptureAssetStore.h>
#include <wavpack.h>
#include <bit>
#include <thread>
#include <array>
using namespace m3;
static std::vector<char> packFloats(const std::vector<float>& samples)
{
    std::vector<char> output;
    const auto write=[](void* id,void* bytes,int32_t count)->int
    { auto& v=*static_cast<std::vector<char>*>(id); auto p=static_cast<char*>(bytes); v.insert(v.end(),p,p+count); return 1; };
    std::unique_ptr<WavpackContext,decltype(&WavpackCloseFile)> ctx(WavpackOpenFileOutput(write,&output,nullptr),&WavpackCloseFile);
    REQUIRE(ctx); WavpackConfig config{}; config.sample_rate=48000; config.num_channels=2; config.channel_mask=3;
    config.bits_per_sample=32; config.bytes_per_sample=4; config.float_norm_exp=127;
    REQUIRE(WavpackSetConfiguration64(ctx.get(),&config,static_cast<int64_t>(samples.size()/2),nullptr));
    REQUIRE(WavpackPackInit(ctx.get())); std::vector<int32_t> words;
    for(auto sample:samples) words.push_back(std::bit_cast<int32_t>(sample));
    REQUIRE(WavpackPackSamples(ctx.get(),words.data(),static_cast<uint32_t>(samples.size()/2)));
    REQUIRE(WavpackFlushSamples(ctx.get())); return output;
}
int main() { return runTest([]
{
    TestDirectory root; const auto managed=root.file.getChildFile("managed"); SwellAssetStore store(managed);
    const auto original=root.file.getChildFile(juce::String::fromUTF8("external-\xc3\xa9.wav"));
    const std::vector<float> samples{0,0,1.5f,-1.25f,0.25f,-0.5f,0,0};
    writeTestWav(original,2,48000,samples); const auto originalHash=fileHash(original);
    Asset ir; juce::String error; REQUIRE(store.importIR(original,previewMemoryBudget,ir,error));
    ImpulseResponse bundled;REQUIRE(bundledImpulse(bundled,error));
    REQUIRE(bundled.rate==48000 && bundled.channels==2 && bundled.frames()==86400);
    ImpulseResponse retainedBundle=bundled;const char corruptBytes[]{'b','a','d'};
    REQUIRE(!decodeDefaultImpulse(corruptBytes,sizeof(corruptBytes),retainedBundle,error) && retainedBundle.samples==bundled.samples);
    REQUIRE(!decodeDefaultImpulse(nullptr,0,retainedBundle,error));
    // A fresh cache is created concurrently by capture and default-IR publication.
    for(int pass=0;pass<20;++pass)
    {
        const auto shared=root.file.getChildFile("race-"+juce::String(pass)).getChildFile("project/instance");
        std::atomic<bool> start{false};std::array<bool,4> results{};std::vector<std::thread> writers;
        for(int worker=0;worker<4;++worker)writers.emplace_back([&,worker]
        {
            while(!start.load())std::this_thread::yield();juce::String localError;
            if(worker%2==0)
            {SwellAssetStore target(shared);ImpulseResponse tiny;tiny.channels=1;tiny.rate=48000;tiny.samples={1,.5f};Asset ref;results[worker]=target.writeIR(tiny,ref,localError);}
            else
            {CaptureAssetStore target(shared);CaptureManifestRange range;range.startSample=worker*2;range.frameCount=2;range.kind=CaptureCoverageKind::audio;
                range.assetPath="segments/"+juce::Uuid().toDashedString().toStdString()+".wav";float samples[]{.1f,.2f,.3f,.4f};results[worker]=target.writeAudioChunk(range,48000,samples,localError);}
        });
        start=true;for(auto& writer:writers)writer.join();for(bool result:results)REQUIRE(result);
    }
    REQUIRE(std::any_of(bundled.samples.begin(),bundled.samples.end(),[](float v){return v!=0;}));
    Asset defaultA,defaultB;REQUIRE(installBundledImpulse(store,defaultA,error));
    SwellAssetStore other(root.file.getChildFile("other"));REQUIRE(installBundledImpulse(other,defaultB,error));
    REQUIRE(defaultA.content==defaultB.content && store.verify(defaultA,error) && other.verify(defaultB,error));
    for(double rate:{1000.0,44100.0,96000.0})
    {ImpulseResponse prepared;REQUIRE(prepareImpulseResponse(bundled,rate,previewMemoryBudget,prepared,error));REQUIRE(prepared.frames()==static_cast<size_t>(std::llround(1.8*rate)));}
    REQUIRE(fileHash(original)==originalHash && ir.content.length()==64 && ir.path.startsWith("irs/"));
    ImpulseResponse decoded; REQUIRE(store.readIR(ir,previewMemoryBudget,decoded,error)); REQUIRE(decoded.samples==samples);
    const auto packed=packFloats(samples); const auto disguised=root.file.getChildFile("packed.wav");
    REQUIRE(disguised.replaceWithData(packed.data(),packed.size())); Asset packedIR;
    REQUIRE(store.importIR(disguised,previewMemoryBudget,packedIR,error)); REQUIRE(packedIR.content==ir.content);
    REQUIRE(store.readIR(packedIR,previewMemoryBudget,decoded,error) && decoded.samples==samples);
    REQUIRE(original.moveFileTo(root.file.getChildFile("moved.wav")));
    REQUIRE(store.readIR(ir,previewMemoryBudget,decoded,error) && decoded.samples==samples);
    ImpulseResponse mono; mono.channels=1; mono.rate=44100; mono.samples={0,0.5f,-0.25f}; Asset monoRef;
    REQUIRE(store.writeIR(mono,monoRef,error) && store.readIR(monoRef,previewMemoryBudget,decoded,error));
    REQUIRE(decoded.samples==mono.samples && decoded.channels==1 && decoded.rate==44100);
    auto bad=ir; bad.path="../outside.wav"; REQUIRE(!store.verify(bad,error));
    bad=ir; bad.frames++; REQUIRE(!store.verify(bad,error));
    REQUIRE(!store.readIR(ir,1,decoded,error));
    WetRender wet; wet.rate=48000; wet.start=-3; wet.sourceStart=0; wet.sourceEnd=4; wet.samples={0,0,1,-1,2,-2,0.1f,-0.1f,0,0,0,0,0,0};
    SongTime time{true,0,0,48000,120,4,4}; Asset render;
    REQUIRE(store.writeRender(wet,ir,"source-hash",time,render,error));
    auto wrongBounds=wet;wrongBounds.sourceEnd++;Asset rejected;
    REQUIRE(!store.writeRender(wrongBounds,ir,"bad",time,rejected,error) && !rejected.present());
    WetRender loaded; REQUIRE(store.readRender(render,previewMemoryBudget,loaded,error));
    REQUIRE(loaded.samples==wet.samples && loaded.start==-3 && loaded.sourceEnd==4 && loaded.peak==2);
    const auto previous=render; wet.samples[2]=3; Asset next;
    REQUIRE(store.writeRender(wet,ir,"new-source",time,next,error));
    REQUIRE(next.path!=render.path && store.readRender(previous,previewMemoryBudget,loaded,error) && loaded.samples[2]==1);
    // Model deletion/Clear never invoke asset deletion. Older references remain usable.
    Session s; Swell w; w.ir=ir; w.render=render; s.swells.push_back(w); REQUIRE(erase(s,w.id));
    REQUIRE(store.verify(ir,error) && store.verify(render,error));
    const auto metadata=managed.getChildFile(next.path).withFileExtension(".json");
    REQUIRE(metadata.deleteFile()); REQUIRE(!store.verify(next,error)); // Interrupted asset pair is never current.
    REQUIRE(store.readRender(previous,previewMemoryBudget,loaded,error));
    REQUIRE(managed.getChildFile(next.path).replaceWithText("corrupt"));
    REQUIRE(!store.readRender(next,previewMemoryBudget,loaded,error));
    const auto blocked=root.file.getChildFile("blocked"); REQUIRE(blocked.createDirectory().wasOk());
    REQUIRE(blocked.getChildFile("renders").replaceWithText("blocked directory")); SwellAssetStore failed(blocked);
    auto retained=previous; REQUIRE(!failed.writeRender(wet,ir,"fail",time,retained,error)); REQUIRE(retained.path==previous.path);
    REQUIRE(store.readRender(previous,previewMemoryBudget,loaded,error));
    Asset corrupt=ir; const auto irFile=managed.getChildFile(ir.path); REQUIRE(irFile.appendText("damage"));
    REQUIRE(!store.verify(corrupt,error)); REQUIRE(store.verify(packedIR,error));
    REQUIRE(!safeAssetPath("irs/../../external.wav") && !safeAssetPath("C:/audio.wav") && !safeAssetPath("irs\\a.wav"));
}); }
