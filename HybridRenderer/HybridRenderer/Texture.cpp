#include "Texture.h"

#include "Utility.h"
#include "Initilizers.h"

Texture::~Texture()
{
    if (!destroyed)
    {
        //if (hasSampler)
        //    vkDestroySampler(devices->logicalDevice, sampler, nullptr);
        if (hasImageView)
            vkDestroyImageView(devices->logicalDevice, imageView, nullptr);

        vkDestroyImage(devices->logicalDevice, image, nullptr);
        vkFreeMemory(devices->logicalDevice, memory, nullptr);
    }
	devices = nullptr;
}


void Texture::Create(DeviceContext* _devices, uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties) {

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

void Texture::createVkImage() {

    VkImageCreateInfo imageInfo = 
        Initialisers::imageCreateInfo(VK_IMAGE_TYPE_2D, width, height, 1, format, usage, tiling, VK_SAMPLE_COUNT_1_BIT);

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

void Texture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask, VkImageAspectFlags aspectMask,
    uint32_t layerCount, uint32_t baseArrayLayer,
    uint32_t baseMipLevel, uint32_t levelCount) {
    VkCommandBuffer commandBuffer = devices->generateCommandBuffer();

    VkImageMemoryBarrier barrier = 
        Initialisers::imageMemoryBarrier(image, 
            oldLayout, newLayout, 
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, 
            aspectMask, baseMipLevel, 
            levelCount, baseArrayLayer, layerCount);


    switch (oldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        barrier.srcAccessMask = 0;
        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;
    }

    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        if (barrier.srcAccessMask == 0)
        {
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask, dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    devices->EndCommandBuffer(commandBuffer);

    descriptorInfo.imageLayout = newLayout;
}

//void Texture::createSampler() {
//    VkPhysicalDeviceProperties properties{};
//    vkGetPhysicalDeviceProperties(devices->physicalDevice, &properties);
//
//    //VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_TRUE, properties.limits.maxSamplerAnisotropy, VK_SAMPLER_MIPMAP_MODE_LINEAR);
//    VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, properties.limits.maxSamplerAnisotropy, VK_SAMPLER_MIPMAP_MODE_LINEAR, 
//        VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_INT_OPAQUE_BLACK);
//
//    if (vkCreateSampler(devices->logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create texture sampler!");
//    }
//
//    descriptorInfo.sampler = sampler;
//    hasSampler = true;
//}
//
//void Texture::createSampler(const VkSamplerCreateInfo& samplerInfo) {
//    VkPhysicalDeviceProperties properties{};
//    vkGetPhysicalDeviceProperties(devices->physicalDevice, &properties);
//
//    ////VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_TRUE, properties.limits.maxSamplerAnisotropy, VK_SAMPLER_MIPMAP_MODE_LINEAR);
//    //VkSamplerCreateInfo samplerInfo = Initialisers::samplerCreateInfo(VK_FILTER_LINEAR, properties.limits.maxSamplerAnisotropy, VK_SAMPLER_MIPMAP_MODE_LINEAR,
//    //    VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_INT_OPAQUE_BLACK);
//
//    if (vkCreateSampler(devices->logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create texture sampler!");
//    }
//
//    descriptorInfo.sampler = sampler;
//    hasSampler = true;
//}

void Texture::createImageView(VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo = Initialisers::imageViewCreateInfo(image, VK_IMAGE_VIEW_TYPE_2D, format, aspectFlags);

    if (vkCreateImageView(devices->logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    descriptorInfo.imageView = imageView;
    hasImageView = true;
}

void Texture::CopyFromBuffer(VkBuffer buffer) {
    VkCommandBuffer commandBuffer = devices->generateCommandBuffer();

    VkBufferImageCopy region = Initialisers::bufferImageCopy(width, height);

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    devices->EndCommandBuffer(commandBuffer);
}

void Texture::CopyFromTexture(Texture other)
{
    VkCommandBuffer commandBuffer = devices->generateCommandBuffer();

    VkBufferImageCopy region = Initialisers::bufferImageCopy(width, height);

    //vkCmdCopyImage(commandBuffer, other.image, other.descriptorInfo.imageLayout, image, )

    devices->EndCommandBuffer(commandBuffer);

}


void Texture::Destroy() {
    //if(hasSampler)
    //    vkDestroySampler(devices->logicalDevice, sampler, nullptr);
    if(hasImageView)
        vkDestroyImageView(devices->logicalDevice, imageView, nullptr);

    vkDestroyImage(devices->logicalDevice, image, nullptr);
    vkFreeMemory(devices->logicalDevice, memory, nullptr);
    destroyed = true;
}

void Texture::DestroyImageViews() {
    if (hasImageView)
        vkDestroyImageView(devices->logicalDevice, imageView, nullptr);

    destroyed = true;
}


void Texture::insertImageMemoryBarrier(
    VkCommandBuffer cmdbuffer,
    VkAccessFlags srcAccessMask,
    VkAccessFlags dstAccessMask,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkImageSubresourceRange subresourceRange)
{
    VkImageMemoryBarrier imageMemoryBarrier = Initialisers::imageMemoryBarrier();
    imageMemoryBarrier.srcAccessMask = srcAccessMask;
    imageMemoryBarrier.dstAccessMask = dstAccessMask;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(
        cmdbuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
}