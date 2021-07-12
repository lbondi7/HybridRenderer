#pragma once

#include <chrono>

class Timer
{
public:
	Timer();
	~Timer();

	void update();


	std::chrono::high_resolution_clock::time_point prevTime;
	std::chrono::high_resolution_clock::time_point startTime;

	float dt;
	float elapsed;

};

