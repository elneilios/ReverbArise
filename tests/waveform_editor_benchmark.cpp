
#include "AudioTestFiles.h"
#include "WorkflowTestSupport.h"
#include <Plugin/PluginEditor.h>
using namespace m3;
struct Host:juce::AudioPlayHead {PositionInfo position;juce::Optional<PositionInfo> getPosition()const override{return position;}};
template<class F>void waitFor(F f){const auto end=juce::Time::getMillisecondCounter()+60000;while(!f()){REQUIRE(juce::Time::getMillisecondCounter()<end);juce::Thread::sleep(2);}}
int main(int argc,char** argv){return runTest([&]{
 const bool review=argc>1 && (juce::String(argv[1])=="--review" || juce::String(argv[1])=="--capture");
 juce::ScopedJuceInitialiser_GUI gui;TestDirectory root;Host host;auto p=std::make_unique<PreverbAudioProcessor>(root.file);
 p->setPlayHead(&host);p->prepareToPlay(48000,4096);auto& engine=p->swellEngine();engine.setAutomaticProcessing(false);engine.setPostRoll(.25);
 host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});host.position.setIsPlaying(true);
 juce::AudioBuffer<float> audio(2,4096);juce::MidiBuffer midi;
 REQUIRE(setFixtureCaptureArmed(*p));
 for(int block=0;block<48;++block){host.position.setTimeInSamples(block*4096);host.position.setPpqPosition(block*4096/24000.);for(int c=0;c<2;++c)for(int i=0;i<4096;++i){const auto at=block*4096+i,local=at%48000;audio.setSample(c,i,review?static_cast<float>((c==0?.25:.175)*std::exp(-local/6500.)*std::sin(at*.07)):.125f);}p->processBlock(audio,midi);REQUIRE(p->flushCaptureForTesting());}
 finishCaptureFixture(*p,host.position);REQUIRE(p->flushCaptureForTesting());waitFor([&]{return engine.savedSession().defaultIR.present();});REQUIRE(engine.addTarget(48000));waitFor([&]{return engine.snapshot().auditionReady;});const auto original=engine.savedSession();
 std::cout<<"definitions,observation,refresh_and_paint_ms\n";
 for(int count:{1,200}){auto session=original;while(session.swells.size()<count){auto w=session.swells.front();w.id=juce::Uuid().toString();session.swells.push_back(w);}engine.restore(session);waitFor([&]{const auto s=engine.snapshot();return s.raw && s.auditionReady;});
 auto panel=std::make_unique<PreverbAudioProcessorEditor>(*p);if(argc<=1 || juce::String(argv[1])!="--review")panel->stopAutomaticRefreshForReview();panel->setSize(1100,850);panel->refresh();
 juce::Image surface(juce::Image::ARGB,1100,850,true,juce::SoftwareImageType{});juce::Graphics graphics(surface);
 if(review && count==1)
 {
     class Window final:public juce::DocumentWindow
     {
     public:Window(juce::Component& content):DocumentWindow("Issue #67 - Waveform paint review",m3::ui::background,closeButton)
         {setUsingNativeTitleBar(true);setContentNonOwned(&content,true);setResizable(true,false);centreWithSize(1100,850);setVisible(true);}
         void closeButtonPressed()override{juce::MessageManager::getInstance()->stopDispatchLoop();}
     };
     juce::Image picture(juce::Image::ARGB,panel->getWidth(),panel->getHeight(),true,juce::SoftwareImageType{});
     {juce::Graphics drawing(picture);panel->paintEntireComponent(drawing,true);}
     auto imageFile=juce::File::getCurrentWorkingDirectory().getChildFile("out/issue67/native-review.png").createOutputStream();
     REQUIRE(imageFile && imageFile->setPosition(0) && imageFile->truncate().wasOk() && juce::PNGImageFormat().writeImageToStream(picture,*imageFile));
     imageFile->flush();imageFile.reset();
     if(juce::String(argv[1])=="--capture")return;
     Window window(*panel);juce::MessageManager::getInstance()->runDispatchLoop();return;
 }
 juce::String before,error;REQUIRE(encode(engine.savedSession(),before,error));const auto source=p->getCaptureSnapshot().reference;
 for(int n=-5;n<50;++n){const auto start=juce::Time::getMillisecondCounterHiRes();panel->refresh();panel->paintEntireComponent(graphics,true);const auto elapsed=juce::Time::getMillisecondCounterHiRes()-start;if(n>=0)std::cout<<count<<','<<n<<','<<elapsed<<'\n';}
 juce::String after;REQUIRE(encode(engine.savedSession(),after,error));REQUIRE(before==after && source==p->getCaptureSnapshot().reference);
 }
 p->setPlayHead(nullptr);
});}
