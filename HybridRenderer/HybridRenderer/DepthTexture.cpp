#include "DepthTexture.h"

#include "Utility.h"
#include "Initilizers.h"


void DepthTexture::Create(DeviceContext* _devices, uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties)
{
}


//void DepthTexture::createImage() {
//
//    VkImageCreateInfo imageInfo = Initialisers::imageCreateInfo(VK_IMAGE_TYPE_2D, width, height, 1, format, usage, tiling, VK_SAMPLE_COUNT_1_BIT);
//
//    if (vkCreateImage(devices->logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create image!");
//    }
//
//    VkMemoryRequirements memRequirements;
//    vkGetImageMemoryRequirements(devices->logicalDevice, image, &memRequirements);
//
//    VkMemoryAllocateInfo allocInfo = Initialisers::memoryAllocateInfo(memRequirements.size, Utility::findMemoryType(memRequirements.memoryTypeBits, devices->physicalDevice, properties));
//    //allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//    //allocInfo.allocationSize = memRequirements.size;
//    //allocInfo.memoryTypeIndex = Utility::findMemoryType(memRequirements.memoryTypeBits, devices->physicalDevice, properties);
//
//    if (vkAllocateMemory(devices->logicalDevice, &allocInfo, nullptr, &vkMemory) != VK_SUCCESS) {
//        throw std::runtime_error("failed to allocate image memory!");
//    }
//
//    vkBindImageMemory(devices->logicalDevice, image, vkMemory, 0);
//}