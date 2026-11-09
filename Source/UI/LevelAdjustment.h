#pragma once
#include <Core/SwellSession.h>
#include <algorithm>
#include <cmath>

namespace m3::ui::levelAdjustment
{
// The authoring limits do not constrain restored Shape values.
inline constexpr double minimumDb=-12.0,maximumDb=12.0;
inline constexpr double spanDb=maximumDb-minimumDb;
inline constexpr double homeBandDb=.3;
inline double bounded(double db)noexcept
{return std::clamp(db,minimumDb,maximumDb);}
inline bool accepts(double db)noexcept
{return std::isfinite(db) && db>=minimumDb && db<=maximumDb;}
inline double fraction(double db)noexcept
{return (bounded(db)-minimumDb)/spanDb;}
inline const juce::String help="Adjust Level from -12 to +12 dB. Drag through 0 dB at 12 o'clock to feel the detent. Use Ctrl+wheel for fine adjustment. Shift still fine-adjusts. Double-click to reset to 0 dB.";
}
