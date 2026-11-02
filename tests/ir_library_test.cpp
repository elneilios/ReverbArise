#include "AudioTestFiles.h"
#include <DSP/DefaultImpulse.h>
#include <Core/ContentHash.h>
#include <Windows.h>
#include <psapi.h>
#include <chrono>
#include <set>
using namespace m3;
static double now(){return juce::Time::getMillisecondCounterHiRes();}
int main(int argc,char** argv){return runTest([&]
{
    TestDirectory directory;SwellAssetStore store(directory.file);juce::String error;juce::Array<juce::var> rows;
    REQUIRE(bundledImpulses().size()==6);std::set<juce::String> ids;
    for(const auto& entry:bundledImpulses()){
        REQUIRE(ids.insert(entry.id).second && entry.revision==1 && juce::String(entry.license)=="CC0-1.0");
        const auto decodeStart=now();ImpulseResponse original;REQUIRE(decodeBundledImpulse(entry,previewMemoryBudget,original,error));
        const auto decodeMs=now()-decodeStart;REQUIRE(sampleHash(original.samples,original.rate,original.channels)==entry.decodedHash);
        Asset asset,repeated;const auto firstStart=now();REQUIRE(installBundledImpulse(store,entry.id,previewMemoryBudget,asset,error));const auto firstMs=now()-firstStart;
        const auto repeatStart=now();REQUIRE(installBundledImpulse(store,entry.id,previewMemoryBudget,repeated,error) && asset==repeated);const auto repeatMs=now()-repeatStart;
        ImpulseResponse restored;REQUIRE(store.readIR(asset,previewMemoryBudget,restored,error));REQUIRE(restored.samples==original.samples);
        // Managed content survives a changed display name/reordered catalogue view.
        auto renamed=asset;renamed.path=asset.path;REQUIRE(store.verify(renamed,error));
        for(double rate:{44100.,48000.,96000.}){
            const auto prepareStart=now();ImpulseResponse prepared;
            REQUIRE(prepareImpulseResponse(original,rate,previewMemoryBudget,prepared,error));const auto prepareMs=now()-prepareStart;
            REQUIRE(prepared.frames()==static_cast<size_t>(std::llround(entry.frames*rate/entry.rate)));
            std::vector<float> source(64,0);source[4]=.25f;source[5]=-.125f;source[22]=-.0625f;source[23]=.1875f;
            const auto overhead=original.samples.capacity()*sizeof(float);uint64_t estimated=0;
            REQUIRE(estimateRenderMemory(32,prepared.frames(),overhead,estimated,error));
            const auto renderStart=now();WetRender wet;REQUIRE(renderReverseConvolution(source,123,rate,prepared,overhead,wet,error));
            const auto renderMs=now()-renderStart;
            REQUIRE(wet.frames()==32+prepared.frames()-1 && wet.start==123-static_cast<int64_t>(prepared.frames())+1);
            // Sparse source gives an independent O(M) direct-convolution oracle.
            for(size_t n=0;n<wet.frames();++n)for(int c=0;c<2;++c){double expected=0;
                for(size_t at:{size_t{2},size_t{11}})if(n>=at && n-at<prepared.frames())expected+=source[at*2+c]*prepared.samples[(prepared.frames()-1-(n-at))*2+c];
                REQUIRE(std::isfinite(wet.samples[n*2+c]) && std::abs(wet.samples[n*2+c]-expected)<2e-5);}
            PROCESS_MEMORY_COUNTERS memory{};memory.cb=sizeof(memory);REQUIRE(GetProcessMemoryInfo(GetCurrentProcess(),&memory,sizeof(memory)));
            auto row=juce::var(new juce::DynamicObject);auto* fields=row.getDynamicObject();
            fields->setProperty("id",entry.id);fields->setProperty("rate",rate);fields->setProperty("decode_ms",decodeMs);
            fields->setProperty("prepare_ms",prepareMs);fields->setProperty("render_ms",renderMs);
            fields->setProperty("first_install_ms",firstMs);fields->setProperty("repeat_install_ms",repeatMs);
            fields->setProperty("estimated_bytes",static_cast<juce::int64>(estimated));fields->setProperty("process_cumulative_peak_working_set_bytes",static_cast<juce::int64>(memory.PeakWorkingSetSize));
            fields->setProperty("managed_ir_bytes",static_cast<juce::int64>(store.directory().getChildFile(asset.path).getSize()));rows.add(row);
        }
    }
    ImpulseResponse mono;mono.rate=48000;mono.channels=1;mono.samples={.5f,0,-.25f};
    std::vector<float> asymmetric{1,-.5f,0,0,.25f,.75f};WetRender wet;
    REQUIRE(renderReverseConvolution(asymmetric,0,48000,mono,0,wet,error));
    for(size_t n=0;n<wet.frames();++n)for(int c=0;c<2;++c){double expected=0;
        for(size_t j=0;j<3;++j)if(n>=j && n-j<3)expected+=asymmetric[j*2+c]*mono.samples[2-(n-j)];REQUIRE(std::abs(expected-wet.samples[n*2+c])<1e-6);}
    uint64_t rejectedBytes=0;REQUIRE(!estimateRenderMemory(48000*60,96000*12,previewMemoryBudget,rejectedBytes,error));
    if(argc>1)REQUIRE(juce::File(juce::String::fromUTF8(argv[1])).replaceWithText(juce::JSON::toString(rows)));
});}
