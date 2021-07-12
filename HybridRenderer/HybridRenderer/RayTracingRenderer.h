//#pragma once
//
//#include "Constants.h"
//#include "Buffer.h"
//#include "DeviceContext.h"
//
//struct RayTracingScratchBuffer
//{
//	uint64_t deviceAddress = 0;
//	VkBuffer handle = VK_NULL_HANDLE;
//	VkDeviceMemory memory = VK_NULL_HANDLE;
//};
//
//// Ray tracing acceleration structure
//struct AccelerationStructure {
//	VkAccelerationStructureKHR handle;
//	uint64_t deviceAddress = 0;
//	VkDeviceMemory memory;
//	VkBuffer buffer;
//};
//
//
//class RayTracingRenderer
//{
//public:
//	RayTracingRenderer() = default;
//	~RayTracingRenderer();
//
//	void initialise();
//
//	RayTracingScratchBuffer createScratchBuffer(VkDeviceSize size);
//
//	void deleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer);
//
//	void createAccelerationStructureBuffer(AccelerationStructure& accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
//
//	uint64_t getBufferDeviceAddress(VkBuffer buffer);
//
//	void createStorageImage();
//
//	void createBottomLevelAccelerationStructure();
//
//	void createTopLevelAccelerationStructure();
//
//	void createShaderBindingTable();
//
//	void createDescriptorSets();
//
//	void createRayTracingPipeline();
//
//	void createUniformBuffer();
//
//	void handleResize();
//
//	void buildCommandBuffers();
//
//	void updateUniformBuffers();
//
//	void getEnabledFeatures();
//
//
//	//PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
//	//PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
//	//PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
//	//PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
//	//PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
//	//PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
//	//PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
//	//PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
//	//PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
//	//PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
//
//	//VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
//	//VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
//
//	//VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
//	//VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
//	//VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
//
//	AccelerationStructure bottomLevelAS{};
//	AccelerationStructure topLevelAS{};
//
//	Buffer vertexBuffer;
//	Buffer indexBuffer;
//	uint32_t indexCount;
//	Buffer transformBuffer;
//	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
//	Buffer raygenShaderBindingTable;
//	Buffer missShaderBindingTable;
//	Buffer hitShaderBindingTable;
//
//	struct StorageImage {
//		VkDeviceMemory memory;
//		VkImage image;
//		VkImageView view;
//		VkFormat format;
//	} storageImage;
//
//	struct UniformData {
//		glm::mat4 viewInverse;
//		glm::mat4 projInverse;
//	} uniformData;
//	Buffer ubo;
//
//	VkPipeline pipeline;
//	VkPipelineLayout pipelineLayout;
//	VkDescriptorSet descriptorSet;
//	VkDescriptorSetLayout descriptorSetLayout;
//
//	DeviceContext* deviceContext;
//
//};
//
