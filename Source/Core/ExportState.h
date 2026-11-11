#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include <optional>
namespace m3
{
inline constexpr size_t maximumExportRevisions=256;
struct LoopExportOptions
{
    static constexpr int maximumRepeats=1024;
    int repeats=1,gridBeats=4;
    bool operator==(const LoopExportOptions&)const=default;
};
struct ExportMixLevels
{
    float dry=0,wet=1;
    bool operator==(const ExportMixLevels&)const=default;
};
struct LoopExportPreferences
{
    bool initialized=false;
    LoopExportOptions options;
    ExportMixLevels levels;
    bool operator==(const LoopExportPreferences&)const=default;
};
struct LoopExportDescriptor
{
    LoopExportOptions options;
    int64_t periodFrames=0;
    bool operator==(const LoopExportDescriptor&)const=default;
};
struct ExportFile
{
    juce::String composition,checksum,path;
    double rate=0;
    int64_t origin=0,frames=0;
    float wet=1;
    float dry=0;
    int gainLaw=2;
    std::optional<LoopExportDescriptor> loop;
    bool operator==(const ExportFile&)const=default;
};
struct ExportState
{
    bool initialized=false;
    float wet=1;
    float dry=0;
    juce::String desiredIdentity,completedPath;
    std::vector<ExportFile> history;
    LoopExportPreferences loop;
    ExportMixLevels levels(bool loopSource)const noexcept{return loopSource?loop.levels:ExportMixLevels{dry,wet};}
    bool operator==(const ExportState&)const=default;
};
bool validExportState(const ExportState&,juce::String& error);
juce::var exportStateMetadata(const ExportState&);
bool parseExportState(const juce::var&,ExportState&,juce::String& error);
}
