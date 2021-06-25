#pragma once

#include "Constants.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Vertex.h"
#include "Shader.h"


struct PipelineInfo {
    std::vector<Shader*> shaders;
    bool emptyVertexInputState = false;
    std::vector<VertexAttributes> attributes;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkPrimitiveTopology topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
    VkBool32 depthBiasEnable = VK_FALSE;
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    std::vector<VkDynamicState> dynamicStates;
    bool specializationInfo = false;
};

class Pipeline
{
public:

    Pipeline() = default;
    ~Pipeline();


    struct {
        VkPipeline offscreen;
        VkPipeline sceneShadow;
        VkPipeline sceneShadowPCF;
        VkPipeline debug;
    } pipelines;


    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline vkPipeline;

    VkDescriptorPool descriptorPool;

    void Create(Device* _devices, RenderPass* _renderPass, const PipelineInfo& _pipelineInfo);


    void Init();

    void createDescriptorSetLayout();

    void createGraphicsPipeline();

    VkShaderModule createShaderModule(const std::vector<char>& code);

    void Destroy();

private:

    Device* devices = nullptr;
    //SwapChain* swapChain = nullptr;
    RenderPass* renderPass = nullptr;
    PipelineInfo pipelineInfo;
};

