#include "ShadowMap.h"

#include "Initilizers.h"
#include "Vertex.h"
#include "Shader.h"

ShadowMap::~ShadowMap()
{
	devices = nullptr;
	swapChain = nullptr;
}

void ShadowMap::Create(Device* _devices, SwapChain* _swapChain)
{
	devices = _devices;
	swapChain = _swapChain;
}

void ShadowMap::Init()
{
	//render pass

	renderPass.Create(devices, swapChain);

	//pipeline


    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions({VertexAttributes::POSITION});

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = Initialisers::pipelineVertexInputStateCreateInfo(&bindingDescription, 1, attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()));

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = Initialisers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    //VkViewport viewport = Initialisers::viewport(0, 0, (float)swapChain->extent.width, (float)swapChain->extent.height);

    //VkRect2D scissor = Initialisers::scissor(swapChain->extent);

    VkPipelineViewportStateCreateInfo viewportState = Initialisers::pipelineViewportStateCreateInfo(nullptr, 1, nullptr, 1);

    VkPipelineRasterizationStateCreateInfo rasterizer = Initialisers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 1.0f, VK_FALSE, VK_TRUE);

    VkPipelineMultisampleStateCreateInfo multisampling = Initialisers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

    VkPipelineDepthStencilStateCreateInfo depthStencil = Initialisers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineColorBlendAttachmentState colorBlendAttachment = Initialisers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

    VkPipelineColorBlendStateCreateInfo colorBlending = Initialisers::pipelineColorBlendStateCreateInfo(&colorBlendAttachment, 0, VK_FALSE, VK_LOGIC_OP_COPY);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = Initialisers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);

    if (vkCreatePipelineLayout(devices->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = Initialisers::graphicsPipelineCreateInfo(pipelineLayout, renderPass.vkRenderPass, 0);
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.stageCount = 1;

    Shader shader;
    shader.Init(devices, "shadowmapping/offscreen", VK_SHADER_STAGE_VERTEX_BIT);
    pipelineInfo.pStages = &shader.shaderInfo;

    std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };
    VkPipelineDynamicStateCreateInfo dynamicState =
        Initialisers::pipelineDynamicStateCreateInfo(
            dynamicStates.data(),
            dynamicStates.size());

    pipelineInfo.pDynamicState = &dynamicState;

    if (vkCreateGraphicsPipelines(devices->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

	//frame buffers

	depthTexture.Create(devices, width, height, devices->getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	depthTexture.createImage();
	depthTexture.createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
	VkFilter shadowmap_filter = devices->formatIsFilterable(devices->getDepthFormat(), VK_IMAGE_TILING_OPTIMAL) ?
		VK_FILTER_LINEAR :
		VK_FILTER_NEAREST;
	depthTexture.createSampler(Initialisers::samplerCreateInfo(
		shadowmap_filter, 1.0f, 
		VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_INT_OPAQUE_WHITE));

    depthTexture.descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	frameBuffer.Create(devices, renderPass.vkRenderPass);

	for (size_t i = 0; i < swapChain->imageCount; i++)
	{
		VkExtent2D extent = {width, height};
		std::vector<VkImageView> attachments{depthTexture.imageView};
		frameBuffer.createFramebuffer(attachments, extent);
	}

}
