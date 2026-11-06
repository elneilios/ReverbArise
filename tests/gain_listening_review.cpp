// Explicit offline musical gain review. Original synthetic sources, CC0-1.0.
#include "AudioTestFiles.h"
#include <DSP/DefaultImpulse.h>
#include <DSP/SwellShape.h>
#include <DSP/GainStaging.h>
#include <DSP/AuditionMix.h>
#include <Core/MonitoringSafety.h>
#include <Core/ContentHash.h>
#include <cmath>
#include <numbers>
using namespace m3;
constexpr double rate=48000;constexpr int64_t arrival=12*48000;
static double db(double v){return v>0?20*std::log10(v):-180;}
static juce::var object(){return new juce::DynamicObject;}
static void put(juce::var& v,const char* key,juce::var value){v.getDynamicObject()->setProperty(key,std::move(value));}
static float peak(const std::vector<float>& x){float p=0;for(auto v:x)p=std::max(p,std::abs(v));return p;}
static double rms(const std::vector<float>& x){long double s=0;for(auto v:x)s+=v*v;return x.empty()?0:std::sqrt(static_cast<double>(s/x.size()));}
static void writeAudio(const juce::File& file,const std::vector<float>& samples)
{
    auto stream=file.createOutputStream();REQUIRE(stream && stream->setPosition(0) && stream->truncate().wasOk());
    juce::WavAudioFormat format;auto options=juce::AudioFormatWriterOptions().withSampleRate(rate).withNumChannels(2).withBitsPerSample(16);
    std::unique_ptr<juce::OutputStream> base(std::move(stream));auto writer=format.createWriterFor(base,options);REQUIRE(writer);
    juce::AudioBuffer<float> audio(2,static_cast<int>(samples.size()/2));
    for(int n=0;n<audio.getNumSamples();++n)for(int c=0;c<2;++c){REQUIRE(std::isfinite(samples[n*2+c]) && std::abs(samples[n*2+c])<1);audio.setSample(c,n,samples[n*2+c]);}
    REQUIRE(writer->writeFromAudioSampleBuffer(audio,0,audio.getNumSamples()));
}
static std::vector<float> source(int kind,float level)
{
    const int frames=60000;std::vector<float> data(frames*2);uint32_t noise=0x4741494e;
    for(int n=0;n<frames;++n){const double t=n/rate,attack=std::min(1.,t/.003);double v=0;
        if(kind==0){for(double f:{261.625565,329.627557,391.995436})for(int h=1;h<=5;++h)v+=std::sin(2*std::numbers::pi*f*h*t)*std::exp(-t*(4+h))/h;v*=attack;}
        if(kind==1){for(double f:{130.812783,195.997718,261.625565})v+=std::sin(2*std::numbers::pi*f*t)+.15*std::sin(4*std::numbers::pi*f*t);v*=std::pow(std::sin(std::numbers::pi*n/(frames-1)),2);}
        if(kind==2){for(int h=1;h<=24;++h){const double f=110*h,formant=std::exp(-std::pow((f-700)/250,2))+.7*std::exp(-std::pow((f-1200)/350,2));v+=std::sin(2*std::numbers::pi*f*t)*formant/h;}v*=attack*std::exp(-t*2);}
        if(kind==3){for(int h=1;h<=8;++h)v+=std::sin(2*std::numbers::pi*440*h*t)*std::exp(-t*(8+2*h))/h;noise=1664525*noise+1013904223;v+=.15*(noise/2147483648.-1)*std::exp(-t*600);v*=attack;}
        v*=std::min(1.,(frames-1-n)/(rate*.025));data[n*2]=static_cast<float>(v);data[n*2+1]=static_cast<float>(v*.91);}
    const auto p=peak(data);for(auto& v:data)v*=level/p;return data;
}
static ImpulseResponse imported()
{
    ImpulseResponse ir;ir.rate=rate;ir.channels=2;ir.name="Imported chamber fixture";ir.identity="review-import-v1";ir.samples.resize(4*48000*2);
    uint32_t noise=19;for(size_t n=0;n<ir.frames();++n){noise=1664525*noise+1013904223;const auto t=n/rate;
        const auto tail=static_cast<float>((noise/2147483648.-1)*std::exp(-t*1.7)*.018);ir.samples[n*2]=tail;ir.samples[n*2+1]=-tail*.83f;}
    ir.samples[0]=.35f;ir.samples[1]=.28f;ir.samples[2400]=.18f;ir.samples[2401]=-.12f;return ir;
}
int main(int argc,char** argv){return runTest([&]{
    const bool levelReview=argc==3 && juce::String(argv[2])=="--level-review";
    REQUIRE(argc==2 || levelReview);const juce::File output(juce::String::fromUTF8(argv[1]));REQUIRE(output.createDirectory().wasOk());
    const char* ids[]{"preverb-hall","monumental","intimate-plate","vintage-dark-hall","shimmer-cloud","whisper","imported-chamber","imported-chamber-quiet","imported-chamber-hot","imported-sparse"};
    const char* names[]{"Preverb Hall","Monumental","Intimate Plate","Vintage Dark Hall","Shimmer Cloud","Whisper","Imported chamber","Imported chamber (-20 dB recording)","Imported chamber (+20 dB recording)","Imported sparse response"};
    const char* fixtures[]{"transient chord","sustained pad","voice-like attack","busy overlapping plucks","quiet chord","strong chord"};const float levels[]{.25f,.12f,.7f,.32f,.025f,.7f};
    const char* variants[]{"fixed-plus-6","source-peak","energy-zero","energy-plus-3","energy-plus-6"};juce::Array<juce::var> responses;juce::String error;
    const char* levelVariants[]{"level-minus12","level-minus24"};
    juce::String html="<!doctype html><meta charset='utf-8'><title>ReverbArise gain review</title><style>body{font:15px system-ui;background:#10181c;color:#e8eef0;max-width:1200px;margin:auto;padding:24px}section{border:1px solid #405058;border-radius:10px;padding:16px;margin:18px 0}table{width:100%;border-collapse:collapse}td,th{padding:6px;border-top:1px solid #34434a;text-align:left}audio{width:230px}code{color:#28dbe8}</style><h1>ReverbArise musical gain-staging review</h1><p>Compare Legacy, fixed +3/+6/+9 dB, and local source-peak matching. Musical candidates use the provisional -1 dBFS ceiling. Files use production reverse convolution, creation-default shape and audition protection. Use Wet Only to judge impact; use Dry + Wet for musical balance.</p>";
    html=html.upToFirstOccurrenceOf("<h1>",false,false)+"<h1>Source-energy calibration review</h1><p>Compare your preferred fixed +6 dB reference with peak matching and energy matching at 0, +3 and +6 dB target offsets. All candidates use the same -1 dBFS wet ceiling and neutral Level. Dry + Wet uses the same captured source at 50% blend. Start with transient chord, then quiet/strong chord; compare imported chamber at three recording levels. Strong swells can require peak protection.</p><p>20 ms RMS windows / 1 ms hop, up to 100 ms dry attack, -60 dBFS RMS floors, +24 dB boost cap. These are listening candidates, not final defaults. One player runs at a time.</p><label>Source <select id='source' onchange='filter()'><option value='all'>All</option>";
    if(levelReview)html=html.upToFirstOccurrenceOf("<h1>",false,false)+"<h1>Level adjustment review</h1><p>Approved source-energy +6 dB calibration with Level -12 versus -24 dB. Hall, Whisper and an imported chamber, quiet/strong chords. Production convolution, shape and protection; 50% Dry + Wet is a fixed comparison blend, separate from current live monitoring. Keep the same device volume. This is new Level feedback, not a request to reapprove calibration.</p><label>Source <select id='source' onchange='filter()'><option value='all' selected>All</option>";
    for(int i=0;i<6;++i)if(!levelReview || i>=4)html+="<option value='"+juce::String(i)+"'"+(!levelReview && i==0?" selected":"")+">"+juce::String(fixtures[i])+"</option>";
    html+="</select></label><script>function filter(){const v=document.getElementById('source').value;for(const r of document.querySelectorAll('tr[data-source]'))r.hidden=v!=='all'&&r.dataset.source!==v}document.addEventListener('DOMContentLoaded',filter);document.addEventListener('play',e=>{for(const a of document.querySelectorAll('audio'))if(a!==e.target)a.pause()},true)</script>";
    for(int response=0;response<10;++response){if(levelReview && response!=0 && response!=5 && response!=6)continue;ImpulseResponse ir;if(response>=6){ir=imported();if(response==7 || response==8)for(auto& v:ir.samples)v*=response==7?.1f:10.0f;
            if(response==9){ir.samples.assign(48000*2,0);ir.samples[0]=1;ir.samples[1]=.7f;ir.samples[6000]=.3f;ir.samples[6001]=-.2f;}
            const auto file=output.getChildFile(juce::String(ids[response])+"-input.wav");writeTestWav(file,2,rate,ir.samples);REQUIRE(loadImpulseResponse(file,previewMemoryBudget,ir,error));
        }else {const auto* descriptor=findBundledImpulse(ids[response]);REQUIRE(descriptor);REQUIRE(decodeBundledImpulse(*descriptor,previewMemoryBudget,ir,error));}
        auto section=object();put(section,"id",ids[response]);put(section,"ir_pcm_identity",ir.identity);if(response<6)put(section,"factory_wav_sha256",findBundledImpulse(ids[response])->wavHash);
        juce::Array<juce::var> rows;html+="<section><h2>"+juce::String(names[response])+"</h2><table><tr><th>Source</th><th>Candidate</th><th>Wet Only</th><th>Dry + Wet</th></tr>";
        for(int kind=0;kind<6;++kind){if(levelReview && kind<4)continue;auto drySamples=source(kind>=4?0:kind,levels[kind]);WetRender raw;REQUIRE(renderReverseConvolution(drySamples,arrival,rate,ir,ir.samples.capacity()*sizeof(float),raw,error));
            SourceInterval dry;dry.start=arrival;dry.end=arrival+static_cast<int64_t>(drySamples.size()/2);dry.rate=rate;dry.samples=drySamples;dry.dependency="review-source-"+std::to_string(kind);
            Swell base;base.arrival=base.sourceStart=arrival;base.sourceEnd=dry.end;base.shape=initialShape(rate);base.ir.content=ir.identity.isNotEmpty()?ir.identity:juce::String(ids[response]);
            Session musical;musical.gainPolicy=GainPolicy::musical;musical.gainRevision=candidateGainRevision;GainCalibration matched;juce::String note;
            REQUIRE(prepareSourcePeakCalibration(musical,base,raw,dry,dry.end,"review-render",matched,note));juce::Array<juce::var> candidates;
            GainCalibration energy;auto energySession=musical;energySession.gainRevision=energyGainRevision;
            REQUIRE(prepareSourceEnergyCalibration(energySession,base,raw,dry,dry.end,"review-render",energy,note));
            for(int variant=0;variant<(levelReview?2:5);++variant){Session policy=musical;Swell w=base;
                w.calibration=variant==0?fixedCalibration(policy,w,6,"review-render"):variant==1?matched:energy;
                if(variant>=2){policy=energySession;REQUIRE(prepareSourceEnergyCalibration(policy,base,raw,dry,dry.end,"review-render",w.calibration,note,{},static_cast<float>((variant-2)*3)));}
                if(levelReview){policy=energySession;policy.gainRevision=approvedGainRevision;
                    REQUIRE(prepareSourceEnergyCalibration(policy,base,raw,dry,dry.end,"review-render",w.calibration,note,{},approvedEnergyTargetDb));
                    w.shape.gainDb=variant==0?-12.f:-24.f;}
                const auto* variantName=levelReview?levelVariants[variant]:variants[variant];
                WetRender shaped;REQUIRE(prepareShape(w,raw,previewMemoryBudget,shaped,error));
                for(size_t n=0;n<shaped.frames();++n)for(int c=0;c<2;++c)shaped.samples[n*2+c]*=monitorEdgeGain(n,shaped.frames(),rate,.002);
                auto listeningDry=dry;
                if(kind==3){const size_t offset=4800;const auto originalWet=shaped.samples;shaped.samples.resize(originalWet.size()+offset*2);
                    for(size_t n=0;n<originalWet.size()/2;++n)for(int c=0;c<2;++c)shaped.samples[(n+offset)*2+c]+=originalWet[n*2+c];
                    const auto originalDry=dry.samples;listeningDry.samples.resize(originalDry.size()+offset*2);listeningDry.end+=offset;
                    for(size_t n=0;n<originalDry.size()/2;++n)for(int c=0;c<2;++c)listeningDry.samples[(n+offset)*2+c]+=originalDry[n*2+c];}
                shaped.peak=peak(shaped.samples);listeningDry.end+=96000;listeningDry.samples.resize(static_cast<size_t>(listeningDry.end-listeningDry.start)*2);
                WetRender wet,mix;float wetDb=0,sumDb=0,dummy=0;
                REQUIRE(prepareAuditionMix(shaped,nullptr,AuditionMode::wetOnly,1,previewMemoryBudget,wet,wetDb,dummy,error,false,policy.gainPolicy));
                REQUIRE(prepareAuditionMix(shaped,&listeningDry,AuditionMode::dryWet,.5f,previewMemoryBudget,mix,wetDb,sumDb,error,false,policy.gainPolicy));
                const auto stem=juce::String(ids[response])+"-"+juce::String(kind)+"-"+variantName;writeAudio(output.getChildFile(stem+"-wet.wav"),wet.samples);writeAudio(output.getChildFile(stem+"-mix.wav"),mix.samples);
                auto row=object();put(row,"candidate",variantName);put(row,"level_db",w.shape.gainDb);put(row,"baseline_db",w.calibration.baselineDb);put(row,"dry_reference_peak_dbfs",db(energy.dryPeak));
                put(row,"dry_rms_dbfs",db(energy.dryRms));put(row,"default_wet_rms_dbfs",db(energy.wetRms));put(row,"window_frames",energy.windowFrames);put(row,"hop_frames",energy.hopFrames);
                put(row,"reference_start",energy.referenceStart);put(row,"reference_end",energy.referenceEnd);put(row,"target_offset_db",levelReview?6:variant>=2?(variant-2)*3:0);
                put(row,"wet_to_dry_energy_db",db(energy.wetRms)+w.calibration.baselineDb+w.shape.gainDb+wetDb-db(energy.dryRms));
                const auto dryDb=db(peak(drySamples)),wetPeakDb=db(wet.peak);
                put(row,"default_wet_peak_dbfs",db(matched.wetPeak));put(row,"prepared_peak_dbfs",wetPeakDb);put(row,"prepared_rms_dbfs",db(rms(wet.samples)));
                put(row,"wet_to_dry_peak_db",wetPeakDb-dryDb);put(row,"ceiling_occupancy_db",wetPeakDb-candidateWetCeilingDb);
                put(row,"wet_protection_db",wetDb);put(row,"sum_protection_db",sumDb);put(row,"matching_state",static_cast<int>(w.calibration.state));put(row,"matching_capped",w.calibration.state==CalibrationState::capped);candidates.add(row);
                const auto status=w.calibration.state==CalibrationState::unavailable?juce::String("Matching unavailable; baseline 0 dB"):
                    "Baseline "+juce::String(w.calibration.baselineDb,1)+" dB"+(w.calibration.state==CalibrationState::capped?" (capped)":"");
                html+="<tr data-source='"+juce::String(kind)+"'><td>"+juce::String(fixtures[kind])+"</td><td><code>"+juce::String(variantName)+"</code><br><small>"+status+"<br>Wet protection "+juce::String(wetDb,1)+" dB</small></td><td><audio controls preload='none' src='"+stem+"-wet.wav'></audio></td><td><audio controls preload='none' src='"+stem+"-mix.wav'></audio></td></tr>";
            }auto fixture=object();ContentHash checksum;checksum.update(drySamples.data(),drySamples.size()*sizeof(float));put(fixture,"source_pcm_sha256",checksum.finish());
            put(fixture,"name",fixtures[kind]);put(fixture,"source_peak_dbfs",db(peak(drySamples)));put(fixture,"candidates",candidates);rows.add(fixture);
        }html+="</table></section>";put(section,"fixtures",rows);responses.add(section);
    }
    auto report=object();put(report,"revision",levelReview?approvedGainRevision:energyGainRevision);put(report,"rate",rate);put(report,"attack_window_ms",candidateAttackSeconds*1000);put(report,"floor_dbfs",candidateMatchFloorDb);put(report,"maximum_boost_db",candidateMaximumBoostDb);put(report,"target_offset_db",levelReview?6:0);put(report,"ceiling_dbfs",candidateWetCeilingDb);put(report,"responses",responses);
    REQUIRE(output.getChildFile("metrics.json").replaceWithText(juce::JSON::toString(report)));html+="<p>Exact metrics: <a href='metrics.json'>metrics.json</a>. Approval is intentionally pending.</p>";REQUIRE(output.getChildFile("index.html").replaceWithText(html));
});}
