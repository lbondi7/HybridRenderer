#include "ShadowMap.h"

#include "Initilizers.h"
#include "Vertex.h"
#include "Shader.h"
#include "Utility.h"

ShadowMap::~ShadowMap()
{
	devices = nullptr;
	swapChain = nullptr;
}

void ShadowMap::update(bool& resize)
{

	if (static_cast<uint32_t>(resolution) != width)
	{
		resize = true;
	}
}

void ShadowMap::Create(DeviceContext* _devices, SwapChain* _swapChain)
{
	devices = _devices;
	swapChain = _swapChain;
}

void ShadowMap::Init(DescriptorSetManager* dsManager, const PipelineInfo& pipelineInfo)
{
	//render pass

	dsm = dsManager;
	RenderPassInfo info{};
	info.attachments.push_back({ AttachmentType::DEPTH, Utility::findDepthFormat(devices->physicalDevice), VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED });

	info.dependencies.emplace_back(Initialisers::subpassDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));
	info.dependencies.emplace_back(Initialisers::subpassDependency(0, VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT));

	renderPass.Create(devices, info);

	//pipeline

    //Shader shader;
    //shader.Init(devices, "shadowmapping/offscreen", VK_SHADER_STAGE_VERTEX_BIT);

	height = width = static_cast<uint32_t>(resolution);

    pipeline.Create(devices, &renderPass, dsManager, pipelineInfo);

	//frame buffers

	depthTexture.Create(devices, width, height, devices->getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
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


	DescriptorSetRequest request;
	request.ids = { { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER } };

	dsm->getDescriptorSets(descriptorSets, request);

	for (size_t i = 0; i < dsm->imageCount; i++) {

		std::vector<VkWriteDescriptorSet> descriptorWrites{
		Initialisers::writeDescriptorSet(descriptorSets[i], 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &depthTexture.descriptorInfo)
		};

		vkUpdateDescriptorSets(devices->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

}

void ShadowMap::reinit(bool complete)
{
	Destroy(complete);

	renderPass.Init();

	pipeline.Init();


	height = width = static_cast<uint32_t>(resolution);

	depthTexture.Create(devices, width, height, devices->getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	depthTexture.createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
	VkFilter shadowmap_filter = devices->formatIsFilterable(devices->getDepthFormat(), VK_IMAGE_TILING_OPTIMAL) ?
		VK_FILTER_LINEAR :
		VK_FILTER_NEAREST;
	depthTexture.createSampler(Initialisers::samplerCreateInfo(
		shadowmap_filter, 1.0f,
		VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_INT_OPAQUE_WHITE));



	//depthCubemap.Create(devices, width, height, devices->getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	//depthCubemap.createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
	//depthCubemap.createSampler();


	depthTexture.descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	frameBuffer.Create(devices, renderPass.vkRenderPass);

	for (size_t i = 0; i < swapChain->imageCount; i++)
	{
		VkExtent2D extent = { width, height };
		std::vector<VkImageView> attachments{ depthTexture.imageView };
		frameBuffer.createFramebuffer(attachments, extent);
	}

	//DescriptorSetRequest request;
	//request.requests = { { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER } };

	//dsm->getDescriptorSets(descriptorSets, request);

	for (size_t i = 0; i < dsm->imageCount; i++) {

		std::vector<VkWriteDescriptorSet> descriptorWrites{
		Initialisers::writeDescriptorSet(descriptorSets[i], 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &depthTexture.descriptorInfo)
		};

		vkUpdateDescriptorSets(devices->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

}

void ShadowMap::Destroy(bool complete)
{

	pipeline.Destroy();
	if (!complete)
		return;

    depthTexture.Destroy();

    frameBuffer.Destroy();

    renderPass.Destroy();
}
