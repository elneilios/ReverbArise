#include "TestSupport.h"
#include <DSP/GainStaging.h>
#include <DSP/SwellShape.h>
#include <cmath>
using namespace m3;

namespace
{
void energyCase(double rate)
{
    Session s;s.gainPolicy=GainPolicy::musical;s.gainRevision=energyGainRevision;
    Swell w;w.arrival=w.sourceStart=static_cast<int64_t>(rate);w.sourceEnd=w.arrival+static_cast<int64_t>(rate*.2);w.ir.content="ir";
    WetRender raw;raw.rate=rate;raw.start=0;raw.sourceStart=w.sourceStart;raw.sourceEnd=w.sourceEnd;
    raw.samples.assign(static_cast<size_t>(w.sourceEnd)*2,.125f);
    SourceInterval dry;dry.start=w.arrival;dry.end=w.sourceEnd;dry.rate=rate;dry.dependency="source";dry.samples.assign(static_cast<size_t>(dry.end-dry.start)*2,.5f);
    GainCalibration c;juce::String note;REQUIRE(prepareSourceEnergyCalibration(s,w,raw,dry,dry.end,"raw",c,note));
    REQUIRE(c.state==CalibrationState::matched && std::abs(c.dryRms-.5f)<1e-7f && std::abs(c.wetRms-.125f)<1e-6f);
    REQUIRE(std::abs(c.baselineDb-20*std::log10(4.0))<1e-5);
    // Whole-buffer silence outside the default support cannot dilute RMS.
    auto padded=raw;padded.samples.resize(padded.samples.size()+48000,0);GainCalibration withTail;
    REQUIRE(prepareSourceEnergyCalibration(s,w,padded,dry,dry.end,"padded",withTail,note));REQUIRE(withTail.baselineDb==c.baselineDb);
    const auto original=c;for(auto& x:raw.samples)x*=4;
    {auto approved=s;approved.gainRevision=approvedGainRevision;GainCalibration release,reference;
        REQUIRE(prepareSourceEnergyCalibration(approved,w,raw,dry,dry.end,"approved",release,note,{},approvedEnergyTargetDb));
        REQUIRE(prepareSourceEnergyCalibration(s,w,raw,dry,dry.end,"review",reference,note,{},6));
        REQUIRE(release.baselineDb==reference.baselineDb && release.dryRms==reference.dryRms && release.wetRms==reference.wetRms
            && release.revision==approvedGainRevision);}
    REQUIRE(prepareSourceEnergyCalibration(s,w,raw,dry,dry.end,"scaled",c,note));
    REQUIRE(std::abs(c.baselineDb)<1e-5 && std::abs(c.wetRms*std::pow(10.0f,c.baselineDb/20)-original.wetRms*std::pow(10.0f,original.baselineDb/20))<1e-6f);
    w.shape=initialShape(rate);w.calibration=c;
    for(const auto level:{-60.0f,-6.0f,0.0f,6.0f,12.0f}){w.shape.gainDb=level;WetRender shaped;juce::String error;
        REQUIRE(prepareShape(w,raw,previewMemoryBudget,shaped,error));
        REQUIRE(std::abs(shaped.peak-.5f*juce::Decibels::decibelsToGain(level))<1e-5f);
        REQUIRE(w.calibration==c);}
    for(auto& value:raw.samples)value=-value;
    GainCalibration inverted;REQUIRE(prepareSourceEnergyCalibration(s,w,raw,dry,dry.end,"inverted",inverted,note));
    REQUIRE(inverted.baselineDb==c.baselineDb && inverted.wetRms==c.wetRms);
    for(auto& value:raw.samples)value=-value;
    for(auto& x:dry.samples)x*=.25f;for(auto& x:raw.samples)x*=.25f;
    REQUIRE(prepareSourceEnergyCalibration(s,w,raw,dry,dry.end,"quiet",c,note));REQUIRE(std::abs(c.baselineDb)<1e-5 && std::abs(c.dryRms-.125f)<1e-7f);
    // Independent direct-window RMS oracle for sparse, asymmetric stereo.
    for(size_t n=0;n<dry.samples.size()/2;++n){dry.samples[n*2]=(n%31==0?.4f:0);dry.samples[n*2+1]=.2f;}
    const auto end=w.arrival+static_cast<int64_t>(rate*.007);
    REQUIRE(prepareSourceEnergyCalibration(s,w,raw,dry,end,"short",c,note));
    REQUIRE(c.windowFrames==end-w.arrival && c.referenceEnd==end);
    double power=0;for(int64_t n=0;n<c.windowFrames;++n)power+=dry.samples[n*2]*dry.samples[n*2];
    REQUIRE(std::abs(c.dryRms-std::max(.2,std::sqrt(power/c.windowFrames)))<1e-6);
    // A louder next event must not be measured, including the final hop.
    for(size_t n=0;n<dry.samples.size()/2;++n){dry.samples[n*2]=n<static_cast<size_t>(end-w.arrival)?.1f:1;dry.samples[n*2+1]=0;}
    REQUIRE(prepareSourceEnergyCalibration(s,w,raw,dry,end,"bounded",c,note));REQUIRE(std::abs(c.dryRms-.1f)<1e-6);
    auto missing=dry;missing.start=w.arrival+1;REQUIRE(prepareSourceEnergyCalibration(s,w,raw,missing,end,"missing",c,note));REQUIRE(c.state==CalibrationState::unavailable);
    dry.samples.assign(dry.samples.size(),0);REQUIRE(prepareSourceEnergyCalibration(s,w,raw,dry,dry.end,"silent",c,note));REQUIRE(c.state==CalibrationState::unavailable);
    dry.samples.assign(dry.samples.size(),1);raw.samples.assign(raw.samples.size(),.0011f);
    REQUIRE(prepareSourceEnergyCalibration(s,w,raw,dry,dry.end,"capped",c,note));REQUIRE(c.state==CalibrationState::capped && c.baselineDb==24);
    REQUIRE(prepareSourceEnergyCalibration(s,w,raw,dry,dry.end,"cancel",c,note,[]{return true;}));REQUIRE(c.state==CalibrationState::unavailable);
}
void rateCase(double rate)
{
    Session session;session.gainPolicy=GainPolicy::musical;session.gainRevision=candidateGainRevision;
    Swell swell;swell.arrival=1000;swell.sourceStart=1000;
    swell.sourceEnd=1000+static_cast<int64_t>(std::llround(rate*.2));swell.ir.content="ir";
    WetRender raw;raw.start=0;raw.sourceStart=swell.sourceStart;raw.sourceEnd=swell.sourceEnd;raw.rate=rate;
    raw.samples.resize(static_cast<size_t>(swell.sourceEnd+100)*2,.25f);raw.peak=.25f;
    SourceInterval source;source.start=swell.arrival;source.end=swell.sourceEnd;source.rate=rate;source.dependency="source";
    source.samples.resize(static_cast<size_t>(source.end-source.start)*2,.5f);
    GainCalibration calibration;juce::String note;const auto next=swell.arrival+static_cast<int64_t>(std::llround(rate*.05));
    REQUIRE(prepareSourcePeakCalibration(session,swell,raw,source,next,"render",calibration,note));
    REQUIRE(calibration.state==CalibrationState::matched && calibration.referenceStart==swell.arrival && calibration.referenceEnd==next);
    REQUIRE(std::abs(calibration.dryPeak-.5f)<1e-7f && std::abs(calibration.wetPeak-.25f)<.001f);
    REQUIRE(std::abs(calibration.baselineDb-juce::Decibels::gainToDecibels(calibration.dryPeak/calibration.wetPeak))<1e-5f);
    swell.calibration=calibration;auto shapedSwell=swell;shapedSwell.shape=initialShape(rate);
    WetRender shaped;juce::String error;REQUIRE(prepareShape(shapedSwell,raw,previewMemoryBudget,shaped,error));
    REQUIRE(std::abs(shaped.peak-.5f)<.002f);
    const auto saved=shapedSwell.calibration;shapedSwell.shape.gainDb=-6;shapedSwell.shape.inTension=.7;shapedSwell.placement=10;
    REQUIRE(shapedSwell.calibration==saved);
}
}

int main(){return runTest([]{
    rateCase(44100);rateCase(48000);rateCase(96000);
    energyCase(44100);energyCase(48000);energyCase(96000);
    Session session;session.gainPolicy=GainPolicy::musical;session.gainRevision=candidateGainRevision;
    Swell swell;swell.arrival=100;swell.sourceStart=100;swell.sourceEnd=300;swell.ir.content="ir";
    WetRender raw;raw.start=0;raw.sourceStart=100;raw.sourceEnd=300;raw.rate=1000;raw.samples.resize(800,.25f);raw.peak=.25f;
    SourceInterval source;source.start=100;source.end=300;source.rate=1000;source.dependency="source";source.samples.resize(400,0);
    GainCalibration result;juce::String note;
    REQUIRE(prepareSourcePeakCalibration(session,swell,raw,source,300,"render",result,note));
    REQUIRE(result.state==CalibrationState::unavailable && note.contains("below -60"));
    source.start=101;
    REQUIRE(prepareSourcePeakCalibration(session,swell,raw,source,300,"render",result,note));
    REQUIRE(result.state==CalibrationState::unavailable && note.contains("not captured"));
    source.start=100;source.samples.resize(400,0);source.samples[1]=.75f;
    REQUIRE(prepareSourcePeakCalibration(session,swell,raw,source,300,"render",result,note));
    REQUIRE(result.state==CalibrationState::matched && std::abs(result.dryPeak-.75f)<1e-7f);
    std::fill(raw.samples.begin(),raw.samples.end(),0.0f);raw.peak=0;
    REQUIRE(prepareSourcePeakCalibration(session,swell,raw,source,300,"render",result,note));
    REQUIRE(result.state==CalibrationState::unavailable && std::isfinite(result.baselineDb));
    std::fill(source.samples.begin(),source.samples.end(),1.0f);std::fill(raw.samples.begin(),raw.samples.end(),.002f);raw.peak=.002f;
    REQUIRE(prepareSourcePeakCalibration(session,swell,raw,source,300,"render",result,note));
    REQUIRE(result.state==CalibrationState::capped && result.baselineDb==candidateMaximumBoostDb && note.contains("capped"));
    auto fixed=fixedCalibration(session,swell,6,"render");REQUIRE(fixed.state==CalibrationState::fixed && fixed.baselineDb==6);
    swell.calibration=fixed;swell.shape.automaticStart=false;swell.shape.startOffset=-50;swell.shape.endOffset=50;
    WetRender shaped;juce::String error;REQUIRE(prepareShape(swell,raw,previewMemoryBudget,shaped,error));
    const auto at0=shaped.samples[0];swell.shape.gainDb=6;WetRender boosted;REQUIRE(prepareShape(swell,raw,previewMemoryBudget,boosted,error));
    REQUIRE(std::abs(boosted.samples[0]/at0-juce::Decibels::decibelsToGain(6.0f))<1e-5f);
    Session legacy;auto ignored=fixedCalibration(legacy,swell,9,"render");REQUIRE(ignored.state==CalibrationState::unavailable);

    struct Positioned{int64_t start;std::vector<float> samples;};
    const auto measure=[&](const std::vector<Positioned>& members,double rate,float& attenuation,float& peak){
        juce::String scanError;return measureAggregateGain(-9,19,candidateWetCeilingDb,0,previewMemoryBudget,
            [&](int64_t at,int frames,float* sum,juce::String&){for(const auto& member:members)for(int n=0;n<frames;++n){const auto source=at+n-member.start;
                    if(source>=0 && static_cast<size_t>(source)*2+1<member.samples.size()){sum[n*2]+=member.samples[static_cast<size_t>(source)*2];sum[n*2+1]+=member.samples[static_cast<size_t>(source)*2+1];}}
                return true;}, {},attenuation,peak,scanError);};
    for(const auto rate:{44100.0,48000.0,96000.0}){
        float attenuation=0,peak=0;
        REQUIRE(measure({{-4,{.8f,.1f, 0,0, 0,0, 0,0}}, {0,{0,0, .8f,.2f, 0,0, 0,0}}},rate,attenuation,peak));
        REQUIRE(std::abs(peak-.8f)<1e-7f && attenuation==1); // Independent maxima do not coincide.
        REQUIRE(measure({{-4,{.7f,.1f, .7f,.1f}}, {-4,{.7f,.2f, .7f,.2f}}},rate,attenuation,peak));
        REQUIRE(std::abs(peak-1.4f)<1e-6f && attenuation<1); // Correlated overlap.
        REQUIRE(measure({{-4,{.7f,.4f, .7f,.4f}}, {-4,{-.7f,-.2f, -.7f,-.2f}}},rate,attenuation,peak));
        REQUIRE(std::abs(peak-.2f)<1e-6f && attenuation==1); // Anticorrelation and stereo-linked measurement.
    }
    {float attenuation=0,peak=0;juce::String scanError;int calls=0;
        REQUIRE(!measureAggregateGain(0,10000,candidateWetCeilingDb,0,previewMemoryBudget,
            [&](int64_t,int,float*,juce::String&){++calls;return true;},[&]{return calls>0;},attenuation,peak,scanError));
        REQUIRE(scanError.contains("superseded") && calls==1);
        REQUIRE(!measureAggregateGain(0,100,candidateWetCeilingDb,0,16,
            [&](int64_t,int,float*,juce::String&){return true;},{},attenuation,peak,scanError));
        REQUIRE(scanError.contains("Invalid aggregate"));}
});}
