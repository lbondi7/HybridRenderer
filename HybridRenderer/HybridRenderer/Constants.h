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

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/";
const std::string TEXTURE_PATH = "textures/texture.jpg";

const int MAX_FRAMES_IN_FLIGHT = 2;
 
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME
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

    using BindingType = std::pair<uint32_t, VkDescriptorType>;
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
