#include "CubemapTexture.h"

#include "Initilizers.h"
#include "Utility.h"

void CubemapTexture::Create(DeviceContext* _devices, uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties)
{
    devices = _devices;
    width = _width;
    height = _height;
    format = _format;
    tiling = _tiling;
    usage = _usage;
    properties = _properties;

    createVkImage();
    destroyed = false;
}

void CubemapTexture::createVkImage()
{

    VkImageCreateInfo imageInfo =
        Initialisers::imageCreateInfo(VK_IMAGE_TYPE_2D, width, height, 1, format, usage, tiling, VK_SAMPLE_COUNT_1_BIT);
    imageInfo.arrayLayers = 6;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    if (vkCreateImage(devices->logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(devices->logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = Initialisers::memoryAllocateInfo(memRequirements.size, Utility::findMemoryType(memRequirements.memoryTypeBits, devices->physicalDevice, properties));

    if (vkAllocateMemory(devices->logicalDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(devices->logicalDevice, image, memory, 0);

}

void CubemapTexture::CreateImageView(VkImageAspectFlags aspectMask)
{
    VkImageViewCreateInfo view = 
        Initialisers::imageViewCreateInfo(image, VK_IMAGE_VIEW_TYPE_CUBE, format, VK_IMAGE_ASPECT_COLOR_BIT);
    view.components = { VK_COMPONENT_SWIZZLE_R };
    view.subresourceRange.layerCount = 6;
    vkCreateImageView(devices->logicalDevice, &view, nullptr, &imageView);
}

void CubemapTexture::CreateSampler()
{
    VkSamplerCreateInfo samplerInfo = 
        Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, 1.0f, VK_SAMPLER_MIPMAP_MODE_LINEAR,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    vkCreateSampler(devices->logicalDevice, &samplerInfo, nullptr, &sampler);

}
