#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>

#include "Utility.h"

#include "Constants.h"
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

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}



class RasteriseRenderer {
public:

    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:

    std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    Device devices;

    SwapChain swapChain;
    ColourRenderPass renderPass;
    FrameBuffer frameBuffer;
    Pipeline pipeline;

    Camera camera;
    Resources resources;

    std::vector<GameObject> gameObjects;

    int gameObjectCount = 3;

    ShadowMap shadowMap;

   // std::vector<Buffer> uniformBuffers;

    VkDescriptorPool descriptorPool;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    bool framebufferResized = false;

    struct {
        std::vector<Buffer> scene;
        std::vector<Buffer> scene2;
        std::vector<Buffer> offscreen;
        std::vector<Buffer> offscreen2;
    } uniformBuffers;

    struct UBOS {
        alignas(16) glm::mat4 projection;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 depthBiasMVP;
        alignas(16) glm::vec3 lightPos;
        alignas(16) glm::vec3 camPos;
    };

    struct {
        alignas(16) glm::mat4 depthMVP;
    } uboOffscreenVS;

    struct {
        std::vector<VkDescriptorSet> offscreen;
        std::vector<VkDescriptorSet> offscreen2;
        std::vector<VkDescriptorSet> scene;
        std::vector<VkDescriptorSet> scene2;
        std::vector<VkDescriptorSet> debug;
    } descriptorSets;

    OffscreenPass offscreenPass;

    UBOS uboVSscene1;
    UBOS uboVSscene2;

    float timer = 0.0f;
    bool countUp = true;

    glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
    //float lightFOV = 45.0f;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<RasteriseRenderer*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        devices.SetupDevices(instance, surface);
        swapChain.Create(window, surface, &devices);

        renderPass.Create(&devices, &swapChain);
        pipeline.Create(&devices, &swapChain, &renderPass, offscreenPass);
        frameBuffer.Create(&devices, renderPass.vkRenderPass);
        for (size_t i = 0; i < swapChain.imageCount; i++)
        {
            std::vector<VkImageView> attachments{ swapChain.images[i].imageView, swapChain.depthImage.imageView };
            frameBuffer.createFramebuffer(attachments, swapChain.extent);
        }

        shadowMap.descriptorSetLayout = pipeline.descriptorSetLayout;
        shadowMap.Create(&devices, &swapChain);
        shadowMap.Init();

        createCamera();
        loadResources();
        createModelBuffers();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();

    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(devices.logicalDevice);
    }

    void cleanupSwapChain() {

        frameBuffer.Destroy();

        vkFreeCommandBuffers(devices.logicalDevice, devices.commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

        pipeline.Destroy();
        renderPass.Destroy();

        vkDestroyRenderPass(devices.logicalDevice, renderPass.vkRenderPass, nullptr);

        swapChain.Destroy();

        for (size_t i = 0; i < swapChain.images.size(); i++) {
            uniformBuffers.offscreen[i].Destroy();
        }

        vkDestroyDescriptorPool(devices.logicalDevice, descriptorPool, nullptr);
    }

    void cleanup() {
        cleanupSwapChain();

        for (auto& go : gameObjects) {
            go.Destroy();
        }

        vkDestroyDescriptorSetLayout(devices.logicalDevice, pipeline.descriptorSetLayout, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(devices.logicalDevice, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(devices.logicalDevice, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(devices.logicalDevice, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(devices.logicalDevice, devices.commandPool, nullptr);

        devices.Destroy();

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(devices.logicalDevice);

        cleanupSwapChain();

        swapChain.Init();
        renderPass.Init();
        //renderPass.Init(offscreenPass);
        pipeline.Init(offscreenPass);
        frameBuffer.Create(&devices, &swapChain, renderPass.vkRenderPass, offscreenPass);
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();

        imagesInFlight.resize(swapChain.images.size(), VK_NULL_HANDLE);
    }

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = Initialisers::applicationInfo("Hybrid Renderer", "No Engine");

        auto extensions = getRequiredExtensions();

        VkInstanceCreateInfo createInfo = Initialisers::instanceCreateInfo(&appInfo, static_cast<uint32_t>(extensions.size()), extensions.data());

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void loadResources() {

        resources.Init(&devices);

        resources.LoadMesh("cube");
        resources.LoadMesh("sphere");
        resources.LoadMesh("tree");
        resources.LoadMesh("plane");
        resources.LoadImage("texture");
    }

    void createModelBuffers() {


        for (size_t i = 0; i < gameObjectCount; i++)
        {
            auto& go = gameObjects.emplace_back(GameObject());
            go.mesh = resources.meshes[i != 1 ? i == 0 ? "tree" : "sphere" : "plane"].get();
            go.image = resources.images["texture"].get();
            go.Init();
            if (i == 2)
            {
                go.shadowCaster = false;
                go.shadowReceiver = false;
                go.transform.scale = glm::vec3(0.2f, 0.2f, 0.2f);
            }
        }
    }

    void createUniformBuffers() {

        for (auto& go : gameObjects) {
            go.uniformBuffers.resize(swapChain.images.size());

            for (size_t i = 0; i < swapChain.images.size(); i++) {
                VkDeviceSize bufferSize = sizeof(UBOS);
                go.uniformBuffers[i].Create(&devices, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            }
        }
        uniformBuffers.offscreen.resize(swapChain.images.size());

        for (size_t i = 0; i < swapChain.images.size(); i++) {
            VkDeviceSize bufferSize = sizeof(uboOffscreenVS);
            uniformBuffers.offscreen[i].Create(&devices, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }

    void createDescriptorPool() {
        std::vector<VkDescriptorPoolSize> poolSizes{
        Initialisers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(swapChain.images.size() * gameObjectCount)),
        Initialisers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(swapChain.images.size()* gameObjectCount))
        };

        VkDescriptorPoolCreateInfo poolInfo = Initialisers::descriptorPoolCreateInfo(static_cast<uint32_t>(poolSizes.size()), poolSizes.data(), static_cast<uint32_t>(swapChain.images.size() * 50));

        if (vkCreateDescriptorPool(devices.logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void createDescriptorSets() {

        // Image descriptor for the shadow map attachment
        //VkDescriptorImageInfo shadowMapDescriptor =
        //    Initialisers::descriptorImageInfo(
        //        offscreenPass.depth.view,
        //        offscreenPass.depthSampler,
        //        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

        std::vector<VkDescriptorSetLayout> layouts(swapChain.images.size(), pipeline.descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(descriptorPool, static_cast<uint32_t>(swapChain.images.size()), layouts.data());

        //descriptorSets.debug.resize(swapChain.images.size());
        //if (vkAllocateDescriptorSets(devices.logicalDevice, &allocInfo, descriptorSets.debug.data()) != VK_SUCCESS) {
        //    throw std::runtime_error("failed to allocate descriptor sets!");
        //}

        //for (size_t i = 0; i < swapChain.images.size(); i++) {

        //    std::vector<VkWriteDescriptorSet> descriptorWrites{
        //    Initialisers::writeImageDescriptorSet(descriptorSets.debug[i], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &shadowMap.depthTexture.descriptorInfo)
        //    };

        //    vkUpdateDescriptorSets(devices.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        //}

        descriptorSets.offscreen.resize(swapChain.images.size());
        if (vkAllocateDescriptorSets(devices.logicalDevice, &allocInfo, descriptorSets.offscreen.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < swapChain.images.size(); i++) {
            VkDescriptorBufferInfo bufferInfo = Initialisers::descriptorBufferInfo(uniformBuffers.offscreen[i].vkBuffer, sizeof(uboOffscreenVS));

            std::vector<VkWriteDescriptorSet> descriptorWrites{
            Initialisers::writeBufferDescriptorSet(descriptorSets.offscreen[i], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfo),
            Initialisers::writeBufferDescriptorSet(descriptorSets.offscreen[i], 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &gameObjects[0].uniformBuffers[i].descriptorInfo)
            };

            vkUpdateDescriptorSets(devices.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }


        descriptorSets.offscreen2.resize(swapChain.images.size());
        if (vkAllocateDescriptorSets(devices.logicalDevice, &allocInfo, descriptorSets.offscreen2.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < swapChain.images.size(); i++) {
            VkDescriptorBufferInfo bufferInfo = Initialisers::descriptorBufferInfo(uniformBuffers.offscreen[i].vkBuffer, sizeof(uboOffscreenVS));

            std::vector<VkWriteDescriptorSet> descriptorWrites{
            Initialisers::writeBufferDescriptorSet(descriptorSets.offscreen2[i], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfo),
            Initialisers::writeBufferDescriptorSet(descriptorSets.offscreen2[i], 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &gameObjects[1].uniformBuffers[i].descriptorInfo)
            };

            vkUpdateDescriptorSets(devices.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        for (auto& go : gameObjects) {
            go.descriptorSets.resize(swapChain.images.size());
            if (vkAllocateDescriptorSets(devices.logicalDevice, &allocInfo, go.descriptorSets.data()) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            for (size_t i = 0; i < swapChain.images.size(); i++) {

                std::vector<VkWriteDescriptorSet> descriptorWrites{
                Initialisers::writeBufferDescriptorSet(go.descriptorSets[i], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &go.uniformBuffers[i].descriptorInfo),
                Initialisers::writeImageDescriptorSet(go.descriptorSets[i], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &shadowMap.depthTexture.descriptorInfo),
                Initialisers::writeImageDescriptorSet(go.descriptorSets[i], 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &go.image->texture.descriptorInfo)
                };

                vkUpdateDescriptorSets(devices.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }
    }

    void createCommandBuffers() {
        commandBuffers.resize(frameBuffer.vkFrameBuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = devices.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(devices.logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) 
            throw std::runtime_error("failed to allocate command buffers!");


        VkCommandBufferBeginInfo beginInfo = Initialisers::commandBufferBeginInfo();

        std::array<VkClearValue, 2> clearValues;

        for (int32_t i = 0; i < commandBuffers.size(); ++i)
        {

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }


            /*
                First render pass: Generate shadow map by rendering the scene from light's POV
            */
            {
                clearValues[0].depthStencil = { 1.0f, 0 };

              //  VkExtent2D offscreenExtent = { shadowMap.width , shadowMap.height };

                VkRenderPassBeginInfo renderPassBeginInfo = Initialisers::renderPassBeginInfo(shadowMap.renderPass.vkRenderPass, shadowMap.frameBuffer.vkFrameBuffers[i], 
                    { shadowMap.width , shadowMap.height }, 1, &clearValues[0]);

                vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                // Set depth bias (aka "Polygon offset")
                // Required to avoid shadow mapping artifacts
                vkCmdSetDepthBias(
                    commandBuffers[i],
                    1.25f,
                    0.0f,
                    1.75f);

    
                VkViewport viewport = Initialisers::viewport(0, 0, (float)shadowMap.width, (float)shadowMap.height);
                VkRect2D scissor = Initialisers::scissor({ shadowMap.width, shadowMap.height });

                vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
                vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

                vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.vkPipeline);
                vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipelineLayout, 0, 1, &descriptorSets.offscreen[i], 0, nullptr);

                VkDeviceSize offsets[] = { 0 };

                int j = 0;
                for (auto& go : gameObjects) {
                    if (!go.shadowCaster || !go.shadowReceiver)
                        continue;

                    if(j == 0)
                        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipelineLayout, 0, 1, &descriptorSets.offscreen[i], 0, nullptr);
                    else
                        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipelineLayout, 0, 1, &descriptorSets.offscreen2[i], 0, nullptr);
                    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &go.mesh->vertexBuffer->vkBuffer, offsets);

                    vkCmdBindIndexBuffer(commandBuffers[i], go.mesh->indexBuffer->vkBuffer, 0, VK_INDEX_TYPE_UINT32);

                    vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(go.mesh->indices.size()), 1, 0, 0, 0);
                    j++;
                }

                vkCmdEndRenderPass(commandBuffers[i]);
            }

            /*
                Note: Explicit synchronization is not required between the render pass, as this is done implicit via sub pass dependencies
            */

            /*
                Second pass: Scene rendering with applied shadow map
            */

            {
                clearValues[0].color = { 0.025f, 0.025f, 0.025f, 1.0f };
                clearValues[1].depthStencil = { 1.0f, 0 };

                VkRenderPassBeginInfo renderPassBeginInfo = Initialisers::renderPassBeginInfo(renderPass.vkRenderPass, frameBuffer.vkFrameBuffers[i], swapChain.extent,
                           static_cast<uint32_t>(clearValues.size()), clearValues.data());

                vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


                VkViewport viewport = Initialisers::viewport(0, 0, (float)swapChain.extent.width, (float)swapChain.extent.height);

                VkRect2D scissor = Initialisers::scissor(swapChain.extent);
                vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

                vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

                vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vkPipeline);

                VkDeviceSize offsets[] = { 0 };
                for (auto& go : gameObjects) {
                    //if (!go.shadowCaster || !go.shadowReceiver)
                    //    continue;
                    vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &go.descriptorSets[i], 0, nullptr);
                    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &go.mesh->vertexBuffer->vkBuffer, offsets);

                    vkCmdBindIndexBuffer(commandBuffers[i], go.mesh->indexBuffer->vkBuffer, 0, VK_INDEX_TYPE_UINT32);

                    vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(go.mesh->indices.size()), 1, 0, 0, 0);
                }

                vkCmdEndRenderPass(commandBuffers[i]);
            }

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(swapChain.images.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = Initialisers::semaphoreCreateInfo();

        VkFenceCreateInfo fenceInfo = Initialisers::fenceCreateInfo();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(devices.logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(devices.logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(devices.logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void createCamera() {
        camera.lookAt = glm::vec3(0, 0, 0);
        camera.transform.position = glm::vec3(0, 0, 10);
        camera.transform.rotation.y = 180.f;
    }

    void updateUniformBuffer(uint32_t currentImage) {
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - startTime).count()  / 1000.0f;
        startTime = std::chrono::high_resolution_clock::now();

        timer += time;
        /*if (countUp)
        {
            timer += time;
            if (timer >= 1.0f)
                countUp = !countUp;
        }
        else {
            timer -= time;
            if (timer <= -0.0f)
                countUp = !countUp;
        }*/

        //lightPos.x = cos(glm::radians(timer * 360.0f)) * 4.0f;
        //lightPos.y = 5.0f + sin(glm::radians(timer * 360.0f)) * 2.0f;
        //lightPos.z = 2.5f + sin(glm::radians(timer * 360.0f)) * 5.0f;


        if (glfwGetKey(window, GLFW_KEY_W) != GLFW_RELEASE)
        {
            camera.transform.position += camera.transform.forward * 10.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_S) != GLFW_RELEASE)
        {
            camera.transform.position -= camera.transform.forward * 10.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE)
        {
            camera.transform.position += camera.transform.right * 10.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_D) != GLFW_RELEASE)
        {
            camera.transform.position -= camera.transform.right * 10.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_RELEASE)
        {
            camera.transform.position += camera.transform.up * 10.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_RELEASE)
        {
            camera.transform.position -= camera.transform.up * 10.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) != GLFW_RELEASE)
        {
            camera.transform.rotation.y -= 50.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_E) != GLFW_RELEASE)
        {
            camera.transform.rotation.y += 50.0f * time;
        }

        if (glfwGetKey(window, GLFW_KEY_KP_8) != GLFW_RELEASE)
        {
            lightPos += camera.transform.forward * 5.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_5) != GLFW_RELEASE)
        {
            lightPos -= camera.transform.forward* 5.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_4) != GLFW_RELEASE)
        {
            lightPos += camera.transform.right * 5.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_6) != GLFW_RELEASE)
        {
            lightPos -= camera.transform.right * 5.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_9) != GLFW_RELEASE)
        {
            lightPos.y += 5.0f * time;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_7) != GLFW_RELEASE)
        {
            lightPos.y -= 5.0f * time;
        }

        if (glfwGetKey(window, GLFW_KEY_P) != GLFW_RELEASE)
        {
            gameObjects[0].transform.scale += glm::vec3(1.0f) * time;
        }
        if (glfwGetKey(window, GLFW_KEY_O) != GLFW_RELEASE)
        {
            gameObjects[0].transform.scale -= glm::vec3(1.0f) * time;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            lightPos.x = 0.0f;
            lightPos.z = 0.0f;
        }
       

        camera.update(static_cast<float>(swapChain.extent.width), static_cast<float>(swapChain.extent.height));

        float zNear = 1.0f;
        float zFar = 100.0f;

        glm::vec3 direction(1, -2, -1);
        direction = glm::normalize(direction);

        glm::vec3 lightLookAt = glm::vec3(0.0f, 0.0f, 0.0f);

        float lightFOV = 45.0f;

        // Matrix from light's point of view
        glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
        //glm::mat4 depthProjectionMatrix = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, zNear, zFar);
        glm::mat4 depthViewMatrix = glm::lookAt(lightPos, lightLookAt, glm::vec3(0, 1, 0));
        glm::mat4 depthModelMatrix = glm::mat4(1.0f);

        uboOffscreenVS.depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

        uniformBuffers.offscreen[currentImage].Map(&uboOffscreenVS);

        gameObjects[1].transform.scale = glm::vec3(15, 1, 15);
        gameObjects[2].transform.position = lightPos;

        for (auto& go : gameObjects)
        {
            go.Update();

            UBOS ubos;
            ubos.projection = camera.projection;
            ubos.view = camera.view;
            ubos.model = go.model;
            ubos.lightPos = lightPos;
            ubos.depthBiasMVP = uboOffscreenVS.depthMVP;
            ubos.camPos = camera.transform.position;
            go.uniformBuffers[currentImage].Map(&ubos);
        }
    }

    void drawFrame() {
        vkWaitForFences(devices.logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(devices.logicalDevice, swapChain.vkSwapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffer(imageIndex);

        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(devices.logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

        VkSubmitInfo submitInfo = Initialisers::submitInfo(&commandBuffers[imageIndex], 1, waitSemaphores, 1, signalSemaphores, 1, waitStages);

        vkResetFences(devices.logicalDevice, 1, &inFlightFences[currentFrame]);

        if (vkQueueSubmit(devices.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkSwapchainKHR swapChains[] = { swapChain.vkSwapChain };
        VkPresentInfoKHR presentInfo = Initialisers::presentInfoKHR(signalSemaphores, 1, swapChains, 1, &imageIndex);

        result = vkQueuePresentKHR(devices.presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

int main() {
    RasteriseRenderer app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

