// Investigation evidence only: compile standalone with /std:c++20 /EHsc /I Source.
// Synthetic counterexamples, NOT a host recording or a wet-audio reproduction.
#include <Playback/LiveDiagnostics.h>
#include <cstdlib>
#include <iostream>
#include <vector>

static int checks = 0;
static void require(bool value) { ++checks; if (!value) std::exit(1); }
static SongTime at(int64_t sample, double offset = 0)
{ return {true, sample, (sample-offset)/24000.0, 48000, 120, 4, 4}; }

int main()
{
    // Each trace is valid for a short Song arrangement/loop OR a Pattern.
    // Host mode is deliberately absent: it is absent from TransportTime too.
    const std::vector<std::pair<const char*, std::vector<int64_t>>> traces{
        {"normal/bar-1", {0,256,512}},
        {"short-loop/repeated-position", {0,256,0,256}},
        {"seek", {48000,0,96000}},
        {"restart", {0,256,0}}
    };
    for (const auto& [name, positions] : traces) {
        for (auto sample : positions) {
            const TransportTime song{at(sample),true,true,0,96000};
            const TransportTime pattern = song; // admissible identical API inputs
            require(m3::liveMappingGate(at(0),song.song)==m3::LiveGate::admitted);
            require(m3::liveMappingGate(at(0),pattern.song)==m3::LiveGate::admitted);
            // Wet timing admission cannot depend on whether dry input is silent:
            // neither gate accepts an audio-input argument.
            require(song.playing==pattern.playing && song.loopEnd==pattern.loopEnd);
        }
        std::cout << name << ": identical synthetic inputs admitted for either label\n";
    }
    require(m3::liveMappingGate(at(0),at(256,16))==m3::LiveGate::origin);
    require(m3::liveAssetGate(at(0),at(256,16))==m3::LiveGate::admitted);
    require(m3::liveMappingGate(at(0,16),at(256,16))==m3::LiveGate::admitted);
    // Origin drift is a timing rejection, not evidence of host mode. A new
    // playing baseline with that offset passes the actual mapping helper.
    m3::LiveObservationSlot<m3::LiveCallbackObservation> first, second;
    m3::LiveCallbackObservation a{}, b{}; a.revision=11; b.revision=22;
    first.publish(a); second.publish(b);
    require(first.read()->revision==11 && second.read()->revision==22);
    a.playing=false; first.publish(a);
    require(!first.read()->playing && second.read()->revision==22);
    std::cout << "PASS " << checks << " assertions; no FL Studio, PCM, capture, or scheduler execution\n";
}
