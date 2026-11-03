#pragma once
#include "OnsetAnalysis.h"

namespace m3
{
int analysisWindow(double rate) noexcept;
int64_t targetTolerance(double rate,double separation) noexcept;
bool suppressAndErase(Session&,const juce::String&,juce::String& error);
using CompletedPasses = std::vector<std::pair<int64_t,int64_t>>;
CompletedPasses usableCompletedPasses(const CaptureManifest&,const CompletedPasses&);
bool reconcileTargets(Session&,const std::vector<Candidate>&,int64_t start,int64_t end,
                      const SongTime&,juce::String& error,const CompletedPasses& completed={},
                      const CaptureManifest* sourceContexts=nullptr,std::optional<double> detectorSeparation={});
bool updateAutomaticWindows(Session&,const SongTime&,juce::String& error,const CompletedPasses& completed={});
}
