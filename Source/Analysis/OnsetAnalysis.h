#pragma once
#include <Capture/SourceVisitor.h>
#include <Core/SwellSession.h>

namespace m3
{
inline constexpr int attackPolicyVersion=3;
struct Candidate { int64_t sample=0; float strength=0; };
bool suggestOnsets(CaptureAssetStore&, const CaptureManifest&, int64_t start, int64_t end,
                   float sensitivity, double separationSeconds, std::vector<Candidate>&,
                   juce::String& error, const std::function<bool()>& cancelled = {});
}
