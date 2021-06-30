#pragma once

#include "Constants.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Vertex.h"
#include "Shader.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSetManager.h"

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
    bool conservativeRasterisation = false;
};

class Pipeline
{
public:

    Pipeline() = default;
    ~Pipeline();


    //DescriptorSetLayout* descriptorSetLayouts;
    std::vector< DescriptorSetLayout*> descriptorSetLayouts;
    uint32_t layoutCount = 0;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline vkPipeline;

    //std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    //std::vector<DescriptorSetLayout> descriptorSetLayouts;

    //VkDescriptorPool descriptorPool;

    void Create(DeviceContext* _devices, RenderPass* _renderPass, DescriptorSetManager* dsManager, const PipelineInfo& _pipelineInfo);


    void Init();

    void createDescriptorSetLayouts();

    void createGraphicsPipeline();

    //VkShaderModule createShaderModule(const std::vector<char>& code);

    void Destroy(bool complete = true);

private:

    DeviceContext* devices = nullptr;
    //SwapChain* swapChain = nullptr;
    RenderPass* renderPass = nullptr;
    PipelineInfo pipelineInfo;
    DescriptorSetManager* descriptorSetManager = nullptr;

};

