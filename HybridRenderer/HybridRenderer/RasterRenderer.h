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
#include "Image.h"
#include "Texture.h"
#include "Camera.h"
#include "GameObject.h"
#include "Resources.h"
#include "ShadowMap.h"
#include "Window.h"
#include "DescriptorSetManager.h"
#include "ImGUI_.h"

class RasterRenderer {
public:

    void run();

private:

    std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    Window window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    DeviceContext devices;

    SwapChain swapChain;
    RenderPass renderPass;
    FrameBuffer frameBuffer;
    RenderPass penultimateRenderPass;
    FrameBuffer penultimateFrameBuffer;
    Pipeline pipeline;

    Camera camera;
    Resources resources;

    std::vector<GameObject> gameObjects;

    int gameObjectCount = 2;

    ShadowMap shadowMap;

    DescriptorSetManager descriptorSetManager;

    VkDescriptorPool descriptorPool;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    bool framebufferResized = false;
    bool ortho = false;

    ImGUI imgui;

    bool lMouse = false;
    bool rMouse = false;

    glm::vec2 mousePos;

    struct CameraUBO {
        alignas(16) glm::mat4 projection;
        alignas(16) glm::mat4 view;
        alignas(16) glm::vec3 camPos;
    }cameraUBO;

    struct ModelUBO {
        alignas(16) glm::mat4 model;
    };

    struct LightUBO {
        alignas(16) glm::mat4 depthBiasMVP;
        alignas(16) glm::vec3 lightPos;
    }lightUBO;


    std::vector<VkDescriptorSet> cameraDescSets;
    std::vector<VkDescriptorSet> lightDescSets;

    std::vector<Buffer> lightBuffers;
    std::vector<Buffer> cameraBuffers;


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

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static void mouseCallback(GLFWwindow* window, int button, int action, int mods);

    static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

    static void cursorCallback(GLFWwindow* window, double xOffset, double yOffset);

    void initWindow();

    void initVulkan();

    void mainLoop();

    void cleanupSwapChain();

    void cleanup();

    void recreateSwapChain();

    void createInstance();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void setupDebugMessenger();

    void createSurface();

    void loadResources();

    void createModelBuffers();

    void createUniformBuffers();

    void createDescriptorSets();

    void AllocateCommandBuffers();

    void buildCommandBuffers();
    void buildCommandBuffer(uint32_t i);
    void buildCommandBuffersImGui();

    void createSyncObjects();

    void createCamera();

    void updateUniformBuffer(uint32_t currentImage);

    void drawFrame();

    std::vector<const char*> getRequiredExtensions();

    bool checkValidationLayerSupport();
};