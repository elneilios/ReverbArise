#include "SongMix.h"
#include <Playback/LoopCycles.h>
#include "SwellShape.h"
#include <Core/MonitoringSafety.h>
#include <algorithm>
namespace m3
{
namespace {constexpr uint64_t readerAllowance=16384,fixedPages=3*exportPageFrames*2*sizeof(float);constexpr size_t maximumPeaks=4096;}
uint64_t SongMix::workingBytes()const noexcept
{return fixedPages+dry.workingBytes()+readers.size()*readerAllowance+occupied.capacity()*sizeof(occupied[0])
    +(measured.dryPeaks.capacity()+measured.wetPeaks.capacity())*sizeof(CapturePeak);}
bool SongMix::prepare(std::shared_ptr<const SongExportPlan> input,uint64_t available,juce::String& error,const RenderCancellation& cancel)
{
    ready=false;layersReady=false;current.reset();readers.clear();occupied.clear();measured={};
    const auto reserve=fixedPages+maximumPeaks*2*sizeof(CapturePeak)+(input?input->members.size():0)*readerAllowance;
    if(!input || available<reserve){error="Export reader working budget exceeded";return false;}
    if(!std::isfinite(input->loopWetGain) || input->loopWetGain<=0)
    {error="Invalid Perform wet gain";return false;}
    if(!dry.prepare(input,available-reserve,error,cancel))return false;
    auto verified=std::make_shared<SongExportPlan>(*input);verified->warnings=dry.warnings();verified->identity=songExportIdentity(*verified);
    if(verified->identity.isEmpty()){error="Invalid verified Export composition";return false;}current=std::move(verified);
    SwellAssetStore store(directory);juce::WavAudioFormat format;
    for(const auto& member:current->members)
    {
        if(cancel && cancel()){error="Export preparation superseded";return false;}
        std::unique_ptr<juce::AudioFormatReader> reader;
        if(current->wet>0){
            const auto& a=member.accepted.render;if(!store.verify(a,error)){error=member.accepted.id+": "+error;return false;}
            auto input=directory.getChildFile(a.path).createInputStream();if(!input){error="Export wet is missing or unreadable: "+member.accepted.id;return false;}
            reader.reset(format.createReaderFor(input.release(),true));
            if(!reader || reader->numChannels!=2 || reader->bitsPerSample!=32 || !reader->usesFloatingPointData
                || reader->sampleRate!=a.rate || reader->lengthInSamples!=a.frames){error="Export wet header incompatible: "+member.accepted.id;return false;}
            occupied.emplace_back(std::max(member.start,current->origin),std::min(member.end,current->end));
        }
        readers.push_back(std::move(reader));
    }
    if(current->dry>0)for(const auto& r:current->source.ranges)if(r.kind==CaptureCoverageKind::audio)
        occupied.emplace_back(std::max(r.startSample,current->origin),std::min(r.startSample+static_cast<int64_t>(r.frameCount),current->end));
    if(current->loop){occupied.clear();occupied.emplace_back(current->origin,current->end);}
    std::sort(occupied.begin(),occupied.end());std::vector<std::pair<int64_t,int64_t>> merged;
    for(const auto& range:occupied){if(range.second<=range.first)continue;
        if(!merged.empty() && range.first<=merged.back().second)merged.back().second=std::max(merged.back().second,range.second);else merged.push_back(range);}
    occupied=std::move(merged);layersReady=true;error.clear();return true;
}
bool SongMix::branches(int64_t at,std::span<float> source,std::span<float> wet,juce::String& error)
{
    int64_t end;
    if(!current || source.size()!=wet.size() || source.empty() || source.size()%2 || source.size()/2>exportPageFrames
        || !checkedAdd(at,static_cast<int64_t>(source.size()/2),end) || at<current->origin || end>current->end)
    {error="Invalid Export mixing page";return false;}
    if(!dry.read(at,source,error))return false;std::fill(wet.begin(),wet.end(),0.f);
    if(current->wet==0)return true;
    for(size_t i=0;i<current->members.size();++i){const auto& member=current->members[i];const auto& w=member.accepted;
        bool success=true;
        const auto contribution=[&](int64_t offset){
        if(!success)return;
        const auto a=std::max(at,member.start+offset),b=std::min(end,member.end+offset);if(a>=b)return;
        const auto count=static_cast<int>(b-a);const auto raw=a-offset-w.placement-w.render.start;
        if(!readers[i]->read(&scratch,0,count,raw,true,true)){error="Export wet read failed: "+w.id;success=false;return;}
        for(int n=0;n<count;++n){const auto pos=a+n;
            const auto local=pos-offset;
            const auto gain=swellEnvelope(w,local,member.start,member.end)*monitorEdgeGain(static_cast<size_t>(local-member.start),static_cast<size_t>(member.end-member.start),current->timing.rate,.002);
            for(int c=0;c<2;++c){auto& sum=wet[static_cast<size_t>(pos-at)*2+c];sum+=static_cast<float>(scratch.getSample(c,n)*gain);
                if(!std::isfinite(sum)){error="Non-finite Export wet sum";success=false;return;}}}
        };
        if(current->loop){
            const auto period=current->source.loop->endSample;
            const auto past=std::max(int64_t{0},member.end)/period+1;
            const auto future=std::max(int64_t{0},-member.start)/period+1+(end-at)/period+1;
            visitLoopCycles(loopFloorCycle(at,period),0,current->loop->repeats-1,past,future,[&](int64_t cycle){
                if(const auto offset=loopCycleAnchor(0,cycle,period))contribution(*offset);
                else {error="Loop wet extent overflows";success=false;}
            });
        }else contribution(0);
        if(!success)return false;
    }
    if(current->loop && current->loopWetGain!=1)
        for(auto& sample:wet)sample*=current->loopWetGain;
    return true;
}
bool SongMix::scan(const std::function<bool(int64_t,int)>& visit,juce::String& error,const RenderCancellation& cancel)
{
    for(const auto& span:occupied)for(auto at=span.first;at<span.second;){
        if(cancel && cancel()){error="Export measurement superseded";return false;}
        const auto frames=static_cast<int>(std::min(int64_t{exportPageFrames},span.second-at));
        if(!branches(at,{dryPage.data(),static_cast<size_t>(frames)*2},{wetPage.data(),static_cast<size_t>(frames)*2},error) || !visit(at,frames))return false;
        at+=frames;
    }return true;
}
bool SongMix::measure(juce::String& error,const RenderCancellation& cancel)
{
    ready=false;measured={};if(!current){error="No Export composition";return false;}
    if(!scan([&](int64_t,int frames){for(int n=0;n<frames*2;++n){measured.wetPeak=std::max(measured.wetPeak,std::abs(wetPage[n]));measured.dryPeak=std::max(measured.dryPeak,std::abs(dryPage[n]));}return true;},error,cancel))return false;
    float combinedPeak=0;
    const auto value=[&](int n){return dryPage[n]*current->dry+wetPage[n]*current->wet;};
    if(!scan([&](int64_t,int frames){for(int n=0;n<frames*2;++n){const auto v=value(n);if(!std::isfinite(v)){error="Non-finite Export mix";return false;}combinedPeak=std::max(combinedPeak,std::abs(v));}return true;},error,cancel))return false;
    measured.peak=combinedPeak;
    measured.displayGain=1/std::max({1.f,measured.dryPeak*current->dry,measured.wetPeak*current->wet});
    const auto frames=current->end-current->origin;const auto bin=std::max(int64_t{1},(frames+static_cast<int64_t>(maximumPeaks)-1)/static_cast<int64_t>(maximumPeaks));
    for(auto at=current->origin;at<current->end;){const auto count=static_cast<uint32_t>(std::min(bin,current->end-at));
        measured.dryPeaks.push_back({at,count,0,0});measured.wetPeaks.push_back({at,count,0,0});at+=count;}
    if(!scan([&](int64_t at,int count){for(int n=0;n<count;++n){bool audible=false;const auto index=static_cast<size_t>((at+n-current->origin)/bin);
        auto& d=measured.dryPeaks[index];auto& w=measured.wetPeaks[index];for(int c=0;c<2;++c){const auto k=n*2+c;
            const auto dv=dryPage[k]*current->dry*measured.sumGain,wv=wetPage[k]*measured.wetGain*current->wet*measured.sumGain;
            d.minimum=std::min(d.minimum,dv);d.maximum=std::max(d.maximum,dv);w.minimum=std::min(w.minimum,wv);w.maximum=std::max(w.maximum,wv);
            audible=audible || value(k)*measured.sumGain!=0;}
        if(audible){if(!measured.audibleStart)measured.audibleStart=at+n;measured.audibleEnd=at+n+1;}}return true;},error,cancel))return false;
    ready=true;error.clear();return true;
}
bool SongMix::read(int64_t at,std::span<float> output,juce::String& error)
{
    if(!ready || output.empty() || output.size()%2 || output.size()/2>exportPageFrames){error="Export mix is not ready or page invalid";return false;}
    if(!branches(at,{dryPage.data(),output.size()},{wetPage.data(),output.size()},error))return false;
    for(size_t n=0;n<output.size();++n)output[n]=(dryPage[n]*current->dry+wetPage[n]*measured.wetGain*current->wet)*measured.sumGain;
    return true;
}
bool SongMix::adoptMeasurements(const SongMixMetrics& m,const juce::String& identity,juce::String& error)
{
    if(!current || current->identity!=identity || !std::isfinite(m.wetGain) || !std::isfinite(m.sumGain)
        || !std::isfinite(m.displayGain) || m.wetGain<=0 || m.wetGain>1 || m.sumGain<=0 || m.sumGain>1
        || m.displayGain<=0 || m.displayGain>1 || m.wetGain!=1 || m.sumGain!=1){error="Waveform measurements do not match accepted composition";return false;}
    measured.wetGain=m.wetGain;measured.sumGain=m.sumGain;measured.displayGain=m.displayGain;
    measured.audibleStart=m.audibleStart;measured.audibleEnd=m.audibleEnd;ready=true;return true;
}
bool SongMix::summarize(int64_t first,int64_t end,size_t bins,SongMixDetail& output,juce::String& error,const RenderCancellation& cancel)
{
    if(!current || !ready || bins==0 || first<current->origin || end>current->end || end<=first){error="Invalid waveform detail range";return false;}
    SongMixDetail result;result.composition=current->identity;result.first=first;result.end=end;result.bins=std::min(bins,maximumPeaks);
    const auto bin=std::max(int64_t{1},(end-first+static_cast<int64_t>(result.bins)-1)/static_cast<int64_t>(result.bins));
    for(auto at=first;at<end;){const auto count=static_cast<uint32_t>(std::min(bin,end-at));result.dry.push_back({at,count,0,0});result.wet.push_back({at,count,0,0});at+=count;}
    for(const auto& span:occupied)for(auto at=std::max(first,span.first);at<std::min(end,span.second);){
        if(cancel && cancel()){error="Waveform detail superseded";return false;}
        const auto frames=static_cast<int>(std::min(int64_t{exportPageFrames},std::min(end,span.second)-at));
        if(!branches(at,{dryPage.data(),static_cast<size_t>(frames)*2},{wetPage.data(),static_cast<size_t>(frames)*2},error))return false;
        for(int n=0;n<frames;++n){const auto index=static_cast<size_t>((at+n-first)/bin);auto& d=result.dry[index];auto& w=result.wet[index];
            for(int c=0;c<2;++c){const auto i=n*2+c;const auto dv=dryPage[i]*current->dry*measured.sumGain,wv=wetPage[i]*measured.wetGain*current->wet*measured.sumGain;
                d.minimum=std::min(d.minimum,dv);d.maximum=std::max(d.maximum,dv);w.minimum=std::min(w.minimum,wv);w.maximum=std::max(w.maximum,wv);}}
        at+=frames;
    }
    output=std::move(result);error.clear();return true;
}
}
