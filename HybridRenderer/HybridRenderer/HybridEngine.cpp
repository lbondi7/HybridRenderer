#include "HybridEngine.h"

#include "ImGUI_.h"

HybridEngine::~HybridEngine()
{
}

void HybridEngine::run()
{

	initialise();

	while (window->isActive() && !quit) {

        window->resize();
        timer.update();
		prepare();
		update();
		render();
	}

	vkDeviceWaitIdle(core->deviceContext->logicalDevice);

    deinitilise();
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

    //renderer = std::make_unique<RasterRenderer>(window.get(), core.get());
    rayTracing = std::make_unique<RayTracingRenderer>();

    resources.Init(core->deviceContext.get());

    resources.LoadTexture("texture.jpg");

    scene.Initialise(core->deviceContext.get(), &resources);

    rayTracing->Initialise(core->deviceContext.get(), core->surface, window.get(), &resources, &scene);

    //renderer->Initialise(&resources);

    auto imageCount = core->deviceContext->imageCount;

    camera.lookAt = glm::vec3(0, 0, 0);
    camera.transform.position = glm::vec3(0, 0, 10);
    camera.transform.rotation.y = 180.f;

    //camera.init(core->deviceContext.get(), renderer->swapChain.extent);
    camera.init(core->deviceContext.get(), rayTracing->swapChain.extent);
}

void HybridEngine::prepare()
{

    //renderer->Prepare();
    //imageIndex = renderer->imageIndex;

}

void HybridEngine::update()
{
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
        auto forward = camera.transform.forward;
        forward.y = 0;
        camera.transform.position += forward * 10.0f * timer.dt;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
        auto forward = camera.transform.forward;
        forward.y = 0;
        camera.transform.position -= forward * 10.0f * timer.dt;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
        auto right = camera.transform.right;
        right.y = 0;
        camera.transform.position += right * 10.0f * timer.dt;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
        auto right = camera.transform.right;
        right.y = 0;
        camera.transform.position -= right * 10.0f * timer.dt;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera.transform.position += camera.worldUp * 10.0f * timer.dt;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        camera.transform.position -= camera.worldUp * 10.0f * timer.dt;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.transform.rotation.y -= 50.0f * timer.dt;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.transform.rotation.y += 50.0f * timer.dt;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_Z) == GLFW_PRESS)
    {
        camera.transform.rotation.x -= 50.0f * timer.dt;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_C) == GLFW_PRESS)
    {
        camera.transform.rotation.x += 50.0f * timer.dt;
    }
    
    //if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_8) == GLFW_PRESS)
    //{
    //        lightPos += camera.transform.forward * 5.0f * timer.dt;
    //}
    //if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_5) == GLFW_PRESS)
    //{
    //        lightPos -= camera.transform.forward * 5.0f * timer.dt;
    //}
    //if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_4) == GLFW_PRESS)
    //{
    //        lightPos += camera.transform.right * 5.0f * timer.dt;
    //}
    //if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_6) == GLFW_PRESS)
    //{
    //        lightPos -= camera.transform.right * 5.0f * timer.dt;
    //}
    //if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_9) == GLFW_PRESS)
    //{
    //        lightPos.y += 5.0f * timer.dt;
    //}
    //if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_7) == GLFW_PRESS)
    //{
    //        lightPos.y -= 5.0f * timer.dt;
    //}
    //if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_1) == GLFW_PRESS)
    //{
    //    lightRot.y -= 5.0f * timer.dt;
    //}
    //if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_3) == GLFW_PRESS)
    //{
    //    lightRot.y -= 5.0f * timer.dt;
    //}

    camera.update({static_cast<uint32_t>(window->width), static_cast<uint32_t>(window->height)});

    CameraUBO cameraUBO{};
    cameraUBO.camPos = camera.transform.position;
    cameraUBO.view = camera.view;
    cameraUBO.projection = camera.projection;
    camera.buffers[imageIndex].AllocatedMap(&cameraUBO);

    scene.Update(imageIndex, timer.dt);
}

void HybridEngine::render()
{
    //if (ImGUI::enabled && widget.enabled) {
    //    if (widget.NewMainMenu())
    //    {
    //        if (widget.NewMenu("File")) {


    //            widget.EndMenu();
    //        }

    //        if (widget.NewMenu("Objects")) {
    //            widget.MenuItem("Camera", &camera.widget.enabled);
    //            widget.MenuItem("ShadowMap", &renderer->shadowMap.widget.enabled);

    //            widget.EndMenu();
    //        }

    //        widget.EndMainMenu();
    //    }
    //}

    //renderer->Render(&camera, &scene);

    rayTracing->Render(&camera);
}

void HybridEngine::deinitilise()
{
    //renderer->cleanup();
    rayTracing->cleanup();

    scene.Destroy();

    resources.Destroy();

    core->deinitialise();
}

void HybridEngine::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_SPACE)
        {
            //Log(app->lightPos, "Light Pos");
            //app->ortho = !app->ortho;
        }

        if (key == GLFW_KEY_F2)
        {
            //Log(app->lightPos, "Light Pos");
            //app->ortho = !app->ortho;

            auto enabled = ImGUI::enabled = !ImGUI::enabled;
            app->widget.enabled = ImGUI::enabled;
            app->renderer->commandBuffersReady = false;
            vkQueueWaitIdle(app->core->deviceContext->presentQueue);
        }

        if (key == GLFW_KEY_ESCAPE)
            app->window->active = false;
    }
}

void HybridEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));
    //app->renderer->rebuildSwapChain = true;
    app->window->width = width;
    app->window->height = height;
    app->camera.update({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
}

void HybridEngine::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));

    app->renderer->imgui.leftMouse = button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_RELEASE;
    app->renderer->imgui.rightMouse = button == GLFW_MOUSE_BUTTON_RIGHT && action != GLFW_RELEASE;
}

void HybridEngine::scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));
    app->renderer->imgui.mouseWheel += static_cast<float>(yOffset) * 0.01f;
}

void HybridEngine::cursorCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));

    //app->renderer->imgui.mousePos.x = xOffset;
    //app->renderer->imgui.mousePos.y = yOffset;
}