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
#include "GameObject.h"
#include "Resources.h"
#include "ShadowMap.h"
#include "Window.h"
#include "DescriptorSetManager.h"
#include "ImGUI_.h"
#include "VulkanCore.h"
#include "Descriptor.h"

class RasterRenderer {
public:

    RasterRenderer() = default;
    RasterRenderer(Window* window, VulkanCore* core);


    void run();

    void initialise(Resources* _resources, DescriptorSetManager* _descriptorSetManager);

    void prepare();

    void render(Camera* camera, std::vector<GameObject>& gameObjects, Descriptor& lightDescs);

    DeviceContext* deviceContext;

    SwapChain swapChain;
    RenderPass renderPass;
    FrameBuffer frameBuffer;
    RenderPass penultimateRenderPass;
    FrameBuffer penultimateFrameBuffer;
    Pipeline pipeline;

    //Camera camera;
    Resources* resources;

    //std::vector<GameObject> gameObjects;

    //int gameObjectCount = 2;

    ShadowMap shadowMap;

    DescriptorSetManager* descriptorSetManager;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    bool framebufferResized = false;
    bool rebuildSwapChain = false;
    bool ortho = false;

    ImGUI imgui;

    bool lMouse = false;
    bool rMouse = false;

    glm::vec2 mousePos;

    //struct CameraUBO {
    //    alignas(16) glm::mat4 projection;
    //    alignas(16) glm::mat4 view;
    //    alignas(16) glm::vec3 camPos;
    //}cameraUBO;

    //struct LightUBO {
    //    alignas(16) glm::mat4 depthBiasMVP;
    //    alignas(16) glm::vec3 lightPos;
    //}lightUBO;

    bool commandBuffersReady = false;


    int counted = 100;

    const char* imagedata;

    bool printImage = false;

    float timer = 0.0f;
    bool countUp = true;

    glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);
    glm::vec3 lightPos = glm::vec3(-19.0f, 20.0f, -30.0f);

    float lightFOV = 90.0f;

    bool conservativeRendering = false;
    bool prevConservativeRendering = true;

    VkDescriptorSet descTest;

    uint32_t imageIndex;


    void cleanupSwapChain();

    void cleanup();

    void recreateSwapChain();


    void AllocateCommandBuffers();

    void buildCommandBuffers(Camera* camera, std::vector<GameObject>& gameObjects, Descriptor& lightDescs);

    void rebuildCommandBuffer(uint32_t i, Camera* camera, std::vector<GameObject>& gameObjects, Descriptor& lightDescs);

    void createSyncObjects();

};