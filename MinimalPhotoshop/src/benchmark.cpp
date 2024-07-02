#include "benchmark.h"
#include <iostream>
#define GLFW_INCLUDE_NONE
#include <glfw3.h>

void Benchmark::DoBenchmark()
{
	lastFrame = now;
	++framecount;

	if (now - previousTime >= 1.0) {
		framerate = framecount;
		framecount = 0;
		previousTime = now;
	}

	now = glfwGetTime();
	deltaTime = now - lastFrame;
}

void Benchmark::MeasureBegin()
{
	t1 = std::chrono::high_resolution_clock::now();
}

void Benchmark::MeasureEnd()
{
	t2 = std::chrono::high_resolution_clock::now();
	duration = duration_cast<std::chrono::milliseconds>(t2 - t1);
}
