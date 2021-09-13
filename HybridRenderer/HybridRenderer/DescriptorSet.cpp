#include "DescriptorSet.h"

#include "Initilizers.h"

DescriptorSet::~DescriptorSet()
{
	poolData = nullptr;
}

void DescriptorSet::initialise()
{
}

//void DescriptorSet::update(VkDevice logicalDevice, const DescriptorSetRequest& _request)
//{
//    auto writeCount = static_cast<uint32_t>(_request.data.size()) / 3;
//
//    for (size_t i = 0; i < 3; i++) {
//
//        std::vector<VkWriteDescriptorSet> descriptorWrites;
//
//        for (size_t j = 0; j < writeCount; ++j) {
//            bool isImage = false;
//
//            auto& descriptor = _request.ids[j];
//            isImage =
//                descriptor.second == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
//                descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
//                descriptor.second == VK_DESCRIPTOR_TYPE_SAMPLER ||
//                descriptor.second == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
//
//            if (isImage)
//                descriptorWrites.push_back(Initialisers::writeDescriptorSet(descriptorSet, descriptor.first, descriptor.second, (const VkDescriptorImageInfo*)_request.data[i * writeCount + j]));
//            else
//                descriptorWrites.push_back(Initialisers::writeDescriptorSet(descriptorSet, descriptor.first, descriptor.second, (const VkDescriptorBufferInfo*)_request.data[i * writeCount + j]));
//        }
//
//        vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
//
//    }
//
//}
