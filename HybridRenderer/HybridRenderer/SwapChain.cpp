#include "SwapChain.h"
#include "Utility.h"

#include "Initilizers.h"

#include <stdexcept>

SwapChain::~SwapChain()
{
    devices = nullptr;
    window = nullptr;
    surface = nullptr;
}

void SwapChain::Create(GLFWwindow* _window, VkSurfaceKHR _surface, Device* _devices)
{
    devices = _devices;
    window = _window;
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

    vkDestroySwapchainKHR(devices->logicalDevice, vkSwapChain, nullptr);

    for (auto& image : _images)
    {
        image = VK_NULL_HANDLE;
    }

}

void SwapChain::createSwapChain(GLFWwindow* window, VkSurfaceKHR surface) {

    SwapChainSupportDetails swapChainSupport = Utility::querySwapChainSupport(surface, devices->physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = Utility::chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = Utility::chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D _extent = Utility::chooseSwapExtent(window, swapChainSupport.capabilities);

    imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = Initialisers::swapchainCreateInfoKHR(surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, 
        _extent, swapChainSupport.capabilities.currentTransform, presentMode);

    QueueFamilyIndices indices = Utility::findQueueFamilies(devices->physicalDevice, surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }


    if (vkCreateSwapchainKHR(devices->logicalDevice, &createInfo, nullptr, &vkSwapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(devices->logicalDevice, vkSwapChain, &imageCount, nullptr);

    _images.resize(imageCount);
    images.resize(imageCount);


    vkGetSwapchainImagesKHR(devices->logicalDevice, vkSwapChain, &imageCount, _images.data());
    for (int i = 0; i < imageCount; i++) {
        images[i].image = _images[i];
        images[i].format = surfaceFormat.format;
    }
    imageFormat = surfaceFormat.format;
    extent = _extent;
}


void SwapChain::createImageViews() {

    for (uint32_t i = 0; i < images.size(); i++) {
        images[i].devices = devices;
        images[i].createImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    }
}


void SwapChain::createDepthResources() {
    VkFormat depthFormat = Utility::findDepthFormat(devices->physicalDevice);

    depthImage.Create(devices, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    depthImage.createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
    //createImage(extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    //depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}