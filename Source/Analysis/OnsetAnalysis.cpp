#include "OnsetAnalysis.h"
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace m3
{
bool suggestOnsets(CaptureAssetStore& store,const CaptureManifest& map,int64_t start,int64_t end,
                   float sensitivity,double separation,std::vector<Candidate>& output,
                   juce::String& error,const std::function<bool()>& cancelled)
{
    if(!std::isfinite(map.sampleRate) || map.sampleRate<1000 || map.sampleRate>384000
        || !std::isfinite(sensitivity) || sensitivity<0 || sensitivity>1 || !std::isfinite(separation) || separation<0.001 || separation>2)
    { error="Invalid analysis rate or controls"; return false; }
    int order=8; while(order<13 && (1<<order)<map.sampleRate*0.021) ++order;
    const auto size=1<<order,hop=size/4;
    const auto minDistance=static_cast<int64_t>(std::llround(separation*map.sampleRate));
    juce::dsp::FFT fft(order);
    std::vector<float> ring(static_cast<size_t>(size)*2),work(size*2),previous(static_cast<size_t>(size/2+1)*2),spectrum(size/2+1);
    std::array<float,32> history{};
    std::array<double,4> energyHistory{},peakHistory{}; double hopEnergy=0, acceptedEnergy=0, lastEnergy=0, energyPeak=0;
    int64_t attackUntil=0;
    size_t energyPosition=0; bool armed=true,lastAttack=false;
    size_t ringPosition=0,historyPosition=0; int64_t seen=0,runStart=0;
    float last=0,before=0,lastThreshold=0; int64_t lastPosition=0; std::vector<Candidate> result;
    const auto peak=[&](float next)
    {
        if(lastAttack && armed && last>1e-7f && last>=lastThreshold && last>before && last>=next)
        {
            Candidate candidate{std::max(runStart,lastPosition),last};
            armed=false; acceptedEnergy=std::max(lastEnergy,energyPeak); attackUntil=0;
            if(!result.empty() && candidate.sample-result.back().sample<minDistance)
            { if(candidate.strength>result.back().strength) result.back()=candidate; }
            else result.push_back(candidate);
        }
    };
    const auto reset=[&](int64_t at)
    {
        peak(0); std::fill(ring.begin(),ring.end(),0.0f); std::fill(previous.begin(),previous.end(),0.0f); history.fill(0.0f);
        attackUntil=0; peakHistory.fill(0); energyHistory.fill(0); energyPosition=0; hopEnergy=acceptedEnergy=lastEnergy=energyPeak=0; armed=true; lastAttack=false;
        ringPosition=historyPosition=0; seen=0; runStart=at; last=before=lastThreshold=0; lastPosition=at;
    };
    const auto accept=[&](int64_t at,const float* samples,int frames,bool newRun)
    {
        if(newRun) reset(at);
        for(int frame=0;frame<frames;++frame)
        {
            ring[ringPosition*2]=samples[frame*2]; ring[ringPosition*2+1]=samples[frame*2+1];
            hopEnergy+=(static_cast<double>(samples[frame*2])*samples[frame*2]+static_cast<double>(samples[frame*2+1])*samples[frame*2+1])/2;
            ringPosition=(ringPosition+1)%static_cast<size_t>(size); ++seen;
            if(seen%hop) continue;
            float novelty=0;
            for(int channel=0;channel<2;++channel)
            {
                std::fill(work.begin(),work.end(),0.0f);
                for(int n=0;n<size;++n)
                    work[n]=ring[((ringPosition+static_cast<size_t>(n))%static_cast<size_t>(size))*2+channel]
                        * (0.5f-0.5f*std::cos(juce::MathConstants<float>::twoPi*n/(size-1)));
                fft.performFrequencyOnlyForwardTransform(work.data());
                for(int bin=0;bin<=size/2;++bin)
                {
                    const auto index=static_cast<size_t>(bin)*2+channel;
                    novelty+=std::max(0.0f,work[bin]-previous[index]); previous[index]=work[bin];
                }
            }
            novelty/=size;
            float average=0; for(auto value:history) average+=value/history.size();
            peak(novelty);
            const auto energy=hopEnergy/hop; hopEnergy=0;
            double preceding=0; for(auto e:energyHistory)preceding+=e/energyHistory.size();
            if(energy<acceptedEnergy*0.85)armed=true;
            // Confirm a real short-time energy attack, not positive flux from release,
            // moving partials or a modulated decay. Timestamp remains the flux peak.
            if(armed && energy>1e-10 && energy>peakHistory[energyPosition%peakHistory.size()]*(0.9*std::exp((0.5-sensitivity)*3.0)) && energy>preceding*(1.75-0.8*sensitivity))attackUntil=seen+size;
            lastAttack=seen<attackUntil;
            energyPeak=std::max(energy,energyPeak*std::exp(-hop/(map.sampleRate*0.5)));
            lastEnergy=energy;peakHistory[energyPosition%peakHistory.size()]=energyPeak; energyHistory[energyPosition++%energyHistory.size()]=energy;
            before=last; last=novelty; lastPosition=at+frame+1-size/2;
            lastThreshold=std::max(1e-7f,average*(3.0f-2.0f*sensitivity));
            history[historyPosition++%history.size()]=novelty;
            if(result.size()>65536 || (cancelled && cancelled())) return false;
        }
        return true;
    };
    if(!visitSource(store,map,start,end,true,accept,error,cancelled)) return false;
    peak(0);
    std::erase_if(result,[&](const auto& c){return c.sample<start || c.sample>=end;});
    output=std::move(result); return true;
}
}
