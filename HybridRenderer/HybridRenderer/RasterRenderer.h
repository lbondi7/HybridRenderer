#pragma once
#include <chrono>

#include "Constants.h"
#include "Utility.h"
#include "Device.h"
#include "SwapChain.h"
#include "ColourRenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "Texture.h"
#include "Camera.h"
#include "Scene.h"
#include "Resources.h"
#include "ShadowMap.h"
#include "Window.h"
#include "DescriptorSetManager.h"
#include "ImGUI_.h"
#include "VulkanCore.h"
#include "Descriptor.h"
#include "ImGUIWidgets.h"
#include "AccelerationStructure.h"

class RasterRenderer {
public:

    RasterRenderer() = default;
    RasterRenderer(Window* window, VulkanCore* core, SwapChain* swapChain);

    void Initialise(Window* window, VulkanCore* core, SwapChain* swapChain, Resources* _resources);

    void Prepare();

    void GetCommandBuffer(uint32_t imageIndex, std::vector<VkCommandBuffer>& submitCommandBuffers, Camera* camera, Scene* scene);

    void GetImGuiCommandBuffer(uint32_t imageIndex, std::vector<VkCommandBuffer>& submitCommandBuffers, VkExtent2D extent);

    void Deinitialise(bool total = false);

    void Reinitialise();

    void AllocateCommandBuffers();

    void buildCommandBuffers(Camera* camera, Scene* scene);

    void rebuildCommandBuffer(uint32_t i, Camera* camera, Scene* scene);

    DeviceContext* deviceContext;

    SwapChain* swapChain;
    RenderPass renderPass;
    FrameBuffer frameBuffer;
    RenderPass penultimateRenderPass;
    FrameBuffer penultimateFrameBuffer;
    Pipeline pipeline;

    Resources* resources;

    ShadowMap shadowMap;
    TextureSampler storageImage;
    uint32_t storageImageSize = 5;

    Descriptor storageImageDescriptor;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
    uint32_t imageIndex;

    bool framebufferResized = false;
    bool rebuild = false;
    bool ortho = false;

    ImGUI imgui;

    bool lMouse = false;
    bool rMouse = false;

    glm::vec2 mousePos;

    bool commandBuffersReady = false;

    int counted = 100;

    const char* imagedata;

    bool printImage = false;

    float timer = 0.0f;
    bool countUp = true;

    ImGUIWidget widget;

    glm::vec3 depthBias = glm::vec3(0.0, 0.0, -0.075);

};