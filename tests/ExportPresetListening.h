#pragma once
#include <DSP/DefaultImpulse.h>
#include <DSP/SourceEdgeConditioning.h>
#include <numbers>

// Explicit review fixture only: no user/session audio or installed plugin is read.
inline void renderExportPresetListening(const juce::File& destination)
{
    using namespace m3;
    const auto directory=destination.getChildFile("generated-"+juce::Uuid().toString());
    REQUIRE(directory.createDirectory().wasOk());
    CaptureAssetStore capture(directory);SwellAssetStore assets(directory);juce::String error;
    constexpr int rate=48000,seconds=8,noteFrames=28800;
    Session session;session.timing={true,0,0,rate,120,4,4};session.gainPolicy=GainPolicy::musical;session.gainRevision=approvedGainRevision;
    ImpulseResponse impulse;REQUIRE(bundledImpulse(impulse,error));
    ImpulseResponse prepared;REQUIRE(prepareImpulseResponse(impulse,rate,previewMemoryBudget,prepared,error));
    Asset ir;REQUIRE(assets.writeIR(prepared,ir,error));
    std::vector<float> dry(rate*seconds*2,0);
    const std::array<double,4> frequencies{261.625565,329.627557,391.995436,523.251131};
    for(size_t note=0;note<frequencies.size();++note){
        const int64_t arrival=static_cast<int64_t>((2+note)*rate);std::vector<float> source(noteFrames*2);
        for(int n=0;n<noteFrames;++n){const double t=static_cast<double>(n)/rate;
            const auto attack=std::min(1.,t/.003),release=std::min(1.,(noteFrames-n-1)/(rate*.025));
            const auto value=.055*attack*release*std::exp(-t*6)*(std::sin(2*std::numbers::pi*frequencies[note]*t)+.3*std::sin(4*std::numbers::pi*frequencies[note]*t));
            source[n*2]=static_cast<float>(value);source[n*2+1]=static_cast<float>(value*.9);
            dry[static_cast<size_t>(arrival+n)*2]=source[n*2];dry[static_cast<size_t>(arrival+n)*2+1]=source[n*2+1];
        }
        SourceInterval reference{arrival,arrival+noteFrames,rate,source,"generated-musical-reference"};
        auto conditioned=source;REQUIRE(conditionWetSourceEdges(conditioned,rate,error));
        WetRender wet;REQUIRE(renderReverseConvolution(conditioned,arrival,rate,prepared,0,wet,error));
        Swell swell;swell.arrival=swell.sourceStart=arrival;swell.sourceEnd=arrival+noteFrames;
        swell.shape=initialShape(rate);swell.phase=Phase::ready;swell.ir=ir;swell.irName=prepared.name;
        REQUIRE(assets.writeRender(wet,ir,"generated-preset-listening",session.timing,swell.render,error));
        juce::String noteText;REQUIRE(prepareSourceEnergyCalibration(session,swell,wet,reference,reference.end,swell.render.content,swell.calibration,noteText,{},6));
        REQUIRE(swell.calibration.state!=CalibrationState::unavailable);session.swells.push_back(swell);
    }
    CaptureManifest manifest;manifest.sampleRate=rate;manifest.projectId=juce::Uuid().toDashedString().toStdString();manifest.instanceId=juce::Uuid().toDashedString().toStdString();
    for(int chunk=0;chunk<4;++chunk){CaptureManifestRange range{chunk*rate*2,rate*2,CaptureCoverageKind::audio,1,"audio/generated-"+std::to_string(chunk)+".wav",{}};
        REQUIRE(capture.writeAudioChunk(range,rate,dry.data()+chunk*rate*4,error));manifest.ranges.push_back(range);}
    struct Candidate {const char* name;float dry,wet;};
    constexpr std::array<Candidate,6> candidates{{{"Dry reference",1,0},{"Wet Layer",0,1},{"Subtle",1,.25f},{"Balanced",1,.5f},{"Cinematic",1,1},{"Wet Forward",.65f,1}}};
    juce::String report="Generated 8-second plucked-note phrase; bundled default IR; approved source-energy +6 dB calibration. No output attenuation or per-file normalization. Stereo Float32, 48 kHz, common song origin.\n";
    for(const auto& candidate:candidates){std::shared_ptr<const SongExportPlan> plan;
        REQUIRE(prepareSongExportPlan(session,manifest,"generated-listening-source",session.timing,candidate.dry,candidate.wet,plan,error));
        SongMix mix(directory);REQUIRE(mix.prepare(plan,previewMemoryBudget,error) && mix.measure(error));
        ExportFile file;REQUIRE(publishSongExport(mix,directory,file,error));REQUIRE(verifyExportFile(directory,file,error));
        REQUIRE(mix.metrics().peak<1); // Quiet fixture; no playback gain stage added.
        report+=juce::String(candidate.name)+" | Dry "+juce::String(candidate.dry*100,0)+"% Wet "+juce::String(candidate.wet*100,0)+"% | Peak "+juce::String(juce::Decibels::gainToDecibels(mix.metrics().peak),2)+" dBFS | "+directory.getChildFile(file.path).getFullPathName()+"\n";
    }
    REQUIRE(directory.getChildFile("listening.txt").replaceWithText(report));std::cout<<report;
}
