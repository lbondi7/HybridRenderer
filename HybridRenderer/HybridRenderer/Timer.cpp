#include "Timer.h"

Timer::Timer()
{
    prevTime = std::chrono::high_resolution_clock::now();
    SetFrameRate(2000);
    average = false;
    mspfCount = 10;
}

Timer::~Timer()
{
}

void Timer::Update()
{
    auto time = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration<double, std::chrono::milliseconds::period>(time - prevTime).count();
    prevTime = std::chrono::high_resolution_clock::now();

    elapsed = std::chrono::duration<double, std::chrono::milliseconds::period>(time - startTime).count() / 1000.0;

    mspf = deltaTime;
    prevMspf.emplace_front(deltaTime);
    if (prevMspf.size() > mspfCount)
        prevMspf.pop_back();

    if (average) {
        mspf = 0.0;
        for (size_t i = 0; i < prevMspf.size(); i++)
        {
            mspf += prevMspf[i];
        }

        mspf /= static_cast<double>(prevMspf.size());
    }


    deltaTime /= 1000.0;

}

float Timer::DeltaTime_f() {
    return static_cast<float>(deltaTime);
}

double Timer::DeltaTime_d() {
    return deltaTime;
}

double Timer::Difference_d()
{
    return mspfThreshold - mspf;
}

float Timer::Difference_f()
{
    return static_cast<float>(mspfThreshold - mspf);
}

int Timer::Difference_i()
{
    return static_cast<int>((mspfThreshold - mspf));
}

void Timer::SetThreshold(double threshold)
{
    this->mspfThreshold = threshold;
}

void Timer::SetFrameRate(int frameRate)
{
    mspfThreshold = 1000.0 / static_cast<double>(frameRate);
}

void Timer::SetBuffer(double buffer)
{
    mspfBuffer = buffer;
}
