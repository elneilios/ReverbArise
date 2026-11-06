#include "TestSupport.h"
#include <UI/WaveformDisplay.h>
#include <random>

int main(){return runTest([]
{
    using namespace m3::ui;
    const auto values=[](const CapturePeak& p){return std::pair{p.minimum,p.maximum};};
    std::mt19937 random(67);
    std::vector<CapturePeak> peaks;
    for(int i=0;i<4000;++i)
    {
        // Actual silence and holes, one-sample impulses, and negative lead-in.
        if(i%11==0)continue;
        peaks.push_back({i*13-17000,static_cast<uint32_t>(i%7==0?1:9),
            i%17==0?0.f:-static_cast<float>(random()%1001)/1000,
            i%17==0?0.f:static_cast<float>(random()%1001)/1000});
    }
    for(const auto placement:{int64_t{-3000},int64_t{0},int64_t{40000}})
    for(const auto view:std::vector<std::pair<double,double>>{{-18000,40000},{-200,700},{1700.25,1706.75},{40000,41000}})
    for(const auto width:{1,31,800,1600})
    {
        uint64_t visited=0;
        const auto actual=waveformColumns(peaks,view.first,view.second,width,placement,values,visited);
        // Independent deliberately slow oracle: intersect every pixel interval
        // with every input peak. It does not use reduction/search helpers.
        for(int x=0;x<width;++x)
        {
            WaveformColumn expected;
            const long double a=view.first+(static_cast<long double>(view.second)-view.first)*x/width;
            const long double b=view.first+(static_cast<long double>(view.second)-view.first)*(x+1)/width;
            for(const auto& p:peaks)
                if(static_cast<long double>(p.startSample)+placement<b && static_cast<long double>(p.startSample)+placement+p.frameCount>a)
                {
                    if(!expected.present)expected={p.minimum,p.maximum,true};
                    else {expected.minimum=std::min(expected.minimum,p.minimum);expected.maximum=std::max(expected.maximum,p.maximum);}
                }
            REQUIRE(actual[x].present==expected.present);
            if(expected.present)
            {
                if(actual[x].minimum!=expected.minimum || actual[x].maximum!=expected.maximum)
                    std::cerr<<"Column mismatch "<<placement<<' '<<view.first<<' '<<view.second<<' '<<width<<' '<<x
                        <<" actual "<<actual[x].minimum<<','<<actual[x].maximum<<" expected "<<expected.minimum<<','<<expected.maximum<<'\n';
                REQUIRE(actual[x].minimum==expected.minimum);REQUIRE(actual[x].maximum==expected.maximum);
            }
        }
        if(view.second-view.first<100)REQUIRE(visited<20);
    }
    uint64_t visited=0;
    const std::vector<CapturePeak> edge{{INT64_MAX-8,8,-1,1}};
    const auto shifted=waveformColumns(edge,-20,20,40,INT64_MIN,values,visited);
    REQUIRE(shifted[11].present && shifted[18].present && !shifted[19].present);
    REQUIRE(waveformColumns(peaks,0,0,100,0,values,visited).empty());
    const std::vector<CapturePeak> coarse{{0,100,-1,1},{200,100,-.5f,.5f}};
    const auto detail=waveformColumns(coarse,0,400,800,0,values,visited,true);
    REQUIRE(std::count_if(detail.begin(),detail.end(),[](const auto& c){return c.present;})==2);
    REQUIRE(detail[0].minimum==-1 && detail[0].maximum==1 && detail[400].maximum==.5f);
    // Pan wholly inside one coarse bucket, including fractional viewport starts
    // and translated negative lead-in. Keep exactly one clipped summary stroke;
    // neither blank captured coverage nor fill the viewport with invented PCM.
    for(const auto placement:{int64_t{-300},int64_t{0},int64_t{500}})
    for(const auto offset:{25.0,25.25,99.5})
    {
        uint64_t clippedVisited=0;
        const auto clipped=waveformColumns(coarse,placement+offset,placement+offset+.25,80,placement,values,clippedVisited,true);
        REQUIRE(clippedVisited==1);
        REQUIRE(std::count_if(clipped.begin(),clipped.end(),[](const auto& c){return c.present;})==1);
        REQUIRE(clipped[0].present && clipped[0].minimum==-1 && clipped[0].maximum==1);
    }
    const auto boundary=waveformColumns(coarse,100,200,80,0,values,visited,true);
    REQUIRE(std::none_of(boundary.begin(),boundary.end(),[](const auto& c){return c.present;}));
    const auto crossing=waveformColumns(coarse,99.5,200.5,202,0,values,visited,true);
    REQUIRE(crossing[0].present && crossing[201].present);
    REQUIRE(std::count_if(crossing.begin(),crossing.end(),[](const auto& c){return c.present;})==2);
    std::cout<<"Pixel extrema, impulse/silence/gap, negative/overflow-safe placement and visible search reference checks passed.\n";
});}
