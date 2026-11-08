#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

class OneTake
{
public:
    enum class State { idle, waitForLow, waitForRise, recording, complete, cancelled, rejected };
    enum class Gate { unknown, low, high };
    struct Take
    {
        std::int64_t start = 0;
        std::int64_t end = 0;
        int rate = 0;
        int frames = 0;
    };

    explicit OneTake(int capacity)
        : left(static_cast<std::size_t>(capacity)), right(static_cast<std::size_t>(capacity)) {}

    void prepare(int sampleRate) noexcept
    {
        rate = sampleRate;
        gate = Gate::unknown;
        if (active())
            state = State::rejected;
    }

    void arm() noexcept
    {
        if (active() || state == State::complete)
            return;
        take = {};
        if (rate <= 0 || rate > static_cast<int>(left.size()))
        {
            state = State::rejected;
            return;
        }
        state = gate == Gate::low ? State::waitForRise : State::waitForLow;
    }

    void cancel() noexcept
    {
        if (active())
            state = State::cancelled;
    }

    void edge(bool high, std::int64_t sample) noexcept
    {
        if (high)
        {
            if (state == State::waitForRise && gate == Gate::low)
            {
                take = { sample, 0, rate, 0 };
                state = State::recording;
            }
            gate = Gate::high;
            return;
        }

        if (state == State::waitForLow)
            state = State::waitForRise;
        else if (state == State::recording && gate == Gate::high)
        {
            take.end = sample;
            state = take.frames > 0 && take.end - take.start == take.frames
                ? State::complete : State::rejected;
        }
        gate = Gate::low;
    }

    void append(const float* leftInput, const float* rightInput, int frames) noexcept
    {
        if (state != State::recording || frames == 0)
            return;
        const int frameLimit = std::min(static_cast<int>(left.size()), rate * 15);
        if (leftInput == nullptr || rightInput == nullptr || frames > frameLimit - take.frames)
        {
            state = State::rejected;
            return;
        }
        std::copy_n(leftInput, frames, left.data() + take.frames);
        std::copy_n(rightInput, frames, right.data() + take.frames);
        take.frames += frames;
    }

    void releaseTake() noexcept
    {
        if (state == State::complete)
            state = State::idle;
    }

    State state = State::idle;
    Gate gate = Gate::unknown;
    Take take;
    std::vector<float> left;
    std::vector<float> right;

private:
    bool active() const noexcept
    {
        return state == State::waitForLow || state == State::waitForRise || state == State::recording;
    }

    int rate = 0;
};
