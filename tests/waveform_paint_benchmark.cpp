#include <UI/SwellTimeline.h>
#include "TestSupport.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#define NOMINMAX
#include <Windows.h>
using namespace m3;
int main(int argc,char** argv)
{
    const std::string prefix=argc>1?argv[1]:"out/issue67/raw.csv";
    std::ofstream raw(argc>1?argv[1]:"out/issue67/raw.csv");raw<<"minutes,swells,view,component,observation,ms\n";
    juce::ScopedJuceInitialiser_GUI gui;
    TestDirectory root;
    SwellEngine engine(root.file,[]{return PreviewContext{};});
    engine.setAutomaticProcessing(false);
    SwellTimeline timeline(engine);timeline.setSize(800,380);timeline.setInlineOverview(false);
    SwellOverview overview(timeline);overview.setSize(1050,125);
    std::ofstream metrics(argc>1?std::string(argv[1])+"-cache.csv":"out/issue67/cache.csv");metrics<<"minutes,swells,view,component,builds,hits,peaks_visited,columns_painted,cache_bytes\n";
    LARGE_INTEGER freq;QueryPerformanceFrequency(&freq);
    std::cout<<"minutes,swells,view,component,p50_ms,p95_ms,max_ms\n";
    for(int minutes:{3,5,20})for(int count:{10,100,200}) {
        CaptureSessionSnapshot capture;capture.manifest.sampleRate=48000;
        for(int second=0;second<minutes*60;second+=5) {
            CaptureOverviewRange range;range.range.startSample=int64_t(second)*48000;range.range.frameCount=240000;
            range.range.kind=CaptureCoverageKind::audio;capture.manifest.ranges.push_back(range.range);
            for(int n=0;n<240000;n+=512)range.peaks.push_back({range.range.startSample+n,512,-.25f,.25f});
            capture.overview.push_back(std::move(range));
        }
        EditorSnapshot state;auto wave=std::make_shared<Waveform>();wave->start=0;wave->end=192000;wave->peak=.25f;
        for(int n=0;n<192000;n+=94)wave->peaks.push_back({n,static_cast<uint32_t>(std::min(94,192000-n)),-.25f,.25f});
        for(int n=0;n<count;++n) {
            Swell w;w.id=juce::String(n);w.name=w.id;w.arrival=0;
            w.sourceStart=0;w.sourceEnd=192000;w.shape.endOffset=192000;w.placement=int64_t(n)*minutes*60*48000/count;w.phase=Phase::ready;
            w.render.path="renders/synthetic.wav";w.render.content="fixture";w.render.frames=192000;w.render.rate=48000;w.render.start=0;w.render.sourceEnd=192000;
            state.session.swells.push_back(w);state.wetSummaries[w.id]=wave;
        }
        state.session.primary="0";state.raw=state.shaped=state.sourceDetail=wave;
        timeline.setData(capture,state,{true,0,0,48000,120,4,4});
        for(bool full:{true,false}) {
            SwellViewport view;view.fit(0,full?minutes*60.*48000:4.*48000);timeline.setViewport(view);
            for(juce::Component* component:{static_cast<juce::Component*>(&timeline),static_cast<juce::Component*>(&overview)}) {
                juce::Image image(juce::Image::ARGB,component->getWidth(),component->getHeight(),true,juce::SoftwareImageType{});juce::Graphics g(image);std::vector<double> times;
                for(int n=-5;n<50;++n) {
                    if(!full)timeline.navigateOverview((n+5)%50/50.);
                    LARGE_INTEGER a,b;QueryPerformanceCounter(&a);component->paintEntireComponent(g,true);QueryPerformanceCounter(&b);
                    if(n>=-5){const auto ms=(b.QuadPart-a.QuadPart)*1000./freq.QuadPart;if(n>=0)times.push_back(ms);raw<<minutes<<','<<count<<','<<(full?"full":"zoom_scroll")<<','<<(component==&timeline?"timeline":"overview")<<','<<n<<','<<ms<<'\n';}
                }
                if(minutes==5 && count==200 && full){auto file=juce::File::getCurrentWorkingDirectory().getChildFile(juce::String(prefix)+(component==&timeline?"-timeline.png":"-overview.png"));auto stream=file.createOutputStream();if(stream)juce::PNGImageFormat().writeImageToStream(image,*stream);}
                const auto cache=component==&timeline?timeline.waveformPaintMetrics():overview.waveformPaintMetrics();
                const auto wet=component==&timeline?timeline.wetPaintMetrics():ui::WaveformPaintMetrics{};
                metrics<<minutes<<','<<count<<','<<(full?"full":"zoom_scroll")<<','<<(component==&timeline?"timeline":"overview")<<','<<cache.builds<<','<<cache.hits<<','<<cache.peaksVisited+wet.peaksVisited<<','<<cache.columnsPainted+wet.columnsPainted<<','<<cache.bytes+wet.bytes<<'\n';
                std::sort(times.begin(),times.end());std::cout<<minutes<<','<<count<<','<<(full?"full":"zoom_scroll")<<','<<(component==&timeline?"timeline":"overview")<<','<<times[24]<<','<<times[46]<<','<<times.back()<<'\n';
            }
        }
    }
}
