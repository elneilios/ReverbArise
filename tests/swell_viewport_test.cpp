#include "TestSupport.h"
#include <UI/MusicalPosition.h>
#include <UI/SwellViewport.h>
int main(){return runTest([]
{
    m3::SwellViewport view;view.fit(-24000,192000);
    for(int64_t sample:{-24000,0,12345,96000,192000})
    {const auto pixel=view.pixel(static_cast<double>(sample),20,1000);REQUIRE(std::abs(view.sample(pixel,20,1000)-sample)<=1);}
    const auto size=view.span(),first=view.first;view.shift(1);REQUIRE(view.first==first+size);view.shift(-1);REQUIRE(view.first==first);
    view.zoom(0.5);REQUIRE(view.span()==size/2);
    SongTime time{true,480000,20,48000,120,4,4};view.page(16,time);REQUIRE(view.span()==1536000);
    REQUIRE(view.snap(12345,time,false)==12345 && view.snap(12345,time,true)==12000);
    view.fit(8000000,8200000);REQUIRE(std::abs(view.sample(view.pixel(8123456,0,1200),0,1200)-8123456)<=1);
    view.fit(-100,100);REQUIRE(view.first<0 && view.last>0);
    const m3::SwellViewport limits{-1000,1000};view=limits;
    REQUIRE(view.zoomBounded(.5,.25,limits,100));REQUIRE(view.first==-750 && view.last==250);
    REQUIRE(view.pixel(-500,0,1000)==250); // Independent pointer-anchor oracle.
    for(int n=0;n<100;++n)REQUIRE(view.zoomBounded(.01,.3,limits,100));
    REQUIRE(view.span()==100);
    for(int n=0;n<100;++n)REQUIRE(view.zoomBounded(100,.7,limits,100));
    REQUIRE(view.first==-1000 && view.last==1000);
    view={-1.e300,1.e300};REQUIRE(view.constrain(limits,100));REQUIRE(view.first==-1000 && view.last==1000);
    const auto unchanged=view;
    for(const double factor:{-1.,0.,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()})
    {REQUIRE(!view.zoomBounded(factor,.5,limits,100));REQUIRE(view.first==unchanged.first && view.last==unchanged.last);}
    REQUIRE(!view.zoomBounded(.5,std::numeric_limits<double>::quiet_NaN(),limits,100));
    REQUIRE(!view.constrain({1,1},100));
    REQUIRE(view.zoomBounded(std::numeric_limits<double>::max(),.5,limits,100));
    REQUIRE(view.first==-1000 && view.last==1000);
    for(auto rate:{44100.0,48000.0,96000.0})for(auto bpm:{82.0,120.0,137.3})
    {
        time.rate=rate;time.bpm=bpm;time.sample=123456;time.ppq=7.2;
        for(int64_t sample:{-123456,0,13,123456,9876543})
        {int64_t parsed;REQUIRE(m3::parseMusicalPosition(m3::musicalPosition(sample,time),time,parsed) && parsed==sample);}
    }
    int64_t ignored;REQUIRE(!m3::parseMusicalPosition("1.9.000000000",time,ignored));
    REQUIRE(!m3::parseMusicalPosition("1.1.nan",time,ignored));
});}
