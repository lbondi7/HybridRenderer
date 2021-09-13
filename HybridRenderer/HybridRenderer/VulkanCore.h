#pragma once

#include "Constants.h"

#include "Window.h" 
#include "Device.h"

class VulkanCore
{
public:

    VulkanCore();
    ~VulkanCore();


    void initialise(GLFWwindow* glfwWindow);
    void Deinitialise();

    void createInstance();

    VkInstance instance;
    VkSurfaceKHR surface;


    std::unique_ptr<DeviceContext> deviceContext;

private:


    std::vector<const char*> getRequiredExtensions();
    void createSurface(GLFWwindow* glfwWindow);
};

