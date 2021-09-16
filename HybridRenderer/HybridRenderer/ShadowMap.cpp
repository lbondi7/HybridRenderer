#include "ShadowMap.h"

#include "Initilizers.h"
#include "Vertex.h"
#include "Shader.h"
#include "Utility.h"
#include "ImGUI_.h"

ShadowMap::~ShadowMap()
{
	devices = nullptr;
	swapChain = nullptr;
}

void ShadowMap::Create(DeviceContext* _devices, SwapChain* _swapChain)
{
	devices = _devices;
	swapChain = _swapChain;
}

bool ShadowMap::Update(uint32_t imageIndex) {


	bool updated = false;
	if (ImGUI::enabled && widget.enabled) {
		if (widget.NewWindow("Shadow Map")) {

			widget.Slider("RayQuery", &shadowUBO.shadowMap, 0, 1);
			if (widget.CheckBox("Conservative Rasterisation", &pipeline.pipelineInfo.conservativeRasterisation)) {
				vkQueueWaitIdle(devices->presentQueue);
				Reinitialise();
				updated = true;
			}
			if (widget.Slider("Resolution", &resolution, 1, 2048))
			{
				vkQueueWaitIdle(devices->presentQueue);
				Reinitialise();
				updated = true;
			}
			widget.Image(0, { 300, 300 });
		}
		widget.EndWindow();
	}

	buffers[imageIndex].AllocatedMap(&shadowUBO);

	return updated;
}

void ShadowMap::Initialise()
{
	height = width = static_cast<uint32_t>(resolution);

	//frame buffers

	depthTexture.Create(devices, width, height, devices->getDepthFormat(), 
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	depthTexture.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
	VkFilter shadowmap_filter = devices->formatIsFilterable(devices->getDepthFormat(), VK_IMAGE_TILING_OPTIMAL) 
		? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
	depthTexture.CreateSampler(Initialisers::samplerCreateInfo(shadowmap_filter, 1.0f,
		VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_INT_OPAQUE_WHITE));

	depthTexture.descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	frameBuffer.Create(devices, renderPass.vkRenderPass);

	for (size_t i = 0; i < swapChain->imageCount; i++)
	{
		VkExtent2D extent = { width, height };
		std::vector<VkImageView> attachments{ depthTexture.imageView };
		frameBuffer.createFramebuffer(attachments, extent);
	}


	DescriptorSetRequest request;
	request.ids.emplace_back(
		DescriptorSetRequest::BindingType(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
			VK_SHADER_STAGE_FRAGMENT_BIT ));
	request.ids.emplace_back(DescriptorSetRequest::BindingType(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
		VK_SHADER_STAGE_FRAGMENT_BIT));

	request.data.reserve(devices->imageCount * request.ids.size());

	for (size_t i = 0; i < devices->imageCount; i++) {

		request.data.emplace_back(&depthTexture.descriptorInfo);
		request.data.push_back(&buffers[i].descriptorInfo);
	}

	devices->GetDescriptors(descriptor, &request);

	widget.SetupImage(0, depthTexture);
}


void ShadowMap::Initialise(const PipelineInfo& pipelineInfo)
{
	//render pass
	RenderPassInfo info{};
	info.attachments.push_back({ AttachmentType::DEPTH, devices->getDepthFormat(), VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED });

	info.dependencies.emplace_back(Initialisers::subpassDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));
	info.dependencies.emplace_back(Initialisers::subpassDependency(0, VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT));

	renderPass.Create(devices, info);

	pipeline.Create(devices, &renderPass, pipelineInfo);

	buffers.resize(devices->imageCount);
	for (auto& buffer : buffers)
	{
		VkDeviceSize bufferSize = sizeof(ShadowUBO);
		buffer.Allocate(devices, bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	Initialise();

}

void ShadowMap::Reinitialise(bool complete)
{
	Destroy(complete);

	renderPass.Init();

	pipeline.Init();

	Initialise();
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
