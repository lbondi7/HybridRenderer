#include "SwapChain.h"
#include "Utility.h"

#include "Initilizers.h"

#include <stdexcept>

SwapChain::~SwapChain()
{
    deviceContext = nullptr;
    window = nullptr;
    windowWidth = nullptr;
    windowHeight = nullptr;
    surface = VK_NULL_HANDLE;
}

void SwapChain::Create(VkSurfaceKHR _surface, DeviceContext* _devices, int* _windowWidth, int* _windowHeight)
{
    deviceContext = _devices;
    windowWidth = _windowWidth;
    windowHeight = _windowHeight;
    surface = _surface;

    Init();
}

void SwapChain::Init()
{
    createSwapChain(window, surface);
    createImageViews();
    createDepthResources();
}

void SwapChain::Destroy()
{
    depthImage.Destroy();
    for (auto& image : images)
    {
        image.DestroyImageViews();
    }

    vkDestroySwapchainKHR(deviceContext->logicalDevice, vkSwapChain, nullptr);

    for (auto& image : _images)
    {
        image = VK_NULL_HANDLE;
    }

}

VkResult SwapChain::AquireNextImage(VkSemaphore imageAvailableSemaphore, uint32_t& imageIndex)
{
    auto result = vkAcquireNextImageKHR(deviceContext->logicalDevice, vkSwapChain,
        UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
   
    return result;
}


VkResult SwapChain::Present(VkSemaphore presentSemaphore, uint32_t& imageIndex)
{
    VkPresentInfoKHR presentInfo = Initialisers::presentInfoKHR(&presentSemaphore, 1, &vkSwapChain, 1, &imageIndex);
    auto result = vkQueuePresentKHR(deviceContext->presentQueue, &presentInfo);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    return result;
}

void SwapChain::createSwapChain(GLFWwindow* window, VkSurfaceKHR surface) {

    SwapChainSupportDetails swapChainSupport = Utility::querySwapChainSupport(surface, deviceContext->physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = Utility::chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = Utility::chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D _extent = Utility::chooseSwapExtent(*windowWidth, *windowHeight, swapChainSupport.capabilities);

    imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = Initialisers::swapchainCreateInfoKHR(surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
        _extent, swapChainSupport.capabilities.currentTransform, presentMode);

    QueueFamilyIndices indices = Utility::findQueueFamilies(deviceContext->physicalDevice, surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceContext->physicalDevice, surface, &surfaceCapabilities);

    // Enable transfer source on swap chain images if supported
    if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Enable transfer destination on swap chain images if supported
    if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }


    if (vkCreateSwapchainKHR(deviceContext->logicalDevice, &createInfo, nullptr, &vkSwapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(deviceContext->logicalDevice, vkSwapChain, &imageCount, nullptr);

    _images.resize(imageCount);
    images.resize(imageCount);


    vkGetSwapchainImagesKHR(deviceContext->logicalDevice, vkSwapChain, &imageCount, _images.data());
    for (int i = 0; i < imageCount; i++) {
        images[i].image = _images[i];
        images[i].format = surfaceFormat.format;
    }
    imageFormat = surfaceFormat.format;
    extent = _extent;
    deviceContext->imageCount = imageCount;
    outdated = false;
}


void SwapChain::createImageViews() {

    for (uint32_t i = 0; i < images.size(); i++) {
        images[i].devices = deviceContext;
        images[i].CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    }
}


void SwapChain::createDepthResources() {
    VkFormat depthFormat = Utility::findDepthFormat(deviceContext->physicalDevice);

    depthImage.Create(deviceContext, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    depthImage.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
}