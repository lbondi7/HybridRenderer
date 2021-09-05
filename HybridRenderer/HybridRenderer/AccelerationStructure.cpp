#include "AccelerationStructure.h"

#include "Utility.h"
#include "Initilizers.h"

AccelerationStructure::~AccelerationStructure()
{
}

void AccelerationStructure::Initialise(DeviceContext* _deviceContext)
{
	deviceContext = _deviceContext;

	vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkGetBufferDeviceAddressKHR"));
	vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkCmdBuildAccelerationStructuresKHR"));
	vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkBuildAccelerationStructuresKHR"));
	vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkCreateAccelerationStructureKHR"));
	vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkDestroyAccelerationStructureKHR"));
	vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkGetAccelerationStructureBuildSizesKHR"));
	vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(deviceContext->logicalDevice, "vkGetAccelerationStructureDeviceAddressKHR"));
}

void AccelerationStructure::Destroy() {
	
	vkDestroyAccelerationStructureKHR(deviceContext->logicalDevice, handle, nullptr);
	asBuffer.Destroy();
	if (type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) 
	{
		vertexBuffer.Destroy();
		indexBuffer.Destroy();
	}
	transformBuffer.Destroy();
}

ScratchBuffer AccelerationStructure::createScratchBuffer(VkDeviceSize size)
{
	ScratchBuffer scratchBuffer{};

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

void AccelerationStructure::deleteScratchBuffer(ScratchBuffer& scratchBuffer)
{
	if (scratchBuffer.memory != VK_NULL_HANDLE) {
		vkFreeMemory(deviceContext->logicalDevice, scratchBuffer.memory, nullptr);
	}
	if (scratchBuffer.handle != VK_NULL_HANDLE) {
		vkDestroyBuffer(deviceContext->logicalDevice, scratchBuffer.handle, nullptr);
	}
}

void AccelerationStructure::createAccelerationStructureBuffer(VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
{
	asBuffer.Create(deviceContext, buildSizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	//VkBufferCreateInfo bufferCreateInfo{};
	//bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
	//bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	//vkCreateBuffer(deviceContext->logicalDevice, &bufferCreateInfo, nullptr, &buffer);
	//VkMemoryRequirements memoryRequirements{};
	//vkGetBufferMemoryRequirements(deviceContext->logicalDevice, buffer, &memoryRequirements);
	//VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	//memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	//memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	//VkMemoryAllocateInfo memoryAllocateInfo{};
	//memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	//memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	//memoryAllocateInfo.allocationSize = memoryRequirements.size;
	//memoryAllocateInfo.memoryTypeIndex = Utility::findMemoryType(memoryRequirements.memoryTypeBits, deviceContext->physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	//vkAllocateMemory(deviceContext->logicalDevice, &memoryAllocateInfo, nullptr, &memory);
	//vkBindBufferMemory(deviceContext->logicalDevice, buffer, memory, 0);
}


/*
	Create the bottom level acceleration structure contains the scene's actual geometry (vertices, triangles)
*/
void AccelerationStructure::createBottomLevelAccelerationStructure(Mesh* mesh)
{
	type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

	// Setup identity transform matrix
	VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};

	// Create buffers
	// For the sake of simplicity we won't stage the vertex data to the GPU memory
	// Vertex buffer
	vertexBuffer.Create(deviceContext, mesh->vertices.size() * sizeof(Vertex),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mesh->vertices.data());

	// Index buffer
	indexBuffer.Create(deviceContext, mesh->indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		mesh->indices.data());
	// Transform buffer
	transformBuffer.Create(deviceContext, sizeof(VkTransformMatrixKHR),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&transformMatrix);

	VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

	vertexBufferDeviceAddress.deviceAddress = vertexBuffer.GetDeviceAddress();
	indexBufferDeviceAddress.deviceAddress = indexBuffer.GetDeviceAddress();
	transformBufferDeviceAddress.deviceAddress = transformBuffer.GetDeviceAddress();

	VkAccelerationStructureGeometryTrianglesDataKHR triangles = Initialisers::ASGTriangleData(vertexBufferDeviceAddress,
		sizeof(Vertex), mesh->vertices.size(), indexBufferDeviceAddress, transformBufferDeviceAddress);

	// Build
	VkAccelerationStructureGeometryKHR accelerationStructureGeometry =
		Initialisers::ASG(triangles);

	// Get size info
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = Initialisers::BLABuildGeometryInfo(&accelerationStructureGeometry);

	const uint32_t numTriangles = mesh->indices.size() / 3;
	CreateBuildRange(accelerationStructureBuildGeometryInfo, numTriangles);
}

void AccelerationStructure::createTopLevelAccelerationStructure(std::vector<AccelerationStructure>& blas)
{
	type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f };


	std::vector<VkAccelerationStructureInstanceKHR>instances;
	instances.resize(blas.size());
	uint32_t i = 0;
	for (auto& instance : instances)
	{
		instance = Initialisers::ASInstance(transformMatrix, VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR, blas[i].deviceAddress, i);
		i++;
	}

	uint32_t primitiveCount = static_cast<uint32_t>(blas.size());

	// Buffer for instance data
	Buffer instancesBuffer;
	instancesBuffer.Create(deviceContext, sizeof(VkAccelerationStructureInstanceKHR) * primitiveCount,
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		instances.data());

	VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
	instanceDataDeviceAddress.deviceAddress = instancesBuffer.GetDeviceAddress();

	VkAccelerationStructureGeometryInstancesDataKHR instanceData = Initialisers::ASGInstanceData(instanceDataDeviceAddress);

	VkAccelerationStructureGeometryKHR accelerationStructureGeometry = Initialisers::ASG(instanceData);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = Initialisers::TLABuildGeometryInfo(&accelerationStructureGeometry);

	CreateBuildRange(accelerationStructureBuildGeometryInfo, primitiveCount);
	instancesBuffer.Destroy();
}

void AccelerationStructure::CreateBuildRange(const VkAccelerationStructureBuildGeometryInfoKHR& accelerationStructureBuildGeometryInfo,  uint32_t primitiveCount) {

	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo = Initialisers::ASBuildSizesInfo();
	vkGetAccelerationStructureBuildSizesKHR(
		deviceContext->logicalDevice,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&primitiveCount,
		&accelerationStructureBuildSizesInfo);

	createAccelerationStructureBuffer(accelerationStructureBuildSizesInfo);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = Initialisers::ASCreateInfo(asBuffer.vkBuffer,
		accelerationStructureBuildSizesInfo.accelerationStructureSize, 0, type);
	vkCreateAccelerationStructureKHR(deviceContext->logicalDevice, &accelerationStructureCreateInfo, nullptr, &handle);

	ScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo =
		Initialisers::ABuildGeometryInfo(accelerationStructureBuildGeometryInfo, handle, scratchBuffer.deviceAddress);

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo = Initialisers::ASBuildRangeInfo(primitiveCount);

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

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo = Initialisers::ASDeviceAddressInfo(handle);
	deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(deviceContext->logicalDevice, &accelerationDeviceAddressInfo);

	deleteScratchBuffer(scratchBuffer);
}