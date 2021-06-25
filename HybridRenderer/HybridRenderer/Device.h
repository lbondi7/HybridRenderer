#pragma once
#include "Constants.h"


class Device
{
public:


	VkDevice logicalDevice;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkCommandPool commandPool;

	VkPhysicalDeviceProperties physicalDeviceProperties;

	void SetupDevices(VkInstance instance, VkSurfaceKHR surface);
	void Destroy();

	VkCommandBuffer generateCommandBuffer();
	void EndCommandBuffer(VkCommandBuffer cmdBuffer);

	void createCommandPool(VkSurfaceKHR surface);

	VkFormat getDepthFormat();

	VkBool32 formatIsFilterable(VkFormat format, VkImageTiling tiling);



private:
	void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
	void createLogicalDevice(VkSurfaceKHR surface);

	VkFormat depthFormat;
	bool hasDepthFormat = false;
	VkFormatProperties depthFormatProps;
	bool hasFormatProps = false;

};

