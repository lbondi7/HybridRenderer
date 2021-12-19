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

void ShadowMap::Create(DeviceContext* _devices, int descriptorSet, const std::string& windowName)
{
	devices = _devices;
	widget.enabled = true;
	this->descriptorSet = descriptorSet;
	this->windowName = windowName;
}

bool ShadowMap::Update(uint32_t imageIndex) 
{

	bool updated = false;
	if (ImGUI::enabled && widget.enabled) {
		if (widget.NewWindow(windowName.c_str())) {

			if (widget.Slider("Estimation Enabled", &shadowUBO.shadow.x, 0, 1))
			{
				for (auto& buffer : buffers)
				{
					buffer.AllocatedMap(&shadowUBO);
				}
			}
			if (widget.Slider("Terminate Ray", &shadowUBO.shadow.y, 0, 1))
			{
				for (auto& buffer : buffers)
				{
					buffer.AllocatedMap(&shadowUBO);
				}
			}
			if (widget.Slider("Samples", &shadowUBO.shadow.z, 1, 512))
			{
				for (auto& buffer : buffers)
				{
					buffer.AllocatedMap(&shadowUBO);
				}
			}
			if (widget.Slider("Ray Tracing Samples", &shadowUBO.shadow.w, 1, 64))
			{
				for (auto& buffer : buffers)
				{
					buffer.AllocatedMap(&shadowUBO);
				}
			}
			if (widget.Slider("Blocker Scale", &shadowUBO.blocker.x, 0.0, 0.5))
			{
				for (auto& buffer : buffers)
				{
					buffer.AllocatedMap(&shadowUBO);

				}
			}
			if (widget.Slider("Ray Tracing Scale", &shadowUBO.blocker.y, 0.0, 0.5))
			{
				for (auto& buffer : buffers)
				{
					buffer.AllocatedMap(&shadowUBO);
				}
			}
			if (widget.Slider("PCF Bias", &shadowUBO.blocker.z, 0.0, 2.0))
			{
				for (auto& buffer : buffers)
				{
					buffer.AllocatedMap(&shadowUBO);

				}
			}
			if (widget.Slider("Search Bias", &shadowUBO.blocker.w, -0.05, 0.05))
			{
				for (auto& buffer : buffers)
				{
					buffer.AllocatedMap(&shadowUBO);
				
				}
			}
			//float dg = 0.0;
			//if (widget.Slider("AlphaThreshold", &dg, 0.0, 1.0))
			//{
			//	for (auto& buffer : buffers)
			//	{
			//		buffer.AllocatedMap(&shadowUBO);
			//	}
			//}

			if (widget.CheckBox("Conservative Rasterisation", &pipeline.pipelineInfo.conservativeRasterisation)) {
				vkQueueWaitIdle(devices->presentQueue);
				Reinitialise();
				updated = true;
			}
			if (widget.Slider("Resolution", &resolution, 1, 8000))
			{
				vkQueueWaitIdle(devices->presentQueue);
				Reinitialise();
				updated = true;
			}

			widget.Image(0, { 300, 300 });
		}
		widget.EndWindow();
	}

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
	depthTexture.CreateSampler(Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, 1.0f,
		VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_INT_OPAQUE_WHITE));

	depthTexture.descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	frameBuffer.Create(devices, renderPass.vkRenderPass);

	for (size_t i = 0; i < devices->imageCount; i++)
	{
		VkExtent2D extent = { width, height };
		std::vector<VkImageView> attachments{ depthTexture.imageView };
		frameBuffer.createFramebuffer(attachments, extent);
	}

	DescriptorSetRequest request({ {"scene", descriptorSet} });
	request.AddDescriptorBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	request.AddDescriptorBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	request.AddDescriptorImageData(0, &depthTexture.descriptorInfo);
	request.AddDescriptorBufferData(1, buffers.data());

	devices->GetDescriptors(descriptor, &request);

	widget.SetupImage(0, depthTexture);
}


void ShadowMap::Initialise(const PipelineInfo& pipelineInfo)
{
	shadowUBO.blocker.x = 0.1f;
	shadowUBO.blocker.z = 1.0f;
	shadowUBO.shadow.x = 1;
	shadowUBO.shadow.y = 1;
	shadowUBO.shadow.z = 64;
	shadowUBO.shadow.w = 3;
	resolution = 1024;
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

	buffers.resize(devices->imageCount);
	for (auto& buffer : buffers)
	{
		VkDeviceSize bufferSize = sizeof(ShadowUBO);
		buffer.Allocate(devices, bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &shadowUBO);
	}
	Initialise();

	pipeline.Create(devices, &renderPass, pipelineInfo);


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
