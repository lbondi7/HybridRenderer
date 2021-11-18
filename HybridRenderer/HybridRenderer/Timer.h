#pragma once

#include <chrono>
#include <deque>

class Timer
{
public:
	Timer();
	~Timer();

	void Update();

	float DeltaTime_f();

	double DeltaTime_d();

	double Difference_d();

	float Difference_f();

	int Difference_i();

	double MSPF_d();

	float MSPF_f();

	double Threshold_d();

	float Threshold_f();

	void SetThreshold(double threshold);

	void SetFrameRate(int frameRate);

	void SetFramerateBufferLimits(int lowerFramerate, int upperFramerate);

	int FPS();

	float Threshold_f(int framerate);

	double Threshold_d(int framerate);

private:
	std::chrono::high_resolution_clock::time_point prevTime;
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point prevSecond;

	double deltaTime;
	double elapsed;
	double mspf;
	double mspfThreshold;
	double mspfBuffer;
	bool average = false;
	double bufferLowerLimit;
	double bufferUpperLimit;

	size_t mspfCount;
	
	std::deque<double> prevMspf;
	int fps;
	int frameCount = 0;
};

