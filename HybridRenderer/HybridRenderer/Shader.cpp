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
    createModule(readFile("shaders/" + shaderName + getShaderTypeExtention(stage) + ".spv"));

    getDescriptors("shaders/" + shaderName + getShaderTypeExtention(stage) + ".json");

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

    //std::ifstream file(shaderName);

    //if (!file.is_open()) {
    //    throw std::runtime_error("failed to open non-binary file!");
    //}

    //std::string line;
    //while (getline(file, line))
    //{
    //    uint32_t set, binding;
    //    if (line.find("binding") == std::string::npos && line.find("Binding") == std::string::npos)
    //        continue;

    //    if (line.find("set") == std::string::npos && line.find("Set") == std::string::npos)
    //        set = 0;
    //    

    //    bool hasL = false;
    //    for(auto c : line)
    //    {
    //        if (c == 'l')
    //        {
    //            hasL = true;
    //            break;
    //        }
    //    }
    //    if (!hasL)
    //        continue;

    //    std::string word = "";
    //    bool isDigit = false;
    //    for (auto c : line)
    //    {
    //        word += c;
    //        if (!isDigit && std::isdigit(c))
    //        {
    //            isDigit = true;
    //        }
    //        if (c == ' ' || c == '=')
    //        {
    //            if (isDigit)
    //            {

    //            }
    //            word.clear();
    //        }
    //    }
    //}
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
    }
}