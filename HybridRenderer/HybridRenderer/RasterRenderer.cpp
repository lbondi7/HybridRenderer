#include "RasterRenderer.h"

#include "DebugLogger.h"


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

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

void RasterRenderer::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void RasterRenderer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto app = reinterpret_cast<RasterRenderer*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_SPACE)
        {
            Log(app->lightPos, "Light Pos");
            //app->ortho = !app->ortho;
        }

        if (key == GLFW_KEY_F2)
        {
            //Log(app->lightPos, "Light Pos");
            //app->ortho = !app->ortho;

            app->imgui.enabled = !app->imgui.enabled;
            vkQueueWaitIdle(app->devices.presentQueue);
            app->buildCommandBuffers();
        }

        if (key == GLFW_KEY_ESCAPE)
            app->window.active = false;
    }
}

void RasterRenderer::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<RasterRenderer*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void RasterRenderer::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<RasterRenderer*>(glfwGetWindowUserPointer(window));

    app->imgui.leftMouse = button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_RELEASE;
    app->imgui.rightMouse = button == GLFW_MOUSE_BUTTON_RIGHT && action != GLFW_RELEASE;
}

void RasterRenderer::scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<RasterRenderer*>(glfwGetWindowUserPointer(window));


    app->imgui.mouseWheel = static_cast<float>(yOffset) * 0.5f;
}

void RasterRenderer::cursorCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<RasterRenderer*>(glfwGetWindowUserPointer(window));

    app->imgui.mousePos.x = xOffset;
    app->imgui.mousePos.y = yOffset;


}

void RasterRenderer::initWindow() {

    window.init(this);
    window.setupFramebufferResize(RasterRenderer::framebufferResizeCallback);
    window.setupKeyCallback(RasterRenderer::keyCallback);
    window.setupMouseCallback(RasterRenderer::mouseCallback);
    window.setupScrollCallback(RasterRenderer::scrollCallback);
    window.setupCursorCallback(RasterRenderer::cursorCallback);

}

void RasterRenderer::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    devices.SetupDevices(instance, surface);


    loadResources();


    swapChain.Create(window.glfwWindow, surface, &devices);

    renderPass.Create(&devices, &swapChain);

    descriptorSetManager.init(&devices, swapChain.imageCount);


    PipelineInfo pipelineInfo{};

    pipelineInfo.shaders = { resources.vertexShaders["scene"].get(), resources.fragmentShaders["scene"].get() };
    pipelineInfo.vertexInputAttributes = Vertex::getAttributeDescriptions({ VertexAttributes::POSITION, VertexAttributes::UV_COORD, VertexAttributes::COLOUR, VertexAttributes::NORMAL });
    pipelineInfo.vertexInputBindings = { Vertex::getBindingDescription() };
    pipelineInfo.specializationInfo = true;
    pipelineInfo.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    pipeline.Create(&devices, &renderPass, &descriptorSetManager, pipelineInfo);
    frameBuffer.Create(&devices, renderPass.vkRenderPass);
    for (size_t i = 0; i < swapChain.imageCount; i++)
    {
        std::vector<VkImageView> attachments{ swapChain.images[i].imageView, swapChain.depthImage.imageView };
        frameBuffer.createFramebuffer(attachments, swapChain.extent);
    }


    shadowMap.Create(&devices, &swapChain);
    pipelineInfo.shaders = { resources.vertexShaders["offscreen"].get() };
    pipelineInfo.depthBiasEnable = VK_TRUE;
    pipelineInfo.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };
    pipelineInfo.vertexInputAttributes = Vertex::getAttributeDescriptions({ VertexAttributes::POSITION});
    pipelineInfo.specializationInfo = false;
    pipelineInfo.conservativeRasterisation = true;
    shadowMap.Init(&descriptorSetManager, pipelineInfo);


    pipelineInfo.shaders = { resources.vertexShaders["ui"].get() ,  resources.fragmentShaders["ui"].get() };
    pipelineInfo.depthBiasEnable = VK_FALSE;
    pipelineInfo.pushConstants = { {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock)} };
    pipelineInfo.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    pipelineInfo.cullMode = VK_CULL_MODE_NONE;
    pipelineInfo.vertexInputBindings = imgui.bindingDescriptions();
    pipelineInfo.vertexInputAttributes = imgui.attributeDescriptions();
    pipelineInfo.blendEnabled = VK_TRUE;
    pipelineInfo.conservativeRasterisation = false;
    imgui.create(&devices, &renderPass, &descriptorSetManager, pipelineInfo);
    //imgui.create(window.glfwWindow, instance, surface, &devices, &swapChain, &renderPass);

    createCamera();
    createModelBuffers();
    createUniformBuffers();
    createDescriptorSets();
    AllocateCommandBuffers();
    buildCommandBuffers();
    createSyncObjects();

}

void RasterRenderer::mainLoop() {

    while (window.isActive()) {

        //if (g_SwapChainRebuild)
        //{
        //    g_SwapChainRebuild = false;
        //    ImGui_ImplVulkan_SetMinImageCount(3);
        //    imgui.vw.Surface = surface;
        //    ImGui_ImplVulkanH_CreateWindow(instance, devices.physicalDevice, devices.logicalDevice, &imgui.vw,
        //        devices.indices.graphicsFamily.value(), nullptr, 800, 600, imgui.vw.ImageCount);
        //    imgui.vw.FrameIndex = 0;
        //}

        //ImGui_ImplVulkan_NewFrame();
        //ImGui_ImplGlfw_NewFrame();
        //ImGui::NewFrame();
        //
        ////ImGui::ShowAboutWindow();
        ////ImGui::ShowDemoWindow();

        //if (ImGui::Begin("Depth Texture"));
        ////ImGui::Text("Hello I am under de water");
        //ImGui::Image((ImTextureID)ImGui_ImplVulkan_AddTexture(shadowMap.depthTexture.sampler, shadowMap.depthTexture.imageView, shadowMap.depthTexture.descriptorInfo.imageLayout), ImVec2(250, 250));
        //ImGui::End();

        //ImGui::Render();

        //ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        //memcpy(&imgui.vw.ClearValue.color.float32[0], &clear_color, 4 * sizeof(float));

        drawFrame();
    }

    vkDeviceWaitIdle(devices.logicalDevice);
}

void RasterRenderer::cleanupSwapChain() {

    frameBuffer.Destroy();

    vkFreeCommandBuffers(devices.logicalDevice, devices.commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    pipeline.Destroy(false);
    renderPass.Destroy();

    swapChain.Destroy();


    //imgui.destroy();
    imgui.deinit();

}

void RasterRenderer::cleanup() {
    cleanupSwapChain();

    for (size_t i = 0; i < swapChain.imageCount; i++) {
        lightBuffers[i].Destroy();
        cameraBuffers[i].Destroy();
    }

    imgui.destroy();

    shadowMap.Destroy();

    descriptorSetManager.destroy();

    for (auto& go : gameObjects) {
        go.Destroy();
    }

    resources.Destroy();

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

    window.destroy();
}

void RasterRenderer::recreateSwapChain() {
    //int width = 0, height = 0;
    //glfwGetFramebufferSize(window.glfwWindow, &width, &height);
    //while (width == 0 || height == 0) {
    //    glfwGetFramebufferSize(window.glfwWindow, &width, &height);
    //    glfwWaitEvents();
    //}
    window.resize();
    vkDeviceWaitIdle(devices.logicalDevice);

    cleanupSwapChain();

    swapChain.Init();
    renderPass.Init();

    frameBuffer.Create(&devices, renderPass.vkRenderPass);
    for (size_t i = 0; i < swapChain.imageCount; i++)
    {
        std::vector<VkImageView> attachments{ swapChain.images[i].imageView, swapChain.depthImage.imageView };
        frameBuffer.createFramebuffer(attachments, swapChain.extent);
    }

    pipeline.Init();
    camera.init(swapChain.extent);
    imgui.reinit();
    //createUniformBuffers();
    //createDescriptorSets();
    AllocateCommandBuffers();
    buildCommandBuffers();

    imagesInFlight.resize(swapChain.imageCount, VK_NULL_HANDLE);
}

void RasterRenderer::createInstance() {
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

void RasterRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void RasterRenderer::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void RasterRenderer::createSurface() {
    if (glfwCreateWindowSurface(instance, window.glfwWindow, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void RasterRenderer::loadResources() {

    resources.Init(&devices);

    resources.LoadMesh("cube");
    resources.LoadMesh("sphere");
    resources.LoadMesh("tree");
    resources.LoadMesh("plane");
    resources.LoadImage("texture");
    resources.LoadShader("shadowmapping/scene", VK_SHADER_STAGE_VERTEX_BIT);
    resources.LoadShader("shadowmapping/scene", VK_SHADER_STAGE_FRAGMENT_BIT);
    resources.LoadShader("shadowmapping/offscreen", VK_SHADER_STAGE_VERTEX_BIT);
    resources.LoadShader("imgui/ui", VK_SHADER_STAGE_VERTEX_BIT);
    resources.LoadShader("imgui/ui", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void RasterRenderer::createModelBuffers() {

    float value = 10.0f;
    float x = -value;
    float z = -value;
    for (size_t i = 0; i < gameObjectCount; i++)
    {
        auto& go = gameObjects.emplace_back(GameObject());
        if (i < gameObjectCount - 2) {
            go.transform.position = glm::vec3(x, -1.0f, z);
            go.transform.scale = glm::vec3(5, 5, 5);
            go.mesh = resources.meshes["tree"].get();
        }
        else if (i == gameObjectCount - 2)
        {
            go.transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
            go.transform.scale = glm::vec3(30, 1, 30);
            //go.shadowCaster = false;
            go.mesh = resources.meshes["plane"].get();

        }
        else if (i == gameObjectCount - 1)
        {
            go.shadowCaster = false;
            go.shadowReceiver = false;
            go.transform.scale = glm::vec3(0.2f, 0.2f, 0.2f);
            go.mesh = resources.meshes["sphere"].get();
        }
        go.image = resources.images["texture"].get();
        go.Init();


        if (x >= value)
        {
            z += value;
            x = -value;
        }
        else {
            x += value;
        }
    }
}

void RasterRenderer::createUniformBuffers() {

    for (auto& go : gameObjects) {
        go.uniformBuffers.resize(swapChain.imageCount);

        for (size_t i = 0; i < swapChain.imageCount; i++) {
            VkDeviceSize bufferSize = sizeof(ModelUBO);
            go.uniformBuffers[i].Create(&devices, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }

    //uniformBuffers.offscreen.resize(swapChain.imageCount);
    cameraBuffers.resize(swapChain.imageCount);
    lightBuffers.resize(swapChain.imageCount);
    for (size_t i = 0; i < swapChain.imageCount; i++) {
        VkDeviceSize bufferSize = sizeof(cameraUBO);
        cameraBuffers[i].Create(&devices, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        bufferSize = sizeof(lightUBO);
        lightBuffers[i].Create(&devices, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    }
}

void RasterRenderer::createDescriptorSets() {


    DescriptorSetRequest request;
    request.requests.push_back({ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
    descriptorSetManager.getDescriptorSets(cameraDescSets, request);
    descriptorSetManager.getDescriptorSets(lightDescSets, request);
    for (size_t i = 0; i < swapChain.imageCount; i++) {
        VkDescriptorBufferInfo bufferInfo = Initialisers::descriptorBufferInfo(cameraBuffers[i].vkBuffer, sizeof(cameraUBO));

        std::vector<VkWriteDescriptorSet> descriptorWrites{
            Initialisers::writeDescriptorSet(cameraDescSets[i], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo)
        };

        vkUpdateDescriptorSets(devices.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

        bufferInfo = Initialisers::descriptorBufferInfo(lightBuffers[i].vkBuffer, sizeof(lightUBO));

        descriptorWrites = {
        Initialisers::writeDescriptorSet(lightDescSets[i], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo),
        };

        vkUpdateDescriptorSets(devices.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    }

    DescriptorSetRequest request2;
    request2.requests.push_back({ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
    request2.requests.push_back({ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });
    request2.requests.push_back({ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });

    for (auto& go : gameObjects) {

        descriptorSetManager.getDescriptorSets(go.descriptorSets, request2);
        descriptorSetManager.getDescriptorSets(go.offModelDescSets, request);

        for (size_t i = 0; i < swapChain.imageCount; i++) {

            std::vector<VkWriteDescriptorSet> descriptorWrites{
            Initialisers::writeDescriptorSet(go.descriptorSets[i], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &go.uniformBuffers[i].descriptorInfo),
            Initialisers::writeDescriptorSet(go.descriptorSets[i], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &shadowMap.depthTexture.descriptorInfo),
            Initialisers::writeDescriptorSet(go.descriptorSets[i], 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &go.image->texture.descriptorInfo)
            };

            vkUpdateDescriptorSets(devices.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

            descriptorWrites = {
            Initialisers::writeDescriptorSet(go.offModelDescSets[i], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &go.uniformBuffers[i].descriptorInfo)
            };

            vkUpdateDescriptorSets(devices.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

        }
    }


    request.requests = { { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER } };

    descriptorSetManager.getDescriptorSets(shadowMap.descriptorSets, request);

    for (size_t i = 0; i < swapChain.imageCount; i++) {

        std::vector<VkWriteDescriptorSet> descriptorWrites{
        Initialisers::writeDescriptorSet(shadowMap.descriptorSets[i], 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &shadowMap.depthTexture.descriptorInfo)
        };

        vkUpdateDescriptorSets(devices.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

}

void RasterRenderer::AllocateCommandBuffers() {
    commandBuffers.resize(frameBuffer.vkFrameBuffers.size());

    VkCommandBufferAllocateInfo allocInfo = Initialisers::commandBufferAllocateInfo(devices.commandPool, static_cast<uint32_t>(commandBuffers.size()));

    if (vkAllocateCommandBuffers(devices.logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");

}

void RasterRenderer::buildCommandBuffers() {

    VkCommandBufferBeginInfo beginInfo = Initialisers::commandBufferBeginInfo();

    std::array<VkClearValue, 2> clearValues;

    if(imgui.enabled)
        imgui.updateBuffers();

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

            shadowMap.renderPass.Begin(commandBuffers[i], shadowMap.frameBuffer.vkFrameBuffers[i], { shadowMap.width , shadowMap.height }, &clearValues[0]);

            // Set depth bias (aka "Polygon offset")
            // Required to avoid shadow mapping artifacts
            vkCmdSetDepthBias(commandBuffers[i], 1.25f, 0.0f, 1.75f);

            VkViewport viewport = Initialisers::viewport(0, 0, (float)shadowMap.width, (float)shadowMap.height);
            VkRect2D scissor = Initialisers::scissor({ shadowMap.width, shadowMap.height });

            vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipeline.vkPipeline);

            VkDeviceSize offsets[] = { 0 };

            int j = 0;
            VkBuffer lastBuffer = nullptr;
            for (auto& go : gameObjects) {
                if (!go.shadowCaster)
                    continue;

                std::array<VkDescriptorSet, 2> descriptorSets = { lightDescSets[i] , go.offModelDescSets[i] };
                vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipeline.pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

                if (go.mesh->vertexBuffer->vkBuffer != lastBuffer)
                {
                    lastBuffer = go.mesh->vertexBuffer->vkBuffer;
                    go.mesh->Bind(commandBuffers[i]);

                }
                vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(go.mesh->indices.size()), 1, 0, 0, 0);
                j++;
            }

            shadowMap.renderPass.End(commandBuffers[i]);
        }

        {
            clearValues[0].color = { 0.025f, 0.025f, 0.025f, 1.0f };
            clearValues[1].depthStencil = { 1.0f, 0 };

            renderPass.Begin(commandBuffers[i], frameBuffer.vkFrameBuffers[i], swapChain.extent, clearValues.data(), static_cast<uint32_t>(clearValues.size()));

            camera.setViewport(commandBuffers[i]);

            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vkPipeline);

            VkBuffer lastBuffer = nullptr;
            VkDeviceSize offsets[] = { 0 };
            for (auto& go : gameObjects) {

                std::array<VkDescriptorSet, 3> descriptorSets = { cameraDescSets[i], go.descriptorSets[i], lightDescSets[i] };

                vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

                if (go.mesh->vertexBuffer->vkBuffer != lastBuffer)
                {
                    lastBuffer = go.mesh->vertexBuffer->vkBuffer;
                    go.mesh->Bind(commandBuffers[i]);
                }
                vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(go.mesh->indices.size()), 1, 0, 0, 0);


            }

            if (imgui.enabled)
            {
                imgui.Draw(commandBuffers[i], i);
            }

            renderPass.End(commandBuffers[i]);
        }

        //{

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void RasterRenderer::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChain.imageCount, VK_NULL_HANDLE);

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

void RasterRenderer::createCamera() {
    camera.lookAt = glm::vec3(0, 0, 0);
    camera.transform.position = glm::vec3(0, 0, 10);
    camera.transform.rotation.y = 180.f;

    camera.init(swapChain.extent);
}


void RasterRenderer::updateUniformBuffer(uint32_t currentImage) {

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - startTime).count() / 1000.0f;
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


    if (glfwGetKey(window.glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
        auto forward = camera.transform.forward;
        forward.y = 0;
        camera.transform.position += forward * 10.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
        auto forward = camera.transform.forward;
        forward.y = 0;
        camera.transform.position -= forward * 10.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
        auto right = camera.transform.right;
        right.y = 0;
        camera.transform.position += right * 10.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
        auto right = camera.transform.right;
        right.y = 0;
        camera.transform.position -= right * 10.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera.transform.position += camera.worldUp * 10.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        camera.transform.position -= camera.worldUp * 10.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.transform.rotation.y -= 50.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.transform.rotation.y += 50.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_Z) == GLFW_PRESS)
    {
        camera.transform.rotation.x -= 50.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_C) == GLFW_PRESS)
    {
        camera.transform.rotation.x += 50.0f * time;
    }

    if (glfwGetKey(window.glfwWindow, GLFW_KEY_KP_8) == GLFW_PRESS)
    {
        if (ortho)
            lightInvDir += glm::vec3(0, 1, 0) * 5.0f * time;
        else
            lightPos += camera.transform.forward * 5.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_KP_5) == GLFW_PRESS)
    {
        if (ortho)
            lightInvDir -= glm::vec3(0, 1, 0) * 5.0f * time;
        else
            lightPos -= camera.transform.forward * 5.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_KP_4) == GLFW_PRESS)
    {
        if (ortho)
            lightInvDir -= glm::vec3(1, 0, 0) * 5.0f * time;
        else
            lightPos += camera.transform.right * 5.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_KP_6) == GLFW_PRESS)
    {
        if (ortho)
            lightInvDir += glm::vec3(1, 0, 0) * 5.0f * time;
        else
            lightPos -= camera.transform.right * 5.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_KP_9) == GLFW_PRESS)
    {
        if (ortho)
            lightInvDir += glm::vec3(0, 0, 1) * 5.0f * time;
        else
            lightPos.y += 5.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_KP_7) == GLFW_PRESS)
    {
        if (ortho)
            lightInvDir -= glm::vec3(0, 0, 1) * 5.0f * time;
        else
            lightPos.y -= 5.0f * time;
    }

    if (glfwGetKey(window.glfwWindow, GLFW_KEY_P) == GLFW_PRESS)
    {
        gameObjects[0].transform.scale += glm::vec3(1.0f) * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_O) == GLFW_PRESS)
    {
        gameObjects[0].transform.scale -= glm::vec3(1.0f) * time;
    }

    if (glfwGetKey(window.glfwWindow, GLFW_KEY_M) == GLFW_PRESS)
    {
        lightFOV += 5.0f * time;
    }
    if (glfwGetKey(window.glfwWindow, GLFW_KEY_N) == GLFW_PRESS)
    {
        lightFOV -= 5.0f * time;
    }

    camera.update(static_cast<float>(swapChain.extent.width), static_cast<float>(swapChain.extent.height));

    float zNear = 1.0f;
    float zFar = 100.0f;

    glm::vec3 lightLookAt = lightPos + glm::vec3(0.0f, -0.5f, 0.5f);
    //lightPos = camera.transform.position;

    // Matrix from light's point of view
    glm::mat4 depthProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 depthViewMatrix = glm::mat4(1.0f);
    if (!ortho)
    {
        depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
        depthViewMatrix = glm::lookAt(lightPos, lightLookAt, glm::vec3(0, 1, 0));
    }
    //depthProjectionMatrix[1][1] *= -1;
    else
    {
        //depthProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, zNear, zFar);
        depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
        depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0.0f), glm::vec3(0, 1, 0));
    }
    glm::mat4 depthModelMatrix = glm::mat4(1.0f);

    //uboOffscreenVS.depthMVP = depthProjectionMatrix * depthViewMatrix *depthModelMatrix;

    lightUBO.depthBiasMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
    lightUBO.lightPos = lightPos;
    lightBuffers[currentImage].Map(&lightUBO);

    cameraUBO.camPos = camera.transform.position;
    cameraUBO.view = camera.view;
    cameraUBO.projection = camera.projection;
    cameraBuffers[currentImage].Map(&cameraUBO);

    // uniformBuffers.offscreen[currentImage].Map(&uboOffscreenVS);

    gameObjects[gameObjectCount - 1].transform.position = lightPos;

    for (auto& go : gameObjects)
    {
        go.Update();

        ModelUBO ubos;
        ubos.model = go.model;
        go.uniformBuffers[currentImage].Map(&ubos);
    }

    if (imgui.enabled)
    {
        imgui.update(swapChain.extent);
        bool temp = false;
        imgui.startFrame();

        //imgui.NewWindow("Window", &temp);

        ImGui::Begin("Depth Texture");
        ImGui::Image((ImTextureID)shadowMap.depthTexture.image, ImVec2(500, 250));
        ImGui::End();


        //imgui.NewImage(shadowMap.depthTexture., shadowMap.depthTexture.descriptorInfo.imageLayout);

        imgui.endFrame();
    }
}

void RasterRenderer::drawFrame() {
    
    //std::array<VkClearValue, 2> clearValues = {};
    //clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    //clearValues[1].depthStencil = { 1.0f, 0 };

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

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(devices.logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    VkResult err;


   // ImGui_ImplVulkanH_Frame* fd = &imgui.vw.Frames[imageIndex];
    //{
    //    err = vkWaitForFences(devices.logicalDevice, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking

    //    err = vkResetFences(devices.logicalDevice, 1, &fd->Fence);
    //}
    //{
    //    err = vkResetCommandPool(devices.logicalDevice, imgui.commandPool, 0);
    //    VkCommandBufferBeginInfo info = {};
    //    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //    err = vkBeginCommandBuffer(imgui.commandBuffers[imageIndex], &info);
    //}
    //{
    //    VkRenderPassBeginInfo info = {};
    //    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //    info.renderPass = imgui.renderPass;
    //    info.framebuffer = imgui.framebuffers[imageIndex];
    //    info.renderArea.extent = swapChain.extent;
    //    info.clearValueCount = static_cast<uint32_t>(clearValues.size());
    //    info.pClearValues = clearValues.data();
    //    vkCmdBeginRenderPass(imgui.commandBuffers[imageIndex], &info, VK_SUBPASS_CONTENTS_INLINE);
    //}

    //// Record Imgui Draw Data and draw funcs into command buffer
    //ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imgui.commandBuffers[imageIndex]);
    //vkCmdEndRenderPass(imgui.commandBuffers[imageIndex]);
    //err = vkEndCommandBuffer(imgui.commandBuffers[imageIndex]);
    updateUniformBuffer(imageIndex);


    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };


    std::array<VkCommandBuffer, 1> submitCommandBuffers =
    { commandBuffers[imageIndex] };

    VkSubmitInfo submitInfo = Initialisers::submitInfo(submitCommandBuffers.data(), static_cast<uint32_t>(submitCommandBuffers.size()), waitSemaphores, 1, signalSemaphores, 1, waitStages);

    vkResetFences(devices.logicalDevice, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(devices.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
 

    /// present
    //VkSwapchainKHR swapChains[] = { swapChain.vkSwapChain, imgui.vw.Swapchain };
    VkSwapchainKHR swapChains[] = { swapChain.vkSwapChain };

    VkPresentInfoKHR presentInfo = Initialisers::presentInfoKHR(signalSemaphores, 1, swapChains, 1, &imageIndex);

    result = vkQueuePresentKHR(devices.presentQueue, &presentInfo);

    if (imgui.enabled)
    {
        vkQueueWaitIdle(devices.presentQueue);
        buildCommandBuffers();
    }

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.framebufferResized) {
        window.framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

std::vector<const char*> RasterRenderer::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    return extensions;
}

bool RasterRenderer::checkValidationLayerSupport() {
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