#pragma once
#include "Constants.h"

#include "DescriptorPoolData.h"

class DescriptorSet
{
public:
	DescriptorSet() = default;
	~DescriptorSet();

	void initialise();

	void update(VkDevice logicalDevice, const DescriptorSetRequest& _request);


	VkDescriptorSet descriptorSet;

	DescriptorPoolData* poolData;

};

