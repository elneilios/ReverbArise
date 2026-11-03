#include <string_view>
#define REVERB_STRINGIFY_DETAIL(x) #x
#define REVERB_STRINGIFY(x) REVERB_STRINGIFY_DETAIL(x)
static_assert(std::string_view(JucePlugin_VersionString) == EXPECTED_PRODUCT_VERSION);
static_assert(JucePlugin_VersionCode == EXPECTED_PRODUCT_VERSION_CODE);
static_assert(JucePlugin_ManufacturerCode == 0x4b374d61);
static_assert(JucePlugin_PluginCode == 0x50727631);
static_assert(std::string_view(JucePlugin_Name) == "ReverbArise");
static_assert(std::string_view(REVERB_STRINGIFY(JucePlugin_CFBundleIdentifier)) == "com.k7ma.preverb");
static_assert(JUCE_VST3_CAN_REPLACE_VST2 == 0);
int main() { return 0; }
