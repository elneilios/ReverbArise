#include "FlStudioHostTrace.h"

std::unique_ptr<FlStudioHostTrace> FlStudioHostTrace::create()
{
    const juce::File directory(PREVERB_FL_STUDIO_TRACE_DIR);
    if (!directory.getChildFile("ENABLE").existsAsFile())
        return {};
    auto file = directory.getChildFile("host-" + juce::Uuid().toString() + ".csv").createOutputStream();
    if (!file || !file->openedOk())
        return {};
    return std::unique_ptr<FlStudioHostTrace>(new FlStudioHostTrace(std::move(file)));
}

FlStudioHostTrace::FlStudioHostTrace(std::unique_ptr<juce::FileOutputStream> file)
    : juce::Thread("FL Studio host trace"), output(std::move(file))
{
    *output << "block,callback_ticks,frames,rate,offline,playing,sample,ppq,bpm,numerator,denominator,"
               "loop_start,loop_end,prepares,resets,releases,first_dry,midi_offset,midi_size,status,data1,data2\n";
    startThread();
}

FlStudioHostTrace::~FlStudioHostTrace()
{
    stopThread(-1);
    drain();
    *output << "# dropped=" << static_cast<int>(dropped.load())
            << ",midi_overflow=" << static_cast<int>(midiOverflow.load())
            << ",ticks_per_second=" << juce::Time::getHighResolutionTicksPerSecond() << "\n";
    output->flush();
}

void FlStudioHostTrace::observe(const HostPositionDiagnostics& timing, const juce::AudioBuffer<float>& audio,
                                const juce::MidiBuffer& midi, bool offline) noexcept
{
    Row row;
    row.timing = timing;
    row.block = block++;
    row.callbackTicks = juce::Time::getHighResolutionTicks();
    row.offline = offline;
    row.prepares = prepared.load(std::memory_order_relaxed);
    row.resets = reset.load(std::memory_order_relaxed);
    row.releases = released.load(std::memory_order_relaxed);
    if (audio.getNumChannels() > 0)
    {
        const auto* left = audio.getReadPointer(0);
        for (int i = 0; i < audio.getNumSamples(); ++i)
        {
            if (std::abs(left[i]) > 0.04f)
            {
                row.firstDry = i;
                break;
            }
        }
    }
    publish(row);

    unsigned count = 0;
    for (const auto event : midi)
    {
        if (++count > 128)
        {
            midiOverflow.fetch_add(1, std::memory_order_relaxed);
            break;
        }
        row.midiOffset = event.samplePosition;
        row.midiSize = event.numBytes;
        row.status = event.numBytes > 0 ? event.data[0] : 0;
        row.data1 = event.numBytes > 1 ? event.data[1] : 0;
        row.data2 = event.numBytes > 2 ? event.data[2] : 0;
        publish(row);
    }
}

void FlStudioHostTrace::publish(const Row& row) noexcept
{
    const auto slot = fifo.write(published < maximumRows ? 1 : 0);
    if (slot.blockSize1 != 1)
    {
        dropped.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    rows[static_cast<size_t>(slot.startIndex1)] = row;
    ++published;
}

void FlStudioHostTrace::run()
{
    while (!threadShouldExit())
    {
        drain();
        wait(50);
    }
}

void FlStudioHostTrace::drain()
{
    const auto ready = fifo.read(capacity);
    const auto emit = [&](int start, int size)
    {
        for (int index = start; index < start + size; ++index)
        {
            const auto& row = rows[static_cast<size_t>(index)];
            const auto& time = row.timing;
            *output << row.block << ',' << row.callbackTicks << ',' << time.blockSize << ','
                    << juce::String(time.sampleRate, 3) << ',' << static_cast<int>(row.offline) << ','
                    << static_cast<int>(time.isPlaying) << ','
                    << (time.samplePositionAvailable ? juce::String(time.samplePosition) : "") << ','
                    << (time.ppqPositionAvailable ? juce::String(time.ppqPosition, 12) : "") << ','
                    << (time.tempoAvailable ? juce::String(time.bpm, 9) : "") << ','
                    << (time.meterAvailable ? juce::String(time.numerator) : "") << ','
                    << (time.meterAvailable ? juce::String(time.denominator) : "") << ','
                    << (time.isLooping ? juce::String(time.loopStart) : "") << ','
                    << (time.isLooping ? juce::String(time.loopEnd) : "") << ','
                    << static_cast<int>(row.prepares) << ',' << static_cast<int>(row.resets) << ','
                    << static_cast<int>(row.releases) << ',' << row.firstDry << ',' << row.midiOffset
                    << ',' << row.midiSize << ',' << row.status << ',' << row.data1 << ',' << row.data2 << '\n';
        }
    };
    emit(ready.startIndex1, ready.blockSize1);
    emit(ready.startIndex2, ready.blockSize2);
}
