#pragma once
#include "SongMix.h"
namespace m3
{
enum class ExportFileStage { write, shortWrite, close, verify, publish };
using ExportCommit=std::function<bool(const std::function<bool()>&)>;
using ExportFileFault=std::function<bool(ExportFileStage)>;
bool verifyExportFile(const juce::File& instance,const ExportFile&,juce::String& error,const RenderCancellation& cancel={});
bool publishSongExport(SongMix&,const juce::File& instance,ExportFile& output,juce::String& error,
                       const RenderCancellation& cancel={},const ExportFileFault& fault={});
bool saveExportFile(const juce::File& instance,const ExportFile&,const juce::File& destination,bool overwrite,
                    juce::String& error,const RenderCancellation& cancel={},const ExportFileFault& fault={},const ExportCommit& commit={});
}
