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
    bool outdated = false;

    void Create(VkSurfaceKHR surface, DeviceContext* _devices, int* windowWidth, int* windowHeight);

    void Init();

    void Destroy();

    VkResult AquireNextImage(VkSemaphore imageAvailableSemaphore, uint32_t& imageIndex);

    VkResult Present(VkSemaphore presentSemaphore, uint32_t& imageIndex);
private:

    std::vector<VkImage> _images;

    DeviceContext* deviceContext = nullptr;
    GLFWwindow* window = nullptr;
    VkSurfaceKHR surface;


    void createSwapChain(GLFWwindow* window, VkSurfaceKHR surface);
    void createImageViews();
    void createDepthResources();

    int* windowWidth;
    int* windowHeight;
};

