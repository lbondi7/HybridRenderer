#include "Scene.h"

#include "DebugLogger.h"
#include "ImGUI_.h"

Scene::~Scene()
{
}

void Scene::Initialise(DeviceContext* deviceContext, Resources* resources)
{
    this->deviceContext = deviceContext;
    this->resources = resources;
    gameObjectCount = 1;
    gameObjects.reserve(10000);

    resources->GetModel("tree2");
    //resources->GetModel("Dragon3");

    topLevelAS.Initialise(deviceContext);

    int treeCount = 100;
    auto sr = std::sqrtf(treeCount);
    float size = 8.0f;
    float x = -size * (sr / 2.0) , z = -size * (sr / 2.0);
    for(int i = 0; i < treeCount; ++i)
    {
        auto go = CreateGameObject(resources->GetModel("tree2"));
        go->name = "Tree Parent 0";
        go->transform.position = glm::vec3(x, 0.0, z);
        go->transform.scale = glm::vec3(1);
        x += size;
        if (x > size * (sr / 2.0))
        {
            x = -size * (sr / 2.0);
            z += size;
        }
    }



    //{
    //    auto go = CreateGameObject(resources->GetModel("tree2"));
    //    go->name = "Tree";
    //}

    //{
    //    auto go = CreateGameObject(resources->GetModel("Dragon3"));
    //    go->transform.position = glm::vec3(0, 1, 0);
    //    go->name = "Dragon";
    //    go->transform.scale = glm::vec3(0.01);
    //}

    //{
    //    auto go = CreateGameObject(resources->GetModel("Cat0.05"));
    //    go->transform.position = glm::vec3(0, 0, 1);
    //    go->name = "Dragon";
    //    go->transform.scale = glm::vec3(0.01);
    //    go->transform.position.y = 0.0;
    //    go->render = false;
    //}

    {
        auto go = CreateGameObject(resources->GetModel("plane"));
        go->transform.scale = glm::vec3(100.0f);
        go->name = "Floor ";
    }


    gameObjectCount = gameObjects.size();

    if (deviceContext->validGPU == 2) {

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
            if (!go.inBVH)
                continue;

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
                        //Log(objDesc.textureIndex, "Texture Index");
                        textureFound = true;
                        break;
                    }
                }

                if (!textureFound)
                {
                    textures.emplace_back(go.texture->descriptorInfo);
                    objDesc.textureIndex = textures.size() - 1;
                    //Log(objDesc.textureIndex, "Texture Index");
                }

                objecDescs.emplace_back(objDesc);
            }
        }

        topLevelAS.Initialise(deviceContext);
        topLevelAS.createTopLevelAccelerationStructure(bottomLevelASs);

        objectBuffer.Create(deviceContext, sizeof(ObjDesc) * objecDescs.size(),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, objecDescs.data());

        auto accelerationStructureInfo = Initialisers::descriptorSetAccelerationStructureInfo(&topLevelAS.handle);
        DescriptorSetRequest accelerationStructureRequest({ {"sceneRQ", 4} }, 3);
        accelerationStructureRequest.AddDescriptorBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_FRAGMENT_BIT);
        accelerationStructureRequest.AddDescriptorBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
        accelerationStructureRequest.AddDescriptorBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<uint32_t>(textures.size()));
        accelerationStructureRequest.AddDescriptorImageData(0, &accelerationStructureInfo);
        accelerationStructureRequest.AddDescriptorImageData(1, &objectBuffer.descriptorInfo);
        accelerationStructureRequest.AddDescriptorImageData(2, textures.data());
        deviceContext->GetDescriptors(asDescriptor, &accelerationStructureRequest);
    }

    auto imageCount = 3;
    lightUBO.position = glm::vec3(50.0f, 20.0f, -50.0f);
    lightUBO.colour.w = 2.0f;
    lightUBO.size_clippingPlanes.z = 1.0f;
    lightUBO.size_clippingPlanes.w = 250.0f;

    lightBuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        VkDeviceSize bufferSize = sizeof(LightUBO);
        lightBuffers[i].Allocate(deviceContext, bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }


    std::string offscreenShader = (deviceContext->validGPU == 2 ? "offscreenRQ" : "offscreen");
    std::string sceneShader = (deviceContext->validGPU == 2 ? "sceneRQ" : "scene");

    DescriptorSetRequest lightRequest({ {sceneShader, 2}, {offscreenShader, 0} }, 1);
    lightRequest.AddDescriptorBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    lightRequest.AddDescriptorBufferData(0, lightBuffers.data());
    deviceContext->GetDescriptors(lightDescriptor, &lightRequest);

    lightWidget.enabled = true;
}

void Scene::Update(uint32_t imageIndex, float dt)
{
    if (ImGUI::enabled && lightWidget.enabled) {

        //if (lightWidget.Button("Dragon")) 
        //{
        //    LoadScene("Dragon");
        //}
        //if (lightWidget.Button("Tree")) 
        //{
        //    LoadScene("Tree");
        //}

        lightWidget.Text("Light");

        lightWidget.Slider3("Position", lightUBO.position, -50.0f, 50.0f);
        if(!ortho)
            lightWidget.Slider("Light FOV", &lightFOV, 1.0f, 180.0f);
        else
            lightWidget.Slider("Fustrum Size", lightUBO.size_clippingPlanes.y, 1.0f, 100.0f);
        lightWidget.Slider("Size", &lightUBO.size_clippingPlanes.x, 0.0f, 5.0f);
        lightWidget.Slider("Near", lightUBO.size_clippingPlanes.z, 0.0f, 5.0f);
        lightWidget.Slider("Far", lightUBO.size_clippingPlanes.w, 5.0f, 500.0f);
        lightWidget.ColourEdit3("Light Colour", lightUBO.colour);
        lightWidget.Slider("Light Intensisty", lightUBO.colour.w, 0.0f, 10.0f);

        if (lightWidget.CheckBox("Orthographic", &ortho)) 
        {
            lightUBO.extra.x = ortho;


            if (ortho) {

                lightUBO.size_clippingPlanes.y = 60.0f;
                lightUBO.size_clippingPlanes.x = 1.0f;
                lightUBO.size_clippingPlanes.z = 5.0f;
                lightUBO.size_clippingPlanes.w = 500.0f;
            }
            else {
                lightFOV = 90.0f;
                lightUBO.size_clippingPlanes.x = 1.0f;
                lightUBO.size_clippingPlanes.z = 1.0f;
                lightUBO.size_clippingPlanes.w = 500.0f;
            }


        }
    }

    //lightPos = glm::vec3(0.0f, 5.0f, -5.0f);
   // lightFOV = 90.0f; 
    glm::vec3 lightLookAt = glm::vec3(0, 1, 0);
    // Matrix from light's point of views
    glm::mat4 depthProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 depthViewMatrix = glm::mat4(1.0f);
    if(lightUBO.extra.x == 0)
        depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, lightUBO.size_clippingPlanes.z, lightUBO.size_clippingPlanes.w);
    else
        depthProjectionMatrix = glm::ortho(-lightUBO.size_clippingPlanes.y, 
        lightUBO.size_clippingPlanes.y, 
        -lightUBO.size_clippingPlanes.y, lightUBO.size_clippingPlanes.y,
        lightUBO.size_clippingPlanes.z, lightUBO.size_clippingPlanes.w);

    depthViewMatrix = glm::lookAt(lightUBO.position, lightLookAt, glm::vec3(0, 1, 0));
    depthProjectionMatrix[1][1] *= -1;

    glm::mat4 depthModelMatrix = glm::yawPitchRoll(lightRot.y, lightRot.x, lightRot.z);

    //lightUBO.depthMVP = depthProjectionMatrix * depthViewMatrix *depthModelMatrix;

    lightUBO.view = depthViewMatrix;
    lightUBO.proj = depthProjectionMatrix;
    lightUBO.direction = glm::normalize(lightLookAt - lightUBO.position);
    //lightUBO.fustrumSize = dt;
    lightBuffers[imageIndex].AllocatedMap(&lightUBO);

    //gameObjects[gameObjectCount - 1].transform.position = lightPos;

    for (auto& go : gameObjects)
    {
        if (!go.mesh)
            continue;

        go.Update();

        ModelUBO ubos;
        ubos.model = go.GetMatrix();
        ubos.colour = glm::vec3(1.0);
        go.uniformBuffers[imageIndex].AllocatedMap(&ubos);
    }

}

void Scene::Destroy()
{

    objectBuffer.Destroy();

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

GameObject* Scene::CreateGameObject(Model* model)
{
    auto& parent = gameObjects.emplace_back();
    if (model->meshes.size() > 1) {
        parent.name = "Parent";
        for (auto& mesh : model->meshes)
        {
            auto& go = gameObjects.emplace_back();
            go.name = mesh->name;
            go.parent = &parent;
            go.mesh = mesh.get();
            go.Init(deviceContext);
            parent.children.emplace_back(&go);
        }
        parent.Init(deviceContext);
    }
    else {
        parent.name = model->meshes[0]->name;
        parent.mesh = model->meshes[0].get();
        parent.Init(deviceContext);
    }
    return &parent;
}

void Scene::LoadScene(const std::string& scene) 
{

    if(scene == "Dragon")
    {
        gameObjects[0].mesh = resources->GetModel("Dragon3")->meshes[0].get();
        //gameObjects[0].SetTexture(resources->GetTexture("white.jpg"));
        gameObjects[1].render = false;
        gameObjects[1].inBVH = false;
        gameObjects[0].transform.scale = glm::vec3(0.1);
    }
    else if (scene == "Tree")
    {
        gameObjects[0].mesh = resources->GetModel("tree2")->meshes[0].get();
        gameObjects[1].mesh = resources->GetModel("tree2")->meshes[1].get();
        gameObjects[0].transform.scale = glm::vec3(1.0);
        gameObjects[1].transform.scale = glm::vec3(1.0);
        gameObjects[1].render = true;
        gameObjects[1].inBVH = true;
        //go->shadowCaster = false;
    }


    gameObjectCount = gameObjects.size();

    if (deviceContext->validGPU == 2) {

        bottomLevelASs.clear();
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
            if (!go.inBVH)
                continue;

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
                        textureFound = true;
                        break;
                    }
                }

                if (!textureFound)
                {
                    if(go.texture)
                        textures.emplace_back(go.texture->descriptorInfo);
                    else
                        textures.emplace_back(resources->GetTexture("white.png")->descriptorInfo);
                    objDesc.textureIndex = textures.size() - 1;
                }

                objecDescs.emplace_back(objDesc);
            }
        }

        topLevelAS.createTopLevelAccelerationStructure(bottomLevelASs);

        objectBuffer.Destroy();

        objectBuffer.Create(deviceContext, sizeof(ObjDesc) * objecDescs.size(),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, objecDescs.data());

        auto accelerationStructureInfo = Initialisers::descriptorSetAccelerationStructureInfo(&topLevelAS.handle);
        DescriptorSetRequest accelerationStructureRequest({ {"sceneRQ", 4} }, 3);
        accelerationStructureRequest.AddDescriptorBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_FRAGMENT_BIT);
        accelerationStructureRequest.AddDescriptorBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
        accelerationStructureRequest.AddDescriptorBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<uint32_t>(textures.size()));
        accelerationStructureRequest.AddDescriptorImageData(0, &accelerationStructureInfo);
        accelerationStructureRequest.AddDescriptorImageData(1, &objectBuffer.descriptorInfo);
        accelerationStructureRequest.AddDescriptorImageData(2, textures.data());
        deviceContext->GetDescriptors(asDescriptor, &accelerationStructureRequest);
    }
}