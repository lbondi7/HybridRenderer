#include "Timer.h"
#include "DebugLogger.h"
#include "ImGUI_.h"
#include "SGSmooth.hpp"


Timer::Timer()
{
    prevTime = prevFixedDeltaTime = prevSecond = std::chrono::high_resolution_clock::now();
    SetFrameRate(2000);
    average = true;
    mspfCount = 30;
    prevMSPFCount = 30;
    prevMspf.reserve(mspfCount);
    widget.enabled = true;
    outputAverageMSPF.reserve(1000000);
    //outputMSPF.reserve(1000000);
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
        outputAverageMSPF.emplace_back(mspf);
        //Log(frameCount);
        prevSecond = std::chrono::high_resolution_clock::now();
        frameCount = 0;
    }

    //time = std::chrono::high_resolution_clock::now();

    if (auto diff = std::chrono::duration<double, std::chrono::milliseconds::period>(time - prevFixedDeltaTime).count() >= (1000/20.0)) {
       // outputAverageMSPF.emplace_back(mspf);
        prevFixedDeltaTime = std::chrono::high_resolution_clock::now();
    }

    elapsed = std::chrono::duration<double, std::chrono::milliseconds::period>(time - startTime).count() / 1000.0;

    prevMspf.emplace_back(deltaTime);
    if (prevMspf.size() > mspfCount)
        prevMspf.erase(prevMspf.begin(), prevMspf.end() - mspfCount);

    auto smoothedBuffer = sg_smooth(prevMspf, 9, 3);

    mspf = deltaTime;
    //outputMSPF.emplace_back(mspf);
    if (average) {
        mspf = 0.0;

        for (size_t i = 0; i < smoothedBuffer.size(); i++)
        {
            mspf += smoothedBuffer[i];
        }

        mspf /= static_cast<double>(mspfCount);
    }

    deltaTime /= 1000.0;
}

void Timer::Render()
{
    if (ImGUI::enabled && widget.enabled)
    {
        if (widget.NewWindow("Timer")) {
            widget.Slider("Number of Frames for Average", &mspfCount, 1, 100);
            widget.CheckBox("Use Average", &average);
        }
        widget.EndWindow();
    }
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

float Timer::SPF_f()
{
    return static_cast<float>(mspf / 1000.0);
}

double Timer::PrevMSPFAverage_d()
{
    return prevMSPFAverage;
}

float Timer::PrevMSPFAverage_f()
{
    return static_cast<float>(prevMSPFAverage);
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

float Timer::GetDifference_f(int framerate)
{
    return static_cast<float>(mspf - 1000.0 / static_cast<double>(framerate));
}

float Timer::GetDifferenceWithBuffer_f(int framerate, int buffer)
{
    if (mspf > Threshold_d(framerate + buffer) && mspf < Threshold_d(framerate - buffer))
        return 0.0f;

    auto t = 1000.0f / static_cast<float>(framerate);
    return t - static_cast<float>(mspf);
}
