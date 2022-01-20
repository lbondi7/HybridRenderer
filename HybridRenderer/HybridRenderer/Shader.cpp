#include "Shader.h"

#include "Initilizers.h"

#include "DebugLogger.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;
Shader::~Shader()
{
    devices = nullptr;
}

void Shader::Init(DeviceContext* _devices, const std::string& shaderName, VkShaderStageFlagBits _stage, const std::string& _entryPoint)
{
    devices = _devices;
    stage = _stage;
    entryPoint = _entryPoint;

    //auto model = r

    createModule(readFile("shaders/" + shaderName + getShaderTypeExtention(stage) + ".spv"));

    //getDescriptors("shaders/" + shaderName + getShaderTypeExtention(stage) + ".json");

    //shaderInfo = Initialisers::pipelineShaderStageCreateInfo(stage, shaderModule, entryPoint.c_str());
}

void Shader::createModule(const std::vector<char>& code) {

    VkShaderModuleCreateInfo createInfo = Initialisers::shaderModuleCreateInfo(reinterpret_cast<const uint32_t*>(code.data()), code.size());

    if (vkCreateShaderModule(devices->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
}

void Shader::getDescriptors(const std::string& shaderName)
{
    std::ifstream file(shaderName);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open json file!");
      return;
    }

    json j;
    file >> j;
    auto setCount = j["sets"].size();

    for (size_t i = 0; i < (setCount > 0 ? setCount : j["bindings"].size()); i++)
    {
        DescriptorData descriptorData;

        if (setCount > 0)
            descriptorData.set = j["sets"][i];
        else
            descriptorData.set = 0;
        descriptorData.binding = j["bindings"][i];
        descriptorData.type = static_cast<VkDescriptorType>(j["type"][i]);
        descriptorData.stage = stage;
        descriptors.emplace_back(descriptorData);
    }
    file.close();
}

void Shader::Destroy()
{
    descriptors.clear();
    vkDestroyShaderModule(devices->logicalDevice, shaderModule, nullptr);
   
}

VkPipelineShaderStageCreateInfo Shader::createInfo()
{
    return Initialisers::pipelineShaderStageCreateInfo(stage, shaderModule, entryPoint.c_str());
}

const char* Shader::getShaderTypeExtention(VkShaderStageFlagBits _stage) {
    switch (_stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT: 
    {
        return ".vert";
    }
    case VK_SHADER_STAGE_GEOMETRY_BIT: 
    {
        return ".geom";
    }
    case VK_SHADER_STAGE_FRAGMENT_BIT: 
    {
        return ".frag";
    }
    case VK_SHADER_STAGE_COMPUTE_BIT: 
    {
        return ".comp";
    }
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
    {
        return ".rgen";
    }
    case VK_SHADER_STAGE_MISS_BIT_KHR:
    {
        return ".rmiss";
    }
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
    {
        return ".rchit";
    }
    }
}