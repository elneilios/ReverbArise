#pragma once
#include <Core/MusicalPosition.h>
#include <charconv>
namespace m3::ui
{
template<class Number> inline juce::String preciseValue(Number value)
{
    if(!std::isfinite(value))return "unavailable";
    char buffer[512];const auto result=std::to_chars(buffer,buffer+sizeof(buffer),value,std::chars_format::fixed);
    return result.ec==std::errc{}?juce::String::fromUTF8(buffer,static_cast<int>(result.ptr-buffer)):juce::String("unavailable");
}
inline juce::String compactValue(double value,int decimals)
{
    if(!std::isfinite(value))return "unavailable";
    auto result=juce::String(value,decimals);
    if(result.containsChar('.')){while(result.endsWithChar('0'))result=result.dropLastCharacters(1);if(result.endsWithChar('.'))result=result.dropLastCharacters(1);}
    return result=="-0"?juce::String("0"):result;
}
// Presentation only: retain the core exact formatter/parser and durable names.
inline juce::String musicalReadout(int64_t sample,const SongTime& timing)
{
    const auto exact=musicalPosition(sample,timing);auto parts=juce::StringArray::fromTokens(exact,".","");
    if(parts.size()!=3)return exact;
    auto bar=parts[0].getLargeIntValue();auto beat=parts[1].getIntValue();
    auto fraction=static_cast<int>(std::lround(("0."+parts[2]).getDoubleValue()*1000));
    if(fraction==1000){fraction=0;if(++beat>timing.numerator){beat=1;if(bar==INT64_MAX)return "unavailable";++bar;}}
    auto text=juce::String(bar)+"."+juce::String(beat);
    if(fraction!=0){auto digits=juce::String(fraction).paddedLeft('0',3);while(digits.endsWithChar('0'))digits=digits.dropLastCharacters(1);text+="."+digits;}
    return text;
}
inline juce::String footerPosition(int64_t sample,const SongTime& timing)
{
    const auto parts=juce::StringArray::fromTokens(musicalReadout(sample,timing),".","");
    if(parts.size()<2)return "Position unavailable";
    return "Bar "+parts[0]+"  |  Beat "+parts[1]+(parts.size()==3?"."+parts[2]:juce::String{});
}
}
