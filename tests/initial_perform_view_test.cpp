#include "TestSupport.h"
#include <UI/InitialPerformView.h>
using namespace m3;
int main(){return runTest([]
{
    Session session;REQUIRE(ui::firstChronologicalSwell(session).isEmpty());
    Swell late,early;late.arrival=late.sourceStart=1000000;late.sourceEnd=1024000;
    early.arrival=early.sourceStart=4800;early.sourceEnd=28800;
    early.ir.path="irs/available.wav";early.ir.frames=96000;early.ir.rate=48000;
    session.swells={late,early};REQUIRE(ui::firstChronologicalSwell(session)==early.id);
    auto tie=early;tie.id=juce::Uuid().toDashedString();session.swells.push_back(tie);
    REQUIRE(ui::firstChronologicalSwell(session)==std::min(early.id,tie.id));
    SongTime timing{true,0,0,48000,120,4,4};
    const auto [first,last]=ui::initialSwellBounds(early,timing);
    REQUIRE(first==4800-95999 && last>=early.arrival && last>=early.sourceEnd);
    SwellViewport current;const auto span=current.span();
    auto revealed=ui::revealInitialSwell(current,early,timing);
    REQUIRE(revealed.span()==span && revealed.first<first && revealed.last>last);
    const auto retained=ui::revealInitialSwell(revealed,early,timing);
    REQUIRE(retained.first==revealed.first && retained.last==revealed.last);
    current={0,1000};revealed=ui::revealInitialSwell(current,early,timing);
    REQUIRE(revealed.first<first && revealed.last>last && revealed.span()>current.span());
    current={-10000000,10000000};revealed=ui::revealInitialSwell(current,early,timing);
    REQUIRE(revealed.span()<current.span() && revealed.first<first && revealed.last>last);
    current={0,(last-first)*10};revealed=ui::revealInitialSwell(current,early,timing);
    REQUIRE(revealed.span()<current.span() && revealed.first<first && revealed.last>last);
    early.phase=Phase::checking;early.render.path="renders/previous.wav";early.render.start=-2000000;
    early.render.irContent="old IR";early.ir.content="current IR";
    REQUIRE(ui::initialSwellBounds(early,timing).first==first); // Pending old wet cannot drive initial framing.
    early.ir={};early.arrival=early.sourceStart=-48000;early.sourceEnd=-24000;
    const auto negative=ui::initialSwellBounds(early,timing);REQUIRE(negative.first<=-72000 && negative.second>=-24000);
});}
