# Issue #25 disposable native scheduler benchmark

This is benchmark documentation, not a plugin target. Extract the C++ block below
to `out/audit/scheduler.cpp`, compile Release against the **same revision's** built
libraries and JUCE headers, and write stdout to CSV. Do not alter a production
callback. Run after the build wrapper exits, without competing builds. Use the
Release include paths, definitions and linker dependencies from
`preverb_transport_playback_test.vcxproj`; add `/O2 /std:c++20 /EHsc /MD` and
`/I tests`. Output objects/executable only under `out/audit/`.

The harness invokes the actual TransportPlayback implementation, not a simulated
mixer. Default fixtures share identical immutable PCM to keep setup small;
publication accounting therefore intentionally overcounts this shared memory.
Pass `distinct` to repeat just 200 definitions / 64 overlapping voices with
separately allocated PCM for each voice and 1/4 players. This checks the larger
memory footprint hidden by shared-PCM setup. Save it as `scheduler-distinct.csv`.
It measures
component execution, excluding the host, processor diagnostics, capture, page
preparation and UI. QPC surrounds the call; timing and allocation counters are
read outside it. Normal-priority, unpaced calls cannot establish DAW dropouts.
Percentiles use sorted observations (index floor(p*(N-1))). Each row has 100
warmups and 1000 observations; every 128 blocks the position seeks back to zero.
Totals above overlap place the remaining swells far in the future.

```cpp
#include <Playback/TransportPlayback.h>
#include "AllocationGuard.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <vector>
#define NOMINMAX
#include <Windows.h>
using namespace m3;
int main(int argc,char**)
{
    const bool distinct=argc>1;
    LARGE_INTEGER freq; QueryPerformanceFrequency(&freq);
    std::cout << "rate,block,total,overlap,instances,mode,p50_us,p95_us,p99_us,max_us,deadline_us,overruns,allocations,deallocations\n";
    for (double rate : {44100.,48000.,96000.})
    for (int block : {64,128,512})
    for (int total : {0,10,50,100,200})
    for (int overlap : {0,1,10,50,64}) {
        if (overlap>total) continue;
        if (distinct && !(total==200 && overlap==64)) continue;
        // Additional multi-instance rows use only the dense 200/64 case.
        for (int instances : {1,2,4}) {
        if (distinct && instances==2) continue;
        if (instances>1 && !(total==200 && overlap==64)) continue;
        for (bool allows : {true,false}) {
        if (distinct && !allows) continue;
        if (!allows && !(total==200 && overlap==64 && instances==1)) continue;
        std::vector<std::unique_ptr<TransportPlayback>> players;
        const SongTime mapping{true,0,0,rate,120,4,4};
        auto audio=std::make_shared<WetRender>(); audio->start=0;audio->rate=rate;
        audio->samples.resize(131072*2,.001f);
        for (int instance=0;instance<instances;++instance) {
            auto player=std::make_unique<TransportPlayback>();
            auto pub=std::make_unique<LivePublication>();pub->mapping=mapping;pub->start=0;pub->end=131072;
            for (int i=0;i<total;++i) {
                const int64_t start=i<overlap?0:1000000+int64_t(i)*262144;
                std::shared_ptr<WetRender> pcm=i<overlap?(distinct?std::make_shared<WetRender>(*audio):audio):nullptr;
                pub->entries.push_back({size_t(i),1,start,start+131072,.001f,pcm,{}});
                player->invalidate(i,1);
            }
            juce::String error;
            if (!prepareLiveSegments(*pub,error) || !player->publish(std::move(pub))) return 2;
            players.push_back(std::move(player));
        }
        std::array<float,512> l{},r{};std::vector<double> times;times.reserve(1000);
        unsigned allocations=0,deallocations=0,overruns=0;
        const double deadline=block*1000000./rate;
        for(int n=-100;n<1000;++n) {
            auto song=mapping;song.sample=int64_t((n+100)%128)*block;song.ppq=song.sample*2./rate;
            TransportTime time{song,true};l.fill(0);r.fill(0);
            LARGE_INTEGER before,after;
            {
                AudioAllocationGuard guard;
                QueryPerformanceCounter(&before);
                for(auto& p:players)p->process(l.data(),r.data(),block,time,allows);
                QueryPerformanceCounter(&after);
            }
            if(n>=0) {
                const auto us=(after.QuadPart-before.QuadPart)*1000000./freq.QuadPart;
                times.push_back(us);overruns+=us>deadline;
                allocations+=watchedAllocations;deallocations+=watchedDeallocations;
            }
        }
        std::sort(times.begin(),times.end());
        std::cout<<rate<<','<<block<<','<<total<<','<<overlap<<','<<instances<<','<<(allows?"perform":"workflow_bypass")<<','
            <<times[499]<<','<<times[949]<<','<<times[989]<<','<<times.back()<<','<<deadline<<','<<overruns<<','<<allocations<<','<<deallocations<<'\n';
        }
        }
    }
    // Verify the explicit overlap boundary independently of timing rows.
    LivePublication excess;excess.start=0;excess.end=100;
    for(size_t n=0;n<65;++n)excess.entries.push_back({n,1,0,100,.01f,{},{}});
    juce::String error;
    if(prepareLiveSegments(excess,error) || !error.contains("64 simultaneous"))return 3;
}
```

Run at least three separate invocations. Preserve all rows, including maxima and
deadline misses; do not discard OS scheduling outliers. For a follow-up change,
compare the same source fixtures, compiler flags, machine and idle conditions.
Measure full `processBlock()` and paced multi-instance playback separately before
claiming a host performance improvement. Allocation guards observe C++ new/delete,
not every possible CRT/driver allocation, lock, page fault or scheduler wait.

## Disposable software UI benchmark

Extract this second block to `out/audit/ui.cpp`. Compile with the Release
settings/dependencies of `preverb_swell_ui_test.vcxproj` (including generated
plugin headers), outputting only to `out/audit/`. This exercises the actual
components using synthetic summaries, not real audio or the unmerged Learn
workspace. Each row has five warmups and 50 paints into reused software images.
It excludes compositor, input latency, DPI changes and the editor's 10 Hz refresh.
The timeline has its inline overview disabled as in the current editor.

```cpp
#include <UI/SwellTimeline.h>
#include "TestSupport.h"
#include <algorithm>
#include <iostream>
#define NOMINMAX
#include <Windows.h>
using namespace m3;
int main()
{
    juce::ScopedJuceInitialiser_GUI gui;
    TestDirectory root;
    SwellEngine engine(root.file,[]{return PreviewContext{};});
    engine.setAutomaticProcessing(false);
    SwellTimeline timeline(engine);timeline.setSize(800,380);timeline.setInlineOverview(false);
    SwellOverview overview(timeline);overview.setSize(1050,125);
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
                juce::Image image(juce::Image::ARGB,component->getWidth(),component->getHeight(),true);juce::Graphics g(image);std::vector<double> times;
                for(int n=-5;n<50;++n) {
                    if(!full)timeline.navigateOverview((n+5)%50/50.);
                    LARGE_INTEGER a,b;QueryPerformanceCounter(&a);component->paintEntireComponent(g,true);QueryPerformanceCounter(&b);
                    if(n>=0)times.push_back((b.QuadPart-a.QuadPart)*1000./freq.QuadPart);
                }
                std::sort(times.begin(),times.end());std::cout<<minutes<<','<<count<<','<<(full?"full":"zoom_scroll")<<','<<(component==&timeline?"timeline":"overview")<<','<<times[24]<<','<<times[46]<<','<<times.back()<<'\n';
            }
        }
    }
}
```

## Disposable capture and immutable-write benchmark

Extract the third block to `out/audit/assets.cpp`. It uses the real transactional
capture session and asset store in a UUID temporary directory that its fixture
deletes on completion. It never reads an existing user cache. Process memory
peaks are cumulative for this executable; private bytes are sampled after each
phase, not a complete transient-allocation trace. Source PCM is one reused
five-second stereo block, not an in-memory full song. File totals exclude the
test harness itself. The protected pass replays changed input with Recapture off;
the explicit recapture replaces it while retaining immutable old files.

```cpp
#include "TestSupport.h"
#include <Capture/CaptureSession.h>
#include <DSP/SwellAssetStore.h>
#define NOMINMAX
#include <Windows.h>
#include <psapi.h>
int main(){return runTest([]{
    std::cout<<"minutes,phase,elapsed_ms,files,disk_bytes,private_bytes,cumulative_peak_working_set_bytes\n";
    for(int minutes:{3,5}) {
        TestDirectory root;CaptureSession session(root.file);
        std::vector<float> pcm(240000*2,.125f);
        const auto record=[&](const char* phase,double elapsed){
            int files=0;int64_t bytes=0;
            for(const auto& f:root.file.findChildFiles(juce::File::findFiles,true)){++files;bytes+=f.getSize();}
            PROCESS_MEMORY_COUNTERS_EX memory{};memory.cb=sizeof(memory);
            REQUIRE(GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory),sizeof(memory)));
            std::cout<<minutes<<','<<phase<<','<<elapsed<<','<<files<<','<<bytes<<','<<memory.PrivateUsage<<','<<memory.PeakWorkingSetSize<<'\n';
        };
        for(int pass=1;pass<=3;++pass) {
            if(pass==2)std::fill(pcm.begin(),pcm.end(),.25f);
            const auto started=juce::Time::getMillisecondCounterHiRes();
            REQUIRE(session.beginPass(pass,0,pass==3));
            for(int second=0;second<minutes*60;second+=5)
                REQUIRE(session.stagePass(pass,int64_t(second)*48000,48000,pcm.data(),240000));
            const auto result=session.finishPass(pass,0);REQUIRE(result.success);
            REQUIRE(result.changed==(pass!=2));
            record(pass==1?"capture":pass==2?"protected_replay":"explicit_recapture",juce::Time::getMillisecondCounterHiRes()-started);
        }
        const auto& map=session.snapshot().manifest;
        m3::SwellAssetStore store(root.file.getChildFile(map.projectId).getChildFile(map.instanceId));
        ImpulseResponse ir;ir.channels=1;ir.rate=48000;ir.samples={1,.5f};m3::Asset first,second;juce::String error;
        const auto writesStarted=juce::Time::getMillisecondCounterHiRes();
        REQUIRE(store.writeIR(ir,first,error) && store.writeIR(ir,second,error));
        REQUIRE(first.content==second.content && first.path!=second.path);
        record("two_identical_direct_ir_writes",juce::Time::getMillisecondCounterHiRes()-writesStarted);
    }
});}
```

## Disposable full-processor baseline

The fourth block uses a real processor with a synthetic host, disposable one-second
capture and one prepared default-IR swell. Capture setup is drained before timing.
Learn timing is **stopped/idle**; Perform and Export timing are playing. No claim
about active Learn throughput follows from this baseline. Each row has 100 warmups
and 1000 timed callbacks; seeks every 128 blocks are included. No editor or real
audio driver is involved. Synthetic host queries allocate nothing.

```cpp
#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Plugin/PluginProcessor.h>
#define NOMINMAX
#include <Windows.h>
struct Host:juce::AudioPlayHead {PositionInfo position;juce::Optional<PositionInfo> getPosition()const override{return position;}};
template<class F>void until(F f){const auto end=juce::Time::getMillisecondCounter()+30000;while(!f()){REQUIRE(juce::Time::getMillisecondCounter()<end);juce::Thread::sleep(2);}}
int main(){return runTest([]{
    juce::ScopedJuceInitialiser_GUI gui;LARGE_INTEGER frequency;QueryPerformanceFrequency(&frequency);
    std::cout<<"rate,block,mode,p50_us,p95_us,p99_us,max_us,deadline_us,overruns,allocations,deallocations\n";
    for(double rate:{44100.,48000.,96000.}) {
        TestDirectory root;auto p=std::make_unique<PreverbAudioProcessor>(root.file);auto& engine=p->swellEngine();engine.setAutomaticProcessing(false);
        p->prepareToPlay(rate,512);Host host;p->setPlayHead(&host);juce::MidiBuffer midi;juce::AudioBuffer<float> buffer(2,512);
        const auto position=[&](int64_t at,bool playing){host.position.setTimeInSamples(at);host.position.setPpqPosition(at*2./rate);host.position.setBpm(120);host.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});host.position.setIsPlaying(playing);};
        for(int64_t at=0;at<rate;at+=512){position(at,true);for(int c=0;c<2;++c)std::fill_n(buffer.getWritePointer(c),512,.125f);p->processBlock(buffer,midi);juce::Thread::sleep(2);}
        position(static_cast<int64_t>(rate),false);buffer.clear();p->processBlock(buffer,midi);REQUIRE(p->flushCaptureForTesting());
        REQUIRE(p->getWorkflowSnapshot().mode==workflow::Mode::live);
        until([&]{return engine.savedSession().defaultIR.present();});REQUIRE(engine.addTarget(0));engine.generate();until([&]{return engine.snapshot().auditionReady;});
        position(-static_cast<int64_t>(rate),false);p->processBlock(buffer,midi);until([&]{return engine.snapshot().liveReady;});
        const auto accepted=p->getCaptureSnapshot().reference;
        for(auto mode:{workflow::Mode::capture,workflow::Mode::live,workflow::Mode::finalise}) {
            REQUIRE(p->requestWorkflowMode(mode));
            for(int frames:{64,128,512}) {
                buffer.setSize(2,frames);std::vector<double> times;times.reserve(1000);unsigned allocations=0,deallocations=0,overruns=0;
                const auto deadline=frames*1000000./rate;
                for(int n=-100;n<1000;++n) {
                    position(-static_cast<int64_t>(rate)+int64_t((n+100)%128)*frames,mode!=workflow::Mode::capture);buffer.clear();LARGE_INTEGER a,b;
                    {AudioAllocationGuard guard;QueryPerformanceCounter(&a);p->processBlock(buffer,midi);QueryPerformanceCounter(&b);}
                    if(n>=0){const auto us=(b.QuadPart-a.QuadPart)*1000000./frequency.QuadPart;times.push_back(us);overruns+=us>deadline;allocations+=watchedAllocations;deallocations+=watchedDeallocations;}
                }
                std::sort(times.begin(),times.end());std::cout<<rate<<','<<frames<<','<<(mode==workflow::Mode::capture?"learn_stopped":mode==workflow::Mode::live?"perform":"export")<<','<<times[499]<<','<<times[949]<<','<<times[989]<<','<<times.back()<<','<<deadline<<','<<overruns<<','<<allocations<<','<<deallocations<<'\n';
            }
        }
        REQUIRE(p->flushCaptureForTesting() && p->getCaptureSnapshot().reference==accepted);
    }
});}
```

## Wall-time tail diagnostic

The fifth block repeats the sparse 96 kHz / 64-frame component case one million
times on an ordinary thread. QPC measures elapsed time; QueryThreadCycleTime
samples charged thread cycles around the same call. This can distinguish large
wall-time tails with little charged work from consistently expensive processing;
it cannot identify a driver, fault or scheduler cause without tracing. Cycle
counts include the timing-call overhead and vary with CPU frequency. The raw
summary retains the charged cycles associated with the largest wall-time sample.

```cpp
#include <Playback/TransportPlayback.h>
#define NOMINMAX
#include <Windows.h>
#include <algorithm>
#include <iostream>
#include <vector>
using namespace m3;
int main(){
    SongTime song{true,0,0,96000,120,4,4};TransportPlayback player;
    auto pub=std::make_unique<LivePublication>();pub->mapping=song;pub->start=0;pub->end=16384;
    auto audio=std::make_shared<WetRender>();audio->rate=96000;audio->samples.resize(16384*2,.001f);
    for(size_t n=0;n<100;++n){const int64_t start=n?1000000+int64_t(n)*32768:0;
        pub->entries.push_back({n,1,start,start+16384,.001f,n?nullptr:audio,{}});player.invalidate(n,1);}
    juce::String error;if(!prepareLiveSegments(*pub,error) || !player.publish(std::move(pub)))return 2;
    LARGE_INTEGER frequency;QueryPerformanceFrequency(&frequency);const HANDLE thread=GetCurrentThread();
    std::vector<double> wall;wall.reserve(1000000);std::vector<ULONG64> cycles;cycles.reserve(1000000);
    double largest=0;ULONG64 atLargest=0;unsigned overruns=0;float l[64]{},r[64]{};
    for(int n=-100;n<1000000;++n){song.sample=int64_t((n+100)%128)*64;song.ppq=song.sample*2./song.rate;TransportTime time{song,true};
        LARGE_INTEGER a,b;ULONG64 before,after;if(!QueryThreadCycleTime(thread,&before))return 3;
        QueryPerformanceCounter(&a);player.process(l,r,64,time);QueryPerformanceCounter(&b);
        if(!QueryThreadCycleTime(thread,&after))return 3;
        if(n>=0){const auto us=(b.QuadPart-a.QuadPart)*1000000./frequency.QuadPart;const auto used=after-before;
            wall.push_back(us);cycles.push_back(used);overruns+=us>64000000./96000;
            if(us>largest){largest=us;atLargest=used;}}
    }
    std::sort(wall.begin(),wall.end());std::sort(cycles.begin(),cycles.end());
    std::cout<<"observations,p50_us,p99_us,max_us,deadline_us,overruns,median_thread_cycles,max_thread_cycles,thread_cycles_at_max_wall\n"
        <<wall.size()<<','<<wall[499999]<<','<<wall[989999]<<','<<largest<<",666.667,"<<overruns<<','<<cycles[499999]<<','<<cycles.back()<<','<<atLargest<<'\n';
}
```

## Extraction and compilation used for this run

Run from the isolated checkout after `scripts/build.ps1 -Configuration Release
-Test` completes. This reuses completed libraries; it does not reconfigure CMake,
change a target or rebuild the plugin. The VS path is the installed toolchain on
the audit machine; adjust only that path for another machine. Run binaries
serially and capture three scheduler CSVs and one UI CSV.

```powershell
param([ValidateSet('scheduler','ui','assets','processor','cycles')] [string]$Kind='scheduler')
$ErrorActionPreference='Stop'
$root=(Get-Location).Path
$binary=Join-Path $root 'out/build/windows-msvc'
$audit=Join-Path $root 'out/audit'
New-Item -ItemType Directory -Force -Path $audit | Out-Null
$markdown=Get-Content docs/validation/2026-11-03-performance-harness.md -Raw
$blocks=[regex]::Matches($markdown,'(?ms)^```cpp\r?\n(.*?)^```\r?$')
$index=if($Kind -eq 'scheduler'){0}elseif($Kind -eq 'ui'){1}elseif($Kind -eq 'assets'){2}elseif($Kind -eq 'processor'){3}else{4}
$source=Join-Path $audit "$Kind.cpp"
[IO.File]::WriteAllText($source,$blocks[$index].Groups[1].Value)
$target=if($Kind -in @('ui','processor')){'preverb_swell_ui_test'}else{'preverb_transport_playback_test'}
[xml]$project=Get-Content (Join-Path $binary "$target.vcxproj")
$settings=$project.Project.ItemDefinitionGroup | Where-Object Condition -Like '*Release|x64*'
$vs='C:/Program Files/Microsoft Visual Studio/18/Community'
Import-Module "$vs/Common7/Tools/Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vs -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null
$arguments=@('/nologo','/O2','/std:c++20','/EHsc','/MD','/utf-8',"/I`"$root/tests`"")
foreach($include in $settings.ClCompile.AdditionalIncludeDirectories.Split(';')) {
    if($include -notlike '%*'){$arguments+="/I`"$include`""}
}
foreach($define in $settings.ClCompile.PreprocessorDefinitions.Split(';')) {
    if($define -notlike '%*' -and $define -notlike 'CMAKE_INTDIR*'){$arguments+="/D`"$define`""}
}
$arguments+=@("`"$source`"","/Fo`"$audit/$Kind.obj`"","/Fe`"$audit/$Kind.exe`"")
foreach($library in $settings.Link.AdditionalDependencies.Split(';')) {
    $path=if($library.Contains('\')){Join-Path $binary $library}else{$library}
    $arguments+="`"$path`""
}
$response=Join-Path $audit "$Kind.rsp"
[IO.File]::WriteAllLines($response,$arguments)
& cl.exe "@$response" /link /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /MACHINE:X64 psapi.lib
if($LASTEXITCODE -ne 0){throw "Audit compilation failed: $LASTEXITCODE"}
```
