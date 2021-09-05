#pragma once

#include "Device.h"
#include "Mesh.h"

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

	ScratchBuffer CreateScratchBuffer(VkDeviceSize size);

	void DeleteScratchBuffer(ScratchBuffer& scratchBuffer);

	void createAccelerationStructureBuffer(VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);

	void createBottomLevelAccelerationStructure(Mesh* mesh);

	void createTopLevelAccelerationStructure(std::vector<AccelerationStructure>& blas);

	void CreateBuildRange(const VkAccelerationStructureBuildGeometryInfoKHR& accelerationStructureBuildGeometryInfo, uint32_t primitiveCount);

	VkAccelerationStructureKHR handle;
	uint64_t deviceAddress = 0;
	Buffer asBuffer;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	Buffer transformBuffer;
	uint32_t indexCount;
	VkAccelerationStructureTypeKHR type;

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

