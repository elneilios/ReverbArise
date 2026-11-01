#include "TestSupport.h"
#include <Core/TransportTime.h>
#include <limits>
int main(){return runTest([]{
    SongTime prepared{true,1000,0,48000,120,4,4};
    auto now=prepared;now.sample=145000;now.ppq=6;
    REQUIRE(transportMappingMatches(prepared,now));
    now.sample=-23000;now.ppq=-1;REQUIRE(transportMappingMatches(prepared,now));
    ++now.sample;REQUIRE(transportMappingMatches(prepared,now));
    ++now.sample;REQUIRE(!transportMappingMatches(prepared,now));
    for(int change=0;change<8;++change){now=prepared;
        if(change==0)now.valid=false;if(change==1)now.rate=44100;
        if(change==2)now.bpm=121;if(change==3)now.numerator=3;
        if(change==4)now.denominator=8;if(change==5)now.ppq=std::numeric_limits<double>::infinity();
        if(change==6)now.ppq=1e300;if(change==7)now.sample=INT64_MAX;
        REQUIRE(!transportMappingMatches(prepared,now));}
    now=prepared;now.sample=static_cast<int64_t>(std::llround(48000*60.0/82*144));now.ppq=144;now.bpm=82;
    prepared=now;prepared.sample=0;prepared.ppq=0;
    REQUIRE(transportMappingMatches(prepared,now));
});}
