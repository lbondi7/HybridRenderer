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

	void GetLayouts(std::vector<DescriptorSetLayout*>& layouts, const std::string& name);

	void getDescriptor(Descriptor& descriptor, const DescriptorSetRequest& request);

	void freeDescriptorSet(VkDescriptorSet* descriptorSet);

	void update(Descriptor& descriptor, const DescriptorSetRequest& request);

	std::vector<std::unique_ptr<DescriptorSetLayout>> layouts;

	std::vector<DescriptorSetLayout> m_layouts;

	std::vector<DescriptorPool> pools;

	uint32_t imageCount = 0;

	VkDevice logicalDevice;

private:

};