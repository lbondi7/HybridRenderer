#include "Timer.h"
#include "DebugLogger.h"

Timer::Timer()
{
    prevTime = prevSecond = std::chrono::high_resolution_clock::now();
    SetFrameRate(2000);
    average = true;
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

    frameCount++;
    if (std::chrono::duration<double, std::chrono::seconds::period>(prevTime - prevSecond).count() >= 1.0) {
        fps = frameCount;
        Log(frameCount);
        prevSecond = std::chrono::high_resolution_clock::now();
        frameCount = 0;
    }

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
    return (mspfThreshold - mspf) * 1000.0;
}

float Timer::Difference_f()
{
    return static_cast<float>(mspfThreshold * 1000.0 - mspf * 1000.0);
}

int Timer::Difference_i()
{
    return static_cast<int>((mspfThreshold - mspf) * 1000.0);
}

double Timer::MSPF_d()
{
    return mspf;
}

float Timer::MSPF_f()
{
    return static_cast<float>(mspf);
}

double Timer::Threshold_d()
{
    return mspfThreshold;
}

float Timer::Threshold_f()
{
    return  static_cast<float>(mspfThreshold);
}

void Timer::SetThreshold(double threshold)
{
    this->mspfThreshold = threshold;
}

void Timer::SetFrameRate(int frameRate)
{
    mspfThreshold = 1000.0 / static_cast<double>(frameRate);
}

void Timer::SetFramerateBufferLimits(int lowerFramerate, int upperFramerate)
{
    bufferLowerLimit = 1000.0 / static_cast<double>(lowerFramerate);
    bufferUpperLimit = 1000.0 / static_cast<double>(upperFramerate);
}

int Timer::FPS()
{
    return fps;
}

float Timer::Threshold_f(int framerate)
{
    return 1000.0f / static_cast<float>(framerate);
}

double Timer::Threshold_d(int framerate)
{
    return 1000.0 / static_cast<double>(framerate);
}
