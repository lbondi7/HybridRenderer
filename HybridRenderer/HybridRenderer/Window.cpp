#include "Window.h"

Window::~Window()
{
}

void Window::init(void* pointer)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(glfwWindow, pointer);
}

void Window::destroy()
{

    glfwDestroyWindow(glfwWindow);

    glfwTerminate();
}

void Window::setupKeyCallback(void* keyCallback)
{
    glfwSetKeyCallback(glfwWindow, (GLFWkeyfun)keyCallback);
}

void Window::setupFramebufferResize(void* resizeCallback)
{
    glfwSetFramebufferSizeCallback(glfwWindow, (GLFWframebuffersizefun)resizeCallback);
}

void Window::setupMouseCallback(void* mouseCallback)
{
    glfwSetMouseButtonCallback(glfwWindow, (GLFWmousebuttonfun)mouseCallback);
}

void Window::setupScrollCallback(void*scrollCallback)
{
    glfwSetScrollCallback(glfwWindow, (GLFWscrollfun)scrollCallback);
}

void Window::setupCursorCallback(void* cursorCallback)
{
    glfwSetCursorPosCallback(glfwWindow, (GLFWcursorposfun)cursorCallback);
}

void Window::resize()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(glfwWindow, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(glfwWindow, &width, &height);
        glfwWaitEvents();
    }
}

bool Window::isActive()
{
    glfwPollEvents();
    return active && static_cast<bool>(!glfwWindowShouldClose(glfwWindow));
}
