#pragma once
#include "Constants.h"

struct DescriptorBinding {

    void AddBufferData(void* data);

    void AddData(void* data);

    std::vector<void*> data;
    uint32_t descriptorCount = 1;
    VkShaderStageFlagBits shaderFlags;
    VkDescriptorType type;
    uint32_t binding;
};

struct DescriptorSetRequest {

    using LayoutSetOrder = std::pair<std::string, uint32_t>;

    DescriptorSetRequest(size_t i = 1);
    DescriptorSetRequest(const std::vector<std::pair<std::string, uint32_t>>& tags, size_t i = 1);
    ~DescriptorSetRequest() = default;
    void AddDescriptorBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits shaderFlags, uint32_t count = 1U);
    void AddDescriptorBufferData(size_t binding, void* data);
    void AddDescriptorImageData(size_t binding, void* data);
    void AddDescriptorSetLayoutTags(const std::vector<std::string>& tags);
    void AddDescriptorSetLayoutTag(const std::string& tag);
    std::vector<DescriptorBinding> bindings;
    std::vector<LayoutSetOrder> layoutTags;
    uint32_t totalSets;
};