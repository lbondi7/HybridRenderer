#include "Shader.h"

#include "Initilizers.h"

Shader::~Shader()
{
}

void Shader::Init(Device* _devices, const std::string& shaderName, VkShaderStageFlagBits _stage, const std::string& _entryPoint)
{
    devices = _devices;
    stage = _stage;
    entryPoint = _entryPoint;
    createModule(readFile("shaders/" + shaderName + getShaderTypeExtention(stage) + ".spv"));

    shaderInfo = Initialisers::pipelineShaderStageCreateInfo(stage, module, entryPoint.c_str());
}

void Shader::createModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = Initialisers::shaderModuleCreateInfo(reinterpret_cast<const uint32_t*>(code.data()), code.size());

    if (vkCreateShaderModule(devices->logicalDevice, &createInfo, nullptr, &module) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

 
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