#include "OneTake.h"
#include <iostream>

namespace
{
using State = OneTake::State;

bool check(bool result, const char* message)
{
    if (!result)
        std::cerr << message << '\n';
    return result;
}
}

int main()
{
    OneTake capture(32);
    capture.prepare(8);
    capture.edge(true, 0);
    capture.arm();
    bool passed = check(capture.state == State::waitForLow, "Armed high");
    capture.edge(true, 2);
    passed &= check(capture.state == State::waitForLow, "Duplicate high started capture");
    capture.edge(false, 4);
    passed &= check(capture.state == State::waitForRise, "Fall did not prepare the next rise");
    capture.edge(true, 6);
    const float left[]{ 0, 1, 2, 3, 4, 5, 6, 7 };
    const float right[]{ 0, -1, -2, -3, -4, -5, -6, -7 };
    capture.append(left + 6, right + 6, 2);
    capture.append(left, right, 3);
    capture.edge(false, 11);
    passed &= check(capture.state == State::complete && capture.take.frames == 5
                    && capture.take.start == 6 && capture.take.end == 11, "Partial blocks");
    passed &= check(capture.left[0] == 6 && capture.left[4] == 2
                    && capture.right[0] == -6 && capture.right[4] == -2, "Stereo payload");
    capture.edge(true, 16);
    passed &= check(capture.state == State::complete && capture.take.start == 6, "Repeat replaced take");
    capture.releaseTake();
    capture.edge(false, 18);
    capture.arm();
    capture.edge(true, 20);
    capture.append(left, right, 2);
    capture.cancel();
    capture.edge(false, 22);
    passed &= check(capture.state == State::cancelled, "Cancelled take settled");
    capture.arm();
    capture.edge(true, 24);
    capture.append(left, right, 8);
    capture.prepare(16);
    passed &= check(capture.state == State::rejected, "Rate change retained an active take");
    capture.edge(false, 32);
    capture.arm();
    capture.edge(true, 34);
    for (int block = 0; block < 5; ++block)
        capture.append(left, right, 8);
    passed &= check(capture.state == State::rejected, "Buffer limit did not reject the take");
    if (passed)
        std::cout << "One-take boundary, stereo copy, high gate, cancel, rate and capacity checks passed.\n";
    return passed ? 0 : 1;
}
