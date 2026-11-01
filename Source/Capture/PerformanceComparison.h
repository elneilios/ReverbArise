#pragma once
#include <array>
#include <algorithm>
#include <cmath>

// Streaming comparison of stereo coarse spectral envelopes. Ignored input is
// never installed, so successive small deviations cannot move the baseline.
class PerformanceComparison
{
public:
    PerformanceComparison(double rate,double tolerance):limit(tolerance),window(std::max(1,static_cast<int>(rate*.05)))
    {for(int band=0;band<3;++band)alpha[band]=1-std::exp(-6.283185307179586*std::min(rate*.4,250.0*std::pow(4.0,band))/rate);}
    bool push(const float* old,const float* incoming,int frames)
    {
        for(int n=0;n<frames;++n){for(int c=0;c<2;++c){
            const double values[]{old[n*2+c],incoming[n*2+c]};
            for(int side=0;side<2;++side){if(!std::isfinite(values[side]))return false;
                double filtered[3];for(int b=0;b<3;++b){auto& state=low[side][c][b];state+=alpha[b]*(values[side]-state);filtered[b]=state;}
                const double bands[]{filtered[0],filtered[1]-filtered[0],filtered[2]-filtered[1],values[side]-filtered[2]};
                for(int b=0;b<4;++b)energy[side][c*4+b]+=bands[b]*bands[b];}}
            if(++count==window){if(!check())return false;}}
        return true;
    }
    bool finish(){return count==0 || check();}
private:
    bool check(){double difference=0,oldTotal=0,newTotal=0;
        for(int b=0;b<8;++b){const auto a=std::sqrt(energy[0][b]/count),v=std::sqrt(energy[1][b]/count);difference+=std::abs(a-v);oldTotal+=a;newTotal+=v;}
        energy={};count=0;
        return difference<=.001 || difference<=limit*std::max(oldTotal,newTotal);
    }
    double limit;int window,count=0;
    std::array<double,3> alpha{};
    std::array<std::array<std::array<double,3>,2>,2> low{};
    std::array<std::array<double,8>,2> energy{};
};
