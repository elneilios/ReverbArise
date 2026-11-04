#pragma once
#include <Core/ManagedAsset.h>
#include <vector>

struct CaptureManifest;
enum class CaptureContextOrigin { pass,legacy };
struct CaptureGenerationContext
{
    bool operator==(const CaptureGenerationContext&)const=default;
    juce::String id;
    CaptureContextOrigin origin=CaptureContextOrigin::pass;
    bool automatic=false;
    float sensitivity=.5f;
    double separationSeconds=.08;
    m3::Asset ir;
    juce::String irName;
};
struct CaptureContextSpan
{
    bool operator==(const CaptureContextSpan&)const=default;
    int64_t start=0,end=0;
    juce::String context;
};
inline constexpr size_t maximumCaptureContexts=4096,maximumCaptureContextSpans=65536;
inline constexpr size_t maximumCaptureContextBytes=8*1024*1024;
juce::String captureGenerationContextId(const CaptureGenerationContext&);
juce::var captureContextMetadata(const CaptureManifest&);
bool validateCaptureContexts(const CaptureManifest&,juce::String& error);
bool parseCaptureContexts(const juce::var&,CaptureManifest&,juce::String& error);
// Explicit migration only. This pure metadata operation cannot rewrite PCM or
// existing swell assignments, and labels unknown history as legacy.
bool captureWithLegacyContext(const CaptureManifest&,CaptureGenerationContext baseline,CaptureManifest& output,juce::String& error);
const CaptureGenerationContext* captureContextAt(const CaptureManifest&,int64_t arrival);
// Called with candidate PCM ranges already installed. Updates only encountered
// ownership; the final candidate must still exactly cover source and fit bounds.
bool captureWithContext(const CaptureManifest&,const CaptureGenerationContext&,int64_t start,int64_t end,CaptureManifest& output,juce::String& error);

// Legacy/unknown history does not invent an Off policy; explicit accepted Off
// ownership blocks implicit refresh of any intersecting source input.
bool captureAllowsAutomatic(const CaptureManifest&,int64_t start,int64_t end);
