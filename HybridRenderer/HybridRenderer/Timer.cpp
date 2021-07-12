#include "Timer.h"

Timer::Timer()
{
    prevTime = std::chrono::high_resolution_clock::now();
}

Timer::~Timer()
{
}

void Timer::update()
{
    auto time = std::chrono::high_resolution_clock::now();
    dt = std::chrono::duration<float, std::chrono::milliseconds::period>(time - prevTime).count() / 1000.0f;
    prevTime = std::chrono::high_resolution_clock::now();

    elapsed = std::chrono::duration<float, std::chrono::milliseconds::period>(time - startTime).count() / 1000.0f;
}
