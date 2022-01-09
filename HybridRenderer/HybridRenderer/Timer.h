#pragma once

#include <chrono>
#include <deque>

#include "ImGUIWidgets.h"

class Timer
{
public:
	Timer();
	~Timer();

	void Update();

	void Render();

	float DeltaTime_f();

	double DeltaTime_d();

	double Difference_d();

	float Difference_f();

	int Difference_i();

	double MSPF_d();

	float MSPF_f();

	float SPF_f();

	double PrevMSPFAverage_d();

	float PrevMSPFAverage_f();

	double Threshold_d();

	float Threshold_f();

	void SetThreshold(double threshold);

	void SetFrameRate(int frameRate);

	void SetFramerateBufferLimits(int lowerFramerate, int upperFramerate);

	int FPS();

	float Threshold_f(int framerate);

	double Threshold_d(int framerate);

	float GetDifference_f(int framerate);

	float GetDifferenceWithBuffer_f(int framerate, int buffer = 5);

	ImGUIWidget widget;

	std::vector<double> outputMSPF;
	std::vector<double> outputAverageMSPF;
	std::vector<double> prevMspf;
	float lerpAmount = 0.4f;
private:
	std::chrono::high_resolution_clock::time_point prevTime;
	std::chrono::high_resolution_clock::time_point prevFixedDeltaTime;
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point prevSecond;

	double deltaTime;
	double elapsed;
	double mspf;
	double mspfThreshold;
	double prevMSPFAverage;
	double mspfBuffer;
	bool average = false;
	double bufferLowerLimit;
	double bufferUpperLimit;

	size_t mspfCount;
	size_t prevMSPFCount;
	
	int fps;
	int frameCount = 0;
	int deltaTimeCount = 0;

};

