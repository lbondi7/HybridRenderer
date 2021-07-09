#pragma once

#include "Constants.h"
#include "Device.h"

class DescriptorPool
{
public:

	DescriptorPool() = default;
	~DescriptorPool();

	void init(DeviceContext* _devices, uint32_t _imageCount, const DescriptorSetRequest& request, bool _temporary = false);

	void destroy();

	bool isAvailable(const DescriptorSetRequest& request);

	VkDescriptorSet allocateSet(VkDescriptorSetLayout layout, const DescriptorSetRequest& request);


	void allocateSets(std::vector<VkDescriptorSet>& _sets, VkDescriptorSetLayout layout, const DescriptorSetRequest& request);

	void allocateAndUpdateSets(std::vector<VkDescriptorSet>& _sets, VkDescriptorSetLayout layout, const DescriptorSetRequest& request);

	void allocateAndUpdateSet(VkDescriptorSet* _set, VkDescriptorSetLayout layout, const DescriptorSetRequest& request);

	void freeDescriptorSet(VkDescriptorSet* _set);

	
	VkDescriptorPool pool;

	std::vector<VkDescriptorSet> sets;

	bool temporary = false;

	uint32_t maxSets = 1000;
	uint32_t count = 0;
	uint32_t imageCount = 0;

	DeviceContext* devices = nullptr;

	std::vector<VkDescriptorType> types;
};

