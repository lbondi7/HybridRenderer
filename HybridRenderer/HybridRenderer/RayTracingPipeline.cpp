#include "RayTracingPipeline.h"

void RayTracingPipeline::Create(DeviceContext* deviceContext, const RTPipelineInfo& pipelineInfo)
{

    this->deviceContext = deviceContext;
    this->pipelineInfo = pipelineInfo;

    vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkCreateRayTracingPipelinesKHR"));



    CreateDescriptorSetLayouts();
    Init();

}

void RayTracingPipeline::Init()
{
    CreatePipeline();
}

void RayTracingPipeline::CreateDescriptorSetLayouts() {

    std::vector<DescriptorData> descriptors;

    for (auto& shader : pipelineInfo.shaders)
    {
        for (auto& descriptor : shader->descriptors)
        {
            if (descriptor.set >= layoutCount)
                layoutCount = descriptor.set;

            if (!DescriptorData::contains(descriptor, descriptors))
            {
                descriptors.emplace_back(descriptor);
            }
        }
    }

    layoutCount += 1;
    descriptorSetLayouts.reserve(layoutCount);
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (size_t i = 0; i < layoutCount; i++)
    {
        bindings.reserve(descriptors.size());
        for (auto& descriptor : descriptors)
        {
            if (descriptor.set != i)
                continue;

            bindings.emplace_back(
                Initialisers::descriptorSetLayoutBinding(descriptor.binding, descriptor.type, descriptor.stage));

        }

        descriptorSetLayouts.push_back(deviceContext->descriptorSetManager.addLayoutAndReturn(i, bindings));

        bindings.clear();
    }
}

void RayTracingPipeline::CreatePipeline() {

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.reserve(layoutCount);

    for (size_t i = 0; i < layoutCount; ++i)
    {
        layouts.emplace_back(descriptorSetLayouts[i]->layout);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo =
        Initialisers::pipelineLayoutCreateInfo(layouts.data(), static_cast<uint32_t>(layouts.size()));


    if (vkCreatePipelineLayout(deviceContext->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    for (auto& shader : pipelineInfo.shaders)
    {
        shaderStages.emplace_back(shader->createInfo());

        switch (shader->stage) {
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:

            shaderGroups.push_back(
                Initialisers::rayTracingGeneralShaderGroup(static_cast<uint32_t>(shaderStages.size()) - 1));
            break;
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            shaderGroups.push_back(
                Initialisers::rayTracingClosestHitShaderGroup(static_cast<uint32_t>(shaderStages.size()) - 1));
            break;

        }
    }

    /*
        Create the ray tracing pipeline
    */
    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI = Initialisers::RayTracingPipelineCreateInfo(pipelineLayout,
        shaderStages.data(), static_cast<uint32_t>(shaderStages.size()), shaderGroups.data(), static_cast<uint32_t>(shaderGroups.size()));

    vkCreateRayTracingPipelinesKHR(deviceContext->logicalDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline);

}

void RayTracingPipeline::Destroy() {
    vkDestroyPipeline(deviceContext->logicalDevice, pipeline, nullptr);
    vkDestroyPipelineLayout(deviceContext->logicalDevice, pipelineLayout, nullptr);
}