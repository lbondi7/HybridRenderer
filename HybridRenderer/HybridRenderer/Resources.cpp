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

void Resources::Init(Device* _devices)
{
    devices = _devices;
}

void Resources::Destroy()
{

    for (auto& mesh : meshes)
    {
        mesh.second->Destroy();
    }

    for (auto& image : images)
    {
        image.second->texture.Destroy();
    }

    for (auto& shader : vertexShaders)
    {
        shader.second->Destroy();
    }


    for (auto& shader : fragmentShaders)
    {
        shader.second->Destroy();
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

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, ("models/"+ name + ".obj").c_str())) {
        throw std::runtime_error(warn + err);
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

void Resources::LoadImage(const std::string& name)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(("textures/" + name + ".jpg").c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    Buffer stagingBuffer;

    stagingBuffer.Create(devices, imageSize, pixels);

    stbi_image_free(pixels);

    images.emplace(name, std::make_unique<Image>());

    auto& image = images[name];
    image->width = texWidth;
    image->height = texHeight;
    image->channels = texChannels;

    image->texture.Create(devices, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    image->texture.transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    image->texture.CopyFromBuffer(stagingBuffer.vkBuffer);
    image->texture.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image->texture.createImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    image->texture.createSampler();
    image->texture.descriptorInfo = Initialisers::descriptorImageInfo(image->texture.imageView, image->texture.sampler);
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
        break;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        break;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        break;
    case VK_SHADER_STAGE_MISS_BIT_KHR:
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
        newName += letter;
        if (letter == '\\' || letter == '/')
            newName.clear();

    }


    return newName;
}


