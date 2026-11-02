#pragma once
#include <Core/SwellSession.h>
#include "ReverseConvolution.h"

namespace m3
{
double fadeFactor(double u, Curve, double tension=0) noexcept;
double envelope(const Shape&, int64_t position, int64_t start, int64_t end) noexcept;
double swellEnvelope(const Swell&,int64_t position,int64_t start,int64_t end) noexcept;
bool prepareShape(const Swell&, const WetRender&, uint64_t available, WetRender&,
                  juce::String& error, const RenderCancellation& cancelled = {});
}
