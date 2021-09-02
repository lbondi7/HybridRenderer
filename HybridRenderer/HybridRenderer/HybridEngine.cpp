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

    rayTracing->initialise(core->deviceContext.get(), core->surface, window.get(), &resources);

    //renderer->initialise(&resources);


    //float value = 0;
    //float dist = 20.0f;
    //if (gameObjectCount < 3)
    //    gameObjectCount = 3;

    //for (size_t i = 0; i < gameObjectCount - 2; i++)
    //{
    //    if (i * i > gameObjectCount - 2)
    //    {
    //        value = i - 1;
    //        break;
    //    }
    //}

    //float max = ((value * dist) / 2.0f);

    //float x = gameObjectCount > 3 ? -max : 0;
    //float z = gameObjectCount > 5 ? -max : 0;

    //gameObjects.reserve(static_cast<uint32_t>(gameObjectCount));

    //for (size_t i = 0; i < gameObjectCount; i++)
    //{
    //    auto& go = gameObjects.emplace_back(GameObject());
    //    if (i < gameObjectCount - 2) {
    //        go.transform.position = glm::vec3(x, -1.0f, z);
    //        go.transform.scale = glm::vec3(5, 5, 5);
    //        go.model = resources.GetModel("tree2");
    //    }
    //    if (i == gameObjectCount - 2)
    //    {
    //        go.transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
    //        go.transform.scale = glm::vec3(max > 5 ? max : 5, 1, max > 5 ? max : 5);
    //        go.model = resources.GetModel("plane");
    //    }
    //    else if (i == gameObjectCount - 1)
    //    {
    //        go.shadowCaster = false;
    //        go.shadowReceiver = false;
    //        go.transform.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    //        go.model = resources.GetModel("sphere");
    //    }
    //    go.texture = resources.GetTexture("texture.jpg");
    //    go.Init(core->deviceContext.get());

    //    if (x >= max)
    //    {
    //        z += dist;
    //        x = -max;
    //    }
    //    else {
    //        x += dist;
    //    }
    //}

    //auto imageCount = core->deviceContext->imageCount;

    //camera.lookAt = glm::vec3(0, 0, 0);
    //camera.transform.position = glm::vec3(0, 0, 10);
    //camera.transform.rotation.y = 180.f;

    //camera.init(core->deviceContext.get(), renderer->swapChain.extent);

    //lightBuffers.resize(imageCount);
    //for (size_t i = 0; i < imageCount; i++) {
    //    VkDeviceSize bufferSize = sizeof(LightUBO);
    //    lightBuffers[i].Create2(core->deviceContext.get(), bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //}

    //using BindingType = std::pair<uint32_t, VkDescriptorType>;
    //DescriptorSetRequest request;
    //request.ids.emplace_back(BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
    //request.data.reserve(imageCount);
    //for (size_t i = 0; i < imageCount; i++) {

    //    request.data.push_back(&lightBuffers[i].descriptorInfo);
    //}
    //core->deviceContext->getDescriptors(lightDescriptor, request);
}

void HybridEngine::prepare()
{

    //renderer->prepare();
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
    //
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

    //camera.update({static_cast<uint32_t>(window->width), static_cast<uint32_t>(window->height)});

    //float zNear = 1.0f;
    //float zFar = 100.0f;

    //glm::vec3 lightLookAt = glm::vec3(0, 0, 0);
    //    // Matrix from light's point of view
    //glm::mat4 depthProjectionMatrix = glm::mat4(1.0f);
    //glm::mat4 depthViewMatrix = glm::mat4(1.0f);
    //depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
    //depthViewMatrix = glm::lookAt(lightPos, lightLookAt, glm::vec3(0, 1, 0));
    //depthProjectionMatrix[1][1] *= -1;

    ////if (!ortho)
    ////{
    ////    depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
    ////    depthViewMatrix = glm::lookAt(lightPos, lightLookAt, glm::vec3(0, 1, 0));
    ////}
    //////depthProjectionMatrix[1][1] *= -1;
    ////else
    ////{
    ////    depthProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, zNear, zFar);
    ////    depthViewMatrix = glm::lookAt(lightInvDir, -lightInvDir, glm::vec3(0, 1, 0));
    ////}
    //glm::mat4 depthModelMatrix = glm::yawPitchRoll(lightRot.y, lightRot.x, lightRot.z);

    ////uboOffscreenVS.depthMVP = depthProjectionMatrix * depthViewMatrix *depthModelMatrix;

    //lightUBO.depthBiasMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
    //lightUBO.lightPos = lightPos;
    //lightBuffers[imageIndex].Map2(&lightUBO);

    //CameraUBO cameraUBO{};
    //cameraUBO.camPos = camera.transform.position;
    //cameraUBO.view = camera.view;
    //cameraUBO.projection = camera.projection;
    //camera.buffers[imageIndex].Map2(&cameraUBO);

    //gameObjects[gameObjectCount - 1].transform.position = lightPos;

    //for (auto& go : gameObjects)
    //{
    //    go.Update();

    //    ModelUBO ubos;
    //    ubos.model = go.modelMatrix;
    //    go.uniformBuffers[imageIndex].Map2(&ubos);
    //}

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

    //renderer->render(&camera, gameObjects, lightDescriptor);

    rayTracing->render();
}

void HybridEngine::deinitilise()
{
   // renderer->cleanup();

    rayTracing->cleanup();

    for (auto& go : gameObjects) {
        go.Destroy();
    }

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
            if (!enabled)
            {
                app->renderer->commandBuffersReady = false;
                vkQueueWaitIdle(app->core->deviceContext->presentQueue);
            }
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

   // app->renderer->imgui.leftMouse = button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_RELEASE;
   // app->renderer->imgui.rightMouse = button == GLFW_MOUSE_BUTTON_RIGHT && action != GLFW_RELEASE;
}

void HybridEngine::scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));
    //app->imgui.mouseWheel += static_cast<float>(yOffset) * 0.01f;
}

void HybridEngine::cursorCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));

   // app->renderer->imgui.mousePos.x = xOffset;
   // app->renderer->imgui.mousePos.y = yOffset;
}