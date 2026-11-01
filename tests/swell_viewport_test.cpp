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
    for(auto rate:{44100.0,48000.0,96000.0})for(auto bpm:{82.0,120.0,137.3})
    {
        time.rate=rate;time.bpm=bpm;time.sample=123456;time.ppq=7.2;
        for(int64_t sample:{-123456,0,13,123456,9876543})
        {int64_t parsed;REQUIRE(m3::parseMusicalPosition(m3::musicalPosition(sample,time),time,parsed) && parsed==sample);}
    }
    int64_t ignored;REQUIRE(!m3::parseMusicalPosition("1.9.000000000",time,ignored));
    REQUIRE(!m3::parseMusicalPosition("1.1.nan",time,ignored));
});}
