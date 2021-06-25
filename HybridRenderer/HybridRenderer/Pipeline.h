#pragma once

#include "Constants.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Vertex.h"
#include "Shader.h"


struct PipelineInfo {
    bool emptyInputStruct;
    std::vector<Shader*> shaders;
    VkCullModeFlagBits cullMode;
    VkPrimitiveTopology topology;
    VkPolygonMode polygonMode;
    std::vector<VkDynamicState> dynamicStates;
    VkBool32 depthBiasEnable;
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

    void Create(Device* _devices, SwapChain* _swapChain, RenderPass* _renderPass, OffscreenPass& offscreenPass);


    void Init(OffscreenPass& offscreenPass);

    void createDescriptorSetLayout();

    void createGraphicsPipeline(OffscreenPass& offscreenPass);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    void Destroy();

private:

    Device* devices = nullptr;
    SwapChain* swapChain = nullptr;
    RenderPass* renderPass = nullptr;

};

