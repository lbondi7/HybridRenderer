#pragma once

#include "Constants.h"
#include "Device.h"
#include "Texture.h"

class SwapChain
{
public:

    SwapChain() = default;
    ~SwapChain();

    VkSwapchainKHR vkSwapChain;
    std::vector<Texture> images;
    uint32_t imageCount;
    VkFormat imageFormat;
    VkExtent2D extent;

    Texture depthImage;

    void Create(VkSurfaceKHR surface, DeviceContext* _devices, int* windowWidth, int* windowHeight);

    void Init();

    void Destroy();

private:
    std::vector<VkImage> _images;

    DeviceContext* devices = nullptr;
    GLFWwindow* window = nullptr;
    VkSurfaceKHR surface;

    void createSwapChain(GLFWwindow* window, VkSurfaceKHR surface);
    void createImageViews();
    void createDepthResources();

    int* windowWidth;
    int* windowHeight;
};

