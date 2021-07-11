#pragma once

#include "Constants.h"
#include "DescriptorPoolData.h"
#include "DescriptorSet.h"
#include "Descriptor.h"

class DescriptorPool
{
public:

	DescriptorPool() = default;
	~DescriptorPool();

	void init(VkDevice logicalDevice, const DescriptorSetRequest& request, bool _temporary = false);

	void destroy();

	bool isAvailable(const DescriptorSetRequest& request);

	bool isAvailable2(const DescriptorSetRequest& request);

	VkDescriptorSet allocateSet(VkDescriptorSetLayout layout, const DescriptorSetRequest& request);


	void allocateSets(std::vector<VkDescriptorSet>& _sets, VkDescriptorSetLayout layout, const DescriptorSetRequest& request);

	void allocateAndUpdateSets(std::vector<VkDescriptorSet>* _sets, VkDescriptorSetLayout layout, const DescriptorSetRequest& request);

	void allocateAndUpdateSet(VkDescriptorSet* _set, VkDescriptorSetLayout layout, const DescriptorSetRequest& request);

	void freeDescriptorSet(VkDescriptorSet* _set);

	void allocateAndUpdateSets(std::vector<DescriptorSet*>* _sets, VkDescriptorSetLayout layout, const DescriptorSetRequest& request);


	void allocate(Descriptor& descriptor, VkDescriptorSetLayout layout, const DescriptorSetRequest& request);

	
	VkDescriptorPool pool;

	std::vector<VkDescriptorSet> sets;

	bool temporary = false;

	DescriptorPoolData data;

	uint32_t maxSets = 0;
	uint32_t count = 0;
	size_t currentIndex = 0;

	VkDevice logicalDevice;

	std::vector<DescriptorSet> dsets;

	std::vector<VkDescriptorType> types;
};

