#pragma once
#include "Constants.h"

class ValidationLayers
{
public:


	static bool hasSupport();

	static bool enabled();

	static const char* extentionNames();

	static void addValidationLayers(VkInstanceCreateInfo* createInfo);

	static void addValidationLayers(VkDeviceCreateInfo* createInfo);

	static void setupDebugMessenger(VkInstance instance);

	static void destroyDebugMessenger(VkInstance instance);

};

