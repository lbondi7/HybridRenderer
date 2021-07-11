#pragma once
#include "Constants.h"

class DescriptorSetLayout
{
public:
	DescriptorSetLayout() = default;
	~DescriptorSetLayout();

	void init(VkDevice logicalDevice);

	void destroy();

	bool checkBindings(const std::vector<VkDescriptorSetLayoutBinding>& other);

	bool matches(const DescriptorSetRequest& request);

	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	uint32_t set = 0;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
private:
	VkDevice logicalDevice;
};