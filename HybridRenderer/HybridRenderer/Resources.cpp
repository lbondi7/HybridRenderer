#include "Resources.h"

#include "Initilizers.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Resources::~Resources()
{
    devices = nullptr;
}

void Resources::Init(DeviceContext* _devices)
{
    devices = _devices;
}

void Resources::Destroy()
{

    for (auto& mesh : meshes)
    {
        mesh.second->Destroy();
    }

    for (auto& model : models)
    {
        model.second->Destroy();
    }

    for (auto& texture : textures)
    {
        texture.second->Destroy();
    }

    for (auto& shader : vertexShaders)
    {
        shader.second->Destroy();
    }


    for (auto& shader : fragmentShaders)
    {
        shader.second->Destroy();
    }

    for (auto& shader : raygenShaders)
    {
        shader.second->Destroy();
    }


    for (auto& shader : raymissShaders)
    {
        shader.second->Destroy();
    }

    for (auto& shader : rayclosesthitShaders)
    {
        shader.second->Destroy();
    }

}

void Resources::GetShaders(std::vector<Shader*>& shaders, const std::vector<std::string>& shaderNames)
{
    size_t reserveSize = 0;
    for (auto& name : shaderNames) {
        if (vertexShaders.contains(name))
            reserveSize++;
        if (fragmentShaders.contains(name))
            reserveSize++;
    }
    shaders.resize(reserveSize);
    size_t i = 0;
    for (auto& name : shaderNames) {
        if (vertexShaders.contains(name))
        {
            shaders[i] = vertexShaders[name].get();
            i++;
        }
        if (fragmentShaders.contains(name))
        {
            shaders[i] = fragmentShaders[name].get();
            i++;
        }
    }
}

Shader* Resources::GetShader(const std::string& shaderName, VkShaderStageFlagBits shaderStage)
{
    auto trimmedName = trimName(shaderName);
    switch (shaderStage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:
        if (vertexShaders.contains(trimmedName)) {
            return vertexShaders[trimmedName].get();
        }

        vertexShaders.emplace(trimmedName, std::make_unique<Shader>());
        InitShader(shaderName, vertexShaders[trimmedName].get(), shaderStage);
        return vertexShaders[trimmedName].get();
        break;
    case VK_SHADER_STAGE_GEOMETRY_BIT:
        break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        if (fragmentShaders.contains(trimmedName)) {
            return fragmentShaders[trimmedName].get();
        }

        fragmentShaders.emplace(trimmedName, std::make_unique<Shader>());
        InitShader(shaderName, fragmentShaders[trimmedName].get(), shaderStage);
        return fragmentShaders[trimmedName].get();
        break;
    case VK_SHADER_STAGE_COMPUTE_BIT:
        break;
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        if (raygenShaders.contains(trimmedName)) {
            return raygenShaders[trimmedName].get();
        }

        raygenShaders.emplace(trimmedName, std::make_unique<Shader>());
        InitShader(shaderName, raygenShaders[trimmedName].get(), shaderStage);
        return raygenShaders[trimmedName].get();
        break;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        break;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        if (rayclosesthitShaders.contains(trimmedName)) {
            return rayclosesthitShaders[trimmedName].get();
        }

        rayclosesthitShaders.emplace(trimmedName, std::make_unique<Shader>());
        InitShader(shaderName, rayclosesthitShaders[trimmedName].get(), shaderStage);
        return rayclosesthitShaders[trimmedName].get();
        break;
    case VK_SHADER_STAGE_MISS_BIT_KHR:
        if (raymissShaders.contains(trimmedName)) {
            return raymissShaders[trimmedName].get();
        }

        raymissShaders.emplace(trimmedName, std::make_unique<Shader>());
        InitShader(shaderName, raymissShaders[trimmedName].get(), shaderStage);
        return raymissShaders[trimmedName].get();
        break;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        break;
    case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
        break;
    default:
        break;
    }
}

void Resources::LoadMesh(const std::string& name)
{
    meshes.emplace(name, std::make_unique<Mesh>());

    auto& mesh = meshes[name];

    mesh->name = name;
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, ("models/"+ name + "/"+ name + ".obj").c_str())) {
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, ("models/" + name + ".obj").c_str())) {
            throw std::runtime_error(warn + err);
        }
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};


    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(mesh->vertices.size());
                mesh->vertices.push_back(vertex);
            }

            mesh->indices.push_back(uniqueVertices[vertex]);
        }
    }

    mesh->Init(devices);
}


Model* Resources::GetModel(const std::string& name) {
    if (models.contains(name)) {
        return models[name].get();
    }

    LoadModel(name);
    return models[name].get();
}


void Resources::LoadModel(const std::string& name)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, ("models/" + name + "/" + name + ".obj").c_str())) {
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, ("models/" + name + ".obj").c_str())) {
            throw std::runtime_error(warn + err);
        }
    }
 
    models.emplace(name, std::make_unique<Model>());

    auto model = models[name].get();

    model->max = glm::vec3(-FLT_MAX);
    model->min = glm::vec3(FLT_MAX);

    models[name]->name = name;
    models[name]->meshes.resize(shapes.size());
    size_t id = 0;
    for (const auto& shape : shapes) {
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        models[name]->meshes[id] = std::make_unique<Mesh>();
        models[name]->meshes[id]->name = shape.name;
        
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            model->min.x = std::min(model->min.x, vertex.pos.x);
            model->min.y = std::min(model->min.y, vertex.pos.y);
            model->min.z = std::min(model->min.z, vertex.pos.z);
            model->max.x = std::max(model->max.x, vertex.pos.x);
            model->max.y = std::max(model->max.y, vertex.pos.y);
            model->max.z = std::max(model->max.z, vertex.pos.z);

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(models[name]->meshes[id]->vertices.size());
                models[name]->meshes[id]->vertices.push_back(vertex);
            }

            models[name]->meshes[id]->indices.push_back(uniqueVertices[vertex]);
        }

        auto materialId = shape.mesh.material_ids[0];
        if (materials.size() > 1 && materialId != -1)
        {
            //LoadTexture(name + "/"+ materials[materialId].diffuse_texname);
            //models[name]->meshes[id]->texture = textures[trimName(materials[materialId].diffuse_texname)].get();


            models[name]->meshes[id]->texture = GetTexture(name + "/" + materials[materialId].diffuse_texname);

        }
        else {
            //models[name]->meshes[id]->texture = textures["texture"].get();
            models[name]->meshes[id]->texture = GetTexture("texture.jpg");
        }
        models[name]->meshes[id]->Init(devices);

        id++;
    }
}

TextureSampler* Resources::GetTexture(const std::string& name) {
    auto trimmedName = trimName(name);
    if (textures.contains(trimmedName)) {
        return textures[trimmedName].get();
    }
    LoadTexture(name);
    return textures[trimmedName].get();
}

void Resources::LoadTexture(const std::string& name)
{
    int texWidth, texHeight, texChannels;
    auto trimmedName = trimName(name);
    stbi_uc* pixels = stbi_load(("textures/" + name).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    Buffer stagingBuffer;

    stagingBuffer.Create(devices, imageSize, pixels);

    stbi_image_free(pixels);

    textures.emplace(trimmedName, std::make_unique<TextureSampler>());

    auto& texture = textures[trimmedName];

    texture->Create(devices, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    texture->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT);
    texture->CopyFromBuffer(stagingBuffer.vkBuffer);
    texture->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    texture->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    texture->createSampler();
    texture->descriptorInfo = Initialisers::descriptorImageInfo(texture->imageView, texture->sampler);
    stagingBuffer.Destroy();
}

void Resources::LoadShader(const std::string& name, VkShaderStageFlagBits stage)
{
    auto trimmedName = trimName(name);
    switch (stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:
        vertexShaders.emplace(trimmedName, std::make_unique<Shader>());
        InitShader(name, vertexShaders[trimmedName].get(), stage);
        break;
    case VK_SHADER_STAGE_GEOMETRY_BIT:
        break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        fragmentShaders.emplace(trimmedName, std::make_unique<Shader>());
        InitShader(name, fragmentShaders[trimmedName].get(), stage);
        break;
    case VK_SHADER_STAGE_COMPUTE_BIT:
        break;
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        raygenShaders.emplace(trimmedName, std::make_unique<Shader>());
        InitShader(name, raygenShaders[trimmedName].get(), stage);
        break;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        break;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        rayclosesthitShaders.emplace(trimmedName, std::make_unique<Shader>());
        InitShader(name, rayclosesthitShaders[trimmedName].get(), stage);
        break;
    case VK_SHADER_STAGE_MISS_BIT_KHR:
        raymissShaders.emplace(trimmedName, std::make_unique<Shader>());
        InitShader(name, raymissShaders[trimmedName].get(), stage);
        break;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        break;
    case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
        break;
    default:
        break;
    }

}

void Resources::InitShader(const std::string& name, Shader* shader, VkShaderStageFlagBits stage)
{
    shader->Init(devices, name, stage);
}

const std::string Resources::trimName(const std::string& name)
{
    std::string newName="";
    for (auto letter : name)
    {
        if (letter == '.')
            break;

        newName += letter;
        if (letter == '\\' || letter == '/')
            newName.clear();

    }


    return newName;
}


