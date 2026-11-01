#pragma once
#include "ReverseConvolution.h"
bool loadWavPackImpulseResponse(const juce::File&, uint64_t availableBytes,
                                ImpulseResponse&, juce::String& error);
