#include "Pipeline.h"
#include "Initilizers.h"

#include <array>
#include <stdexcept>

Pipeline::~Pipeline()
{
    devices = nullptr;
    swapChain = nullptr;
    renderPass = nullptr;
}

void Pipeline::Create(Device* _devices, SwapChain* _swapChain, RenderPass* _renderPass, OffscreenPass& offscreenPass)
{

    devices = _devices;
    swapChain = _swapChain;
    renderPass = _renderPass;

    createDescriptorSetLayout();
    Init(offscreenPass);

}

void Pipeline::Init(OffscreenPass& offscreenPass)
{
    createGraphicsPipeline(offscreenPass);
}

void Pipeline::createDescriptorSetLayout() {
    //VkDescriptorSetLayoutBinding uboLayoutBinding = Initialisers::descriptorSetLayoutBinding(0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);

    //VkDescriptorSetLayoutBinding samplerLayoutBinding = Initialisers::descriptorSetLayoutBinding(1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);


    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        Initialisers::descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
        Initialisers::descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        Initialisers::descriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        Initialisers::descriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),

    };
    VkDescriptorSetLayoutCreateInfo layoutInfo = Initialisers::descriptorSetLayoutCreateInfo(bindings.data(), static_cast<uint32_t>(bindings.size()));

    if (vkCreateDescriptorSetLayout(devices->logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void Pipeline::createGraphicsPipeline(OffscreenPass& offscreenPass) {

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = Initialisers::pipelineVertexInputStateCreateInfo(&bindingDescription, 1, attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()));

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = Initialisers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    //VkViewport viewport = Initialisers::viewport(0, 0, (float)swapChain->extent.width, (float)swapChain->extent.height);

    //VkRect2D scissor = Initialisers::scissor(swapChain->extent);

    VkPipelineViewportStateCreateInfo viewportState = Initialisers::pipelineViewportStateCreateInfo(nullptr, 1, nullptr, 1);

    VkPipelineRasterizationStateCreateInfo rasterizer = Initialisers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 1.0f, VK_FALSE);

    VkPipelineMultisampleStateCreateInfo multisampling = Initialisers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

    VkPipelineDepthStencilStateCreateInfo depthStencil = Initialisers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineColorBlendAttachmentState colorBlendAttachment = Initialisers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

    VkPipelineColorBlendStateCreateInfo colorBlending = Initialisers::pipelineColorBlendStateCreateInfo(&colorBlendAttachment, 1, VK_FALSE, VK_LOGIC_OP_COPY);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = Initialisers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);

    if (vkCreatePipelineLayout(devices->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState =
        Initialisers::pipelineDynamicStateCreateInfo(
            dynamicStates.data(),
            dynamicStates.size());


   Shader vertexShader;
   vertexShader.Init(devices, "shadowmapping/scene", VK_SHADER_STAGE_VERTEX_BIT);
   Shader fragmentShader;
   fragmentShader.Init(devices, "shadowmapping/scene", VK_SHADER_STAGE_FRAGMENT_BIT);
        

    //auto vertShaderCode = readFile("shaders/shadowmapping/scene.vert.spv");
    //auto fragShaderCode = readFile("shaders/shadowmapping/scene.frag.spv");

    //auto vertShaderModule = createShaderModule(vertShaderCode);
    //auto fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo shaderStages[] = { 
        vertexShader.shaderInfo, 
        fragmentShader.shaderInfo
    };
    //VkPipelineShaderStageCreateInfo shaderStages[] = { 
    //    Initialisers::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule, "main"), 
    //    Initialisers::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule, "main") 
    //};

    VkGraphicsPipelineCreateInfo pipelineInfo = Initialisers::graphicsPipelineCreateInfo(pipelineLayout, renderPass->vkRenderPass, 0);
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    uint32_t enablePCF = 0;
    VkSpecializationMapEntry specializationMapEntry = Initialisers::specializationMapEntry(0, 0, sizeof(uint32_t));
    VkSpecializationInfo specializationInfo = Initialisers::specializationInfo(1, &specializationMapEntry, sizeof(uint32_t), &enablePCF);
    shaderStages[1].pSpecializationInfo = &specializationInfo;

    if (vkCreateGraphicsPipelines(devices->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    enablePCF = 1;

    if (vkCreateGraphicsPipelines(devices->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.sceneShadowPCF) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    //vkDestroyShaderModule(devices->logicalDevice, fragShaderModule, nullptr);
    //vkDestroyShaderModule(devices->logicalDevice, vertShaderModule, nullptr);

    //vertShaderCode = readFile("shaders/shadowmapping/offscreen.vert.spv");

    //vertShaderModule = createShaderModule(vertShaderCode);

    //// Offscreen pipeline (vertex shader only)
    //shaderStages[0] = Initialisers::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule, "main");
    //pipelineInfo.stageCount = 1;
    //// No blend attachment states (no color attachments used)
    //colorBlending.attachmentCount = 0;
    //// Cull front faces
    //depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    //// Enable depth bias
    //rasterizer.depthBiasEnable = VK_TRUE;
    //////std::vector<VkDynamicState> dynamicStates;
    //////// Add depth bias to dynamic state, so we can change it at runtime
    //////dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
    //////VkPipelineDynamicStateCreateInfo dynamicState =
    //////    Initialisers::pipelineDynamicStateCreateInfo(
    //////        dynamicStates.data(),
    //////        dynamicStates.size());

    //pipelineInfo.pDynamicState = &dynamicState;

    //pipelineInfo.renderPass = offscreenPass.renderPass;
    //if (vkCreateGraphicsPipelines(devices->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.offscreen) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to create graphics pipeline!");
    //}
}


VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = Initialisers::shaderModuleCreateInfo(reinterpret_cast<const uint32_t*>(code.data()), code.size());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(devices->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void Pipeline::Destroy() {
    vkDestroyPipeline(devices->logicalDevice, vkPipeline, nullptr);
    vkDestroyPipelineLayout(devices->logicalDevice, pipelineLayout, nullptr);
}