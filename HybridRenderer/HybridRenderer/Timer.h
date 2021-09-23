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

	void SetThreshold(double threshold);

	void SetFrameRate(int frameRate);

	void SetBuffer(double buffer);

private:
	std::chrono::high_resolution_clock::time_point prevTime;
	std::chrono::high_resolution_clock::time_point startTime;

	double deltaTime;
	double elapsed;
	double mspf;
	double mspfThreshold;
	double mspfBuffer;
	bool average = false;

	size_t mspfCount;
	
	std::deque<double> prevMspf;
};

