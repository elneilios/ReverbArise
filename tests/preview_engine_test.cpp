#include "AudioTestFiles.h"
#include <DSP/PreviewEngine.h>
#include <Capture/CaptureSession.h>
#include <source_location>

template <typename Condition> static void waitUntil(Condition condition, const std::source_location location = std::source_location::current())
{
    const auto deadline = juce::Time::getMillisecondCounter() + 5000;
    while (! condition())
    {
        if (juce::Time::getMillisecondCounter() >= deadline)
            throw std::runtime_error("Timed out waiting at test line " + std::to_string(location.line()));
        juce::Thread::sleep(1);
    }
}

int main() { return runTest([]
{
    TestDirectory root;
    CaptureSession session(root.file);
    std::vector<float> source(6000, 0); source[0] = 1; source[1000] = 0.5f;
    REQUIRE(session.commit(0, 1000, source.data(), 3000));
    juce::CriticalSection contextLock;
    PreviewContext context{session.snapshot(), SongTime{true, 5000, 20, 1000, 240, 4, 4}, 0};
    PreviewEngine engine(root.file, [&] { const juce::ScopedLock guard(contextLock); return context; });
    writeTestWav(root.file.getChildFile("ir.wav"), 1, 1000, {4,2,1});
    engine.loadIR(root.file.getChildFile("ir.wav"));
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::canGenerate; });
    engine.loadIR({}); REQUIRE(engine.snapshot().phase == PreviewPhase::canGenerate);
    engine.generate();
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::ready; });
    const auto ready = engine.snapshot();
    REQUIRE(ready.waveform && ready.waveform->start == -2 && ready.waveform->frames == 1002);
    REQUIRE(ready.waveform->sourceStart == 0 && ready.waveform->end == 1000);
    REQUIRE(ready.waveform->peaks.front().maximum > 3.99f);
    REQUIRE(ready.auditionLevelDb == -12 && ready.auditionAttenuationDb < -18);
    REQUIRE(engine.audition());
    float left[64]{}, right[64]{};
    engine.processAudio(left, right, 64, context.time);
    const auto safePeak = juce::Decibels::decibelsToGain(-18.0f);
    REQUIRE(left[0]==0 && std::abs(left[2] - safePeak * .4f) < 1.0e-5);
    engine.setAuditionLevelDb(12); // Clamp to 0 dB; protection still applies on replay.
    REQUIRE(engine.snapshot().auditionLevelDb == 0 && engine.audition());
    std::fill_n(left, 64, 0); std::fill_n(right, 64, 0);
    engine.processAudio(left, right, 64, context.time);
    REQUIRE(std::abs(left[2] - juce::Decibels::decibelsToGain(-6.0f)*.4f) < 1.0e-5);
    REQUIRE(engine.snapshot().waveform == ready.waveform && ready.waveform->peak > 3.99f);
    engine.setAuditionLevelDb(-100); REQUIRE(engine.snapshot().auditionLevelDb == -60);
    engine.setAuditionLevelDb(std::numeric_limits<float>::quiet_NaN()); REQUIRE(engine.snapshot().auditionLevelDb == -12);
    // Recapture outside the chosen interval but inside the same original chunk.
    float replacement[2]{0.7f,-0.7f};
    REQUIRE(session.commit(2000, 1000, replacement, 1));
    { const juce::ScopedLock guard(contextLock); context.capture = session.snapshot(); }
    juce::Thread::sleep(80);
    REQUIRE(engine.snapshot().phase == PreviewPhase::ready);
    REQUIRE(session.commit(10, 1000, replacement, 1));
    { const juce::ScopedLock guard(contextLock); context.capture = session.snapshot(); }
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::stale; });
    REQUIRE(! engine.audition());
    std::fill_n(left, 64, 0); engine.processAudio(left, right, 64, context.time);
    REQUIRE(std::all_of(left, left + 64, [](float v) { return v == 0; }));
    engine.generate(); waitUntil([&] { return engine.snapshot().phase == PreviewPhase::ready; });
    REQUIRE(engine.audition());
    auto changed = context.time; changed.bpm = 200; changed.sample = 6000; // PPQ 20 at 200 BPM, preserving Song origin zero.
    std::fill_n(left, 64, 0); engine.processAudio(left, right, 64, changed);
    REQUIRE(std::all_of(left, left + 64, [](float v) { return v == 0; }));
    { const juce::ScopedLock guard(contextLock); context.time = changed; }
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::stale; });
    engine.generate(); waitUntil([&] { return engine.snapshot().phase == PreviewPhase::ready; });
    { const juce::ScopedLock guard(contextLock); context.time.numerator = 3; }
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::stale; });
    engine.generate(); waitUntil([&] { return engine.snapshot().phase == PreviewPhase::ready; });
    { const juce::ScopedLock guard(contextLock); context.captureGeneration++; }
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::stale; });
    engine.generate(); waitUntil([&] { return engine.snapshot().phase == PreviewPhase::ready; });
    { const juce::ScopedLock guard(contextLock); context.time.rate = 2000; }
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::stale; });
    { const juce::ScopedLock guard(contextLock); context.time.rate = 1000; }
    engine.setSourceBars(1, INT64_MAX); engine.generate();
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::error; });
    REQUIRE(engine.snapshot().waveform != nullptr && ! engine.audition());
    engine.setSourceBars(1, 100001); engine.generate();
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::error; });
    REQUIRE(engine.snapshot().status.contains("256 MiB") && engine.snapshot().waveform != nullptr);
    engine.setSourceBars(10, 11); engine.generate();
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::error; });
    REQUIRE(engine.snapshot().status.contains("uncaptured"));
    engine.setSourceBars(1, 2);
    engine.loadIR(root.file.getChildFile("missing.wav"));
    waitUntil([&] { return engine.snapshot().phase == PreviewPhase::error; });
    REQUIRE(engine.snapshot().waveform != nullptr);
    // Rapid supersession and restore must never resurrect a result or start a voice.
    for (int i = 0; i < 20; ++i) { engine.generate(); engine.setSourceBars(1, 2 + i % 2); }
    engine.reset(); juce::Thread::sleep(80);
    REQUIRE(engine.snapshot().phase == PreviewPhase::empty && ! engine.snapshot().waveform && ! engine.audition());
    std::fill_n(left, 64, 0.0f);
    engine.processAudio(left, right, 64, context.time);
    REQUIRE(std::all_of(left, left + 64, [](float v) { return v == 0; }));
}); }
