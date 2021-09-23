#include "RayTracingRenderer.h"

#include "Initilizers.h"
#include "Utility.h"
#include "ImGUI_.h"


RayTracingRenderer::RayTracingRenderer(VulkanCore* core, Window* _window)
{
	
}

RayTracingRenderer::~RayTracingRenderer()
{

}

void RayTracingRenderer::Initialise(DeviceContext* _deviceContext, 
	Window* _window, SwapChain* swapChain, Resources* _resources) {


	deviceContext = _deviceContext;
	resources = _resources;
	window = _window;
	this->swapChain = swapChain;

	drawCmdBuffers.resize(3);

	VkCommandBufferAllocateInfo allocInfo = Initialisers::commandBufferAllocateInfo(deviceContext->commandPool, static_cast<uint32_t>(drawCmdBuffers.size()));

	if (vkAllocateCommandBuffers(deviceContext->logicalDevice, &allocInfo, drawCmdBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate command buffers!");

	vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkCmdTraceRaysKHR"));
	vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkGetRayTracingShaderGroupHandlesKHR"));

	createStorageImage();
	createUniformBuffer();
	CreateRayTracingPipeline();
	CreateShaderBindingTable();
	CreateDescriptorSets();

}


void RayTracingRenderer::Deinitialise() {

	vkDeviceWaitIdle(deviceContext->logicalDevice);
	vkFreeCommandBuffers(deviceContext->logicalDevice, deviceContext->commandPool,
		static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());

	storageImage.Destroy();
	rtPipeline.Destroy();

	raygenShaderBindingTable.Destroy();
	missShaderBindingTable.Destroy();
	hitShaderBindingTable.Destroy();
	ubo.Destroy();
}

void RayTracingRenderer::GetCommandBuffers(uint32_t imageIndex, std::vector<VkCommandBuffer>& submitCommandBuffers, Scene* scene)
{
	if (!commandBuffersReady)
		buildCommandBuffers(scene);

	submitCommandBuffers.emplace_back(drawCmdBuffers[imageIndex]);
}

/*
	Set up a storage image that the ray generation shader will be writing to
*/
void RayTracingRenderer::createStorageImage()
{

	storageImage.Create(deviceContext, window->width, window->height, swapChain->imageFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, 
		VK_SAMPLE_COUNT_1_BIT
	);
	storageImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

	VkCommandBuffer cmdBuffer = deviceContext->generateCommandBuffer();
	Utility::setImageLayout(cmdBuffer, storageImage.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
	deviceContext->EndCommandBuffer(cmdBuffer);
}

/*
	Create the Shader Binding Tables that binds the programs and top-level acceleration structure

	SBT Layout used in this sample:

		/-----------\
		| raygen    |
		|-----------|
		| miss      |
		|-----------|
		| hit       |
		\-----------/

*/
void RayTracingRenderer::CreateShaderBindingTable() {

	const uint32_t handleSize = deviceContext->rayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t handleSizeAligned = 
		Utility::alignedSize(deviceContext->rayTracingPipelineProperties.shaderGroupHandleSize, 
			deviceContext->rayTracingPipelineProperties.shaderGroupHandleAlignment);
	const uint32_t groupCount = static_cast<uint32_t>(rtPipeline.shaderGroups.size());
	const uint32_t sbtSize = groupCount * handleSizeAligned;

	std::vector<uint8_t> shaderHandleStorage(sbtSize);
	vkGetRayTracingShaderGroupHandlesKHR(deviceContext->logicalDevice, rtPipeline.pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data());

	const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	raygenShaderBindingTable.Create(deviceContext, handleSize, bufferUsageFlags, memoryUsageFlags);
	missShaderBindingTable.Create(deviceContext, handleSize, bufferUsageFlags, memoryUsageFlags);
	hitShaderBindingTable.Create(deviceContext, handleSize, bufferUsageFlags, memoryUsageFlags);

	// Copy handles
	raygenShaderBindingTable.Map();
	missShaderBindingTable.Map();
	hitShaderBindingTable.Map();
	memcpy(raygenShaderBindingTable.data, shaderHandleStorage.data(), handleSize);
	memcpy(missShaderBindingTable.data, shaderHandleStorage.data() + handleSizeAligned, handleSize);
	memcpy(hitShaderBindingTable.data, shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
}

/*
	Create the descriptor sets used for the ray tracing dispatch
*/
void RayTracingRenderer::CreateDescriptorSets()
{

	VkDescriptorImageInfo storageImageDescriptor{};
	storageImageDescriptor.imageView = storageImage.imageView;
	storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	DescriptorSetRequest request{};
	request.ids.push_back({0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR });
	request.ids.push_back({1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR });

	request.data.reserve(request.ids.size() * deviceContext->imageCount);
	for (size_t i = 0; i < deviceContext->imageCount; i++)
	{
		request.data.push_back(&storageImageDescriptor);
		request.data.push_back(&ubo.descriptorInfo);
	}

	deviceContext->GetDescriptors(rtDescriptor, &request);
}

/*
	Create our ray tracing pipeline
*/
void RayTracingRenderer::CreateRayTracingPipeline()
{
	RTPipelineInfo pipelineInfo{};
	pipelineInfo.shaders =
	{ resources->GetShader("raytracing/raygen", VK_SHADER_STAGE_RAYGEN_BIT_KHR),
		resources->GetShader("raytracing/miss", VK_SHADER_STAGE_MISS_BIT_KHR),
		resources->GetShader("raytracing/closesthit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
	};

	rtPipeline.Create(deviceContext, pipelineInfo);
}

/*
	Create the uniform buffer used to pass matrices to the ray tracing ray generation shader
*/
void RayTracingRenderer::createUniformBuffer()
{
	ubo.Create(deviceContext, sizeof(uniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&ubo);

	ubo.Map();
}

/*
	If the window has been resized, we need to recreate the storage image and it's descriptor
*/
void RayTracingRenderer::Reinitialise()
{
	// Delete allocated resources
	storageImage.Destroy();
	// Recreate image
	createStorageImage();
	
	VkDescriptorImageInfo storageImageDescriptor{};
	storageImageDescriptor.imageView = storageImage.imageView;
	storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	DescriptorSetRequest request{};
	request.ids.push_back({ 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR });
	request.ids.push_back({ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR });

	request.data.reserve(request.ids.size() * deviceContext->imageCount);
	for (size_t i = 0; i < deviceContext->imageCount; i++)
	{
		request.data.push_back(&storageImageDescriptor);
		request.data.push_back(&ubo.descriptorInfo);
	}
	rtDescriptor.requestData = request;
	deviceContext->GetDescriptors(rtDescriptor);

	commandBuffersReady = false;
}

void RayTracingRenderer::buildCommandBuffers(Scene* scene)
{
	vkQueueWaitIdle(deviceContext->presentQueue);
	vkDeviceWaitIdle(deviceContext->logicalDevice);
	VkCommandBufferBeginInfo cmdBufInfo = Initialisers::commandBufferBeginInfo();

	VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
	{
		vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo);

		const uint32_t handleSizeAligned = Utility::alignedSize(deviceContext->rayTracingPipelineProperties.shaderGroupHandleSize, deviceContext->rayTracingPipelineProperties.shaderGroupHandleAlignment);

		VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry = Initialisers::StridedDeviceAddressRegion(raygenShaderBindingTable.GetDeviceAddress(), handleSizeAligned, handleSizeAligned);
		VkStridedDeviceAddressRegionKHR missShaderSbtEntry = Initialisers::StridedDeviceAddressRegion(missShaderBindingTable.GetDeviceAddress(), handleSizeAligned, handleSizeAligned);
		VkStridedDeviceAddressRegionKHR hitShaderSbtEntry = Initialisers::StridedDeviceAddressRegion(hitShaderBindingTable.GetDeviceAddress(), handleSizeAligned, handleSizeAligned);
		VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtPipeline.pipeline);
		std::vector<VkDescriptorSet> descriptors{rtDescriptor.sets[i], scene->rtASDescriptor.sets[i]};

		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtPipeline.pipelineLayout, 0, descriptors.size(), descriptors.data(), 0, 0);

		vkCmdTraceRaysKHR(drawCmdBuffers[i],
			&raygenShaderSbtEntry, &missShaderSbtEntry,
			&hitShaderSbtEntry, &callableShaderSbtEntry,
			window->width, window->height,
			1);

		Utility::setImageLayout(drawCmdBuffers[i], swapChain->images[i].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		Utility::setImageLayout(drawCmdBuffers[i], storageImage.image,
			VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			subresourceRange);

		VkImageCopy copyRegion = Initialisers::imageCopy(static_cast<uint32_t>(window->width), static_cast<uint32_t>(window->height));
		vkCmdCopyImage(drawCmdBuffers[i], storageImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			swapChain->images[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		Utility::setImageLayout(drawCmdBuffers[i], swapChain->images[i].image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			//VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			ImGUI::enabled ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			subresourceRange);

		Utility::setImageLayout(drawCmdBuffers[i], storageImage.image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
			subresourceRange);

		vkEndCommandBuffer(drawCmdBuffers[i]);
	}

	commandBuffersReady = true;
}

void RayTracingRenderer::updateUniformBuffers(Camera* camera)
{
	uniformData.projInverse = glm::inverse(camera->gpuData.projection);
	uniformData.viewInverse = glm::inverse(camera->gpuData.view);
	memcpy(ubo.data, &uniformData, sizeof(uniformData));
}