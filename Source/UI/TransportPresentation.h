#pragma once
#include <UI/PerformText.h>
#include <array>
#include <optional>

namespace m3::ui
{
struct TransportPresentation
{
    // Values only; labels and units have their own invariant geometry.
    std::array<juce::String,4> values{"unavailable","unavailable","unavailable","unavailable"};
    juce::String exactPosition="unavailable";
    bool loopRelative=false;
    juce::String description()const
    {return "Bar "+values[0]+" | Beat "+values[1]+" | "+values[2]+" BPM | Meter "+values[3];}
    juce::String help()const
    {return description()+(loopRelative?". Position, tempo and meter are relative to the captured Loop.":juce::String{})
        + " Bar and denominator-beat position; fractional beats are rounded for display. Exact: "+exactPosition;}
    bool operator==(const TransportPresentation&)const=default;
};
inline TransportPresentation transportPresentation(const SongTime& time,bool positionAvailable,
    bool tempoAvailable,double bpm,bool meterAvailable,int numerator,int denominator,
    std::optional<int64_t> position={})
{
    TransportPresentation result;
    if(positionAvailable && time.valid)
    {
        const auto sample=position.value_or(time.sample);
        const auto parts=juce::StringArray::fromTokens(musicalReadout(sample,time),".","");
        if(parts.size()>=2){result.values[0]=parts[0];result.values[1]=parts[1]+(parts.size()==3?"."+parts[2]:juce::String{});}
        result.exactPosition=musicalPosition(sample,time);
    }
    if(tempoAvailable)result.values[2]=compactValue(bpm,2);
    if(meterAvailable)result.values[3]=juce::String(numerator)+" / "+juce::String(denominator);
    return result;
}
inline TransportPresentation loopTransportPresentation(const SongTime& timing,std::optional<int64_t> playhead)
{
    auto result=transportPresentation(timing,playhead && *playhead>=0,
        timing.valid,timing.bpm,timing.valid,timing.numerator,timing.denominator,playhead);
    result.loopRelative=true;
    return result;
}
}
