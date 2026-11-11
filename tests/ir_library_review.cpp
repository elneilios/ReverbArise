// Explicit offline review utility. Original synthetic material, CC0-1.0.
// This runs production convolution/shaping/monitoring; it never touches a DAW.
#include "AudioTestFiles.h"
#include <DSP/DefaultImpulse.h>
#include <DSP/SwellShape.h>
#include <DSP/AuditionMix.h>
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace m3;
constexpr double rate=48000;
constexpr int64_t arrival=12*48000;
static double db(double v){return v>0?20*std::log10(v):-180.;}
static juce::var object(){return new juce::DynamicObject;}
static void put(juce::var& v,const char* key,juce::var value){v.getDynamicObject()->setProperty(key,std::move(value));}
static void writeAudio(const juce::File& file,const std::vector<float>& samples)
{
    auto stream=file.createOutputStream();REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk());
    juce::WavAudioFormat format;const auto options=juce::AudioFormatWriterOptions().withSampleRate(rate).withNumChannels(2).withBitsPerSample(24);
    std::unique_ptr<juce::OutputStream> base(std::move(stream));auto writer=format.createWriterFor(base,options);REQUIRE(writer);
    juce::AudioBuffer<float> audio(2,static_cast<int>(samples.size()/2));
    for(int n=0;n<audio.getNumSamples();++n)for(int c=0;c<2;++c){const auto x=samples[n*2+c];REQUIRE(std::isfinite(x) && std::abs(x)<1);audio.setSample(c,n,x);}
    REQUIRE(writer->writeFromAudioSampleBuffer(audio,0,audio.getNumSamples()));
}

static std::vector<float> fixture(int kind)
{
    const int frames=60000;std::vector<float> data(frames*2);uint32_t state=0x41554431;
    for(int n=0;n<frames;++n){const double t=n/rate;double v=0;
        const double attack=std::min(1.,t/.003);
        if(kind==0){for(double f:{261.625565,329.627557,391.995436})for(int h=1;h<=5;++h)
            v+=std::sin(2*std::numbers::pi*f*h*t)*std::exp(-t*(4.+h))/h;v*=attack;}
        if(kind==1){for(double f:{130.812783,195.997718,261.625565})
            v+=std::sin(2*std::numbers::pi*f*t)+.15*std::sin(4*std::numbers::pi*f*t);v*=std::pow(std::sin(std::numbers::pi*n/(frames-1)),2);}
        if(kind==2){for(int h=1;h<=24;++h){double f=110*h;
            const auto formant=std::exp(-std::pow((f-700)/250,2))+.7*std::exp(-std::pow((f-1200)/350,2))+.4*std::exp(-std::pow((f-2400)/500,2));
            v+=std::sin(2*std::numbers::pi*f*t)*formant/h;}
            v*=attack*std::exp(-t*2)*(0.6+.4*std::cos(2*std::numbers::pi*2*t));}
        if(kind==3){for(int h=1;h<=8;++h)v+=std::sin(2*std::numbers::pi*440*h*t)*std::exp(-t*(8+2*h))/h;
            state=1664525*state+1013904223;v+=.15*(state/2147483648.-1)*std::exp(-t*600);v*=attack;}
        v*=std::min(1.,(frames-1-n)/(rate*.025));data[n*2]=static_cast<float>(v);data[n*2+1]=static_cast<float>(v*.93);
    }
    float peak=0;for(auto v:data)peak=std::max(peak,std::abs(v));for(auto& v:data)v*=.25f/peak;return data;
}
static juce::var measure(const WetRender& w)
{
    auto v=object();double peak=0,sum=0,activeSum=0;uint64_t activeCount=0;
    std::vector<double> windows;constexpr size_t window=960*2;
    for(size_t i=0;i<w.samples.size();i+=window){double s=0;const auto end=std::min(i+window,w.samples.size());
        for(size_t j=i;j<end;++j){const double x=w.samples[j];REQUIRE(std::isfinite(x));peak=std::max(peak,std::abs(x));s+=x*x;}windows.push_back(s/(end-i));sum+=s;}
    const double gate=*std::max_element(windows.begin(),windows.end())*.0001;
    for(size_t k=0;k<windows.size();++k)if(windows[k]>=gate){const auto count=std::min(window,w.samples.size()-k*window);activeSum+=windows[k]*count;activeCount+=count;}
    put(v,"peak_dbfs",db(peak));put(v,"rms_dbfs",db(std::sqrt(sum/w.samples.size())));
    put(v,"active_rms_dbfs",db(std::sqrt(activeSum/activeCount)));put(v,"frames",static_cast<juce::int64>(w.frames()));
    put(v,"start",static_cast<juce::int64>(w.start));return v;
}
int main(int argc,char** argv){return runTest([&]
{
    REQUIRE(argc==3);const juce::File input(juce::String::fromUTF8(argv[1])),output(juce::String::fromUTF8(argv[2]));
    REQUIRE(output.createDirectory().wasOk());juce::String error;juce::Array<juce::var> reports;
    const char* ids[]{"preverb-hall","monumental","intimate-plate","vintage-dark-hall","shimmer-cloud","whisper"};
    const char* names[]{"piano","pad","voice-like","pluck"};
    for(const auto id:ids){ImpulseResponse ir;if(juce::String(id)==defaultImpulseId)REQUIRE(bundledImpulse(ir,error));
        else REQUIRE(loadImpulseResponse(input.getChildFile(juce::String(id)+".wav"),previewMemoryBudget,ir,error));
        const auto folder=output.getChildFile(id);REQUIRE(folder.createDirectory().wasOk());
        std::vector<float> sequence;juce::Array<juce::var> entry;
        for(int kind=0;kind<4;++kind){auto source=fixture(kind);WetRender raw;
            REQUIRE(renderReverseConvolution(source,arrival,rate,ir,ir.samples.capacity()*sizeof(float),raw,error));
            REQUIRE(raw.frames()==source.size()/2+ir.frames()-1 && raw.start==arrival-static_cast<int64_t>(ir.frames())+1);
            Swell w;w.arrival=w.sourceStart=arrival;w.sourceEnd=arrival+source.size()/2;w.shape=initialShape(rate);WetRender shaped;
            REQUIRE(prepareShape(w,raw,previewMemoryBudget,shaped,error));
            auto common=w;common.shape.automaticStart=false;common.shape.startOffset=-12000;WetRender commonShape;
            REQUIRE(prepareShape(common,raw,previewMemoryBudget,commonShape,error));
            SourceInterval dry;dry.start=arrival;dry.end=arrival+source.size()/2;dry.rate=rate;dry.samples=source;
            WetRender mixed,wet;
            REQUIRE(prepareAuditionMix(shaped,&dry,AuditionMode::dryWet,.5f,previewMemoryBudget,mixed,error));
            REQUIRE(prepareAuditionMix(shaped,nullptr,AuditionMode::wetOnly,1,previewMemoryBudget,wet,error));
            const auto stem=folder.getChildFile(names[kind]);
            writeAudio(stem.getSiblingFile(stem.getFileName()+"-dry.wav"),source);
            writeAudio(stem.getSiblingFile(stem.getFileName()+"-wet.wav"),wet.samples);
            writeAudio(stem.getSiblingFile(stem.getFileName()+"-mix.wav"),mixed.samples);
            auto forwardIR=ir;for(size_t a=0,b=ir.frames()-1;a<b;++a,--b)for(int c=0;c<ir.channels;++c)std::swap(forwardIR.samples[a*ir.channels+c],forwardIR.samples[b*ir.channels+c]);
            WetRender forward;REQUIRE(renderReverseConvolution(source,arrival,rate,forwardIR,ir.samples.capacity()*sizeof(float),forward,error));
            const auto forwardGain=std::min(1.,.5/std::max(.000001,static_cast<double>(forward.peak)));
            for(auto& v:forward.samples)v*=static_cast<float>(forwardGain);
            writeAudio(stem.getSiblingFile(stem.getFileName()+"-forward.wav"),forward.samples);
            auto row=object();put(row,"fixture",names[kind]);put(row,"raw",measure(raw));put(row,"shaped",measure(shaped));put(row,"mix",measure(mixed));
            put(row,"common_250ms_preroll",measure(commonShape));
            put(row,"forward_display_gain_db",db(forwardGain));entry.add(row);
            // Fixed 12-second slots align all presets to the same dry event time.
            const size_t first=sequence.size();sequence.resize(first+15*48000*2,0);
            const auto leading=static_cast<size_t>(std::max(int64_t{0},mixed.start))*2;
            REQUIRE(leading+mixed.samples.size()<=15*48000*2);
            std::copy(mixed.samples.begin(),mixed.samples.end(),sequence.begin()+first+leading);
        }
        writeAudio(output.getChildFile(juce::String(id)+"-comparison.wav"),sequence);
        auto report=object();put(report,"id",id);put(report,"fixtures",entry);reports.add(report);std::cout<<id<<" rendered\n";
    }
    auto report=object();put(report,"rate",rate);put(report,"active_window","20 ms windows >= -40 dB relative to maximum window power; no quiet boost");
    put(report,"monitoring","50% dry/wet, initial 20 ms Smooth release; production wet/sum protection; zero trim");
    put(report,"fixture_license","CC0-1.0 original synthetic signals; voice-like is not a recording");put(report,"responses",reports);
    REQUIRE(output.getChildFile("render-report.json").replaceWithText(juce::JSON::toString(report)));
});}
