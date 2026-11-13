#include "AudioTestFiles.h"
#include <DSP/SwellEngine.h>
#include <DSP/DefaultImpulse.h>
#include <source_location>
using namespace m3;
template<class F>static void waitFor(F test,const std::source_location line=std::source_location::current())
{const auto end=juce::Time::getMillisecondCounter()+30000;while(!test()){if(juce::Time::getMillisecondCounter()>end)throw std::runtime_error("IR choices timeout "+std::to_string(line.line()));juce::Thread::sleep(2);}}
int main(){return runTest([]
{
    TestDirectory root;CaptureSession capture(root.file);std::vector<float> dry(4000,.1f);
    REQUIRE(capture.commit(0,1000,dry.data(),2000));
    const PreviewContext context{capture.snapshot(),{true,0,0,1000,120,4,4},0};
    SwellEngine engine(root.file,[&]{return context;});engine.setAutomaticProcessing(false);
    waitFor([&]{return engine.savedSession().defaultIR.present();});
    const auto catalogue=engine.irChoices();
    REQUIRE(catalogue.size()==bundledImpulses().size()+1);
    REQUIRE(std::any_of(catalogue.begin(),catalogue.end(),[](const auto& choice){return choice.presetId==pingPongPresetId;}));
    REQUIRE(engine.addTarget(100));const auto id=engine.savedSession().primary;
    const auto before=engine.savedSession().swells.front();
    const auto file=root.file.getChildFile("personal.wav");writeTestWav(file,1,1000,std::vector<float>(123,.003f));
    REQUIRE(engine.assignDefaultIR(file));waitFor([&]{return !engine.snapshot().defaultIRPending;});
    const auto accepted=engine.savedSession();const auto custom=accepted.defaultIR;
    REQUIRE(custom.present() && accepted.defaultIRName=="personal.wav" && accepted.importedIRs.size()==1);
    REQUIRE(sameSavedSwell(before,accepted.swells.front()));
    const auto alias=root.file.getChildFile("same-audio.wav");writeTestWav(alias,1,1000,std::vector<float>(123,.003f));
    REQUIRE(engine.assignDefaultIR(alias));waitFor([&]{return !engine.snapshot().defaultIRPending;});
    REQUIRE(engine.savedSession().defaultIR.content==custom.content && engine.savedSession().importedIRs.size()==1);
    REQUIRE(engine.assignDefaultBundledIR("monumental"));waitFor([&]{return !engine.snapshot().defaultIRPending;});
    REQUIRE(engine.savedSession().importedIRs.size()==1 && engine.irChoices().size()==catalogue.size()+1);
    REQUIRE(file.deleteFile());
    REQUIRE(engine.assignManagedIR(id,find(engine.savedSession(),id)->inputGeneration,custom,"personal.wav"));
    waitFor([&]{return find(engine.savedSession(),id)->ir==custom;});
    REQUIRE(engine.savedSession().importedIRs.size()==1);
    REQUIRE(engine.assignBundledIR(id,find(engine.savedSession(),id)->inputGeneration,"preverb-hall"));
    waitFor([&]{return find(engine.savedSession(),id)->irName=="Preverb Hall";});
    // No assignment still uses this import; history and managed bytes retain it.
    juce::String json,error;REQUIRE(encode(engine.savedSession(),json,error));Session restored;
    REQUIRE(decode(json,restored,error) && restored.importedIRs.size()==1 && restored.importedIRs[0].asset==custom);
    SwellEngine copy(root.file,[&]{return context;});copy.restore(restored);
    REQUIRE(copy.irChoices().size()==catalogue.size()+1);
    REQUIRE(copy.assignDefaultManagedIR(custom,"personal.wav"));waitFor([&]{return !copy.snapshot().defaultIRPending;});
    REQUIRE(copy.savedSession().defaultIR==custom);
    const auto other=root.file.getChildFile("other.wav");writeTestWav(other,1,1000,std::vector<float>(125,.004f));
    REQUIRE(copy.assignDefaultIR(other));waitFor([&]{return !copy.snapshot().defaultIRPending;});
    REQUIRE(copy.savedSession().importedIRs.size()==2 && engine.savedSession().importedIRs.size()==1);
    const auto retained=engine.savedSession().defaultIR;
    REQUIRE(!engine.assignDefaultIR({}));
    REQUIRE(engine.assignDefaultIR(root.file.getChildFile("invalid.wav")));waitFor([&]{return !engine.snapshot().defaultIRPending;});
    REQUIRE(engine.savedSession().defaultIR==retained && engine.savedSession().importedIRs.size()==1);
    // An already running older import cannot win or add an unaccepted history entry.
    engine.holdDefaultImportsForTesting(true);REQUIRE(engine.assignDefaultIR(other));
    waitFor([&]{return engine.defaultImportWaitingForTesting();});
    REQUIRE(engine.assignDefaultManagedIR(custom,"personal.wav"));engine.holdDefaultImportsForTesting(false);
    waitFor([&]{return !engine.snapshot().defaultIRPending;});
    REQUIRE(engine.savedSession().defaultIR==custom && engine.savedSession().importedIRs.size()==1);
    engine.holdDefaultImportsForTesting(true);REQUIRE(engine.assignDefaultIR(other));
    waitFor([&]{return engine.defaultImportWaitingForTesting();});engine.restore(restored);
    engine.holdDefaultImportsForTesting(false);waitFor([&]{return !engine.defaultImportWaitingForTesting();});
    REQUIRE(!engine.snapshot().defaultIRPending && engine.savedSession().defaultIR==restored.defaultIR && engine.savedSession().importedIRs.size()==1);
    // Restoration validates the collection transactionally and migrates known v8 choices.
    auto bad=restored;bad.importedIRs.push_back(bad.importedIRs.front());REQUIRE(!encode(bad,json,error));
    REQUIRE(encode(restored,json,error));auto duplicate=juce::JSON::parse(json);auto& list=*duplicate.getDynamicObject()->getProperty("importedIRs").getArray();
    const auto duplicateEntry=list[0];list.add(duplicateEntry);const auto unchanged=restored.defaultIR;
    REQUIRE(!decode(juce::JSON::toString(duplicate),restored,error) && restored.defaultIR==unchanged && restored.importedIRs.size()==1);
    bad=restored;bad.importedIRs[0].asset.path="../outside.wav";REQUIRE(!encode(bad,json,error));
    bad=restored;bad.importedIRs.resize(maximumImportedIRs+1);REQUIRE(!encode(bad,json,error));
    REQUIRE(encode(accepted,json,error));auto legacy=juce::JSON::parse(json);legacy.getDynamicObject()->setProperty("version",8);
    legacy.getDynamicObject()->removeProperty("importedIRs");
    REQUIRE(decode(juce::JSON::toString(legacy),restored,error) && restored.importedIRs.size()==2 && restored.importedIRs[0].asset==custom);
});}
