#include "AccelerationStructure.h"

#include "Utility.h"
#include "Initilizers.h"
#include "DebugLogger.h"

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
	buffer.Destroy();
}

/*
	Create the bottom level acceleration structure contains the scene's actual geometry (vertices, triangles)
*/
void AccelerationStructure::createBottomLevelAccelerationStructure(GameObject& go)
{
	type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

	auto matrix = go.GetMatrix();

	// Setup identity transform matrix
	transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};

	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			transformMatrix.matrix[i][j] = matrix[j][i];
		}
	}

	Buffer accelerationVertexBuffer;
	Buffer accelerationIndexBuffer;
	Buffer transformBuffer;

	accelerationVertexBuffer.Create(deviceContext, go.mesh->vertices.size() * sizeof(Vertex),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, go.mesh->vertices.data());

	accelerationIndexBuffer.Create(deviceContext, go.mesh->indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		go.mesh->indices.data());

	transformBuffer.Create(deviceContext, sizeof(VkTransformMatrixKHR),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&transformMatrix);

	VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

	vertexBufferDeviceAddress.deviceAddress = go.mesh->vertexBuffer.GetDeviceAddress();
	indexBufferDeviceAddress.deviceAddress = go.mesh->indexBuffer.GetDeviceAddress();
	transformBufferDeviceAddress.deviceAddress = transformBuffer.GetDeviceAddress();

	VkAccelerationStructureGeometryTrianglesDataKHR triangles = Initialisers::ASGTriangleData(vertexBufferDeviceAddress,
		sizeof(Vertex), go.mesh->vertices.size(), indexBufferDeviceAddress, transformBufferDeviceAddress);

	// Build
	VkAccelerationStructureGeometryKHR accelerationStructureGeometry =
		Initialisers::ASG(triangles);

	// Get size info
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = Initialisers::BLABuildGeometryInfo(&accelerationStructureGeometry);

	const uint32_t numTriangles = go.mesh->indices.size() / 3;
	CreateBuildRange(accelerationStructureBuildGeometryInfo, numTriangles);
	accelerationIndexBuffer.Destroy();
	accelerationVertexBuffer.Destroy();
	transformBuffer.Destroy();
}

void AccelerationStructure::createTopLevelAccelerationStructure(std::vector<AccelerationStructure>& blas)
{
	type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

	transformMatrix = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f
	};

	std::vector<VkAccelerationStructureInstanceKHR>instances;
	instances.resize(blas.size());
	uint32_t i = 0;
	for (auto& instance : instances)
	{
		instance = Initialisers::ASInstance(transformMatrix, VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR, blas[i].deviceAddress, i);
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

	buffer.Allocate(deviceContext, accelerationStructureBuildSizesInfo.accelerationStructureSize, 
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = Initialisers::ASCreateInfo(buffer.bufferInfo.buffer,
		accelerationStructureBuildSizesInfo.accelerationStructureSize, buffer.bufferInfo.offset, type);

//	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = Initialisers::ASCreateInfo(buffer.vkBuffer,
//		accelerationStructureBuildSizesInfo.accelerationStructureSize, 0, type);

	vkCreateAccelerationStructureKHR(deviceContext->logicalDevice, &accelerationStructureCreateInfo, nullptr, &handle);

	Buffer scratchBuffer;
	scratchBuffer.Create(deviceContext, accelerationStructureBuildSizesInfo.buildScratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	//ScratchBuffer scratchBuffer = CreateScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo =
		Initialisers::ABuildGeometryInfo(accelerationStructureBuildGeometryInfo, handle, scratchBuffer.GetDeviceAddress());

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo = Initialisers::ASBuildRangeInfo(primitiveCount);

	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	if (deviceContext->accelerationStructureFeatures.accelerationStructureHostCommands)
	{
		// Implementation supports building acceleration structure building on host
		vkBuildAccelerationStructuresKHR(
			deviceContext->logicalDevice,
			VK_NULL_HANDLE, 1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());
	}
	else
	{
		// Acceleration structure needs to be build on the deviceContext->logicalDevice
		VkCommandBuffer commandBuffer = deviceContext->generateCommandBuffer();
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer, 1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());
		deviceContext->EndCommandBuffer(commandBuffer);
	}

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo = Initialisers::ASDeviceAddressInfo(handle);
	deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(deviceContext->logicalDevice, &accelerationDeviceAddressInfo);

	scratchBuffer.Destroy();
}