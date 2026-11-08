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
    {
        if (musicalRoute.load())
            invalidate();
        changed();
    }
    return true;
}

bool LoopCapture::setSettings(Settings next)
{
    const juce::ScopedLock lock(control);
    if (busy() || (next.method != Method::followGate && next.method != Method::fixedLength)
        || next.bars < 0 || next.bars > maxBars || next.beats < 0 || next.beats > maxBeats
        || (next.bars == 0 && next.beats == 0))
        return false;
    settings = next;
    if (!musicalRoute.exchange(true))
        invalidate();
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
    armedSettings.store(static_cast<uint32_t>(settings.method) | (static_cast<uint32_t>(settings.bars) << 8)
                        | (static_cast<uint32_t>(settings.beats) << 16));
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
        clock = {};
        clockReady.store(false, std::memory_order_relaxed);
    }
    auto current = state.load();
    const bool musical = active(phase(current))
        ? (armedSettings.load() & 255) != static_cast<uint32_t>(Method::legacyGate) : musicalRoute.load();
    if (musical)
    {
        if (enabled() || active(phase(current)))
            processMusical(input, midi, current, eligible, generation);
        return;
    }
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
            recordedLength = {};
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

void LoopCapture::Clock::clear() noexcept
{
    lastBeat = -1;
    period = beatsSinceBar = meter = 0;
    sawBar = false;
}

bool LoopCapture::Clock::expired(int64_t sample) const noexcept
{
    // CLKD produces adjacent integer intervals at fractional samples per beat.
    return period > 0 && sample - lastBeat > period + 1;
}

bool LoopCapture::Clock::tick(int64_t sample, int rate, bool bar) noexcept
{
    bool continuous = true;
    if (lastBeat >= 0)
    {
        const auto interval = sample - lastBeat;
        continuous = interval >= rate / 10 && interval <= 3 * rate
            && (period == 0 || std::abs(interval - period) <= 1);
        if (!continuous)
            clear();
        else
            period = static_cast<int>(interval);
    }

    if (sawBar && lastBeat >= 0)
    {
        ++beatsSinceBar;
        if (bar)
        {
            if (beatsSinceBar > maxBeats + 1 || (meter && meter != beatsSinceBar))
            {
                clear();
                continuous = false;
            }
            else
                meter = beatsSinceBar;
        }
        else if (beatsSinceBar >= (meter ? meter : maxBeats + 1))
        {
            clear();
            continuous = false;
        }
    }
    if (bar)
    {
        sawBar = true;
        beatsSinceBar = 0;
    }
    lastBeat = sample;
    return continuous;
}

void LoopCapture::processMusical(const juce::AudioBuffer<float>& input, const juce::MidiBuffer& midi,
                                 uint64_t current, bool eligible, uint64_t generation) noexcept
{
    const int count = input.getNumSamples();
    const int rate = preparedRate.load();
    const auto armed = armedSettings.load();
    const bool fixed = (armed & 255) == static_cast<uint32_t>(Method::fixedLength);
    const auto moveTo = [&](Phase next)
    {
        auto expected = current;
        const auto nextState = withPhase(current, next);
        current = state.compare_exchange_strong(expected, nextState) ? nextState : withPhase(current, Phase::idle);
    };
    const auto recording = [&] { return phase(current) == Phase::recording || phase(current) == Phase::waitForBar; };
    const auto loseClock = [&]
    {
        clock.clear();
        if (recording())
            moveTo(Phase::interrupted);
    };

    if (active(phase(current)) && phase(current) != Phase::settling)
    {
        if (!eligible || armEpoch.load() != observedEpoch || armGeneration.load() != generation)
            moveTo(Phase::interrupted);
        else if (input.getNumChannels() < 2 || rate == 0)
            moveTo(Phase::invalidAudio);
        else if (phase(current) == Phase::armed)
            moveTo(gate == Gate::low ? Phase::waitForRise : Phase::waitForLow);
    }

    const auto blockStart = clock.nextSample;
    clock.nextSample += count;
    int cursor = 0;
    const auto appendTo = [&](int end)
    {
        const int length = end - cursor;
        if (recording() && length > 0)
        {
            if (length > recordingRate * maxSeconds - frames)
                moveTo(Phase::tooLong);
            else
            {
                audio.copyFrom(0, frames, input, 0, cursor, length);
                audio.copyFrom(1, frames, input, 1, cursor, length);
                frames += length;
            }
        }
        cursor = end;
    };
    auto event = midi.cbegin();
    while (event != midi.cend())
    {
        const auto offset = (*event).samplePosition;
        auto nextGate = gate;
        bool beat = false, bar = false, phrase = false, reset = false;
        while (event != midi.cend() && (*event).samplePosition == offset)
        {
            const auto message = *event++;
            if (message.numBytes != 3)
                continue;
            const auto type = message.data[0] & 0xf0;
            if (type != 0x80 && type != 0x90)
                continue;
            const auto note = message.data[1];
            const bool high = type == 0x90 && message.data[2] != 0;
            if (note == gateNote)
                nextGate = high ? Gate::high : Gate::low;
            else if (high)
            {
                beat |= note == beatNote;
                bar |= note == barNote;
                phrase |= note == phraseNote;
                reset |= note == resetNote;
            }
        }
        if (!(beat || bar || phrase || reset || nextGate != gate))
            continue;
        if (offset < cursor || offset >= count)
        {
            if (active(phase(current)) && phase(current) != Phase::settling)
                moveTo(Phase::invalidAudio);
            loseClock();
            gate = Gate::unknown;
            continue;
        }
        const auto sample = blockStart + offset;
        if (reset || clock.expired(sample))
            loseClock();
        appendTo(offset);
        if (bar && !beat)
            loseClock();
        if (beat && rate > 0 && !clock.tick(sample, rate, bar) && recording())
            moveTo(Phase::interrupted);

        const bool rising = gate == Gate::low && nextGate == Gate::high;
        const bool falling = gate == Gate::high && nextGate == Gate::low;
        gate = nextGate;

        if (recording())
        {
            if (beat)
                ++recordedLength.beats;
            if (falling && !fixed)
                moveTo(Phase::waitForBar);
            if ((fixed && beat && recordedLength.beats == fixedEnd)
                || (phase(current) == Phase::waitForBar && bar))
                moveTo(frames > 0 ? Phase::settling : Phase::invalidAudio);
        }
        else if (phase(current) == Phase::waitForLow && gate == Gate::low)
            moveTo(Phase::waitForRise);
        else if (phase(current) == Phase::waitForRise && rising)
        {
            const auto additionalBeats = static_cast<int>((armed >> 16) & 255);
            if (reset || !beat || !bar || !phrase || clock.meter == 0)
                moveTo(Phase::invalidStart);
            else if (fixed && additionalBeats >= clock.meter)
                moveTo(Phase::invalidLength);
            else
            {
                frames = 0;
                recordingRate = rate;
                recordedLength = {0, clock.meter};
                fixedEnd = static_cast<int>((armed >> 8) & 255) * clock.meter + additionalBeats;
                recordingTicket = current & ~uint64_t{255};
                moveTo(Phase::recording);
            }
        }
    }
    if (clock.expired(clock.nextSample - 1))
        loseClock();
    appendTo(count);
    clockReady.store(clock.meter > 0, std::memory_order_relaxed);
}

LoopCapture::View LoopCapture::view() const
{
    const juce::ScopedLock lock(control);
    auto visiblePhase = phase(state.load());
    const auto interrupted = active(visiblePhase) && armEpoch.load() != prepareEpoch.load();
    if (interrupted)
        visiblePhase = Phase::interrupted;
    return {enabled(), visiblePhase, hasAcceptedLoop(), restoreGeneration.has_value(), interrupted,
            accepted.rate > 0 ? static_cast<double>(accepted.frames) / accepted.rate : 0, acceptedFile, acceptedWaveform,
            settings, clockReady.load(std::memory_order_relaxed), accepted.musicalLength};
}

std::optional<LoopCapture::Source> LoopCapture::source() const
{
    const juce::ScopedLock lock(control);
    if (!hasAcceptedLoop() || restoreGeneration || acceptedFile == juce::File{})
        return {};
    return Source{accepted.id, accepted.hash, accepted.rate, accepted.frames, acceptedFile, accepted.musicalLength};
}

bool LoopCapture::Source::read(int first, int end, uint64_t budget, std::vector<float>& output,
                               juce::String& error) const
{
    const auto fail = [&](const char* message) { error = message; return false; };
    if (!Take{id, hash, rate, frames, musicalLength}.valid() || first < 0 || end <= first || end > frames)
        return fail("The selected interval is outside the verified Loop take.");
    const auto pcmBytes = uint64_t{8} * static_cast<unsigned>(frames);
    const auto selectionBytes = uint64_t{8} * static_cast<unsigned>(end - first);
    auto input = file.createInputStream();
    if (!input)
        return fail("The Loop WAV is unavailable.");
    const auto size = static_cast<uint64_t>(input->getTotalLength());
    if (size < pcmBytes + 44 || size > pcmBytes + 4096 || size + 2 * selectionBytes > budget)
        return fail("The Loop WAV is incomplete or exceeds the preparation budget.");

    // Verify the exact bytes used to decode. A replaced file cannot race a
    // separate verification and subsequent reopen of the path.
    juce::MemoryBlock bytes(static_cast<size_t>(size));
    if (input->read(bytes.getData(), static_cast<int>(size)) != static_cast<int>(size))
        return fail("Could not read the complete Loop WAV.");
    m3::ContentHash checksum;
    checksum.update(bytes.getData(), bytes.getSize());
    if (checksum.finish() != hash)
        return fail("The Loop WAV no longer matches the accepted take.");
    juce::WavAudioFormat wav;
    const std::unique_ptr<juce::AudioFormatReader> reader(
        wav.createReaderFor(new juce::MemoryInputStream(bytes, false), true));
    juce::AudioBuffer<float> decoded(2, end - first);
    if (!reader || reader->sampleRate != rate || reader->lengthInSamples != frames
        || reader->numChannels != 2 || reader->bitsPerSample != 32 || !reader->usesFloatingPointData
        || !reader->read(&decoded, 0, end - first, first, true, true))
        return fail("Could not decode the verified stereo Loop interval.");
    std::vector<float> selected;
    selected.reserve(static_cast<size_t>(2 * (end - first)));
    for (int i = 0; i < end - first; ++i)
        for (int channel = 0; channel < 2; ++channel)
        {
            const auto value = decoded.getSample(channel, i);
            if (!std::isfinite(value))
                return fail("The selected Loop audio contains invalid samples.");
            selected.push_back(value);
        }
    output = std::move(selected);
    error.clear();
    return true;
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
    if (settings.method != Method::legacyGate && (enabled() || settings != Settings{}))
    {
        stateXml.setAttribute("learnLoopMethod", static_cast<int>(settings.method));
        stateXml.setAttribute("learnLoopBars", settings.bars);
        stateXml.setAttribute("learnLoopBeats", settings.beats);
    }
    if (hasAcceptedLoop())
    {
        stateXml.setAttribute("learnLoopTakeVersion", 1);
        stateXml.setAttribute("learnLoopTakeId", accepted.id);
        stateXml.setAttribute("learnLoopTakeHash", accepted.hash);
        stateXml.setAttribute("learnLoopTakeRate", accepted.rate);
        stateXml.setAttribute("learnLoopTakeFrames", accepted.frames);
        if (accepted.musicalLength.beats > 0)
        {
            stateXml.setAttribute("learnLoopTakeBeats", accepted.musicalLength.beats);
            stateXml.setAttribute("learnLoopTakeBeatsPerBar", accepted.musicalLength.beatsPerBar);
        }
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
    settings = {};
    if (!stateXml.hasAttribute("learnLoopMethod") && enabled())
        settings.method = Method::legacyGate;
    else
    {
        if (stateXml.getIntAttribute("learnLoopMethod") == static_cast<int>(Method::fixedLength))
            settings.method = Method::fixedLength;
        const auto bars = stateXml.getIntAttribute("learnLoopBars", 1);
        const auto beats = stateXml.getIntAttribute("learnLoopBeats", 0);
        if (bars >= 0 && bars <= maxBars && beats >= 0 && beats <= maxBeats && (bars || beats))
        {
            settings.bars = bars;
            settings.beats = beats;
        }
    }
    musicalRoute.store(settings.method != Method::legacyGate);
    if (stateXml.hasAttribute("learnLoopTakeVersion"))
    {
        acceptedLoop.store(true);
        accepted = {stateXml.getStringAttribute("learnLoopTakeId"), stateXml.getStringAttribute("learnLoopTakeHash"),
                    stateXml.getIntAttribute("learnLoopTakeRate"), stateXml.getIntAttribute("learnLoopTakeFrames"),
                    {stateXml.getIntAttribute("learnLoopTakeBeats"), stateXml.getIntAttribute("learnLoopTakeBeatsPerBar")}};
        if (accepted.musicalLength.beats <= 0 || accepted.musicalLength.beatsPerBar < 1
            || accepted.musicalLength.beatsPerBar > maxBeats + 1 || accepted.musicalLength.beats > 256)
            accepted.musicalLength = {};
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
    Take take{juce::Uuid().toString(), {}, recordingRate, frames, recordedLength};
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
