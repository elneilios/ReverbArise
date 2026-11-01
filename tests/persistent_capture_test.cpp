#include "Capture/CaptureSession.h"
#include "Capture/CaptureWorker.h"
#include <iostream>
#include <memory>

#define CHECK(condition) do { if (! (condition)) { std::cerr << "Failed line " << __LINE__ << ": " #condition "\n"; return false; } } while (false)

struct TemporaryRoot
{
    juce::File file = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("PreverbPersistence-" + juce::Uuid().toDashedString());
    ~TemporaryRoot() { file.deleteRecursively(); }
};

bool sessionTests()
{
    TemporaryRoot root;
    CaptureSession session(root.file);
    std::vector<float> original{1,-1, 2,-2, 3,-3, 4,-4, 5,-5, 6,-6, 7,-7, 8,-8};
    CHECK(session.commit(100, 48000, original.data(), 8));
    const auto saved = session.snapshot();
    std::vector<float> replacement{0.25f,-0.25f, 0.5f,-0.5f};
    CHECK(session.commit(103, 48000, replacement.data(), 2));
    const float silence[8]{};
    CHECK(session.commit(1000, 48000, silence, 4));
    auto latest = session.snapshot();
    // Repeated loop capture with identical PCM must not rewrite managed assets
    // or publish a new dependency revision, even across old chunk boundaries.
    CHECK(session.commit(103,48000,replacement.data(),2));
    CHECK(session.snapshot().manifest.revision==latest.manifest.revision && session.snapshot().reference==latest.reference);
    CaptureSession reopened(root.file);
    CHECK(reopened.restore(latest.manifest.projectId, latest.manifest.instanceId, latest.reference, latest.manifest.revision));
    float left = 0, right = 0;
    for (int64_t position = 100; position < 108; ++position)
    {
        CHECK(reopened.readSample(position, left, right));
        const auto expected = position == 103 ? 0.25f : position == 104 ? 0.5f : static_cast<float>(position - 99);
        CHECK(left == expected && right == -expected);
    }
    CHECK(! reopened.readSample(999, left, right));
    CHECK(reopened.readSample(1002, left, right) && left == 0 && right == 0);
    CHECK(! reopened.readSample(1004, left, right));
    CHECK(reopened.snapshot().overview.back().range.kind == CaptureCoverageKind::silence);
    CHECK(! reopened.snapshot().overview.front().peaks.empty());
    CHECK(reopened.snapshot().overview.front().peaks.front().minimum == -3);
    CHECK(reopened.snapshot().overview.front().peaks.front().maximum == 3);
    // Saved project state remains authoritative over newer, unsaved captures.
    CHECK(reopened.restore(saved.manifest.projectId, saved.manifest.instanceId, saved.reference, saved.manifest.revision));
    CHECK(reopened.readSample(103, left, right) && left == 4);
    CHECK(! reopened.readSample(1000, left, right));
    CHECK(! reopened.restore(saved.manifest.projectId, saved.manifest.instanceId, saved.reference, 999));
    CHECK(reopened.snapshot().unavailable);
    CHECK(reopened.restore(latest.manifest.projectId, latest.manifest.instanceId, latest.reference, latest.manifest.revision));
    CHECK(! reopened.commit(3000, 44100, replacement.data(), 2));
    CHECK(reopened.snapshot().reference == latest.reference);
    // Failure after writing new chunks must not replace the committed map or clear it.
    const auto cache = root.file.getChildFile(latest.manifest.projectId).getChildFile(latest.manifest.instanceId);
    const auto manifests = cache.getChildFile("manifests");
    const auto backup = cache.getChildFile("held-manifests");
    CHECK(manifests.moveFileTo(backup));
    CHECK(manifests.replaceWithText("blocked directory"));
    CHECK(! reopened.commit(103, 48000, original.data(), 2));
    CHECK(! reopened.clear(48000));
    CHECK(reopened.snapshot().reference == latest.reference);
    CHECK(reopened.readSample(103, left, right) && left == 0.25f);
    CHECK(manifests.deleteFile() && backup.moveFileTo(manifests));
    CHECK(reopened.restore(latest.manifest.projectId, latest.manifest.instanceId, latest.reference, latest.manifest.revision));
    // Missing/corrupt data is unavailable, with unrelated valid audio and silence retained.
    CHECK(cache.getChildFile(latest.manifest.ranges.front().assetPath).deleteFile());
    CHECK(! reopened.restore(latest.manifest.projectId, latest.manifest.instanceId, latest.reference, latest.manifest.revision));
    CHECK(! reopened.readSample(100, left, right));
    CHECK(reopened.readSample(103, left, right) && left == 0.25f);
    CHECK(reopened.readSample(1000, left, right) && left == 0);
    CHECK(cache.getChildFile(latest.manifest.ranges[1].assetPath).replaceWithText("corrupt"));
    CHECK(! reopened.restore(latest.manifest.projectId, latest.manifest.instanceId, latest.reference, latest.manifest.revision));
    CHECK(! reopened.readSample(103, left, right));
    CHECK(reopened.readSample(105, left, right) && left == 6);
    CHECK(reopened.clear(44100));
    CHECK(! reopened.snapshot().unavailable && reopened.snapshot().manifest.ranges.empty());
    CHECK(reopened.commit(0, 44100, replacement.data(), 2));
    CHECK(! reopened.restore("../escape", latest.manifest.instanceId, latest.reference, latest.manifest.revision));
    TemporaryRoot appendRoot;
    CaptureSession appending(appendRoot.file);
    if (! appending.commit(100, 48000, original.data(), 8))
    {
        std::cerr << appending.snapshot().status << "\n";
        return false;
    }
    const auto readsBeforeAppend = appending.audioReadCount();
    CHECK(appending.commit(2000, 48000, original.data(), 8));
    // Appending a chunk reads only that new waveform; old PCM is neither rehashed nor rebuilt.
    CHECK(appending.audioReadCount() == readsBeforeAppend + 1);
    return true;
}

bool workerTests()
{
    TemporaryRoot root;
    auto queue = std::make_unique<PluginCaptureQueue>();
    auto worker = std::make_unique<CaptureWorker>(*queue, root.file);
    worker->setHostRate(100); // 500-frame limit exercises bounded chunk assembly quickly.
    const float left[20]{1}, right[20]{-1};
    for (int i = 0; i < 32; ++i) CHECK(queue->tryPush(i * 20, 100, left, right, 20));
    CHECK(! queue->tryPush(640, 100, left, right, 20));
    worker->startThread();
    CHECK(worker->flush());
    CHECK(queue->tryPush(660, 100, left, right, 20));
    const float silence[20]{};
    CHECK(queue->tryPush(1000, 100, silence, silence, 20));
    CHECK(worker->flush());
    const auto captured = worker->snapshot();
    for (const auto& range : captured.manifest.ranges) CHECK(range.frameCount <= 500);
    CaptureSession verify(root.file);
    CHECK(verify.restore(captured.manifest.projectId, captured.manifest.instanceId, captured.reference, captured.manifest.revision));
    float l = 0, r = 0;
    CHECK(verify.readSample(639, l, r));
    CHECK(! verify.readSample(640, l, r));
    CHECK(verify.readSample(660, l, r) && l == 1);
    CHECK(verify.readSample(1000, l, r) && l == 0);
    CHECK(! verify.readSample(900, l, r));
    CHECK(worker->discontinuities() == 2);
    worker->setHostRate(200);
    CHECK(worker->flush());
    CHECK(! worker->canMap());
    CHECK(queue->tryPush(2000, 200, left, right, 20, worker->captureGeneration()));
    CHECK(worker->flush());
    CHECK(worker->snapshot().reference == captured.reference);
    worker->requestClear();
    CHECK(worker->flush());
    CHECK(worker->canMap() && worker->snapshot().manifest.ranges.empty());
    // A block still in flight when Clear was requested belongs to the old generation.
    CHECK(queue->tryPush(1500, 200, left, right, 20, 0));
    CHECK(queue->tryPush(2000, 200, left, right, 20, worker->captureGeneration()));
    CHECK(worker->flush());
    CHECK(worker->snapshot().manifest.sampleRate == 200);
    CHECK(worker->snapshot().manifest.ranges.size() == 1);
    CHECK(worker->snapshot().manifest.ranges.front().startSample == 2000);
    worker->requestRestore(captured.manifest.projectId, captured.manifest.instanceId, captured.reference, captured.manifest.revision);
    worker->setHostRate(100);
    CHECK(worker->flush());
    CHECK(worker->canMap());
    CHECK(worker->snapshot().reference == captured.reference);
    worker->stopThread(-1);
    return true;
}

int main() { return sessionTests() && workerTests() ? 0 : 1; }
