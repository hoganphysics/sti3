
#include "EngineClock.h"

using STI::Engine::EngineClock;


EngineClock::EngineClock()
{
}

void EngineClock::reset()
{
    reset(0);
}

void EngineClock::reset(double offset)
{
    tStart = std::chrono::high_resolution_clock::now();
    tStart += std::chrono::nanoseconds( static_cast<int64_t>(offset) );
}

int64_t EngineClock::getTime() const
{
    auto tNow = std::chrono::high_resolution_clock::now();

    int64_t delta_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tNow - tStart).count();

    return delta_ns;
}


int64_t EngineClock::getWaitInterval(double time) const
{
    return static_cast<int64_t>(time) - getTime();
}

