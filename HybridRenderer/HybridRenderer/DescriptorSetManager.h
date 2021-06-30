#pragma once
#include "Constants.h"
#include "DescriptorSetLayout.h"
#include "DescriptorPool.h"

class DescriptorSetManager
{
public:
	DescriptorSetManager() = default;
	~DescriptorSetManager();

	void init(DeviceContext* _devices, uint32_t _imageCount);

	void destroy(bool complete = true);

	void addLayout(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	DescriptorSetLayout* addLayoutAndReturn(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	void getDescriptorSets(std::vector<VkDescriptorSet>& sets, const DescriptorSetRequest& request);

	std::vector<std::unique_ptr<DescriptorSetLayout>> layouts;

	std::vector<DescriptorPool> pools;

	uint32_t imageCount = 0;

	DeviceContext* devices = nullptr;
private:

};

