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

void Pipeline::Create(DeviceContext* _devices, RenderPass* _renderPass, DescriptorSetManager* dsManager, const PipelineInfo& _pipelineInfo)
{

    devices = _devices;
    //swapChain = _swapChain;
    renderPass = _renderPass;
    pipelineInfo = _pipelineInfo;
    descriptorSetManager = dsManager;

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
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    for (size_t i = 0; i < layoutCount; i++)
    {
        for (auto& descriptor : descriptors)
        {
            if (descriptor.set != i)
                continue;

            bindings.emplace_back(
                Initialisers::descriptorSetLayoutBinding(descriptor.binding, descriptor.type, descriptor.stage));

        }

        descriptorSetLayouts.push_back(descriptorSetManager->addLayoutAndReturn(i, bindings));

        bindings.clear();
    }


    //descriptorSetLayouts.reserve(layoutCount);
    //descriptorSetLayouts.resize(layoutCount);

    //for (size_t i = 0; i < layoutCount; i++)
    //{
    //    descriptorSetLayouts[i].set = i;
    //    for (auto& descriptor : descriptors)
    //    {
    //        if (descriptor.set != i)
    //            continue;

    //        descriptorSetLayouts[i].bindings.emplace_back(
    //            Initialisers::descriptorSetLayoutBinding(descriptor.binding, descriptor.type, descriptor.stage));

    //    }
    //    //if(i == 0)
    //    //else
    //    descriptorSetLayouts[i].init(devices);
    //}

    //for (size_t i = 0; i < setCount; i++)
    //{
    //    std::vector<VkDescriptorSetLayoutBinding> bind;
    //    for (auto& descriptor : descriptors)
    //    {
    //        if (descriptor.set != i)
    //            continue;
    //        bind.emplace_back(Initialisers::descriptorSetLayoutBinding(descriptor.binding, descriptor.type, descriptor.stage));
    //    }
    //    VkDescriptorSetLayoutCreateInfo layoutInfo =
    //        Initialisers::descriptorSetLayoutCreateInfo(bind.data(), static_cast<uint32_t>(bind.size()));
    //    if (vkCreateDescriptorSetLayout(devices->logicalDevice, &layoutInfo, nullptr, &descriptorSetLayouts[i]) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create descriptor set layout!");
    //    }
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
        viewportState = 
            Initialisers::pipelineViewportStateCreateInfo(pipelineInfo.viewports.data(), static_cast<uint32_t>(pipelineInfo.viewports.size()), pipelineInfo.scissors.data(), static_cast<uint32_t>(pipelineInfo.scissors.size()));
    }

    VkPipelineRasterizationStateCreateInfo rasterizer = Initialisers::pipelineRasterizationStateCreateInfo(pipelineInfo.polygonMode, pipelineInfo.cullMode, VK_FRONT_FACE_COUNTER_CLOCKWISE, 1.0f, VK_FALSE, pipelineInfo.depthBiasEnable);

    VkPipelineMultisampleStateCreateInfo multisampling = Initialisers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

    VkPipelineDepthStencilStateCreateInfo depthStencil = Initialisers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineColorBlendAttachmentState colorBlendAttachment = Initialisers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

    VkPipelineColorBlendStateCreateInfo colorBlending = Initialisers::pipelineColorBlendStateCreateInfo(&colorBlendAttachment, 1, VK_FALSE, VK_LOGIC_OP_COPY);

    std::vector<VkDescriptorSetLayout> layouts;
    //for (auto& layout : descriptorSetLayouts)
    //{
    //    layouts.emplace_back(layout.layout);
    //}

    for (size_t i = 0; i < layoutCount; ++i)
    {
        layouts.emplace_back(descriptorSetLayouts[i]->layout);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = 
        Initialisers::pipelineLayoutCreateInfo(layouts.data(), static_cast<uint32_t>(layouts.size()));

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

    if (pipelineInfo.conservativeRasterisation) {

        VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterProps{};

        VkPhysicalDeviceProperties2KHR deviceProps2{};
        conservativeRasterProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
        deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
        deviceProps2.pNext = &conservativeRasterProps;
        vkGetPhysicalDeviceProperties2(devices->physicalDevice, &deviceProps2);

        VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterStateCI{};
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


//VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
//    VkShaderModuleCreateInfo createInfo = Initialisers::shaderModuleCreateInfo(reinterpret_cast<const uint32_t*>(code.data()), code.size());
//
//    VkShaderModule shaderModule;
//    if (vkCreateShaderModule(devices->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create shader module!");
//    }
//
//    return shaderModule;
//}

void Pipeline::Destroy(bool complete) {
    vkDestroyPipeline(devices->logicalDevice, vkPipeline, nullptr);
    vkDestroyPipelineLayout(devices->logicalDevice, pipelineLayout, nullptr);

    if (!complete)
        return;

    for (auto& layout : descriptorSetLayouts)
    {
        layout = nullptr;
    }

    descriptorSetLayouts.clear();
}