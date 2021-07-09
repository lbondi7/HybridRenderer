#include "DescriptorPool.h"
#include "Initilizers.h"

#include <algorithm>

DescriptorPool::~DescriptorPool()
{
}

void DescriptorPool::init(DeviceContext* _devices, uint32_t _imageCount, const DescriptorSetRequest& request, bool _temporary)
{
	devices = _devices;
    imageCount = _imageCount;
    temporary = _temporary;
    maxSets *= imageCount;

    std::vector<VkDescriptorPoolSize> poolSizes;
    

    for (auto r : request.ids)
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
    //vkFreeDescriptorSets(devices->logicalDevice, pool, count, sets.data());
    vkDestroyDescriptorPool(devices->logicalDevice, pool, nullptr);
    count = 0;
}

bool DescriptorPool::isAvailable(const DescriptorSetRequest& request)
{
    if (count >= maxSets)
        return false;

    for (auto r : request.ids)
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

VkDescriptorSet DescriptorPool::allocateSet(VkDescriptorSetLayout layout, const DescriptorSetRequest& request)
{

    std::vector<VkDescriptorSetLayout> layouts(1, layout);
    VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(pool, 1, layouts.data());

    if (vkAllocateDescriptorSets(devices->logicalDevice, &allocInfo, &sets[count]) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    count += 1;
    return sets[count - 1];
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

void DescriptorPool::allocateAndUpdateSets(std::vector<VkDescriptorSet>& _sets, VkDescriptorSetLayout layout, const DescriptorSetRequest& request)
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

    auto writeCount = static_cast<uint32_t>(request.data.size()) / imageCount;
    for (size_t i = 0; i < imageCount; i++) {

        std::vector<VkWriteDescriptorSet> descriptorWrites;
        
        //VkDescriptorImageInfo
        
        for (size_t j = 0; j < writeCount; ++j) {
            bool isImage = false;

            auto& descriptor = request.ids[j];
            isImage = 
                descriptor.second == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || 
                descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
                descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLER || 
                descriptor.second == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

            if (isImage)
                descriptorWrites.push_back(Initialisers::writeDescriptorSet(_sets[i], descriptor.first, descriptor.second, (const VkDescriptorImageInfo*)request.data[i * writeCount + j]));
            else                                                            
                descriptorWrites.push_back(Initialisers::writeDescriptorSet(_sets[i], descriptor.first, descriptor.second, (const VkDescriptorBufferInfo*)request.data[i * writeCount + j]));
        }

        vkUpdateDescriptorSets(devices->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    }

    count += imageCount;

}

void DescriptorPool::allocateAndUpdateSet(VkDescriptorSet* _set, VkDescriptorSetLayout layout, const DescriptorSetRequest& request)
{
    std::vector<VkDescriptorSetLayout> layouts(1, layout);
    VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(pool, 1, layouts.data());


    if (vkAllocateDescriptorSets(devices->logicalDevice, &allocInfo, _set) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    auto writeCount = static_cast<uint32_t>(request.data.size());

    std::vector<VkWriteDescriptorSet> descriptorWrites;

    //VkDescriptorImageInfo

    for (size_t j = 0; j < writeCount; ++j) {
        bool isImage = false;

        auto& descriptor = request.ids[j];
        isImage =
            descriptor.second == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
            descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
            descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLER ||
            descriptor.second == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

        if (isImage)
            descriptorWrites.push_back(Initialisers::writeDescriptorSet(*_set, descriptor.first, descriptor.second, (const VkDescriptorImageInfo*)request.data[j]));
        else
            descriptorWrites.push_back(Initialisers::writeDescriptorSet(*_set, descriptor.first, descriptor.second, (const VkDescriptorBufferInfo*)request.data[j]));
    }

    vkUpdateDescriptorSets(devices->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);


    count++;

}

void DescriptorPool::freeDescriptorSet(VkDescriptorSet *_set)
{

    vkFreeDescriptorSets(devices->logicalDevice, pool, 1, _set);
    count--;
}