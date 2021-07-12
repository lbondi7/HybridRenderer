#include "Pipeline.h"
#include "Initilizers.h"

#include <array>
#include <stdexcept>

Pipeline::~Pipeline()
{
    for (auto& layout : descriptorSetLayouts)
    {
        layout = nullptr;
    }

    descriptorSetLayouts.clear();
    devices = nullptr;
    //swapChain = nullptr;
    renderPass = nullptr;
}

void Pipeline::Create(DeviceContext* _devices, RenderPass* _renderPass, const PipelineInfo& _pipelineInfo)
{

    devices = _devices;
    //swapChain = _swapChain;
    renderPass = _renderPass;
    pipelineInfo = _pipelineInfo;

    createDescriptorSetLayouts();
    Init();

}

void Pipeline::Init()
{
    createGraphicsPipeline();
}

void Pipeline::createDescriptorSetLayouts() {

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

        descriptorSetLayouts.push_back(devices->dsm.addLayoutAndReturn(i, bindings));

        bindings.clear();
    }
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void Pipeline::createGraphicsPipeline() {

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.reserve(layoutCount);

    for (size_t i = 0; i < layoutCount; ++i)
    {
        layouts.emplace_back(descriptorSetLayouts[i]->layout);
    }


    VkPipelineLayoutCreateInfo pipelineLayoutInfo =
        Initialisers::pipelineLayoutCreateInfo(layouts.data(), static_cast<uint32_t>(layouts.size()));

    if (!pipelineInfo.pushConstants.empty()) {
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<float>(pipelineInfo.pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pipelineInfo.pushConstants.data();
    }

    if (vkCreatePipelineLayout(devices->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo = Initialisers::graphicsPipelineCreateInfo(pipelineLayout, renderPass->vkRenderPass, 0);


    auto bindingDescription = pipelineInfo.vertexInputBindings;
    auto attributeDescriptions = pipelineInfo.vertexInputAttributes;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = pipelineInfo.emptyVertexInputState ? Initialisers::pipelineVertexInputStateCreateInfo() :
        Initialisers::pipelineVertexInputStateCreateInfo(bindingDescription.data(), 1, attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()));

    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;


    VkPipelineInputAssemblyStateCreateInfo inputAssembly = Initialisers::pipelineInputAssemblyStateCreateInfo(pipelineInfo.topology);

    pipelineCreateInfo.pInputAssemblyState = &inputAssembly;


    VkPipelineViewportStateCreateInfo viewportState;
    if(pipelineInfo.viewports.empty())
        viewportState = Initialisers::pipelineViewportStateCreateInfo(nullptr, 1, nullptr, 1);
    else {
        viewportState = 
            Initialisers::pipelineViewportStateCreateInfo(pipelineInfo.viewports.data(), static_cast<uint32_t>(pipelineInfo.viewports.size()), pipelineInfo.scissors.data(), static_cast<uint32_t>(pipelineInfo.scissors.size()));
    }

    pipelineCreateInfo.pViewportState = &viewportState;

    VkPipelineRasterizationStateCreateInfo rasterizer = 
        Initialisers::pipelineRasterizationStateCreateInfo(
            static_cast<VkPolygonMode>(pipelineInfo.polygonMode), pipelineInfo.cullMode, VK_FRONT_FACE_COUNTER_CLOCKWISE, 1.0f, VK_FALSE, pipelineInfo.depthBiasEnable);

    pipelineCreateInfo.pRasterizationState = &rasterizer;

    VkPipelineMultisampleStateCreateInfo multisampling = Initialisers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

    pipelineCreateInfo.pMultisampleState = &multisampling;

    VkPipelineDepthStencilStateCreateInfo depthStencil = Initialisers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    pipelineCreateInfo.pDepthStencilState = &depthStencil;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = Initialisers::pipelineColourBlendAttachmentState(0xf, pipelineInfo.blendEnabled);

    VkPipelineColorBlendStateCreateInfo colorBlending = Initialisers::pipelineColourBlendStateCreateInfo(&colorBlendAttachment, 1, VK_FALSE, VK_LOGIC_OP_COPY);
    pipelineCreateInfo.pColorBlendState = &colorBlending;


    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for(auto& shader : pipelineInfo.shaders)
    {
        shaderStages.emplace_back(shader->createInfo());
    }

    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();


    VkPipelineDynamicStateCreateInfo dynamicState = Initialisers::pipelineDynamicStateCreateInfo(pipelineInfo.dynamicStates.data(), static_cast<uint32_t>(pipelineInfo.dynamicStates.size()));
    //dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    float shu = 0.0f;

    VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterStateCI{};

    if (pipelineInfo.conservativeRasterisation) {

        VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterProps{};

        VkPhysicalDeviceProperties2KHR deviceProps2{};
        conservativeRasterProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
        deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
        deviceProps2.pNext = &conservativeRasterProps;
        vkGetPhysicalDeviceProperties2(devices->physicalDevice, &deviceProps2);

        conservativeRasterStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
        conservativeRasterStateCI.conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
        conservativeRasterStateCI.extraPrimitiveOverestimationSize = conservativeRasterProps.maxExtraPrimitiveOverestimationSize;
        
        // Conservative rasterization state has to be chained into the pipeline rasterization state create info structure
        rasterizer.pNext = &conservativeRasterStateCI;
    }

    if (vkCreateGraphicsPipelines(devices->logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &vkPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void Pipeline::Destroy(bool complete) {
    vkDestroyPipeline(devices->logicalDevice, vkPipeline, nullptr);
    vkDestroyPipelineLayout(devices->logicalDevice, pipelineLayout, nullptr);

    if (!complete)
        return;
}