#include "HybridEngine.h"

#include "ImGUI_.h"
#include "DebugLogger.h"

HybridEngine::~HybridEngine()
{
}

void HybridEngine::run()
{

	initialise();

	while (window->isActive() && !quit) {

        window->resize();
        timer.Update();
		prepare();
		update();
		render();
	}

	vkDeviceWaitIdle(core->deviceContext->logicalDevice);

    Deinitilise();
}

void HybridEngine::initialise()
{
    //std::cout << "How many trees?: " << std::endl;
    //std::cin >> gameObjectCount;
    //gameObjectCount += 2;

    window = std::make_unique<Window>();
    window->init(this);
    window->setupFramebufferResize(HybridEngine::framebufferResizeCallback);
    window->setupKeyCallback(HybridEngine::keyCallback);
    window->setupMouseCallback(HybridEngine::mouseCallback);
    window->setupScrollCallback(HybridEngine::scrollCallback);
    window->setupCursorCallback(HybridEngine::cursorCallback);

    core = std::make_unique<VulkanCore>();
	core->initialise(window->glfwWindow);

    swapChain.Create(core->surface, core->deviceContext.get(), &window->width, &window->height);

    resources.Init(core->deviceContext.get());

    resources.LoadTexture("texture.jpg");

    raster.Initialise(window.get(), core.get(), &swapChain, &resources);

    //rayTracing.Initialise(core->deviceContext.get(), window.get(), &swapChain, &resources);

    scene.Initialise(core->deviceContext.get(), &resources);

    auto imageCount = core->deviceContext->imageCount;

    camera.lookAt = glm::vec3(0, 0, 0);
    camera.transform.position = glm::vec3(0, 0, 10);
    camera.transform.rotation.y = 180.f;

    camera.init(core->deviceContext.get(), swapChain.extent);

    nextImageSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    presentSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChain.imageCount, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = Initialisers::semaphoreCreateInfo();

    VkFenceCreateInfo fenceInfo = Initialisers::fenceCreateInfo();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        if (vkCreateSemaphore(core->deviceContext->logicalDevice, &semaphoreInfo, nullptr, &nextImageSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(core->deviceContext->logicalDevice, &semaphoreInfo, nullptr, &presentSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(core->deviceContext->logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void HybridEngine::prepare()
{
    VkSemaphore iAS = nextImageSemaphores[currentFrame];

    auto result = swapChain.AquireNextImage(iAS, imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapChain.outdated = true;
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    raster.Prepare();
}

void HybridEngine::update()
{
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
        auto forward = camera.transform.forward;
        forward.y = 0;
        camera.transform.position += forward * 10.0f * timer.DeltaTime_f();
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
        auto forward = camera.transform.forward;
        forward.y = 0;
        camera.transform.position -= forward * 10.0f * timer.DeltaTime_f();
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
        auto right = camera.transform.right;
        right.y = 0;
        camera.transform.position += right * 10.0f * timer.DeltaTime_f();
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
        auto right = camera.transform.right;
        right.y = 0;
        camera.transform.position -= right * 10.0f * timer.DeltaTime_f();
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera.transform.position += camera.worldUp * 10.0f * timer.DeltaTime_f();
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        camera.transform.position -= camera.worldUp * 10.0f * timer.DeltaTime_f();
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.transform.rotation.y -= 50.0f * timer.DeltaTime_f();
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.transform.rotation.y += 50.0f * timer.DeltaTime_f();
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_Z) == GLFW_PRESS)
    {
        camera.transform.rotation.x -= 50.0f * timer.DeltaTime_f();
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_C) == GLFW_PRESS)
    {
        camera.transform.rotation.x += 50.0f * timer.DeltaTime_f();
    }

    auto diff = timer.Difference_f();
    Log(diff);
    if (diff < 5.0f)
        camera.gpuData.rayCullDistance -= timer.DeltaTime_f() * 5.0f;
    else if(diff < 15.0f)
        camera.gpuData.rayCullDistance += timer.DeltaTime_f() * 5.0f;

    camera.update();

    //camera.gpuData.view = camera.view;
    //camera.gpuData.projection = camera.projection;
    camera.buffers[imageIndex].AllocatedMap(&camera.gpuData);

    scene.Update(imageIndex, timer.DeltaTime_f());
}

void HybridEngine::render()
{
    if (swapChain.outdated) {
        RecreateSwapChain();
        return;
    }

    if (ImGUI::enabled && widget.enabled) {
        if (widget.NewMainMenu())
        {
            if (widget.NewMenu("File")) {


                widget.EndMenu();
            }
            if (widget.NewMenu("Renderers")) {
                if (widget.MenuItem("Raster", &rasterEnabled)) {
                    raster.commandBuffersReady = false;
                }
                if (widget.MenuItem("Ray Tracing", &raytraceEnabled)) {
                    rayTracing.commandBuffersReady = false;
                }
                widget.EndMenu();
            }

            if (widget.NewMenu("Objects")) {

                widget.MenuItem("Raster Renderer", &raster.widget.enabled);
                widget.MenuItem("Camera", &camera.widget.enabled);
                widget.MenuItem("ShadowMap", &raster.shadowMap.widget.enabled);
                
                widget.EndMenu();
            }

            widget.EndMainMenu();
        }
    }

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];
    vkWaitForFences(core->deviceContext->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(core->deviceContext->logicalDevice, 1, &inFlightFences[currentFrame]);

    std::vector<VkCommandBuffer> submitCommandBuffers;
    submitCommandBuffers.reserve(3);

    if(rasterEnabled)
        raster.GetCommandBuffer(imageIndex, submitCommandBuffers, &camera, &scene);

    if (raytraceEnabled) 
    {
        //rayTracing.updateUniformBuffers(&camera);
        //rayTracing.GetCommandBuffers(imageIndex, submitCommandBuffers, &scene);
    }



    if(ImGUI::enabled)
        raster.GetImGuiCommandBuffer(imageIndex, submitCommandBuffers, swapChain.extent);

    VkSemaphore nis = nextImageSemaphores[currentFrame];
    VkSemaphore ps = presentSemaphores[currentFrame];
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo = Initialisers::submitInfo(
        submitCommandBuffers.data(), static_cast<uint32_t>(submitCommandBuffers.size()), &nis, 1, &ps, 1, waitStages);


    if (vkQueueSubmit(core->deviceContext->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
    auto res = swapChain.Present(presentSemaphores[currentFrame], imageIndex);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        RecreateSwapChain();
    }


    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HybridEngine::RecreateSwapChain() {

    vkQueueWaitIdle(core->deviceContext->presentQueue);
    swapChain.Destroy();
    swapChain.Init();
    imagesInFlight.resize(swapChain.imageCount, VK_NULL_HANDLE);
    raster.Reinitialise();
    rayTracing.Reinitialise();
}

void HybridEngine::Deinitilise()
{
    raster.Deinitialise(true);
    //rayTracing.Deinitialise();

    swapChain.Destroy();


    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(core->deviceContext->logicalDevice, nextImageSemaphores[i], nullptr);
        vkDestroySemaphore(core->deviceContext->logicalDevice, presentSemaphores[i], nullptr);
        vkDestroyFence(core->deviceContext->logicalDevice, inFlightFences[i], nullptr);
    }

    scene.Destroy();

    resources.Destroy();

    core->Deinitialise();
}

void HybridEngine::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_SPACE)
        {

        }

        if (key == GLFW_KEY_F2)
        {
            auto enabled = ImGUI::enabled = !ImGUI::enabled;
            app->widget.enabled = ImGUI::enabled;
            app->raster.commandBuffersReady = false;
            app->rayTracing.commandBuffersReady = false;
            vkQueueWaitIdle(app->core->deviceContext->presentQueue);
        }

        if (key == GLFW_KEY_F3)
        {
            auto enabled = app->rasterEnabled = !app->rasterEnabled;
            app->raster.commandBuffersReady = false;
            app->rayTracing.commandBuffersReady = false;
            //vkQueueWaitIdle(app->core->deviceContext->presentQueue);
        }
        if (key == GLFW_KEY_F4)
        {
            auto enabled = app->raytraceEnabled = !app->raytraceEnabled;
            app->raster.commandBuffersReady = false;
            app->rayTracing.commandBuffersReady = false;
            //vkQueueWaitIdle(app->core->deviceContext->presentQueue);
        }

        if (key == GLFW_KEY_ESCAPE)
            app->window->active = false;
    }
}

void HybridEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));
    app->window->width = width;
    app->window->height = height;
    app->camera.update({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
    app->RecreateSwapChain();
}

void HybridEngine::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));

    app->raster.imgui.leftMouse = button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_RELEASE;
    app->raster.imgui.rightMouse = button == GLFW_MOUSE_BUTTON_RIGHT && action != GLFW_RELEASE;
}

void HybridEngine::scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));
    app->raster.imgui.mouseWheel += static_cast<float>(yOffset) * 0.01f;
}

void HybridEngine::cursorCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));

    app->raster.imgui.mousePos.x = xOffset;
    app->raster.imgui.mousePos.y = yOffset;
}