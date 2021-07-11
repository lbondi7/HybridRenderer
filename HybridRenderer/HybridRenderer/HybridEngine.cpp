#include "HybridEngine.h"

HybridEngine::~HybridEngine()
{
}

void HybridEngine::run()
{

	initialise();

	while (window->isActive() && !quit) {

		prepare();
		update();
		render();
	}

	vkDeviceWaitIdle(core->deviceContext->logicalDevice);
}

void HybridEngine::initialise()
{

    window = std::make_unique<Window>();
    window->init(this);
    window->setupFramebufferResize(HybridEngine::framebufferResizeCallback);
    window->setupKeyCallback(HybridEngine::keyCallback);
    window->setupMouseCallback(HybridEngine::mouseCallback);
    window->setupScrollCallback(HybridEngine::scrollCallback);
    window->setupCursorCallback(HybridEngine::cursorCallback);

    core = std::make_unique<VulkanCore>();
	core->initialise(window->glfwWindow);

    renderer = std::make_unique<RasterRenderer>(window.get(), core.get());

    resources.Init(core->deviceContext.get());

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

  //  descriptorSetManager = new DescriptorSetManager();

    //descriptorSetManager->init(core->deviceContext.get());

    //DescriptorSetManager::initilise(descriptorSetManager);

    renderer->initialise(&resources, nullptr);


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

    gameObjects.reserve(static_cast<uint32_t>(gameObjectCount));

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
        go.Init(core->deviceContext.get());


        if (x >= max)
        {
            z += dist;
            x = -max;
        }
        else {
            x += dist;
        }
    }


    auto imageCount = core->deviceContext->imageCount;

    camera.lookAt = glm::vec3(0, 0, 0);
    camera.transform.position = glm::vec3(0, 0, 10);
    camera.transform.rotation.y = 180.f;

    camera.init(core->deviceContext.get(), renderer->swapChain.extent);

    //for (auto& go : gameObjects) {
    //    go.uniformBuffers.resize(imageCount);

    //    for (size_t i = 0; i < imageCount; i++) {
    //        VkDeviceSize bufferSize = sizeof(ModelUBO);
    //        go.uniformBuffers[i].Create2(core->deviceContext.get(), bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //    }
    //}

    //uniformBuffers.offscreen.resize(swapChain.imageCount);
    lightBuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        VkDeviceSize bufferSize = sizeof(LightUBO);
        //camera.buffers[i].Create2(core->deviceContext.get(), bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        bufferSize = sizeof(LightUBO);
        lightBuffers[i].Create2(core->deviceContext.get(), bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    }


    using BindingType = std::pair<uint32_t, VkDescriptorType>;
    DescriptorSetRequest request;
    request.ids.emplace_back(BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
    request.data.reserve(imageCount);
    for (size_t i = 0; i < imageCount; i++) {

        request.data.push_back(&lightBuffers[i].descriptorInfo);
    }
    core->deviceContext->getDescriptors(lightDescriptor, request);

    //lightDescriptor.initialise(core->deviceContext.get(), request);

    //descriptorSetManager->createDescriptorSets(&lightDescSets, request);


    //request.data.clear();
    //request.data.reserve(imageCount);
    //for (size_t i = 0; i < imageCount; i++) {

    //    request.data.push_back(&camera.buffers[i].descriptorInfo);
    //}
    //descriptorSetManager->createDescriptorSets(&camera.cameraDescSets, request);
    
    //DescriptorSetRequest request2;
    //request2.ids.reserve(2);
    //request2.ids.emplace_back(BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
    //request2.ids.emplace_back(BindingType(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));

    //int i = 0;
    //for (auto& go : gameObjects)
    //{
    //    request2.data.clear();
    //    request2.data.reserve(imageCount);
    //    for (size_t i = 0; i < imageCount; i++) {

    //        request2.data.push_back(&go.uniformBuffers[i].descriptorInfo);
    //        request2.data.push_back(&go.texture->descriptorInfo);
    //    }
    //    descriptorSetManager->createDescriptorSets(&go.descriptorSets, request2);

    //    request.data.clear();
    //    request.data.reserve(imageCount);
    //    for (size_t i = 0; i < imageCount; i++) {

    //        request.data.push_back(&go.uniformBuffers[i].descriptorInfo);
    //    }
    //    descriptorSetManager->createDescriptorSets(&go.offModelDescSets, request);
    //}


}

void HybridEngine::prepare()
{

    renderer->prepare();
    imageIndex = renderer->imageIndex;

}

void HybridEngine::update()
{
    float time = 0.001f;

    if (glfwGetKey(window->glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
        auto forward = camera.transform.forward;
        forward.y = 0;
        camera.transform.position += forward * 10.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
        auto forward = camera.transform.forward;
        forward.y = 0;
        camera.transform.position -= forward * 10.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
        auto right = camera.transform.right;
        right.y = 0;
        camera.transform.position += right * 10.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
        auto right = camera.transform.right;
        right.y = 0;
        camera.transform.position -= right * 10.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera.transform.position += camera.worldUp * 10.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        camera.transform.position -= camera.worldUp * 10.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.transform.rotation.y -= 50.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.transform.rotation.y += 50.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_Z) == GLFW_PRESS)
    {
        camera.transform.rotation.x -= 50.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_C) == GLFW_PRESS)
    {
        camera.transform.rotation.x += 50.0f * time;
    }
    
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_8) == GLFW_PRESS)
    {
            lightPos += camera.transform.forward * 5.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_5) == GLFW_PRESS)
    {
            lightPos -= camera.transform.forward * 5.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_4) == GLFW_PRESS)
    {
            lightPos += camera.transform.right * 5.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_6) == GLFW_PRESS)
    {
            lightPos -= camera.transform.right * 5.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_9) == GLFW_PRESS)
    {
            lightPos.y += 5.0f * time;
    }
    if (glfwGetKey(window->glfwWindow, GLFW_KEY_KP_7) == GLFW_PRESS)
    {
            lightPos.y -= 5.0f * time;
    }

    camera.update(renderer->swapChain.extent);

    float zNear = 1.0f;
    float zFar = 100.0f;

    glm::vec3 lightLookAt = lightPos + glm::vec3(0.0f, -0.5f, 0.5f);

        // Matrix from light's point of view
    glm::mat4 depthProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 depthViewMatrix = glm::mat4(1.0f);
    depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
    depthViewMatrix = glm::lookAt(lightPos, lightLookAt, glm::vec3(0, 1, 0));

    //if (!ortho)
    //{
    //    depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
    //    depthViewMatrix = glm::lookAt(lightPos, lightLookAt, glm::vec3(0, 1, 0));
    //}
    ////depthProjectionMatrix[1][1] *= -1;
    //else
    //{
    //    depthProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, zNear, zFar);
    //    depthViewMatrix = glm::lookAt(lightInvDir, -lightInvDir, glm::vec3(0, 1, 0));
    //}
    glm::mat4 depthModelMatrix = glm::mat4(1.0f);

    //uboOffscreenVS.depthMVP = depthProjectionMatrix * depthViewMatrix *depthModelMatrix;

    lightUBO.depthBiasMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
    lightUBO.lightPos = lightPos;
    lightBuffers[imageIndex].Map2(&lightUBO);

    CameraUBO cameraUBO{};
    cameraUBO.camPos = camera.transform.position;
    cameraUBO.view = camera.view;
    cameraUBO.projection = camera.projection;
    camera.buffers[imageIndex].Map2(&cameraUBO);

    gameObjects[gameObjectCount - 1].transform.position = lightPos;

    for (auto& go : gameObjects)
    {
        go.Update();

        ModelUBO ubos;
        ubos.model = go.model;
        go.uniformBuffers[imageIndex].Map2(&ubos);
    }

}

void HybridEngine::render()
{
    renderer->render(&camera, gameObjects, lightDescriptor);
}

void HybridEngine::deinitilise()
{
    //descriptorSetManager->destroy();

    for (auto& go : gameObjects) {
        go.Destroy();
    }

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

            auto enabled = app->renderer->imgui.enabled = !app->renderer->imgui.enabled;
            if (!enabled)
            {
                app->renderer->commandBuffersReady = false;
                vkQueueWaitIdle(app->core->deviceContext->presentQueue);
            }//app->buildCommandBuffers();
        }

        if (key == GLFW_KEY_ESCAPE)
            app->window->active = false;
    }
}

void HybridEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));
    app->renderer->rebuildSwapChain = true;
}

void HybridEngine::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));

    app->renderer->imgui.leftMouse = button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_RELEASE;
    app->renderer->imgui.rightMouse = button == GLFW_MOUSE_BUTTON_RIGHT && action != GLFW_RELEASE;
}

void HybridEngine::scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));


    //app->imgui.mouseWheel += static_cast<float>(yOffset) * 0.01f;
}

void HybridEngine::cursorCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto app = reinterpret_cast<HybridEngine*>(glfwGetWindowUserPointer(window));

    app->renderer->imgui.mousePos.x = xOffset;
    app->renderer->imgui.mousePos.y = yOffset;


}