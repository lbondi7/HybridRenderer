#pragma once
#include "Constants.h"
#include "Device.h"

class DescriptorSetLayout
{
public:
	DescriptorSetLayout() = default;
	~DescriptorSetLayout();



	void init(DeviceContext* _devices);

	void destroy();

	bool checkBindings(const std::vector<VkDescriptorSetLayoutBinding>& other);

	bool matches(const DescriptorSetRequest& request);

	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	uint32_t set = 0;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
private:
	DeviceContext *devices = nullptr;
};

//bool DescriptorSetLayout::operator == (const std::vector<VkDescriptorSetLayoutBinding>& other)
//{
//	if (bindings.size() != other.size())
//		return false;
//
//	for (size_t i = 0; i < bindings.size(); i++)
//	{
//		if (bindings[i].binding != other[i].binding || bindings[i].descriptorCount != other[i].descriptorCount ||
//			bindings[i].descriptorType != other[i].descriptorType || bindings[i].stageFlags != other[i].stageFlags)
//			return false;
//	}
//
//	return true;
//}