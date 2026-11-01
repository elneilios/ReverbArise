#pragma once
#include <juce_core/juce_core.h>

// Capture and IR workers may create the same instance ancestors concurrently.
// JUCE's Windows createDirectory reports ERROR_ALREADY_EXISTS for that race.
inline juce::Result ensureManagedDirectory(const juce::File& directory)
{
    if(directory.isDirectory())return juce::Result::ok();
    const auto parent=directory.getParentDirectory();
    if(parent==directory)return juce::Result::fail("Cannot create managed directory root");
    if(const auto result=ensureManagedDirectory(parent);result.failed())return result;
    const auto result=directory.createDirectory();
    return result.wasOk() || directory.isDirectory()?juce::Result::ok():result;
}
