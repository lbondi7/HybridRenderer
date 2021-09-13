#pragma once

struct Descriptor
{
	std::vector<VkDescriptorSet> sets;
	std::vector<VkDescriptorType> types;
	VkDescriptorPool pool;
	DescriptorSetRequest requestData;
};

