#pragma once

#include "Constants.h"

#include <vector>

namespace Utility
{
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkPhysicalDevice physicalDevice, VkImageTiling tiling, VkFormatFeatureFlags features);

	uint32_t findMemoryType(uint32_t typeFilter, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties);

	VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

	bool hasStencilComponent(VkFormat format);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D chooseSwapExtent(int width, int height, const VkSurfaceCapabilitiesKHR& capabilities);

	SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

	bool isDeviceSuitable(VkSurfaceKHR surface, VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	uint32_t alignedSize(uint32_t value, uint32_t alignment);



	void setImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
		VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
};