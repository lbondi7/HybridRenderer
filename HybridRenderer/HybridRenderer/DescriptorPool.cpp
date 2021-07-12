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
        bool hasType = false;
        for (auto type : types)
        {
            if (type == r.second)
                hasType = true;
        }

        if (hasType)
            continue;

        types.emplace_back(r.second);
        //poolSizes.emplace_back(Initialisers::descriptorPoolSize(r.second, maxSets));
    }


    std::vector<VkDescriptorPoolSize> poolSizes =
    {
        //{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        //{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        //{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        //{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        //{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        //{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        //{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        //{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        //{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
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

bool DescriptorPool::isAvailable2(const DescriptorSetRequest& request)
{

    for (auto r : request.ids)
    {
        auto& count = data.count[r.second];

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


void DescriptorPool::allocateSets(std::vector<VkDescriptorSet>& _sets, VkDescriptorSetLayout layout, const DescriptorSetRequest& request)
{

    //std::vector<VkDescriptorSetLayout> layouts(imageCount, layout);
    //VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(pool, imageCount, layouts.data());

    //for (size_t i = count; i < count + devices->imageCount; i++)
    //{
    //    _sets.emplace_back(sets[count]);
    //}

    //if (vkAllocateDescriptorSets(devices->logicalDevice, &allocInfo, _sets.data()) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to allocate descriptor sets!");
    //}
    //count += devices->imageCount;
}

void DescriptorPool::allocateAndUpdateSets(std::vector<VkDescriptorSet>* _sets, VkDescriptorSetLayout layout, const DescriptorSetRequest& request)
{
    //std::vector<VkDescriptorSetLayout> layouts(imageCount, layout);
    //VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(pool, imageCount, layouts.data());

    //for (size_t i = count; i < count + devices->imageCount; i++)
    //{
    //    _sets->emplace_back(sets[count]);
    //}

    //if (vkAllocateDescriptorSets(devices->logicalDevice, &allocInfo, _sets->data()) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to allocate descriptor sets!");
    //}

    //auto writeCount = static_cast<uint32_t>(request.data.size()) / devices->imageCount;
    //for (size_t i = 0; i < devices->imageCount; i++) {

    //    std::vector<VkWriteDescriptorSet> descriptorWrites;
    //    
    //    //VkDescriptorImageInfo
    //    
    //    for (size_t j = 0; j < writeCount; ++j) {
    //        bool isImage = false;

    //        auto& descriptor = request.ids[j];
    //        isImage = 
    //            descriptor.second == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || 
    //            descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
    //            descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLER || 
    //            descriptor.second == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    //        if (isImage)
    //            descriptorWrites.push_back(Initialisers::writeDescriptorSet((*_sets)[i], descriptor.first, descriptor.second, (const VkDescriptorImageInfo*)request.data[i * writeCount + j]));
    //        else                                                            
    //            descriptorWrites.push_back(Initialisers::writeDescriptorSet((*_sets)[i], descriptor.first, descriptor.second, (const VkDescriptorBufferInfo*)request.data[i * writeCount + j]));
    //    }

    //    vkUpdateDescriptorSets(devices->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    //}

    //count += devices->imageCount;

}

void DescriptorPool::allocateAndUpdateSet(VkDescriptorSet* _set, VkDescriptorSetLayout layout, const DescriptorSetRequest& request)
{
    //std::vector<VkDescriptorSetLayout> layouts(1, layout);
    //VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(pool, 1, layouts.data());


    //if (vkAllocateDescriptorSets(devices->logicalDevice, &allocInfo, _set) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to allocate descriptor sets!");
    //}

    //auto writeCount = static_cast<uint32_t>(request.data.size());

    //std::vector<VkWriteDescriptorSet> descriptorWrites;

    ////VkDescriptorImageInfo

    //for (size_t j = 0; j < writeCount; ++j) {
    //    bool isImage = false;

    //    auto& descriptor = request.ids[j];
    //    isImage =
    //        descriptor.second == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
    //        descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
    //        descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLER ||
    //        descriptor.second == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    //    if (isImage)
    //        descriptorWrites.push_back(Initialisers::writeDescriptorSet(*_set, descriptor.first, descriptor.second, (const VkDescriptorImageInfo*)request.data[j]));
    //    else
    //        descriptorWrites.push_back(Initialisers::writeDescriptorSet(*_set, descriptor.first, descriptor.second, (const VkDescriptorBufferInfo*)request.data[j]));
    //}

    //vkUpdateDescriptorSets(devices->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);


    //count++;

}

void DescriptorPool::freeDescriptorSet(VkDescriptorSet *_set)
{

    vkFreeDescriptorSets(logicalDevice, pool, 1, _set);
    count--;
}


void DescriptorPool::allocateAndUpdateSets(std::vector<DescriptorSet*>* _sets, VkDescriptorSetLayout layout, const DescriptorSetRequest& request)
{

    auto imageCount = 3;
    VkDescriptorSetAllocateInfo allocInfo = Initialisers::descriptorSetAllocateInfo(data.pool, 1, &layout);

    _sets->reserve(imageCount);

    for (size_t i = 0; i < imageCount; i++)
    {
        dsets[currentIndex].poolData = &data;
        if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &dsets[currentIndex].descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        _sets->emplace_back(&dsets[currentIndex]);
        ++currentIndex;

        for (auto& r : request.ids)
        {
            data.count[r.second].first++;
        }

        (*_sets)[i]->update(logicalDevice, request);
    }

    //auto writeCount = static_cast<uint32_t>(request.data.size()) / devices->imageCount;
    //for (size_t i = 0; i < devices->imageCount; i++) {

    //    std::vector<VkWriteDescriptorSet> descriptorWrites;

    //    for (size_t j = 0; j < writeCount; ++j) {
    //        bool isImage = false;

    //        auto& descriptor = request.ids[j];
    //        isImage =
    //            descriptor.second == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
    //            descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
    //            descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLER ||
    //            descriptor.second == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    //        if (isImage)
    //            descriptorWrites.push_back(Initialisers::writeDescriptorSet((*_sets)[i]->descriptorSet, descriptor.first, descriptor.second, (const VkDescriptorImageInfo*)request.data[i * writeCount + j]));
    //        else
    //            descriptorWrites.push_back(Initialisers::writeDescriptorSet((*_sets)[i]->descriptorSet, descriptor.first, descriptor.second, (const VkDescriptorBufferInfo*)request.data[i * writeCount + j]));
    //    }

    //    vkUpdateDescriptorSets(devices->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    //}
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
            data.count[r.second].first++;
        }
    }
}

//size_t getTypeID(VkDescriptorType type) {
//
//    //switch (type) {
//    //    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
//    //    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
//    //    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
//    //    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
//    //    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
//    //    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
//    //    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
//    //    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
//    //    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
//    //    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
//    //    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
//    //}
//
//}