#include "Scene.h"

Scene::~Scene()
{
}

void Scene::Initialise(DeviceContext* deviceContext, Resources* resources)
{
    this->deviceContext = deviceContext;

    gameObjectCount = 400;
    gameObjects.reserve(gameObjectCount * 10);
    gameObjects.resize(static_cast<uint32_t>(gameObjectCount));

    float value = 0;
    float dist = 10.0f;

    for (size_t i = 0; i < gameObjectCount - 1; i++)
    {
        if (i * i > gameObjectCount - 1)
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
        if (i == 0)
        {
            CreateGameObject(&gameObjects[i], resources->GetModel("plane"));
            gameObjects[i].transform.scale = glm::vec3(100.0f);
            gameObjects[i].SetTexture(resources->GetTexture("white.jpg"));
            gameObjects[i].name = "Floor " + i;
            continue;
        }
        else {
            CreateGameObject(&gameObjects[i], resources->GetModel("tree2"));
            gameObjects[i].Init(deviceContext);
            gameObjects[i].name = "Tree Parent " + i;
            gameObjects[i].transform.position = glm::vec3(x, 0, z);
        }
        if (x >= max)
        {
            z += dist;
            x = -max;
        }
        else {
            x += dist;
        }
    }

    gameObjectCount = gameObjects.size();

    for (auto& go : gameObjects)
    {
        if (go.mesh) {
            AccelerationStructure blas;
            blas.Initialise(deviceContext);
            blas.createBottomLevelAccelerationStructure(go);
            bottomLevelASs.push_back(blas);
        }
    }

    topLevelAS.Initialise(deviceContext);
    topLevelAS.createTopLevelAccelerationStructure(bottomLevelASs);

    DescriptorSetRequest request;
    auto imageCount = 3;
    request.ids.emplace_back(DescriptorSetRequest::BindingType(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_FRAGMENT_BIT));
    request.data.reserve(imageCount);
    auto accelerationStructureInfo = Initialisers::descriptorSetAccelerationStructureInfo(&topLevelAS.handle);
    for (size_t i = 0; i < imageCount; i++) {

        request.data.push_back(&accelerationStructureInfo);
    }
    deviceContext->GetDescriptors(asDescriptor, &request);

    request.ids.clear();
    request.ids.emplace_back(DescriptorSetRequest::BindingType(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR));

    deviceContext->GetDescriptors(rtASDescriptor, &request);

    request.data.clear();
    request.ids.clear();

    lightBuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        VkDeviceSize bufferSize = sizeof(LightUBO);
        lightBuffers[i].Allocate(deviceContext, bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    request.ids.emplace_back(DescriptorSetRequest::BindingType(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT));
    request.data.reserve(imageCount);
    for (size_t i = 0; i < imageCount; i++) {

        request.data.push_back(&lightBuffers[i].descriptorInfo);
    }
    deviceContext->GetDescriptors(lightDescriptor, &request);

}

void Scene::Update(uint32_t imageIndex, float dt)
{
    float zNear = 1.0f;
    float zFar = 1000.0f;

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
        if (!go.mesh)
            continue;

        go.Update();

        ModelUBO ubos;
        ubos.model = go.GetMatrix();
        go.uniformBuffers[imageIndex].AllocatedMap(&ubos);
    }

}

void Scene::Destroy()
{

    for (auto& blas : bottomLevelASs)
    {
        blas.Destroy();
    }

    topLevelAS.Destroy();

    for(auto& lightBuffer : lightBuffers)
    {
        lightBuffer.Destroy();
    }

    for (auto& go : gameObjects) {
        go.Destroy();
    }
}

void Scene::CreateGameObject(GameObject* object, Model* model)
{
    if (model->meshes.size() > 1) {
        for (auto& mesh : model->meshes)
        {
            GameObject go;
            go.name = mesh->name;
            go.parent = object;
            go.mesh = mesh.get();
            go.Init(deviceContext);
            gameObjects.emplace_back(go);
            object->children.emplace_back(&go);
        }
    }
    else {
        object->name = model->meshes[0]->name;
        object->mesh = model->meshes[0].get();
        object->Init(deviceContext);
    }
}
