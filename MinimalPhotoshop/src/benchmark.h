#pragma once
#include <chrono>

class Benchmark
{
public:
	inline static float deltaTime = 0.0f;
	inline static int framerate = 0;
	inline static std::chrono::milliseconds duration = duration_cast<std::chrono::milliseconds>
		(std::chrono::high_resolution_clock::now() - std::chrono::high_resolution_clock::now());

	static void DoBenchmark();
	static void MeasureBegin();
	static void MeasureEnd();

private:
	inline static std::chrono::steady_clock::time_point t1, t2;
	inline static int framecount = 0;
	inline static float lastFrame = 0.0f;
	inline static float now = 0.0f;
	inline static float previousTime = 0.0f;
};
