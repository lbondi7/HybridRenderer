#pragma once
#include "Constants.h"
#include "DescriptorSetRequest.h"
#include <map>

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
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	//std::map<std::string, uint32_t> setOrder;
	std::string tag;
	uint32_t set;
private:
	VkDevice logicalDevice;
};