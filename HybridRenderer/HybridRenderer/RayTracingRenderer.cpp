#include "RayTracingRenderer.h"

#include "Initilizers.h"
#include "Utility.h"


RayTracingRenderer::~RayTracingRenderer()
{

}

void RayTracingRenderer::initialise(DeviceContext* _deviceContext, VkSurfaceKHR surface, Window* _window, Resources* _resources) {


	deviceContext = _deviceContext;
	resources = _resources;
	window = _window;

	swapChain.Create(surface, deviceContext, &window->width, &window->height);

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapChain.imageCount, VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = Initialisers::semaphoreCreateInfo();

	VkFenceCreateInfo fenceInfo = Initialisers::fenceCreateInfo();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(deviceContext->logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(deviceContext->logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(deviceContext->logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

	camera.lookAt = glm::vec3(0, 0, 0);
	camera.transform.position = glm::vec3(0, 0, 10);
	camera.transform.rotation.y = 180.f;

	camera.init(deviceContext, swapChain.extent);

	camera.update(window->width, window->height);

	//rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	//VkPhysicalDeviceProperties2 deviceProperties2{};
	//deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	//deviceProperties2.pNext = &rayTracingPipelineProperties;
	//vkGetPhysicalDeviceProperties2(deviceContext->physicalDevice, &deviceProperties2);

	//// Get acceleration structure properties, which will be used later on in the sample
	//accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	//VkPhysicalDeviceFeatures2 deviceFeatures2{};
	//deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	//deviceFeatures2.pNext = &accelerationStructureFeatures;
	//vkGetPhysicalDeviceFeatures2(deviceContext->physicalDevice, &deviceFeatures2);

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

	bottomLevelASs.resize(2);
	auto model = resources->GetModel("tree2");
	blas.resize(2);
	for (size_t i = 0; i < 2; i++)
	{
		blas[i].Initialise(deviceContext);
		blas[i].createBottomLevelAccelerationStructure(model->meshes[i].get());
	}
	tlas.Initialise(deviceContext);
	tlas.createTopLevelAccelerationStructure(blas);
//	createTopLevelAccelerationStructure();

	createStorageImage();
	createUniformBuffer();
	createRayTracingPipeline();
	createShaderBindingTable();
	createDescriptorSets();
	buildCommandBuffers();

}


void RayTracingRenderer::cleanup() {
	vkDestroyPipeline(deviceContext->logicalDevice, pipeline, nullptr);
	vkDestroyPipelineLayout(deviceContext->logicalDevice, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(deviceContext->logicalDevice, descriptorSetLayout, nullptr);
	vkDestroyImageView(deviceContext->logicalDevice, storageImage.view, nullptr);
	vkDestroyImage(deviceContext->logicalDevice, storageImage.image, nullptr);
	vkFreeMemory(deviceContext->logicalDevice, storageImage.memory, nullptr);
	vkFreeMemory(deviceContext->logicalDevice, bottomLevelAS.memory, nullptr);
	vkDestroyBuffer(deviceContext->logicalDevice, bottomLevelAS.buffer, nullptr);
	vkDestroyAccelerationStructureKHR(deviceContext->logicalDevice, bottomLevelAS.handle, nullptr);
	//vkFreeMemory(deviceContext->logicalDevice, topLevelAS.memory, nullptr);
	//vkDestroyBuffer(deviceContext->logicalDevice, topLevelAS.buffer, nullptr);
	//vkDestroyAccelerationStructureKHR(deviceContext->logicalDevice, topLevelAS.handle, nullptr);
	swapChain.Destroy();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(deviceContext->logicalDevice, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(deviceContext->logicalDevice, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(deviceContext->logicalDevice, inFlightFences[i], nullptr);
	}

	vkDestroyDescriptorPool(deviceContext->logicalDevice, descriptorPool, nullptr);

	vertexBuffer.Destroy();
	indexBuffer.Destroy();
	transformBuffer.Destroy();
	raygenShaderBindingTable.Destroy();
	missShaderBindingTable.Destroy();
	hitShaderBindingTable.Destroy();
	ubo.Destroy();
}

void RayTracingRenderer::render()
{

	VkSemaphore iAS = imageAvailableSemaphores[currentFrame];

	auto result = vkAcquireNextImageKHR(deviceContext->logicalDevice, swapChain.vkSwapChain, UINT64_MAX, iAS, VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	vkWaitForFences(deviceContext->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(deviceContext->logicalDevice, 1, &inFlightFences[currentFrame]);


	std::vector<VkCommandBuffer> submitCommandBuffers =
	{ drawCmdBuffers[imageIndex] };

	VkSemaphore rFS = renderFinishedSemaphores[currentFrame];
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = Initialisers::submitInfo(
		submitCommandBuffers.data(), static_cast<uint32_t>(submitCommandBuffers.size()), &iAS, 1, &rFS, 1, waitStages);


	if (vkQueueSubmit(deviceContext->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkSwapchainKHR swapChains[] = { swapChain.vkSwapChain };
	VkPresentInfoKHR presentInfo = Initialisers::presentInfoKHR(&rFS, 1, swapChains, 1, &imageIndex);
	result = vkQueuePresentKHR(deviceContext->presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		//rebuildSwapChain = false;
		//recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RayTracingRenderer::createAccelerationStructureBuffer(AccelerationStructure2& accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	vkCreateBuffer(deviceContext->logicalDevice, &bufferCreateInfo, nullptr, &accelerationStructure.buffer);
	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(deviceContext->logicalDevice, accelerationStructure.buffer, &memoryRequirements);
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = Utility::findMemoryType(memoryRequirements.memoryTypeBits, deviceContext->physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(deviceContext->logicalDevice, &memoryAllocateInfo, nullptr, &accelerationStructure.memory);
	vkBindBufferMemory(deviceContext->logicalDevice, accelerationStructure.buffer, accelerationStructure.memory, 0);
}

/*
	Set up a storage image that the ray generation shader will be writing to
*/
void RayTracingRenderer::createStorageImage()
{
	VkImageCreateInfo image = Initialisers::imageCreateInfo(VK_IMAGE_TYPE_2D, window->width, window->height, 1, swapChain.imageFormat,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT
	);

	vkCreateImage(deviceContext->logicalDevice, &image, nullptr, &storageImage.image);

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(deviceContext->logicalDevice, storageImage.image, &memReqs);
	VkMemoryAllocateInfo memoryAllocateInfo = Initialisers::memoryAllocateInfo(memReqs.size, Utility::findMemoryType(memReqs.memoryTypeBits, deviceContext->physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
	vkAllocateMemory(deviceContext->logicalDevice, &memoryAllocateInfo, nullptr, &storageImage.memory);
	vkBindImageMemory(deviceContext->logicalDevice, storageImage.image, storageImage.memory, 0);

	VkImageViewCreateInfo colorImageView = Initialisers::imageViewCreateInfo(storageImage.image, VK_IMAGE_VIEW_TYPE_2D, swapChain.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(deviceContext->logicalDevice, &colorImageView, nullptr, &storageImage.view);

	VkCommandBuffer cmdBuffer = deviceContext->generateCommandBuffer();
	Utility::setImageLayout(cmdBuffer, storageImage.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
	deviceContext->EndCommandBuffer(cmdBuffer);
}

/*
	Create the bottom level acceleration structure contains the scene's actual geometry (vertices, triangles)
*/

/*
	The top level acceleration structure contains the scene's object instances
*/

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
void RayTracingRenderer::createShaderBindingTable() {

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
void RayTracingRenderer::createDescriptorSets()
{
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 3 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 }
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Initialisers::descriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), 1);
	vkCreateDescriptorPool(deviceContext->logicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool);

	//descriptorSets.resize(swapChain.imageCount);
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Initialisers::descriptorSetAllocateInfo(descriptorPool, 1, &descriptorSetLayout);
	vkAllocateDescriptorSets(deviceContext->logicalDevice, &descriptorSetAllocateInfo, &descriptorSets);

	//for (size_t i = 0; i < swapChain.imageCount; i++)
	//{
		VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo = Initialisers::descriptorSetAccelerationStructureInfo(&tlas.handle);

		VkWriteDescriptorSet accelerationStructureWrite = Initialisers::writeDescriptorSet(descriptorSets, 0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, &descriptorAccelerationStructureInfo);

		VkDescriptorImageInfo storageImageDescriptor{};
		storageImageDescriptor.imageView = storageImage.view;
		storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet resultImageWrite = Initialisers::writeDescriptorSet(descriptorSets, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &storageImageDescriptor);
		VkWriteDescriptorSet uniformBufferWrite = Initialisers::writeDescriptorSet(descriptorSets, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &ubo.descriptorInfo);

		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			accelerationStructureWrite,
			resultImageWrite,
			uniformBufferWrite
		};
		vkUpdateDescriptorSets(deviceContext->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
	//}
}

/*
	Create our ray tracing pipeline
*/
void RayTracingRenderer::createRayTracingPipeline()
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

	/*
		Setup ray tracing shader groups
	*/
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
	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
	rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	rayTracingPipelineCI.pStages = shaderStages.data();
	rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups.size());
	rayTracingPipelineCI.pGroups = shaderGroups.data();
	rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;
	rayTracingPipelineCI.layout = pipelineLayout;
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
	//VK_CHECK_RESULT(vulkanDevice->createBuffer(
	//	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	//	&ubo,
	//	sizeof(uniformData),
	//	&uniformData));
	//VK_CHECK_RESULT(ubo.map());

	updateUniformBuffers();
}

/*
	If the window has been resized, we need to recreate the storage image and it's descriptor
*/
void RayTracingRenderer::handleResize()
{
	// Delete allocated resources
	vkDestroyImageView(deviceContext->logicalDevice, storageImage.view, nullptr);
	vkDestroyImage(deviceContext->logicalDevice, storageImage.image, nullptr);
	vkFreeMemory(deviceContext->logicalDevice, storageImage.memory, nullptr);
	// Recreate image
	createStorageImage();
	// Update descriptor
	VkDescriptorImageInfo storageImageDescriptor{ VK_NULL_HANDLE, storageImage.view, VK_IMAGE_LAYOUT_GENERAL };
	//for (size_t i = 0; i < swapChain.imageCount; i++)
	//{
		VkWriteDescriptorSet resultImageWrite = Initialisers::writeDescriptorSet(descriptorSets, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &storageImageDescriptor);
		vkUpdateDescriptorSets(deviceContext->logicalDevice, 1, &resultImageWrite, 0, VK_NULL_HANDLE);
	//}
}

/*
	Command buffer generation
*/
void RayTracingRenderer::buildCommandBuffers()
{
	if (resized)
	{
		handleResize();
	}

	VkCommandBufferBeginInfo cmdBufInfo = Initialisers::commandBufferBeginInfo();

	VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
	{
		vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo);

		/*
			Setup the buffer regions pointing to the shaders in our shader binding table
		*/

		const uint32_t handleSizeAligned = Utility::alignedSize(deviceContext->rayTracingPipelineProperties.shaderGroupHandleSize, deviceContext->rayTracingPipelineProperties.shaderGroupHandleAlignment);

		VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
		raygenShaderSbtEntry.deviceAddress = raygenShaderBindingTable.GetDeviceAddress();
		raygenShaderSbtEntry.stride = handleSizeAligned;
		raygenShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
		missShaderSbtEntry.deviceAddress = missShaderBindingTable.GetDeviceAddress();
		missShaderSbtEntry.stride = handleSizeAligned;
		missShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
		hitShaderSbtEntry.deviceAddress = hitShaderBindingTable.GetDeviceAddress();
		hitShaderSbtEntry.stride = handleSizeAligned;
		hitShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

		/*
			Dispatch the ray tracing commands
		*/
		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSets, 0, 0);

		vkCmdTraceRaysKHR(
			drawCmdBuffers[i],
			&raygenShaderSbtEntry,
			&missShaderSbtEntry,
			&hitShaderSbtEntry,
			&callableShaderSbtEntry,
			window->width,
			window->height,
			1);

		/*
			Copy ray tracing output to swap chain image
		*/


		// Prepare current swap chain image as transfer destination
		//vks::tools::setImageLayout(
		//	drawCmdBuffers[i],
		//	swapChain.images[i],
		//	VK_IMAGE_LAYOUT_UNDEFINED,
		//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	subresourceRange);

		// Prepare ray tracing output image as transfer source
		//vks::tools::setImageLayout(
		//	drawCmdBuffers[i],
		//	storageImage.image,
		//	VK_IMAGE_LAYOUT_GENERAL,
		//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		//	subresourceRange);

		Utility::setImageLayout(drawCmdBuffers[i],
			swapChain.images[i].image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		Utility::setImageLayout(drawCmdBuffers[i],
			storageImage.image,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			subresourceRange);


		VkImageCopy copyRegion = Initialisers::imageCopy(static_cast<uint32_t>(window->width), static_cast<uint32_t>(window->height));
		vkCmdCopyImage(drawCmdBuffers[i], storageImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapChain.images[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);


		Utility::setImageLayout(drawCmdBuffers[i],
			swapChain.images[i].image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			subresourceRange);

		Utility::setImageLayout(drawCmdBuffers[i],
			storageImage.image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_GENERAL,
			subresourceRange);

		//// Transition swap chain image back for presentation
		//vks::tools::setImageLayout(
		//	drawCmdBuffers[i],
		//	swapChain.images[i],
		//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		//	subresourceRange);

		//// Transition ray tracing output image back to general layout
		//vks::tools::setImageLayout(
		//	drawCmdBuffers[i],
		//	storageImage.image,
		//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		//	VK_IMAGE_LAYOUT_GENERAL,
		//	subresourceRange);

		vkEndCommandBuffer(drawCmdBuffers[i]);
	}
}

void RayTracingRenderer::updateUniformBuffers()
{
	uniformData.projInverse = glm::inverse(camera.projection);
	uniformData.viewInverse = glm::inverse(camera.view);
	memcpy(ubo.data, &uniformData, sizeof(uniformData));
}