#pragma once
#define GLFW_INCLUDE_NONE
#include <glfw3.h>
#include "quad.h"

class Application
{
public:
	GLFWwindow* window = nullptr;
	Quad* original = nullptr;
	Quad* altered = nullptr;

	Application();
	~Application();
	void Run();
	void ProcessInput();
	void Quit();

private:
	bool quit = false;
};
