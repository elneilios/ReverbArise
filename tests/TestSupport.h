#pragma once
#include <juce_core/juce_core.h>
#include <iostream>
#include <stdexcept>
#define REQUIRE(condition) do { if (! (condition)) throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + " " #condition); } while (false)
struct TestDirectory
{
    juce::File file = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("PvM2-" + juce::Uuid().toString());
    ~TestDirectory() { file.deleteRecursively(); }
};
template <typename Function> int runTest(Function test)
{
    try { test(); return 0; }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
