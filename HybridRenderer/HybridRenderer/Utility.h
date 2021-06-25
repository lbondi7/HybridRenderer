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

	VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

	SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

	bool isDeviceSuitable(VkSurfaceKHR surface, VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

};

