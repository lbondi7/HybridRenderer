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
	layouts[layouts.size() - 1]->init(logicalDevice);

	return layouts[layouts.size() - 1].get();
}

void DescriptorSetManager::GetLayouts(std::vector<DescriptorSetLayout*>& layouts, const std::string& name)
{
	for (auto& layout : this->layouts)
	{
		if (layout->tag == name)
			layouts.push_back(layout.get());
	}
}

void DescriptorSetManager::getDescriptor(Descriptor& descriptor, const DescriptorSetRequest& request)
{

	VkDescriptorSetLayout chosenLayout = VK_NULL_HANDLE;

	for (auto& layout : layouts)
	{
		if (layout->matches(request))
		{
			chosenLayout = layout->layout;
		}
	}

	if (chosenLayout == VK_NULL_HANDLE)
	{
		for (auto& tag : request.layoutTags)
		{
			layouts.push_back(std::make_unique<DescriptorSetLayout>());
			auto& layout = layouts[layouts.size() - 1];
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			bindings.reserve(request.bindings.size());
			for (size_t i = 0; i < request.bindings.size(); i++)
			{
				auto& binding = request.bindings[i];
				auto& b = bindings.emplace_back();
				b.binding = binding.binding;
				b.descriptorCount = binding.descriptorCount;
				b.descriptorType = binding.type;
				b.stageFlags = binding.shaderFlags;
			}

			layout->bindings = bindings;
			layout->tag = tag.first;
			layout->set = tag.second;
			layout->init(logicalDevice);
			chosenLayout = layout->layout;
		}
	}

	for (auto& pool : pools)
	{
		if (pool.isAvailable(request))
		{
			pool.allocate(descriptor, chosenLayout, request);
			update(descriptor, request);
			return;
		}
	}

	auto& pool = pools.emplace_back();
	pool.init(logicalDevice, request);
	if (pool.isAvailable(request))
	{
		pool.allocate(descriptor, chosenLayout, request);
		update(descriptor, request);
		return;
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

	auto writeCount = static_cast<uint32_t>(request.bindings.size());


	for (size_t i = 0; i < 3; i++) {

		std::vector<VkWriteDescriptorSet> descriptorWrites;

		for (size_t j = 0; j < writeCount; ++j) {
			bool isImage = false;

			auto& descriptorInfo = request.bindings[j];
			isImage =
				descriptorInfo.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
				descriptorInfo.type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
				descriptorInfo.type == VK_DESCRIPTOR_TYPE_SAMPLER ||
				descriptorInfo.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

			bool isAccelerationStructure = descriptorInfo.type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR ||
				descriptorInfo.type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

			if (isImage)
				descriptorWrites.push_back(Initialisers::writeDescriptorSet(descriptor.sets[i],
					descriptorInfo.binding, descriptorInfo.type, (const VkDescriptorImageInfo*)descriptorInfo.data[i], descriptorInfo.descriptorCount));
			else if(isAccelerationStructure)
				descriptorWrites.push_back(Initialisers::writeDescriptorSet(descriptor.sets[i], 
					descriptorInfo.binding, descriptorInfo.type, (const VkWriteDescriptorSetAccelerationStructureKHR*)descriptorInfo.data[i], descriptorInfo.descriptorCount));
			else
				descriptorWrites.push_back(Initialisers::writeDescriptorSet(descriptor.sets[i], 
					descriptorInfo.binding, descriptorInfo.type, (const VkDescriptorBufferInfo*)descriptorInfo.data[i], descriptorInfo.descriptorCount));
		}

		vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	}
}