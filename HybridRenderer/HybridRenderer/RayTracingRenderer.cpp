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
	Window* _window, SwapChain* swapChain, Resources* _resources, Scene* scene) {


	deviceContext = _deviceContext;
	resources = _resources;
	window = _window;
	this->swapChain = swapChain;

	drawCmdBuffers.resize(3);

	VkCommandBufferAllocateInfo allocInfo = Initialisers::commandBufferAllocateInfo(deviceContext->commandPool, static_cast<uint32_t>(drawCmdBuffers.size()));

	if (vkAllocateCommandBuffers(deviceContext->logicalDevice, &allocInfo, drawCmdBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate command buffers!");

	vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkGetBufferDeviceAddressKHR"));
	vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkCmdBuildAccelerationStructuresKHR"));
	vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkBuildAccelerationStructuresKHR"));
	vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkCreateAccelerationStructureKHR"));
	vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkDestroyAccelerationStructureKHR"));
	vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkGetAccelerationStructureBuildSizesKHR"));
	vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkGetAccelerationStructureDeviceAddressKHR"));
	vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkCmdTraceRaysKHR"));
	vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkGetRayTracingShaderGroupHandlesKHR"));
	vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkCreateRayTracingPipelinesKHR"));

	//auto model = resources->GetModel("tree2");
	blas.reserve(scene->gameObjectCount * 2);
	for (auto& go : scene->gameObjects)
	{
		auto model = go.model;
		for (auto& mesh : model->meshes)
		{
			AccelerationStructure as;
			as.Initialise(deviceContext);
			as.createBottomLevelAccelerationStructure(go.transform, mesh.get());
			blas.emplace_back(as);
		}
	}
	//for (size_t i = 0; i < 2; i++)
	//{
	//	blas[i].Initialise(deviceContext);
	//	blas[i].createBottomLevelAccelerationStructure(model->meshes[i].get());
	//}

	createStorageImage();
	createUniformBuffer();
	CreateRayTracingPipeline();
	CreateShaderBindingTable();
	
	tlas.Initialise(deviceContext);
	tlas.createTopLevelAccelerationStructure(blas);
	CreateDescriptorSets();
	buildCommandBuffers();
}


void RayTracingRenderer::cleanup() {

	vkDeviceWaitIdle(deviceContext->logicalDevice);
	vkFreeCommandBuffers(deviceContext->logicalDevice, deviceContext->commandPool,
		static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());

	vkDestroyPipeline(deviceContext->logicalDevice, pipeline, nullptr);
	vkDestroyPipelineLayout(deviceContext->logicalDevice, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(deviceContext->logicalDevice, descriptorSetLayout, nullptr);
	//vkDestroyImageView(deviceContext->logicalDevice, storageImage.view, nullptr);
	//vkDestroyImage(deviceContext->logicalDevice, storageImage.image, nullptr);
	//vkFreeMemory(deviceContext->logicalDevice, storageImage.memory, nullptr);

	storageImage.Destroy();

	vkDestroyDescriptorPool(deviceContext->logicalDevice, descriptorPool, nullptr);

	tlas.Destroy();
	for (auto& bla : blas)
	{
		bla.Destroy();
	}
	raygenShaderBindingTable.Destroy();
	missShaderBindingTable.Destroy();
	hitShaderBindingTable.Destroy();
	ubo.Destroy();
}
void RayTracingRenderer::GetCommandBuffers(uint32_t imageIndex, std::vector<VkCommandBuffer>& submitCommandBuffers)
{
	if (!commandBuffersReady)
		buildCommandBuffers();

	submitCommandBuffers.emplace_back(drawCmdBuffers[imageIndex]);
}

/*
	Set up a storage image that the ray generation shader will be writing to
*/
void RayTracingRenderer::createStorageImage()
{

	storageImage.Create(deviceContext, window->width, window->height, swapChain->imageFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_SAMPLE_COUNT_1_BIT
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
	const uint32_t handleSizeAligned = Utility::alignedSize(deviceContext->rayTracingPipelineProperties.shaderGroupHandleSize, deviceContext->rayTracingPipelineProperties.shaderGroupHandleAlignment);
	const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
	const uint32_t sbtSize = groupCount * handleSizeAligned;

	std::vector<uint8_t> shaderHandleStorage(sbtSize);
	vkGetRayTracingShaderGroupHandlesKHR(deviceContext->logicalDevice, pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data());

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
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 3 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 }
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Initialisers::descriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), 9);
	vkCreateDescriptorPool(deviceContext->logicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool);

	descriptorSets.resize(deviceContext->imageCount);
	std::vector<VkDescriptorSetLayout> layouts(deviceContext->imageCount, descriptorSetLayout);
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = 
		Initialisers::descriptorSetAllocateInfo(descriptorPool, deviceContext->imageCount, layouts.data());
	vkAllocateDescriptorSets(deviceContext->logicalDevice, &descriptorSetAllocateInfo, descriptorSets.data());

	VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo =
		Initialisers::descriptorSetAccelerationStructureInfo(&tlas.handle);
	VkDescriptorImageInfo storageImageDescriptor{};
	storageImageDescriptor.imageView = storageImage.imageView;
	storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	for (size_t i = 0; i < deviceContext->imageCount; i++)
	{
		VkWriteDescriptorSet accelerationStructureWrite = 
			Initialisers::writeDescriptorSet(descriptorSets[i], 0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, &descriptorAccelerationStructureInfo);
		VkWriteDescriptorSet resultImageWrite = 
			Initialisers::writeDescriptorSet(descriptorSets[i], 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &storageImageDescriptor);
		VkWriteDescriptorSet uniformBufferWrite = 
			Initialisers::writeDescriptorSet(descriptorSets[i], 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &ubo.descriptorInfo);

		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			accelerationStructureWrite,
			resultImageWrite,
			uniformBufferWrite
		};
		vkUpdateDescriptorSets(deviceContext->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
	}
}

/*
	Create our ray tracing pipeline
*/
void RayTracingRenderer::CreateRayTracingPipeline()
{
	VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding = 
		Initialisers::descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

	VkDescriptorSetLayoutBinding resultImageLayoutBinding = 
		Initialisers::descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

	VkDescriptorSetLayoutBinding uniformBufferBinding = 
		Initialisers::descriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

	std::vector<VkDescriptorSetLayoutBinding> bindings({
		accelerationStructureLayoutBinding,
		resultImageLayoutBinding,
		uniformBufferBinding
		});

	VkDescriptorSetLayoutCreateInfo descriptorSetlayoutCI = Initialisers::descriptorSetLayoutCreateInfo(bindings.data(), static_cast<uint32_t>(bindings.size()));
	vkCreateDescriptorSetLayout(deviceContext->logicalDevice, &descriptorSetlayoutCI, nullptr, &descriptorSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutCI = Initialisers::pipelineLayoutCreateInfo(&descriptorSetLayout);
	vkCreatePipelineLayout(deviceContext->logicalDevice, &pipelineLayoutCI, nullptr, &pipelineLayout);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	// Ray generation group
	{
		shaderStages.push_back(resources->GetShader("raytracing/raygen", VK_SHADER_STAGE_RAYGEN_BIT_KHR)->createInfo());
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup = Initialisers::rayTracingGeneralShaderGroup(static_cast<uint32_t>(shaderStages.size()) - 1);
		shaderGroups.push_back(shaderGroup);
	}

	// Miss group
	{
		shaderStages.push_back(resources->GetShader("raytracing/miss", VK_SHADER_STAGE_MISS_BIT_KHR)->createInfo());
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup = Initialisers::rayTracingGeneralShaderGroup(static_cast<uint32_t>(shaderStages.size()) - 1);
		shaderGroups.push_back(shaderGroup);
	}

	// Closest hit group
	{
		shaderStages.push_back(resources->GetShader("raytracing/closesthit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)->createInfo());
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup = Initialisers::rayTracingClosestHitShaderGroup(static_cast<uint32_t>(shaderStages.size()) - 1);
		shaderGroups.push_back(shaderGroup);
	}

	/*
		Create the ray tracing pipeline
	*/
	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI = Initialisers::RayTracingPipelineCreateInfo(pipelineLayout, 
		shaderStages.data(), static_cast<uint32_t>(shaderStages.size()), shaderGroups.data(), static_cast<uint32_t>(shaderGroups.size()));

	vkCreateRayTracingPipelinesKHR(deviceContext->logicalDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline);
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
	// Update descriptor
	VkDescriptorImageInfo storageImageDescriptor{ VK_NULL_HANDLE, storageImage.imageView, VK_IMAGE_LAYOUT_GENERAL };
	for (size_t i = 0; i < deviceContext->imageCount; i++)
	{
		VkWriteDescriptorSet resultImageWrite = Initialisers::writeDescriptorSet(descriptorSets[i], 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &storageImageDescriptor);
		vkUpdateDescriptorSets(deviceContext->logicalDevice, 1, &resultImageWrite, 0, VK_NULL_HANDLE);
	}
	buildCommandBuffers();
}

void RayTracingRenderer::buildCommandBuffers()
{
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

		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSets[i], 0, 0);

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
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ImGUI::enabled ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
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
	uniformData.projInverse = glm::inverse(camera->projection);
	uniformData.viewInverse = glm::inverse(camera->view);
	memcpy(ubo.data, &uniformData, sizeof(uniformData));
}