#include "DescriptorSetManager.h"

DescriptorSetManager::~DescriptorSetManager()
{
}

void DescriptorSetManager::init(DeviceContext* _devices, uint32_t _imageCount)
{
	devices = _devices;
	imageCount = _imageCount;
}

void DescriptorSetManager::destroy(bool complete)
{
	for (auto& pool : pools)
	{
		pool.destroy();
	}

	pools.clear();

	if (!complete)
		return;

	for (auto& layout : layouts)
	{
		layout->destroy();
	}
}


void DescriptorSetManager::addLayout(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	layouts.push_back(std::make_unique<DescriptorSetLayout>());

	layouts[layouts.size() - 1]->bindings = bindings;
	layouts[layouts.size() - 1]->set = set;
	layouts[layouts.size() - 1]->init(devices);
}

DescriptorSetLayout* DescriptorSetManager::addLayoutAndReturn(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings, bool newLayout)
{
	if (!newLayout) {
		for (auto& layout : layouts)
		{
			if (layout->checkBindings(bindings))
				return layout.get();
		}
	}

	layouts.push_back(std::make_unique<DescriptorSetLayout>());


	layouts[layouts.size() - 1]->bindings = bindings;
	layouts[layouts.size() - 1]->set = set;
	layouts[layouts.size() - 1]->init(devices);

	return layouts[layouts.size() - 1].get();
}

void DescriptorSetManager::getDescriptorSets(std::vector<VkDescriptorSet>& sets, const DescriptorSetRequest& request)
{

	for (auto& pool : pools)
	{
		if (pool.isAvailable(request))
		{
			for (auto& layout :layouts)
			{
				if (layout->matches(request))
				{
					pool.allocateSets(sets, layout->layout, request);
					return;
				}
			}
		}
	}

	auto& pool = pools.emplace_back();
	pool.init(devices, imageCount, request);
	if (pool.isAvailable(request))
	{
		for (auto& layout : layouts)
		{
			if (layout->matches(request))
			{
				pool.allocateSets(sets, layout->layout, request);
				return;
			}
		}
	}
}

void DescriptorSetManager::createDescriptorSets(std::vector<VkDescriptorSet>& sets, const DescriptorSetRequest& request)
{
	for (auto& pool : pools)
	{
		if (pool.isAvailable(request))
		{
			for (auto& layout : layouts)
			{
				if (layout->matches(request))
				{
					pool.allocateAndUpdateSets(sets, layout->layout, request);
					return;
				}
			}
		}
	}

	auto& pool = pools.emplace_back();
	pool.init(devices, imageCount, request);
	if (pool.isAvailable(request))
	{
		for (auto& layout : layouts)
		{
			if (layout->matches(request))
			{
				pool.allocateAndUpdateSets(sets, layout->layout, request);
				return;
			}
		}
	}

}

VkDescriptorSet DescriptorSetManager::getDescriptorSet(const DescriptorSetRequest& request)
{

	for (auto pool : pools)
	{
		if (pool.isAvailable(request))
		{
			for (auto& layout : layouts)
			{
				if (layout->matches(request))
				{
					return pool.allocateSet(layout->layout, request);
				}
			}
		}
	}

	auto& pool = pools.emplace_back();
	pool.init(devices, imageCount, request);
	if (pool.isAvailable(request))
	{
		for (auto& layout : layouts)
		{
			if (layout->matches(request))
			{
				return pool.allocateSet(layout->layout, request);
			}
		}
	}
}

void DescriptorSetManager::getTempDescriptorSet(VkDescriptorSet* descriptorSet, const DescriptorSetRequest& request, bool temp)
{

	for (auto pool : pools)
	{
		if (pool.isAvailable(request) && pool.temporary)
		{
			for (auto& layout : layouts)
			{
				if (layout->matches(request))
				{
					pool.allocateAndUpdateSet(descriptorSet, layout->layout, request);
					return;
				}
			}
		}
	}

	auto& pool = pools.emplace_back();
	pool.init(devices, imageCount, request, temp);
	for (auto& layout : layouts)
	{
		if (layout->matches(request))
		{
			pool.allocateAndUpdateSet(descriptorSet, layout->layout, request);
			return;
		}
	}
}

void DescriptorSetManager::freeDescriptorSet(VkDescriptorSet* descriptorSet)
{
	for (auto pool : pools)
	{
		if (pool.temporary)
		{
			pool.freeDescriptorSet(descriptorSet);
		}
	}
}