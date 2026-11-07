#pragma once
#include <Core/SwellSession.h>
#include <algorithm>
#include <cmath>

namespace m3::ui::levelAdjustment
{
// The authoring limits do not constrain restored Shape values.
inline constexpr double minimumDb=-12.0,maximumDb=12.0;
inline constexpr double spanDb=maximumDb-minimumDb;
inline double bounded(double db)noexcept
{return std::clamp(db,minimumDb,maximumDb);}
inline bool accepts(double db)noexcept
{return std::isfinite(db) && db>=minimumDb && db<=maximumDb;}
inline double fraction(double db)noexcept
{return (bounded(db)-minimumDb)/spanDb;}
inline const juce::String help="Adjust Level from -12 to +12 dB. 0 dB is at 12 o'clock. Use Shift for fine adjustment. Double-click to reset to 0 dB.";
}
