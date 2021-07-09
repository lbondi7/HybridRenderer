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

    Log("How many Objects? ");
    std::cin >> gameObjectCount;
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
    app->window.framebufferResized = true;
}

void RasterRenderer::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<RasterRenderer*>(glfwGetWindowUserPointer(window));

    app->imgui.leftMouse = button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_RELEASE;
    app->imgui.rightMouse = button == GLFW_MOUSE_BUTTON_RIGHT && action != GLFW_RELEASE;
}

void RasterRenderer::scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<RasterRenderer*>(glfwGetWindowUserPointer(window));


    //app->imgui.mouseWheel += static_cast<float>(yOffset) * 0.01f;
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

    RenderPassInfo info{};
    info.attachments.push_back({ AttachmentType::COLOUR, swapChain.imageFormat, VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED});
    info.attachments.push_back({ AttachmentType::DEPTH, Utility::findDepthFormat(devices.physicalDevice), VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED });

    info.dependencies.emplace_back(Initialisers::subpassDependency(VK_SUBPASS_EXTERNAL, 0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));
    renderPass.Create(&devices, info);
    info.attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    penultimateRenderPass.Create(&devices, info);

    descriptorSetManager.init(&devices, swapChain.imageCount);

    PipelineInfo pipelineInfo{};

    pipelineInfo.shaders = { resources.vertexShaders["scene"].get(), resources.fragmentShaders["scene"].get() };
    pipelineInfo.vertexInputAttributes = Vertex::getAttributeDescriptions({ VertexAttributes::POSITION, VertexAttributes::UV_COORD, VertexAttributes::V_COLOUR, VertexAttributes::NORMAL });
    pipelineInfo.vertexInputBindings = { Vertex::getBindingDescription() };
    pipelineInfo.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    pipeline.Create(&devices, &renderPass, &descriptorSetManager, pipelineInfo);
    frameBuffer.Create(&devices, renderPass.vkRenderPass);
    penultimateFrameBuffer.Create(&devices, penultimateRenderPass.vkRenderPass);
    for (size_t i = 0; i < swapChain.imageCount; i++)
    {
        std::vector<VkImageView> attachments{ swapChain.images[i].imageView, swapChain.depthImage.imageView };
        frameBuffer.createFramebuffer(attachments, swapChain.extent);
        penultimateFrameBuffer.createFramebuffer(attachments, swapChain.extent);
    }

    shadowMap.Create(&devices, &swapChain);
    pipelineInfo.shaders = { resources.vertexShaders["offscreen"].get() };
    pipelineInfo.depthBiasEnable = VK_TRUE;
    pipelineInfo.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };
    pipelineInfo.vertexInputAttributes = Vertex::getAttributeDescriptions({ VertexAttributes::POSITION});
    pipelineInfo.conservativeRasterisation = true;
    pipelineInfo.collorAttachmentCount = 0;
    shadowMap.Init(&descriptorSetManager, pipelineInfo);

    imgui.create(window.glfwWindow, instance, surface, &devices, &swapChain);

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

        drawFrame();
    }

    vkDeviceWaitIdle(devices.logicalDevice);
}

void RasterRenderer::cleanupSwapChain() {

    frameBuffer.Destroy();
    penultimateFrameBuffer.Destroy();

    vkFreeCommandBuffers(devices.logicalDevice, devices.commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    pipeline.Destroy(false);
    renderPass.Destroy();
    penultimateRenderPass.Destroy();

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

    shadowMap.Destroy(true);

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

    window.resize();
    vkDeviceWaitIdle(devices.logicalDevice);

    cleanupSwapChain();

    swapChain.Init();
    renderPass.Init();
    penultimateRenderPass.Init();

    frameBuffer.Create(&devices, renderPass.vkRenderPass);
    penultimateFrameBuffer.Create(&devices, renderPass.vkRenderPass);
    for (size_t i = 0; i < swapChain.imageCount; i++)
    {
        std::vector<VkImageView> attachments{ swapChain.images[i].imageView, swapChain.depthImage.imageView };
        frameBuffer.createFramebuffer(attachments, swapChain.extent);
        penultimateFrameBuffer.createFramebuffer(attachments, swapChain.extent);
    }

    shadowMap.reinit(true);
    pipeline.Init();
    camera.init(swapChain.extent);
    imgui.reinit();
    AllocateCommandBuffers();
    buildCommandBuffers();
    //buildCommandBuffersImGui();

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
    resources.LoadTexture("texture");
    resources.LoadShader("shadowmapping/scene", VK_SHADER_STAGE_VERTEX_BIT);
    resources.LoadShader("shadowmapping/scene", VK_SHADER_STAGE_FRAGMENT_BIT);
    resources.LoadShader("shadowmapping/offscreen", VK_SHADER_STAGE_VERTEX_BIT);
    resources.LoadShader("imgui/ui", VK_SHADER_STAGE_VERTEX_BIT);
    resources.LoadShader("imgui/ui", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void RasterRenderer::createModelBuffers() {

    float value = 0;
    float dist = 20.0f;
    if (gameObjectCount < 3)
        gameObjectCount = 3;
    
    for (size_t i = 0; i < gameObjectCount - 2; i++)
    {
        if (i * i > gameObjectCount - 2)
        {
            value = i - 1;
            break;
        }
    }
    
    float max = ((value * dist) / 2.0f);

    float x = gameObjectCount > 3 ? -max : 0;
    float z = gameObjectCount > 5 ? -max : 0;
    for (size_t i = 0; i < gameObjectCount; i++)
    {
        auto& go = gameObjects.emplace_back(GameObject());
        if (i < gameObjectCount - 2) {
            go.transform.position = glm::vec3(x, -1.0f, z);
            go.transform.scale = glm::vec3(5, 5, 5);
            go.mesh = resources.meshes["tree"].get();
        }
        if (i == gameObjectCount - 2)
        {
            go.transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
            go.transform.scale = glm::vec3(max > 5 ? max : 5, 1, max > 5 ? max : 5);
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
        go.texture = resources.textures["texture"].get();
        go.Init();


        if (x >= max)
        {
            z += dist;
            x = -max;
        }
        else {
            x += dist;
        }
    }
}

void RasterRenderer::createUniformBuffers() {

    for (auto& go : gameObjects) {
        go.uniformBuffers.resize(swapChain.imageCount);

        for (size_t i = 0; i < swapChain.imageCount; i++) {
            VkDeviceSize bufferSize = sizeof(ModelUBO);
            go.uniformBuffers[i].Create2(&devices, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }

    //uniformBuffers.offscreen.resize(swapChain.imageCount);
    cameraBuffers.resize(swapChain.imageCount);
    lightBuffers.resize(swapChain.imageCount);
    for (size_t i = 0; i < swapChain.imageCount; i++) {
        VkDeviceSize bufferSize = sizeof(cameraUBO);
        cameraBuffers[i].Create2(&devices, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        bufferSize = sizeof(lightUBO);
        lightBuffers[i].Create2(&devices, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    }
}

void RasterRenderer::createDescriptorSets() {

    using BindingType = std::pair<uint32_t, VkDescriptorType>;
    DescriptorSetRequest request;
    request.ids.emplace_back(BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));

    for (size_t i = 0; i < swapChain.imageCount; i++) {

        request.data.push_back(&lightBuffers[i].descriptorInfo);
    }
    descriptorSetManager.createDescriptorSets(lightDescSets, request);
    request.data.clear();
    for (size_t i = 0; i < swapChain.imageCount; i++) {

        request.data.push_back(&cameraBuffers[i].descriptorInfo);
    }
    descriptorSetManager.createDescriptorSets(cameraDescSets, request);
    DescriptorSetRequest request2;
    request2.ids.emplace_back(BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
    request2.ids.emplace_back(BindingType(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));

    int i = 0;
    for (auto& go : gameObjects)
    {
        request2.data.clear();
        for (size_t i = 0; i < swapChain.imageCount; i++) {

            request2.data.push_back(&go.uniformBuffers[i].descriptorInfo);
            request2.data.push_back(&go.texture->descriptorInfo);
        }
        descriptorSetManager.createDescriptorSets(go.descriptorSets, request2);

        request.data.clear();
        for (size_t i = 0; i < swapChain.imageCount; i++) {

            request.data.push_back(&go.uniformBuffers[i].descriptorInfo);
        }
        descriptorSetManager.createDescriptorSets(go.offModelDescSets, request);
    }

    descTest = ImGui_ImplVulkan_AddTextureD(gameObjects[0].texture->sampler, gameObjects[0].texture->imageView, gameObjects[0].texture->descriptorInfo.imageLayout);

}

void RasterRenderer::AllocateCommandBuffers() {
    commandBuffers.resize(frameBuffer.frames.size());

    VkCommandBufferAllocateInfo allocInfo = Initialisers::commandBufferAllocateInfo(devices.commandPool, static_cast<uint32_t>(commandBuffers.size()));

    if (vkAllocateCommandBuffers(devices.logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");

}

void RasterRenderer::buildCommandBuffers() {


    for (int32_t i = 0; i < commandBuffers.size(); ++i)
    {
        buildCommandBuffer(i);
    }
}

void RasterRenderer::buildCommandBuffer(uint32_t i) {

    VkCommandBufferBeginInfo beginInfo = Initialisers::commandBufferBeginInfo();

    std::array<VkClearValue, 2> clearValues;

    //if(imgui.enabled)
    //    imgui.updateBuffers();

    if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    /*
        First render pass: Generate shadow map by rendering the scene from light's POV
    */
    {
        clearValues[0].depthStencil = { 1.0f, 0 };

        auto& frame = shadowMap.frameBuffer.frames[i];
        shadowMap.renderPass.Begin(commandBuffers[i], frame.vkFrameBuffer, frame.extent, &clearValues[0]);

        // Set depth bias (aka "Polygon offset")
        // Required to avoid shadow mapping artifacts
        vkCmdSetDepthBias(commandBuffers[i], 1.25f, 0.0f, 1.75f);

        VkViewport viewport = Initialisers::viewport(0, 0, (float)frame.extent.width, (float)frame.extent.height);
        VkRect2D scissor = Initialisers::scissor(frame.extent);

        vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipeline.vkPipeline);

        VkDeviceSize offsets[] = { 0 };

        int j = 0;
        VkBuffer lastBuffer = VK_NULL_HANDLE;
        for (auto& go : gameObjects) {
            if (!go.shadowCaster)
                continue;

            std::array<VkDescriptorSet, 2> descriptorSets = { lightDescSets[i] , go.offModelDescSets[i] };
            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipeline.pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
            go.mesh->Bind(commandBuffers[i]);
            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(go.mesh->indices.size()), 1, 0, 0, 0);
            j++;
        }

        shadowMap.renderPass.End(commandBuffers[i]);
    }

    {
        clearValues[0].color = { 0.025f, 0.025f, 0.025f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };

        if(!imgui.enabled)
            renderPass.Begin(commandBuffers[i], frameBuffer.frames[i].vkFrameBuffer, swapChain.extent, clearValues.data(), static_cast<uint32_t>(clearValues.size()));
        else
            penultimateRenderPass.Begin(commandBuffers[i], penultimateFrameBuffer.frames[i].vkFrameBuffer, swapChain.extent, clearValues.data(), static_cast<uint32_t>(clearValues.size()));

        camera.setViewport(commandBuffers[i]);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vkPipeline);

        VkBuffer lastBuffer = VK_NULL_HANDLE;
        VkDeviceSize offsets[] = { 0 };
        for (auto& go : gameObjects) {

            std::array<VkDescriptorSet, 4> descriptorSets = { cameraDescSets[i], go.descriptorSets[i], lightDescSets[i], shadowMap.descriptorSets[i] };

            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

            go.mesh->Bind(commandBuffers[i]);
            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(go.mesh->indices.size()), 1, 0, 0, 0);
        }

        if (!imgui.enabled)
            renderPass.End(commandBuffers[i]);
        else
            penultimateRenderPass.End(commandBuffers[i]);

        //renderPass.End(commandBuffers[i]);

    }

    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    if (imgui.enabled) {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(imgui.commandBuffers[i], &info);
        {

            imgui.renderPass.Begin(imgui.commandBuffers[i], imgui.frameBuffer.frames[i].vkFrameBuffer, swapChain.extent, clearValues.data(), static_cast<uint32_t>(clearValues.size()));
            //VkRenderPassBeginInfo info = {};
            //info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            //info.renderPass = imgui.vkRenderPass;
            //info.framebuffer = imgui.frameBuffers[i];
            //info.renderArea.extent = swapChain.extent;
            //info.clearValueCount = static_cast<uint32_t>(clearValues.size());
            //info.pClearValues = clearValues.data();
            //vkCmdBeginRenderPass(imgui.commandBuffers[i], &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        if (imgui.drawn) {
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imgui.commandBuffers[i]);
            imgui.drawn = false;
        }
        imgui.renderPass.End(imgui.commandBuffers[i]);
        //vkCmdEndRenderPass(imgui.commandBuffers[i]);
        vkEndCommandBuffer(imgui.commandBuffers[i]);

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

    //imageAvailableSemaphores.resize(3);
    //renderFinishedSemaphores.resize(3);
    //inFlightFences.resize(3);
    //imagesInFlight.resize(swapChain.imageCount, VK_NULL_HANDLE);

    //VkSemaphoreCreateInfo semaphoreInfo = Initialisers::semaphoreCreateInfo();

    //VkFenceCreateInfo fenceInfo = Initialisers::fenceCreateInfo();

    //for (size_t i = 0; i < 3; i++) {
    //    if (vkCreateSemaphore(devices.logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
    //        vkCreateSemaphore(devices.logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
    //        vkCreateFence(devices.logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create synchronization objects for a frame!");
    //    }
    //}
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
        depthProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, zNear, zFar);
        depthViewMatrix = glm::lookAt(lightInvDir, -lightInvDir, glm::vec3(0, 1, 0));
    }
    glm::mat4 depthModelMatrix = glm::mat4(1.0f);

    //uboOffscreenVS.depthMVP = depthProjectionMatrix * depthViewMatrix *depthModelMatrix;

    lightUBO.depthBiasMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
    lightUBO.lightPos = lightPos;
    lightBuffers[currentImage].Map2(&lightUBO);

    cameraUBO.camPos = camera.transform.position;
    cameraUBO.view = camera.view;
    cameraUBO.projection = camera.projection;
    cameraBuffers[currentImage].Map2(&cameraUBO);

    // uniformBuffers.offscreen[currentImage].Map(&uboOffscreenVS);

    gameObjects[gameObjectCount - 1].transform.position = lightPos;

    for (auto& go : gameObjects)
    {
        go.Update();

        ModelUBO ubos;
        ubos.model = go.model;
        go.uniformBuffers[currentImage].Map2(&ubos);
    }

    if (imgui.enabled)
    {

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();


        //imgui.update(swapChain.extent);
        bool temp = false;
        //imgui.startFrame();

        ImGui::Begin("Options");
        ImGui::Checkbox("Orthographic", &ortho);
        if (ImGui::Checkbox("Conservative Rasterisation", &shadowMap.pipeline.pipelineInfo.conservativeRasterisation))
            window.framebufferResized = true;
        if (ImGui::SliderInt("Shadow Map Resultion", &shadowMap.resolution, 1, 8192))
            window.framebufferResized = true;


        //std::vector<VkWriteDescriptorSet> descriptorWrites{

        //    Initialisers::writeDescriptorSet(shadowMap.descriptorSets[currentImage], 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &shadowMap.depthTexture.descriptorInfo) 
        //};

        //vkUpdateDescriptorSets(devices.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

        //VkDescriptorSet descSet;
        //DescriptorSetRequest request;
        //request.ids = { {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER} };
        //request.data = {&gameObjects[0].texture->descriptorInfo};
        //descriptorSetManager.getTempDescriptorSet(&descSet, request, true);
        //ImGui::Image((ImTextureID)descSet, {200, 200 });
        ImGui::Image((ImTextureID)descTest, {200, 200 });
        //ImGui_ImplVulkan_AddTexture();

        //descriptorSetManager.freeDescriptorSet(&descSet);
        ImGui::End();
        ImGui::Render();
        imgui.drawn = true;

       // vkFreeDescriptorSets(devices.logicalDevice, imgui.imGuiDescriptorPool, 1, &desc);
       // imgui.endFrame();


    }

    //if (prevConservativeRendering != shadowMap.pipeline.pipelineInfo.conservativeRasterisation)
    //{
    //    prevConservativeRendering = shadowMap.pipeline.pipelineInfo.conservativeRasterisation;
    //    window.framebufferResized = true;
    //}

    //shadowMap.update(window.framebufferResized);

}

void RasterRenderer::drawFrame() {
    

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkSemaphore iAS = imageAvailableSemaphores[currentFrame];
    VkSemaphore rFS = renderFinishedSemaphores[currentFrame];

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(devices.logicalDevice, swapChain.vkSwapChain, UINT64_MAX, iAS, VK_NULL_HANDLE, &imageIndex);


    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(imageIndex);

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    vkWaitForFences(devices.logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(devices.logicalDevice, 1, &inFlightFences[currentFrame]);


    std::vector<VkCommandBuffer> submitCommandBuffers =
    { commandBuffers[imageIndex] };

    if (imgui.enabled) {

        buildCommandBuffer(imageIndex);

        submitCommandBuffers.emplace_back(imgui.commandBuffers[imageIndex]);
    }

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo = Initialisers::submitInfo(
        submitCommandBuffers.data(), static_cast<uint32_t>(submitCommandBuffers.size()), &iAS, 1, &rFS, 1, waitStages);


    if (vkQueueSubmit(devices.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = { swapChain.vkSwapChain };
    VkPresentInfoKHR presentInfo = Initialisers::presentInfoKHR(&rFS, 1, swapChains, 1, &imageIndex);
    result = vkQueuePresentKHR(devices.presentQueue, &presentInfo);


    //vkWaitForFences(devices.logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    //uint32_t imageIndex;
    //VkResult result = vkAcquireNextImageKHR(devices.logicalDevice, swapChain.vkSwapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);


    //if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    //    recreateSwapChain();
    //    return;
    //}
    //else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    //    throw std::runtime_error("failed to acquire swap chain image!");
    //}

    //updateUniformBuffer(imageIndex);

    //imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    //VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
    //VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    //VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };


    //std::array<VkCommandBuffer, 1> submitCommandBuffers =
    //{ commandBuffers[imageIndex] };

    //VkSubmitInfo submitInfo = Initialisers::submitInfo(submitCommandBuffers.data(), static_cast<uint32_t>(submitCommandBuffers.size()), waitSemaphores, 1, signalSemaphores, 1, waitStages);

    //vkResetFences(devices.logicalDevice, 1, &inFlightFences[currentFrame]);

    //if (vkQueueSubmit(devices.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to submit draw command buffer!");
    //}
 
    //VkSwapchainKHR swapChains[] = { swapChain.vkSwapChain };

    //VkPresentInfoKHR presentInfo = Initialisers::presentInfoKHR(signalSemaphores, 1, swapChains, 1, &imageIndex);

    //result = vkQueuePresentKHR(devices.presentQueue, &presentInfo);

    //if (imgui.enabled)
    //{
    //    vkQueueWaitIdle(devices.presentQueue);
    //    buildCommandBuffers();
    //}

    //if (counted == 0)
    //{

    //    Texture dstImage;
    //    dstImage.Create(&devices, 800, 600, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //    VkCommandBuffer copyCmd = devices.generateCommandBuffer();

    //    dstImage.insertImageMemoryBarrier(
    //        copyCmd,
    //        0,
    //        VK_ACCESS_TRANSFER_WRITE_BIT,
    //        VK_IMAGE_LAYOUT_UNDEFINED,
    //        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //        VK_PIPELINE_STAGE_TRANSFER_BIT,
    //        VK_PIPELINE_STAGE_TRANSFER_BIT,
    //        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    //    // colorAttachment.image is already in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, and does not need to be transitioned

    //    VkImageCopy imageCopyRegion{};
    //    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    imageCopyRegion.srcSubresource.layerCount = 1;
    //    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    imageCopyRegion.dstSubresource.layerCount = 1;
    //    imageCopyRegion.extent.width = 800;
    //    imageCopyRegion.extent.height = 600;
    //    imageCopyRegion.extent.depth = 1;

    //    vkCmdCopyImage(
    //        copyCmd,
    //        swapChain.images[0].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    //        dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //        1,
    //        &imageCopyRegion);

    //    dstImage.insertImageMemoryBarrier(
    //        copyCmd,
    //        VK_ACCESS_TRANSFER_WRITE_BIT,
    //        VK_ACCESS_MEMORY_READ_BIT,
    //        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //        VK_IMAGE_LAYOUT_GENERAL,
    //        VK_PIPELINE_STAGE_TRANSFER_BIT,
    //        VK_PIPELINE_STAGE_TRANSFER_BIT,
    //        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });


    //    devices.EndCommandBuffer(copyCmd);

    //    // Get layout of the image (including row pitch)
    //    VkImageSubresource subResource{};
    //    subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    VkSubresourceLayout subResourceLayout;

    //    vkGetImageSubresourceLayout(devices.logicalDevice, dstImage.image, &subResource, &subResourceLayout);

    //    // Map image memory so we can start copying from it
    //    vkMapMemory(devices.logicalDevice, dstImage.vkMemory, 0, VK_WHOLE_SIZE, 0, (void**)&imagedata);
    //    imagedata += subResourceLayout.offset;


    //    const char* filename = "headless.ppm";
    //    std::ofstream file(filename, std::ios::out | std::ios::binary);

    //    // ppm header
    //    file << "P6\n" << 800 << "\n" << 600 << "\n" << 255 << "\n";

    //    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    //    // Check if source is BGR and needs swizzle
    //    std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
    //    const bool colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formatsBGR.end());

    //    // ppm binary pixel data
    //    for (int32_t y = 0; y < 600; y++) {
    //        unsigned int* row = (unsigned int*)imagedata;
    //        for (int32_t x = 0; x < 800; x++) {
    //            if (colorSwizzle) {
    //                file.write((char*)row + 2, 1);
    //                file.write((char*)row + 1, 1);
    //                file.write((char*)row, 1);
    //            }
    //            else {
    //                file.write((char*)row, 3);
    //            }
    //            row++;
    //        }
    //        imagedata += subResourceLayout.rowPitch;
    //    }
    //    file.close();

    //    printImage = false;

    //    vkUnmapMemory(devices.logicalDevice, dstImage.vkMemory);
    //    dstImage.Destroy();
    //    printImage = true;
    //    counted--;
    //}

    //if(!printImage)
    //    counted--;

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