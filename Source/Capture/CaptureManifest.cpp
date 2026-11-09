#include "CaptureManifest.h"

#include <cmath>
#include <filesystem>
#include <limits>

namespace
{
bool isUuid(const std::string& value)
{
    if (value.size() != 36)
        return false;

    for (size_t index = 0; index < value.size(); ++index)
    {
        if (index == 8 || index == 13 || index == 18 || index == 23)
        {
            if (value[index] != '-')
                return false;
        }
        else if (! ((value[index] >= '0' && value[index] <= '9')
                    || (value[index] >= 'a' && value[index] <= 'f')
                    || (value[index] >= 'A' && value[index] <= 'F')))
        {
            return false;
        }
    }

    return true;
}

bool isChecksum(const std::string& value)
{
    if (value.size() != 64)
        return false;

    for (const auto character : value)
        if (! ((character >= '0' && character <= '9')
               || (character >= 'a' && character <= 'f')
               || (character >= 'A' && character <= 'F')))
            return false;

    return true;
}

bool readInteger(const juce::var& value, int64_t& result)
{
    if (value.isInt() || value.isInt64())
    {
        result = static_cast<int64_t>(value);
        return true;
    }

    return false;
}

bool readUnsigned(const juce::var& value, uint64_t& result)
{
    int64_t signedValue = 0;
    if (! readInteger(value, signedValue) || signedValue < 0)
        return false;

    result = static_cast<uint64_t>(signedValue);
    return true;
}

bool readString(const juce::var& value, std::string& result)
{
    if (! value.isString())
        return false;

    result = value.toString().toStdString();
    return true;
}

bool readDouble(const juce::var& value, double& result)
{
    if (! (value.isDouble() || value.isInt() || value.isInt64()))
        return false;

    result = static_cast<double>(value);
    return std::isfinite(result);
}

bool readPositiveInt(const juce::var& value, int& result)
{
    int64_t number = 0;
    if (!readInteger(value, number) || number <= 0 || number > std::numeric_limits<int>::max())
        return false;
    result = static_cast<int>(number);
    return true;
}

bool readLoopTiming(const juce::var& value, LoopPhraseTiming& timing)
{
    return value.isObject()
        && readDouble(value["tempoBpm"], timing.tempoBpm)
        && readPositiveInt(value["meterNumerator"], timing.meterNumerator)
        && readPositiveInt(value["meterDenominator"], timing.meterDenominator)
        && readPositiveInt(value["beats"], timing.beats)
        && readInteger(value["originSample"], timing.originSample)
        && readInteger(value["endSample"], timing.endSample);
}

bool validLoopTiming(const LoopPhraseTiming& timing, double sampleRate)
{
    if (!std::isfinite(timing.tempoBpm) || timing.tempoBpm <= 0
        || timing.meterNumerator <= 0 || timing.meterDenominator <= 0 || timing.beats <= 0
        || timing.originSample < 0 || timing.endSample <= 0
        || timing.originSample > std::numeric_limits<int64_t>::max() - timing.endSample)
        return false;

    const auto period = static_cast<long double>(sampleRate) * 240 * timing.beats
        / (static_cast<long double>(timing.tempoBpm) * timing.meterDenominator);
    return timing.endSample == std::floor(period) || timing.endSample == std::ceil(period);
}

juce::var makeRangeObject(const CaptureManifestRange& range)
{
    auto object = juce::var(new juce::DynamicObject());
    auto* properties = object.getDynamicObject();
    properties->setProperty("startSample", range.startSample);
    properties->setProperty("frameCount", static_cast<juce::int64>(range.frameCount));
    properties->setProperty("kind", range.kind == CaptureCoverageKind::audio ? "audio" : "silence");
    properties->setProperty("revision", static_cast<juce::int64>(range.revision));
    properties->setProperty("assetPath", juce::String::fromUTF8(range.assetPath.data(),
                                                                 static_cast<int>(range.assetPath.size())));
    properties->setProperty("checksum", juce::String::fromUTF8(range.checksum.data(),
                                                                static_cast<int>(range.checksum.size())));
    return object;
}
}

bool isSafeCaptureAssetPath(const std::string& path) noexcept
{
    if (path.empty() || path.find('\\') != std::string::npos || path.find(':') != std::string::npos)
        return false;

    const std::filesystem::path relativePath(path);
    if (relativePath.is_absolute() || relativePath.has_root_name() || relativePath.has_root_directory())
        return false;

    for (const auto& component : relativePath)
        if (component.empty() || component == "." || component == "..")
            return false;

    return true;
}

bool isValidCaptureManifest(const CaptureManifest& manifest, juce::String& error)
{
    if (manifest.schemaVersion < 1 || manifest.schemaVersion > CaptureManifest::latestSupportedSchemaVersion)
    {
        error = "Unsupported capture manifest schema version";
        return false;
    }

    if (! isUuid(manifest.projectId) || ! isUuid(manifest.instanceId))
    {
        error = "Capture manifest identifiers must be UUIDs";
        return false;
    }

    if (! std::isfinite(manifest.sampleRate) || manifest.sampleRate <= 0.0 || manifest.channelCount != 2)
    {
        error = "Capture manifest must describe a positive sample rate and stereo audio";
        return false;
    }

    if ((manifest.schemaVersion == CaptureManifest::loopSchemaVersion) != manifest.loop.has_value()
        || (manifest.loop && (!manifest.revision || !validLoopTiming(*manifest.loop, manifest.sampleRate))))
    {
        error = "Capture manifest Loop timing is invalid";
        return false;
    }

    int64_t previousEnd = manifest.loop ? 0 : std::numeric_limits<int64_t>::min();
    bool usefulAudio = false;
    for (const auto& range : manifest.ranges)
    {
        if (range.frameCount == 0
            || range.frameCount > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())
            || range.startSample > std::numeric_limits<int64_t>::max() - static_cast<int64_t>(range.frameCount)
            || range.startSample < previousEnd)
        {
            error = "Capture manifest ranges must be non-empty, ordered, and non-overlapping";
            return false;
        }
        if (manifest.loop && (range.startSample != previousEnd || range.revision != manifest.revision))
        {
            error = "Loop coverage must be contiguous and belong to the accepted revision";
            return false;
        }

        const auto endSample = range.startSample + static_cast<int64_t>(range.frameCount);
        previousEnd = endSample;

        if (range.kind == CaptureCoverageKind::audio)
        {
            usefulAudio = true;
            if (! isSafeCaptureAssetPath(range.assetPath) || ! isChecksum(range.checksum))
            {
                error = "Audio ranges require a safe relative asset path and SHA-256 checksum";
                return false;
            }
        }
        else if (range.kind == CaptureCoverageKind::silence)
        {
            if (! range.assetPath.empty() || ! range.checksum.empty())
            {
                error = "Captured-silence ranges must not reference an audio asset";
                return false;
            }
        }
        else
        {
            error = "Capture manifest contains an unknown coverage kind";
            return false;
        }
    }

    if (manifest.loop && (previousEnd != manifest.loop->endSample || !usefulAudio))
    {
        error = "Loop coverage must contain audio and match the exclusive phrase end";
        return false;
    }

    if(!validateCaptureContexts(manifest,error))return false;
    error.clear();
    return true;
}

bool serializeCaptureManifest(const CaptureManifest& manifest, juce::String& json, juce::String& error)
{
    if (! isValidCaptureManifest(manifest, error))
        return false;

    auto root = juce::var(new juce::DynamicObject());
    auto* properties = root.getDynamicObject();
    properties->setProperty("schemaVersion", manifest.schemaVersion);
    properties->setProperty("projectId", juce::String::fromUTF8(manifest.projectId.data(),
                                                                 static_cast<int>(manifest.projectId.size())));
    properties->setProperty("instanceId", juce::String::fromUTF8(manifest.instanceId.data(),
                                                                  static_cast<int>(manifest.instanceId.size())));
    properties->setProperty("sampleRate", manifest.sampleRate);
    properties->setProperty("channelCount", manifest.channelCount);
    properties->setProperty("revision", static_cast<juce::int64>(manifest.revision));

    juce::Array<juce::var> ranges;
    for (const auto& range : manifest.ranges)
        ranges.add(makeRangeObject(range));

    properties->setProperty("ranges", juce::var(ranges));
    if (manifest.schemaVersion >= CaptureManifest::currentSchemaVersion)
    {
        const auto metadata=captureContextMetadata(manifest);
        properties->setProperty("generationContexts",metadata["generationContexts"]);
        properties->setProperty("contextSpans",metadata["contextSpans"]);
    }
    if (manifest.loop)
    {
        auto timing = juce::var(new juce::DynamicObject());
        auto& fields = *timing.getDynamicObject();
        fields.setProperty("tempoBpm", manifest.loop->tempoBpm);
        fields.setProperty("meterNumerator", manifest.loop->meterNumerator);
        fields.setProperty("meterDenominator", manifest.loop->meterDenominator);
        fields.setProperty("beats", manifest.loop->beats);
        fields.setProperty("originSample", manifest.loop->originSample);
        fields.setProperty("endSample", manifest.loop->endSample);
        properties->setProperty("loop", timing);
    }
    json = juce::JSON::toString(root, true);
    error.clear();
    return true;
}

bool parseCaptureManifest(const juce::String& json, CaptureManifest& manifest, juce::String& error)
{
    const auto root = juce::JSON::parse(json);
    const auto* properties = root.getDynamicObject();
    if (properties == nullptr)
    {
        error = "Capture manifest is not a JSON object";
        return false;
    }

    CaptureManifest parsed;
    int64_t integer = 0;
    if (! readInteger(properties->getProperty("schemaVersion"), integer)
        || integer < 0 || integer > std::numeric_limits<int>::max())
    {
        error = "Capture manifest schemaVersion is invalid";
        return false;
    }
    parsed.schemaVersion = static_cast<int>(integer);

    if (! readString(properties->getProperty("projectId"), parsed.projectId)
        || ! readString(properties->getProperty("instanceId"), parsed.instanceId))
    {
        error = "Capture manifest identifiers are missing or invalid";
        return false;
    }

    if (! readDouble(properties->getProperty("sampleRate"), parsed.sampleRate)
        || ! readInteger(properties->getProperty("channelCount"), integer)
        || integer < 0 || integer > std::numeric_limits<int>::max())
    {
        error = "Capture manifest audio format is invalid";
        return false;
    }
    parsed.channelCount = static_cast<int>(integer);

    if (! readUnsigned(properties->getProperty("revision"), parsed.revision))
    {
        error = "Capture manifest revision is invalid";
        return false;
    }

    const auto rangesValue = properties->getProperty("ranges");
    const auto* ranges = rangesValue.getArray();
    if (ranges == nullptr)
    {
        error = "Capture manifest ranges must be an array";
        return false;
    }

    parsed.ranges.reserve(static_cast<size_t>(ranges->size()));
    for (const auto& rangeValue : *ranges)
    {
        const auto* rangeProperties = rangeValue.getDynamicObject();
        if (rangeProperties == nullptr)
        {
            error = "Capture manifest range is not an object";
            return false;
        }

        CaptureManifestRange range;
        if (! readInteger(rangeProperties->getProperty("startSample"), range.startSample)
            || ! readUnsigned(rangeProperties->getProperty("frameCount"), range.frameCount)
            || ! readUnsigned(rangeProperties->getProperty("revision"), range.revision))
        {
            error = "Capture manifest range position, size, or revision is invalid";
            return false;
        }

        std::string kind;
        if (! readString(rangeProperties->getProperty("kind"), kind)
            || ! readString(rangeProperties->getProperty("assetPath"), range.assetPath)
            || ! readString(rangeProperties->getProperty("checksum"), range.checksum))
        {
            error = "Capture manifest range metadata is missing or invalid";
            return false;
        }

        if (kind == "audio")
            range.kind = CaptureCoverageKind::audio;
        else if (kind == "silence")
            range.kind = CaptureCoverageKind::silence;
        else
        {
            error = "Capture manifest range kind is unknown";
            return false;
        }

        parsed.ranges.push_back(std::move(range));
    }

    if (parsed.schemaVersion == CaptureManifest::loopSchemaVersion)
    {
        LoopPhraseTiming timing;
        if (!readLoopTiming(properties->getProperty("loop"), timing))
        {
            error = "Capture manifest is missing verified Loop timing";
            return false;
        }
        parsed.loop = timing;
    }
    else if (properties->hasProperty("loop"))
    {
        error = "Loop timing requires the Loop capture schema";
        return false;
    }

    if(parsed.schemaVersion >= CaptureManifest::currentSchemaVersion)
    {if(!parseCaptureContexts(root,parsed,error))return false;}
    else if(properties->hasProperty("generationContexts") || properties->hasProperty("contextSpans"))
    {error="Generation context fields require a context-aware capture schema";return false;}
    if (! isValidCaptureManifest(parsed, error))
        return false;

    manifest = std::move(parsed);
    return true;
}
