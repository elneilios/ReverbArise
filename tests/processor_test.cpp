#include "../Source/Plugin/PluginProcessor.h"

#include <memory>
#include "WorkflowTestSupport.h"

class TestPlayHead final : public juce::AudioPlayHead
{
public:
    juce::Optional<PositionInfo> getPosition() const override
    {
        return position;
    }

    PositionInfo position;
};

#include "M2ProcessorCases.h"
#include "M3ProcessorCases.h"
#include "AutomaticProcessorCases.h"
#include "LiveProcessorCases.h"
#include "LiveResourceCases.h"
#include "LoopCaptureCases.h"
#include "PerformanceExportCases.h"
#include "MultipleExportCases.h"

int main(int argc,char** argv)
{
    // Optional focused iteration; CTest's argument-free command still executes
    // every processor regression below.
    if(argc==2 && juce::String(argv[1])=="multiple-export")return runTest([]{multipleExportCases();});
    const auto root = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("PreverbProcessorTest-" + juce::Uuid().toDashedString());
    struct Cleanup { juce::File directory; ~Cleanup() { directory.deleteRecursively(); } } cleanup{root};
    auto processor = std::make_unique<PreverbAudioProcessor>(root);
    processor->prepareToPlay(48000.0, 4);

    juce::AudioBuffer<float> buffer(2, 4);
    buffer.setSample(0, 0, 0.125f);
    buffer.setSample(0, 1, -0.25f);
    buffer.setSample(0, 2, 0.5f);
    buffer.setSample(0, 3, -1.0f);
    buffer.setSample(1, 0, -0.0625f);
    buffer.setSample(1, 1, 0.1875f);
    buffer.setSample(1, 2, -0.375f);
    buffer.setSample(1, 3, 0.75f);

    const auto original = buffer;
    juce::MidiBuffer midiMessages;
    processor->processBlock(buffer, midiMessages);

    const auto diagnostics = processor->getHostPositionDiagnostics();
    if (diagnostics.positionAvailable
        || diagnostics.samplePositionAvailable
        || diagnostics.ppqPositionAvailable
        || diagnostics.tempoAvailable
        || diagnostics.meterAvailable
        || diagnostics.blockSize != 4
        || diagnostics.sampleRate != 48000.0)
        return 2;

    for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
            if (buffer.getSample(channel, sample) != original.getSample(channel, sample))
                return 1;

    TestPlayHead stoppedPlayHead;
    stoppedPlayHead.position.setTimeInSamples(0);
    stoppedPlayHead.position.setIsPlaying(false);
    processor->setPlayHead(&stoppedPlayHead);
    processor->setMappingEnabled(true);
    processor->processBlock(buffer, midiMessages);
    if (processor->getCapturedRegionCount() != 0 || processor->getDroppedCaptureBlockCount() != 0)
        return 10;

    processor->setTestToneEnabled(true);
    processor->setToneEndBar(22);
    processor->setToneStartBar(20);
    juce::MemoryBlock savedState;
    processor->getStateInformation(savedState);

    auto restoredProcessor = std::make_unique<PreverbAudioProcessor>(root);
    restoredProcessor->setStateInformation(savedState.getData(), static_cast<int>(savedState.getSize()));
    if (! restoredProcessor->isTestToneEnabled()
        || restoredProcessor->getToneStartBar() != 20
        || restoredProcessor->getToneEndBar() != 22)
        return 11;

    // Capture input (not generated output), save compact v2 state, then restore asynchronously.
    stoppedPlayHead.position.setIsPlaying(true);
    stoppedPlayHead.position.setTimeInSamples(100);
    stoppedPlayHead.position.setPpqPosition(100.0/24000);stoppedPlayHead.position.setBpm(120);
    stoppedPlayHead.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4,4});
    processor->processBlock(buffer, midiMessages);
    finishCaptureFixture(*processor,stoppedPlayHead.position);
    if (! processor->flushCaptureForTesting()) return 12;
    const auto committed = processor->getCaptureSnapshot();
    if (committed.manifest.ranges.size() != 1 || committed.reference.isEmpty()) return 13;
    processor->getStateInformation(savedState);
    if (savedState.getSize() > 4096) return 14;
    // Capture suppresses generated tone and stores only silent original input.
    processor->setToneStartBar(8);
    processor->setToneEndBar(10);
    stoppedPlayHead.position.setTimeInSamples(672000);
    stoppedPlayHead.position.setPpqPosition(28.0);
    stoppedPlayHead.position.setBpm(120.0);
    stoppedPlayHead.position.setTimeSignature(juce::AudioPlayHead::TimeSignature{4, 4});
    buffer.setSize(2, 128);
    buffer.clear();
    processor->setMappingEnabled(true);
    stoppedPlayHead.position.setIsPlaying(true);
    processor->processBlock(buffer, midiMessages);
    finishCaptureFixture(*processor,stoppedPlayHead.position);
    if (buffer.getMagnitude(0, 128) != 0 || ! processor->flushCaptureForTesting()) return 23;
    if (processor->getCaptureSnapshot().manifest.ranges.back().kind != CaptureCoverageKind::silence) return 24;
    // The diagnostic tone remains independently testable in Live.
    stoppedPlayHead.position.setIsPlaying(true);buffer.clear();processor->processBlock(buffer,midiMessages);
    if (processor->getGeneratedTonePeak() <= 0) return 25;
    processor->setTestToneEnabled(false);
    for (int pass = 0; pass < 4; ++pass)
    {
        buffer.clear();
        if (pass % 2 != 0)
            for (int frame = 0; frame < buffer.getNumSamples(); ++frame)
            {
                buffer.setSample(0, frame, 0.25f);
                buffer.setSample(1, frame, -0.125f);
            }
        const auto dry = buffer;
        processor->processBlock(buffer, midiMessages);
        if (processor->getGeneratedTonePeak() != 0) return 26;
        for (int channel = 0; channel < 2; ++channel)
            for (int frame = 0; frame < buffer.getNumSamples(); ++frame)
                if (buffer.getSample(channel, frame) != dry.getSample(channel, frame)) return 27;
    }
    // Saved Off state must not re-enable the oscillator on reopen.
    juce::MemoryBlock toneOffState;
    processor->getStateInformation(toneOffState);
    restoredProcessor->setStateInformation(toneOffState.getData(), static_cast<int>(toneOffState.getSize()));
    if (restoredProcessor->isTestToneEnabled()) return 28;
    restoredProcessor->prepareToPlay(48000, 4);
    restoredProcessor->setStateInformation(savedState.getData(), static_cast<int>(savedState.getSize()));
    juce::MemoryBlock savedDuringRestore;
    restoredProcessor->getStateInformation(savedDuringRestore);
    const auto duringRestore = juce::AudioProcessor::getXmlFromBinary(savedDuringRestore.getData(),
        static_cast<int>(savedDuringRestore.getSize()));
    if (! duringRestore || duringRestore->getStringAttribute("manifestReference") != committed.reference) return 22;
    if (! restoredProcessor->flushCaptureForTesting()) return 15;
    const auto restored = restoredProcessor->getCaptureSnapshot();
    if (restored.manifest.projectId != committed.manifest.projectId
        || restored.manifest.instanceId != committed.manifest.instanceId
        || restored.reference != committed.reference || restored.unavailable
        || restored.manifest.ranges.size() != 1) return 16;
    CaptureSession reader(root);
    if (! reader.restore(restored.manifest.projectId, restored.manifest.instanceId,
                         restored.reference, restored.manifest.revision)) return 17;
    float left = 0, right = 0;
    if (! reader.readSample(100, left, right) || left != 0.125f || right != -0.0625f) return 18;

    // A v1 state resets capture references while retaining its existing tone controls.
    juce::XmlElement legacy("PreverbState");
    legacy.setAttribute("stateVersion", 1);
    legacy.setAttribute("toneEnabled", true);
    legacy.setAttribute("toneStartBar", 4);
    legacy.setAttribute("toneEndBar", 6);
    juce::MemoryBlock legacyState;
    juce::AudioProcessor::copyXmlToBinary(legacy, legacyState);
    restoredProcessor->setStateInformation(legacyState.getData(), static_cast<int>(legacyState.getSize()));
    if (! restoredProcessor->flushCaptureForTesting()
        || ! restoredProcessor->getCaptureSnapshot().manifest.ranges.empty()
        || restoredProcessor->getCaptureSnapshot().manifest.instanceId == committed.manifest.instanceId
        || restoredProcessor->getToneStartBar() != 4 || restoredProcessor->getToneEndBar() != 6
        || ! restoredProcessor->isTestToneEnabled()) return 19;

    // Restoring a missing snapshot reports unavailable, without inventing coverage.
    const auto cache = root.getChildFile(committed.manifest.projectId).getChildFile(committed.manifest.instanceId);
    if (! cache.getChildFile(committed.reference).deleteFile()) return 20;
    restoredProcessor->setStateInformation(savedState.getData(), static_cast<int>(savedState.getSize()));
    if (! restoredProcessor->flushCaptureForTesting()
        || ! restoredProcessor->getCaptureSnapshot().unavailable
        || ! restoredProcessor->getCaptureSnapshot().overview.empty()) return 21;

    processor->setPlayHead(nullptr);

    const auto m2Result = runTest([] { m2ProcessorCases(); });
    if (m2Result != 0) return 29;
    const auto m3Result=runTest([]{m3ProcessorCases();});if(m3Result!=0)return 30;
    if(runTest([]{automaticProcessorCases();})!=0)return 31;
    if(runTest([]{liveProcessorCases();})!=0)return 32;
    if(runTest([]{liveResourceCases();})!=0)return 33;
    if(runTest([]{coherentHostTimingCases();loopCaptureCases();})!=0)return 34;

    if(runTest([]{performanceExportCases();})!=0)return 35;
    if(runTest([]{multipleExportCases();})!=0)return 36;
    return 0;
}
