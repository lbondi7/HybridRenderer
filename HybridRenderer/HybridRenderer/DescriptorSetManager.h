#pragma once
#include "Constants.h"
#include "DescriptorSetLayout.h"
#include "DescriptorPool.h"
#include "Descriptor.h"

class DescriptorSetManager
{
public:
	DescriptorSetManager() = default;
	~DescriptorSetManager();

	void init(VkDevice _logicalDevice);

	void destroy(bool complete = true);

	void addLayout(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings);

	DescriptorSetLayout* addLayoutAndReturn(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings, bool newLayout = false);

	void getDescriptorSets(std::vector<VkDescriptorSet>& sets, const DescriptorSetRequest& request);

	void getDescriptor(Descriptor& descriptor, const DescriptorSetRequest& request);

	void createDescriptorSets(std::vector<VkDescriptorSet>* sets, const DescriptorSetRequest& request);

	VkDescriptorSet getDescriptorSet(const DescriptorSetRequest& request);

	void getTempDescriptorSet(VkDescriptorSet* descriptorSet, const DescriptorSetRequest& request, bool temp);

	void freeDescriptorSet(VkDescriptorSet* descriptorSet);

	void getDescriptorSets(std::vector<DescriptorSet*>* descriptorSets, const DescriptorSetRequest& request);

	void update(Descriptor& descriptor, const DescriptorSetRequest& request);

	std::vector<std::unique_ptr<DescriptorSetLayout>> layouts;

	std::vector<DescriptorPool> pools;

	uint32_t imageCount = 0;

	VkDevice logicalDevice;

private:

};