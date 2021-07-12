#pragma once
#include "Constants.h"


class Window
{
public:
	Window() = default;
	~Window();

	void init(void* pointer);

	void destroy();

	void setupKeyCallback(void* keyCallback);

	void setupFramebufferResize(void* resizeCallback);

	void setupMouseCallback(void* mouseCallback);

	void setupScrollCallback(void* scrollCallback);

	void setupCursorCallback(void* cursorCallback);

	void resize();

	bool isActive();

	GLFWwindow* glfwWindow = nullptr;

	int width;
	int height;

	bool active = true;
	bool framebufferResized = false;
};

