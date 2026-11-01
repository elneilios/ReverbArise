#pragma once
#include "CaptureAssetStore.h"
#include <functional>

namespace m3
{
using SourceBlockVisitor = std::function<bool(int64_t start, const float* stereo, int frames, bool newRun)>;
bool visitSource(CaptureAssetStore&, const CaptureManifest&, int64_t start, int64_t end, bool allowGaps,
                 const SourceBlockVisitor&, juce::String& error, const std::function<bool()>& cancelled = {});
bool sourceFingerprint(CaptureAssetStore&, const CaptureManifest&, int64_t start, int64_t end,
                       juce::String& fingerprint, juce::String& error, const std::function<bool()>& cancelled = {});
}
