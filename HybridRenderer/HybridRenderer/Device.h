#pragma once
#include "Constants.h"

#include "Allocator.h"
#include "DescriptorSetManager.h"
#include "Descriptor.h"

class DeviceContext
{
public:

	VkDevice logicalDevice;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkCommandPool commandPool;

	VkPhysicalDeviceProperties physicalDeviceProperties;

	void SetupDevices(VkInstance instance, VkSurfaceKHR surface);

	void SetupAllocator();

	void Destroy();

	VkCommandBuffer generateCommandBuffer();

	void EndCommandBuffer(VkCommandBuffer cmdBuffer);

	void createCommandPool(VkSurfaceKHR surface);

	VkFormat getDepthFormat();

	VkBool32 formatIsFilterable(VkFormat format, VkImageTiling tiling);

	void GetDescriptors(Descriptor& descriptor, const DescriptorSetRequest& request, bool update = false);


	QueueFamilyIndices indices;

	Allocator allocator;

	uint32_t imageCount;

	DescriptorSetManager dsm;


	VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

	VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
	VkPhysicalDeviceRayQueryFeaturesKHR enabledRayQueryFeatures{};

private:
	void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
	void createLogicalDevice(VkSurfaceKHR surface);

	VkFormat depthFormat;
	bool hasDepthFormat = false;
	VkFormatProperties depthFormatProps;
	bool hasFormatProps = false;

};

