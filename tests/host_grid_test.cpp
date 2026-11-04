#include <Core/HostGrid.h>
#include <UI/SwellViewport.h>
#include <iostream>
#include <stdexcept>
#include <limits>
#define CHECK(value) do {if(!(value))throw std::runtime_error(#value);}while(false)

// Independent integer oracle: known sample intervals, no host/PPQ conversion.
static int64_t oracle(int64_t candidate,int64_t origin,int64_t interval)
{
    const auto delta=candidate-origin,whole=delta/interval,remainder=delta%interval;
    const auto rounded=whole+(std::abs(remainder)*2>=interval?(delta<0?-1:1):0);
    return origin+rounded*interval;
}
int main()
{
    try
    {
        using namespace m3;
        for(const auto meter:{std::pair{4,4},std::pair{3,4},std::pair{6,8}})
        {
            const SongTime accepted{true,1000,0,48000,120,meter.first,meter.second};
            const int64_t beat=meter.second==8?12000:24000;
            const std::array<int64_t,7> intervals{beat/16,beat/8,beat/4,beat,beat*meter.first,beat*meter.first*2,beat*meter.first*4};
            for(size_t n=0;n<gridChoices.size();++n)
            {
                const auto grid=HostGrid::make(accepted,accepted,48000,gridChoices[n].division);CHECK(grid);
                for(int64_t sample=-200000;sample<=200000;sample+=137)
                {
                    CHECK(grid->snap(sample)==oracle(sample,1000,intervals[n]));
                    CHECK(SwellViewport{}.snapToGrid(sample,accepted,accepted,48000,gridChoices[n].division)==oracle(sample,1000,intervals[n]));
                }
                CHECK(grid->snap(1000+intervals[n]/2)==1000+intervals[n]);
                CHECK(grid->snap(1000-intervals[n]/2)==1000-intervals[n]);
                CHECK(grid->adjacent(1000,1)==1000+intervals[n] && grid->adjacent(1000,-1)==1000-intervals[n]);
                CHECK(grid->adjacent(1001,-1)==1000 && grid->adjacent(999,1)==1000);
                for(const auto domain:{std::pair<int64_t,int64_t>{-12345,23456},{100,500},{-500,-100},{1000,1000}})
                    for(int64_t candidate=-40000;candidate<40000;candidate+=997)
                    {
                        std::optional<int64_t> expected;int64_t nearest=INT64_MAX,chosen=0;
                        for(int64_t index=-64;index<=64;++index)
                        {
                            const auto tick=1000+index*intervals[n];if(tick<domain.first || tick>domain.second)continue;
                            const auto distance=std::abs(tick-candidate);
                            if(!expected || distance<nearest || (distance==nearest && std::abs(index)>std::abs(chosen)))
                            {expected=tick;nearest=distance;chosen=index;}
                        }
                        CHECK(grid->snapWithin(candidate,domain.first,domain.second)==expected);
                        CHECK(SwellViewport{}.snapToGridWithin(candidate,domain.first,domain.second,accepted,accepted,48000,gridChoices[n].division)==expected);
                    }
            }
        }
        SongTime accepted{true,1000,.25,48000,120,4,4},now=accepted;
        now.sample=145000;now.ppq=6.25;
        auto grid=HostGrid::make(accepted,now,48000,GridDivision::quarterBeat);CHECK(grid);
        CHECK(grid->snap(-2000)==1000);CHECK(grid->snap(-8000)==-11000);
        now.sample=-23000;now.ppq=-.75;CHECK(HostGrid::make(accepted,now,48000,GridDivision::beat));
        ++now.sample;CHECK(HostGrid::make(accepted,now,48000,GridDivision::beat));
        ++now.sample;CHECK(!HostGrid::make(accepted,now,48000,GridDivision::beat));
        for(int field=0;field<8;++field)
        {
            now=accepted;
            if(field==0)now.valid=false;if(field==1)now.rate=44100;if(field==2)now.bpm=121;
            if(field==3)now.numerator=3;if(field==4)now.denominator=8;
            if(field==5)now.ppq=std::numeric_limits<double>::quiet_NaN();
            if(field==6)now.ppq=1.e300;if(field==7)now.sample=INT64_MAX;
            CHECK(!HostGrid::make(accepted,now,48000,GridDivision::beat));
            CHECK(!SwellViewport{}.snapToGrid(100,accepted,now,48000,GridDivision::beat));
        }
        CHECK(!HostGrid::make(accepted,accepted,44100,GridDivision::beat));
        CHECK(!HostGrid::make(accepted,accepted,std::numeric_limits<double>::quiet_NaN(),GridDivision::beat));
        CHECK(!HostGrid::make(accepted,accepted,48000,static_cast<GridDivision>(100)));
        for(int field=0;field<7;++field)
        {
            auto invalid=accepted;
            if(field==0)invalid.valid=false;if(field==1)invalid.rate=0;if(field==2)invalid.bpm=0;
            if(field==3)invalid.numerator=0;if(field==4)invalid.denominator=0;
            if(field==5)invalid.bpm=std::numeric_limits<double>::infinity();
            if(field==6)invalid.ppq=std::numeric_limits<double>::quiet_NaN();
            CHECK(!HostGrid::make(invalid,invalid,invalid.rate,GridDivision::beat));
        }
        accepted.ppq=.10001;
        grid=HostGrid::make(accepted,accepted,48000,GridDivision::quarterBeat);CHECK(grid);
        CHECK(grid->snap(4600)==4600); // Fractional sample origin, one final sample rounding.
        CHECK(grid->snapWithin(4600,4500,4700)==4600);
        CHECK(!grid->snapWithin(100,100,500));
        accepted={true,0,0,1,60,1,4};
        grid=HostGrid::make(accepted,accepted,1,GridDivision::sixteenthBeat);CHECK(grid);
        CHECK(!grid->snap(INT64_MAX) && !grid->snap(INT64_MIN));
        CHECK(grid->snap(0)==0);
        CHECK(grid->snapWithin(1,1,1)==1 && grid->snapWithin(0,0,0)==0);
        CHECK(!grid->snapWithin(INT64_MAX,INT64_MAX,INT64_MAX));
        CHECK(grid->adjacent(1,1)==2 && grid->adjacent(1,-1)==0 && !grid->adjacent(INT64_MAX,1));
        std::cout<<"Host grid integer-oracle/meter/tie/mapping/overflow checks passed.\n";
        return 0;
    }
    catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
