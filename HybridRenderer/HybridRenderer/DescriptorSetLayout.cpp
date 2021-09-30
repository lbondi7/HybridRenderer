#include "DescriptorSetLayout.h"

#include "Initilizers.h"

DescriptorSetLayout::~DescriptorSetLayout()
{
}

void DescriptorSetLayout::init(VkDevice _logicalDevice)
{
    logicalDevice = _logicalDevice;

    VkDescriptorSetLayoutCreateInfo layoutInfo =
        Initialisers::descriptorSetLayoutCreateInfo(bindings.data(), static_cast<uint32_t>(bindings.size()));

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorSetLayout::destroy()
{
    vkDestroyDescriptorSetLayout(logicalDevice, layout, nullptr);
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
    if (request.bindings.size() != bindings.size())
        return false;

    for (size_t i = 0; i < request.bindings.size(); i++)
    {
        auto& descriptor = request.bindings[i];
        if (bindings[i].binding != descriptor.binding ||
            bindings[i].descriptorType != descriptor.type ||
            bindings[i].stageFlags != descriptor.shaderFlags ||
            bindings[i].descriptorCount != descriptor.descriptorCount)
            return false;
    }

    for (auto& tag : request.layoutTags) {
        if (this->tag == tag.first) {
            if (set == tag.second)
                return true;
        }
    }

    return false;
}
