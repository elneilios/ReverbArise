#pragma once
static void performNavigationStateCases(const m3::Session& source)
{
    using namespace m3;
    auto expected=source;expected.performUi={true,false,-54321.25,765432.5,GridDivision::twoBars};
    juce::String json,error;REQUIRE(encode(expected,json,error));Session decoded;
    REQUIRE(decode(json,decoded,error) && decoded.performUi==expected.performUi && decoded.exportState==expected.exportState);
    const auto musical=[&](const Session& state){REQUIRE(state.primary==source.primary && state.selected==source.selected && state.swells.size()==source.swells.size());
        for(size_t n=0;n<state.swells.size();++n){const auto& a=state.swells[n];const auto& b=source.swells[n];REQUIRE(a.id==b.id && a.render==b.render && a.ir==b.ir && a.shape==b.shape && a.sourceStart==b.sourceStart && a.sourceEnd==b.sourceEnd && a.arrival==b.arrival);}};
    musical(decoded);
    for(int malformed=0;malformed<6;++malformed)
    {
        auto object=juce::JSON::parse(json);auto preferences=object["performUi"];
        if(malformed==0)object.getDynamicObject()->removeProperty("performUi");
        if(malformed==1)object.getDynamicObject()->setProperty("performUi","invalid");
        if(malformed==2)preferences.getDynamicObject()->setProperty("snap","false");
        if(malformed==3)preferences.getDynamicObject()->setProperty("grid","unknown-grid");
        if(malformed==4)preferences["view"].getDynamicObject()->setProperty("last","nan");
        if(malformed==5)preferences["view"].getDynamicObject()->setProperty("last",-100000);
        REQUIRE(decode(juce::JSON::toString(object),decoded,error));musical(decoded);REQUIRE(decoded.exportState==expected.exportState);
        if(malformed<2)REQUIRE(decoded.performUi==PerformNavigationState{});
        if(malformed==2)REQUIRE(decoded.performUi.snap && decoded.performUi.grid==GridDivision::twoBars && decoded.performUi.hasView);
        if(malformed==3)REQUIRE(!decoded.performUi.snap && decoded.performUi.grid==GridDivision::quarterBeat && decoded.performUi.hasView);
        if(malformed>=4)REQUIRE(!decoded.performUi.hasView && !decoded.performUi.snap && decoded.performUi.grid==GridDivision::twoBars);
    }
    auto legacy=juce::JSON::parse(json);legacy.getDynamicObject()->setProperty("version",9);
    legacy.getDynamicObject()->removeProperty("performUi");legacy.getDynamicObject()->removeProperty("export");
    REQUIRE(decode(juce::JSON::toString(legacy),decoded,error) && decoded.performUi==PerformNavigationState{});musical(decoded);
    auto broken=juce::JSON::parse(json);broken.getDynamicObject()->setProperty("export","invalid");REQUIRE(!decode(juce::JSON::toString(broken),decoded,error));
    REQUIRE(decode(json,decoded,error));auto copy=decoded;copy.performUi.snap=true;
    REQUIRE(!decoded.performUi.snap && copy.performUi.snap);
    auto invalid=expected.performUi;invalid.last=std::numeric_limits<double>::infinity();
    REQUIRE(!normalizedPerformNavigation(invalid).hasView);
}
