#include "TestSupport.h"
#include <UI/PresentationFit.h>
#include <climits>

using namespace m3::ui;
namespace
{
class MemoryStorage final:public PresentationStorage
{
public:
    LoadedPresentation load()override{return {value,PresentationReadStatus::valid};}
    juce::Result save(PresentationPatch patch)override
    {
        ++attempts;if(fail)return juce::Result::fail("Injected unavailable storage");
        REQUIRE(patch.valid());patch.apply(value);++writes;return juce::Result::ok();
    }
    PresentationPreferences value;int attempts=0,writes=0;bool fail=false;
};
void codecAndCoalescing()
{
    const PresentationPreferences baseline{};
    REQUIRE(decodePresentation("broken").value==baseline);
    REQUIRE(decodePresentation("[]").value==baseline);
    REQUIRE(decodePresentation(R"({"version":1,"contentWidth":1100,"contentHeight":"bad","scalePercent":125})").value==(PresentationPreferences{1100,600,125}));
    REQUIRE(decodePresentation(R"({"version":1,"contentWidth":900.5,"contentHeight":850,"scalePercent":75})").value==(PresentationPreferences{900,850,100}));
    REQUIRE(decodePresentation(R"({"version":1,"contentWidth":9223372036854775807,"contentHeight":-1,"scalePercent":"125"})").value==baseline);
    REQUIRE(decodePresentation(R"({"version":1,"contentWidth":true,"contentHeight":[],"scalePercent":null})").value==baseline);
    REQUIRE(decodePresentation(R"({"version":2,"contentWidth":1100})").status==PresentationReadStatus::unsupportedVersion);
    REQUIRE(decodePresentation(encodePresentation({1400,900,125})).value==(PresentationPreferences{1400,900,125}));
    MemoryStorage storage;PresentationPreferenceSession a(storage),b(storage);
    REQUIRE(a.preferences()==baseline && b.preferences()==baseline);
    REQUIRE(!a.setScale(90,0) && !a.setScale(75,0) && !a.setSize(899,600,0));
    REQUIRE(!a.hasPendingSave());
    REQUIRE(a.setSize(1000,600,0));REQUIRE(a.setSize(1100,600,100));
    REQUIRE(a.saveIfDue(849) && storage.attempts==0);REQUIRE(a.saveIfDue(850) && storage.writes==1);
    REQUIRE(b.preferences()==baseline); // Another editor cannot jump after a save.
    REQUIRE(b.setSize(900,850,1000));REQUIRE(b.saveIfDue(1000,true));
    REQUIRE(storage.value==(PresentationPreferences{1100,850,100})); // Merge changed fields, not stale records.
    REQUIRE(a.preferences()==(PresentationPreferences{1100,600,100}));
    PresentationPreferenceSession reopened(storage);REQUIRE(reopened.preferences()==storage.value);
    storage.fail=true;REQUIRE(reopened.setScale(125,2000));
    REQUIRE(!reopened.saveIfDue(2750));REQUIRE(reopened.hasPendingSave());REQUIRE(reopened.preferences().scalePercent==125);
    const auto attempts=storage.attempts;REQUIRE(reopened.saveIfDue(3499) && storage.attempts==attempts);
    storage.fail=false;REQUIRE(reopened.saveIfDue(3500));REQUIRE(!reopened.hasPendingSave());REQUIRE(storage.value.scalePercent==125);
    REQUIRE(reopened.setSize(1200,900,4000));REQUIRE(reopened.saveIfDue(4000,true)); // Explicit editor-close flush.
}
void filesAndFaults()
{
    TestDirectory directory;const auto path=directory.file.getChildFile("presentation-v1.json");
    FilePresentationStorage storage(path);REQUIRE(storage.load().status==PresentationReadStatus::missing);
    REQUIRE(storage.save({1100,850,125}).wasOk());const auto original=path.loadFileAsString();
    FilePresentationStorage reopened(path);REQUIRE(reopened.load().value==(PresentationPreferences{1100,850,125}));
    REQUIRE(storage.save({899,{},75}).failed());REQUIRE(path.loadFileAsString()==original);
    bool observed=false;
    FilePresentationStorage brokenPublisher(path,[&](const auto& scratch,const auto& destination)
    {observed=true;REQUIRE(destination.loadFileAsString()==original);REQUIRE(decodePresentation(scratch.loadFileAsString()).value.width==1200);return false;});
    REQUIRE(brokenPublisher.save({1200,{},{}}).failed() && observed);REQUIRE(path.loadFileAsString()==original);
    REQUIRE(directory.file.findChildFiles(juce::File::findFiles,false).size()==1);
    {
        juce::FileInputStream pinnedOld(path);REQUIRE(!pinnedOld.failedToOpen());
        REQUIRE(storage.save({1400,{},{}}).wasOk());REQUIRE(storage.load().value.width==1400);
        REQUIRE(pinnedOld.readEntireStreamAsString()==original);
    }
    REQUIRE(path.replaceWithText(R"({"version":99,"contentWidth":1400})"));const auto future=path.loadFileAsString();
    REQUIRE(storage.save({1000,{},{}}).failed());REQUIRE(path.loadFileAsString()==future);
    REQUIRE(path.replaceWithText(R"({"version":1,"contentWidth":"bad","contentHeight":900,"scalePercent":125})"));
    REQUIRE(storage.load().value==(PresentationPreferences{900,900,125}));REQUIRE(storage.save({1000,{},{}}).wasOk());
    REQUIRE(storage.load().value==(PresentationPreferences{1000,900,125}));
    REQUIRE(path.replaceWithText(juce::String::repeatedString("x",17000)));const auto huge=path.loadFileAsString();
    REQUIRE(storage.load().status==PresentationReadStatus::failed);REQUIRE(storage.save({1100,{},{}}).failed());REQUIRE(path.loadFileAsString()==huge);
    const auto blocked=directory.file.getChildFile("not-a-directory");REQUIRE(blocked.replaceWithText("retain"));
    FilePresentationStorage unavailable(blocked.getChildFile("presentation.json"));REQUIRE(unavailable.save({1100,{},{}}).failed());REQUIRE(blocked.loadFileAsString()=="retain");
}
void fitRecovery()
{
    const PresentationFitContext desktop{{1920,1080},{16,80}};
    const PresentationPreferences desired{1600,1000,125};const auto saved=desired;
    REQUIRE(presentationExtent({900,600,125})==(PresentationExtent{1125,750}));
    REQUIRE(presentationFits({900,600,125},{{1141,830},{16,80}}));
    REQUIRE(!presentationFits({900,600,125},{{1140,830},{16,80}}));
    REQUIRE(!presentationFits({900,600,75},desktop));
    auto fit=fitPresentation(desired,desktop);REQUIRE(fit.supported && fit.recovered && fit.effective==(PresentationPreferences{1523,800,125}));
    fit=fitPresentation(desired,{{1000,750},{16,80}});REQUIRE(fit.supported && fit.effective==(PresentationPreferences{984,670,100}));
    REQUIRE(desired==saved); // Effective recovery never overwrites retained intent.
    fit=fitPresentation(desired,{{2400,1500},{16,80}});REQUIRE(fit.supported && !fit.recovered && fit.effective==desired);
    fit=fitPresentation(desired,{{900,600},{16,80}});REQUIRE(!fit.supported && fit.effective==(PresentationPreferences{}));
    REQUIRE(!fitPresentation(desired,{{-1,-1},{INT_MAX,INT_MAX}}).supported);
    REQUIRE(fitPresentation(desired,{{INT_MAX,INT_MAX},{0,0}}).effective==desired);
    fit=fitPresentation({INT_MAX,-1,75},desktop);REQUIRE(fit.supported && fit.recovered && fit.effective==PresentationPreferences{});
    // Synthetic work areas already converted by the host/desktop scale path.
    // These are policy cases, not physical Windows DPI acceptance evidence.
    REQUIRE(fitPresentation(desired,{{1920,1032},{16,80}}).effective==(PresentationPreferences{1523,761,125}));
    REQUIRE(fitPresentation(desired,{{1536,826},{16,80}}).effective==(PresentationPreferences{1520,746,100}));
    REQUIRE(fitPresentation(desired,{{1280,688},{16,80}}).effective==(PresentationPreferences{1264,608,100}));
    REQUIRE(!fitPresentation(desired,{{960,516},{16,80}}).supported);
    for(int scale:{100,125})for(int width:{900,1100,1400,1600})for(int height:{600,850,900,1000})
    {
        const PresentationPreferences choice{width,height,scale};const auto actual=presentationExtent(choice);
        REQUIRE(presentationFits(choice,{{actual.width+16,actual.height+80},{16,80}}));
        REQUIRE(!presentationFits(choice,{{actual.width+15,actual.height+80},{16,80}}));
        REQUIRE(!presentationFits(choice,{{actual.width+16,actual.height+79},{16,80}}));
    }
}
void writeField(const juce::File& path,bool width)
{
    FilePresentationStorage storage(path);
    for(int i=0;i<20;++i)
    {
        const PresentationPatch patch=width?PresentationPatch{1100+i,{},{}}:PresentationPatch{{},700+i,{}};
        bool saved=false;
        for(int attempt=0;attempt<500 && !saved;++attempt){saved=storage.save(patch).wasOk();if(!saved)juce::Thread::sleep(1);}
        REQUIRE(saved);
    }
}
void concurrentProcesses()
{
    TestDirectory directory;const auto path=directory.file.getChildFile("preferences.json");FilePresentationStorage storage(path);
    REQUIRE(storage.save({900,600,100}).wasOk());
    const auto exe=juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName();
    struct Processes
    {
        juce::ChildProcess width,height;
        ~Processes(){if(width.isRunning())width.kill();if(height.isRunning())height.kill();}
    } processes;
    auto& width=processes.width;auto& height=processes.height;
    REQUIRE(width.start(juce::StringArray{exe,"--write-width",path.getFullPathName()}));
    REQUIRE(height.start(juce::StringArray{exe,"--write-height",path.getFullPathName()}));
    const auto deadline=juce::Time::getMillisecondCounterHiRes()+20000;
    while((width.isRunning() || height.isRunning()) && juce::Time::getMillisecondCounterHiRes()<deadline)
    {REQUIRE(storage.load().status==PresentationReadStatus::valid);juce::Thread::sleep(1);}
    const auto timedOut=width.isRunning() || height.isRunning();
    if(timedOut){width.kill();height.kill();}
    REQUIRE(!timedOut);REQUIRE(width.getExitCode()==0 && height.getExitCode()==0);
    REQUIRE(storage.load().value==(PresentationPreferences{1119,719,100}));
}
}
int main(int argc,char** argv)
{
    return runTest([&]
    {
        if(argc==3 && (juce::String(argv[1])=="--write-width" || juce::String(argv[1])=="--write-height"))
        {writeField(juce::File(argv[2]),juce::String(argv[1])=="--write-width");return;}
        REQUIRE(argc==1);
        codecAndCoalescing();filesAndFaults();fitRecovery();concurrentProcesses();
        std::cout<<"Presentation codec/coalescing/atomic faults/process isolation/fit recovery checks passed.\n";
    });
}
