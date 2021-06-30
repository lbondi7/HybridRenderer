#include "DescriptorPool.h"
#include "Initilizers.h"

#include <algorithm>

DescriptorPool::~DescriptorPool()
{
}

void DescriptorPool::init(DeviceContext* _devices, uint32_t _imageCount, const DescriptorSetRequest& request)
{
	devices = _devices;
    imageCount = _imageCount;

    maxSets *= imageCount;

    std::vector<VkDescriptorPoolSize> poolSizes;
    

    for (auto r : request.requests)
    {
        bool hasType = false;
        for (auto type : types)
        {
            if (type == r.second)
                hasType = true;
        }

        if (hasType)
            continue;

        types.emplace_back(r.second);
        poolSizes.emplace_back(Initialisers::descriptorPoolSize(r.second, maxSets));
    }


    VkDescriptorPoolCreateInfo poolInfo = Initialisers::descriptorPoolCreateInfo(static_cast<uint32_t>(poolSizes.size()), poolSizes.data(), maxSets);

    if (vkCreateDescriptorPool(devices->logicalDevice, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    sets.reserve(static_cast<size_t>(maxSets));
    sets.resize(static_cast<size_t>(maxSets));
}

void DescriptorPool::destroy()
{

    vkDestroyDescriptorPool(devices->logicalDevice, pool, nullptr);
}

bool DescriptorPool::isAvailable(const DescriptorSetRequest& request)
{
    if (count >= maxSets)
        return false;

    for (auto r : request.requests)
    {
        bool hasType = false;
        for (auto type : types)
        {
            if (type == r.second)
            {
                hasType = true;
            }
        }

        if (hasType)
            continue;

        return false;
    }

    return true;
}

void DescriptorPool::allocateSets(std::vector<VkDescriptorSet>& _sets, VkDescriptorSetLayout layout, const DescriptorSetRequest& request)
{

    std::vector<VkDescriptorSetLayout> layouts(imageCount, layout);
    VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(pool, imageCount, layouts.data());

    for (size_t i = count; i < count + imageCount; i++)
    {
        _sets.emplace_back(sets[count]);
    }

    if (vkAllocateDescriptorSets(devices->logicalDevice, &allocInfo, _sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    count += imageCount;
}
