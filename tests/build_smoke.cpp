#include <juce_core/juce_core.h>

int main()
{
    return juce::String{"Preverb JUCE build smoke test"}.isNotEmpty() ? 0 : 1;
}