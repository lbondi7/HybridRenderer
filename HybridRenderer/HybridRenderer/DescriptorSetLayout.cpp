#include "DescriptorSetLayout.h"

#include "Initilizers.h"

DescriptorSetLayout::~DescriptorSetLayout()
{
}

void DescriptorSetLayout::init(DeviceContext* _devices)
{
    devices = _devices;

    VkDescriptorSetLayoutCreateInfo layoutInfo =
        Initialisers::descriptorSetLayoutCreateInfo(bindings.data(), static_cast<uint32_t>(bindings.size()));

    if (vkCreateDescriptorSetLayout(devices->logicalDevice, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorSetLayout::destroy()
{
    vkDestroyDescriptorSetLayout(devices->logicalDevice, layout, nullptr);
}

bool DescriptorSetLayout::checkBindings(const std::vector<VkDescriptorSetLayoutBinding>& other)
{
    if (bindings.size() != other.size())
        return false;

    for (size_t i = 0; i < bindings.size(); i++)
    {
        if (bindings[i].binding != other[i].binding || bindings[i].descriptorCount != other[i].descriptorCount ||
            bindings[i].descriptorType != other[i].descriptorType || bindings[i].stageFlags != other[i].stageFlags)
            return false;
    }

    return true;
}

bool DescriptorSetLayout::matches(const DescriptorSetRequest& request)
{

    if (request.requests.size() != bindings.size())
        return false;

    for (size_t i = 0; i < request.requests.size(); i++)
    {
        if (bindings[i].binding != request.requests[i].first ||
            bindings[i].descriptorType != request.requests[i].second)
            return false;
    }

    return true;
}
