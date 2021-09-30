#pragma once

#include "Device.h"
#include "GameObject.h"

struct ScratchBuffer
{
	uint64_t deviceAddress = 0;
	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

class AccelerationStructure
{
public:
	
	AccelerationStructure() = default;
	~AccelerationStructure();
	
	void Initialise(DeviceContext* _deviceContext);

	void Destroy();

	void createBottomLevelAccelerationStructure(GameObject& go);

	void createTopLevelAccelerationStructure(std::vector<AccelerationStructure>& blas);

	void CreateBuildRange(const VkAccelerationStructureBuildGeometryInfoKHR& accelerationStructureBuildGeometryInfo, uint32_t primitiveCount);

	VkAccelerationStructureKHR handle;
	uint64_t deviceAddress = 0;
	Buffer buffer;
	uint32_t indexCount;
	VkAccelerationStructureTypeKHR type;
	VkTransformMatrixKHR transformMatrix;

private:

	DeviceContext* deviceContext;

	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
	PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;

};

