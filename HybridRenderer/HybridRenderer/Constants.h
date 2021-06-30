#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/hash.hpp>

#include <string>
#include <optional>
#include <vector>
#include <iostream>
#include <fstream>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/";
const std::string TEXTURE_PATH = "textures/texture.jpg";

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_LUNARG_monitor"
};
 
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

// Framebuffer for offscreen rendering
//struct FrameBufferAttachment {
//    VkImage image;
//    VkDeviceMemory mem;
//    VkImageView view;
//};
//
//struct OffscreenPass {
//    int32_t width = 2048, height = 2048;
//    std::vector<VkFramebuffer> frameBuffers;
//    FrameBufferAttachment depth;
//    VkRenderPass renderPass;
//    VkSampler depthSampler;
//    VkDescriptorImageInfo descriptor;
//};

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

static std::vector<char> readNormalFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    std::vector<char> buffer;

    char element;
    while (file >> element)
    {
        buffer.push_back(element);
    }

    //file.seekg(0);
    //file.read(buffer.data(), fileSize);

    //file.close();

    return buffer;
}


enum VertexAttributes {
    POSITION,
    UV_COORD,
    COLOUR,
    NORMAL
};


struct DescriptorSetRequest {

    DescriptorSetRequest& operator = (const DescriptorSetRequest& other)
    {
        this->requests = other.requests;
        return *this;
    }

    std::vector<std::pair<uint32_t, VkDescriptorType>> requests;
};
