#include "TestSupport.h"
#include "AllocationGuard.h"
#include <Playback/AuditionVoice.h>

int main() { return runTest([]
{
    for(const auto length:{1,2,3,19,100})
    {
        AuditionVoice looping;auto audio=std::make_shared<WetRender>();audio->start=-40;audio->rate=1000;audio->samples.assign(length*2,.25f);
        const auto original=audio->samples;REQUIRE(looping.start(audio,1,true));
        std::vector<float> l(length*3+7),r(l.size());
        {AudioAllocationGuard guard;looping.process(l.data(),r.data(),static_cast<int>(l.size()),1000);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && looping.isPlaying());
        REQUIRE(looping.position()==audio->start+static_cast<int64_t>(l.size()%length));
        for(size_t n=0;n<l.size();++n){const auto i=n%length;const auto ramp=std::min(5,(length-1)/2);const auto expected=length<3?0:.25*std::min({1.,static_cast<double>(i)/ramp,static_cast<double>(length-1-i)/ramp});REQUIRE(std::abs(l[n]-expected)<1e-6 && l[n]==r[n]);}
        looping.collect();REQUIRE(looping.pinnedCount()==1 && audio->samples==original);
        looping.stop();{AudioAllocationGuard guard;looping.process(l.data(),r.data(),static_cast<int>(l.size()),1000);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && !looping.isPlaying() && !looping.position());
        looping.collect();REQUIRE(looping.pinnedCount()==0);
        REQUIRE(looping.start(audio,1,true));looping.stop(true);looping.process(l.data(),r.data(),1,1000);REQUIRE(!looping.isPlaying());
        REQUIRE(looping.start(audio,1,true));looping.process(l.data(),r.data(),1,1000,false);REQUIRE(!looping.isPlaying());
        REQUIRE(looping.start(audio,1,true));looping.process(l.data(),r.data(),1,2000);REQUIRE(!looping.isPlaying());
    }
    {AuditionVoice looping;auto audio=std::make_shared<WetRender>();audio->rate=1000;audio->samples.assign(200,.1f);
        REQUIRE(looping.start(audio,1,true));float l[7]{},r[7]{};looping.process(l,r,7,1000);
        auto replacement=std::make_shared<WetRender>(*audio);replacement->samples.assign(200,.2f);REQUIRE(looping.replace(replacement));
        for(int n=0;n<50;++n){AudioAllocationGuard guard;looping.process(l,r,7,1000);REQUIRE(watchedAllocations==0 && watchedDeallocations==0);}
        REQUIRE(looping.isPlaying() && looping.position()==57);looping.collect();REQUIRE(looping.pinnedCount()==1);
        auto shorter=std::make_shared<WetRender>(*audio);shorter->samples.resize(20);REQUIRE(looping.replace(shorter));
        std::fill_n(l,7,0.f);std::fill_n(r,7,0.f);
        {AudioAllocationGuard guard;looping.process(l,r,7,1000);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && std::abs(l[0]-.2f)<1e-6 && l[0]==r[0]); // Outgoing edge keeps its own gain during restart.
        REQUIRE(looping.isPlaying() && looping.position()==7);
        looping.stop(true);looping.process(l,r,0,1000);looping.collect();REQUIRE(!looping.isPlaying() && looping.pinnedCount()==0);}
    {AuditionVoice edge;auto constant=std::make_shared<WetRender>();constant->rate=1000;constant->samples.assign(200,.25f);
        REQUIRE(edge.start(constant));float l[100]{},r[100]{};edge.process(l,r,100,1000);
        std::cout << "Boundary samples: " << l[0] << ", " << l[99] << "\n";
        REQUIRE(l[0]==0 && l[99]==0);}
    for(int frames:{1,2,3,4,9,40}){AuditionVoice shortVoice;auto constant=std::make_shared<WetRender>();constant->rate=1000;constant->samples.assign(frames*2,.25f);
        REQUIRE(shortVoice.start(constant));std::vector<float> l(frames),r(frames);{AudioAllocationGuard guard;shortVoice.process(l.data(),r.data(),frames,1000);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0 && l.front()==0 && l.back()==0);
        for(int i=0;i<frames;++i){const auto ramp=std::min(5,(frames-1)/2);const auto expected=frames<3?0:.25*std::min({1.0,static_cast<double>(i)/ramp,static_cast<double>(frames-1-i)/ramp});REQUIRE(std::abs(l[i]-expected)<1e-6);}}
    {AuditionVoice cursor;auto audio=std::make_shared<WetRender>();audio->start=-40;audio->rate=1000;audio->samples.assign(200,.1f);
        REQUIRE(cursor.start(audio) && !cursor.position());float l[7]{},r[7]{};
        {AudioAllocationGuard guard;cursor.process(l,r,7,1000);REQUIRE(cursor.position()==-33);}
        REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
        auto replacement=std::make_shared<WetRender>(*audio);replacement->start=-50;replacement->samples.insert(replacement->samples.begin(),20,0);
        REQUIRE(cursor.replace(replacement));cursor.process(l,r,7,1000);REQUIRE(cursor.position()==-26);
        cursor.stop(true);cursor.process(l,r,0,1000);REQUIRE(!cursor.position());
        REQUIRE(cursor.start(audio));std::vector<float> all(100);cursor.process(all.data(),all.data(),100,1000);REQUIRE(!cursor.position());}
    AuditionVoice voice;
    auto buffer = std::make_shared<WetRender>();
    buffer->rate = 1000;
    for (int frame = 0; frame < 19; ++frame) { buffer->samples.push_back(frame * 0.01f); buffer->samples.push_back(-frame * 0.01f); }
    REQUIRE(voice.start(buffer));
    float left[7], right[7];
    for (int block = 0; block < 3; ++block)
    {
        std::fill_n(left, 7, 0.125f); std::fill_n(right, 7, -0.25f);
        { AudioAllocationGuard guard; voice.process(left, right, 7, 1000); }
        REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
        for (int i = 0; i < 7; ++i)
        {
            const auto frame = block * 7 + i;
            const auto wet = frame < 19 ? frame * 0.01f*std::min({1.0f,frame/5.0f,(18-frame)/5.0f}) : 0;
            REQUIRE(std::abs(left[i] - (0.125f + wet)) < 1.0e-6);
            REQUIRE(std::abs(right[i] - (-0.25f - wet)) < 1.0e-6);
        }
    }
    REQUIRE(! voice.isPlaying()); voice.collect(); REQUIRE(voice.pinnedCount() == 0);
    REQUIRE(voice.start(buffer));
    std::fill_n(left, 7, 0); std::fill_n(right, 7, 0);
    voice.process(left, right, 7, 1000);
    REQUIRE(std::abs(left[1] - .002f)<1e-6);
    voice.stop(); voice.process(left, right, 7, 1000); REQUIRE(! voice.isPlaying());
    for (int i = 0; i < 8; ++i) REQUIRE(voice.start(buffer));
    REQUIRE(! voice.start(buffer));
    voice.stop(true);
    std::fill_n(left, 7, 0); std::fill_n(right, 7, 0);
    { AudioAllocationGuard guard; voice.process(left, right, 7, 1000); }
    REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
    REQUIRE(std::all_of(left, left + 7, [](float v) { return v == 0; }));
    voice.collect(); REQUIRE(voice.pinnedCount() == 0);
    std::weak_ptr<const WetRender> retired = buffer;
    REQUIRE(voice.start(buffer)); buffer.reset();
    voice.process(left, right, 7, 44100); // rate mismatch cannot reinterpret the samples
    REQUIRE(! voice.isPlaying() && ! retired.expired());
    voice.collect(); REQUIRE(retired.expired());
    auto loud = std::make_shared<WetRender>();
    loud->rate = 1000; loud->samples.assign(200, 4.0f);
    voice.setLevel(0.25f); REQUIRE(voice.start(loud, 0.125f));
    std::fill_n(left, 7, 0.2f); std::fill_n(right, 7, -0.2f);
    { AudioAllocationGuard guard; voice.process(left, right, 7, 1000); }
    REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
    REQUIRE(std::abs(left[0] - 0.2f) < 1e-6 && std::abs(right[0] + 0.2f) < 1e-6);
    voice.setLevel(1);
    std::fill_n(left, 7, 0); std::fill_n(right, 7, 0);
    { AudioAllocationGuard guard; voice.process(left, right, 7, 1000); }
    REQUIRE(watchedAllocations == 0 && watchedDeallocations == 0);
    REQUIRE(left[0] > 0.125f && left[0] < 0.5f && left[4] == 0.5f && left[6] == 0.5f);
    voice.setLevel(0);
    std::fill_n(left, 7, 0); voice.process(left, right, 7, 1000);
    REQUIRE(left[0] > 0 && left[4] == 0 && left[6] == 0);
    REQUIRE(std::all_of(loud->samples.begin(), loud->samples.end(), [](float v) { return v == 4; }));
    voice.stop(true);voice.setLevel(1);voice.process(left,right,0,1000);
    auto oldMix=std::make_shared<WetRender>();oldMix->rate=1000;oldMix->start=-10;
    for(int n=0;n<100;++n){oldMix->samples.push_back(n*.001f);oldMix->samples.push_back(-n*.001f);}
    auto newMix=std::make_shared<WetRender>(*oldMix);newMix->start=-20;newMix->samples.insert(newMix->samples.begin(),20,0);
    for(auto& v:newMix->samples)v*=2;
    REQUIRE(voice.start(oldMix));std::fill_n(left,7,0.0f);voice.process(left,right,7,1000);
    REQUIRE(voice.replace(newMix));std::fill_n(left,7,0.0f);
    {AudioAllocationGuard guard;voice.process(left,right,7,1000);}
    REQUIRE(watchedAllocations==0 && watchedDeallocations==0);
    REQUIRE(std::abs(left[0]-.007f)<1e-7f && std::abs(left[6]-.026f)<1e-7f); // cursor continued, not restarted
    voice.stop(true);voice.process(left,right,0,1000);voice.collect();
    REQUIRE(voice.pinnedCount()==0);
    REQUIRE(voice.replace(newMix));voice.process(left,right,1,1000);REQUIRE(!voice.isPlaying());
}); }
