#include "ShadowMap.h"

#include "Initilizers.h"
#include "Vertex.h"
#include "Shader.h"

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

		height = static_cast<uint32_t>(resolution);
		width = static_cast<uint32_t>(resolution);
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
	renderPass.Create(devices, swapChain);

	//pipeline

    //Shader shader;
    //shader.Init(devices, "shadowmapping/offscreen", VK_SHADER_STAGE_VERTEX_BIT);

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
	request.requests = { { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER } };

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
