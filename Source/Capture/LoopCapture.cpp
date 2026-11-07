#include "LoopCapture.h"
#include <Core/ContentHash.h>
#include <Core/ManagedDirectory.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <algorithm>
#include <cmath>

#define NOMINMAX
#include <Windows.h>

LoopCapture::LoopCapture(std::function<std::optional<Session>()> getSession, std::function<uint64_t()> getGeneration,
                         std::function<void()> notifyChanged)
    : Thread("ReverbArise Loop WAV"), sessionNow(std::move(getSession)),
      generationNow(std::move(getGeneration)), changed(std::move(notifyChanged)) {}

LoopCapture::~LoopCapture() { shutdown(); }
void LoopCapture::shutdown() { stopThread(-1); }
bool LoopCapture::View::busy() const noexcept { return active(phase) || pendingInterruption; }
bool LoopCapture::busy() const noexcept { return active(phase(state.load())); }

bool LoopCapture::setEnabled(bool enabled)
{
    const juce::ScopedLock lock(control);
    if (busy())
        return false;
    if (loopEnabled.exchange(enabled) != enabled)
        changed();
    return true;
}

bool LoopCapture::arm(const Session& session)
{
    const juce::ScopedLock lock(control);
    if (!enabled() || busy() || restoreGeneration || preparedRate.load() == 0
        || !session.location || !session.location->writable)
        return false;
    if (audio.getNumChannels() == 0)
        audio.setSize(2, maxRate * maxSeconds);
    if (!isThreadRunning() && !startThread())
        return false;
    target = session;
    settlementCancelled = false;
    armEpoch.store(prepareEpoch.load());
    armGeneration.store(session.generation);
    state.store(withPhase(state.load() + 256, Phase::armed));
    notify();
    return true;
}

void LoopCapture::cancel()
{
    const juce::ScopedLock lock(control);
    auto current = state.load();
    while (active(phase(current)))
    {
        if (phase(current) == Phase::settling)
        {
            settlementCancelled = true;
            return;
        }
        if (state.compare_exchange_weak(current, withPhase(current, Phase::cancelled)))
            return;
    }
}

void LoopCapture::prepare(double rate) noexcept
{
    preparedRate.store(rate >= 8000 && rate <= maxRate && rate == std::floor(rate) ? static_cast<int>(rate) : 0);
    invalidate();
}

void LoopCapture::process(const juce::AudioBuffer<float>& input, const juce::MidiBuffer& midi,
                          bool eligible, uint64_t generation) noexcept
{
    const auto epoch = prepareEpoch.load();
    if (epoch != observedEpoch)
    {
        observedEpoch = epoch;
        gate = Gate::unknown;
    }
    auto current = state.load();
    const auto moveTo = [&](Phase next)
    {
        auto expected = current;
        const auto nextState = withPhase(current, next);
        if (state.compare_exchange_strong(expected, nextState))
            current = nextState;
        else
            current = withPhase(current, Phase::idle); // A control action superseded this block's ticket.
    };
    if (active(phase(current)) && phase(current) != Phase::settling)
    {
        if (!eligible || armEpoch.load() != epoch || armGeneration.load() != generation)
            moveTo(Phase::interrupted);
        else if (input.getNumChannels() < 2 || preparedRate.load() == 0)
            moveTo(Phase::invalidAudio);
        else if (phase(current) == Phase::armed)
            moveTo(gate == Gate::low ? Phase::waitForRise : Phase::waitForLow);
    }

    int cursor = 0;
    const auto appendTo = [&](int end)
    {
        const int count = end - cursor;
        if (phase(current) == Phase::recording && count > 0)
        {
            if (count > recordingRate * maxSeconds - frames)
                moveTo(Phase::tooLong);
            else
            {
                audio.copyFrom(0, frames, input, 0, cursor, count);
                audio.copyFrom(1, frames, input, 1, cursor, count);
                frames += count;
            }
        }
        cursor = end;
    };
    for (const auto event : midi)
    {
        if (event.numBytes != 3 || event.data[1] != gateNote)
            continue;
        const auto type = event.data[0] & 0xf0;
        if (type != 0x80 && type != 0x90)
            continue;
        const auto offset = event.samplePosition;
        if (offset < cursor || offset >= input.getNumSamples())
        {
            if (active(phase(current)) && phase(current) != Phase::settling)
                moveTo(Phase::invalidAudio);
            gate = Gate::unknown;
            continue;
        }
        appendTo(offset);
        const bool high = type == 0x90 && event.data[2] != 0;
        if (high && gate == Gate::low && phase(current) == Phase::waitForRise)
        {
            frames = 0;
            recordingRate = preparedRate.load();
            recordingTicket = current & ~uint64_t{255};
            moveTo(Phase::recording);
        }
        else if (!high && phase(current) == Phase::waitForLow)
            moveTo(Phase::waitForRise);
        else if (!high && gate == Gate::high && phase(current) == Phase::recording)
            moveTo(frames > 0 ? Phase::settling : Phase::invalidAudio);
        gate = high ? Gate::high : Gate::low;
    }
    appendTo(input.getNumSamples());
}

LoopCapture::View LoopCapture::view() const
{
    const juce::ScopedLock lock(control);
    auto visiblePhase = phase(state.load());
    const auto interrupted = active(visiblePhase) && armEpoch.load() != prepareEpoch.load();
    if (interrupted)
        visiblePhase = Phase::interrupted;
    return {enabled(), visiblePhase, hasAcceptedLoop(), restoreGeneration.has_value(), interrupted,
            accepted.rate > 0 ? static_cast<double>(accepted.frames) / accepted.rate : 0, acceptedFile, acceptedWaveform};
}

void LoopCapture::acceptSong(uint64_t generation)
{
    const juce::ScopedLock lock(control);
    if (generationNow() == generation)
    {
        clearAccepted();
        if (const auto current = state.load(); !active(phase(current)))
            state.store(withPhase(current, Phase::idle));
    }
}

void LoopCapture::clearAccepted()
{
    acceptedLoop.store(false);
    accepted = {};
    acceptedFile = juce::File{};
    acceptedWaveform = {};
    restoreGeneration.reset();
}

bool LoopCapture::Take::valid() const
{
    return id.length() == 32 && id.containsOnly("0123456789abcdef")
        && hash.length() == 64 && hash.containsOnly("0123456789abcdef")
        && rate >= 8000 && rate <= maxRate && frames > 0 && frames <= rate * maxSeconds;
}

juce::File LoopCapture::takeFile(const Session& session, const Take& take)
{
    if (!session.location || take.id.length() != 32 || !take.id.containsOnly("0123456789abcdef"))
        return {};
    const auto root = session.location->projectsRoot;
    auto file = root.getChildFile(session.project).getChildFile(session.instance)
                    .getChildFile("loop-takes").getChildFile(take.id + ".wav");
    if (!file.isAChildOf(root))
        return {};
    for (auto entry = file; entry != root; entry = entry.getParentDirectory())
    {
        const auto attributes = GetFileAttributesW(entry.getFullPathName().toWideCharPointer());
        if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
            return {};
    }
    return file;
}

std::optional<LoopCapture::Waveform> LoopCapture::verify(const juce::File& file, const Take& take)
{
    if (!take.valid() || file == juce::File{})
        return {};
    const auto bytes = file.getSize();
    const auto audioBytes = int64_t{8} * take.frames;
    if (bytes < 44 + audioBytes || bytes > 4096 + audioBytes)
        return {};
    juce::WavAudioFormat wav;
    auto input = file.createInputStream();
    if (!input)
        return {};
    auto reader = std::unique_ptr<juce::AudioFormatReader>(wav.createReaderFor(input.release(), true));
    if (!reader || reader->numChannels != 2 || reader->bitsPerSample != 32 || !reader->usesFloatingPointData
        || reader->sampleRate != take.rate || reader->lengthInSamples != take.frames || m3::fileHash(file) != take.hash)
        return {};

    Waveform waveform;
    waveform.size = std::min(take.frames, static_cast<int>(waveform.peaks.size()));
    juce::AudioBuffer<float> block(2, 4096);
    for (int start = 0; start < take.frames; start += block.getNumSamples())
    {
        const auto count = std::min(block.getNumSamples(), take.frames - start);
        if (!reader->read(&block, 0, count, start, true, true))
            return {};
        for (int i = 0; i < count; ++i)
        {
            const auto left = block.getSample(0, i), right = block.getSample(1, i);
            if (!std::isfinite(left) || !std::isfinite(right))
                return {};
            auto& peak = waveform.peaks[static_cast<size_t>((start + i) * waveform.size / take.frames)];
            peak.minimum = std::min(peak.minimum, std::min(left, right));
            peak.maximum = std::max(peak.maximum, std::max(left, right));
        }
    }
    return waveform;
}

void LoopCapture::save(juce::XmlElement& stateXml) const
{
    const juce::ScopedLock lock(control);
    if (enabled())
        stateXml.setAttribute("learnLoopEnabled", true);
    if (hasAcceptedLoop())
    {
        stateXml.setAttribute("learnLoopTakeVersion", 1);
        stateXml.setAttribute("learnLoopTakeId", accepted.id);
        stateXml.setAttribute("learnLoopTakeHash", accepted.hash);
        stateXml.setAttribute("learnLoopTakeRate", accepted.rate);
        stateXml.setAttribute("learnLoopTakeFrames", accepted.frames);
    }
}

void LoopCapture::restore(const juce::XmlElement& stateXml, uint64_t generation)
{
    const juce::ScopedLock lock(control);
    cancel();
    invalidate();
    clearAccepted();
    if (const auto current = state.load(); phase(current) != Phase::settling)
        state.store(withPhase(current, Phase::idle));
    loopEnabled.store(stateXml.getBoolAttribute("learnLoopEnabled", false));
    if (stateXml.hasAttribute("learnLoopTakeVersion"))
    {
        acceptedLoop.store(true);
        accepted = {stateXml.getStringAttribute("learnLoopTakeId"), stateXml.getStringAttribute("learnLoopTakeHash"),
                    stateXml.getIntAttribute("learnLoopTakeRate"), stateXml.getIntAttribute("learnLoopTakeFrames")};
        if (stateXml.getIntAttribute("learnLoopTakeVersion") == 1 && accepted.valid() && (isThreadRunning() || startThread()))
            restoreGeneration = generation;
        notify();
    }
}

void LoopCapture::run()
{
    while (!threadShouldExit())
    {
        const auto current = state.load();
        if (phase(current) == Phase::settling)
            settle(current);
        verifyRestored();
        wait(busy() ? 15 : 100);
    }
}

void LoopCapture::settle(uint64_t current)
{
    Session session;
    bool cancelled = false;
    {
        const juce::ScopedLock lock(control);
        session = target;
        cancelled = settlementCancelled;
    }
    Take take{juce::Uuid().toString(), {}, recordingRate, frames};
    Phase result = Phase::storageError;
    juce::File file;
    std::optional<Waveform> waveform;
    if (!cancelled && recordingTicket == (current & ~uint64_t{255}) && armEpoch.load() == prepareEpoch.load())
    {
        float peak = 0;
        bool finite = true;
        for (int channel = 0; channel < 2; ++channel)
            for (int i = 0; i < take.frames; ++i)
            {
                const auto sample = audio.getReadPointer(channel)[i];
                finite = finite && std::isfinite(sample);
                peak = std::max(peak, std::abs(sample));
            }
        if (!finite)
            result = Phase::invalidAudio;
        else if (peak <= 1.0e-6f)
            result = Phase::silent;
        else if (const auto destination = takeFile(session, take); destination != juce::File{}
                 && ensureManagedDirectory(destination.getParentDirectory()).wasOk())
        {
            juce::TemporaryFile temporary(destination);
            juce::WavAudioFormat format;
            const auto options = juce::AudioFormatWriterOptions{}.withSampleRate(take.rate)
                .withNumChannels(2).withBitsPerSample(32)
                .withSampleFormat(juce::AudioFormatWriterOptions::SampleFormat::floatingPoint);
            std::unique_ptr<juce::OutputStream> output(temporary.getFile().createOutputStream());
            if (output)
            {
                auto writer = format.createWriterFor(output, options);
                const bool wrote = writer && writer->writeFromAudioSampleBuffer(audio, 0, take.frames);
                writer.reset();
                if (wrote)
                {
                    take.hash = m3::fileHash(temporary.getFile());
                    waveform = verify(temporary.getFile(), take);
                    if (waveform && !destination.exists() && temporary.overwriteTargetFileWithTemporary())
                    {
                        file = destination;
                        result = Phase::ready;
                    }
                }
            }
        }
    }
    const auto latest = sessionNow();
    {
        const juce::ScopedLock lock(control);
        if (settlementCancelled)
            result = Phase::cancelled;
        else if (armEpoch.load() != prepareEpoch.load() || generationNow() != session.generation || !latest
                 || latest->generation != session.generation || latest->location != session.location
                 || latest->project != session.project || latest->instance != session.instance)
            result = Phase::interrupted;
        if (result == Phase::ready)
        {
            accepted = take;
            acceptedFile = file;
            acceptedWaveform = *waveform;
            acceptedLoop.store(true);
            changed();
        }
        state.store(withPhase(current, result));
    }
    if (result != Phase::ready && file != juce::File{})
        file.deleteFile();
}

void LoopCapture::verifyRestored()
{
    std::optional<uint64_t> generation;
    Take take;
    {
        const juce::ScopedLock lock(control);
        generation = restoreGeneration;
        if (!generation)
            return;
        take = accepted;
    }
    const auto session = sessionNow();
    if (!session)
        return;
    const auto file = session->generation == *generation ? takeFile(*session, take) : juce::File{};
    const auto waveform = verify(file, take);
    const juce::ScopedLock lock(control);
    if (restoreGeneration == generation)
    {
        if (waveform)
        {
            acceptedFile = file;
            acceptedWaveform = *waveform;
        }
        restoreGeneration.reset();
    }
}
