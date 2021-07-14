#include "RayTracingRenderer.h"

#include "Initilizers.h"
#include "Utility.h"


RayTracingRenderer::~RayTracingRenderer()
{
	vkDestroyPipeline(deviceContext->logicalDevice, pipeline, nullptr);
	vkDestroyPipelineLayout(deviceContext->logicalDevice, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(deviceContext->logicalDevice, descriptorSetLayout, nullptr);
	vkDestroyImageView(deviceContext->logicalDevice, storageImage.view, nullptr);
	vkDestroyImage(deviceContext->logicalDevice, storageImage.image, nullptr);
	vkFreeMemory(deviceContext->logicalDevice, storageImage.memory, nullptr);
	vkFreeMemory(deviceContext->logicalDevice, bottomLevelAS.memory, nullptr);
	vkDestroyBuffer(deviceContext->logicalDevice, bottomLevelAS.buffer, nullptr);
	vkDestroyAccelerationStructureKHR(deviceContext->logicalDevice, bottomLevelAS.handle, nullptr);
	vkFreeMemory(deviceContext->logicalDevice, topLevelAS.memory, nullptr);
	vkDestroyBuffer(deviceContext->logicalDevice, topLevelAS.buffer, nullptr);
	vkDestroyAccelerationStructureKHR(deviceContext->logicalDevice, topLevelAS.handle, nullptr);
	vertexBuffer.Destroy();
	indexBuffer.Destroy();
	transformBuffer.Destroy();
	raygenShaderBindingTable.Destroy();
	missShaderBindingTable.Destroy();
	hitShaderBindingTable.Destroy();
	ubo.Destroy();

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

	//camera.init(core->deviceContext.get(), renderer->swapChain.extent);
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


	createBottomLevelAccelerationStructure();
	createTopLevelAccelerationStructure();

	createStorageImage();
	createUniformBuffer();
	createRayTracingPipeline();
	createShaderBindingTable();
	createDescriptorSets();
	buildCommandBuffers();

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

/*
	Create a scratch buffer to hold temporary data for a ray tracing acceleration structure
*/
RayTracingScratchBuffer RayTracingRenderer::createScratchBuffer(VkDeviceSize size)
{
	RayTracingScratchBuffer scratchBuffer{};

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	vkCreateBuffer(deviceContext->logicalDevice, &bufferCreateInfo, nullptr, &scratchBuffer.handle);

	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(deviceContext->logicalDevice, scratchBuffer.handle, &memoryRequirements);

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = Utility::findMemoryType(memoryRequirements.memoryTypeBits, deviceContext->physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(deviceContext->logicalDevice, &memoryAllocateInfo, nullptr, &scratchBuffer.memory);
	vkBindBufferMemory(deviceContext->logicalDevice, scratchBuffer.handle, scratchBuffer.memory, 0);

	VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
	bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAddressInfo.buffer = scratchBuffer.handle;
	scratchBuffer.deviceAddress = vkGetBufferDeviceAddressKHR(deviceContext->logicalDevice, &bufferDeviceAddressInfo);

	return scratchBuffer;
}

void RayTracingRenderer::deleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer)
{
	if (scratchBuffer.memory != VK_NULL_HANDLE) {
		vkFreeMemory(deviceContext->logicalDevice, scratchBuffer.memory, nullptr);
	}
	if (scratchBuffer.handle != VK_NULL_HANDLE) {
		vkDestroyBuffer(deviceContext->logicalDevice, scratchBuffer.handle, nullptr);
	}
}

void RayTracingRenderer::createAccelerationStructureBuffer(AccelerationStructure& accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
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
	Gets the deviceContext->logicalDevice address from a buffer that's required for some of the buffers used for ray tracing
*/
uint64_t RayTracingRenderer::getBufferDeviceAddress(VkBuffer buffer)
{
	VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
	bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAI.buffer = buffer;
	return vkGetBufferDeviceAddressKHR(deviceContext->logicalDevice, &bufferDeviceAI);
}

/*
	Set up a storage image that the ray generation shader will be writing to
*/
void RayTracingRenderer::createStorageImage()
{
	VkImageCreateInfo image = Initialisers::imageCreateInfo(VK_IMAGE_TYPE_2D, window->width, window->height, 1, VK_FORMAT_B8G8R8A8_UNORM,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT
	);
	//image.imageType = VK_IMAGE_TYPE_2D;
	//image.format = swapChain.colorFormat;
	//image.extent.width = width;
	//image.extent.height = height;
	//image.extent.depth = 1;
	//image.mipLevels = 1;
	//image.arrayLayers = 1;
	//image.samples = VK_SAMPLE_COUNT_1_BIT;
	//image.tiling = VK_IMAGE_TILING_OPTIMAL;
	//image.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	//image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkCreateImage(deviceContext->logicalDevice, &image, nullptr, &storageImage.image);

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(deviceContext->logicalDevice, storageImage.image, &memReqs);
	VkMemoryAllocateInfo memoryAllocateInfo = Initialisers::memoryAllocateInfo(memReqs.size, Utility::findMemoryType(memReqs.memoryTypeBits, deviceContext->physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
	//memoryAllocateInfo.allocationSize = memReqs.size;
	//memoryAllocateInfo.memoryTypeIndex = Utility::findMemoryType(memReqs.memoryTypeBits, deviceContext->physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(deviceContext->logicalDevice, &memoryAllocateInfo, nullptr, &storageImage.memory);
	vkBindImageMemory(deviceContext->logicalDevice, storageImage.image, storageImage.memory, 0);

	VkImageViewCreateInfo colorImageView = Initialisers::imageViewCreateInfo(storageImage.image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	//colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	//colorImageView.format = VK_FORMAT_B8G8R8A8_UNORM;
	//colorImageView.subresourceRange = {};
	//colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//colorImageView.subresourceRange.baseMipLevel = 0;
	//colorImageView.subresourceRange.levelCount = 1;
	//colorImageView.subresourceRange.baseArrayLayer = 0;
	//colorImageView.subresourceRange.layerCount = 1;
	//colorImageView.image = storageImage.image;
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
void RayTracingRenderer::createBottomLevelAccelerationStructure()
{
	// Setup vertices for a single triangle
	struct Vertex {
		float pos[3];
	};
	std::vector<Vertex> vertices = {
		{ {  1.0f,  1.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f } },
		{ {  0.0f, -1.0f, 0.0f } }
	};

	// Setup indices
	std::vector<uint32_t> indices = { 0, 1, 2 };
	indexCount = static_cast<uint32_t>(indices.size());

	// Setup identity transform matrix
	VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};

	// Create buffers
	// For the sake of simplicity we won't stage the vertex data to the GPU memory
	// Vertex buffer
	vertexBuffer.Create(deviceContext, vertices.size() * sizeof(Vertex),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertices.data());

	// Index buffer
	indexBuffer.Create(deviceContext, indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		indices.data());
	// Transform buffer
	transformBuffer.Create(deviceContext, sizeof(VkTransformMatrixKHR),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&transformMatrix);

	VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

	vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(vertexBuffer.vkBuffer);
	indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(indexBuffer.vkBuffer);
	transformBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(transformBuffer.vkBuffer);

	// Build
	VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
	accelerationStructureGeometry.geometry.triangles.maxVertex = 3;
	accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
	accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
	accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
	accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
	accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

	// Get size info
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

	const uint32_t numTriangles = 1;
	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	vkGetAccelerationStructureBuildSizesKHR(
		deviceContext->logicalDevice,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&numTriangles,
		&accelerationStructureBuildSizesInfo);

	createAccelerationStructureBuffer(bottomLevelAS, accelerationStructureBuildSizesInfo);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = bottomLevelAS.buffer;
	accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	vkCreateAccelerationStructureKHR(deviceContext->logicalDevice, &accelerationStructureCreateInfo, nullptr, &bottomLevelAS.handle);

	// Create a small scratch buffer used during build of the bottom level acceleration structure
	RayTracingScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS.handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	if (deviceContext->accelerationStructureFeatures.accelerationStructureHostCommands)
	{
		// Implementation supports building acceleration structure building on host
		vkBuildAccelerationStructuresKHR(
			deviceContext->logicalDevice,
			VK_NULL_HANDLE,
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());
	}
	else
	{
		// Acceleration structure needs to be build on the deviceContext->logicalDevice
		VkCommandBuffer commandBuffer = deviceContext->generateCommandBuffer();
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());
		deviceContext->EndCommandBuffer(commandBuffer);
	}

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = bottomLevelAS.handle;
	bottomLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(deviceContext->logicalDevice, &accelerationDeviceAddressInfo);

	deleteScratchBuffer(scratchBuffer);
}

/*
	The top level acceleration structure contains the scene's object instances
*/
void RayTracingRenderer::createTopLevelAccelerationStructure()
{
	VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f };

	VkAccelerationStructureInstanceKHR instance{};
	instance.transform = transformMatrix;
	instance.instanceCustomIndex = 0;
	instance.mask = 0xFF;
	instance.instanceShaderBindingTableRecordOffset = 0;
	instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	instance.accelerationStructureReference = bottomLevelAS.deviceAddress;

	// Buffer for instance data
	Buffer instancesBuffer;
	instancesBuffer.Create(deviceContext, sizeof(VkAccelerationStructureInstanceKHR),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&instance);

	VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
	instanceDataDeviceAddress.deviceAddress = getBufferDeviceAddress(instancesBuffer.vkBuffer);

	VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
	accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

	// Get size info
	/*
	The pSrcAccelerationStructure, dstAccelerationStructure, and mode members of pBuildInfo are ignored. Any VkDeviceOrHostAddressKHR members of pBuildInfo are ignored by this command, except that the hostAddress member of VkAccelerationStructureGeometryTrianglesDataKHR::transformData will be examined to check if it is NULL.*
	*/
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

	uint32_t primitive_count = 1;

	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	vkGetAccelerationStructureBuildSizesKHR(
		deviceContext->logicalDevice,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&primitive_count,
		&accelerationStructureBuildSizesInfo);

	createAccelerationStructureBuffer(topLevelAS, accelerationStructureBuildSizesInfo);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = topLevelAS.buffer;
	accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	vkCreateAccelerationStructureKHR(deviceContext->logicalDevice, &accelerationStructureCreateInfo, nullptr, &topLevelAS.handle);

	// Create a small scratch buffer used during build of the top level acceleration structure
	RayTracingScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = topLevelAS.handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = 1;
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	if (deviceContext->accelerationStructureFeatures.accelerationStructureHostCommands)
	{
		// Implementation supports building acceleration structure building on host
		vkBuildAccelerationStructuresKHR(
			deviceContext->logicalDevice,
			VK_NULL_HANDLE,
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());
	}
	else
	{
		// Acceleration structure needs to be build on the deviceContext->logicalDevice
		VkCommandBuffer commandBuffer = deviceContext->generateCommandBuffer();
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());
		deviceContext->EndCommandBuffer(commandBuffer);
	}

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = topLevelAS.handle;
	topLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(deviceContext->logicalDevice, &accelerationDeviceAddressInfo);

	deleteScratchBuffer(scratchBuffer);
	instancesBuffer.Destroy();
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
	//vulkanDevice->createBuffer(bufferUsageFlags, memoryUsageFlags, &raygenShaderBindingTable, handleSize);
	//vulkanDevice->createBuffer(bufferUsageFlags, memoryUsageFlags, &missShaderBindingTable, handleSize);
	//vulkanDevice->createBuffer(bufferUsageFlags, memoryUsageFlags, &hitShaderBindingTable, handleSize);

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
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Initialisers::descriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), 1);
	vkCreateDescriptorPool(deviceContext->logicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Initialisers::descriptorSetAllocateInfo(descriptorPool, 1, &descriptorSetLayout);
	vkAllocateDescriptorSets(deviceContext->logicalDevice, &descriptorSetAllocateInfo, &descriptorSet);

	VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
	descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
	descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAS.handle;

	VkWriteDescriptorSet accelerationStructureWrite{};
	accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	// The specialized acceleration structure descriptor has to be chained
	accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
	accelerationStructureWrite.dstSet = descriptorSet;
	accelerationStructureWrite.dstBinding = 0;
	accelerationStructureWrite.descriptorCount = 1;
	accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

	VkDescriptorImageInfo storageImageDescriptor{};
	storageImageDescriptor.imageView = storageImage.view;
	storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet resultImageWrite = Initialisers::writeDescriptorSet(descriptorSet, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &storageImageDescriptor);
	VkWriteDescriptorSet uniformBufferWrite = Initialisers::writeDescriptorSet(descriptorSet, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &ubo.descriptorInfo);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		accelerationStructureWrite,
		resultImageWrite,
		uniformBufferWrite
	};
	vkUpdateDescriptorSets(deviceContext->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
}

/*
	Create our ray tracing pipeline
*/
void RayTracingRenderer::createRayTracingPipeline()
{
	VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding{};
	accelerationStructureLayoutBinding.binding = 0;
	accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	accelerationStructureLayoutBinding.descriptorCount = 1;
	accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
	resultImageLayoutBinding.binding = 1;
	resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	resultImageLayoutBinding.descriptorCount = 1;
	resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	VkDescriptorSetLayoutBinding uniformBufferBinding{};
	uniformBufferBinding.binding = 2;
	uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferBinding.descriptorCount = 1;
	uniformBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	std::vector<VkDescriptorSetLayoutBinding> bindings({
		accelerationStructureLayoutBinding,
		resultImageLayoutBinding,
		uniformBufferBinding
		});

	VkDescriptorSetLayoutCreateInfo descriptorSetlayoutCI{};
	descriptorSetlayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetlayoutCI.bindingCount = static_cast<uint32_t>(bindings.size());
	descriptorSetlayoutCI.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(deviceContext->logicalDevice, &descriptorSetlayoutCI, nullptr, &descriptorSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
	vkCreatePipelineLayout(deviceContext->logicalDevice, &pipelineLayoutCI, nullptr, &pipelineLayout);

	/*
		Setup ray tracing shader groups
	*/
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	// Ray generation group
	{
		shaderStages.push_back(resources->raygenShaders["raygen"]->createInfo());
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		shaderGroups.push_back(shaderGroup);
	}

	// Miss group
	{
		//shaderStages.push_back(loadShader(getShadersPath() + "raytracingbasic/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));
		shaderStages.push_back(resources->raymissShaders["miss"]->createInfo());
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		shaderGroups.push_back(shaderGroup);
	}

	// Closest hit group
	{
		//shaderStages.push_back(loadShader(getShadersPath() + "raytracingbasic/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
		shaderStages.push_back(resources->rayclosesthitShaders["closesthit"]->createInfo());
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
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
	VkWriteDescriptorSet resultImageWrite = Initialisers::writeDescriptorSet(descriptorSet, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &storageImageDescriptor);
	vkUpdateDescriptorSets(deviceContext->logicalDevice, 1, &resultImageWrite, 0, VK_NULL_HANDLE);
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
		raygenShaderSbtEntry.deviceAddress = getBufferDeviceAddress(raygenShaderBindingTable.vkBuffer);
		raygenShaderSbtEntry.stride = handleSizeAligned;
		raygenShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
		missShaderSbtEntry.deviceAddress = getBufferDeviceAddress(missShaderBindingTable.vkBuffer);
		missShaderSbtEntry.stride = handleSizeAligned;
		missShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
		hitShaderSbtEntry.deviceAddress = getBufferDeviceAddress(hitShaderBindingTable.vkBuffer);
		hitShaderSbtEntry.stride = handleSizeAligned;
		hitShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

		/*
			Dispatch the ray tracing commands
		*/
		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSet, 0, 0);

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

		Utility::setImageLayout(drawCmdBuffers[i],
			swapChain.images[i].image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		// Prepare ray tracing output image as transfer source
		//vks::tools::setImageLayout(
		//	drawCmdBuffers[i],
		//	storageImage.image,
		//	VK_IMAGE_LAYOUT_GENERAL,
		//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		//	subresourceRange);

		Utility::setImageLayout(drawCmdBuffers[i],
			storageImage.image,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			subresourceRange);


		VkImageCopy copyRegion{};
		copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.srcOffset = { 0, 0, 0 };
		copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.dstOffset = { 0, 0, 0 };
		copyRegion.extent = { static_cast<uint32_t>(window->width), static_cast<uint32_t>(window->height), 1 };
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