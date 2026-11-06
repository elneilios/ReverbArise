#pragma once
#include <juce_core/juce_core.h>
#include <vector>
namespace m3
{
inline constexpr size_t maximumExportRevisions=256;
struct ExportFile
{
    juce::String composition,checksum,path;
    double rate=0;
    int64_t origin=0,frames=0;
    float wet=1;
    float dry=0;
    int gainLaw=2;
    bool operator==(const ExportFile&)const=default;
};
struct ExportState
{
    bool initialized=false;
    float wet=1;
    float dry=0;
    juce::String desiredIdentity,completedPath;
    std::vector<ExportFile> history;
    bool operator==(const ExportState&)const=default;
};
bool validExportState(const ExportState&,juce::String& error);
juce::var exportStateMetadata(const ExportState&);
bool parseExportState(const juce::var&,ExportState&,juce::String& error);
}
