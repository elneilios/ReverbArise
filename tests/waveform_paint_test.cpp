#include "TestSupport.h"
#include <UI/SwellTimeline.h>
#include <UI/WorkspaceTheme.h>

using namespace m3;
int main(){return runTest([]
{
    juce::ScopedJuceInitialiser_GUI gui;TestDirectory root;
    SwellEngine engine(root.file,[]{return PreviewContext{};});engine.setAutomaticProcessing(false);
    SwellTimeline timeline(engine);timeline.setSize(800,380);timeline.setInlineOverview(false);
    SwellOverview overview(timeline);overview.setSize(1050,125);
    CaptureSessionSnapshot capture;capture.reference="accepted-1";capture.manifest.revision=1;
    for(int64_t start:{0,100000})
    {
        CaptureOverviewRange range;range.range.startSample=start;range.range.frameCount=1000;
        for(int n=0;n<100;++n)range.peaks.push_back({start+n*10,10,-.25f,.5f});
        capture.manifest.ranges.push_back(range.range);capture.overview.push_back(range);
    }
    auto wave=std::make_shared<Waveform>();wave->start=0;wave->end=1000;wave->peak=.5f;wave->peaks=capture.overview[0].peaks;
    EditorSnapshot state;state.sessionEpoch=1;
    for(const auto id:{"a","b"})
    {
        Swell swell;swell.id=id;swell.sourceEnd=1000;swell.arrival=0;swell.shape.endOffset=1000;swell.phase=Phase::ready;
        state.session.swells.push_back(swell);state.wetSummaries[id]=wave;
    }
    state.session.primary="a";state.raw=state.shaped=wave;
    SongTime time{true,0,0,48000,120,4,4};
    const auto paint=[](juce::Component& component,float scale=1.f)
    {
        juce::Image image(juce::Image::ARGB,static_cast<int>(std::ceil(component.getWidth()*scale)),static_cast<int>(std::ceil(component.getHeight()*scale)),true,juce::SoftwareImageType{});
        juce::Graphics g(image);g.addTransform(juce::AffineTransform::scale(scale));component.paintEntireComponent(g,true);return image;
    };
    const auto present=[&]{timeline.setData(capture,state,time);paint(timeline);paint(overview);};
    {
        auto loopCapture=capture;
        loopCapture.manifest.loop=LoopPhraseTiming{120,4,4,4,0,1000};
        auto loopState=state;
        auto selectedWave=std::make_shared<Waveform>(*wave);
        selectedWave->peaks={{300,1,-.25f,.5f}};
        loopState.raw=loopState.shaped=selectedWave;
        loopState.wetSummaries.clear();
        timeline.setData(loopCapture,loopState,time);
        timeline.setViewport({0,1000});
        const auto wetLane=timeline.wetLane();
        const auto wetPixel=[&](const juce::Image& image,int64_t sample){
            const auto x=timeline.viewport().pixel(sample,wetLane.getX(),wetLane.getWidth());
            return image.getPixelAt(static_cast<int>(x),static_cast<int>(wetLane.getCentreY())+6);
        };
        const auto selectedBefore=wetPixel(paint(timeline),300);
        REQUIRE(selectedBefore.getGreen()>ui::background.getGreen());
        loopState.loopLeadIn.start=700;
        timeline.setData(loopCapture,loopState,time);
        const auto tintOnly=wetPixel(paint(timeline),850);
        loopState.loopLeadIn.peaks=std::make_shared<const std::vector<CapturePeak>>(
            std::vector<CapturePeak>{{-150,1,-.25f,.5f}});
        timeline.setData(loopCapture,loopState,time);
        const auto playing=paint(timeline);
        REQUIRE(wetPixel(playing,300)==selectedBefore);
        const auto leadIn=wetPixel(playing,850);
        REQUIRE(leadIn.getGreen()>tintOnly.getGreen() && leadIn.getGreen()<selectedBefore.getGreen());
    }
    timeline.setViewport({0,1000});present();
    REQUIRE(timeline.waveformPaintMetrics().peaksVisited==100); // No hidden inline or distant-range work.
    REQUIRE(timeline.waveformPaintMetrics().columnsPainted==100); // Coarse buckets remain narrow at deep zoom.
    const auto dry=timeline.waveformPaintMetrics(),wet=timeline.wetPaintMetrics(),mini=overview.waveformPaintMetrics();
    time.sample=500;time.ppq=500/24000.;state.auditionSample=700;present();
    REQUIRE(timeline.waveformPaintMetrics().builds==dry.builds && timeline.wetPaintMetrics().builds==wet.builds);
    REQUIRE(overview.waveformPaintMetrics().builds==mini.builds);
    REQUIRE(timeline.waveformPaintMetrics().hits>dry.hits && overview.waveformPaintMetrics().hits>mini.hits);
    const auto cursor=paint(timeline);const auto lane=timeline.sourceLane();
    REQUIRE(cursor.getPixelAt(static_cast<int>(timeline.viewport().pixel(500,lane.getX(),lane.getWidth())),static_cast<int>(lane.getCentreY()))==ui::timing);
    const auto changed=[&](auto edit)
    {
        const auto before=timeline.waveformPaintMetrics().builds,other=overview.waveformPaintMetrics().builds;
        edit();present();REQUIRE(timeline.waveformPaintMetrics().builds>before && overview.waveformPaintMetrics().builds>other);
    };
    changed([&]{state.session.primary="b";});
    changed([&]{state.session.swells[1].shape.gainDb=6;});
    changed([&]{state.session.swells[1].shape.fadeOut=250;});
    changed([&]{state.session.swells[1].placement=-100;});
    changed([&]{state.session.swells[1].muted=true;});
    changed([&]{state.session.swells[1].phase=Phase::stale;state.shaped.reset();});
    changed([&]{auto next=std::make_shared<Waveform>(*wave);next->peaks.front().maximum=1;state.raw=state.shaped=next;state.wetSummaries["b"]=next;});
    changed([&]{++capture.manifest.revision;capture.overview[0].peaks[0].maximum=1;});
    changed([&]{capture.overview[0].available=false;});
    REQUIRE(timeline.waveformPaintMetrics().peaksVisited==0);
    // Retained detail must never cover an unavailable source range.
    changed([&]{state.sourceDetail=wave;});
    REQUIRE(timeline.waveformPaintMetrics().peaksVisited==0);
    changed([&]{capture.overview[0].available=true;});
    changed([&]{capture.overview[0].range.kind=CaptureCoverageKind::silence;});
    REQUIRE(timeline.waveformPaintMetrics().peaksVisited==0);
    changed([&]{state.sessionEpoch=2;capture.reference="restored";});
    changed([&]{timeline.setWaveZoomDb(6);});
    changed([&]{timeline.sendLookAndFeelChange();overview.sendLookAndFeelChange();});
    changed([&]{timeline.setInlineOverview(true);});
    REQUIRE(timeline.waveformPaintMetrics().peaksVisited>0);
    changed([&]{timeline.setInlineOverview(false);});
    const auto beforeView=timeline.waveformPaintMetrics().builds,overviewView=overview.waveformPaintMetrics().builds;
    timeline.setViewport({100,500});paint(timeline);paint(overview);
    REQUIRE(timeline.waveformPaintMetrics().builds>beforeView && overview.waveformPaintMetrics().builds==overviewView);
    changed([&]{timeline.setSize(900,420);overview.setSize(1100,140);});
    for(float scale:{1.25f,1.5f,2.f})
    {
        const auto before=timeline.waveformPaintMetrics().builds;
        paint(timeline,scale);paint(overview,scale);
        REQUIRE(timeline.waveformPaintMetrics().builds>before);
        const auto bytes=timeline.waveformPaintMetrics().bytes;
        REQUIRE(bytes==static_cast<uint64_t>(std::ceil(900*scale))*static_cast<uint64_t>(std::ceil(420*scale))*4);
        paint(timeline,scale);REQUIRE(timeline.waveformPaintMetrics().builds==before+1);
    }
    REQUIRE(engine.snapshot().renderCount==0 && engine.savedSession().swells.empty());
    // Exercise the real capture publisher, not just manually revised fixtures.
    // Same-sized peak arrays must invalidate after changed accepted PCM; unchanged
    // publication/restore must retain both peak values and cached images.
    CaptureSession publisher(root.file.getChildFile("capture-publisher"));
    std::vector<float> input(1024*2,.25f);
    REQUIRE(publisher.commit(0,48000,input.data(),1024));
    capture=publisher.snapshot();state={};time.sample=0;time.ppq=0;
    timeline.setViewport({0,1024});present();
    const auto firstCapture=capture;
    REQUIRE(capture.overview[0].peaks.size()==2 && capture.overview[0].peaks[0].maximum==.25f);
    changed([&]
    {
        std::fill(input.begin(),input.end(),-.75f);
        REQUIRE(publisher.commit(0,48000,input.data(),1024));capture=publisher.snapshot();
        REQUIRE(capture.manifest.revision>firstCapture.manifest.revision && capture.reference!=firstCapture.reference);
        REQUIRE(capture.overview[0].peaks.size()==firstCapture.overview[0].peaks.size());
        REQUIRE(capture.overview[0].peaks[0].minimum==-.75f && capture.overview[0].peaks[0].maximum==-.75f);
    });
    const auto unchanged=[&](auto action)
    {
        const auto before=capture;
        const auto dryBuilds=timeline.waveformPaintMetrics().builds,miniBuilds=overview.waveformPaintMetrics().builds;
        action();capture=publisher.snapshot();present();
        REQUIRE(capture.reference==before.reference && capture.manifest.revision==before.manifest.revision);
        REQUIRE(capture.overview.size()==before.overview.size());
        for(size_t r=0;r<capture.overview.size();++r)
        {
            REQUIRE(capture.overview[r].available==before.overview[r].available);
            REQUIRE(capture.overview[r].peaks.size()==before.overview[r].peaks.size());
            for(size_t p=0;p<capture.overview[r].peaks.size();++p)
            {
                const auto& a=capture.overview[r].peaks[p];const auto& b=before.overview[r].peaks[p];
                REQUIRE(a.startSample==b.startSample && a.frameCount==b.frameCount && a.minimum==b.minimum && a.maximum==b.maximum);
            }
        }
        REQUIRE(timeline.waveformPaintMetrics().builds==dryBuilds && overview.waveformPaintMetrics().builds==miniBuilds);
    };
    unchanged([&]{REQUIRE(publisher.commit(0,48000,input.data(),1024));});
    const auto committed=capture;
    unchanged([&]
    {
        REQUIRE(publisher.beginPass(67,1,true));std::fill(input.begin(),input.end(),.5f);
        REQUIRE(publisher.stagePass(67,0,48000,input.data(),1024));
        const auto prepared=publisher.preparePass(67,1);REQUIRE(prepared.success && prepared.changed);
    });
    changed([&]
    {
        REQUIRE(publisher.acceptPreparedPass(67,1));capture=publisher.snapshot();
        REQUIRE(capture.manifest.revision==committed.manifest.revision+1 && capture.reference!=committed.reference);
        REQUIRE(capture.overview[0].peaks.size()==committed.overview[0].peaks.size());
        REQUIRE(capture.overview[0].peaks[0].maximum==.5f);
    });
    unchanged([&]
    {
        REQUIRE(publisher.beginPass(68,1,true));std::fill(input.begin(),input.end(),.875f);
        REQUIRE(publisher.stagePass(68,0,48000,input.data(),1024));publisher.abortPass();
    });
    unchanged([&]{REQUIRE(publisher.restore(capture.manifest.projectId,capture.manifest.instanceId,capture.reference,capture.manifest.revision));});
    const auto saved=capture;
    const auto asset=root.file.getChildFile("capture-publisher").getChildFile(saved.manifest.projectId)
        .getChildFile(saved.manifest.instanceId).getChildFile(saved.overview[0].range.assetPath);
    juce::MemoryBlock audio;REQUIRE(asset.loadFileAsData(audio));REQUIRE(asset.deleteFile());
    changed([&]
    {
        REQUIRE(!publisher.restore(saved.manifest.projectId,saved.manifest.instanceId,saved.reference,saved.manifest.revision));
        capture=publisher.snapshot();
        REQUIRE(capture.reference==saved.reference && capture.manifest.revision==saved.manifest.revision);
        REQUIRE(capture.unavailable && !capture.overview[0].available && capture.overview[0].peaks.empty());
    });
    REQUIRE(asset.replaceWithData(audio.getData(),audio.getSize()));
    changed([&]
    {
        REQUIRE(publisher.restore(saved.manifest.projectId,saved.manifest.instanceId,saved.reference,saved.manifest.revision));
        capture=publisher.snapshot();REQUIRE(!capture.unavailable && capture.overview[0].available);
        REQUIRE(capture.overview[0].peaks[0].maximum==saved.overview[0].peaks[0].maximum);
    });
    const auto populatedOverview=paint(overview);
    const auto populatedBuilds=overview.waveformPaintMetrics().builds;
    REQUIRE(publisher.clear(48000));capture=publisher.snapshot();present();
    REQUIRE(capture.manifest.revision>saved.manifest.revision && capture.overview.empty());
    // Empty Overview bypasses waveform drawing, rather than rebuilding a bitmap.
    REQUIRE(overview.waveformPaintMetrics().builds==populatedBuilds);
    const auto emptyOverview=paint(overview);
    bool pixelsChanged=false;
    for(int y=0;y<emptyOverview.getHeight();++y)for(int x=0;x<emptyOverview.getWidth();++x)
        pixelsChanged=pixelsChanged || emptyOverview.getPixelAt(x,y)!=populatedOverview.getPixelAt(x,y);
    REQUIRE(pixelsChanged);
    std::cout<<"Native waveform cache: hidden/visible work, cursor reuse, publication/selection/shape/gain/placement/availability, restore, theme, zoom/resize/software-scale and unchanged engine checks passed.\n";
});}
