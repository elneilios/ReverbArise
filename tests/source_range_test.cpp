#include "TestSupport.h"
#include <Capture/CaptureSession.h>
#include <Capture/SourceRangeReader.h>
#include <Core/SongTime.h>
#include <limits>

int main() { return runTest([]
{
    TestDirectory root;
    CaptureSession session(root.file);
    std::vector<float> samples{1,-1, 2,-2, 3,-3, 4,-4};
    std::vector<float> zeros(4, 0);
    REQUIRE(session.commit(-4, 100, samples.data(), 4));
    REQUIRE(session.commit(0, 100, zeros.data(), 2));
    REQUIRE(session.commit(2, 100, samples.data(), 4));
    auto manifest = session.snapshot().manifest;
    CaptureAssetStore store(root.file.getChildFile(manifest.projectId).getChildFile(manifest.instanceId));
    SourceInterval result;
    juce::String error;
    REQUIRE(readSourceInterval(store, manifest, -2, 4, 100, result, error));
    REQUIRE(result.samples == std::vector<float>({3,-3,4,-4,0,0,0,0,1,-1,2,-2}));
    REQUIRE(store.audioReadCount() == 2);
    const auto dependency = sourceDependency(manifest, -2, 0);
    REQUIRE(session.commit(-4, 100, zeros.data(), 1));
    REQUIRE(readSourceInterval(store, manifest, -4, 0, 100, result, error));
    REQUIRE(result.samples.front() == 1); // The accepted old revision still reads its immutable asset.
    REQUIRE(sourceDependency(session.snapshot().manifest, -2, 0) == dependency);
    REQUIRE(sourceDependency(session.snapshot().manifest, -4, 0) != sourceDependency(manifest, -4, 0));
    REQUIRE(! readSourceInterval(store, manifest, -2, 8, 100, result, error));
    REQUIRE(error.contains("[6, 8)"));
    REQUIRE(! readSourceInterval(store, manifest, 1, 0, 100, result, error));
    REQUIRE(! readSourceInterval(store, manifest, std::numeric_limits<int64_t>::min(),
                                std::numeric_limits<int64_t>::max(), 100, result, error));
    const auto asset = root.file.getChildFile(manifest.projectId).getChildFile(manifest.instanceId)
        .getChildFile(manifest.ranges.back().assetPath);
    REQUIRE(asset.replaceWithText("corrupt"));
    REQUIRE(! readSourceInterval(store, manifest, 2, 4, 100, result, error));
    REQUIRE(error.contains("[2, 4)"));
    REQUIRE(asset.deleteFile());
    REQUIRE(! readSourceInterval(store, manifest, 2, 4, 100, result, error));
    samples[0] = std::numeric_limits<float>::infinity();
    TestDirectory nonfiniteRoot;
    CaptureSession nonfinite(nonfiniteRoot.file);
    REQUIRE(nonfinite.commit(10, 100, samples.data(), 4));
    const auto badManifest = nonfinite.snapshot().manifest;
    CaptureAssetStore badStore(nonfiniteRoot.file.getChildFile(badManifest.projectId).getChildFile(badManifest.instanceId));
    REQUIRE(! readSourceInterval(badStore, badManifest, 10, 14, 100, result, error));
    REQUIRE(error.contains("non-finite"));
    TestDirectory splittingRoot;
    CaptureSession splitting(splittingRoot.file);
    std::vector<float> mostlySilent(200, 0); mostlySilent[0] = 1;
    REQUIRE(splitting.commit(0, 100, mostlySilent.data(), 100));
    const auto silentDependency = sourceDependency(splitting.snapshot().manifest, 50, 100);
    const float replacement[2]{0.5f,0.5f};
    REQUIRE(splitting.commit(10, 100, replacement, 1));
    REQUIRE(splitting.snapshot().manifest.ranges.back().kind == CaptureCoverageKind::silence);
    REQUIRE(sourceDependency(splitting.snapshot().manifest, 50, 100) == silentDependency);
    SongTime time{true, 144000, 6, 48000, 120, 6, 8};
    int64_t first = 0, end = 0;
    std::string failure;
    REQUIRE(resolveSourceBars(time, 48000, 1, 2, first, end, failure));
    REQUIRE(first == 0 && end == 72000);
    REQUIRE(resolveSourceBars(time, 48000, 600, 601, first, end, failure));
    REQUIRE(first == 599 * 72000LL && end == 600 * 72000LL);
    REQUIRE(! resolveSourceBars(time, 44100, 1, 2, first, end, failure));
    REQUIRE(! resolveSourceBars(time, 48000, 2, 1, first, end, failure));
    time.valid = false;
    REQUIRE(! resolveSourceBars(time, 48000, 1, 2, first, end, failure));
    time.valid = true;
    REQUIRE(! resolveSourceBars(time, 48000, 1, std::numeric_limits<int64_t>::max(), first, end, failure));
}); }
