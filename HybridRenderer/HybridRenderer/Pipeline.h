#pragma once

#include "Constants.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Vertex.h"
#include "Shader.h"
#include "DescriptorSetLayout.h"

struct PipelineInfo {
    std::vector<Shader*> shaders;
    bool emptyVertexInputState = false;
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
    std::vector<VkVertexInputBindingDescription> vertexInputBindings;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    int polygonMode = 0;
    VkBool32 depthBiasEnable = VK_FALSE;
    uint32_t colourAttachmentCount = 1;
    VkBool32 blendEnabled = VK_FALSE;
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    std::vector<VkDynamicState> dynamicStates;
    std::vector<VkPushConstantRange> pushConstants;
    bool conservativeRasterisation = false;
    std::string layoutsName;
};

class Pipeline
{
public:

    Pipeline() = default;
    ~Pipeline();


    //DescriptorSetLayout* descriptorSetLayouts;
    std::vector<DescriptorSetLayout*> descriptorSetLayouts;
    uint32_t layoutCount = 0;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline vkPipeline;

    //std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    //std::vector<DescriptorSetLayout> descriptorSetLayouts;

    //VkDescriptorPool descriptorPool;

    void Create(DeviceContext* _devices, RenderPass* _renderPass, const PipelineInfo& _pipelineInfo);

    bool SortLayouts(DescriptorSetLayout* l1, DescriptorSetLayout* l2);

    void Init();

    void createDescriptorSetLayouts();

    void createGraphicsPipeline();

    //VkShaderModule createShaderModule(const std::vector<char>& code);

    PipelineInfo pipelineInfo;

    void Destroy(bool complete = true);

private:

    DeviceContext* devices = nullptr;
    //SwapChain* swapChain = nullptr;
    RenderPass* renderPass = nullptr;

};

