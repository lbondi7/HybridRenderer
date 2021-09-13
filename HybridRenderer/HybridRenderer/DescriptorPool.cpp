#include "DescriptorPool.h"
#include "Initilizers.h"

#include <algorithm>

DescriptorPool::~DescriptorPool()
{
}

void DescriptorPool::init(VkDevice _logicalDevice, const DescriptorSetRequest& request, bool _temporary)
{
    logicalDevice = _logicalDevice;
    temporary = _temporary;
    

    for (auto r : request.ids)
    {
        auto& descriptorType = std::get<1>(r);
        bool hasType = false;
        for (auto type : types)
        {
            if (type == descriptorType)
                hasType = true;
        }

        if (hasType)
            continue;

        types.emplace_back(descriptorType);
        //poolSizes.emplace_back(Initialisers::descriptorPoolSize(r.second, maxSets));
    }


    std::vector<VkDescriptorPoolSize> poolSizes =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        //{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        //{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        //{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        //{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        //{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1000 }
    };

    //data.count.insert(std::make_pair(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, std::make_pair(0, 1000)));
    //data.count.insert(std::make_pair(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, std::make_pair(0, 1000)));

    for (auto& poolSize : poolSizes)
    {
        data.count.insert(std::make_pair(poolSize.type, std::make_pair(0, poolSize.descriptorCount)));
        maxSets += poolSize.descriptorCount;
    }

    VkDescriptorPoolCreateInfo poolInfo = Initialisers::descriptorPoolCreateInfo(static_cast<uint32_t>(poolSizes.size()), poolSizes.data(), maxSets);

    if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &data.pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    dsets.reserve(static_cast<size_t>(maxSets));
    dsets.resize(static_cast<size_t>(maxSets));
}

void DescriptorPool::destroy()
{
    //vkFreeDescriptorSets(devices->logicalDevice, pool, count, sets.data());
    vkDestroyDescriptorPool(logicalDevice, data.pool, nullptr);
    count = 0;
}

//bool DescriptorPool::isAvailable(const DescriptorSetRequest& request)
//{
//    if (count >= maxSets)
//        return false;
//
//    for (auto r : request.ids)
//    {
//        bool hasType = false;
//        for (auto type : types)
//        {
//            if (type == std::get<1>(r))
//            {
//                hasType = true;
//            }
//        }
//
//        if (hasType)
//            continue;
//
//        return false;
//    }
//
//    return true;
//}

bool DescriptorPool::isAvailable(const DescriptorSetRequest& request)
{

    for (auto r : request.ids)
    {

        auto& count = data.count[std::get<1>(r)];

        if (count.first >= count.second)
            return false;
    }

    return true;
}

VkDescriptorSet DescriptorPool::allocateSet(VkDescriptorSetLayout layout, const DescriptorSetRequest& request)
{

    std::vector<VkDescriptorSetLayout> layouts(1, layout);
    VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(pool, 1, layouts.data());

    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &sets[count]) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    count += 1;
    return sets[count - 1];
}

void DescriptorPool::freeDescriptorSet(VkDescriptorSet *_set)
{

    vkFreeDescriptorSets(logicalDevice, pool, 1, _set);
    count--;
}

void DescriptorPool::allocate(Descriptor& descriptor, VkDescriptorSetLayout layout, const DescriptorSetRequest& request)
{

    auto imageCount = 3;
    std::vector<VkDescriptorSetLayout> layouts(imageCount, layout);
    VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(data.pool, imageCount, layouts.data());

    descriptor.sets.resize(imageCount);
    descriptor.pool = data.pool;
    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, descriptor.sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < imageCount; i++)
    {
        for (auto& r : request.ids)
        {
            data.count[std::get<1>(r)].first++;
        }
    }
}