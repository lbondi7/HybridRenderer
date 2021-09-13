#pragma once

#include <vulkan/vulkan.h>
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

                                                                                        
#define DEFAULT_FENCE_TIMEOUT 100000000000

//const std::string errorString(VkResult errorCode)
//{
//
//    switch (errorCode)
//    {
//#define STR(r) case VK_ ##r: return #r
//        STR(NOT_READY);
//        STR(TIMEOUT);
//        STR(EVENT_SET);
//        STR(EVENT_RESET);
//        STR(INCOMPLETE);
//        STR(ERROR_OUT_OF_HOST_MEMORY);
//        STR(ERROR_OUT_OF_DEVICE_MEMORY);
//        STR(ERROR_INITIALIZATION_FAILED);
//        STR(ERROR_DEVICE_LOST);
//        STR(ERROR_MEMORY_MAP_FAILED);
//        STR(ERROR_LAYER_NOT_PRESENT);
//        STR(ERROR_EXTENSION_NOT_PRESENT);
//        STR(ERROR_FEATURE_NOT_PRESENT);
//        STR(ERROR_INCOMPATIBLE_DRIVER);
//        STR(ERROR_TOO_MANY_OBJECTS);
//        STR(ERROR_FORMAT_NOT_SUPPORTED);
//        STR(ERROR_SURFACE_LOST_KHR);
//        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
//        STR(SUBOPTIMAL_KHR);
//        STR(ERROR_OUT_OF_DATE_KHR);
//        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
//        STR(ERROR_VALIDATION_FAILED_EXT);
//        STR(ERROR_INVALID_SHADER_NV);
//#undef STR
//    default:
//        return "UNKNOWN_ERROR";
//    }
//
//}
//
//const void VkCheckShit(VkResult res) {
//    if (res != VK_SUCCESS)																				
//    {
//        std::cout << "Fatal : VkResult is \"" << errorString(res);
//    }
//}


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/";
const std::string TEXTURE_PATH = "textures/texture.jpg";

const int MAX_FRAMES_IN_FLIGHT = 2;
 
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME,
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
    VK_KHR_RAY_QUERY_EXTENSION_NAME
};

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

    return buffer;
}

enum VertexAttributes {
    POSITION,
    UV_COORD,
    V_COLOUR,
    NORMAL
};

struct DescriptorSetRequest {

    using BindingType = std::tuple<uint32_t, VkDescriptorType, VkShaderStageFlagBits>;
    std::vector<BindingType> ids;
    std::vector<void*> data;
};

struct PushConstBlock {
    glm::vec2 scale;
    glm::vec2 translate;
};


enum AttachmentType : int {
    COLOUR = 0,
    DEPTH = 1,
};

struct AttactmentInfo {
    AttachmentType type;
    VkFormat format;
    VkAttachmentLoadOp loadOp;
    VkImageLayout finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkImageLayout referenceLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct RenderPassInfo {

    std::vector<AttactmentInfo> attachments;
    std::vector<VkSubpassDependency> dependencies;
};
