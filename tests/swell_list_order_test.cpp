#include "TestSupport.h"
#include <Core/SwellSession.h>
#include <UI/Controls/SwellListOrder.h>
#include <limits>

int main(){return runTest([]
{
    m3::Session session; // No musical timing is required for absolute ordering.
    const std::pair<const char*,int64_t> positions[]{
        {"late",10},{"tie-z",2},{"last",INT64_MAX},{"tie-A",2},
        {"early",INT64_MIN},{"tie-a",2},{"origin",0}};
    for(const auto& [id,arrival]:positions)
    {m3::Swell w;w.id=id;w.arrival=arrival;w.name="Bar 100 arbitrary";session.swells.push_back(w);}
    const auto original=session;
    const auto verify=[&]
    {
        const auto order=m3::ui::controls::chronologicalSwellOrder(std::span<const m3::Swell>(session.swells));
        const char* expected[]{"early","origin","tie-A","tie-a","tie-z","late","last"};
        REQUIRE(order.size()==std::size(expected));
        for(size_t i=0;i<order.size();++i)REQUIRE(session.swells[order[i]].id==expected[i]);
    };
    verify();
    for(size_t i=0;i<session.swells.size();++i)REQUIRE(m3::sameSavedSwell(session.swells[i],original.swells[i]));
    session.swells[0].name="AAA";session.swells[0].placement=INT64_MIN;
    session.swells[1].name="ZZZ";session.swells[1].sourceStart=100;
    verify();std::reverse(session.swells.begin(),session.swells.end());verify();
    session.swells.clear();REQUIRE(m3::ui::controls::chronologicalSwellOrder(std::span<const m3::Swell>(session.swells)).empty());
});}
