#include "TestSupport.h"
#include <UI/PresentationFit.h>
#include <UI/EditorPresentationState.h>
#include <climits>

using namespace m3::ui;
namespace
{
class MemoryStorage final:public PresentationStorage
{
public:
    LoadedPresentation load()override
    {return busy?LoadedPresentation{{},PresentationReadStatus::busy,juce::Result::fail("Injected contention")}:LoadedPresentation{value,PresentationReadStatus::valid};}
    juce::Result save(PresentationPatch patch)override
    {
        ++attempts;if(fail)return juce::Result::fail("Injected unavailable storage");
        REQUIRE(patch.valid());patch.apply(value);++writes;return juce::Result::ok();
    }
    PresentationPreferences value;int attempts=0,writes=0;bool fail=false,busy=false;
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
    const auto beforeBusy=path.loadFileAsString();
    FilePresentationStorage contended(path,[&](const auto& scratch,const auto& destination)
    {
        const auto exe=juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName();
        juce::ChildProcess reader;REQUIRE(reader.start(juce::StringArray{exe,"--check-busy",destination.getFullPathName()}));
        const auto finished=reader.waitForProcessToFinish(5000);if(!finished)reader.kill();
        REQUIRE(finished && reader.getExitCode()==0);REQUIRE(destination.loadFileAsString()==beforeBusy);
        return scratch.replaceFileIn(destination);
    });
    REQUIRE(contended.save({1450,{},{}}).wasOk());REQUIRE(storage.load().value.width==1450);
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
void editorState()
{
    MemoryStorage storage;storage.value={1600,1000,125};const auto retained=storage.value;
    const PresentationFitContext large{{2400,1500},{16,80}},small{{1000,750},{16,80}};
    EditorPresentationState a(storage,large),b(storage,large);
    REQUIRE(a.effective().effective==retained && !a.effective().recovered);
    REQUIRE(a.displayChanged(small));REQUIRE(a.effective().effective==(PresentationPreferences{984,670,100}));
    REQUIRE(a.desired()==retained && !a.hasPendingSave());REQUIRE(!a.canSelectScale(125) && !a.canSelectScale(75));
    REQUIRE(a.canSelectSize(900,600) && !a.canSelectSize(1100,850));
    REQUIRE(a.persistIfDue(1000,true) && storage.writes==0);REQUIRE(storage.value==retained);
    REQUIRE(b.effective().effective==retained);REQUIRE(a.displayChanged(large));REQUIRE(a.effective().effective==retained);
    REQUIRE(a.selectSize(900,600,2000));REQUIRE(a.effective().effective==(PresentationPreferences{900,600,125}));
    REQUIRE(a.manualResize({1375,1000},2100));REQUIRE(a.desired()==(PresentationPreferences{1100,800,125}));
    REQUIRE(!a.manualResize({INT_MAX,INT_MAX},2100) && !a.manualResize({-1,600},2100));
    REQUIRE(!a.manualResize({899,600},2100));REQUIRE(a.persistIfDue(2100,true));
    REQUIRE(b.desired()==retained && b.effective().effective==retained);
    REQUIRE(b.persistIfDue(3000,true) && storage.writes==1); // Unedited closing editor cannot overwrite another.
    EditorPresentationState reopened(storage,large);REQUIRE(reopened.desired()==a.desired());
    REQUIRE(reopened.selectScale(100,3000));REQUIRE(reopened.effective().effective==(PresentationPreferences{1100,800,100}));
    REQUIRE(!reopened.selectScale(90,3000));REQUIRE(reopened.persistIfDue(3750));
    REQUIRE(reopened.displayChanged({{800,500},{16,80}}));REQUIRE(!reopened.effective().supported);
    REQUIRE(!reopened.canSelectSize(900,600) && !reopened.canSelectScale(100));
    REQUIRE(!reopened.hasPendingSave());REQUIRE(reopened.displayChanged(large));
    REQUIRE(reopened.effective().effective==(PresentationPreferences{1100,800,100}));
    storage.busy=true;EditorPresentationState deferred(storage,large),edited(storage,large),explicitDefault(storage,large);
    REQUIRE(deferred.initialRead().status==PresentationReadStatus::busy);REQUIRE(!deferred.retryInitialRead());
    REQUIRE(edited.selectScale(100,4000));REQUIRE(explicitDefault.selectSize(900,600,4000));
    storage.busy=false;REQUIRE(deferred.retryInitialRead());REQUIRE(deferred.desired()==storage.value);
    REQUIRE(!deferred.retryInitialRead()); // Initial restore only, not live cross-editor synchronization.
    REQUIRE(edited.persistIfDue(4000,true) && !edited.retryInitialRead());
    REQUIRE(edited.desired()==(PresentationPreferences{}));REQUIRE(storage.value.width==1100 && storage.value.height==800);
    REQUIRE(explicitDefault.persistIfDue(4000,true));REQUIRE(storage.value==PresentationPreferences{});
    REQUIRE(deferred.desired()==(PresentationPreferences{1100,800,100}));
    REQUIRE(deferred.persistIfDue(5000,true) && storage.value==PresentationPreferences{});
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
    {
        const auto observed=storage.load();
        const auto safe=observed.status==PresentationReadStatus::valid || observed.status==PresentationReadStatus::busy;
        if(!safe)
            std::cerr<<"Concurrent preference read status "<<static_cast<int>(observed.status)<<": "<<observed.error.getErrorMessage()
                <<"; fixture exists="<<path.existsAsFile()<<"; bytes="<<path.getSize()<<"; current="<<path.loadFileAsString()<<'\n';
        REQUIRE(safe);juce::Thread::sleep(1);
    }
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
        if(argc==3 && juce::String(argv[1])=="--check-busy")
        {
            FilePresentationStorage storage{juce::File(argv[2])};
            REQUIRE(storage.load().status==PresentationReadStatus::busy);
            REQUIRE(storage.save({1200,{},{}}).failed());return;
        }
        if(argc==3 && (juce::String(argv[1])=="--write-width" || juce::String(argv[1])=="--write-height"))
        {writeField(juce::File(argv[2]),juce::String(argv[1])=="--write-width");return;}
        REQUIRE(argc==1);
        codecAndCoalescing();filesAndFaults();fitRecovery();editorState();concurrentProcesses();
        std::cout<<"Presentation codec/coalescing/atomic faults/process isolation/fit recovery checks passed.\n";
    });
}
