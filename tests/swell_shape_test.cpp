#include "TestSupport.h"
#include <DSP/SwellShape.h>
#include <Core/ContentHash.h>
using namespace m3;
int main() { return runTest([]
{
    WetRender raw; raw.start=-10; raw.sourceStart=0; raw.sourceEnd=10; raw.rate=48000;
    for(int i=0;i<20;++i) {raw.samples.push_back(static_cast<float>(i+1));raw.samples.push_back(-static_cast<float>(i+1));}
    const auto hash=sampleHash(raw.samples,raw.rate,2); Swell w; w.arrival=0; w.sourceEnd=10;
    juce::String error; REQUIRE(fullShape(w,-10,10,error));
    WetRender shaped; REQUIRE(prepareShape(w,raw,previewMemoryBudget,shaped,error)); REQUIRE(shaped.samples==raw.samples);
    for(auto curve:{Curve::linear,Curve::smooth,Curve::exponential})
    {
        auto shape=w.shape; shape.startOffset=-7; shape.endOffset=6; shape.fadeIn=8; shape.fadeOut=9; shape.inCurve=shape.outCurve=curve; shape.gainDb=-6;
        REQUIRE(setShape(w,shape,-10,10,error)); REQUIRE(prepareShape(w,raw,previewMemoryBudget,shaped,error));
        REQUIRE(shaped.start==-7 && shaped.frames()==13 && shaped.samples.front()==0 && shaped.samples[24]==0);
        const auto f=[&](double x){x=std::clamp(x,0.0,1.0);if(curve==Curve::smooth)return 3*x*x-2*x*x*x;if(curve==Curve::exponential)return (std::exp(4*x)-1)/(std::exp(4.0)-1);return x;};
        for(size_t i=0;i<13;++i)
        {
            const auto expected=(i+4)*std::pow(10.0,-6.0/20)*f(i/8.0)*f((12.0-i)/9);
            REQUIRE(std::abs(shaped.samples[i*2]-expected)<1e-5);
            REQUIRE(shaped.samples[i*2+1]==-shaped.samples[i*2]);
        }
    }
    for(auto curve:{Curve::linear,Curve::smooth,Curve::exponential})for(double tension:{-1.0,-0.37,0.0,0.42,1.0})
    {
        double previous=-1;
        for(int i=0;i<=100;++i){const auto u=i/100.0;const auto warp=tension==0?u:(std::exp(6*tension*u)-1)/(std::exp(6*tension)-1);
            const auto expected=curve==Curve::linear?warp:curve==Curve::smooth?warp*warp*(3-2*warp):(std::exp(4*warp)-1)/(std::exp(4.0)-1);
            const auto actual=fadeFactor(u,curve,tension);REQUIRE(std::abs(actual-expected)<1e-10 && actual>=previous);previous=actual;}
        REQUIRE(fadeFactor(0,curve,tension)==0 && std::abs(fadeFactor(1,curve,tension)-1)<1e-12);
        auto shape=w.shape;shape.inTension=tension;shape.outTension=-tension;
        REQUIRE(setShape(w,shape,-10,10,error) && prepareShape(w,raw,previewMemoryBudget,shaped,error));
        for(int64_t i=0;i<static_cast<int64_t>(shaped.frames());++i){const auto at=shaped.start+i;
            const auto expected=raw.samples[(at-raw.start)*2]*std::pow(10.0,shape.gainDb/20.0)
                *fadeFactor(static_cast<double>(i)/shape.fadeIn,shape.inCurve,tension)
                *fadeFactor(static_cast<double>(shaped.frames()-1-i)/shape.fadeOut,shape.outCurve,-tension);
            REQUIRE(std::abs(shaped.samples[i*2]-expected)<1e-5);}
    }
    const auto stationary=shaped.samples;const auto fixed=w.shape;const auto oldStart=shaped.start;
    w.placement=-123;REQUIRE(prepareShape(w,raw,previewMemoryBudget,shaped,error));
    REQUIRE(shaped.samples==stationary && shaped.start==oldStart-123 && w.shape==fixed);
    w.placement=0;
    REQUIRE(sampleHash(raw.samples,raw.rate,2)==hash);
    const auto old=shaped.samples; REQUIRE(!prepareShape(w,raw,1,shaped,error) && shaped.samples==old);
    REQUIRE(!prepareShape(w,raw,previewMemoryBudget,shaped,error,[]{return true;}) && shaped.samples==old);
    auto one=w.shape; one.startOffset=0; one.endOffset=1; one.fadeIn=one.fadeOut=0; one.gainDb=0;
    REQUIRE(setShape(w,one,-10,10,error) && prepareShape(w,raw,previewMemoryBudget,shaped,error));
    REQUIRE(shaped.frames()==1 && shaped.samples[0]==11 && shaped.peak==11);
}); }
