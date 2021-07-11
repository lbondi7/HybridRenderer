#include "VulkanCore.h"
#include "ValidationLayers.h"

#include "Initilizers.h"

VulkanCore::VulkanCore()
{


}

VulkanCore::~VulkanCore()
{


}

void VulkanCore::initialise(GLFWwindow* glfwWindow)
{
    createInstance();
    if (ValidationLayers::enabled()) {
        ValidationLayers::setupDebugMessenger(instance);
    }
    createSurface(glfwWindow);

    deviceContext = std::make_unique<DeviceContext>();
    deviceContext->SetupDevices(instance, surface);
}

void VulkanCore::deinitialise()
{

    vkDestroyCommandPool(deviceContext->logicalDevice, deviceContext->commandPool, nullptr);

    deviceContext->Destroy();

    if (ValidationLayers::enabled()) {
        ValidationLayers::destroyDebugMessenger(instance);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}


void VulkanCore::createInstance() {


    if (ValidationLayers::hasSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = Initialisers::applicationInfo("Hybrid Renderer", "No Engine");

    auto extensions = getRequiredExtensions();

    VkInstanceCreateInfo createInfo = Initialisers::instanceCreateInfo(&appInfo, static_cast<uint32_t>(extensions.size()), extensions.data());



    if (ValidationLayers::enabled())
    {
        ValidationLayers::addValidationLayers(&createInfo);
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }


    //VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    //if (enableValidationLayers) {
    //    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    //    createInfo.ppEnabledLayerNames = validationLayers.data();

    //    populateDebugMessengerCreateInfo(debugCreateInfo);
    //    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    //}
    //else {
    //    createInfo.enabledLayerCount = 0;

    //    createInfo.pNext = nullptr;
    //}

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

std::vector<const char*> VulkanCore::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);


    if(ValidationLayers::enabled())
        extensions.push_back(ValidationLayers::extentionNames());

    //if (enableValidationLayers) {
    //    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    //}

    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    return extensions;
}

void VulkanCore::createSurface(GLFWwindow* glfwWindow) {
    if (glfwCreateWindowSurface(instance, glfwWindow, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}