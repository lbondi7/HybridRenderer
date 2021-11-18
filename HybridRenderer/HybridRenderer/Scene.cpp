#include "Scene.h"

#include "DebugLogger.h"
#include "ImGUI_.h"

Scene::~Scene()
{
}

void Scene::Initialise(DeviceContext* deviceContext, Resources* resources)
{
    this->deviceContext = deviceContext;

    gameObjectCount = 50;
    gameObjects.reserve(10000);
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
        CreateGameObject(&gameObjects[i], resources->GetModel("tree2"));
        gameObjects[i].Init(deviceContext);
        gameObjects[i].name = "Tree Parent " + i;
        gameObjects[i].transform.position = glm::vec3(x, 0, z);

        if (x >= max)
        {
            z += dist;
            x = -max;
        }
        else {
            x += dist;
        }
    }

    {
        auto& go = gameObjects.emplace_back();
        CreateGameObject(&go, resources->GetModel("plane"));
        go.transform.scale = glm::vec3(100.0f);
        go.SetTexture(resources->GetTexture("white3.png"));
        go.name = "Floor ";
    }

    {
        auto& go = gameObjects.emplace_back();
        CreateGameObject(&go, resources->GetModel("plane"));
        go.transform.rotation.y = 90.0f;
        go.transform.position.z = -2.0f;
        go.transform.position.y = 1.0f;
        go.SetTexture(resources->GetTexture("amogus.png"));
        go.name = "Amogus";
    }

    {
        auto& go = gameObjects.emplace_back();
        CreateGameObject(&go, resources->GetModel("plane"));
        go.transform.rotation.y = 90.0f;
        go.transform.position.z = -3.0f;
        go.transform.position.y = 2.0f;
        go.SetTexture(resources->GetTexture("amogus.png"));
        go.name = "Amogus";
    }

    gameObjectCount = gameObjects.size();

    struct ObjDesc {
        int textureIndex;
        uint64_t verticesAddress;
        uint64_t indicesAddress;
    };

    std::vector<ObjDesc> objecDescs;
    std::vector<uint32_t> textureIDs;
    std::vector<VkDescriptorImageInfo> textures;
    objecDescs.reserve(gameObjects.size());
    textures.reserve(gameObjects.size());
    bottomLevelASs.reserve(gameObjects.size());
    for (auto& go : gameObjects)
    {
        if (go.mesh) {
            ObjDesc objDesc;
            objDesc.verticesAddress = go.mesh->vertexBuffer.GetDeviceAddress();
            objDesc.indicesAddress = go.mesh->indexBuffer.GetDeviceAddress();
            AccelerationStructure blas;
            blas.Initialise(deviceContext);
            blas.createBottomLevelAccelerationStructure(go);
            bottomLevelASs.emplace_back(blas);
            bool textureFound = false;
            for (size_t i = 0; i < textures.size(); ++i)
            {
                if (textures[i].imageLayout == go.texture->descriptorInfo.imageLayout &&
                    textures[i].imageView == go.texture->descriptorInfo.imageView &&
                    textures[i].sampler == go.texture->descriptorInfo.sampler)
                {
                    objDesc.textureIndex = i;
                    Log(objDesc.textureIndex, "Texture Index");
                    textureFound = true;
                    break;
                }
            }

            if (!textureFound) 
            {
                textures.emplace_back(go.texture->descriptorInfo);
                objDesc.textureIndex = textures.size() - 1;
                Log(objDesc.textureIndex, "Texture Index");
            }

            objecDescs.emplace_back(objDesc);
        }
    }

    topLevelAS.Initialise(deviceContext);
    topLevelAS.createTopLevelAccelerationStructure(bottomLevelASs);

    Buffer objectBuffer;

    objectBuffer.Create(deviceContext, sizeof(ObjDesc) * objecDescs.size(),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, objecDescs.data());

    auto accelerationStructureInfo = Initialisers::descriptorSetAccelerationStructureInfo(&topLevelAS.handle);
    DescriptorSetRequest accelerationStructureRequest({ {"scene", 4} }, 3);
    accelerationStructureRequest.AddDescriptorBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_FRAGMENT_BIT);
    accelerationStructureRequest.AddDescriptorBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
    accelerationStructureRequest.AddDescriptorBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<uint32_t>(textures.size()));
    accelerationStructureRequest.AddDescriptorImageData(0, &accelerationStructureInfo);
    accelerationStructureRequest.AddDescriptorImageData(1, &objectBuffer.descriptorInfo);
    accelerationStructureRequest.AddDescriptorImageData(2, textures.data());
    deviceContext->GetDescriptors(asDescriptor, &accelerationStructureRequest);

    auto imageCount = 3;
    lightPos = glm::vec3(0.0f, 50.0f, -0.0f);

    lightBuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        VkDeviceSize bufferSize = sizeof(LightUBO);
        lightBuffers[i].Allocate(deviceContext, bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    DescriptorSetRequest lightRequest({ {"scene", 2}, {"offscreen", 0} }, 1);
    lightRequest.AddDescriptorBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    lightRequest.AddDescriptorBufferData(0, lightBuffers.data());
    deviceContext->GetDescriptors(lightDescriptor, &lightRequest);

    lightWidget.enabled = true;
    
}

void Scene::Update(uint32_t imageIndex, float dt)
{
    if (ImGUI::enabled && lightWidget.enabled) {
        if (lightWidget.NewWindow("Light")) {

            lightWidget.Slider3("Position", lightUBO.position, -75.0f, 75.0f);
            lightWidget.Slider3("Direction", lightUBO.direction, -2.0f, 2.0f);
            lightWidget.Slider("Size", &lightUBO.size, 0.0f, 3.0f);
            lightWidget.Slider2("Clipping Planes", lightUBO.clippingPlanes, 0.0f, 100.0f);
            lightWidget.CheckBox("Focus On Centre", &lookAtCentre);
        }
        lightWidget.EndWindow();
    }



    //lightPos = glm::vec3(0.0f, 5.0f, -5.0f);
    lightFOV = 90.0f;
    glm::vec3 lightLookAt = glm::vec3(0, 1, 1);
    // Matrix from light's point of view
    glm::mat4 depthProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 depthViewMatrix = glm::mat4(1.0f);
    //depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
    depthProjectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f,  0.1f, lightUBO.clippingPlanes.y);
    depthViewMatrix = glm::lookAt(lightUBO.position, lookAtCentre ? lightLookAt : lightUBO.position + lightUBO.direction, glm::vec3(0, 1, 0));
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

    //lightUBO.depthMVP = depthProjectionMatrix * depthViewMatrix *depthModelMatrix;

    lightUBO.view = depthViewMatrix;
    lightUBO.proj = depthProjectionMatrix * depthViewMatrix;
    lightUBO.direction = lookAtCentre ? lightLookAt - lightUBO.position : lightUBO.direction;
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

void Scene::KeyCallback(int key, int scancode, int action, int mods)
{
    switch (action)
    {
    case GLFW_PRESS:
    {
        switch (key)
        {
        case GLFW_KEY_KP_4:
            //lightPos.x -= 2.0;
            break;
        case GLFW_KEY_KP_6:
           // lightPos.x += 2.0;
            break;
        case GLFW_KEY_KP_5:
           // lightPos.z -= 2.0;
            break;
        case GLFW_KEY_KP_8:
          //  lightPos.z += 2.0;
            break;
        case GLFW_KEY_KP_ADD:
           // lightPos.y -= 2.0;
            break;
        case GLFW_KEY_KP_SUBTRACT:
           // lightPos.y += 2.0;
            break;
        }

        break;
    }
    case GLFW_RELEASE:
    {
        switch (key)
        {
        case GLFW_KEY_UP:
            break;
        case GLFW_KEY_DOWN:
            break;
        case GLFW_KEY_LEFT:
            break;
        case GLFW_KEY_RIGHT:
            break;
        }
        break;
    }
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
