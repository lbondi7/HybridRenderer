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

bool ShadowMap::Update(uint32_t imageIndex, bool lightOrtho)
{

	bool updated = false;

	if (prevOrtho != lightOrtho) 
	{
		prevOrtho = lightOrtho;
		if (lightOrtho)
		{
			shadowUBO.blocker.x = 0.015f;
			shadowUBO.blocker.y = 0.025f;
		}
		else 
		{
			shadowUBO.blocker.x = 0.5f;
			shadowUBO.blocker.y = 0.5f;
		}
		updated = true;
	}

	if (ImGUI::enabled && widget.enabled) 
	{
		//if (widget.NewWindow(windowName.c_str())) {


		if (widget.CheckBox("Enable Debug View", &debugView))
		{
			updated = true;
			shadowUBO.extra.x = debugView;
		}

		if (widget.Slider("Shadow Map/Ray Query/RRQSS", &shadowUBO.shadow.x, 0, 2))
			updated = true;

		if (shadowUBO.shadow.x != 1)
		{
			widget.Text("Shadow Map");

			if (widget.Slider("Samples", &shadowUBO.shadow.z, 1, 64))
				updated = true;

			if (widget.Slider("Blocker Scale", &shadowUBO.blocker.x, 0.0, 1.0))
				updated = true;

			//if (widget.Slider("PCF Bias", &shadowUBO.blocker.z, -0.1, 0.1))
			//	updated = true;

			//if (widget.Slider("Search Bias", &shadowUBO.blocker.w, -0.1, 0.1))
			//	updated = true;
		}

		if (shadowUBO.shadow.x != 0)
		{
			widget.Text("Ray Tracing");

			if (widget.Slider("Ray Tracing Samples", &shadowUBO.shadow.w, 1, 32))
				updated = true;

			if (widget.Slider("Ray Tracing Scale", &shadowUBO.blocker.y, 0.0, 1.0))
				updated = true;
		}
			//if (widget.Slider("Sample Reduction", &shadowUBO.shadow.y, 0, 3))
			//{
			//	for (auto& buffer : buffers)
			//	{
			//		buffer.AllocatedMap(&shadowUBO);
			//	}
			//}
			//float dg = 0.0;
			//if (widget.Slider("AlphaThreshold", &dg, 0.0, 1.0))
			//{
			//	for (auto& buffer : buffers)
			//	{
			//		buffer.AllocatedMap(&shadowUBO);
			//	}
			//}
			//if (widget.Button("Save")) {
				//saveScreenshot("Screenshot.ppm");
			//}
			//if (widget.CheckBox("Conservative Rasterisation", &pipeline.pipelineInfo.conservativeRasterisation)) {
			//	vkQueueWaitIdle(devices->presentQueue);
			//	vkDeviceWaitIdle(devices->logicalDevice);
			//	Reinitialise();
			//	updated = true;
			//}
			//if (widget.Slider("Resolution", &resolution, 1, 8000))
			//{
			//	vkQueueWaitIdle(devices->presentQueue);
			//	vkDeviceWaitIdle(devices->logicalDevice);
			//	Reinitialise();
			//	updated = true;
			//}

			//widget.Image(0, { 1000, 1000 });
		//}
		//widget.EndWindow();

		if (updated)
		{
			for (auto& buffer : buffers)
			{
				buffer.AllocatedMap(&shadowUBO);

			}
		}
	}

	return updated;
}

void ShadowMap::Initialise()
{
	height = width = static_cast<uint32_t>(resolution);

	//frame buffers

	depthTexture.Create(devices, width, height, devices->getDepthFormat(), 
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
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
	std::string sceneShader = (devices->validGPU == 2 ? "sceneRQ" : "scene");
	DescriptorSetRequest request({ {sceneShader, descriptorSet} });
	request.AddDescriptorBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	request.AddDescriptorBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	request.AddDescriptorImageData(0, &depthTexture.descriptorInfo);
	request.AddDescriptorBufferData(1, buffers.data());

	devices->GetDescriptors(descriptor, &request);

	widget.SetupImage(0, depthTexture);
}


void ShadowMap::Initialise(const PipelineInfo& pipelineInfo)
{
	shadowUBO.blocker.x = 0.5f;
	shadowUBO.blocker.y = 0.5f;
	//shadowUBO.blocker.z = 0.1f;
	shadowUBO.blocker.w = -0.002f;
	shadowUBO.shadow.x = 2;
	shadowUBO.shadow.y = 1;
	shadowUBO.shadow.z = 32;
	shadowUBO.shadow.w = 5;
	resolution = 2048;
	shadowUBO.extra.y = 1;
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


void ShadowMap::saveScreenshot(const char* filename)
{
	// Check blit support for source and destination
	VkFormatProperties formatProps;

	// Check if the device supports blitting from optimal images (the swapchain images are in optimal format)

	// Source for the copy is the last rendered swapchain image

	// Create the linear tiled destination image to copy to and to read the memory from
	Texture copyImage;
	copyImage.Create(devices, depthTexture.width, depthTexture.height, depthTexture.format, 
		VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Do the actual blit from the swapchain image to our host visible destination image
	VkCommandBuffer copyCmd = devices->generateCommandBuffer();
	//VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	copyImage.insertImageMemoryBarrier(copyCmd, 0, VK_ACCESS_TRANSFER_WRITE_BIT, 
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });

	depthTexture.insertImageMemoryBarrier(copyCmd, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT,
		depthTexture.descriptorInfo.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });

	VkImageCopy imageCopyRegion{};
	imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageCopyRegion.srcSubresource.layerCount = 1;
	imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageCopyRegion.dstSubresource.layerCount = 1;
	imageCopyRegion.extent.width = width;
	imageCopyRegion.extent.height = height;
	imageCopyRegion.extent.depth = 1;

	// Issue the copy command
	vkCmdCopyImage(
		copyCmd,
		depthTexture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		copyImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&imageCopyRegion);
	

	copyImage.insertImageMemoryBarrier(copyCmd, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });

	depthTexture.insertImageMemoryBarrier(copyCmd, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, depthTexture.descriptorInfo.imageLayout,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });


	devices->EndCommandBuffer(copyCmd);

	// Get layout of the image (including row pitch)
	VkImageSubresource subResource{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0 };
	VkSubresourceLayout subResourceLayout;
	vkGetImageSubresourceLayout(devices->logicalDevice, copyImage.image, &subResource, &subResourceLayout);

	// Map image memory so we can start copying from it
	const char* data;
	vkMapMemory(devices->logicalDevice, copyImage.memory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
	data += subResourceLayout.offset;

	std::ofstream file(filename, std::ios::out | std::ios::binary);

	// ppm header
	file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";

	// If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
	bool colorSwizzle = false;

	// ppm binary pixel data
	for (uint32_t y = 0; y < height; y++)
	{
		unsigned int* row = (unsigned int*)data;
		for (uint32_t x = 0; x < width; x++)
		{
			file.write((char*)row, 3);
			row++;
		}
		data += subResourceLayout.rowPitch;
	}
	file.close();

	std::cout << "Screenshot saved to disk" << std::endl;

	// Clean up resources
	vkUnmapMemory(devices->logicalDevice, copyImage.memory);
	copyImage.Destroy();
}