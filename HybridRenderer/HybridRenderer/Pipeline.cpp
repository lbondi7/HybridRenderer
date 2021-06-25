#include "Pipeline.h"
#include "Initilizers.h"

#include <array>
#include <stdexcept>

Pipeline::~Pipeline()
{
    devices = nullptr;
    //swapChain = nullptr;
    renderPass = nullptr;
}

void Pipeline::Create(Device* _devices, RenderPass* _renderPass, const PipelineInfo& _pipelineInfo)
{

    devices = _devices;
    //swapChain = _swapChain;
    renderPass = _renderPass;
    pipelineInfo = _pipelineInfo;

    createDescriptorSetLayout();
    Init();

}

void Pipeline::Init()
{
    createGraphicsPipeline();
}

void Pipeline::createDescriptorSetLayout() {
    //VkDescriptorSetLayoutBinding uboLayoutBinding = Initialisers::descriptorSetLayoutBinding(0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);

    //VkDescriptorSetLayoutBinding samplerLayoutBinding = Initialisers::descriptorSetLayoutBinding(1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

    //size_t setCount = 0;
    //for (auto& shader : pipelineInfo.shaders)
    //{
    //    for (auto& descriptor : shader->descriptors)
    //    {
    //        if (descriptor.set > setCount)
    //        {
    //            setCount = descriptor.set;
    //        }
    //    }
    //}

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<DescriptorData> descriptors;
    for (auto& shader : pipelineInfo.shaders)
    {
        for (auto& descriptor : shader->descriptors)
        {
            if (!DescriptorData::contains(descriptor, descriptors))
            {
                bindings.emplace_back(Initialisers::descriptorSetLayoutBinding(descriptor.binding, descriptor.type, descriptor.stage));
                descriptors.emplace_back(descriptor);
            }
        }
    }
    
    //= {
    //Initialisers::descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
    //Initialisers::descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
    //Initialisers::descriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
    //Initialisers::descriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),

    //};
    VkDescriptorSetLayoutCreateInfo layoutInfo = Initialisers::descriptorSetLayoutCreateInfo(bindings.data(), static_cast<uint32_t>(bindings.size()));

    if (vkCreateDescriptorSetLayout(devices->logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    //std::vector<VkDescriptorSetLayoutBinding> bindings = {
    //Initialisers::descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
    //Initialisers::descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
    //Initialisers::descriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
    //Initialisers::descriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),

    //};
    //VkDescriptorSetLayoutCreateInfo layoutInfo = Initialisers::descriptorSetLayoutCreateInfo(bindings.data(), static_cast<uint32_t>(bindings.size()));

    //if (vkCreateDescriptorSetLayout(devices->logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to create descriptor set layout!");
    //}
}

void Pipeline::createGraphicsPipeline() {

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions(pipelineInfo.attributes);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = pipelineInfo.emptyVertexInputState ? Initialisers::pipelineVertexInputStateCreateInfo() :
        Initialisers::pipelineVertexInputStateCreateInfo(&bindingDescription, 1, attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()));

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = Initialisers::pipelineInputAssemblyStateCreateInfo(pipelineInfo.topology);

    VkPipelineViewportStateCreateInfo viewportState;
    if(pipelineInfo.viewports.empty())
        viewportState = Initialisers::pipelineViewportStateCreateInfo(nullptr, 1, nullptr, 1);
    else {
        //VkViewport viewport = Initialisers::viewport(0, 0, (float)swapChain->extent.width, (float)swapChain->extent.height);
        //VkRect2D scissor = Initialisers::scissor(swapChain->extent);

        viewportState = 
            Initialisers::pipelineViewportStateCreateInfo(pipelineInfo.viewports.data(), static_cast<uint32_t>(pipelineInfo.viewports.size()), pipelineInfo.scissors.data(), static_cast<uint32_t>(pipelineInfo.scissors.size()));
    }

    VkPipelineRasterizationStateCreateInfo rasterizer = Initialisers::pipelineRasterizationStateCreateInfo(pipelineInfo.polygonMode, pipelineInfo.cullMode, VK_FRONT_FACE_COUNTER_CLOCKWISE, 1.0f, VK_FALSE, pipelineInfo.depthBiasEnable);

    VkPipelineMultisampleStateCreateInfo multisampling = Initialisers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

    VkPipelineDepthStencilStateCreateInfo depthStencil = Initialisers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineColorBlendAttachmentState colorBlendAttachment = Initialisers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

    VkPipelineColorBlendStateCreateInfo colorBlending = Initialisers::pipelineColorBlendStateCreateInfo(&colorBlendAttachment, 1, VK_FALSE, VK_LOGIC_OP_COPY);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = Initialisers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);

    if (vkCreatePipelineLayout(devices->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for(auto& shader : pipelineInfo.shaders)
    {
        shaderStages.emplace_back(shader->createInfo());
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = Initialisers::graphicsPipelineCreateInfo(pipelineLayout, renderPass->vkRenderPass, 0);
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizer;
    pipelineCreateInfo.pMultisampleState = &multisampling;
    pipelineCreateInfo.pDepthStencilState = &depthStencil;
    pipelineCreateInfo.pColorBlendState = &colorBlending;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    if (!pipelineInfo.dynamicStates.empty())
    {
        VkPipelineDynamicStateCreateInfo dynamicState = Initialisers::pipelineDynamicStateCreateInfo(pipelineInfo.dynamicStates.data(), pipelineInfo.dynamicStates.size());
        pipelineCreateInfo.pDynamicState = &dynamicState;
    }


    if (pipelineInfo.specializationInfo)
    {
        uint32_t enablePCF = 0;
        VkSpecializationMapEntry specializationMapEntry = Initialisers::specializationMapEntry(0, 0, sizeof(uint32_t));
        VkSpecializationInfo specializationInfo = Initialisers::specializationInfo(1, &specializationMapEntry, sizeof(uint32_t), &enablePCF);
        shaderStages[1].pSpecializationInfo = &specializationInfo;
    }

    if (vkCreateGraphicsPipelines(devices->logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &vkPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
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
    vkDestroyDescriptorSetLayout(devices->logicalDevice, descriptorSetLayout, nullptr);
}