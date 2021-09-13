#include "DescriptorSetManager.h"
#include "Initilizers.h"

DescriptorSetManager::~DescriptorSetManager()
{
}

void DescriptorSetManager::init(VkDevice _logicalDevice)
{
	logicalDevice = _logicalDevice;
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
	layouts[layouts.size() - 1]->init(logicalDevice);
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
	layouts[layouts.size() - 1]->init(logicalDevice);

	return layouts[layouts.size() - 1].get();
}

void DescriptorSetManager::getDescriptor(Descriptor& descriptor, const DescriptorSetRequest& request)
{
	for (auto& pool : pools)
	{
		if (pool.isAvailable(request))
		{
			for (auto& layout : layouts)
			{
				if (layout->matches(request))
				{
					pool.allocate(descriptor, layout->layout, request);
					update(descriptor, request);
					return;
				}
			}
		}
	}

	auto& pool = pools.emplace_back();
	pool.init(logicalDevice, request);
	if (pool.isAvailable(request))
	{
		for (auto& layout : layouts)
		{
			if (layout->matches(request))
			{
				pool.allocate(descriptor, layout->layout, request);
				update(descriptor, request);
				return;
			}
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

void DescriptorSetManager::update(Descriptor& descriptor, const DescriptorSetRequest& request)
{

	auto writeCount = static_cast<uint32_t>(request.data.size()) / 3;

	for (size_t i = 0; i < 3; i++) {

		std::vector<VkWriteDescriptorSet> descriptorWrites;

		for (size_t j = 0; j < writeCount; ++j) {
			bool isImage = false;

			auto [binding, descriptorType, shaderStage] = request.ids[j];
			auto& descriptorInfo = request.ids[j];
			isImage =
				descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
				descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
				descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
				descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

			bool isAccelerationStructure = descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR ||
				descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

			if (isImage)
				descriptorWrites.push_back(Initialisers::writeDescriptorSet(descriptor.sets[i], binding, descriptorType, (const VkDescriptorImageInfo*)request.data[i * writeCount + j]));
			else if(isAccelerationStructure)
				descriptorWrites.push_back(Initialisers::writeDescriptorSet(descriptor.sets[i], binding, descriptorType, (const VkWriteDescriptorSetAccelerationStructureKHR*)request.data[i * writeCount + j]));
			else
				descriptorWrites.push_back(Initialisers::writeDescriptorSet(descriptor.sets[i], binding, descriptorType, (const VkDescriptorBufferInfo*)request.data[i * writeCount + j]));
		}

		vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	}
}