#pragma once
#include <Core/SwellSession.h>
#include <algorithm>

namespace m3::ui::levelAdjustment
{
// Presentation/gesture policy, separate from the durable Shape domain.
inline constexpr double mainMinimumDb=-12.0;
inline constexpr double spanDb=maximumSwellLevelDb-mainMinimumDb;
inline double fraction(double db)noexcept
{return std::clamp((db-mainMinimumDb)/spanDb,0.0,1.0);}
inline double gestureMinimum(double initialDb)noexcept
{return initialDb<mainMinimumDb?minimumSwellLevelDb:mainMinimumDb;}
inline const juce::String help="Level relative to the calibrated swell. Dial: -12 to +12 dB; type -60 to +12 dB for extended attenuation. Steps: 1 dB, Shift: 0.1 dB. Double-click to reset to 0 dB; right-click for reset options.";
}
