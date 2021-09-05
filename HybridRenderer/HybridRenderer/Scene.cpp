#include "Scene.h"

Scene::~Scene()
{
}

void Scene::Initialise(DeviceContext* deviceContext, Resources* resources)
{
    this->deviceContext = deviceContext;

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
    //        go.model = resources->GetModel("tree2");
    //    }
    //    if (i == gameObjectCount - 2)
    //    {
    //        go.transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
    //        go.transform.scale = glm::vec3(max > 5 ? max : 5, 1, max > 5 ? max : 5);
    //        go.model = resources->GetModel("plane");
    //    }
    //    else if (i == gameObjectCount - 1)
    //    {
    //        go.shadowCaster = false;
    //        go.shadowReceiver = false;
    //        go.transform.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    //        go.model = resources->GetModel("sphere");
    //    }
    //    go.texture = resources->GetTexture("texture.jpg");
    //    go.Init(deviceContext);

    //    if (x >= max)
    //    {
    //        z += dist;
    //        x = -max;
    //    }
    //    else {
    //        x += dist;
    //    }
    //}


    gameObjects.reserve(static_cast<uint32_t>(gameObjectCount));

    for (size_t i = 0; i < gameObjectCount; i++)
    {
        auto& go = gameObjects.emplace_back(GameObject());
        go.model = resources->GetModel("tree2");
        go.Init(deviceContext);
    }

    auto imageCount = 3;
    lightBuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        VkDeviceSize bufferSize = sizeof(LightUBO);
        lightBuffers[i].Allocate(deviceContext, bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    using BindingType = std::pair<uint32_t, VkDescriptorType>;
    DescriptorSetRequest request;
    request.ids.emplace_back(BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
    request.data.reserve(imageCount);
    for (size_t i = 0; i < imageCount; i++) {

        request.data.push_back(&lightBuffers[i].descriptorInfo);
    }
    deviceContext->GetDescriptors(lightDescriptor, request);
}

void Scene::Update(uint32_t imageIndex, float dt)
{
    float zNear = 1.0f;
    float zFar = 100.0f;

    glm::vec3 lightLookAt = glm::vec3(0, 0, 0);
    // Matrix from light's point of view
    glm::mat4 depthProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 depthViewMatrix = glm::mat4(1.0f);
    depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
    depthViewMatrix = glm::lookAt(lightPos, lightLookAt, glm::vec3(0, 1, 0));
    depthProjectionMatrix[1][1] *= -1;

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
    glm::mat4 depthModelMatrix = glm::yawPitchRoll(lightRot.y, lightRot.x, lightRot.z);

    //uboOffscreenVS.depthMVP = depthProjectionMatrix * depthViewMatrix *depthModelMatrix;

    lightUBO.depthBiasMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
    lightUBO.lightPos = lightPos;
    lightBuffers[imageIndex].AllocatedMap(&lightUBO);

    //gameObjects[gameObjectCount - 1].transform.position = lightPos;

    for (auto& go : gameObjects)
    {
        go.Update();

        ModelUBO ubos;
        ubos.model = go.modelMatrix;
        go.uniformBuffers[imageIndex].AllocatedMap(&ubos);
    }

}

void Scene::Destroy()
{

    for(auto& lightBuffer : lightBuffers)
    {
        lightBuffer.Destroy();
    }

    for (auto& go : gameObjects) {
        go.Destroy();
    }
}
