#pragma once
#include <Core/SongTime.h>
#include <juce_core/juce_core.h>
#include <cmath>
namespace m3
{
inline juce::String musicalPosition(int64_t sample,const SongTime& t)
{
    if(!t.valid || t.rate<=0 || t.bpm<=0 || t.numerator<=0 || t.denominator<=0)return "unavailable";
    const long double quarter=t.rate*60.0L/t.bpm,beat=quarter*4/t.denominator;
    const long double origin=t.sample-t.ppq*quarter,pos=(sample-origin)/beat;
    auto whole=std::floor(pos);
    auto fraction=static_cast<int64_t>(std::llround((pos-whole)*1000000000));
    if(fraction==1000000000){whole+=1;fraction=0;}
    const auto bar=std::floor(whole/t.numerator);
    auto base=juce::String(static_cast<juce::int64>(bar+1))+"."+juce::String(static_cast<int>(whole-bar*t.numerator)+1);
    if(fraction==0)return base;
    auto digits=juce::String(fraction).paddedLeft('0',9);while(digits.endsWithChar('0'))digits=digits.dropLastCharacters(1);
    return base+"."+digits;
}
inline bool parseMusicalPosition(const juce::String& text,const SongTime& t,int64_t& sample)
{
    if(!t.valid || t.rate<=0 || t.bpm<=0 || t.numerator<=0 || t.denominator<=0)return false;
    const auto parts=juce::StringArray::fromTokens(text,".","");
    if(parts.size()!=2 && parts.size()!=3)return false;
    if(parts.size()==3 && (parts[2].isEmpty() || parts[2].length()>9 || !parts[2].containsOnly("0123456789")))return false;
    const auto bar=parts[0].getLargeIntValue();const auto beat=parts[1].getIntValue();
    if(juce::String(bar)!=parts[0] || juce::String(beat)!=parts[1] || beat<1 || beat>t.numerator)return false;
    const long double quarter=t.rate*60.0L/t.bpm,beatLength=quarter*4/t.denominator;
    const auto at=static_cast<long double>(t.sample)-t.ppq*quarter
        +((static_cast<long double>(bar)-1)*t.numerator+beat-1+(parts.size()==3?parts[2].getLargeIntValue()/std::pow(10.0L,parts[2].length()):0))*beatLength;
    if(!std::isfinite(at) || at<=static_cast<long double>(INT64_MIN) || at>=static_cast<long double>(INT64_MAX))return false;
    sample=static_cast<int64_t>(std::llround(at));return true;
}
}
