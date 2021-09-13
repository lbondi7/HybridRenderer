#pragma once
#include "Constants.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Vertex.h"
#include "Shader.h"
#include "DescriptorSetLayout.h"

struct RTPipelineInfo {
    std::vector<Shader*> shaders;
};

class RayTracingPipeline
{
public:


    void Create(DeviceContext* deviceContext, const RTPipelineInfo& pipelineInfo);

    void Init();

    void CreateDescriptorSetLayouts();

    void CreatePipeline();

    void Destroy();



    RTPipelineInfo pipelineInfo;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    std::vector< DescriptorSetLayout*> descriptorSetLayouts;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
    DeviceContext* deviceContext;
    uint32_t layoutCount = 0;

    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

};

